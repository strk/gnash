// impl.cpp:  Implement ActionScript tags, movie loading, library, for Gnash.
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
//

/* $Id: impl.cpp,v 1.111 2007/06/30 18:22:01 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_file.h"
#include "utility.h"
#include "action.h"
#include "impl.h"
#include "font.h"
#include "fontlib.h"
#include "log.h"
#include "render.h"
#include "shape.h"
#include "styles.h"
#include "dlist.h"
#include "timers.h"
#include "image.h"
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
#include "VM.h"
#include "BitmapMovieDefinition.h"
#include "DefineFontAlignZonesTag.h"
#include "PlaceObject2Tag.h"
#include "RemoveObjectTag.h"
#include "DoActionTag.h"
#include "sound_handler.h" // for get_sound_handler
#ifdef GNASH_USE_GC
#include "GC.h"
#endif

#include <string>
#include <map>
#include <memory> // for auto_ptr

namespace gnash
{

static void	clear_library();

/// Namespace for global data (will likely turn into a class)
namespace globals { // gnash::globals

	/// global StreamProvider
	StreamProvider& streamProvider = StreamProvider::getDefaultInstance();

	/// Base url (for relative urls resolution)
	//
	/// we need an auto_ptr becase the URL class
	/// is an immutable one and needs to be set 
	/// at construction time..
	///
	static std::auto_ptr<URL> baseurl;

} // namespace gnash::global

// global Sound handler stuff. Should this be moved to the VM class ?
static sound_handler* _sound_handler = 0;
void	set_sound_handler(sound_handler* s) { _sound_handler = s; }
sound_handler*	get_sound_handler() { return _sound_handler; }

void
set_base_url(const URL& url)
{
	// can call this only once during a single run
	assert(!globals::baseurl.get());
	globals::baseurl.reset(new URL(url));
	log_msg(_("Base url set to: %s"), globals::baseurl->str().c_str());
}

const URL&
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
s_use_cache_files = false;

// Enable/disable attempts to read cache files when loading
// movies.
void
set_use_cache_files(bool use_cache)
{
    s_use_cache_files = use_cache;
}

// Associate the specified tag type with the given tag loader
// function.
void
register_tag_loader(SWF::tag_type t, SWF::TagLoadersTable::loader_function lf)
{
	using SWF::TagLoadersTable;

	TagLoadersTable& table = TagLoadersTable::getInstance();

	bool loader_registered = table.register_loader(t, lf);
	assert(loader_registered);
}

static void	ensure_loaders_registered()
{
	using namespace SWF::tag_loaders;
	using namespace SWF;

	static bool s_registered = false;
	
	if (s_registered) return;

	// Register the standard loaders.
	s_registered = true;

	register_tag_loader(SWF::END,		end_loader);
	register_tag_loader(SWF::DEFINESHAPE,	define_shape_loader);
	register_tag_loader(SWF::FREECHARACTER, fixme_loader); // 03
	register_tag_loader(SWF::PLACEOBJECT,	PlaceObject2Tag::loader);
	register_tag_loader(SWF::REMOVEOBJECT,	RemoveObjectTag::loader); // 05
	register_tag_loader(SWF::DEFINEBITS,	define_bits_jpeg_loader);
	register_tag_loader(SWF::DEFINEBUTTON,	button_character_loader);
	register_tag_loader(SWF::JPEGTABLES,	jpeg_tables_loader);
	register_tag_loader(SWF::SETBACKGROUNDCOLOR, set_background_color_loader);
	register_tag_loader(SWF::DEFINEFONT,	define_font_loader);
	register_tag_loader(SWF::DEFINETEXT,	define_text_loader);
	register_tag_loader(SWF::DOACTION,	DoActionTag::doActionLoader);
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
	// (might be nice to dump the password instead..)
	register_tag_loader(SWF::PROTECT,	null_loader);
	register_tag_loader(SWF::PATHSAREPOSTSCRIPT, fixme_loader); // 25
	register_tag_loader(SWF::PLACEOBJECT2,	PlaceObject2Tag::loader);
	// 27 - _UNKNOWN_ unimplemented
	register_tag_loader(SWF::REMOVEOBJECT2, RemoveObjectTag::loader); // 28
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
	register_tag_loader(SWF::SERIALNUMBER,  serialnumber_loader); // 41
	register_tag_loader(SWF::DEFINETEXTFORMAT, fixme_loader); // 42
	register_tag_loader(SWF::FRAMELABEL,	frame_label_loader); // 43
	// 44 - _UNKNOWN_ unimplemented
	register_tag_loader(SWF::SOUNDSTREAMHEAD2, sound_stream_head_loader); // 45
	register_tag_loader(SWF::DEFINEMORPHSHAPE, define_shape_morph_loader);
	register_tag_loader(SWF::FRAMETAG,	fixme_loader); // 47
	register_tag_loader(SWF::DEFINEFONT2,	define_font_loader); // 48
	register_tag_loader(SWF::GENCOMMAND,	fixme_loader); // 49
	register_tag_loader(SWF::DEFINECOMMANDOBJ, fixme_loader); // 50
	register_tag_loader(SWF::CHARACTERSET,  fixme_loader); // 51
	register_tag_loader(SWF::FONTREF,	fixme_loader); // 52
	// 53 - _UNKNOWN_ unimplemented
	// 54 - _UNKNOWN_ unimplemented
	// 55 - _UNKNOWN_ unimplemented
	register_tag_loader(SWF::EXPORTASSETS,	export_loader); // 56
	register_tag_loader(SWF::IMPORTASSETS,  import_loader); // 57

	//  We're not an authoring tool so we don't care.
	// (might be nice to dump the password instead..)
	register_tag_loader(SWF::ENABLEDEBUGGER, null_loader);    // 58

	register_tag_loader(SWF::INITACTION, DoActionTag::doInitActionLoader);  // 59  

	register_tag_loader(SWF::DEFINEVIDEOSTREAM, define_video_loader); // 60
	register_tag_loader(SWF::VIDEOFRAME, video_loader); // 61
	// 62 - _UNKNOWN_ unimplemented
	// 63 - _UNKNOWN_ unimplemented
	//  We're not an authoring tool so we don't care.
	// (might be nice to dump the password instead..)
	register_tag_loader(SWF::ENABLEDEBUGGER2, null_loader);    // 64
	
	register_tag_loader(SWF::FILEATTRIBUTES, file_attributes_loader); // 69
	register_tag_loader(SWF::PLACEOBJECT3, fixme_loader); // 70
	register_tag_loader(SWF::IMPORTASSETS2, import_loader); // 71

	register_tag_loader(SWF::DEFINEALIGNZONES, DefineFontAlignZonesTag::loader); // 73

	register_tag_loader(SWF::CSMTEXTSETTINGS, fixme_loader); // 74
	register_tag_loader(SWF::DEFINEFONT3, define_font_loader); // 75
	register_tag_loader(SWF::METADATA, metadata_loader); // 77
	register_tag_loader(SWF::DEFINESCALINGGRID, fixme_loader); // 78
	register_tag_loader(SWF::DEFINESHAPE4, fixme_loader); // 83
	register_tag_loader(SWF::DEFINEMORPHSHAPE2, fixme_loader); // 84
}



#if 0 // deprecated
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
    //log_msg(_("%s: url is %s"),  __PRETTY_FUNCTION__, url.str().c_str());

    tu_file*	in = globals::streamProvider.getStream(url);
    if (in == NULL || in->get_error() != TU_FILE_NO_ERROR) {
	log_error(_("get_movie_info(): can't open '%s'"), url.str().c_str());
	if (version) *version = 0;
	//delete in;
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
	log_error(_("get_movie_info(): file '%s' does not start with a SWF header"), url.str().c_str());
	if (version) *version = 0;
	//delete in;
	return;
    }
    bool	compressed = (header & 255) == 'C';
    
    tu_file*	original_in = NULL;
    if (compressed) {
#ifndef HAVE_ZLIB_H
	log_error(_("get_movie_info(): can't read zipped SWF data; gnash was compiled without zlib support"));
	return;
#else
	original_in = in;
	
	// Uncompress the input as we read it.
	in = zlib_adapter::make_inflater(original_in);
	
	// Subtract the size of the 8-byte header, since
	// it's not included in the compressed
	// stream length.
	file_length -= 8;
#endif
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

    //delete in;
    //delete original_in;
}
#endif

// Create a movie_definition from a jpeg stream
// NOTE: this method assumes this *is* a jpeg stream
static movie_definition*
create_jpeg_movie(std::auto_ptr<tu_file> in, const std::string& url)
{
	// FIXME: temporarly disabled
	//log_unimpl(_("Loading of jpegs"));
	//return NULL;


	std::auto_ptr<image::rgb> im ( image::read_jpeg(in.get()) );

	if ( ! im.get() )
	{
		log_error(_("Can't read jpeg"));
		return NULL;
	} 

	BitmapMovieDefinition* mdef = new BitmapMovieDefinition(im, url);
	//log_msg(_("BitmapMovieDefinition %p created"), mdef);
	return mdef;


	// FIXME: create a movie_definition from a jpeg
	//bitmap_character*	 ch = new bitmap_character(bi);
}

// Get type of file looking at first bytes
// return "jpeg", "swf" or "unknown"
//
static std::string
get_file_type(tu_file* in)
{
	in->set_position(0);

	unsigned char buf[3];
	memset(buf, 0, 3);
	if ( 3 < in->read_bytes(buf, 3) )
	{
		log_error(_("Can't read file header"));
		return "unknown";
	}
	
	// This is the magic number for any JPEG format file
	if ((buf[0] == 0xff) && (buf[1] == 0xd8) && (buf[2] == 0xff))
	{
		in->set_position(0);
		return "jpeg";
	}

	// This is for SWF (FWS or CWS)
	if (	(buf[0] == 'F' || buf[0] == 'C') &&
		(buf[1] == 'W') &&
		(buf[2] == 'S') )
	{
		in->set_position(0);
		return "swf";
	}
	
	// Check if it is an swf embedded in a player (.exe-file)
	if ((buf[0] == 'M') && (buf[1] == 'Z')) {

		if ( 3 < in->read_bytes(buf, 3) ) return "unknown";

		while (buf[0]!='F' && buf[0]!='C' || buf[1]!='W' || buf[2]!='S') {
			buf[0] = buf[1];
			buf[1] = buf[2];
			buf[2] = in->read_byte();
			if (in->get_eof()) return "unknown";
		}
		in->set_position(in->get_position()-3);
		return "swf";
	}
	return "unknown";
}

// Create a movie_def_impl from an SWF stream
// NOTE: this method assumes this *is* an SWF stream
//
static movie_def_impl*
create_swf_movie(std::auto_ptr<tu_file> in, const std::string& url, bool startLoaderThread)
{

	// Avoid leaks on error 
	std::auto_ptr<movie_def_impl> m(
		new movie_def_impl(DO_LOAD_BITMAPS, DO_LOAD_FONT_SHAPES)
		);

	if ( ! m->readHeader(in, url) )
	{
		return NULL;
	}

	if ( startLoaderThread && ! m->completeLoad() )
	{
		return NULL;
	}

	return m.release();
}

movie_definition*
create_movie(std::auto_ptr<tu_file> in, const std::string& url, bool startLoaderThread)
{
	assert(in.get());

	ensure_loaders_registered();

	// see if it's a jpeg or an swf
	// TODO: use an integer code rather then a string !
	std::string type = get_file_type(in.get());

	if ( type == "jpeg" )
	{
		if ( startLoaderThread == false )
		{
			log_unimpl(_("Requested to keep from completely loading a movie, but the movie in question is a jpeg, for which we don't yet have the concept of a 'loading thread'"));
		}
		return create_jpeg_movie(in, url);
	}
	else if ( type == "swf" )
	{
		return create_swf_movie(in, url, startLoaderThread);
	}

	log_error(_("unknown file type (%s)"), type.c_str());
	return NULL;
}

movie_definition*
create_movie(const URL& url, const char* reset_url, bool startLoaderThread)
{
	// URL::str() returns by value, save it to a local string
	std::string url_str = url.str();
	const char* c_url = url_str.c_str();

//	log_msg(_("%s: url is %s"),  __PRETTY_FUNCTION__, c_url);

	std::auto_ptr<tu_file> in ( globals::streamProvider.getStream(url) );
	if ( ! in.get() )
	{
	    log_error(_("failed to open '%s'; can't create movie"), c_url);
	    return NULL;
	}
	else if ( in->get_error() )
	{
	    log_error(_("streamProvider opener can't open '%s'"), c_url);
	    return NULL;
	}

	const char* movie_url = reset_url ? reset_url : c_url;
	movie_definition* ret = create_movie(in, movie_url, startLoaderThread);

	if (s_use_cache_files)
	{
		// Try to load a .gsc file.
		// WILL NOT WORK FOR NETWORK URLS, would need an hash
		std::string cache_filename(movie_url);
		cache_filename += ".gsc";
		std::auto_ptr<tu_file> cache_in ( new tu_file(cache_filename.c_str(), "rb") );
		if (cache_in.get() == NULL
			|| cache_in->get_error() != TU_FILE_NO_ERROR)
		{
			// Can't open cache file; don't sweat it.
			IF_VERBOSE_PARSE (
		        log_parse(_("note: couldn't open cache file '%s'"),
					cache_filename.c_str());
			)

			// can't read cache, so generate font texture data.
			ret->generate_font_bitmaps();
		}
		else
		{
			log_msg(_("Loading cache file %s"),
				cache_filename.c_str());
			// Load the cached data.
			ret->input_cached_data(cache_in.get());
		}

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

    return m;
}
#endif


//
// global gnash management
//



void	clear()
    // Maximum release of resources.
{
    // Ideally, we should make sure that function properly signals all threads
    // about exiting and giving them a chance to cleanly exit.
    //
    // If we clear shared memory here we're going to leave threads possibly
    // accessing deleted memory, which would trigger a segfault.
    //
    // My experience is that calling exit(), altought it has the same problem,
    // reduces the chances of segfaulting ...
    //
    // We want this fixed anyway as exit()
    // itselt can also trigger segfaults.
    //
    // See task task #6959 and depending items
    //
    std::cerr << "Any segfault past this message is likely due to improper threads cleanup." << std::endl;
    //exit(EXIT_SUCCESS);

    clear_library();
    fontlib::clear();

    // By setting the soundhandler to NULL we avoid it being used
    // after it's been de-referenced
    set_sound_handler(NULL);

    GC::get().collect();

    GC::cleanup();
}

//
// library stuff, for sharing resources among different movies.
//


//static stringi_hash< boost::intrusive_ptr<movie_definition> >	s_movie_library;

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

	typedef std::map< std::string, boost::intrusive_ptr<movie_definition> > container;

	container _map;

public:

	MovieLibrary() {}

	bool get(const std::string& key, boost::intrusive_ptr<movie_definition>* ret)
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

#ifdef GNASH_USE_GC
	/// Mark all library elements as reachable (for GC)
	void markReachableResources() const
	{
		for ( container::const_iterator i=_map.begin(), e=_map.end(); i!=e; ++i)
		{
			i->second->setReachable();
		}
	}
#endif

	void add(const std::string& key, movie_definition* mov)
	{
		_map[key] = mov;
	}

	void clear() { _map.clear(); }
};

static MovieLibrary s_movie_library;

typedef std::map< movie_definition*, boost::intrusive_ptr<sprite_instance> > library_container_t;
static library_container_t	s_movie_library_inst;
static std::vector<sprite_instance*> s_extern_sprites;

static std::string s_workdir;

void save_extern_movie(sprite_instance* m)
{
    s_extern_sprites.push_back(m);
}

movie_root*
get_current_root()
{
	return &(VM::get().getRoot());
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

static void	clear_library()
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
movie_definition* create_library_movie(const URL& url, const char* real_url, bool startLoaderThread)
{
//    log_msg(_("%s: url is %s"), __PRETTY_FUNCTION__, url.str().c_str());

    // Use real_url as label for cache if available 
    std::string cache_label = real_url ? URL(real_url).str() : url.str();

    // Is the movie already in the library?
    {
	boost::intrusive_ptr<movie_definition>	m;
	if ( s_movie_library.get(cache_label, &m) )
	    {
    		log_msg(_("Movie %s already in library"), cache_label.c_str());
		return m.get();
	    }
    }

	// Try to open a file under the filename, but DO NOT start
	// the loader thread now to avoid IMPORT tag loaders from 
	// calling create_library_movie() again and NOT finding
	// the just-created movie.
	movie_definition* mov = create_movie(url, real_url, false);
	//log_msg(_("create_movie(%s, %s, false) returned %p"), url.str().c_str(), real_url, mov);

	if (mov == NULL)
	{
		log_error(_("Couldn't load library movie '%s'"),
			url.str().c_str());
		return NULL;
	}

	// Movie is good, add to the library 
	s_movie_library.add(cache_label, mov);
    	log_msg(_("Movie %s (SWF%d) added to library"), cache_label.c_str(), mov->get_version());

	// Now complete the load if the movie is an SWF movie
	// 
	// FIXME: add completeLoad() to movie_definition class
	//        to allow loads of JPEG to use a loader thread
	//        too...
	//
	if ( startLoaderThread )
	{
		movie_def_impl* mdi = dynamic_cast<movie_def_impl*>(mov);
		if ( mdi ) {
			mdi->completeLoad();
		}
	}

	//log_msg(_("create_library_movie(%s, %s, startLoaderThread=%d) about to return %p"), url.str().c_str(), real_url, startLoaderThread, mov);
	return mov;
}

sprite_instance* create_library_movie_inst(movie_definition* md)
{
    // Is the movie instance already in the library?
    {
	library_container_t::const_iterator i = s_movie_library_inst.find(md);
	if (i != s_movie_library_inst.end())
	    {
		// Return cached movie instance.
		boost::intrusive_ptr<sprite_instance>   m((*i).second);
		return m.get();
	    }
    }

    // Try to create movie interface
    sprite_instance* mov = md->create_instance();

    if (mov == NULL)
	{
	    log_error(_("%s: couldn't create instance"), __FUNCTION__);

	    return NULL;
	}
    else
	{
	    s_movie_library_inst[md] = mov;
	}

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
    gnash::sprite_instance*	m = movie_def->create_instance();
    if (m == NULL)
	{
	    log_error(_("precompute_cached_data can't create instance of movie"));
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

	    if (m->get_play_state() == gnash::sprite_instance::STOP)
		{
		    // Kick the movie.
		    //log_msg(_("kicking movie, kick ct = %d"), kick_count);
		    m->goto_frame(last_frame + 1);
		    m->set_play_state(gnash::sprite_instance::PLAY);
		    kick_count++;

		    if (kick_count > 10)
			{
			    //log_msg(_("movie is stalled; giving up on playing it through"));
			    break;
			}
		}
	    else if (m->get_current_frame() < last_frame)
		{
		    // Hm, apparently we looped back.  Skip ahead...
		    log_error(_("loop back; jumping to frame " SIZET_FMT), last_frame);
		    m->goto_frame(last_frame + 1);
		}
	    else
		{
		    kick_count = 0;
		}
	}

#ifndef GNASH_USE_GC
    m->drop_ref();
#endif //ndef GNASH_USE_GC
}

#ifdef GNASH_USE_GC
/// A GC root used to mark all reachable collectable pointers
class GnashGcRoot : public GcRoot 
{

public:

	GnashGcRoot()
	{
	}

	void markReachableResources() const
	{
		VM::get().markReachableResources();

		// Mark library movies (TODO: redesign this part)
		s_movie_library.markReachableResources();
	}
};
#endif

void gnashInit()
{
#ifdef GNASH_USE_GC
	static GnashGcRoot gcRoot;
	GC::init(gcRoot);
#endif
}




} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
