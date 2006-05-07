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
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// This needs to be included first for NetBSD systems or we get a weird
// problem with pthread_t being defined too many times if we use any
// STL containers.
#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

#include <zlib.h>

#include "tu_file.h"
#include "utility.h"
#include "action.h"
#include "button.h"
#include "impl.h"
#include "font.h"
#include "fontlib.h"
#include "log.h"
#include "morph2.h"
#include "render.h"
#include "shape.h"
#include "stream.h"
#include "styles.h"
#include "dlist.h"
#include "timers.h"
#include "image.h"
#include "jpeg.h"
#include "zlib_adapter.h"
//#include "Sprite.h"
#include "sprite_definition.h"
#include "Function.h"
//#include "Movie.h"
#include "movie_def_impl.h"
#include "swf.h"

namespace gnash
{
bool	s_verbose_action = false;
bool	s_verbose_parse = false;

#ifndef NDEBUG
bool	s_verbose_debug = true;
#else
bool	s_verbose_debug = false;
#endif

// Enable/disable log messages re: actions.
void
set_verbose_action(bool verbose)
{
    s_verbose_action = verbose;
}

// Enable/disable log messages re: parsing the movie.
void
set_verbose_parse(bool verbose)
{
    s_verbose_parse = verbose;
}

static bool
s_use_cache_files = true;

// Enable/disable attempts to read cache files when loading
// movies.
void
set_use_cache_files(bool use_cache)
{
    s_use_cache_files = use_cache;
}

// Keep a table of loader functions for the different tag types.
hash<int, loader_function>	s_tag_loaders;

    // Associate the specified tag type with the given tag loader
    // function.
void
register_tag_loader(int tag_type, loader_function lf)
{
    assert(s_tag_loaders.get(tag_type, NULL) == false);
    assert(lf != NULL);
    
    s_tag_loaders.add(tag_type, lf);
}


//
// file_opener callback stuff
//
static file_opener_callback	s_opener_function = NULL;

void
register_file_opener_callback(file_opener_callback opener)
    // Host calls this to register a function for opening files,
    // for loading movies.
{
    s_opener_function = opener;
}

character*	character_def::create_character_instance(movie* parent, int id)
    // Default.  Make a generic_character.
{
    return new generic_character(this, parent, id);
}


//
// ref_counted
//


ref_counted::ref_counted()
    :
    m_ref_count(0),
    m_weak_proxy(0)
{
}

ref_counted::~ref_counted()
{
    assert(m_ref_count == 0);

    if (m_weak_proxy)
	{
	    m_weak_proxy->notify_object_died();
	    m_weak_proxy->drop_ref();
	}
}

void	ref_counted::add_ref() const
{
    assert(m_ref_count >= 0);
    m_ref_count++;
}

void	ref_counted::drop_ref() const
{
    assert(m_ref_count > 0);
    m_ref_count--;
    if (m_ref_count <= 0)
	{
	    // Delete me!
	    delete this;
	}
}

weak_proxy* ref_counted::get_weak_proxy() const
{
    assert(m_ref_count > 0);	// By rights, somebody should be holding a ref to us.

    if (m_weak_proxy == NULL)    // Host calls this to register a function for progress bar handling
	// during loading movies.

	{
	    m_weak_proxy = new weak_proxy;
	    m_weak_proxy->add_ref();
	}

    return m_weak_proxy;
}


//
// character
//


void
character::do_mouse_drag()
    // Implement mouse-dragging for this movie.
{
    drag_state	st;
    get_drag_state(&st);
    if (this == st.m_character)
	{
	    // We're being dragged!
	    int	x, y, buttons;
	    get_root_movie()->get_mouse_state(&x, &y, &buttons);

	    point	world_mouse(PIXELS_TO_TWIPS(x), PIXELS_TO_TWIPS(y));
	    if (st.m_bound)
		{
		    // Clamp mouse coords within a defined rect.
		    world_mouse.m_x =
			fclamp(world_mouse.m_x, st.m_bound_x0, st.m_bound_x1);
		    world_mouse.m_y =
			fclamp(world_mouse.m_y, st.m_bound_y0, st.m_bound_y1);
		}

	    if (st.m_lock_center)
		{
		    matrix	world_mat = get_world_matrix();
		    point	local_mouse;
		    world_mat.transform_by_inverse(&local_mouse, world_mouse);

		    matrix	parent_world_mat;
		    if (m_parent)
			{
			    parent_world_mat = m_parent->get_world_matrix();
			}

		    point	parent_mouse;
		    parent_world_mat.transform_by_inverse(&parent_mouse, world_mouse);
					
		    // Place our origin so that it coincides with the mouse coords
		    // in our parent frame.
		    matrix	local = get_matrix();
		    local.m_[0][2] = parent_mouse.m_x;
		    local.m_[1][2] = parent_mouse.m_y;
		    set_matrix(local);
		}
	    else
		{
		    // Implement relative drag...
		}
	}
}

static void	ensure_loaders_registered()
{
    static bool	s_registered = false;
	
    if (s_registered == false)
	{
		// Register the standard loaders.
		s_registered = true;

		register_tag_loader(SWF::END,		end_loader);
		register_tag_loader(SWF::DEFINESHAPE,	define_shape_loader);
		register_tag_loader(SWF::FREECHARACTER, fixme_loader); // 03
		register_tag_loader(SWF::PLACEOBJECT,	place_object_2_loader);
		register_tag_loader(SWF::REMOVEOBJECT,	remove_object_2_loader);
		register_tag_loader(SWF::DEFINEBITS,	define_bits_jpeg_loader);
		register_tag_loader(SWF::DEFINEBUTTON,	button_character_loader);
		register_tag_loader(SWF::JPEGTABLES,	jpeg_tables_loader);
		register_tag_loader(SWF::SETBACKGROUNDCOLOR, set_background_color_loader);
		register_tag_loader(SWF::DEFINEFONT,	define_font_loader);
		register_tag_loader(SWF::DEFINETEXT,	define_text_loader);
		register_tag_loader(SWF::DOACTION,	do_action_loader);
		register_tag_loader(SWF::DEFINEFONTINFO, define_font_info_loader);
		register_tag_loader(SWF::DEFINESOUND,	define_sound_loader);
		register_tag_loader(SWF::STARTSOUND,	start_sound_loader);
		// 16 _UNKNOWN_ unimplemented
		register_tag_loader(SWF::DEFINEBUTTONSOUND, button_sound_loader);
		register_tag_loader(SWF::SOUNDSTREAMHEAD, fixme_loader); // 18
		register_tag_loader(SWF::SOUNDSTREAMBLOCK, fixme_loader); // 19
		register_tag_loader(SWF::DEFINELOSSLESS, define_bits_lossless_2_loader);
		register_tag_loader(SWF::DEFINEBITSJPEG2, define_bits_jpeg2_loader);
		register_tag_loader(SWF::DEFINESHAPE2,	define_shape_loader);
		register_tag_loader(SWF::DEFINEBUTTONCXFORM, fixme_loader); // 23
		// "protect" tag; we're not an authoring tool so we don't care.
		register_tag_loader(SWF::PROTECT,	null_loader);
		register_tag_loader(SWF::PATHSAREPOSTSCRIPT, fixme_loader); // 25
		register_tag_loader(SWF::PLACEOBJECT2,	place_object_2_loader);
		// 27 - _UNKNOWN_ unimplemented
		register_tag_loader(SWF::REMOVEOBJECT2, remove_object_2_loader);
		register_tag_loader(SWF::SYNCFRAME,	fixme_loader); // 29
		// 30 - _UNKNOWN_ unimplemented
		register_tag_loader(SWF::FREEALL,	fixme_loader); // 31
		register_tag_loader(SWF::DEFINESHAPE3,	define_shape_loader);
		register_tag_loader(SWF::DEFINETEXT2,	define_text_loader);
		register_tag_loader(SWF::DEFINEBUTTON2, button_character_loader);
		register_tag_loader(SWF::DEFINEBITSJPEG3, define_bits_jpeg3_loader);
		register_tag_loader(SWF::DEFINELOSSLESS2, define_bits_lossless_2_loader);
		register_tag_loader(SWF::DEFINEEDITTEXT, define_edit_text_loader);
		register_tag_loader(SWF::DEFINEVIDEO,	fixme_loader); // 38
		register_tag_loader(SWF::DEFINESPRITE,  sprite_loader);
		register_tag_loader(SWF::NAMECHARACTER, fixme_loader); // 40
		register_tag_loader(SWF::SERIALNUMBER,  fixme_loader); // 41
		register_tag_loader(SWF::DEFINETEXTFORMAT, fixme_loader); // 42
		register_tag_loader(SWF::FRAMELABEL,	frame_label_loader);
		// 44 - _UNKNOWN_ unimplemented
		register_tag_loader(SWF::SOUNDSTREAMHEAD2, fixme_loader); // 45
		register_tag_loader(SWF::DEFINEMORPHSHAPE, define_shape_morph_loader);
		register_tag_loader(SWF::FRAMETAG,	fixme_loader); // 47
		register_tag_loader(SWF::DEFINEFONT2,	define_font_loader);
		register_tag_loader(SWF::GENCOMMAND,	fixme_loader); // 49
		register_tag_loader(SWF::DEFINECOMMANDOBJ, fixme_loader); // 50
		register_tag_loader(SWF::CHARACTERSET,  fixme_loader); // 51
		register_tag_loader(SWF::FONTREF,	fixme_loader); // 52
		// 53 - _UNKNOWN_ unimplemented
		// 54 - _UNKNOWN_ unimplemented
		// 55 - _UNKNOWN_ unimplemented
		register_tag_loader(SWF::EXPORTASSETS,	export_loader);
		register_tag_loader(SWF::IMPORTASSETS,  import_loader);
		// 58 - _UNKNOWN_ unimplemented
		register_tag_loader(SWF::INITACTION,	do_init_action_loader);   
		register_tag_loader(SWF::DEFINEVIDEOSTREAM, fixme_loader); // 60
		register_tag_loader(SWF::VIDEOFRAME,	fixme_loader); // 61
	}
}



void	get_movie_info(
    const char* filename,
    int* version,
    int* width,
    int* height,
    float* frames_per_second,
    int* frame_count,
    int* tag_count
    )
    // Attempt to read the header of the given .swf movie file.
    // Put extracted info in the given vars.
    // Sets *version to 0 if info can't be extracted.
{
    //printf("%s: filename is %s\n",  __PRETTY_FUNCTION__, filename);

    if (s_opener_function == NULL) {
	log_error("error: get_movie_info(): no file opener function registered\n");
	if (version) *version = 0;
	return;
    }
    
    tu_file*	in = s_opener_function(filename);
    if (in == NULL || in->get_error() != TU_FILE_NO_ERROR) {
	log_error("error: get_movie_info(): can't open '%s'\n", filename);
	if (version) *version = 0;
	delete in;
	return;
    }
    
    Uint32	file_start_pos = in->get_position();
    Uint32	header = in->read_le32();
    Uint32	file_length = in->read_le32();
    Uint32	file_end_pos = file_start_pos + file_length;
    
    int	local_version = (header >> 24) & 255;
    if ((header & 0x0FFFFFF) != 0x00535746
	&& (header & 0x0FFFFFF) != 0x00535743) {
	// ERROR
	log_error("error: get_movie_info(): file '%s' does not start with a SWF header!\n", filename);
	if (version) *version = 0;
	delete in;
	return;
    }
    bool	compressed = (header & 255) == 'C';
    
    tu_file*	original_in = NULL;
    if (compressed) {
#if TU_CONFIG_LINK_TO_ZLIB == 0
	log_error("get_movie_info(): can't read zipped SWF data; TU_CONFIG_LINK_TO_ZLIB is 0!\n");
	return;
#endif
	original_in = in;
	
	// Uncompress the input as we read it.
	in = zlib_adapter::make_inflater(original_in);
	
	// Subtract the size of the 8-byte header, since
	// it's not included in the compressed
	// stream length.
	file_length -= 8;
    }
    
    stream	str(in);
    
    rect	frame_size;
    frame_size.read(&str);
    
    float	local_frame_rate = str.read_u16() / 256.0f;
    int	local_frame_count = str.read_u16();

    if (version) *version = local_version;
    if (width) *width = int(frame_size.width() / 20.0f + 0.5f);
    if (height) *height = int(frame_size.height() / 20.0f + 0.5f);
    if (frames_per_second) *frames_per_second = local_frame_rate;
    if (frame_count) *frame_count = local_frame_count;
    
    if (tag_count)
	{
	    // Count tags.
	    int local_tag_count = 0;
	    while ((Uint32) str.get_position() < file_end_pos)
		{
		    str.open_tag();
		    str.close_tag();
		    local_tag_count++;
		}
	    *tag_count = local_tag_count;
	}

    delete in;
    delete original_in;
}

movie_definition* create_movie(const char* filename)
{
    //printf("%s: filename is %s\n",  __PRETTY_FUNCTION__, filename);
    if (s_opener_function == NULL)
	{
	    // Don't even have a way to open the file.
	    log_error("error: no file opener function; can't create movie.  "
		      "See gnash::register_file_opener_callback\n");
	    return NULL;
	}

    tu_file* in = s_opener_function(filename);
    if (in == NULL)
	{
	    log_error("failed to open '%s'; can't create movie.\n", filename);
	    return NULL;
	}
    else if (in->get_error())
	{
	    log_error("error: file opener can't open '%s'\n", filename);
	    return NULL;
	}

    ensure_loaders_registered();

    movie_def_impl*	m = new movie_def_impl(DO_LOAD_BITMAPS, DO_LOAD_FONT_SHAPES);
    m->read(in);

    delete in;

    if (m && s_use_cache_files)
	{
	    // Try to load a .gsc file.
	    tu_string	cache_filename(filename);
	    cache_filename += ".gsc";
	    tu_file*	cache_in = s_opener_function(cache_filename.c_str());
	    if (cache_in == NULL
		|| cache_in->get_error() != TU_FILE_NO_ERROR)
		{
		    // Can't open cache file; don't sweat it.
		    IF_VERBOSE_PARSE(log_msg("note: couldn't open cache file '%s'\n", cache_filename.c_str()));

		    m->generate_font_bitmaps();	// can't read cache, so generate font texture data.
		}
	    else
		{
		    // Load the cached data.
		    m->input_cached_data(cache_in);
		}

	    delete cache_in;
	}

    m->add_ref();
    return m;
}


static bool	s_no_recurse_while_loading = false;	// @@ TODO get rid of this; make it the normal mode.


movie_definition*	create_movie_no_recurse(
    tu_file* in,
    create_bitmaps_flag cbf,
    create_font_shapes_flag cfs)
{
    ensure_loaders_registered();

    // @@ TODO make no_recurse the standard way to load.
    // In create_movie(), use the visit_ API to keep
    // visiting dependent movies, until everything is
    // loaded.  That way we only have one code path and
    // the resource_proxy stuff gets tested.
    s_no_recurse_while_loading = true;

    movie_def_impl*	m = new movie_def_impl(cbf, cfs);
    m->read(in);

    s_no_recurse_while_loading = false;

    m->add_ref();
    return m;
}


//
// global gnash management
//



void	clear()
    // Maximum release of resources.
{
    clear_library();
    fontlib::clear();
    action_clear();
}


//
// library stuff, for sharing resources among different movies.
//


static stringi_hash< smart_ptr<movie_definition> >	s_movie_library;
static hash< movie_definition*, smart_ptr<movie_interface> >	s_movie_library_inst;
static std::vector<movie_interface*> s_extern_sprites;
static movie_interface* s_current_root;

static tu_string s_workdir;

void save_extern_movie(movie_interface* m)
{
    s_extern_sprites.push_back(m);
}

//#if 0
void delete_unused_root()
{
    for (unsigned int i = 0; i < s_extern_sprites.size(); i++)
	{
	    movie_interface* root_m = s_extern_sprites[i];
	    movie* m = root_m->get_root_movie();
      
	    if (m->get_ref_count() < 2)
		{
		    IF_VERBOSE_ACTION(log_msg("extern movie deleted\n"));
		    s_extern_sprites.erase(s_extern_sprites.begin() + i);
		    i--;
		    root_m->drop_ref();
		}
	}
}
//#endif // 0

movie_interface* get_current_root()
{
    assert(s_current_root != NULL);
    return s_current_root;
}

void set_current_root(movie_interface* m)
{
    assert(m != NULL);
    s_current_root = m;
}

const char* get_workdir()
{
    return s_workdir.c_str();
}

void set_workdir(const char* dir)
{
    assert(dir != NULL);
    s_workdir = dir;
}

void	clear_library()
    // Drop all library references to movie_definitions, so they
    // can be cleaned up.
{
    s_movie_library.clear();
    s_movie_library_inst.clear();
}

// Try to load a movie from the given url, if we haven't
// loaded it already.  Add it to our library on success, and
// return a pointer to it.
movie_definition* create_library_movie(const char* filename)
{
    tu_string	fn(filename);

    //log_msg("%s: filename is %s\n", __PRETTY_FUNCTION__, filename);

    // Is the movie already in the library?
    {
	smart_ptr<movie_definition>	m;
	s_movie_library.get(fn, &m);
	if (m != NULL)
	    {
    		log_msg(" movie already in library\n");
		// Return cached movie.
		m->add_ref();
		return m.get_ptr();
	    }
    }

    // Try to open a file under the filename.
    movie_definition* mov = create_movie(filename);

    if (mov == NULL)
	{
	    log_error("error: couldn't load library movie '%s'\n", filename);
	    return NULL;
	}
    else
	{
	    s_movie_library.add(fn, mov);
	}

    mov->add_ref();
    return mov;
}

movie_interface* create_library_movie_inst(movie_definition* md)
{
    // Is the movie instance already in the library?
    {
	smart_ptr<movie_interface>	m;
	s_movie_library_inst.get(md, &m);
	if (m != NULL)
	    {
		// Return cached movie instance.
		m->add_ref();
		return m.get_ptr();
	    }
    }

    // Try to create movie interface
    movie_interface* mov = md->create_instance();

    if (mov == NULL)
	{
	    log_error("error: couldn't create instance\n");

	    return NULL;
	}
    else
	{
	    s_movie_library_inst.add(md, mov);
	}

    mov->add_ref();
    return mov;
}


void	precompute_cached_data(movie_definition* movie_def)
    // Fill in cached data in movie_def.
    // @@@@ NEEDS TESTING -- MIGHT BE BROKEN!!!
{
    assert(movie_def != NULL);

    // Temporarily install null render and sound handlers,
    // so we don't get output during preprocessing.
    //
    // Use automatic struct var to make sure we restore
    // when exiting the function.
    struct save_stuff
    {
	render_handler*	m_original_rh;
	sound_handler*	m_original_sh;

	save_stuff()
	    {
		// Save.
		m_original_rh = get_render_handler();
		m_original_sh = get_sound_handler();
		set_render_handler(NULL);
		set_sound_handler(NULL);
	    }

	~save_stuff()
	    {
		// Restore.
		set_render_handler(m_original_rh);
		set_sound_handler(m_original_sh);
	    }
    } save_stuff_instance;

    // Need an instance.
    gnash::movie_interface*	m = movie_def->create_instance();
    if (m == NULL)
	{
	    log_error("error: precompute_cached_data can't create instance of movie\n");
	    return;
	}
		
    // Run through the movie's frames.
    //
    // @@ there might be cleaner ways to do this; e.g. do
    // execute_frame_tags(i, true) on movie and all child
    // sprites.
    int	kick_count = 0;
    for (;;)
	{
	    // @@ do we also have to run through all sprite frames
	    // as well?
	    //
	    // @@ also, ActionScript can rescale things
	    // dynamically -- we can't really do much about that I
	    // guess?
	    //
	    // @@ Maybe we should allow the user to specify some
	    // safety margin on scaled shapes.

	    int	last_frame = m->get_current_frame();
	    m->advance(0.010f);
	    m->display();

	    if (m->get_current_frame() == movie_def->get_frame_count() - 1)
		{
		    // Done.
		    break;
		}

	    if (m->get_play_state() == gnash::movie_interface::STOP)
		{
		    // Kick the movie.
		    //printf("kicking movie, kick ct = %d\n", kick_count);
		    m->goto_frame(last_frame + 1);
		    m->set_play_state(gnash::movie_interface::PLAY);
		    kick_count++;

		    if (kick_count > 10)
			{
			    //printf("movie is stalled; giving up on playing it through.\n");
			    break;
			}
		}
	    else if (m->get_current_frame() < last_frame)
		{
		    // Hm, apparently we looped back.  Skip ahead...
		    log_error("loop back; jumping to frame %d\n", last_frame);
		    m->goto_frame(last_frame + 1);
		}
	    else
		{
		    kick_count = 0;
		}
	}

    m->drop_ref();
}


//
// Some tag implementations
//


void	null_loader(stream* in, int tag_type, movie_definition* m)
    // Silently ignore the contents of this tag.
{
}

void	frame_label_loader(stream* in, int tag_type, movie_definition* m)
    // Label the current frame of m with the name from the stream.
{
    char*	n = in->read_string();
    m->add_frame_name(n);
    delete [] n;
}

/// SWF Tag SetBackgroundColor (9)
struct set_background_color : public execute_tag
{
    rgba	m_color;

    void	execute(movie* m)
	{
	    float	current_alpha = m->get_background_alpha();
	    m_color.m_a = frnd(current_alpha * 255.0f);
	    m->set_background_color(m_color);
	}

    void	execute_state(movie* m)
	{
	    execute(m);
	}

    void	read(stream* in)
	{
	    m_color.read_rgb(in);

	    IF_VERBOSE_PARSE(log_msg("  set_background_color: (%d %d %d)\n",
				     m_color.m_r, m_color.m_g, m_color.m_b));
	}
};


void	set_background_color_loader(stream* in, int tag_type, movie_definition* m)
{
    assert(tag_type == 9);
    assert(m);

    set_background_color*	t = new set_background_color;
    t->read(in);

    m->add_execute_tag(t);
}

#if 0
// Bitmap character
struct bitmap_character : public bitmap_character_def
{
    bitmap_character(bitmap_info* bi)
	:
	m_bitmap_info(bi)
	{
	}

// 		bitmap_character(image::rgb* image)
// 		{
// 			assert(image != 0);

// 			// Create our bitmap info, from our image.
// 			m_bitmap_info = gnash::render::create_bitmap_info_rgb(image);
// 		}

// 		bitmap_character(image::rgba* image)
// 		{
// 			assert(image != 0);

// 			// Create our bitmap info, from our image.
// 			m_bitmap_info = gnash::render::create_bitmap_info_rgba(image);
// 		}

    gnash::bitmap_info*	get_bitmap_info()
	{
	    return m_bitmap_info.get_ptr();
	}

private:
    smart_ptr<gnash::bitmap_info>	m_bitmap_info;
};
#endif

void	jpeg_tables_loader(stream* in, int tag_type, movie_definition* m)
    // Load JPEG compression tables that can be used to load
    // images further along in the stream.
{
    assert(tag_type == 8);

#if TU_CONFIG_LINK_TO_JPEGLIB
    jpeg::input*	j_in = jpeg::input::create_swf_jpeg2_header_only(in->get_underlying_stream());
    assert(j_in);

    m->set_jpeg_loader(j_in);
#endif // TU_CONFIG_LINK_TO_JPEGLIB
}


void	define_bits_jpeg_loader(stream* in, int tag_type, movie_definition* m)
    // A JPEG image without included tables; those should be in an
    // existing jpeg::input object stored in the movie.
{
    assert(tag_type == 6);

    Uint16	character_id = in->read_u16();

    //
    // Read the image data.
    //
    bitmap_info*	bi = NULL;

    if (m->get_create_bitmaps() == DO_LOAD_BITMAPS)
	{
#if TU_CONFIG_LINK_TO_JPEGLIB
	    jpeg::input*	j_in = m->get_jpeg_loader();
	    assert(j_in);
	    j_in->discard_partial_buffer();

	    image::rgb*	im = image::read_swf_jpeg2_with_tables(j_in);
	    bi = render::create_bitmap_info_rgb(im);
	    delete im;
#else
	    log_error("gnash is not linked to jpeglib -- can't load jpeg image data!\n");
	    bi = render::create_bitmap_info_empty();
#endif
	}
    else
	{
	    bi = render::create_bitmap_info_empty();
	}

    assert(bi->get_ref_count() == 0);

    bitmap_character*	ch = new bitmap_character(bi);

    m->add_bitmap_character(character_id, ch);
}


void	define_bits_jpeg2_loader(stream* in, int tag_type, movie_definition* m)
{
    assert(tag_type == 21);
		
    Uint16	character_id = in->read_u16();

    IF_VERBOSE_PARSE(log_msg("  define_bits_jpeg2_loader: charid = %d pos = 0x%x\n", character_id, in->get_position()));

    //
    // Read the image data.
    //
		
    bitmap_info*	bi = NULL;

    if (m->get_create_bitmaps() == DO_LOAD_BITMAPS)
	{
#if TU_CONFIG_LINK_TO_JPEGLIB
	    image::rgb* im = image::read_jpeg(in->get_underlying_stream());
	    bi = render::create_bitmap_info_rgb(im);
	    delete im;
#else
	    log_error("gnash is not linked to jpeglib -- can't load jpeg image data!\n");
	    bi = render::create_bitmap_info_empty();
#endif
	}
    else
	{
	    bi = render::create_bitmap_info_empty();
	}

    assert(bi->get_ref_count() == 0);

    bitmap_character*	ch = new bitmap_character(bi);

    m->add_bitmap_character(character_id, ch);
}


#if TU_CONFIG_LINK_TO_ZLIB
void	inflate_wrapper(tu_file* in, void* buffer, int buffer_bytes)
    // Wrapper function -- uses Zlib to uncompress in_bytes worth
    // of data from the input file into buffer_bytes worth of data
    // into *buffer.
{
    assert(in);
    assert(buffer);
    assert(buffer_bytes > 0);

    int err;
    z_stream d_stream; /* decompression stream */

    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = 0;
    d_stream.avail_in = 0;

    d_stream.next_out = (Byte*) buffer;
    d_stream.avail_out = (uInt) buffer_bytes;

    err = inflateInit(&d_stream);
    if (err != Z_OK) {
	log_error("error: inflate_wrapper() inflateInit() returned %d\n", err);
	return;
    }

    Uint8	buf[1];

    for (;;) {
	// Fill a one-byte (!) buffer.
	buf[0] = in->read_byte();
	d_stream.next_in = &buf[0];
	d_stream.avail_in = 1;

	err = inflate(&d_stream, Z_SYNC_FLUSH);
	if (err == Z_STREAM_END) break;
	if (err != Z_OK)
	    {
		log_error("error: inflate_wrapper() inflate() returned %d\n", err);
	    }
    }

    err = inflateEnd(&d_stream);
    if (err != Z_OK)
	{
	    log_error("error: inflate_wrapper() inflateEnd() return %d\n", err);
	}
}
#endif // TU_CONFIG_LINK_TO_ZLIB


void	define_bits_jpeg3_loader(stream* in, int tag_type, movie_definition* m)
    // loads a define_bits_jpeg3 tag. This is a jpeg file with an alpha
    // channel using zlib compression.
{
    assert(tag_type == 35);

    Uint16	character_id = in->read_u16();

    IF_VERBOSE_PARSE(log_msg("  define_bits_jpeg3_loader: charid = %d pos = 0x%x\n", character_id, in->get_position()));

    Uint32	jpeg_size = in->read_u32();
    Uint32	alpha_position = in->get_position() + jpeg_size;

    bitmap_info*	bi = NULL;

    if (m->get_create_bitmaps() == DO_LOAD_BITMAPS)
	{
#if TU_CONFIG_LINK_TO_JPEGLIB == 0 || TU_CONFIG_LINK_TO_ZLIB == 0
	    log_error("gnash is not linked to jpeglib/zlib -- can't load jpeg/zipped image data!\n");
	    bi = render::create_bitmap_info_empty();
#else
	    //
	    // Read the image data.
	    //
		
	    // Read rgb data.
	    image::rgba*	im = image::read_swf_jpeg3(in->get_underlying_stream());

	    // Read alpha channel.
	    in->set_position(alpha_position);

	    int	buffer_bytes = im->m_width * im->m_height;
	    Uint8*	buffer = new Uint8[buffer_bytes];

	    inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);

	    for (int i = 0; i < buffer_bytes; i++)
		{
		    im->m_data[4*i+3] = buffer[i];
		}

	    delete [] buffer;

	    bi = render::create_bitmap_info_rgba(im);

	    delete im;
#endif

	}
    else
	{
	    bi = render::create_bitmap_info_empty();
	}

    // Create bitmap character.
    bitmap_character*	ch = new bitmap_character(bi);

    m->add_bitmap_character(character_id, ch);
}


void	define_bits_lossless_2_loader(stream* in, int tag_type, movie_definition* m)
{
    assert(tag_type == 20 || tag_type == 36);

    Uint16	character_id = in->read_u16();
    Uint8	bitmap_format = in->read_u8();	// 3 == 8 bit, 4 == 16 bit, 5 == 32 bit
    Uint16	width = in->read_u16();
    Uint16	height = in->read_u16();

    IF_VERBOSE_PARSE(log_msg("  defbitslossless2: tag_type = %d, id = %d, fmt = %d, w = %d, h = %d\n",
			     tag_type,
			     character_id,
			     bitmap_format,
			     width,
			     height));

    bitmap_info*	bi = NULL;
    if (m->get_create_bitmaps() == DO_LOAD_BITMAPS)
	{
#if TU_CONFIG_LINK_TO_ZLIB == 0
	    log_error("gnash is not linked to zlib -- can't load zipped image data!\n");
	    return;
#else
	    if (tag_type == 20)
		{
		    // RGB image data.
		    image::rgb*	image = image::create_rgb(width, height);

		    if (bitmap_format == 3)
			{
			    // 8-bit data, preceded by a palette.

			    const int	bytes_per_pixel = 1;
			    int	color_table_size = in->read_u8();
			    color_table_size += 1;	// !! SWF stores one less than the actual size

			    int	pitch = (width * bytes_per_pixel + 3) & ~3;

			    int	buffer_bytes = color_table_size * 3 + pitch * height;
			    Uint8*	buffer = new Uint8[buffer_bytes];

			    inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);
			    assert(in->get_position() <= in->get_tag_end_position());

			    Uint8*	color_table = buffer;

			    for (int j = 0; j < height; j++)
				{
				    Uint8*	image_in_row = buffer + color_table_size * 3 + j * pitch;
				    Uint8*	image_out_row = image::scanline(image, j);
				    for (int i = 0; i < width; i++)
					{
					    Uint8	pixel = image_in_row[i * bytes_per_pixel];
					    image_out_row[i * 3 + 0] = color_table[pixel * 3 + 0];
					    image_out_row[i * 3 + 1] = color_table[pixel * 3 + 1];
					    image_out_row[i * 3 + 2] = color_table[pixel * 3 + 2];
					}
				}

			    delete [] buffer;
			}
		    else if (bitmap_format == 4)
			{
			    // 16 bits / pixel
			    const int	bytes_per_pixel = 2;
			    int	pitch = (width * bytes_per_pixel + 3) & ~3;

			    int	buffer_bytes = pitch * height;
			    Uint8*	buffer = new Uint8[buffer_bytes];

			    inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);
			    assert(in->get_position() <= in->get_tag_end_position());
			
			    for (int j = 0; j < height; j++)
				{
				    Uint8*	image_in_row = buffer + j * pitch;
				    Uint8*	image_out_row = image::scanline(image, j);
				    for (int i = 0; i < width; i++)
					{
					    Uint16	pixel = image_in_row[i * 2] | (image_in_row[i * 2 + 1] << 8);
					
					    // @@ How is the data packed???  I'm just guessing here that it's 565!
					    image_out_row[i * 3 + 0] = (pixel >> 8) & 0xF8;	// red
					    image_out_row[i * 3 + 1] = (pixel >> 3) & 0xFC;	// green
					    image_out_row[i * 3 + 2] = (pixel << 3) & 0xF8;	// blue
					}
				}
			
			    delete [] buffer;
			}
		    else if (bitmap_format == 5)
			{
			    // 32 bits / pixel, input is ARGB format (???)
			    const int	bytes_per_pixel = 4;
			    int	pitch = width * bytes_per_pixel;

			    int	buffer_bytes = pitch * height;
			    Uint8*	buffer = new Uint8[buffer_bytes];

			    inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);
			    assert(in->get_position() <= in->get_tag_end_position());
			
			    // Need to re-arrange ARGB into RGB.
			    for (int j = 0; j < height; j++)
				{
				    Uint8*	image_in_row = buffer + j * pitch;
				    Uint8*	image_out_row = image::scanline(image, j);
				    for (int i = 0; i < width; i++)
					{
					    Uint8	a = image_in_row[i * 4 + 0];
					    Uint8	r = image_in_row[i * 4 + 1];
					    Uint8	g = image_in_row[i * 4 + 2];
					    Uint8	b = image_in_row[i * 4 + 3];
					    image_out_row[i * 3 + 0] = r;
					    image_out_row[i * 3 + 1] = g;
					    image_out_row[i * 3 + 2] = b;
					    a = a;	// Inhibit warning.
					}
				}

			    delete [] buffer;
			}

//				bitmap_character*	ch = new bitmap_character(image);
		    bi = render::create_bitmap_info_rgb(image);
		    delete image;

// 				// add image to movie, under character id.
// 				m->add_bitmap_character(character_id, ch);
		}
	    else
		{
		    // RGBA image data.
		    assert(tag_type == 36);

		    image::rgba*	image = image::create_rgba(width, height);

		    if (bitmap_format == 3)
			{
			    // 8-bit data, preceded by a palette.

			    const int	bytes_per_pixel = 1;
			    int	color_table_size = in->read_u8();
			    color_table_size += 1;	// !! SWF stores one less than the actual size

			    int	pitch = (width * bytes_per_pixel + 3) & ~3;

			    int	buffer_bytes = color_table_size * 4 + pitch * height;
			    Uint8*	buffer = new Uint8[buffer_bytes];

			    inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);
			    assert(in->get_position() <= in->get_tag_end_position());

			    Uint8*	color_table = buffer;

			    for (int j = 0; j < height; j++)
				{
				    Uint8*	image_in_row = buffer + color_table_size * 4 + j * pitch;
				    Uint8*	image_out_row = image::scanline(image, j);
				    for (int i = 0; i < width; i++)
					{
					    Uint8	pixel = image_in_row[i * bytes_per_pixel];
					    image_out_row[i * 4 + 0] = color_table[pixel * 4 + 0];
					    image_out_row[i * 4 + 1] = color_table[pixel * 4 + 1];
					    image_out_row[i * 4 + 2] = color_table[pixel * 4 + 2];
					    image_out_row[i * 4 + 3] = color_table[pixel * 4 + 3];
					}
				}

			    delete [] buffer;
			}
		    else if (bitmap_format == 4)
			{
			    // 16 bits / pixel
			    const int	bytes_per_pixel = 2;
			    int	pitch = (width * bytes_per_pixel + 3) & ~3;

			    int	buffer_bytes = pitch * height;
			    Uint8*	buffer = new Uint8[buffer_bytes];

			    inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);
			    assert(in->get_position() <= in->get_tag_end_position());
			
			    for (int j = 0; j < height; j++)
				{
				    Uint8*	image_in_row = buffer + j * pitch;
				    Uint8*	image_out_row = image::scanline(image, j);
				    for (int i = 0; i < width; i++)
					{
					    Uint16	pixel = image_in_row[i * 2] | (image_in_row[i * 2 + 1] << 8);
					
					    // @@ How is the data packed???  I'm just guessing here that it's 565!
					    image_out_row[i * 4 + 0] = 255;			// alpha
					    image_out_row[i * 4 + 1] = (pixel >> 8) & 0xF8;	// red
					    image_out_row[i * 4 + 2] = (pixel >> 3) & 0xFC;	// green
					    image_out_row[i * 4 + 3] = (pixel << 3) & 0xF8;	// blue
					}
				}
			
			    delete [] buffer;
			}
		    else if (bitmap_format == 5)
			{
			    // 32 bits / pixel, input is ARGB format

			    inflate_wrapper(in->get_underlying_stream(), image->m_data, width * height * 4);
			    assert(in->get_position() <= in->get_tag_end_position());
			
			    // Need to re-arrange ARGB into RGBA.
			    for (int j = 0; j < height; j++)
				{
				    Uint8*	image_row = image::scanline(image, j);
				    for (int i = 0; i < width; i++)
					{
					    Uint8	a = image_row[i * 4 + 0];
					    Uint8	r = image_row[i * 4 + 1];
					    Uint8	g = image_row[i * 4 + 2];
					    Uint8	b = image_row[i * 4 + 3];
					    image_row[i * 4 + 0] = r;
					    image_row[i * 4 + 1] = g;
					    image_row[i * 4 + 2] = b;
					    image_row[i * 4 + 3] = a;
					}
				}
			}

		    bi = render::create_bitmap_info_rgba(image);
//				bitmap_character*	ch = new bitmap_character(image);
		    delete image;

//	 			// add image to movie, under character id.
//	 			m->add_bitmap_character(character_id, ch);
		}
#endif // TU_CONFIG_LINK_TO_ZLIB
	}
    else
	{
	    bi = render::create_bitmap_info_empty();
	}
    assert(bi->get_ref_count() == 0);

    bitmap_character*	ch = new bitmap_character(bi);

    // add image to movie, under character id.
    m->add_bitmap_character(character_id, ch);
}

// This is like null_loader except it prints a message to nag us to fix it.
void
fixme_loader(stream* in, int tag_type, movie_definition* m)
{
    IF_VERBOSE_PARSE(log_msg("  FIXME: tagtype = %d\n", tag_type));
}

void	define_shape_loader(stream* in, int tag_type, movie_definition* m)
{
    assert(tag_type == 2
	   || tag_type == 22
	   || tag_type == 32);

    Uint16	character_id = in->read_u16();
    IF_VERBOSE_PARSE(log_msg("  shape_loader: id = %d\n", character_id));

    shape_character_def*	ch = new shape_character_def;
    ch->read(in, tag_type, true, m);

    IF_VERBOSE_PARSE(log_msg("  bound rect:"); ch->get_bound().print());

    m->add_character(character_id, ch);
}

void define_shape_morph_loader(stream* in, int tag_type, movie_definition* m)
{
    assert(tag_type == 46);
    Uint16 character_id = in->read_u16();
    IF_VERBOSE_PARSE(log_msg("  shape_morph_loader: id = %d\n", character_id));
    morph2_character_def* morph = new morph2_character_def;
    morph->read(in, tag_type, true, m);
    m->add_character(character_id, morph);
}

//
// font loaders
//


void	define_font_loader(stream* in, int tag_type, movie_definition* m)
    // Load a DefineFont or DefineFont2 tag.
{
    assert(tag_type == 10 || tag_type == 48);

    Uint16	font_id = in->read_u16();
		
    font*	f = new font;
    f->read(in, tag_type, m);

    m->add_font(font_id, f);

    // Automatically keeping fonts in fontlib is
    // problematic.  The app should be responsible for
    // optionally adding fonts to fontlib.
    // //fontlib::add_font(f);
}


void	define_font_info_loader(stream* in, int tag_type, movie_definition* m)
    // Load a DefineFontInfo tag.  This adds information to an
    // existing font.
{
    assert(tag_type == 13);

    Uint16	font_id = in->read_u16();
		
    font*	f = m->get_font(font_id);
    if (f)
	{
	    f->read_font_info(in);
	}
    else
	{
	    log_error("define_font_info_loader: can't find font w/ id %d\n", font_id);
	}
}

void swf_event::read(stream* in, Uint32 flags)
{
    assert(flags != 0);

    // Scream if more than one bit is set, since we're not set up to handle
    // that, and it doesn't seem possible to express in ActionScript source,
    // so it's important to know if this ever occurs in the wild.
    if (flags & (flags - 1))
	{
	    log_error("error: swf_event::read() -- more than one event type encoded!  "
		      "unexpected! flags = 0x%x\n", flags);
	}

    // 14 bits reserved, 18 bits used

    static const event_id	s_code_bits[18] =
	{
	    event_id::LOAD,
	    event_id::ENTER_FRAME,
	    event_id::UNLOAD,
	    event_id::MOUSE_MOVE,
	    event_id::MOUSE_DOWN,
	    event_id::MOUSE_UP,
	    event_id::KEY_DOWN,
	    event_id::KEY_UP,
	    event_id::DATA,
	    event_id::INITIALIZE,
	    event_id::PRESS,
	    event_id::RELEASE,
	    event_id::RELEASE_OUTSIDE,
	    event_id::ROLL_OVER,
	    event_id::ROLL_OUT,
	    event_id::DRAG_OVER,
	    event_id::DRAG_OUT,
	};

    // Let's see if the event flag we received is for an event that we know of
    if ((pow(2,int(sizeof(s_code_bits)/sizeof(s_code_bits[0])))-1) < flags)
	{
	    log_error("error: swf_event::read() -- unknown / unhandled event type received, flags = 0x%x\n", flags);
	}

    for (int i = 0, mask = 1; i < int(sizeof(s_code_bits)/sizeof(s_code_bits[0])); i++, mask <<= 1)
	{
	    if (flags & mask)
		{
		    m_event = s_code_bits[i];
		    break;
		}
	}

    // what to do w/ key_press???  Is the data in the reserved parts of the flags???
    if (flags & (1 << 17))
	{
	    log_error("swf_event::read -- KEY_PRESS found, not handled yet, flags = 0x%x\n", flags);
	}

    Uint32	event_length = in->read_u32();
    UNUSED(event_length);

    // Read the actions.
    IF_VERBOSE_ACTION(log_msg("---- actions for event %s\n", m_event.get_function_name().c_str()));
    m_action_buffer.read(in);

    if (m_action_buffer.get_length() != (int) event_length)
	{
	    log_error("error -- swf_event::read(), event_length = %d, but read %d\n",
		      event_length,
		      m_action_buffer.get_length());
	    // @@ discard this event handler??
	}

    // Create a function to execute the actions.
    std::vector<with_stack_entry>	empty_with_stack;
    function_as_object*	func = new function_as_object(&m_action_buffer, NULL, 0, empty_with_stack);
    func->set_length(m_action_buffer.get_length());

    m_method.set_function_as_object(func);
}



/// SWF Tag PlaceObject2 (9) 
struct place_object_2 : public execute_tag
{
    int	m_tag_type;
    char*	m_name;
    float	m_ratio;
    cxform	m_color_transform;
    matrix	m_matrix;
    bool	m_has_matrix;
    bool	m_has_cxform;
    Uint16	m_depth;
    Uint16	m_character_id;
    Uint16 	m_clip_depth;
    enum place_type {
	PLACE,
	MOVE,
	REPLACE,
    } m_place_type;
    std::vector<swf_event*>	m_event_handlers;


    place_object_2()
	:
	m_tag_type(0),
	m_name(NULL),
	m_ratio(0),
	m_has_matrix(false),
	m_has_cxform(false),
	m_depth(0),
	m_character_id(0),
	m_clip_depth(0),
	m_place_type(PLACE)
	{
	}

    ~place_object_2()
	{
	    delete [] m_name;
	    m_name = NULL;

	    for (int i = 0, n = m_event_handlers.size(); i < n; i++)
		{
		    delete m_event_handlers[i];
		}
	    m_event_handlers.resize(0);
	}

    void	read(stream* in, int tag_type, int movie_version)
	{
	    assert(tag_type == 4 || tag_type == 26);

	    m_tag_type = tag_type;

	    if (tag_type == 4)
		{
		    // Original place_object tag; very simple.
		    m_character_id = in->read_u16();
		    m_depth = in->read_u16();
		    m_matrix.read(in);

		    IF_VERBOSE_PARSE(
			log_msg("  char_id = %d\n"
				"  depth = %d\n"
				"  mat = \n",
				m_character_id,
				m_depth);
			m_matrix.print());

		    if (in->get_position() < in->get_tag_end_position())
			{
			    m_color_transform.read_rgb(in);
			    IF_VERBOSE_PARSE(log_msg("  cxform:\n"); m_color_transform.print());
			}
		}
	    else if (tag_type == 26)
		{
		    in->align();

		    bool	has_actions = in->read_uint(1) ? true : false;
		    bool	has_clip_bracket = in->read_uint(1) ? true : false;
		    bool	has_name = in->read_uint(1) ? true : false;
		    bool	has_ratio = in->read_uint(1) ? true : false;
		    bool	has_cxform = in->read_uint(1) ? true : false;
		    bool	has_matrix = in->read_uint(1) ? true : false;
		    bool	has_char = in->read_uint(1) ? true : false;
		    bool	flag_move = in->read_uint(1) ? true : false;

		    m_depth = in->read_u16();
		    IF_VERBOSE_PARSE(log_msg("  depth = %d\n", m_depth));

		    if (has_char) {
			m_character_id = in->read_u16();
			IF_VERBOSE_PARSE(log_msg("  char id = %d\n", m_character_id));
		    }

		    if (has_matrix) {
			m_has_matrix = true;
			m_matrix.read(in);
			IF_VERBOSE_PARSE(log_msg("  mat:\n"); m_matrix.print());
		    }
		    if (has_cxform) {
			m_has_cxform = true;
			m_color_transform.read_rgba(in);
			IF_VERBOSE_PARSE(log_msg("  cxform:\n"); m_color_transform.print());
		    }
				
		    if (has_ratio) {
			m_ratio = (float)in->read_u16() / (float)65535;
			IF_VERBOSE_PARSE(log_msg("  ratio: %f\n", m_ratio));
		    }
				
		    if (has_name) {
			m_name = in->read_string();
			IF_VERBOSE_PARSE(log_msg("  name = %s\n", m_name ? m_name : "<null>"));
		    }
		    if (has_clip_bracket) {
			m_clip_depth = in->read_u16(); 
			IF_VERBOSE_PARSE(log_msg("  clip_depth = %d\n", m_clip_depth));
		    }
		    if (has_actions)
			{
			    Uint16	reserved = in->read_u16();
			    UNUSED(reserved);

			    // The logical 'or' of all the following handlers.
			    // I don't think we care about this...
			    Uint32	all_flags = 0;
			    if (movie_version >= 6)
				{
				    all_flags = in->read_u32();
				}
			    else
				{
				    all_flags = in->read_u16();
				}
			    UNUSED(all_flags);

			    IF_VERBOSE_PARSE(log_msg("  actions: flags = 0x%X\n", all_flags));

			    // Read swf_events.
			    for (;;)
				{
				    // Read event.
				    in->align();

				    Uint32	this_flags = 0;
				    if (movie_version >= 6)
					{
					    this_flags = in->read_u32();
					}
				    else
					{
					    this_flags = in->read_u16();
					}

				    if (this_flags == 0)
					{
					    // Done with events.
					    break;
					}

				    swf_event*	ev = new swf_event;
				    ev->read(in, this_flags);

				    m_event_handlers.push_back(ev);
				}
			}


		    if (has_char == true && flag_move == true)
			{
			    // Remove whatever's at m_depth, and put m_character there.
			    m_place_type = REPLACE;
			}
		    else if (has_char == false && flag_move == true)
			{
			    // Moves the object at m_depth to the new location.
			    m_place_type = MOVE;
			}
		    else if (has_char == true && flag_move == false)
			{
			    // Put m_character at m_depth.
			    m_place_type = PLACE;
			}
                                
		    //log_msg("place object at depth %i\n", m_depth);
		}
	}

		
    void	execute(movie* m)
	// Place/move/whatever our object in the given movie.
	{
	    switch (m_place_type) {
	      case PLACE:
		  m->add_display_object(
		      m_character_id,
		      m_name,
		      m_event_handlers,
		      m_depth,
		      m_tag_type != 4,	// original place_object doesn't do replacement
		      m_color_transform,
		      m_matrix,
		      m_ratio,
		      m_clip_depth);
		  break;
		  
	      case MOVE:
		  m->move_display_object(
		      m_depth,
		      m_has_cxform,
		      m_color_transform,
		      m_has_matrix,
		      m_matrix,
		      m_ratio,
		      m_clip_depth);
		  break;
		  
	      case REPLACE:
		  m->replace_display_object(
		      m_character_id,
		      m_name,
		      m_depth,
		      m_has_cxform,
		      m_color_transform,
		      m_has_matrix,
		      m_matrix,
		      m_ratio,
		      m_clip_depth);
		  break;
	    }
	}
    
    void	execute_state(movie* m)
	{
	    execute(m);
	}
    
    void	execute_state_reverse(movie* m, int frame)
	{
	    switch (m_place_type) {
	      case PLACE:
		  // reverse of add is remove
		  m->remove_display_object(m_depth, m_tag_type == 4 ? m_character_id : -1);
		  break;
		  
	      case MOVE:
		  // reverse of move is move
		  m->move_display_object(
		      m_depth,
		      m_has_cxform,
		      m_color_transform,
		      m_has_matrix,
		      m_matrix,
		      m_ratio,
		      m_clip_depth);
		  break;
		  
	      case REPLACE:
	      {
		  // reverse of replace is to re-add the previous object.
		  execute_tag*	last_add = m->find_previous_replace_or_add_tag(frame, m_depth, -1);
		  if (last_add) {
		      last_add->execute_state(m);
		  } else {
		      log_error("reverse REPLACE can't find previous replace or add tag(%d, %d)\n",
				frame, m_depth);
		      
		  }
		  break;
	      }
	    }
	}
    
    virtual uint32	get_depth_id_of_replace_or_add_tag() const
	// "depth_id" is the 16-bit depth & id packed into one 32-bit int.
	{
	    if (m_place_type == PLACE || m_place_type == REPLACE)
		{
		    int	id = -1;
		    if (m_tag_type == 4)
			{
			    // Old-style PlaceObject; the corresponding Remove
			    // is specific to the character_id.
			    id = m_character_id;
			}
		    return ((m_depth & 0x0FFFF) << 16) | (id & 0x0FFFF);
		}
	    else
		{
		    return (uint32) -1;
		}
	}
};


	
void	place_object_2_loader(stream* in, int tag_type, movie_definition* m)
{
    assert(tag_type == 4 || tag_type == 26);

    IF_VERBOSE_PARSE(log_msg("  place_object_2\n"));

    place_object_2*	ch = new place_object_2;
    ch->read(in, tag_type, m->get_version());

    m->add_execute_tag(ch);
}







void	sprite_loader(stream* in, int tag_type, movie_definition* m)
    // Create and initialize a sprite, and add it to the movie.
{
    assert(tag_type == SWF::DEFINESPRITE); // 39 - DefineSprite
                
    int	character_id = in->read_u16();

    IF_VERBOSE_PARSE(log_msg("  sprite\n  char id = %d\n", character_id));

    sprite_definition*	ch = new sprite_definition(m);	// @@ combine sprite_definition with movie_def_impl
    ch->read(in);

    m->add_character(character_id, ch);
}



//
// end_tag
//

// end_tag doesn't actually need to exist.

void	end_loader(stream* in, int tag_type, movie_definition* m)
{
    assert(tag_type == 0);
    assert(in->get_position() == in->get_tag_end_position());
}


/// SWF Tag RemoveObject2 (28) 
struct remove_object_2 : public execute_tag
{
    int	m_depth, m_id;

    remove_object_2() : m_depth(-1), m_id(-1) {}

    void	read(stream* in, int tag_type)
	{
	    assert(tag_type == 5 || tag_type == 28);

	    if (tag_type == 5)
		{
		    // Older SWF's allow multiple objects at the same depth;
		    // this m_id disambiguates.  Later SWF's just use one
		    // object per depth.
		    m_id = in->read_u16();
		}
	    m_depth = in->read_u16();
	}

    virtual void	execute(movie* m)
	{
	    m->remove_display_object(m_depth, m_id);
	}

    virtual void	execute_state(movie* m)
	{
	    execute(m);
	}

    virtual void	execute_state_reverse(movie* m, int frame)
	{
	    // reverse of remove is to re-add the previous object.
	    execute_tag*	last_add = m->find_previous_replace_or_add_tag(frame, m_depth, m_id);
	    if (last_add)
		{
		    last_add->execute_state(m);
		}
	    else
		{
		    log_error("reverse REMOVE can't find previous replace or add tag(%d, %d)\n",
			      frame, m_depth);
					
		}
	}

    virtual bool	is_remove_tag() const { return true; }
};


void	remove_object_2_loader(stream* in, int tag_type, movie_definition* m)
{
    assert(tag_type == 5 || tag_type == 28);

    remove_object_2*	t = new remove_object_2;
    t->read(in, tag_type);

    IF_VERBOSE_PARSE(log_msg("  remove_object_2(%d)\n", t->m_depth));

    m->add_execute_tag(t);
}


void	button_sound_loader(stream* in, int tag_type, movie_definition* m)
{
    assert(tag_type == 17);

    int	button_character_id = in->read_u16();
    button_character_definition* ch = (button_character_definition*) m->get_character_def(button_character_id);
    assert(ch != NULL);

    ch->read(in, tag_type, m);
}


void	button_character_loader(stream* in, int tag_type, movie_definition* m)
{
    assert(tag_type == 7 || tag_type == 34);

    int	character_id = in->read_u16();

    IF_VERBOSE_PARSE(log_msg("  button character loader: char_id = %d\n", character_id));

    button_character_definition*	ch = new button_character_definition;
    ch->read(in, tag_type, m);

    m->add_character(character_id, ch);
}


//
// export
//


void	export_loader(stream* in, int tag_type, movie_definition* m)
    // Load an export tag (for exposing internal resources of m)
{
    assert(tag_type == 56);

    int	count = in->read_u16();

    IF_VERBOSE_PARSE(log_msg("  export: count = %d\n", count));

    // Read the exports.
    for (int i = 0; i < count; i++)
	{
	    Uint16	id = in->read_u16();
	    char*	symbol_name = in->read_string();
	    IF_VERBOSE_PARSE(log_msg("  export: id = %d, name = %s\n", id, symbol_name));

	    if (font* f = m->get_font(id))
		{
		    // Expose this font for export.
		    m->export_resource(tu_string(symbol_name), f);
		}
	    else if (character_def* ch = m->get_character_def(id))
		{
		    // Expose this movie/button/whatever for export.
		    m->export_resource(tu_string(symbol_name), ch);
		}
	    else if (sound_sample* ch = m->get_sound_sample(id))
		{
		    m->export_resource(tu_string(symbol_name), ch);
		}
	    else
		{
		    log_error("export error: don't know how to export resource '%s'\n",
			      symbol_name);
		}

	    delete [] symbol_name;
	}
}


//
// import
//


void	import_loader(stream* in, int tag_type, movie_definition* m)
    // Load an import tag (for pulling in external resources)
{
    assert(tag_type == 57);

    char*	source_url = in->read_string();
    int	count = in->read_u16();

    IF_VERBOSE_PARSE(log_msg("  import: source_url = %s, count = %d\n", source_url, count));

    // Try to load the source movie into the movie library.
    movie_definition*	source_movie = NULL;

    if (s_no_recurse_while_loading == false)
	{
	    source_movie = create_library_movie(source_url);
	    if (source_movie == NULL)
		{
		    // Give up on imports.
		    log_error("can't import movie from url %s\n", source_url);
		    return;
		}
	}

    // Get the imports.
    for (int i = 0; i < count; i++)
	{
	    Uint16	id = in->read_u16();
	    char*	symbol_name = in->read_string();
	    IF_VERBOSE_PARSE(log_msg("  import: id = %d, name = %s\n", id, symbol_name));

	    if (s_no_recurse_while_loading)
		{
		    m->add_import(source_url, id, symbol_name);
		}
	    else
		{
		    // @@ TODO get rid of this, always use
		    // s_no_recurse_while_loading, change
		    // create_movie().

		    smart_ptr<resource> res = source_movie->get_exported_resource(symbol_name);
		    if (res == NULL)
			{
			    log_error("import error: resource '%s' is not exported from movie '%s'\n",
				      symbol_name, source_url);
			}
		    else if (font* f = res->cast_to_font())
			{
			    // Add this shared font to the currently-loading movie.
			    m->add_font(id, f);
			}
		    else if (character_def* ch = res->cast_to_character_def())
			{
			    // Add this character to the loading movie.
			    m->add_character(id, ch);
			}
		    else
			{
			    log_error("import error: resource '%s' from movie '%s' has unknown type\n",
				      symbol_name, source_url);
			}
		}

	    delete [] symbol_name;
	}

    delete [] source_url;
}
}

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
