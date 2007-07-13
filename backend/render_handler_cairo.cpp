// render_handler_cairo.cpp	-- Timothy Lee <timothy.lee@siriushk.com> 2006

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A render_handler that uses cairo


#include <cairo/cairo.h>
#include "render_handler.h"
#include "render_handler_tri.h"
#include "types.h"
#include "image.h"
#include "utility.h"

#include "tu_config.h"
#include "log.h"


namespace gnash {
namespace renderer {
namespace cairo {

static cairo_t* g_cr_output = 0;
static cairo_t* g_cr = 0;


// Converts from RGB image to 32-bit pixels in CAIRO_FORMAT_RGB24 format
static void
rgb_to_cairo_rgb24(uint8_t* dst, const image::rgb* im)
{
    uint32_t* dst32 = reinterpret_cast<uint32_t*>(dst);
    for (int y = 0;  y < im->m_height;  y++)
    {
	const uint8_t* src = image::scanline(im, y);
	for (int x = 0;  x < im->m_width;  x++, src += 3)
	{
	    // 32-bit RGB data in native endian format
	    *dst32++ = (src[0] << 16) | (src[1] << 8) | src[2];
	}
    }
}


// Converts from RGBA image to 32-bit pixels in CAIRO_FORMAT_ARGB32 format
static void
rgba_to_cairo_argb32(uint8_t* dst, const image::rgba* im)
{
    uint32_t* dst32 = reinterpret_cast<uint32_t*>(dst);
    for (int y = 0;  y < im->m_height;  y++)
    {
	const uint8_t* src = image::scanline(im, y);
	for (int x = 0;  x < im->m_width;  x++, src += 4)
	{
	    // 32-bit ARGB data in native endian format
	    *dst32++ = (src[0] << 24) | (src[1] << 16) | (src[2] << 8) | src[3];
	}
    }
}


// bitmap_info_cairo declaration
class bitmap_info_cairo : public gnash::bitmap_info
{
public:
    // Cairo image surface
    unsigned char*   m_buffer;
    cairo_surface_t* m_image;
    cairo_pattern_t* m_pattern;

    bitmap_info_cairo();
    bitmap_info_cairo(int width, int height, uint8_t* data);
    bitmap_info_cairo(image::rgb* im);
    bitmap_info_cairo(image::rgba* im);

    ~bitmap_info_cairo() {
	if (m_pattern)
	    cairo_pattern_destroy(m_pattern);
	if (m_image)  cairo_surface_destroy(m_image);
	if (m_buffer)  delete [] m_buffer;
    }
};


class render_handler_cairo : public gnash::triangulating_render_handler
{
public:
    // Some renderer state.
    cairo_t*         m_cr_mask;
    cairo_t*    m_cr_dummy;
    int              m_view_width;
    int              m_view_height;
    
    // Video buffer
    uint8_t*    m_video_buffer;
    int	        m_video_bufsize;
    
    // Enable/disable antialiasing.
    bool	m_enable_antialias;
    
    // Output size.
    float	m_display_width;
    float	m_display_height;
	
    gnash::matrix	m_current_matrix;
    gnash::cxform	m_current_cxform;
    void set_antialiased(bool enable) {
	m_enable_antialias = enable;
    }
    
    class fill_style
    {
    public:
	enum mode
	{
	    INVALID,
	    COLOR,
	    BITMAP_WRAP,
	    BITMAP_CLAMP,
	    LINEAR_GRADIENT,
	    RADIAL_GRADIENT
	};
	mode	m_mode;
	gnash::rgba	m_color;
	const bitmap_info_cairo*    m_bitmap_info;
	gnash::matrix	m_bitmap_matrix;
	gnash::cxform	m_bitmap_color_transform;
	bool	m_has_nonzero_bitmap_additive_color;
		
	fill_style()
	    :
	    m_mode(INVALID),
	    m_has_nonzero_bitmap_additive_color(false)
	    {
	    }

	// Push our style into cairo.
	void apply(/*const matrix& current_matrix*/) const
	{
	    assert(m_mode != INVALID);
	    
	    if (m_mode == COLOR)
	    {
		apply_color(m_color);
	    }
	    else if (m_mode == BITMAP_WRAP || m_mode == BITMAP_CLAMP)
	    {
		assert(m_bitmap_info != NULL);
		
		apply_color(m_color);
		
		if (m_bitmap_info != NULL)
		{
		    // Set up the texture for rendering.
		    {
			// Do the modulate part of the color
			// transform in the first pass.  The
			// additive part, if any, needs to
			// happen in a second pass.
			// FIXME!!! bitmap cannot be modulated by RGB
			cairo_set_source_rgba(g_cr,
			    m_bitmap_color_transform.m_[0][0],
			    m_bitmap_color_transform.m_[1][0],
			    m_bitmap_color_transform.m_[2][0],
			    m_bitmap_color_transform.m_[3][0]);
		    }
		    
		    cairo_pattern_t* pattern = m_bitmap_info->m_pattern;
		    
		    if (m_mode == BITMAP_CLAMP)
		    {	
			cairo_pattern_set_extend(pattern, CAIRO_EXTEND_NONE);
		    }
		    else
		    {
			cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
		    }
		    
		    // Set up the bitmap matrix
		    // FIXME!!! scaling and offset is wrong
		    cairo_matrix_t mat;
		    const gnash::matrix& m = m_bitmap_matrix;
		    cairo_matrix_init(&mat, m.m_[0][0], m.m_[1][0],
			m.m_[0][1], m.m_[1][1], m.m_[0][2], m.m_[1][2]);
		    cairo_pattern_set_matrix(pattern, &mat);

		    cairo_set_source(g_cr, pattern);
		}
	    }
	}
	
	
	// Return true if we need to do a second pass to make
	// a valid color.  This is for cxforms with additive
	// parts; this is the simplest way (that we know of)
	// to implement an additive color with stock OpenGL.
	bool	needs_second_pass() const
	    {
		if (m_mode == BITMAP_WRAP || m_mode == BITMAP_CLAMP)
		{
		    return m_has_nonzero_bitmap_additive_color;
		}
		else
		{
		    return false;
		}
	    }
	
	// Set OpenGL state for a necessary second pass.
	void	apply_second_pass() const
	    {
		assert(needs_second_pass());
		
		// The additive color also seems to be modulated by the texture. So,
		// maybe we can fake this in one pass using using the mean value of 
		// the colors: c0*t+c1*t = ((c0+c1)/2) * t*2
		// I don't know what the alpha component of the color is for.
		
		cairo_set_source_rgba(g_cr,
		    m_bitmap_color_transform.m_[0][1] / 255.0f,
		    m_bitmap_color_transform.m_[1][1] / 255.0f,
		    m_bitmap_color_transform.m_[2][1] / 255.0f,
		    m_bitmap_color_transform.m_[3][1] / 255.0f);
		
/*
		glBlendFunc(GL_ONE, GL_ONE);
*/
	    }

	void	cleanup_second_pass() const
	    {
/*
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
*/
	    }


	void	disable()  { m_mode = INVALID; }
	void	set_color(const gnash::rgba& color)  { m_mode = COLOR; m_color = color; }
	void	set_bitmap(const gnash::bitmap_info* bi, const gnash::matrix& m, bitmap_wrap_mode wm, const gnash::cxform& color_transform)
	    {
		m_mode = (wm == WRAP_REPEAT) ? BITMAP_WRAP : BITMAP_CLAMP;
		m_bitmap_info = static_cast<const bitmap_info_cairo*>(bi);
		m_bitmap_matrix = m;
		m_bitmap_color_transform = color_transform;
		m_bitmap_color_transform.clamp();
			
		m_color = gnash::rgba(
		    uint8_t(m_bitmap_color_transform.m_[0][0] * 255.0f),
		    uint8_t(m_bitmap_color_transform.m_[1][0] * 255.0f),
		    uint8_t(m_bitmap_color_transform.m_[2][0] * 255.0f),
		    uint8_t(m_bitmap_color_transform.m_[3][0] * 255.0f));
			
		if (m_bitmap_color_transform.m_[0][1] > 1.0f
		    || m_bitmap_color_transform.m_[1][1] > 1.0f
		    || m_bitmap_color_transform.m_[2][1] > 1.0f
		    || m_bitmap_color_transform.m_[3][1] > 1.0f)
		    {
			m_has_nonzero_bitmap_additive_color = true;
		    }
		else
		    {
			m_has_nonzero_bitmap_additive_color = false;
		    }
	    }
	bool	is_valid() const { return m_mode != INVALID; }
    };


    // Style state.
    enum style_index
    {
	LEFT_STYLE = 0,
	RIGHT_STYLE,
	LINE_STYLE,

	STYLE_COUNT
    };
    fill_style	m_current_styles[STYLE_COUNT];


    gnash::bitmap_info*	create_bitmap_info_rgb(image::rgb* im)
	// Given an image, returns a pointer to a bitmap_info class
	// that can later be passed to fill_styleX_bitmap(), to set a
	// bitmap fill style.
	{
	    return new bitmap_info_cairo(im);
	}


    gnash::bitmap_info*	create_bitmap_info_rgba(image::rgba* im)
	// Given an image, returns a pointer to a bitmap_info class
	// that can later be passed to fill_style_bitmap(), to set a
	// bitmap fill style.
	//
	// This version takes an image with an alpha channel.
	{
	    return new bitmap_info_cairo(im);
	}

    gnash::bitmap_info*	create_bitmap_info_alpha(int w, int h, uint8_t* data)
	// Create a bitmap_info so that it contains an alpha texture
	// with the given data (1 byte per texel).
	{
	    return new bitmap_info_cairo(w, h, data);
	}


    void	delete_bitmap_info(gnash::bitmap_info* bi)
	// Delete the given bitmap info class.
	{
	    delete bi;
	}


    // Constructor
    render_handler_cairo() :
	m_cr_mask(0), m_view_width(0), m_view_height(0),
	m_video_buffer(0), m_video_bufsize(0)
    {
	cairo_surface_t* dummy = cairo_image_surface_create(
	    CAIRO_FORMAT_A8, 1, 1);
	m_cr_dummy = cairo_create(dummy);
	cairo_surface_destroy(dummy);
    }

    // Destructor
    ~render_handler_cairo()
    {
	if (m_video_buffer)  delete [] m_video_buffer;
	if (m_cr_mask)  cairo_destroy(m_cr_mask);
	cairo_destroy(m_cr_dummy);
    }

    void	begin_display(
	const gnash::rgba& background_color,
	int /*viewport_x0*/, int /*viewport_y0*/,
	int viewport_width, int viewport_height,
	float x0, float x1, float y0, float y1)
	// Set up to render a full frame from a movie and fills the
	// background.	Sets up necessary transforms, to scale the
	// movie to fit within the given dimensions.  Call
	// end_display() when you're done.
	//
	// The rectangle (viewport_x0, viewport_y0, viewport_x0 +
	// viewport_width, viewport_y0 + viewport_height) defines the
	// window coordinates taken up by the movie.
	//
	// The rectangle (x0, y0, x1, y1) defines the pixel
	// coordinates of the movie that correspond to the viewport
	// bounds.
	{
	    g_cr = (g_cr_output ? g_cr_output : m_cr_dummy);

	    m_display_width  = fabsf(x1 - x0);
	    m_display_height = fabsf(y1 - y0);
	    m_view_width  = viewport_width;
	    m_view_height = viewport_height;

	    cairo_identity_matrix(g_cr);
	    cairo_rectangle(g_cr, x0, y0, m_display_width, m_display_height);
	    cairo_clip(g_cr);
	    cairo_scale(g_cr, viewport_width / m_display_width,
		viewport_height / m_display_height);
	    cairo_translate(g_cr, x0, y0);

	    // Clear the background, if background color has alpha > 0.
	    if (background_color.m_a > 0)
		{
		    // Draw a big quad.
		    apply_color(background_color);
		    cairo_rectangle(g_cr, x0, y0, x1 - x0, y1 - y0);
		    cairo_fill(g_cr);
		}
	}


    void	end_display()
	// Clean up after rendering a frame.
	{
	}


    void	set_matrix(const gnash::matrix& m)
	// Set the current transform for mesh & line-strip rendering.
	{
	    m_current_matrix = m;
	}


    void	set_cxform(const gnash::cxform& cx)
	// Set the current color transform for mesh & line-strip rendering.
	{
	    m_current_cxform = cx;
	}
	
    static void	apply_matrix(const gnash::matrix& m)
	// add user space transformation
	{
	    cairo_matrix_t mat;
	    cairo_matrix_init(&mat, m.m_[0][0], m.m_[1][0], m.m_[0][1],
		m.m_[1][1], m.m_[0][2], m.m_[1][2]);
	    cairo_transform(g_cr, &mat);
	}

    static void	apply_color(const gnash::rgba& c)
	// Set the given color.
	{
	    cairo_set_source_rgba(g_cr,
		c.m_r / 255.0, c.m_g / 255.0, c.m_b / 255.0, c.m_a / 255.0);
	}

    void	fill_style_disable(int fill_side)
	// Don't fill on the {0 == left, 1 == right} side of a path.
	{
	    assert(fill_side >= 0 && fill_side < 2);

	    m_current_styles[fill_side].disable();
	}


    void	line_style_disable()
	// Don't draw a line on this path.
	{
	    m_current_styles[LINE_STYLE].disable();
	}


    void	fill_style_color(int fill_side, const gnash::rgba& color)
	// Set fill style for the left interior of the shape.  If
	// enable is false, turn off fill for the left interior.
	{
	    assert(fill_side >= 0 && fill_side < 2);

	    m_current_styles[fill_side].set_color(m_current_cxform.transform(color));
	}


    void	line_style_color(const gnash::rgba& color)
	// Set the line style of the shape.  If enable is false, turn
	// off lines for following curve segments.
	{
	    m_current_styles[LINE_STYLE].set_color(m_current_cxform.transform(color));
	}


    void	fill_style_bitmap(int fill_side, const gnash::bitmap_info* bi, const gnash::matrix& m, bitmap_wrap_mode wm)
	{
	    assert(fill_side >= 0 && fill_side < 2);
	    m_current_styles[fill_side].set_bitmap(bi, m, wm, m_current_cxform);
	}
	
    void	line_style_width(float width)
	{
	    if ( width == 1.0 ) // "hairline"
	    {
		width = 20;
	    }
	    cairo_set_line_width(g_cr, width); // twips expected, it seems
	}


    void	draw_mesh_strip(const void* coords, int vertex_count)
	{
	    // Set up current style.
	    m_current_styles[LEFT_STYLE].apply();

	    cairo_save(g_cr);
	    apply_matrix(m_current_matrix);

	    // Draw the tris in cairo
	    const int16_t* vertex = static_cast<const int16_t*>(coords);
	    for (;  vertex_count > 2;  vertex_count--, vertex += 2)
	    {
	    	cairo_move_to(g_cr, vertex[0], vertex[1]);
	    	cairo_line_to(g_cr, vertex[2], vertex[3]);
	    	cairo_line_to(g_cr, vertex[4], vertex[5]);
	    }

	    cairo_surface_t* mask = 0;
	    if (m_cr_mask)  mask = cairo_get_target(m_cr_mask);

	    if (mask)  cairo_mask_surface(g_cr, mask, 0, 0);
	    else  cairo_fill(g_cr);

	    if (m_current_styles[LEFT_STYLE].needs_second_pass())
		{
		    m_current_styles[LEFT_STYLE].apply_second_pass();
		    for (;  vertex_count > 2;  vertex_count--, vertex += 2)
		    {
			cairo_move_to(g_cr, vertex[0], vertex[1]);
			cairo_line_to(g_cr, vertex[2], vertex[3]);
			cairo_line_to(g_cr, vertex[4], vertex[5]);
		    }
		    if (mask)  cairo_mask_surface(g_cr, mask, 0, 0);
		    else  cairo_fill(g_cr);
		    m_current_styles[LEFT_STYLE].cleanup_second_pass();
		}

	    cairo_restore(g_cr);
	}


    void	draw_line_strip(const void* coords, int vertex_count)
	// Draw the line strip formed by the sequence of points.
	{
	    // Set up current style.
	    m_current_styles[LINE_STYLE].apply();

	    cairo_save(g_cr);
	    apply_matrix(m_current_matrix);

	    // Draw the line-strip in cairo
	    const int16_t* vertex = static_cast<const int16_t*>(coords);
	    cairo_move_to(g_cr, vertex[0], vertex[1]);
	    for (vertex += 2;  vertex_count > 1;  vertex_count--, vertex += 2)
	    	cairo_line_to(g_cr, vertex[0], vertex[1]);
	    cairo_stroke(g_cr);

	    cairo_restore(g_cr);
	}


    void	draw_bitmap(
	const gnash::matrix& m,
	const gnash::bitmap_info* binfo,
	const gnash::rect& coords,
	const gnash::rect& /*uv_coords*/,
	const gnash::rgba& color)
	// Draw a rectangle textured with the given bitmap, with the
	// given color.	 Apply given transform; ignore any currently
	// set transforms.
	//
	// Intended for textured glyph rendering.
	{
            gnash::bitmap_info* nonconst_binfo = const_cast<gnash::bitmap_info*>(binfo);
	    bitmap_info_cairo* bi = dynamic_cast<bitmap_info_cairo*>(nonconst_binfo);
	    assert(bi);

	    apply_color(color);

	    gnash::point a, b, c, d;
	    m.transform(&a, gnash::point(coords.get_x_min(), coords.get_y_min()));
	    m.transform(&b, gnash::point(coords.get_x_max(), coords.get_y_min()));
	    m.transform(&c, gnash::point(coords.get_x_max(), coords.get_y_max()));
	    m.transform(&d, gnash::point(coords.get_x_min(), coords.get_y_max()));

	    // FIXME!!! scaling and offset is wrong
	    cairo_matrix_t mat;
	    cairo_matrix_init_scale(&mat, coords.width(), coords.height());
	    cairo_matrix_init_translate(&mat, coords.get_x_min(), coords.get_y_min());

	    cairo_matrix_t new_mat;
	    cairo_matrix_init(&new_mat, m.m_[0][0], m.m_[1][0], m.m_[0][1],
		m.m_[1][1], m.m_[0][2], m.m_[1][2]);

	    cairo_matrix_multiply(&mat, &mat, &new_mat);

	    cairo_pattern_t* pattern = bi->m_pattern;
	    cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
	    cairo_pattern_set_matrix(pattern, &mat);
	    cairo_set_source(g_cr, pattern);

	    cairo_save(g_cr);
	    cairo_move_to(g_cr, a.m_x, a.m_y);
	    cairo_line_to(g_cr, b.m_x, b.m_y);
	    cairo_line_to(g_cr, c.m_x, c.m_y);
	    cairo_line_to(g_cr, d.m_x, d.m_y);
	    cairo_clip(g_cr);
	    cairo_paint(g_cr);
	    cairo_restore(g_cr);
	}
	
    void begin_submit_mask()
	{
	    if (m_cr_mask)  cairo_destroy(m_cr_mask);
	    cairo_surface_t* mask = cairo_image_surface_create(
		CAIRO_FORMAT_A8, m_view_width, m_view_height);
	    m_cr_mask = cairo_create(mask);
	    cairo_surface_destroy(mask);

	    // Start drawing to the mask
	    g_cr = m_cr_mask;
	}
	
    void end_submit_mask()
	{	     
	    // Finished with the mask.  Now draw to output
	    g_cr = (g_cr_output ? g_cr_output : m_cr_dummy);
	}
	
    void disable_mask()
	{	       
	    // Clean up any mask
	    if (m_cr_mask)
	    {
		cairo_destroy(m_cr_mask);
		m_cr_mask = 0;

		// Prepare to draw to output
		g_cr = (g_cr_output ? g_cr_output : m_cr_dummy);
	    }
	}
	
 	// Returns the format the current renderer wants videoframes in.
	int videoFrameFormat() {
		return RGB;
	}
	
	/// Draws the video frames
	void drawVideoFrame(image::image_base* baseframe, const matrix* m, const rect* bounds){
	    // Obtain on-stage bounding rectangle
	    gnash::point a, b, c, d;
	    m->transform(&a, gnash::point(bounds->get_x_min(), bounds->get_y_min()));
	    m->transform(&b, gnash::point(bounds->get_x_max(), bounds->get_y_min()));
	    m->transform(&c, gnash::point(bounds->get_x_max(), bounds->get_y_max()));
	    m->transform(&d, gnash::point(bounds->get_x_min(), bounds->get_y_max()));

	    // Extract frame attributes
	    image::rgb* frame = static_cast<image::rgb*>(baseframe);
	    int         w = frame->m_width;
	    int         h = frame->m_height;

	    // Compute bounding rectangle size relative to video object
	    double w_scale = bounds->width()  / w;
	    double h_scale = bounds->height() / h;

	    // Fit video to bounding rectangle
	    cairo_matrix_t mat;
	    cairo_matrix_init_scale(&mat, w_scale, h_scale);
	    cairo_matrix_translate(&mat,
		bounds->get_x_min(), bounds->get_y_min());

	    // Now apply transformation to video
	    cairo_matrix_t frame_mat;
	    cairo_matrix_init(&frame_mat,
		m->m_[0][0], m->m_[1][0],
		m->m_[0][1], m->m_[1][1],
		m->m_[0][2], m->m_[1][2]);
	    cairo_matrix_multiply(&mat, &mat, &frame_mat);

	    // Inverse the matrix for pattern space
	    cairo_matrix_invert(&mat);

	    // Convert RGB frame to cairo format
	    int buf_size = w * h * 4;
	    if (m_video_bufsize < buf_size)
	    {
		if (m_video_buffer)  delete [] m_video_buffer;
		m_video_buffer  = new unsigned char[buf_size];
		m_video_bufsize = buf_size;
	    }
	    rgb_to_cairo_rgb24(m_video_buffer, frame);

	    // Create a pattern from the the RGB frame
	    cairo_surface_t* surface = cairo_image_surface_create_for_data(
		m_video_buffer, CAIRO_FORMAT_RGB24, w, h, w * 4);
	    cairo_pattern_t* pattern = cairo_pattern_create_for_surface(surface);
	    cairo_pattern_set_extend(pattern, CAIRO_EXTEND_NONE);
	    cairo_pattern_set_matrix(pattern, &mat);

	    // Draw the frame now
	    cairo_save(g_cr);
	    cairo_set_source(g_cr, pattern);
	    cairo_move_to(g_cr, a.m_x, a.m_y);
	    cairo_line_to(g_cr, b.m_x, b.m_y);
	    cairo_line_to(g_cr, c.m_x, c.m_y);
	    cairo_line_to(g_cr, d.m_x, d.m_y);
	    cairo_clip(g_cr);
	    cairo_paint(g_cr);
	    cairo_restore(g_cr);

	    // Clean up
	    cairo_pattern_destroy(pattern);
	    cairo_surface_destroy(surface);
	}

};	// end class render_handler_cairo


// bitmap_info_cairo implementation

bitmap_info_cairo::bitmap_info_cairo()
// Make a placeholder bitmap_info.  Must be filled in later before using.
{
    m_buffer = 0;
    m_image = 0;
    m_original_width = 0;
    m_original_height = 0;
}


bitmap_info_cairo::bitmap_info_cairo(int width, int height, uint8_t* data)
// Initialize this bitmap_info to an alpha image
// containing the specified data (1 byte per texel).
{
    assert(width > 0);
    assert(height > 0);
    assert(data);

    // Allocate output buffer
    int buf_size = width * height;
    m_buffer = new unsigned char[buf_size];

    // Copy alpha data
    memcpy(m_buffer, data, buf_size);

    // Create the image
    m_original_width  = width;
    m_original_height = height;
    m_image = cairo_image_surface_create_for_data(
	m_buffer, CAIRO_FORMAT_A8, width, height, width);
    m_pattern = cairo_pattern_create_for_surface(m_image);
}


bitmap_info_cairo::bitmap_info_cairo(image::rgb* im)
// Version of the constructor that takes an RGB image.
{
    assert(im);

    // Convert 24-bit BGR data to 32-bit RGB
    int buf_size = im->m_width * im->m_height * 4;
    m_buffer = new unsigned char[buf_size];
    rgb_to_cairo_rgb24(m_buffer, im);

    // Create the cairo image
    m_original_width  = im->m_width;
    m_original_height = im->m_height;
    m_image = cairo_image_surface_create_for_data(m_buffer,
	CAIRO_FORMAT_RGB24, im->m_width, im->m_height, im->m_width * 4);
    m_pattern = cairo_pattern_create_for_surface(m_image);
}


bitmap_info_cairo::bitmap_info_cairo(image::rgba* im)
// Version of the constructor that takes an image with alpha.
{
    assert(im);

    // Allocate output buffer
    int buf_size = im->m_width * im->m_height * 4;
    m_buffer = new unsigned char[buf_size];
    rgba_to_cairo_argb32(m_buffer, im);

    // Create the image
    m_original_width  = im->m_width;
    m_original_height = im->m_height;
    m_image = cairo_image_surface_create_for_data(m_buffer,
	CAIRO_FORMAT_ARGB32, im->m_width, im->m_height, im->m_width * 4);
    m_pattern = cairo_pattern_create_for_surface(m_image);
}

DSOEXPORT render_handler*
create_handler()
// Factory.
{
	return new render_handler_cairo();
}

DSOEXPORT void
set_handle(cairo_t* handle)
{
    g_cr_output = handle;
}



} // namespace gnash::renderer::cairo
} // namespace gnash::renderer
} // namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
/* vim: set cindent tabstop=8 softtabstop=4 shiftwidth=4: */
