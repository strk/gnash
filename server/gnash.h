// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
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
#include <cctype>   // for poxy wchar_t
#include <cstdarg>  // for va_list arg to sprite_instance::call_method_args()
#include <string>   // for movie_definition* create_movie(std::auto_ptr<tu_file> in, const std::string& url);

#include "as_value.h" // FIXME: for as_c_function_ptr typedef (register_component)

// FIXME: The local usage of these constants should probably be renamed in this
// file because they conflict with definitions in the system header files. Luckily
// they are used in files we don't need, so we should be able to safely redefine
// these here.
#undef INVALID
#undef ESCAPE

class tu_file;
class render_handler;
class weak_proxy;   // forward decl; defined in base/smart_ptr.h

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
DSOEXPORT void  set_sound_handler(sound_handler* s);

/// Get currently registered sound handler
DSOEXPORT sound_handler* get_sound_handler();


///
/// Log & error reporting control.
///

/// Supply a function pointer to receive log & error messages.
void    register_log_callback(void (*callback)(bool error, const char* message));

/// Control verbosity of action processing
void    set_verbose_action(bool verbose);

/// Control verbosity of SWF parsing
void    set_verbose_parse(bool verbose);

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
void    register_progress_callback(progress_callback progress_handle);

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
DSOEXPORT void  register_fscommand_callback(fscommand_callback handler);

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
void    set_curve_max_pixel_error(float pixel_error);
float   get_curve_max_pixel_error();

// Some helpers that may or may not be compiled into your
// version of the library, depending on platform etc.
DSOEXPORT render_handler*   create_render_handler_xbox();
DSOEXPORT render_handler*   create_render_handler_ogl();
//DSOEXPORT render_handler* create_render_handler_cairo(void* cairohandle);

class font;

/// For caching precomputed stuff.  Generally of
/// interest to gnash_processor and programs like it.
class cache_options
{
public:
    bool    m_include_font_bitmaps;
    
    cache_options()
        :
        m_include_font_bitmaps(true)
        {
        }
};


/// Enable/disable attempts to read cache files (.gsc) when loading movies.
DSOEXPORT void  set_use_cache_files(bool use_cache);
    
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
/// The URL to load the movie from.
///
/// @param real_url
/// The url to encode as the _url member of the resulting
/// movie definition. Use NULL if it is not different from
/// the actual url (default). This is used to simulate a run from
/// the official publication url.
///
/// @param startLoaderThread
/// If false only the header will be read, and you'll need to call completeLoad
/// on the returned movie_definition to actually start it. This is tipically 
/// used to postpone parsing until a VirtualMachine is initialized.
/// Initializing the VirtualMachine requires a target SWF version, which can
/// be found in the SWF header.
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
/// The stream to load the movie from. Ownership is transferred
/// to the returned object.
///
/// @param url
/// The url to use as the _url member of the resulting
/// movie definition. This is required as it can not be
/// derived from the tu_file.
///
/// @param startLoaderThread
/// If false only the header will be read, and you'll need to call completeLoad
/// on the returned movie_definition to actually start it. This is tipically 
/// used to postpone parsing until a VirtualMachine is initialized.
/// Initializing the VirtualMachine requires a target SWF version, which can
/// be found in the SWF header.
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
/// The URL to load the movie from.
///
/// @param real_url
/// The url to encode as the _url member of the resulting
/// movie definition. Use NULL if it is not different from
/// the actual url (default). This is used to simulate a run from
/// the official publication url.
///
/// @param startLoaderThread
/// If false only the header will be read, and you'll need to call completeLoad
/// on the returned movie_definition to actually start it. This is tipically 
/// used to postpone parsing until a VirtualMachine is initialized.
/// Initializing the VirtualMachine requires a target SWF version, which can
/// be found in the SWF header.
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
void    precompute_cached_data(movie_definition* movie_def);

/// Initialize gnash core library
//
DSOEXPORT void  gnashInit();

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
DSOEXPORT void  clear();
    

//
// point: used by rect which is used by render_handler (otherwise would be in internal gnash_types.h)
//


/// Point class
//
/// TODO: move in libgeometry/point2d.{cpp,h}, provide a gnash::point2d typedef
/// TODO: make m_x and m_y members private ? (or rename to x,y)
/// TODO: provide a toString() and an output operator
///
class DSOLOCAL point
{
public:
    float   m_x, m_y;

    /// Construct a point with X and Y values set to 0
    point() : m_x(0), m_y(0) {}

    /// Construct a point with X and Y values set to the given values
    point(float x, float y) : m_x(x), m_y(y) {}

    /// Set X and Y values of the point
    void set(float x, float y)
    {
        m_x = x;
        m_y = y;
    }

    /// Set to a + (b - a) * t
    void    set_lerp(const point& a, const point& b, float t)
        {
            m_x = a.m_x + (b.m_x - a.m_x) * t;
            m_y = a.m_y + (b.m_y - a.m_y) * t;
        }

    /// Compare two points for 2d equality
    bool operator==(const point& p) const { return m_x == p.m_x && m_y == p.m_y; }

    /// Return the square of the distance between this and other point.
    float squareDistance(const point& other) const;

    /// Return the distance between this and other point.
    float distance(const point& other) const;

    /// Compare two points for bitwise equality
    //
    /// TODO: (should this really be different by normal equality?)
    ///
    bool    bitwise_equal(const point& p) const;
};


//
// texture and render callback handler.
//

/// Keyboard handling
// TODO: deprecate this table.
// It's wrongly used for
// (1)onClipKeypress, which should use swf-key-code instead, another table.
// (2)characters received from GUI. Different characters might have same codes.
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
    CONTROL = 17,
    ALT = 18,
    CAPSLOCK = 20,
    ESCAPE = 27,
    SPACE = 32,
    PGUP = 33,
    PGDN = 34,
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
    COMMA = 188,
    MINUS = 189,
    PERIOD = 190,
    SLASH = 191,
    BACKTICK = 192,
    LEFT_BRACKET = 219,
    BACKSLASH = 220,
    RIGHT_BRACKET = 221,
    APOSTROPHE = 222,

    PAREN_RIGHT = 48,
    EXCLAM = 49,
    AT = 50,
    HASH = 51,
    DOLLAR = 52,
    PERCENT = 53,
    ASCIICIRCUM = 54,
    AMPERSAND = 55,
    ASTERISK = 56,
    PAREN_LEFT = 57,
    COLON = 186,
    PLUS = 187,
    LESSTHAN = 188,
    UNDERSCORE = 189,
    MORETHAN = 190,
    QUESTION = 191,
    ASCIITILDE = 192,
    CURLYLEFT = 219,
    PIPE = 220,
    CURLYRIGHT = 221,
    QUOTE = 222,

    // Extended ASCII
    NOBREAKSPACE = 160,
    AGRAVE = 192, 
    NTILDE = 209,
    UGRAVE = 217,
    SSHARPSMALL = 223,
    UGRAVESMALL = 249,
    YACUTESMALL = 255, //Last

    KEYCOUNT
};
enum modifier
{
    MOD_NONE = 0,
    MOD_SHIFT = 1,
    MOD_CONTROL = 2,
    MOD_ALT = 4
};

// Gnash character codes. Each code represents a single character on the keyboard.
// The first 128 code are ordered by their correspond ASCII value.
enum gnashKey
{
  CH_INVALID = 0,
  CH_UNKOWN1,
  CH_UNKOWN2,
  CH_UNKOWN3,
  CH_UNKOWN4,
  CH_UNKOWN5,
  CH_UNKOWN6,
  CH_UNKOWN7,
  CH_BACkSPACE = 8,
  CH_TAB = 9,
  CH_UNKOWN10,
  CH_UNKOWN11,
  CH_CLEAR = 12,
  CH_ENTER = 13,
  CH_UNKOWN14,
  CH_UNKOWN15,
  CH_SHIFT = 16,
  CH_CONTROL = 17,
  CH_ALT = 18,
  CH_PAUSE = 19,
  CH_CAPSLOCK = 20,
  CH_UNKOWN21,
  CH_UNKOWN22,
  CH_UNKOWN23,
  CH_UNKOWN24,
  CH_UNKOWN25,  
  CH_UNKOWN26,
  CH_ESC = 27,
  CH_UNKOWN28,
  CH_UNKOWN29,
  CH_UNKOWN30,
  CH_UNKOWN31,
  CH_SPACE = 32,
  CH_EXCLAM = 33,
  CH_DOUBLE_QUOTE = 34,
  CH_HASH = 35,
  CH_DOLLAR = 36,
  CH_PERCENT = 37,
  CH_AMPERSAND = 38 ,
  CH_SINGLE_QUOTE  = 39,
  CH_PAREN_LEFT = 40,
  CH_PAREN_RIGHT = 41,
  CH_ASTERISK = 42,
  CH_PLUS = 43,
  CH_COMMA = 44,
  CH_MINUS = 45,
  CH_PERIOD = 46,
  CH_SLASH =47,

  CH_0 = 48,
  CH_1,
  CH_2,
  CH_3,
  CH_4,
  CH_5,
  CH_6,
  CH_7,
  CH_8,
  CH_9 = 57,
  
  CH_COLON = 58,
  CH_SEMICOLON = 59,
  CH_LESS = 60,
  CH_EQUAL = 61,
  CH_MORE = 62,
  CH_QUESTION = 63,
  CH_AT = 64,
  
  CH_A = 65,
  CH_B,
  CH_C,
  CH_D,
  CH_E,
  CH_F,
  CH_G,
  CH_H,
  CH_I,
  CH_J,
  CH_K,
  CH_L,
  CH_M,
  CH_N,
  CH_O,
  CH_P,
  CH_Q,
  CH_R,
  CH_S,
  CH_T,
  CH_U,
  CH_V,
  CH_W,
  CH_X,
  CH_Y,
  CH_Z = 90,
  
  CH_LEFTBRACKET =  91,
  CH_BACKSLASH =92,
  CH_RIGHTBRACKET = 93,
  CH_CARET = 94,
  CH_UNDERSCORE = 95,
  CH_BACKQUOTE = 96,
  
  CH_a = 97,
  CH_b,
  CH_c,
  CH_d,
  CH_e,
  CH_f,
  CH_g,
  CH_h,
  CH_i,
  CH_j,
  CH_k,
  CH_l,
  CH_m,
  CH_n,
  CH_o,
  CH_p,
  CH_q,
  CH_r,
  CH_s,
  CH_t,
  CH_u,
  CH_v,
  CH_w,
  CH_x,
  CH_y,
  CH_z = 122,
  CH_LEFTBRACE = 123,
  CH_PIPE = 124,
  CH_RIGHTBRACE = 125,
  CH_ASCIITILDE = 126,
  CH_DELETE = 127,
  
  CH_KP_0 = 128,
  CH_KP_1,
  CH_KP_2,
  CH_KP_3,
  CH_KP_4,
  CH_KP_5,
  CH_KP_6,
  CH_KP_7,
  CH_KP_8,
  CH_KP_9 = 137,
      
  CH_F1 = 138,
  CH_F2,
  CH_F3,
  CH_F4,
  CH_F5,
  CH_F6,
  CH_F7,
  CH_F8,
  CH_F9,
  CH_F10,
  CH_F11,
  CH_F12,
  CH_F13,
  CH_F14,
  CH_F15 = 152,
  
  CH_UP = 153,
  CH_DOWN = 154,
  CH_RIGHT = 155,
  CH_LEFT = 156,
  CH_INSERT = 157,
  CH_HOME = 158,
  CH_END  = 159,
  CH_PAGEUP = 160,
  CH_PAGEDOWN = 161,

  CH_KP_ADD = 162,
  CH_KP_SUBTRACT = 163,
  CH_KP_MULITPLY = 164,
  CH_KP_DEVIDE = 165,
  CH_KP_DECIMAL = 166,
  CH_KP_ENTER = 167,
  
  CH_NUMLOCK = 168,

// Extended ASCII
  CH_NOBREAKSPACE = 169,
  CH_AGRAVE = 170, 
  CH_NTILDE = 171,
  CH_UGRAVE = 172,
  CH_SSHARPSMALL = 173,
  CH_UGRAVESMALL = 174,
  CH_YACUTESMALL = 175, 

  // TODO: add other function keys and extend the codeMap
  CH_COUNT
};

const unsigned char codeMap[CH_COUNT][3] = {
//{swfKeyCode, keycode, asciiKeyCode}
  {0,   0,   0}, // CH_INVALID = 0
  {0,   0,   0}, // CH_UNKOWN1
  {0,   0,   0}, // CH_UNKOWN2
  {0,   0,   0}, // CH_UNKOWN3
  {0,   0,   0}, // CH_UNKOWN4
  {0,   0,   0}, // CH_UNKOWN5
  {0,   0,   0}, // CH_UNKOWN6
  {0,   0,   0}, // CH_UNKOWN7
  {8,   8,   8}, // CH_BACkSPACE = 8
  {18,  9,   9}, // CH_TAB = 9
  {0,   0,   0}, // CH_UNKOWN10
  {0,   0,   0}, // CH_UNKOWN11
  {0,  12,   0}, // CH_CLEAR = 12
  {13, 13,  13}, // CH_ENTER = 13
  {0,   0,   0}, // CH_UNKOWN14
  {0,   0,   0}, // CH_UNKOWN15
  {0,  16,   0}, // CH_SHIFT = 16
  {0,  17,   0}, // CH_CONTROL = 17
  {0,  18,   0}, // CH_ALT = 18
  {0,  19,   0}, // CH_PAUSE = 19
  {0,  20,   0}, // CH_CAPSLOCK = 20
  {0,   0,   0}, // CH_UNKOWN21
  {0,   0,   0}, // CH_UNKOWN22
  {0,   0,   0}, // CH_UNKOWN23
  {0,   0,   0}, // CH_UNKOWN24
  {0,   0,   0}, // CH_UNKOWN25 
  {0,   0,   0}, // CH_UNKOWN26
  {19, 27,  27}, // CH_ESC = 27
  {0,   0,   0}, // CH_UNKOWN28
  {0,   0,   0}, // CH_UNKOWN29
  {0,   0,   0}, // CH_UNKOWN30
  {0,   0,   0}, // CH_UNKOWN31
  {32, 32,  32}, // CH_SPACE = 32
  {33, 33,  33}, // CH_EXCLAM = 33
  {34, 34,  34}, // CH_DOUBLE_QUOTE = 34
  {35, 35,  35}, // CH_HASH = 35
  {36, 36,  36}, // CH_DOLLAR = 36
  {37, 37,  37}, // CH_PERCENT = 37
  {38, 38,  38}, // CH_AMPERSAND = 38 
  {39, 39,  39}, // CH_SINGLE_QUOTE  = 39
  {40, 40,  40}, // CH_PAREN_LEFT = 40
  {41, 41,  41}, // CH_PAREN_RIGHT = 41
  {42, 42,  42}, // CH_ASTERISK = 42
  {43, 43,  43}, // CH_PLUS = 43
  {44, 44,  44}, // CH_COMMA = 44
  {45, 45,  45}, // CH_MINUS = 45
  {46, 46,  46}, // CH_PERIOD = 46
  {47, 47,  47}, // CH_SLASH = 47
  {48, 48,  48}, // CH_0 = 48
  {49, 49,  49}, // CH_1
  {50, 50,  50}, // CH_2
  {51, 51,  51}, // CH_3
  {52, 52,  52}, // CH_4
  {53, 53,  53}, // CH_5
  {54, 54,  54}, // CH_6
  {55, 55,  55}, // CH_7
  {56, 56,  56}, // CH_8
  {57, 57,  57}, // CH_9 = 57
  {58, 58,  58}, // CH_COLON = 58
  {59, 59,  59}, // CH_SEMICOLON = 59
  {60, 60,  60}, // CH_LESS = 60
  {61, 61,  61}, // CH_EQUAL = 61
  {62, 62,  62}, // CH_MORE = 62
  {63, 63,  63}, // CH_QUESTION = 63
  {64, 64,  64}, // CH_AT = 64
  {65, 65,  65}, // CH_A = 65
  {66, 66,  66}, // CH_B
  {67, 67,  67}, // CH_C
  {68, 68,  68}, // CH_D
  {69, 69,  69}, // CH_E
  {70, 70,  70}, // CH_F
  {71, 71,  71}, // CH_G
  {72, 72,  72}, // CH_H
  {73, 73,  73}, // CH_I
  {74, 74,  74}, // CH_J
  {75, 75,  75}, // CH_K
  {76, 76,  76}, // CH_L
  {77, 77,  77}, // CH_M
  {78, 78,  78}, // CH_N
  {79, 79,  79}, // CH_O
  {80, 80,  80}, // CH_P
  {81, 81,  81}, // CH_Q
  {82, 82,  82}, // CH_R
  {83, 83,  83}, // CH_S
  {84, 84,  84}, // CH_T
  {85, 85,  85}, // CH_U
  {86, 86,  86}, // CH_V
  {87, 87,  87}, // CH_W
  {88, 88,  88}, // CH_X
  {89, 89,  89}, // CH_Y
  {90, 90,  90}, // CH_Z = 90
  {91, 91,  91}, // CH_LEFTBRACKET = 91
  {92, 92,  92}, // CH_BACKSLASH = 92
  {93, 93,  93}, // CH_RIGHTBRACKET = 93
  {94, 94,  94}, // CH_CARET = 94
  {95, 95,  95}, // CH_UNDERSCORE = 95
  {96, 96,  96}, // CH_BACKQUOTE = 96
  {97, 65,  97}, // CH_a = 97
  {98, 66,  98}, // CH_b
  {99, 67,  99}, // CH_c
  {100,68, 100}, // CH_d
  {101,69, 101}, // CH_e
  {102,70, 102}, // CH_f
  {103,71, 103}, // CH_g
  {104,72, 104}, // CH_h
  {105,73, 105}, // CH_i
  {106,74, 106}, // CH_j
  {107,75, 107}, // CH_k
  {108,76, 108}, // CH_l
  {109,77, 109}, // CH_m
  {110,78, 110}, // CH_n
  {111,79, 111}, // CH_o
  {112,80, 112}, // CH_p
  {113,81, 113}, // CH_q
  {114,82, 114}, // CH_r
  {115,83, 115}, // CH_s
  {116,84, 116}, // CH_t
  {117,85, 117}, // CH_u
  {118,86, 118}, // CH_v
  {119,87, 119}, // CH_w
  {120,88, 120}, // CH_x
  {121,89, 121}, // CH_y
  {122,90, 122}, // CH_z = 122
  {123,219,123}, // CH_LEFTBRACE = 123
  {124,220,124}, // CH_PIPE = 124
  {125,221,125}, // CH_RIGHTBRACE = 125
  {126,192,126}, // CH_ASCIITILDE = 126
  {6,  46, 127}, // CH_DELETE = 127
  {48, 96,  48}, // CH_KP_0 = 128
  {49, 97,  49}, // CH_KP_1
  {50, 98,  50}, // CH_KP_2
  {51, 99,  51}, // CH_KP_3
  {52, 100, 52}, // CH_KP_4
  {53, 101, 53}, // CH_KP_5
  {54, 102, 54}, // CH_KP_6
  {55, 103, 55}, // CH_KP_7
  {56, 104, 56}, // CH_KP_8
  {57, 105, 57}, // CH_KP_9
  {0,  112,  0}, // CH_F1 = 138
  {0,  113,  0}, // CH_F2
  {0,  114,  0}, // CH_F3
  {0,  115,  0}, // CH_F4
  {0,  116,  0}, // CH_F5
  {0,  117,  0}, // CH_F6
  {0,  118,  0}, // CH_F7
  {0,  119,  0}, // CH_F8
  {0,  120,  0}, // CH_F9
  {0,  121,  0}, // CH_F10
  {0,  122,  0}, // CH_F11
  {0,  123,  0}, // CH_F12
  {0,  124,  0}, // CH_F13
  {0,  125,  0}, // CH_F14
  {0,  126,  0}, // CH_F15 = 152
  {14, 38,   0}, // CH_UP = 153
  {15, 40,   0}, // CH_DOWN = 154
  {2,  39,   0}, // CH_RIGHT = 155
  {1,  37,   0}, // CH_LEFT = 156
  {5,  45,   0}, // CH_INSERT = 157
  {3,  36,   0}, // CH_HOME = 158
  {4,  35,   0}, // CH_END = 159
  {16, 33,   0}, // CH_PAGEUP = 160
  {17, 34,   0}, // CH_PAGEDOWN = 161
  {0, 107,  43}, // CH_KP_ADD = 162
  {0, 109,  45}, // CH_KP_SUBTRACT = 163
  {0, 106,  42}, // CH_KP_MULITPLY = 164
  {0, 111,  47}, // CH_KP_DEVIDE = 165
  {0, 110,  46}, // CH_KP_DECIMAL = 166
  {13, 13,  13}, // CH_KP_ENTER = 167
  {0, 144,   0}, // CH_NUMLOCK = 168
  {0, 160,   0}, // CH_ NOBREAKSPACE = 169,
  {0, 192,   0}, // CH_AGRAVE = 170 
  {0, 209,   0}, // CH_NTILDE = 171
  {0, 217,   0}, // CH_UGRAVE = 172
  {0, 223,   0}, // CH_SSHARPSMALL = 173
  {0, 249,   0}, // CH_UGRAVESMALL = 174
  {0, 255,   0}, // CH_YACUTESMALL = 175 
};

}   // end namespace key

/// Some optional helpers.
namespace tools
{

class DSOLOCAL process_options
{
public:
    /// @@ not implemented yet (low priority?)
    bool    m_zip_whole_file;

    /// removes existing image data; leaves minimal placeholder tags
    bool    m_remove_image_data;

    bool    m_remove_font_glyph_shapes;

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
int process_swf(tu_file* swf_out, tu_file* swf_in, const process_options& options);
}


}   // namespace gnash

#endif // GNASH_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
