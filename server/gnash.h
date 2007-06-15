// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/// \mainpage
///
///  See Related Pages for movies and sprites informations
///

#ifndef GNASH_H
#define GNASH_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"

#include <memory> // for auto_ptr
#include <cctype>	// for poxy wchar_t
#include <cstdarg>	// for va_list arg to sprite_instance::call_method_args()
#include <string>	// for movie_definition* create_movie(std::auto_ptr<tu_file> in, const std::string& url);

#include "as_value.h" // FIXME: for as_c_function_ptr typedef (register_component)

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
// render_handler interface to not depend on these classes at all.
namespace image { class image_base; class rgb; class rgba; }

// forward decl
namespace jpeg { class input; }

namespace gnash {
// Forward declarations.
class action_buffer;
class as_value;
class execute_tag;
class font;
class matrix;
class movie;
class sprite_instance;
class movie_definition;
class render_handler;
class resource;
class rgba;
class sound_handler;
class stream;
class URL;
class rect;

// Sound callbacks stuff

/// \brief
/// Pass in a sound handler, so you can handle audio on behalf of
/// gnash.  This is optional; if you don't set a handler, or set
/// NULL, then sounds won't be played.
///
/// If you want sound support, you should set this at startup,
/// before loading or playing any movies!
///
DSOEXPORT void	set_sound_handler(sound_handler* s);

/// Get currently registered sound handler
DSOEXPORT sound_handler* get_sound_handler();


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
DSOEXPORT void    set_render_handler(render_handler* s);

/// Set the base url against which to resolve relative urls
DSOEXPORT void set_base_url(const URL& url);

/// Return base url
DSOEXPORT const gnash::URL& get_base_url();

/// Signature of file opener callback function
typedef tu_file* (*file_opener_callback)(const URL& url);

/// Signature of progress callback function
typedef void (*progress_callback)(unsigned int loaded_bytes, unsigned int total_bytes);

/// Register a callback for displaying SWF load progress.
void	register_progress_callback(progress_callback progress_handle);

/// Signature of fscommand callback function
typedef void (*fscommand_callback)(sprite_instance* movie, const char* command, const char* arg);

/// ActionScripts embedded in a movie can use the built-in
/// fscommand() function to send data back to the host
/// application.  If you are interested in this data, register
/// a handler, which will be called when the embedded scripts
/// call fscommand().
///
/// The handler gets the sprite_instance* that the script is
/// embedded in, and the two string arguments passed by the
/// script to fscommand().
DSOEXPORT void	register_fscommand_callback(fscommand_callback handler);

/// Use this to register gnash extension
//
/// WARNING: does not convert name, make sure to pass a lowercase
///          name if SWF version is < 7 ! It seems currently no code
///          calls this function..
///
///
void register_component(const std::string& name, as_c_function_ptr handler);

/// Use this to control how finely curves are subdivided.  1.0
/// is the default; it's a pretty good value.  Larger values
/// result in coarser, more angular curves with fewer vertices.
void	set_curve_max_pixel_error(float pixel_error);
float	get_curve_max_pixel_error();

// Some helpers that may or may not be compiled into your
// version of the library, depending on platform etc.
DSOEXPORT render_handler*	create_render_handler_xbox();
DSOEXPORT render_handler*	create_render_handler_ogl();
//DSOEXPORT render_handler*	create_render_handler_cairo(void* cairohandle);

class font;

/// For caching precomputed stuff.  Generally of
/// interest to gnash_processor and programs like it.
class cache_options
{
public:
	bool	m_include_font_bitmaps;
	
	cache_options()
		:
		m_include_font_bitmaps(true)
		{
		}
};


/// Enable/disable attempts to read cache files (.gsc) when loading movies.
DSOEXPORT void	set_use_cache_files(bool use_cache);
	
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
/// Or use boost::intrusive_ptr<T> from base/smart_ptr.h if you want.
///
/// @@ Hm, need to think about these creation API's.  Perhaps
/// divide it into "low level" and "high level" calls.  Also,
/// perhaps we need a "context" object that contains all
/// global-ish flags, libraries, callback pointers, font
/// library, etc.
///
/// IFF real_url is given, the movie's url will be set to that value.
///
/// @param url
///	The URL to load the movie from.
///
/// @param real_url
///	The url to encode as the _url member of the resulting
///	movie definition. Use NULL if it is not different from
///	the actual url (default). This is used to simulate a run from
///	the official publication url.
///
/// @param startLoaderThread
///	If false only the header will be read, and you'll need to call completeLoad
///	on the returned movie_definition to actually start it. This is tipically 
///	used to postpone parsing until a VirtualMachine is initialized.
///	Initializing the VirtualMachine requires a target SWF version, which can
///	be found in the SWF header.
///
movie_definition* create_movie(const URL& url, const char* real_url=NULL, bool startLoaderThread=true);

/// Load a movie from an already opened stream.
//
/// The movie can be both an SWF or JPEG, the url parameter
/// will be used to set the _url member of the resulting object.
///
/// No attempt will be made to load associated .gsc (cache) files
/// by this function.
///
/// @param in
///	The stream to load the movie from. Ownership is transferred
///	to the returned object.
///
/// @param url
///	The url to use as the _url member of the resulting
///	movie definition. This is required as it can not be
///	derived from the tu_file.
///
/// @param startLoaderThread
///	If false only the header will be read, and you'll need to call completeLoad
///	on the returned movie_definition to actually start it. This is tipically 
///	used to postpone parsing until a VirtualMachine is initialized.
///	Initializing the VirtualMachine requires a target SWF version, which can
///	be found in the SWF header.
///
DSOEXPORT movie_definition* create_movie(std::auto_ptr<tu_file> in, const std::string& url, bool startLoaderThread=true);

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
/// Or use boost::intrusive_ptr<T> from base/smart_ptr.h if you want.
///
/// IFF real_url is given, the movie's url will be set to that value.
///
/// @param url
///	The URL to load the movie from.
///
/// @param real_url
///	The url to encode as the _url member of the resulting
///	movie definition. Use NULL if it is not different from
///	the actual url (default). This is used to simulate a run from
///	the official publication url.
///
/// @param startLoaderThread
///	If false only the header will be read, and you'll need to call completeLoad
///	on the returned movie_definition to actually start it. This is tipically 
///	used to postpone parsing until a VirtualMachine is initialized.
///	Initializing the VirtualMachine requires a target SWF version, which can
///	be found in the SWF header.
///
DSOEXPORT movie_definition* create_library_movie(const URL& url, const char* real_url=NULL, bool startLoaderThread=true);
	

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

/// Initialize gnash core library
//
DSOEXPORT void	gnashInit();

/// Maximum release of resources. 
//
/// Calls clear_library() and
/// fontlib::clear(), and also clears some extra internal stuff
/// that may have been allocated (e.g. global ActionScript
/// objects).  This should get all gnash structures off the
/// heap, with the exception of any objects that are still
/// referenced by the host program and haven't had drop_ref()
/// called on them.
///
DSOEXPORT void	clear();

//
// Library management
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
// point: used by rect which is used by render_handler (otherwise would be in internal gnash_types.h)
//


class DSOLOCAL point
{
public:
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

	/// Return the square of the distance between this and other point.
	float squareDistance(const point& other) const;

	/// Return the distance between this and other point.
	float distance(const point& other) const;

	bool	bitwise_equal(const point& p) const;
};


//
// texture and render callback handler.
//

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
enum modifier
{
    MOD_NONE = 0,
    MOD_SHIFT = 1,
    MOD_CONTROL = 2,
    MOD_ALT = 4
};
}	// end namespace key

/// Some optional helpers.
namespace tools
{

class DSOLOCAL process_options
{
public:
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
