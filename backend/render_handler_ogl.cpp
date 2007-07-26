// render_handler_ogl.cpp	-- Willem Kokke <willem@mindparity.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A render_handler that uses SDL & OpenGL

/* $Id: render_handler_ogl.cpp,v 1.76 2007/07/26 19:25:35 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstring>
#include <cmath>

//#include "gnash.h"
#include "render_handler.h"
#include "render_handler_tri.h"
#include "types.h"
#include "image.h"
#include "utility.h"

#if defined(_WIN32) || defined(WIN32)
#  include <Windows.h>
#endif

#if defined(NOT_SGI_GL) || defined(__APPLE_CC__)
#include <AGL/agl.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#else
# include <GL/gl.h>
# ifdef WIN32
#  define GL_CLAMP_TO_EDGE 0x812F
# else
# include <GL/glx.h>
# endif
# include <GL/glu.h>
# ifndef APIENTRY
#  define APIENTRY
# endif
#endif

using namespace gnash;

// choose the resampling method:
// 1 = hardware (experimental, should be fast, somewhat buggy)
// 2 = fast software bilinear (default)
// 3 = use image::resample(), slow software resampling
#define RESAMPLE_METHOD 2


// bitmap_info_ogl declaration
class bitmap_info_ogl : public gnash::bitmap_info
{
public:
    bitmap_info_ogl();
    bitmap_info_ogl(int width, int height, uint8_t* data);
    bitmap_info_ogl(image::rgb* im);
    bitmap_info_ogl(image::rgba* im);

	~bitmap_info_ogl() {
		if (m_texture_id > 0) {
			glDeleteTextures(1, (GLuint*) &m_texture_id);
		}
	}

	virtual void layout_image(image::image_base* im);
};

// static GLint iquad[] = {-1, 1, 1, 1, 1, -1, -1, -1};

class render_handler_ogl : public gnash::triangulating_render_handler
{
public:

    // Some renderer state.
    
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

    // Utility.  Mutates *width, *height and *data to create the
    // next mip level.
    static void make_next_miplevel(int* width, int* height, uint8_t* data)
	{
	    assert(width);
	    assert(height);
	    assert(data);

	    int	new_w = *width >> 1;
	    int	new_h = *height >> 1;
	    if (new_w < 1) new_w = 1;
	    if (new_h < 1) new_h = 1;
		
	    if (new_w * 2 != *width	 || new_h * 2 != *height) {
		// Image can't be shrunk along (at least) one
		// of its dimensions, so don't bother
		// resampling.	Technically we should, but
		// it's pretty useless at this point.  Just
		// change the image dimensions and leave the
		// existing pixels.
	    } else {
		// Resample.  Simple average 2x2 --> 1, in-place.
		for (int j = 0; j < new_h; j++) {
		    uint8_t*	out = ((uint8_t*) data) + j * new_w;
		    uint8_t*	in = ((uint8_t*) data) + (j << 1) * *width;
		    for (int i = 0; i < new_w; i++) {
			int	a;
			a = (*(in + 0) + *(in + 1) + *(in + 0 + *width) + *(in + 1 + *width));
			*(out) = a >> 2;
			out++;
			in += 2;
		    }
		}
	    }
	    
	    // Munge parameters to reflect the shrunken image.
	    *width = new_w;
	    *height = new_h;
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
	gnash::bitmap_info*	m_bitmap_info;
	gnash::matrix	m_bitmap_matrix;
	gnash::cxform	m_bitmap_color_transform;
	bool	m_has_nonzero_bitmap_additive_color;
		
	fill_style()
	    :
	    m_mode(INVALID),
	    m_has_nonzero_bitmap_additive_color(false)
	    {
	    }

	// Push our style into OpenGL.
	void apply(/*const matrix& current_matrix*/) const
	{
//	    GNASH_REPORT_FUNCTION;
	    assert(m_mode != INVALID);
	    
	    if (m_mode == COLOR) {
		apply_color(m_color);
		glDisable(GL_TEXTURE_2D);
	    } else if (m_mode == BITMAP_WRAP
		       || m_mode == BITMAP_CLAMP) {
		assert(m_bitmap_info != NULL);
		
		apply_color(m_color);
		
		if (m_bitmap_info == NULL) {
		    glDisable(GL_TEXTURE_2D);
		} else {
		    // Set up the texture for rendering.
		    {
			// Do the modulate part of the color
			// transform in the first pass.  The
			// additive part, if any, needs to
			// happen in a second pass.
			glColor4f(m_bitmap_color_transform.m_[0][0],
				  m_bitmap_color_transform.m_[1][0],
				  m_bitmap_color_transform.m_[2][0],
				  m_bitmap_color_transform.m_[3][0]
			    );
		    }
		    
				if (m_bitmap_info->m_texture_id == 0 && m_bitmap_info->m_suspended_image != NULL)
				{
					m_bitmap_info->layout_image(m_bitmap_info->m_suspended_image);
					delete m_bitmap_info->m_suspended_image;
					m_bitmap_info->m_suspended_image = NULL;
				}

				// assert(m_bitmap_info->m_texture_id);

				glBindTexture(GL_TEXTURE_2D, m_bitmap_info->m_texture_id);
		    glEnable(GL_TEXTURE_2D);
		    glEnable(GL_TEXTURE_GEN_S);
		    glEnable(GL_TEXTURE_GEN_T);
		    
		    if (m_mode == BITMAP_CLAMP)	{	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		    } else {
			assert(m_mode == BITMAP_WRAP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		    }
		    
		    // Set up the bitmap matrix for texgen.
		    
		    float	inv_width = 1.0f / m_bitmap_info->m_original_width;
		    float	inv_height = 1.0f / m_bitmap_info->m_original_height;
		    
		    const gnash::matrix&	m = m_bitmap_matrix;
		    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		    float	p[4] = { 0, 0, 0, 0 };
		    p[0] = m.m_[0][0] * inv_width;
		    p[1] = m.m_[0][1] * inv_width;
		    p[3] = m.m_[0][2] * inv_width;
		    glTexGenfv(GL_S, GL_OBJECT_PLANE, p);
		    
		    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		    p[0] = m.m_[1][0] * inv_height;
		    p[1] = m.m_[1][1] * inv_height;
		    p[3] = m.m_[1][2] * inv_height;
		    glTexGenfv(GL_T, GL_OBJECT_PLANE, p);
		}
	    }
	}
	
	
	// Return true if we need to do a second pass to make
	// a valid color.  This is for cxforms with additive
	// parts; this is the simplest way (that we know of)
	// to implement an additive color with stock OpenGL.
	bool	needs_second_pass() const
	    {
		if (m_mode == BITMAP_WRAP
		    || m_mode == BITMAP_CLAMP) {
		    return m_has_nonzero_bitmap_additive_color;
		} else {
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
		//glDisable(GL_TEXTURE_2D);
		
		glColor4f(
		    m_bitmap_color_transform.m_[0][1] / 255.0f,
		    m_bitmap_color_transform.m_[1][1] / 255.0f,
		    m_bitmap_color_transform.m_[2][1] / 255.0f,
		    m_bitmap_color_transform.m_[3][1] / 255.0f
		    );
		
		glBlendFunc(GL_ONE, GL_ONE);
	    }

	void	cleanup_second_pass() const
	    {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	    }


	void	disable() { m_mode = INVALID; }
	void	set_color(const gnash::rgba& color) { m_mode = COLOR; m_color = color; }
	void	set_bitmap(const gnash::bitmap_info* bi, const gnash::matrix& m, bitmap_wrap_mode wm, const gnash::cxform& color_transform)
	    {
		m_mode = (wm == WRAP_REPEAT) ? BITMAP_WRAP : BITMAP_CLAMP;
		m_bitmap_info = const_cast<gnash::bitmap_info*> ( bi );
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
	    return new bitmap_info_ogl(im);
	}


    gnash::bitmap_info*	create_bitmap_info_rgba(image::rgba* im)
	// Given an image, returns a pointer to a bitmap_info class
	// that can later be passed to fill_style_bitmap(), to set a
	// bitmap fill style.
	//
	// This version takes an image with an alpha channel.
	{
	    return new bitmap_info_ogl(im);
	}

    gnash::bitmap_info*	create_bitmap_info_alpha(int w, int h, uint8_t* data)
	// Create a bitmap_info so that it contains an alpha texture
	// with the given data (1 byte per texel).
	//
	// Munges *data (in order to make mipmaps)!!
	{
	    return new bitmap_info_ogl(w, h, data);
	}


    void	delete_bitmap_info(gnash::bitmap_info* bi)
	// Delete the given bitmap info class.
	{
	    delete bi;
	}

#define GLYUV 0

 	// Returns the format the current renderer wants videoframes in.
	int videoFrameFormat() {
#if GLYUV
		return YUV;
#else
		return RGB;
#endif
	}
	
	/// Draws the video frames
	void drawVideoFrame(image::image_base* baseframe, const matrix* m, const rect* bounds){
#if GLYUV
		image::yuv* frame = static_cast<image::yuv*>(baseframe);
#else
		image::rgb* frame = static_cast<image::rgb*>(baseframe);
#endif
		glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);

#if GLYUV
		static GLfloat yuv_rgb[16] = {
			1, 1, 1, 0,
			0, -0.3946517043589703515f, 2.032110091743119266f, 0,
			1.139837398373983740f, -0.5805986066674976801f, 0, 0,
			0, 0, 0, 1
		};
#endif

		glMatrixMode(GL_COLOR);
		glPushMatrix();
#if GLYUV
		glLoadMatrixf(yuv_rgb);
		glPixelTransferf(GL_GREEN_BIAS, -0.5f);
		glPixelTransferf(GL_BLUE_BIAS, -0.5f);
#else
		glLoadIdentity();
		glPixelTransferf(GL_GREEN_BIAS, 0.0);
		glPixelTransferf(GL_BLUE_BIAS, 0.0);
#endif
		gnash::point a, b, c, d;
		m->transform(&a, gnash::point(bounds->get_x_min(), bounds->get_y_min()));
		m->transform(&b, gnash::point(bounds->get_x_max(), bounds->get_y_min()));
		m->transform(&c, gnash::point(bounds->get_x_min(), bounds->get_y_max()));
		d.m_x = b.m_x + c.m_x - a.m_x;
		d.m_y = b.m_y + c.m_y - a.m_y;

		float w_bounds = TWIPS_TO_PIXELS(b.m_x - a.m_x);
		float h_bounds = TWIPS_TO_PIXELS(c.m_y - a.m_y);

		unsigned char*   ptr = frame->m_data;
		float xpos = a.m_x < 0 ? 0.0f : a.m_x;	//hack
		float ypos = a.m_y < 0 ? 0.0f : a.m_y;	//hack
		glRasterPos2f(xpos, ypos);	//hack

#if GLYUV
		GLenum rgb[3] = {GL_RED, GL_GREEN, GL_BLUE}; 

		for (int i = 0; i < 3; ++i)
		{
			float zx = w_bounds / (float) frame->planes[i].w;
			float zy = h_bounds / (float) frame->planes[i].h;
			glPixelZoom(zx, - zy);	// flip & zoom image

			if (i > 0)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_ONE);
			}

			glDrawPixels(frame->planes[i].w, frame->planes[i].h, rgb[i], GL_UNSIGNED_BYTE, ptr);
			ptr += frame->planes[i].size;
		}
#else
		int height = frame->m_height;
		int width = frame->m_width;
		float zx = w_bounds / (float) width;
		float zy = h_bounds / (float) height;
		glPixelZoom(zx,  -zy);	// flip & zoom image
		glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, ptr);
#endif

		glMatrixMode(GL_COLOR);
		glPopMatrix();

		glPopAttrib();
	
	}
   
	// Ctor stub.
	render_handler_ogl()
	{
	}

	// Dtor stub.
	~render_handler_ogl()
	{
	}

    void	begin_display(
	const gnash::rgba& background_color,
	int viewport_x0, int viewport_y0,
	int viewport_width, int viewport_height,
	float x0, float x1, float y0, float y1)
	{
//	    GNASH_REPORT_FUNCTION;
	    
	    m_display_width = fabsf(x1 - x0);
	    m_display_height = fabsf(y1 - y0);

	    glViewport(viewport_x0, viewport_y0, viewport_width, viewport_height);

	    glMatrixMode(GL_MODELVIEW);
	    glPushMatrix();
	    glOrtho(x0, x1, y0, y1, -1, 1);

	    glEnable(GL_BLEND);
	    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	// GL_MODULATE

	    glDisable(GL_TEXTURE_2D);

	    // Clear the background, if background color has alpha > 0.
	    if (background_color.m_a > 0)
		{
		    // Draw a big quad.
		    apply_color(background_color);
		    glBegin(GL_QUADS);
		    glVertex2f(x0, y0);
		    glVertex2f(x1, y0);
		    glVertex2f(x1, y1);
		    glVertex2f(x0, y1);
		    glEnd();
		}

	    // Markus: Implement anti-aliasing here...
#if 0
/*
Code from Timo Kanera...
if (gl_antialias)
  {
    glShadeModel (GL_SMOOTH);
    glEnable (GL_POLYGON_SMOOTH);
    glEnable (GL_LINE_SMOOTH);
    glEnable (GL_POINT_SMOOTH);
    mp_msg(MSGT_VO, MSGL_INFO, "[sgi] antialiasing on\n");
}
else {
    glShadeModel (GL_FLAT);
    glDisable (GL_POLYGON_SMOOTH);
    glDisable (GL_LINE_SMOOTH);
    glDisable (GL_POINT_SMOOTH);
    mp_msg(MSGT_VO, MSGL_INFO, "[sgi] antialiasing off\n");
}
*/
 	    // See if we want to, and can, use multitexture
 	    // antialiasing.
 	    bool s_multitexture_antialias = false;
 	    if (m_enable_antialias)
 		{
 		    int	tex_units = 0;
 		    glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &tex_units);
 		    if (tex_units >= 2)
 			{
 			    s_multitexture_antialias = true;
 			}
 		    // Make sure we have an edge texture available.
 		    if (s_multitexture_antialias == true)
 			{
 			    // Very simple texture: 2 texels wide, 1 texel high.
 			    // Both texels are white; left texel is all clear, right texel is all opaque.
 			    unsigned char	edge_data[8] = { 255, 255, 255, 0, 255, 255, 255, 255 };
			    GLuint s_edge_texture_id = 0;

 			    glActiveTextureARB(GL_TEXTURE1_ARB);
 			    glEnable(GL_TEXTURE_2D);
 			    glGenTextures(1, (GLuint*)&s_edge_texture_id);
 			    glBindTexture(GL_TEXTURE_2D, s_edge_texture_id);

 			    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
 			    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 			    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

 			    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, edge_data);

 			    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	// @@ should we use a 1D texture???

 			    glDisable(GL_TEXTURE_2D);
 			    glActiveTextureARB(GL_TEXTURE0_ARB);
 			    glDisable(GL_TEXTURE_2D);
 			}
 		}
#endif // 0
	}


    void	end_display()
	// Clean up after rendering a frame.  Client program is still
	// responsible for calling glSwapBuffers() or whatever.
	{
//	    GNASH_REPORT_FUNCTION;
	    
	    glMatrixMode(GL_MODELVIEW);
	    glPopMatrix();
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
	// multiply current matrix with opengl matrix
	{
	    float	mat[16];
	    memset(&mat[0], 0, sizeof(mat));
	    mat[0] = m.m_[0][0];
	    mat[1] = m.m_[1][0];
	    mat[4] = m.m_[0][1];
	    mat[5] = m.m_[1][1];
	    mat[10] = 1;
	    mat[12] = m.m_[0][2];
	    mat[13] = m.m_[1][2];
	    mat[15] = 1;
	    glMultMatrixf(mat);
	}

    static void	apply_color(const gnash::rgba& c)
	// Set the given color.
	{
	    glColor4ub(c.m_r, c.m_g, c.m_b, c.m_a);
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
		if ( width == 1.0 ) // "hairline", see render_handler_tri.h
		{
			glLineWidth(1); // expected: 1 pixel
		}
		else
		{
			// TODO: OpenGL doesn't seem to handle very
			// low-width lines well, even with anti-aliasing
			// enabled
			// But this is a start (20 TWIPS' width = 1 pixel's)
			glLineWidth(TWIPS_TO_PIXELS(width));
		}
	}


    void	draw_mesh_strip(const void* coords, int vertex_count)
	{
//	    GNASH_REPORT_FUNCTION;
	    
#define NORMAL_RENDERING
//#define MULTIPASS_ANTIALIASING

#ifdef NORMAL_RENDERING
	    // Set up current style.
	    m_current_styles[LEFT_STYLE].apply();

	    glMatrixMode(GL_MODELVIEW);
	    glPushMatrix();
	    apply_matrix(m_current_matrix);

	    // Send the tris to OpenGL
	    glEnableClientState(GL_VERTEX_ARRAY);
	    glVertexPointer(2, GL_SHORT, sizeof(int16_t) * 2, coords);
	    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_count);

	    if (m_current_styles[LEFT_STYLE].needs_second_pass())
		{
		    m_current_styles[LEFT_STYLE].apply_second_pass();
		    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_count);
		    m_current_styles[LEFT_STYLE].cleanup_second_pass();
		}

	    glDisableClientState(GL_VERTEX_ARRAY);

	    glPopMatrix();
#endif // NORMAL_RENDERING

#ifdef MULTIPASS_ANTIALIASING
	    // So this approach basically works.  This
	    // implementation is not totally finished; two pass
	    // materials (i.e. w/ additive color) aren't correct,
	    // and there are some texture etc issues because I'm
	    // just hosing state uncarefully here.  It needs the
	    // optimization of only filling the bounding box of
	    // the shape.  You must have destination alpha.
	    //
	    // It doesn't look quite perfect on my GF4.  For one
	    // thing, you kinda want to crank down the max curve
	    // subdivision error, because suddenly you can see
	    // sub-pixel shape much better.  For another thing,
	    // the antialiasing isn't quite perfect, to my eye.
	    // It could be limited alpha precision, imperfections
	    // GL_POLYGON_SMOOTH, and/or my imagination.

	    glDisable(GL_TEXTURE_2D);

	    glEnable(GL_POLYGON_SMOOTH);
	    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);	// GL_NICEST, GL_FASTEST, GL_DONT_CARE

	    // Clear destination alpha.
	    //
	    // @@ TODO Instead of drawing this huge screen-filling
	    // quad, we should take a bounding-box param from the
	    // caller, and draw the box (after apply_matrix;
	    // i.e. the box is in object space).  The point being,
	    // to only fill the part of the screen that the shape
	    // is in.
	    glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	    glColor4f(1, 1, 1, 0);
	    glBegin(GL_QUADS);
	    glVertex2f(0, 0);
	    glVertex2f(100000, 0);
	    glVertex2f(100000, 100000);
	    glVertex2f(0, 100000);
	    glEnd();

	    // Set mode for drawing alpha mask.
	    glBlendFunc(GL_ONE, GL_ONE);	// additive blending
	    glColor4f(0, 0, 0, m_current_styles[LEFT_STYLE].m_color.m_a / 255.0f);

	    glMatrixMode(GL_MODELVIEW);
	    glPushMatrix();
	    apply_matrix(m_current_matrix);

	    // Send the tris to OpenGL.  This produces an
	    // antialiased alpha mask of the mesh shape, in the
	    // destination alpha channel.
	    glEnableClientState(GL_VERTEX_ARRAY);
	    glVertexPointer(2, GL_SHORT, sizeof(int16_t) * 2, coords);
	    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_count);
	    glDisableClientState(GL_VERTEX_ARRAY);

	    glPopMatrix();
		
	    // Set up desired fill style.
	    m_current_styles[LEFT_STYLE].apply();

	    // Apply fill, modulated with alpha mask.
	    //
	    // @@ TODO see note above about filling bounding box only.
	    glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
	    glBegin(GL_QUADS);
	    glVertex2f(0, 0);
	    glVertex2f(100000, 0);
	    glVertex2f(100000, 100000);
	    glVertex2f(0, 100000);
	    glEnd();

// xxxxx ??? Hm, is our mask still intact, or did we just erase it?
// 		if (m_current_styles[LEFT_STYLE].needs_second_pass())
// 		{
// 			m_current_styles[LEFT_STYLE].apply_second_pass();
// 			glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_count);
// 			m_current_styles[LEFT_STYLE].cleanup_second_pass();
// 		}

	    // @@ hm, there is perhaps more state that needs
	    // fixing here, or setting elsewhere.
	    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif // MULTIPASS_ANTIALIASING
	}


    void	draw_line_strip(const void* coords, int vertex_count)
	// Draw the line strip formed by the sequence of points.
	{
//	    GNASH_REPORT_FUNCTION;
	    // Set up current style.
	    m_current_styles[LINE_STYLE].apply();

	    glMatrixMode(GL_MODELVIEW);
	    glPushMatrix();
	    apply_matrix(m_current_matrix);

	    // Send the line-strip to OpenGL
	    glEnableClientState(GL_VERTEX_ARRAY);
	    glVertexPointer(2, GL_SHORT, sizeof(int16_t) * 2, coords);
	    glDrawArrays(GL_LINE_STRIP, 0, vertex_count);
	    glDisableClientState(GL_VERTEX_ARRAY);

	    glPopMatrix();
	}


    void	draw_bitmap(
	const gnash::matrix& m,
	const gnash::bitmap_info* bi,
	const gnash::rect& coords,
	const gnash::rect& uv_coords,
	const gnash::rgba& color)
	// Draw a rectangle textured with the given bitmap, with the
	// given color.	 Apply given transform; ignore any currently
	// set transforms.
	//
	// Intended for textured glyph rendering.
	{
//	    GNASH_REPORT_FUNCTION;
	    assert(bi);

	    apply_color(color);

	    gnash::point a, b, c, d;
	    m.transform(&a, gnash::point(coords.get_x_min(), coords.get_y_min()));
	    m.transform(&b, gnash::point(coords.get_x_max(), coords.get_y_min()));
	    m.transform(&c, gnash::point(coords.get_x_min(), coords.get_y_max()));
	    d.m_x = b.m_x + c.m_x - a.m_x;
	    d.m_y = b.m_y + c.m_y - a.m_y;

			if (bi->m_texture_id == 0 && bi->m_suspended_image != NULL)
			{
				const_cast<bitmap_info*>(bi)->layout_image(bi->m_suspended_image);
				delete bi->m_suspended_image;
				const_cast<bitmap_info*>(bi)->m_suspended_image = NULL;
			}

			// assert(bi->m_texture_id);

			glBindTexture(GL_TEXTURE_2D, bi->m_texture_id);
	    glEnable(GL_TEXTURE_2D);
	    glDisable(GL_TEXTURE_GEN_S);
	    glDisable(GL_TEXTURE_GEN_T);

	    glBegin(GL_TRIANGLE_STRIP);

            float xmin = uv_coords.get_x_min();
            float xmax = uv_coords.get_x_max();
            float ymin = uv_coords.get_y_min();
            float ymax = uv_coords.get_y_max();

	    glTexCoord2f(xmin, ymin);
	    glVertex2f(a.m_x, a.m_y);

	    glTexCoord2f(xmax, ymin);
	    glVertex2f(b.m_x, b.m_y);

	    glTexCoord2f(xmin, ymax);
	    glVertex2f(c.m_x, c.m_y);

	    glTexCoord2f(xmax, ymax);
	    glVertex2f(d.m_x, d.m_y);

	    glEnd();
	}
	
    void begin_submit_mask()
	{
//	    GNASH_REPORT_FUNCTION;
	    glEnable(GL_STENCIL_TEST); 
	    glClearStencil(0);
	    glClear(GL_STENCIL_BUFFER_BIT);
	    glColorMask(0,0,0,0);	// disable framebuffer writes
	    glEnable(GL_STENCIL_TEST);	// enable stencil buffer for "marking" the mask
	    glStencilFunc(GL_ALWAYS, 1, 1);	// always passes, 1 bit plane, 1 as mask
	    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);	// we set the stencil buffer to 1 where we draw any polygon
							// keep if test fails, keep if test passes but buffer test fails
							// replace if test passes 
	}
	
    void end_submit_mask()
	{	     
	    glColorMask(1,1,1,1);	// enable framebuffer writes
	    glStencilFunc(GL_EQUAL, 1, 1);	// we draw only where the stencil is 1 (where the mask was drawn)
	    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);	// don't change the stencil buffer    
	}
	
    void disable_mask()
	{	       
	    glDisable(GL_STENCIL_TEST); 
	}
	
};	// end class render_handler_ogl


// bitmap_info_ogl implementation

/*

Markus: A. A. I still miss you and the easter 2006, you know.
	A. J. I miss you too, but you'll probably not read this code ever... :/

*/

void	hardware_resample(int bytes_per_pixel, int src_width, int src_height, uint8_t* src_data, int dst_width, int dst_height)
// Code from Alex Streit
//
// Sets the current texture to a resampled/expanded version of the
// given image data.
{
//    GNASH_REPORT_FUNCTION;
    assert(bytes_per_pixel == 3 || bytes_per_pixel == 4);

    unsigned int	in_format = bytes_per_pixel == 3 ? GL_RGB : GL_RGBA;
    unsigned int	out_format = bytes_per_pixel == 3 ? GL_RGB : GL_RGBA;

    // alex: use the hardware to resample the image
    // issue: does not work when image > allocated window size!
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glPushAttrib(GL_TEXTURE_BIT | GL_ENABLE_BIT);
    {
	char* temp = new char[dst_width * dst_height * bytes_per_pixel];
	//memset(temp,255,w*h*3);
	glTexImage2D(GL_TEXTURE_2D, 0, in_format, dst_width, dst_height, 0, out_format, GL_UNSIGNED_BYTE, temp);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, src_width, src_height, out_format, GL_UNSIGNED_BYTE, src_data);

	glLoadIdentity();
	glViewport(0, 0, dst_width, dst_height);
	glOrtho(0, dst_width, 0, dst_height, 0.9, 1.1);
	glColor3f(1, 1, 1);
	glNormal3f(0, 0, 1);
	glBegin(GL_QUADS);
	{
	    glTexCoord2f(0, (float) src_height / dst_height);
	    glVertex3f(0, 0, -1);
	    glTexCoord2f( (float) src_width / dst_width, (float) src_height / dst_height);
	    glVertex3f((float) dst_width, 0, -1);
	    glTexCoord2f( (float) src_width / dst_width, 0);
	    glVertex3f((float) dst_width, (float) dst_height, -1);
	    glTexCoord2f(0, 0);
	    glVertex3f(0, (float) dst_height, -1);
	}
	glEnd();
	glCopyTexImage2D(GL_TEXTURE_2D, 0, out_format, 0,0, dst_width, dst_height, 0);
	delete[] temp;
    }
    glPopAttrib();
    glPopMatrix();
    glPopMatrix();
}



void	software_resample(
    int bytes_per_pixel,
    int src_width,
    int src_height,
    int src_pitch,
    uint8_t* src_data,
    int dst_width,
    int dst_height)
// Code from Alex Streit
//
// Creates an OpenGL texture of the specified dst dimensions, from a
// resampled version of the given src image.  Does a bilinear
// resampling to create the dst image.
{
//    GNASH_REPORT_FUNCTION;
    assert(bytes_per_pixel == 3 || bytes_per_pixel == 4);

    assert(dst_width >= src_width);
    assert(dst_height >= src_height);

    unsigned int	internal_format = bytes_per_pixel == 3 ? GL_RGB : GL_RGBA;
    unsigned int	input_format = bytes_per_pixel == 3 ? GL_RGB : GL_RGBA;

    // FAST bi-linear filtering
    // the code here is designed to be fast, not readable
    uint8_t* rescaled = new uint8_t[dst_width * dst_height * bytes_per_pixel];
    float Uf, Vf;		// fractional parts
    float Ui, Vi;		// integral parts
    float w1, w2, w3, w4;	// weighting
    uint8_t* psrc;
    uint8_t* pdst = rescaled;
    // i1,i2,i3,i4 are the offsets of the surrounding 4 pixels
    const int i1 = 0;
    const int i2 = bytes_per_pixel;
    int i3 = src_pitch;
    int i4 = src_pitch + bytes_per_pixel;
    // change in source u and v
    float dv = (float)(src_height-2) / dst_height;
    float du = (float)(src_width-2) / dst_width;
    // source u and source v
    float U;
    float V=0;

#define BYTE_SAMPLE(offset)	\
	(uint8_t) (w1 * psrc[i1 + (offset)] + w2 * psrc[i2 + (offset)] + w3 * psrc[i3 + (offset)] + w4 * psrc[i4 + (offset)])

    if (bytes_per_pixel == 3)
	{
	    for (int v = 0; v < dst_height; ++v)
		{
		    Vf = modff(V, &Vi);
		    V+=dv;
		    U=0;

		    for (int u = 0; u < dst_width; ++u)
			{
			    Uf = modff(U, &Ui);
			    U+=du;

			    w1 = (1 - Uf) * (1 - Vf);
			    w2 = Uf * (1 - Vf);
			    w3 = (1 - Uf) * Vf;
			    w4 = Uf * Vf;
			    psrc = &src_data[(int) (Vi * src_pitch) + (int) (Ui * bytes_per_pixel)];

			    *pdst++ = BYTE_SAMPLE(0);	// red
			    *pdst++ = BYTE_SAMPLE(1);	// green
			    *pdst++ = BYTE_SAMPLE(2);	// blue

			    psrc += 3;
			}
		}
	}
    else
	{
	    assert(bytes_per_pixel == 4);

	    for (int v = 0; v < dst_height; ++v)
		{
		    Vf = modff(V, &Vi);
		    V+=dv;
		    U=0;

		    for (int u = 0; u < dst_width; ++u)
			{
			    Uf = modff(U, &Ui);
			    U+=du;

			    w1 = (1 - Uf) * (1 - Vf);
			    w2 = Uf * (1 - Vf);
			    w3 = (1 - Uf) * Vf;
			    w4 = Uf * Vf;
			    psrc = &src_data[(int) (Vi * src_pitch) + (int) (Ui * bytes_per_pixel)];

			    *pdst++ = BYTE_SAMPLE(0);	// red
			    *pdst++ = BYTE_SAMPLE(1);	// green
			    *pdst++ = BYTE_SAMPLE(2);	// blue
			    *pdst++ = BYTE_SAMPLE(3);	// alpha

			    psrc += 4;
			}
		}
	}

    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, dst_width, dst_height, 0, input_format, GL_UNSIGNED_BYTE, rescaled);

    delete [] rescaled;
}


bitmap_info_ogl::bitmap_info_ogl()
// Make a placeholder bitmap_info.  Must be filled in later before
// using.
{
//    GNASH_REPORT_FUNCTION;
    m_texture_id = 0;
    m_original_width = 0;
    m_original_height = 0;
}

void bitmap_info_ogl::layout_image(image::image_base* im)
{

	assert(im);

	// Create the texture.
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, (GLuint*)&m_texture_id);
	glBindTexture(GL_TEXTURE_2D, m_texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	m_original_width = im->m_width;
	m_original_height = im->m_height;

	switch (im->m_type)
	{
		case image::image_base::RGB:
		{

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			int	w = 1; while (w < im->m_width) { w <<= 1; }
			int	h = 1; while (h < im->m_height) { h <<= 1; }

			if (w != im->m_width	|| h != im->m_height)
			{
#if (RESAMPLE_METHOD == 1)
				int	viewport_dim[2] = { 0, 0 };
				glGetIntegerv(GL_MAX_VIEWPORT_DIMS, &viewport_dim[0]);
				if (w > viewport_dim[0]
			|| h > viewport_dim[1]
			|| im->m_width * 3 != im->m_pitch)
			{
					// Can't use hardware resample.  Either frame
					// buffer isn't big enough to fit the source
					// texture, or the source data isn't padded
					// quite right.
					software_resample(3, im->m_width, im->m_height, im->m_pitch, im->m_data, w, h);
			}
				else
			{
					hardware_resample(3, im->m_width, im->m_height, im->m_data, w, h);
			}
#elif (RESAMPLE_METHOD == 2)
				{
			// Faster/simpler software bilinear rescale.
			software_resample(3, im->m_width, im->m_height, im->m_pitch, im->m_data, w, h);
				}
#else
				{
			// Fancy but slow software resampling.
			image::rgb*	rescaled = image::create_rgb(w, h);
			image::resample(rescaled, 0, 0, w - 1, h - 1,
					im, 0, 0, (float) im->m_width, (float) im->m_height);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, rescaled->m_data);

			delete rescaled;
				}
#endif
			}
			else
			{
				// Use original image directly.
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, im->m_data);
			}

			break;
		}
		case image::image_base::RGBA:
		{

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			int	w = 1; while (w < im->m_width) { w <<= 1; }
			int	h = 1; while (h < im->m_height) { h <<= 1; }

			if (w != im->m_width	|| h != im->m_height)
			{
#if (RESAMPLE_METHOD == 1)
				int	viewport_dim[2] = { 0, 0 };
				glGetIntegerv(GL_MAX_VIEWPORT_DIMS, &viewport_dim[0]);
				if (w > viewport_dim[0]
			|| h > viewport_dim[1]
			|| im->m_width * 4 != im->m_pitch)
			{
					// Can't use hardware resample.  Either frame
					// buffer isn't big enough to fit the source
					// texture, or the source data isn't padded
					// quite right.
					software_resample(4, im->m_width, im->m_height, im->m_pitch, im->m_data, w, h);
			}
				else
			{
					hardware_resample(4, im->m_width, im->m_height, im->m_data, w, h);
			}
#elif (RESAMPLE_METHOD == 2)
				{
			// Faster/simpler software bilinear rescale.
			software_resample(4, im->m_width, im->m_height, im->m_pitch, im->m_data, w, h);
				}
#else
				{
			// Fancy but slow software resampling.
			image::rgba*	rescaled = image::create_rgba(w, h);
			image::resample(rescaled, 0, 0, w - 1, h - 1,
					im, 0, 0, (float) im->m_width, (float) im->m_height);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rescaled->m_data);

			delete rescaled;
				}
#endif
			}
			else
			{
				// Use original image directly.
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, im->m_data);
			}

			break;
		}
		case image::image_base::ROW:
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

#ifndef NDEBUG
			// You must use power-of-two dimensions!!
			int	w = 1; while (w < im->m_width) { w <<= 1; }
			int	h = 1; while (h < im->m_height) { h <<= 1; }
			assert(w == im->m_width);
			assert(h == im->m_height);
#endif // not NDEBUG

			glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA,
				im->m_width, im->m_height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, im->m_data);

			// Build mips.
			int	level = 1;
			while (im->m_width > 1 || im->m_height > 1)
			{
				render_handler_ogl::make_next_miplevel(&im->m_width, &im->m_height, im->m_data);
				glTexImage2D(GL_TEXTURE_2D, level, GL_ALPHA, 
					im->m_width, im->m_height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, im->m_data);
				level++;
			}

			break;
		}

		default:
//			printf("unsupported image type\n");
			break;
	}

}
inline bool opengl_accessible()
{
#if defined(_WIN32) || defined(WIN32)
	return wglGetCurrentContext() != 0;
#elif defined(__APPLE_CC__)
	return aglGetCurrentContext() != 0;
#else
	return glXGetCurrentContext() != 0;
#endif
}

bitmap_info_ogl::bitmap_info_ogl(int width, int height, uint8_t* data)
// Initialize this bitmap_info to an alpha image
// containing the specified data (1 byte per texel).
//
// !! Munges *data in order to create mipmaps !!
{
//    GNASH_REPORT_FUNCTION;
	assert(width > 0);
	assert(height > 0);
	assert(data);

	// TODO optimization
	image::image_base* im = new image::image_base(data, width, height, 1, image::image_base::ROW);
	memcpy(im->m_data, data, width * height);

	if (opengl_accessible() == false) 
	{
		m_suspended_image = im;
		return;
	}
	layout_image(im);
	delete im;
}


bitmap_info_ogl::bitmap_info_ogl(image::rgb* im)
// NOTE: This function destroys im's data in the process of making mipmaps.
{
//    GNASH_REPORT_FUNCTION;
	assert(im);

	if (opengl_accessible() == false) 
	{
		m_suspended_image = image::create_rgb(im->m_width, im->m_height);
		memcpy(m_suspended_image->m_data, im->m_data, im->m_pitch * im->m_height);
		return;
	}
	layout_image(im);
}


bitmap_info_ogl::bitmap_info_ogl(image::rgba* im)
// Version of the constructor that takes an image with alpha.
// NOTE: This function destroys im's data in the process of making mipmaps.
{
//    GNASH_REPORT_FUNCTION;
	assert(im);

	if (opengl_accessible() == false) 
	{
		m_suspended_image = image::create_rgba(im->m_width, im->m_height);
		memcpy(m_suspended_image->m_data, im->m_data, im->m_pitch * im->m_height);
		return;
	}
	layout_image(im);
}


gnash::render_handler*	gnash::create_render_handler_ogl()
// Factory.
{
//    GNASH_REPORT_FUNCTION;

    // Do some initialisation.
#define OVERSIZE	1.0f
    
    //This makes fonts look nice (actually!)
#if 0
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
#endif    
        // Turn on alpha blending.
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
;        
        // Turn on line smoothing.  Antialiased lines can be used to
        // smooth the outsides of shapes.
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);	// GL_NICEST, GL_FASTEST, GL_DONT_CARE
        
        glMatrixMode(GL_PROJECTION);
        glOrtho(-OVERSIZE, OVERSIZE, OVERSIZE, -OVERSIZE, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        // We don't need lighting effects
        glDisable(GL_LIGHTING);
        // glColorPointer(4, GL_UNSIGNED_BYTE, 0, *);
        // glInterleavedArrays(GL_T2F_N3F_V3F, 0, *)
        glPushAttrib (GL_ALL_ATTRIB_BITS);


#ifdef FIX_I810_LOD_BIAS
	// If 2D textures weren't previously enabled, enable
	// them now and force the driver to notice the update,
	// then disable them again.
	if (!glIsEnabled(GL_TEXTURE_2D)) {
	  // Clearing a mask of zero *should* have no
	  // side effects, but coupled with enbling
	  // GL_TEXTURE_2D it works around a segmentation
	  // fault in the driver for the Intel 810 chip.
	  glEnable(GL_TEXTURE_2D);
	  glClear(0);
	  glDisable(GL_TEXTURE_2D);
	}
#endif

    return new render_handler_ogl;
}


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
