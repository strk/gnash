// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//

/// \mainpage
///
///  See Related Pages for movies and sprites informations
///

#ifndef GNASH_H
#define GNASH_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <ctype.h>	// for poxy wchar_t
#include <stdarg.h>	// for va_list arg to movie_interface::call_method_args()

#include "ref_counted.h" // for bitmap_info inheritance

#include "as_value.h" // for register_component(...)

// FIXME: The local usage of these constants should probably be renamed in this
// file because they conflict with definitions in the system header files. Luckily
// they are used in files we don't need, so we should be able to safely redefine
// these here.
#undef INVALID
#undef ESCAPE

class tu_file;
class render_handler;
class weak_proxy;	// forward decl; defined in base/smart_ptr.h

// @@ forward decl to avoid including base/image.h; TODO change the
// render_handler interface to not depend on these structs at all.
namespace image { struct rgb; struct rgba; }

// forward decl
namespace jpeg { struct input; }
class tu_string;
class tu_stringi;

namespace gnash {
// Forward declarations.
struct action_buffer;
struct as_value;
struct bitmap_info;
class character;
struct execute_tag;
class font;
struct movie;
struct movie_interface;
struct movie_definition;
struct render_handler;
struct resource;
struct rgba;
struct sound_handler;
struct stream;
class URL;

///
/// Log & error reporting control.
///

/// Supply a function pointer to receive log & error messages.
void	register_log_callback(void (*callback)(bool error, const char* message));

/// Control verbosity of action processing
void	set_verbose_action(bool verbose);

/// Control verbosity of SWF parsing
void	set_verbose_parse(bool verbose);

/// Set the render handler.  This is one of the first
/// things you should do to initialise the player (assuming you
/// want to display anything).
void    set_render_handler(render_handler* s);

/// Pass in a sound handler, so you can handle audio on behalf of
/// gnash.  This is optional; if you don't set a handler, or set
/// NULL, then sounds won't be played.
///
/// If you want sound support, you should set this at startup,
/// before loading or playing any movies!
void	set_sound_handler(sound_handler* s);

/// You probably don't need this. (@@ make it private?)
sound_handler*	get_sound_handler();

/// Signature of file opener callback function
typedef tu_file* (*file_opener_callback)(const URL& url);

/// Signature of progress callback function
typedef void (*progress_callback)(unsigned int loaded_bytes, unsigned int total_bytes);

/// Register a callback for displaying SWF load progress.
void	register_progress_callback(progress_callback progress_handle);

/// Signature of fscommand callback function
typedef void (*fscommand_callback)(movie_interface* movie, const char* command, const char* arg);

/// ActionScripts embedded in a movie can use the built-in
/// fscommand() function to send data back to the host
/// application.  If you are interested in this data, register
/// a handler, which will be called when the embedded scripts
/// call fscommand().
///
/// The handler gets the movie_interface* that the script is
/// embedded in, and the two string arguments passed by the
/// script to fscommand().
void	register_fscommand_callback(fscommand_callback handler);

/// Use this to register gnash extension
void register_component(const tu_stringi& name, as_c_function_ptr handler);

/// Use this to control how finely curves are subdivided.  1.0
/// is the default; it's a pretty good value.  Larger values
/// result in coarser, more angular curves with fewer vertices.
void	set_curve_max_pixel_error(float pixel_error);
float	get_curve_max_pixel_error();

// Some helpers that may or may not be compiled into your
// version of the library, depending on platform etc.
render_handler*	create_render_handler_xbox();
render_handler*	create_render_handler_ogl();
render_handler*	create_render_handler_cairo(void* cairohandle);

sound_handler*	create_sound_handler_sdl();

class font;
struct character_def;
struct sound_sample;

/// For caching precomputed stuff.  Generally of
/// interest to gnash_processor and programs like it.
struct cache_options
{
	bool	m_include_font_bitmaps;
	
	cache_options()
		:
		m_include_font_bitmaps(true)
		{
		}
};


/// Try to grab movie info from the header of the given .swf file.
//
/// Sets *version to 0 if info can't be extracted.
///
/// You can pass NULL for any entries you're not interested in.
/// In particular, using a NULL tag_count will avoid scanning
/// the whole movie.
///
/// FIXME: use a stream here, so we can use an already opened one.
///
void	get_movie_info(
	const URL&	url,
	int*		version,
	int*		width,
	int*		height,
	float*		frames_per_second,
	int*		frame_count,
	int*		tag_count
	);

/// Enable/disable attempts to read cache files (.gsc) when loading movies.
void	set_use_cache_files(bool use_cache);
	
/// Create a gnash::movie_definition from the given URL.
//
/// The URL can correspond to either a JPEG or SWF file.
///
/// Normally, will also try to load any cached data file
/// (".gsc") that corresponds to the given movie file.  This
/// will still work even if there is no cache file.  You can
/// disable the attempts to load cache files by calling
/// gnash::use_cache_files(false).
///
/// Uses the global StreamProvider 'streamProvider' 
/// to read the files themselves.
///
/// This calls add_ref() on the newly created definition; call
/// drop_ref() when you're done with it.
/// Or use smart_ptr<T> from base/smart_ptr.h if you want.
///
/// @@ Hm, need to think about these creation API's.  Perhaps
/// divide it into "low level" and "high level" calls.  Also,
/// perhaps we need a "context" object that contains all
/// global-ish flags, libraries, callback pointers, font
/// library, etc.
///
movie_definition*	create_movie(const URL& url);

/// Creates the movie from the given input stream. 
//
/// Only reads from the given stream; does not open files. 
/// If the movie imports resources from other movies, the created movie
/// inserts proxy stubs in place of those resources.  The list
/// of imported movie filenames can be retrieved with
/// movie_definition::visit_imported_movies().  The proxies can
/// be replaced with actual movie_definition's via
/// movie_definition::resolve_proxy(name,def).
///
/// Use DO_NOT_LOAD_BITMAPS if you have pre-processed bitmaps
/// stored externally somewhere, and you plan to install them
/// via get_bitmap_info()->...
enum create_bitmaps_flag
{
	DO_LOAD_BITMAPS,
	DO_NOT_LOAD_BITMAPS
};

/// Use DO_NOT_LOAD_FONT_SHAPES if you know you have
/// precomputed texture glyphs (in cached data) and you know
/// you always want to render text using texture glyphs.
enum create_font_shapes_flag
{
	DO_LOAD_FONT_SHAPES,
	DO_NOT_LOAD_FONT_SHAPES
};

//movie_definition*	create_movie_no_recurse(
//	tu_file*		input_stream,
//	create_bitmaps_flag	cbf,
//	create_font_shapes_flag cfs);

/// \brief
/// Create a gnash::movie_definition from the given URL
//
/// The URL can correspond to either a JPEG or SWF file.
///
/// This is just like create_movie(), except that it checks the
/// "library" to see if a movie of this name has already been
/// created, and returns that movie if so.  Also, if it creates
/// a new movie, it adds it back into the library.
///
/// The "library" is used when importing symbols from external
/// movies, so this call might be useful if you want to
/// explicitly load a movie that you know exports symbols
/// (e.g. fonts) to other movies as well.
///
/// @@ this explanation/functionality could be clearer!
///
/// This calls add_ref() on the newly created definition; call
/// drop_ref() when you're done with it.
/// Or use smart_ptr<T> from base/smart_ptr.h if you want.
///
///
movie_definition* create_library_movie(const URL& url);
	

/// Helper to pregenerate cached data (basically, shape tesselations). 
//
/// Does this by running through each frame of
/// the movie and displaying the shapes with a null renderer.
/// The pregenerated data is stored in the movie_definition
/// object itself, and is included with the cached data written
/// by movie_definition::output_cached_data().
///
/// Note that this tesselates shapes to the resolution they
/// explicitly appear in the linear frames of the movie.  Does
/// not try very hard to run your ActionScript to account for
/// dynamic scaling (that's more or less futile anyway due to
/// the halting problem).
void	precompute_cached_data(movie_definition* movie_def);

/// Maximum release of resources. 
//
/// Calls clear_library() and
/// fontlib::clear(), and also clears some extra internal stuff
/// that may have been allocated (e.g. global ActionScript
/// objects).  This should get all gnash structures off the
/// heap, with the exception of any objects that are still
/// referenced by the host program and haven't had drop_ref()
/// called on them.
void	clear();

//
// Library management
//
	
/// Release any library movies we've cached. 
//
/// Do this when you want maximum cleanup.
void	clear_library();
	
//
// Font library control.  gnash is able to substitute fonts
// from the font library, in case a movie lacks glyphs for a
// declared font.  This would come into play since in recent
// versions of SWF, the movie is allowed to use "system
// fonts".  E.g. it can declare a font named "Arial", but not
// provide glyphs for it, and then the OS is expected to
// provide the font or a suitable replacement.
//
// gnash does not try to handle this automatically; if your
// host program wants to emulate this behavior, it needs to
// load a movie that includes glyph info for the standard
// fonts you want, and then explicitly pull those fonts out of
// the movie_def and add them to fontlib.
//
// @@ TODO: not all public APIs to enable this are in place
// yet!  Need md::get_font_count()/get_font(), and
// fontlib::add_font().
//
// Otherwise, text written in a font with no glyphs just
// doesn't render at all.  (@@ Hm, should probably render it
// as boxes or something?)

class font;	
namespace fontlib
{
// Controls how large to render textured glyphs.
// Applies to fonts processed *after* this call only.
// The "nominal" size is perhaps around twice the
// average glyph height.
void	set_nominal_glyph_pixel_size(int pixel_size);

// For accessing the fonts in the library.
void	clear();
int	get_font_count();
font*	get_font(int index);
font*	get_font(const char* name);
const char*	get_font_name(const font* f);

// @@ also need to add color controls (or just set the diffuse color
// in the API?), perhaps matrix xform, and maybe spacing, etc.
//
// // For direct text rendering from the host app.
void	draw_string(const font* f, float x, float y, float size, const char* text);
// void	draw_string(const font* f, float x, float y, float size, const wchar_t* text);	// wide-char version
}
	
	
//
// Sound callback handler.
//
	
// You may define a subclass of this, and pass an instance to
// set_sound_handler().
struct sound_handler
{
	enum format_type
	{
		FORMAT_RAW = 0,		// unspecified format.  Useful for 8-bit sounds???
		FORMAT_ADPCM = 1,	// gnash doesn't pass this through; it uncompresses and sends FORMAT_NATIVE16
		FORMAT_MP3 = 2,
		FORMAT_UNCOMPRESSED = 3,	// 16 bits/sample, little-endian
		FORMAT_NELLYMOSER = 6,	// Mystery proprietary format; see nellymoser.com
				
		// gnash tries to convert data to this format when possible:
		FORMAT_NATIVE16 = 7	// gnash extension: 16 bits/sample, native-endian
	};
	// If stereo is true, samples are interleaved w/ left sample first.
	
	// gnash calls at load-time with sound data, to be
	// played later.  You should create a sample with the
	// data, and return a handle that can be used to play
	// it later.  If the data is in a format you can't
	// deal with, then you can return 0 (for example), and
	// then ignore 0's in play_sound() and delete_sound().
	//
	// Assign handles however you like.
	virtual int	create_sound(
		void*		data,
		int		data_bytes,
		int		sample_count,
		format_type	format,
		int		sample_rate,	/* one of 5512, 11025, 22050, 44100 */
		bool		stereo
		) = 0;
		
	// gnash calls this when it wants you to play the defined sound.
	//
	// loop_count == 0 means play the sound once (1 means play it twice, etc)
	virtual void	play_sound(int sound_handle, int loop_count, int secondOffset) = 0;

	//	stops all sounds currently playing in a SWF file without stopping the playhead.
	//	Sounds set to stream will resume playing as the playhead moves over the frames they are in.
	virtual void	stop_all_sounds() = 0;

	//	returns the sound volume level as an integer from 0 to 100,
	//	where 0 is off and 100 is full volume. The default setting is 100.
	virtual int	get_volume(int sound_handle) = 0;
	
	//	A number from 0 to 100 representing a volume level. 
	//	100 is full volume and 0 is no volume. The default setting is 100.
	virtual void	set_volume(int sound_handle, int volume) = 0;
		
	// Stop the specified sound if it's playing.
	// (Normally a full-featured sound API would take a
	// handle specifying the *instance* of a playing
	// sample, but SWF is not expressive that way.)
	virtual void	stop_sound(int sound_handle) = 0;
		
	// gnash calls this when it's done with a particular sound.
	virtual void	delete_sound(int sound_handle) = 0;
		
	virtual ~sound_handler() {};
};
	

//
// matrix type, used by render handler
//

struct point;

/// Matrix type, used by render handler
struct matrix
{
	/// [x,y][scale,rotate,translate]
	float	m_[2][3];
	
	/// The identity matrix (no transforms)
	static matrix	identity;
	
	/// Defaults to identity
	matrix();

	/// Check validity of the matrix values
	bool	is_valid() const;

	/// Set the matrix to identity.
	void	set_identity();

	/// Concatenate m's transform onto ours. 
	//
	/// When transforming points, m happens first,
	/// then our original xform.
	void	concatenate(const matrix& m);

	/// Concatenate a translation onto the front of our matrix.
	//
	/// When transforming points, the translation
	/// happens first, then our original xform.
	///
	void	concatenate_translation(float tx, float ty);

	/// Concatenate a uniform scale onto the front of our matrix.
	//
	/// When transforming points, the scale
	/// happens first, then our original xform.
	///
	void	concatenate_scale(float s);

	/// Set this matrix to a blend of m1 and m2, parameterized by t.
	void	set_lerp(const matrix& m1, const matrix& m2, float t);

	/// Set the scale & rotation part of the matrix. angle in radians.
	void	set_scale_rotation(float x_scale, float y_scale, float rotation);

	/// Initialize from the SWF input stream.
	void	read(stream* in);

	/// Debug log.
	void	print() const;

	/// Transform point 'p' by our matrix. 
	//
	/// Put the result in *result.
	///
	void	transform(point* result, const point& p) const;

	/// Transform vector 'v' by our matrix. Doesn't apply translation.
	//
	/// Put the result in *result.
	///
	void	transform_vector(point* result, const point& p) const;

	/// Transform point 'p' by the inverse of our matrix. 
	//
	/// Put result in *result.
	///
	void	transform_by_inverse(point* result, const point& p) const;

	/// Set this matrix to the inverse of the given matrix.
	void	set_inverse(const matrix& m);

	/// Return true if this matrix reverses handedness.
	bool	does_flip() const;	

	/// Return the determinant of the 2x2 rotation/scale part only.
	float	get_determinant() const;

	/// Return the maximum scale factor that this transform applies.
	//
	/// For assessing scale, when determining acceptable
	/// errors in tesselation.
	///
	float	get_max_scale() const;	

	/// return the magnitude scale of our x coord output
	float	get_x_scale() const;

	/// return the magnitude scale of our y coord output
	float	get_y_scale() const;

	/// return our rotation component (in radians)
	float	get_rotation() const;
};


//
// point: used by rect which is used by render_handler (otherwise would be in internal gnash_types.h)
//


struct point
{
	float	m_x, m_y;

	point() : m_x(0), m_y(0) {}
	point(float x, float y) : m_x(x), m_y(y) {}

	void	set_lerp(const point& a, const point& b, float t)
		// Set to a + (b - a) * t
		{
			m_x = a.m_x + (b.m_x - a.m_x) * t;
			m_y = a.m_y + (b.m_y - a.m_y) * t;
		}

	bool operator==(const point& p) const { return m_x == p.m_x && m_y == p.m_y; }

	bool	bitwise_equal(const point& p) const;
};


//
// rect: rectangle type, used by render handler
//


struct rect
{
	float	m_x_min, m_x_max, m_y_min, m_y_max;

	void	read(stream* in);
	void	print() const;
	bool	point_test(float x, float y) const;
	void	expand_to_point(float x, float y);
	float	width() const { return m_x_max-m_x_min; }
	float	height() const { return m_y_max-m_y_min; }

	point	get_corner(int i) const;

	void	enclose_transformed_rect(const matrix& m, const rect& r);

	void	set_lerp(const rect& a, const rect& b, float t);
};


/// Color transform type, used by render handler
struct cxform
{
    /// [RGBA][multiply, add]
    float	m_[4][2];
    
    /// Initialize to the identity color transform (no transform)
    cxform();
    
    /// Concatenate c's transform onto ours. 
    //
    /// When transforming colors, c's transform is applied
    /// first, then ours.
    ///
    void concatenate(const cxform& c);
    
    /// Apply our transform to the given color; return the result.
    rgba transform(const rgba in) const;
    
    /// Read RGB from the SWF input stream.
    void read_rgb(stream* in);
    
    /// Read RGBA from the SWF input stream.
    void read_rgba(stream* in);
    
    /// Force component values to be in range.
    void clamp();
    
    /// Debug log.
    void print() const;
    
    /// The identity color transform (no transform)
    static cxform	identity;
};


//
// texture and render callback handler.
//

/// Your render_handler creates bitmap_info's for gnash.  You
/// need to subclass bitmap_info in order to add the
/// information and functionality your app needs to render
/// using textures.
struct bitmap_info : public ref_counted
{
	unsigned int	m_texture_id;		// nuke?
	int		m_original_width;	// nuke?
	int		m_original_height;	// nuke?
		
	bitmap_info()
		:
		m_texture_id(0),
		m_original_width(0),
		m_original_height(0)
		{
		}
};
	
/// You must define a subclass of render_handler, and pass an
/// instance to set_render_handler().
struct render_handler
{
	virtual ~render_handler() {}

	// Your handler should return these with a ref-count of 0.  (@@ is that the right policy?)
	virtual bitmap_info*	create_bitmap_info_empty() = 0;	// used when DO_NOT_LOAD_BITMAPS is set
	virtual bitmap_info*	create_bitmap_info_alpha(int w, int h, unsigned char* data) = 0;
	virtual bitmap_info*	create_bitmap_info_rgb(image::rgb* im) = 0;
	virtual bitmap_info*	create_bitmap_info_rgba(image::rgba* im) = 0;

	virtual void	delete_bitmap_info(bitmap_info* bi) = 0;
		
	/// Bracket the displaying of a frame from a movie.
	/// Fill the background color, and set up default
	/// transforms, etc.
	virtual void	begin_display(
		rgba background_color,
		int viewport_x0, int viewport_y0,
		int viewport_width, int viewport_height,
		float x0, float x1, float y0, float y1) = 0;

	virtual void	end_display() = 0;
		
	/// Geometric transforms for mesh and line_strip rendering.
	virtual void	set_matrix(const matrix& m) = 0;

	/// Color transforms for mesh and line_strip rendering.
	virtual void	set_cxform(const cxform& cx) = 0;
		
	/// Draw triangles using the current fill-style 0.
	//
	/// Clears the style list after rendering.
	///
	/// coords is a list of (x,y) coordinate pairs, in
	/// triangle-strip order.  The type of the array should
	/// be int16_t[vertex_count*2]
	///
	virtual void	draw_mesh_strip(const void* coords, int vertex_count) = 0;
		
	/// Draw a line-strip using the current line style.
	//
	/// Clear the style list after rendering.
	///
	/// Coords is a list of (x,y) coordinate pairs, in
	/// sequence.  Each coord is a 16-bit signed integer.
	///
	virtual void	draw_line_strip(const void* coords, int vertex_count) = 0;
		
	/// Set line and fill styles for mesh & line_strip rendering.
	enum bitmap_wrap_mode
	{
		WRAP_REPEAT,
		WRAP_CLAMP
	};
	virtual void	fill_style_disable(int fill_side) = 0;
	virtual void	fill_style_color(int fill_side, rgba color) = 0;
	virtual void	fill_style_bitmap(int fill_side, const bitmap_info* bi, const matrix& m, bitmap_wrap_mode wm) = 0;
		
	virtual void	line_style_disable() = 0;
	virtual void	line_style_color(rgba color) = 0;
	virtual void	line_style_width(float width) = 0;
		
	/// Special function to draw a rectangular bitmap.
	//
	/// Intended for textured glyph rendering.  Ignores
	/// current transforms.
	///
	virtual void	draw_bitmap(
		const matrix&		m,
		const bitmap_info*	bi,
		const rect&		coords,
		const rect&		uv_coords,
		rgba			color) = 0;
		
	virtual void	set_antialiased(bool enable) = 0;
		
	virtual void begin_submit_mask() = 0;
	virtual void end_submit_mask() = 0;
	virtual void disable_mask() = 0;
};
	
/// Keyboard handling
namespace key {
enum code
{
	INVALID = 0,
	A = 65,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	_0 = 48,
	_1,
	_2,
	_3,
	_4,
	_5,
	_6,
	_7,
	_8,
	_9,
	KP_0 = 96,
	KP_1,
	KP_2,
	KP_3,
	KP_4,
	KP_5,
	KP_6,
	KP_7,
	KP_8,
	KP_9,
	KP_MULTIPLY,
	KP_ADD,
	KP_ENTER,
	KP_SUBTRACT,
	KP_DECIMAL,
	KP_DIVIDE,
	F1 = 112,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,
	F13,
	F14,
	F15,
	BACKSPACE = 8,
	TAB,
	CLEAR = 12,
	ENTER,
	SHIFT = 16,
	CONTROL,
	ALT,
	CAPSLOCK = 20,
	ESCAPE = 27,
	SPACE = 32,
	PGDN,
	PGUP,
	END = 35,
	HOME,
	LEFT,
	UP,
	RIGHT,
	DOWN,
	INSERT = 45,
	DELETEKEY,
	HELP,
	NUM_LOCK = 144,
	SEMICOLON = 186,
	EQUALS = 187,
	MINUS = 189,
	SLASH = 191,
	BACKTICK = 192,
	LEFT_BRACKET = 219,
	BACKSLASH = 220,
	RIGHT_BRACKET = 221,
	QUOTE = 222,

	KEYCOUNT
};
}	// end namespace key

/// Key events are global throughout gnash.
/// @@ Maybe someday make these local to the movie_interface?
void	notify_key_event(key::code k, bool down);


/// Some optional helpers.
namespace tools
{

struct process_options
{
	/// @@ not implemented yet (low priority?)
	bool	m_zip_whole_file;

	/// removes existing image data; leaves minimal placeholder tags
	bool	m_remove_image_data;

	bool	m_remove_font_glyph_shapes;

	process_options()
		:
		m_zip_whole_file(false),
		m_remove_image_data(false),
		m_remove_font_glyph_shapes(false)
		{
		}
};

/// Copy tags from *in to *out, applying the given
/// options.  *in should be a SWF-format stream.  The
/// output will be a SWF-format stream.
///
/// Returns 0 on success, or a non-zero error-code on
/// failure.
int	process_swf(tu_file* swf_out, tu_file* swf_in, const process_options& options);
}


}	// namespace gnash

#endif // GNASH_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
