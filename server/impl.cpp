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
//#include "button.h"
#include "impl.h"
#include "font.h"
#include "fontlib.h"
#include "log.h"
//#include "morph2.h"
#include "render.h"
#include "shape.h"
//#include "stream.h"
#include "styles.h"
#include "dlist.h"
#include "timers.h"
#include "image.h"
#include "jpeg.h"
#include "zlib_adapter.h"
#include "sprite_definition.h"
#include "movie_def_impl.h"
#include "swf.h"
#include "swf/TagLoadersTable.h"
#include "swf/tag_loaders.h"
#include "generic_character.h"
#include "URL.h"
#include "StreamProvider.h"
#include "sprite_instance.h"

#include <string>
#include <map>
#include <memory> // for auto_ptr

namespace gnash
{


/// Namespace for global data (will likely turn into a class)
namespace globals { // gnash::globals

	/// global StreamProvider
	static StreamProvider streamProvider;

	/// Base url (for relative urls resolution)
	//
	/// we need an auto_ptr becase the URL class
	/// is an immutable one and needs to be set 
	/// at construction time..
	///
	std::auto_ptr<URL> baseurl;

} // namespace gnash::global

void
set_base_url(const URL& url)
{
	// can call this only once during a single run
	assert(!globals::baseurl.get());
	globals::baseurl.reset(new URL(url));
	log_msg("Base url set to: %s", globals::baseurl->str().c_str());
}

URL&
get_base_url()
{
	// Don't call me if you haven't set me !
	assert(globals::baseurl.get());
	return *globals::baseurl;
}


bool	s_verbose_action = false;
bool	s_verbose_parse = false;

#ifndef NDEBUG
bool	s_verbose_debug = true;
#else
bool	s_verbose_debug = false;
#endif

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
SWF::TagLoadersTable s_tag_loaders;

// Associate the specified tag type with the given tag loader
// function.
void
register_tag_loader(SWF::tag_type t, SWF::TagLoadersTable::loader_function lf)
{
	bool loader_registered = s_tag_loaders.register_loader(t, lf);
	//if ( !loader_registered )
	// log_error("Duplicate loader registered for tag %d", t);
	assert(loader_registered);
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


static void	ensure_loaders_registered()
{
	using namespace SWF::tag_loaders;

	static bool s_registered = false;
	
	if (s_registered) return;

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
	register_tag_loader(SWF::DEFINEFONTINFO2, define_font_info_loader);
	register_tag_loader(SWF::DEFINESOUND,	define_sound_loader);
	register_tag_loader(SWF::STARTSOUND,	start_sound_loader);
	// 16 _UNKNOWN_ unimplemented
	register_tag_loader(SWF::DEFINEBUTTONSOUND, button_sound_loader);
	register_tag_loader(SWF::SOUNDSTREAMHEAD, sound_stream_head_loader); // 18
	register_tag_loader(SWF::SOUNDSTREAMBLOCK, sound_stream_block_loader); // 19
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
	register_tag_loader(SWF::FRAMELABEL,	frame_label_loader); // 43
	// 44 - _UNKNOWN_ unimplemented
	register_tag_loader(SWF::SOUNDSTREAMHEAD2, sound_stream_head_loader); // 45
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
	register_tag_loader(SWF::INITACTION, do_init_action_loader);   
	register_tag_loader(SWF::DEFINEVIDEOSTREAM, fixme_loader); // 60

	register_tag_loader(SWF::VIDEOFRAME, fixme_loader); // 61

}



void	get_movie_info(
    const URL& url,
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
    //printf("%s: url is %s\n",  __PRETTY_FUNCTION__, url.str().c_str());

    tu_file*	in = globals::streamProvider.getStream(url);
    if (in == NULL || in->get_error() != TU_FILE_NO_ERROR) {
	log_error("get_movie_info(): can't open '%s'\n", url.str().c_str());
	if (version) *version = 0;
	delete in;
	return;
    }
    
    uint32_t	file_start_pos = in->get_position();
    uint32_t	header = in->read_le32();
    uint32_t	file_length = in->read_le32();
    uint32_t	file_end_pos = file_start_pos + file_length;
    
    int	local_version = (header >> 24) & 255;
    if ((header & 0x0FFFFFF) != 0x00535746
	&& (header & 0x0FFFFFF) != 0x00535743) {
	// ERROR
	log_error("get_movie_info(): file '%s' does not start with a SWF header!\n", url.str().c_str());
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
	    while ((uint32_t) str.get_position() < file_end_pos)
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

// Create a movie_definition from a jpeg stream
// NOTE: this method assumes this *is* a jpeg stream
static movie_definition*
create_jpeg_movie(tu_file* in, const char* /*url*/)
{
	// FIXME: temporarly disabled
	log_msg("Loading of jpegs unsupported");
	return NULL;


	bitmap_info* bi = NULL;
	image::rgb* im = image::read_jpeg(in);
	if (im != NULL) {
		bi = render::create_bitmap_info_rgb(im);
		delete im;
	} else {
		log_error("Can't read jpeg\n");
		return NULL;
	}

	// FIXME: create a movie_definition from a jpeg
	//bitmap_character*	 ch = new bitmap_character(bi);

}

// Get type of file looking at first bytes
// return 'jpeg', 'swf' or 'unknown'
//
static std::string
get_file_type(tu_file* in)
{
	in->set_position(0);

	unsigned char buf[5];
	memset(buf, 0, 5);
	if ( 4 < in->read_bytes(buf, 4) )
	{
		log_error("Can't read file header!\n");
		return "unknown";
	}
	
	// This is the magic number for any JPEG format file
	if ((buf[0] == 0xff) && (buf[1] == 0xd8) && (buf[2] == 0xff))
	{
		return "jpeg";
	}

	// This is for SWF (FWS or CWS)
	if (	(buf[0] == 'F' || buf[0] == 'C') &&
		(buf[1] == 'W') &&
		(buf[2] == 'S') )
	{
		return "swf";
	}

	return "unknown";
}

// Create a movie_definition from an SWF stream
// NOTE: this method assumes this *is* an SWF stream
static movie_definition*
create_swf_movie(tu_file* in, const char* url)
{

	in->set_position(0);

	movie_def_impl* m = new movie_def_impl(DO_LOAD_BITMAPS,
		DO_LOAD_FONT_SHAPES);
	if ( ! m->read(in, url) ) return NULL;

	return m;
}

movie_definition*
create_movie(const URL& url, const char* reset_url)
{
	// URL::str() returns by value, save it to a local string
	std::string url_str = url.str();
	const char* c_url = url_str.c_str();

//	printf("%s: url is %s\n",  __PRETTY_FUNCTION__, c_url);

	tu_file* in = globals::streamProvider.getStream(url);
	if (in == NULL)
	{
	    log_error("failed to open '%s'; can't create movie.\n", c_url);
	    return NULL;
	}
	else if (in->get_error())
	{
	    log_error("streamProvider opener can't open '%s'\n", c_url);
	    return NULL;
	}

	ensure_loaders_registered();

	// see if it's a jpeg or an swf
	std::string type = get_file_type(in);

	movie_definition* ret = NULL;

	if ( type == "jpeg" )
	{
		ret = create_jpeg_movie(in, reset_url?reset_url:c_url);
	}
	else if ( type == "swf" )
	{
		ret = create_swf_movie(in, reset_url?reset_url:c_url);
	}
	else
	{
		log_error("unknown file type\n");
		ret = NULL;
	}

	if ( ! ret )
	{
		delete in;
		return NULL;
	}

	ret->add_ref();

	if (s_use_cache_files)
	{
		// Try to load a .gsc file.
		// WILL NOT WORK FOR NETWORK URLS, would need an hash
		tu_string	cache_filename(c_url);
		cache_filename += ".gsc";
		tu_file* cache_in = new tu_file(cache_filename.c_str(), "rb");
		if (cache_in == NULL
			|| cache_in->get_error() != TU_FILE_NO_ERROR)
		{
			// Can't open cache file; don't sweat it.
		        log_parse("note: couldn't open cache file '%s'\n",
					cache_filename.c_str());

			// can't read cache, so generate font texture data.
			ret->generate_font_bitmaps();
		}
		else
		{
			log_msg("Loading cache file %s",
				cache_filename.c_str());
			// Load the cached data.
			ret->input_cached_data(cache_in);
		}

		delete cache_in;
	}

	return ret;
}


bool	s_no_recurse_while_loading = false;	// @@ TODO get rid of this; make it the normal mode.


#if 0 // This function seems unused
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
    if ( ! m->read(in) ) return NULL;

    s_no_recurse_while_loading = false;

    m->add_ref();
    return m;
}
#endif


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


//static stringi_hash< smart_ptr<movie_definition> >	s_movie_library;

/// Library of SWF movies indexed by URL strings
//
/// Elements are actually movie_def_impl, the ones
/// associated with URLS. Dunno why, but we were using
/// movie_definition here before so this didn't change
/// when the new class was introduced.
///
class MovieLibrary
{
private:

	typedef std::map< std::string, smart_ptr<movie_definition> > container;

	container _map;

public:

	MovieLibrary() {}

	bool get(const std::string& key, smart_ptr<movie_definition>* ret)
	{
		container::iterator it = _map.find(key);
		if ( it != _map.end() )
		{
			*ret = it->second;
			return true;
		}
		else
		{
			return false;
		}
	}

	void add(const std::string& key, movie_definition* mov)
	{
		_map[key] = mov;
	}

	void clear() { _map.clear(); }
};

static MovieLibrary s_movie_library;

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
	    sprite_instance* m = root_m->get_root_movie();
      
	    if (m->get_ref_count() < 2)
		{
		    log_action("extern movie deleted");
		    s_extern_sprites.erase(s_extern_sprites.begin() + i);
		    i--;
		    root_m->drop_ref();
		}
	}
}
//#endif // 0

movie_interface* get_current_root()
{
//    assert(s_current_root != NULL);
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
//
movie_definition* create_library_movie(const URL& url, const char* real_url)
{
//    log_msg("%s: url is %s", __PRETTY_FUNCTION__, url.str().c_str());

    // Use real_url as label for cache if available 
    std::string cache_label = real_url ? real_url : url.str();

    // Is the movie already in the library?
    {
	smart_ptr<movie_definition>	m;
	if ( s_movie_library.get(cache_label, &m) )
	    {
    		log_msg(" movie already in library");
		// Return cached movie.
		m->add_ref();
		return m.get_ptr();
	    }
    }

    // Try to open a file under the filename.
    movie_definition* mov = create_movie(url, real_url);

    if (mov == NULL)
	{
	    log_error("couldn't load library movie '%s'\n", url.str().c_str());
	    return NULL;
	}
    else
	{
	    s_movie_library.add(cache_label, mov);
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
	    log_error("couldn't create instance\n");

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
    // Use automatic class var to make sure we restore
    // when exiting the function.
    class save_stuff
    {
    public:
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
	    log_error("precompute_cached_data can't create instance of movie\n");
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

	    size_t last_frame = m->get_current_frame();
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
		    log_error("loop back; jumping to frame %u\n", last_frame);
		    m->goto_frame(last_frame + 1);
		}
	    else
		{
		    kick_count = 0;
		}
	}

    m->drop_ref();
}


} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
