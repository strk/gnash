// impl.cpp:  Implement ActionScript tags, movie loading, library, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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
//


#include "smart_ptr.h" // GNASH_USE_GC
#include "IOChannel.h"
#include "utility.h"
#include "impl.h"
#include "font.h"
#include "fontlib.h"
#include "log.h"
#include "image.h"
#include "sprite_definition.h"
#include "SWFMovieDefinition.h"
#include "swf.h"
#include "swf/TagLoadersTable.h"
#include "URL.h"
#include "StreamProvider.h"
#include "sprite_instance.h"
#include "VM.h"
#include "ScriptLimitsTag.h"
#include "BitmapMovieDefinition.h"
#include "DefineFontAlignZonesTag.h"
#include "DefineButtonCxformTag.h"
#include "CSMTextSettingsTag.h"
#include "PlaceObject2Tag.h"
#include "RemoveObjectTag.h"
#include "DoActionTag.h"
#include "DoInitActionTag.h"
#include "SetBackgroundColorTag.h"
#include "StartSoundTag.h"
#include "StreamSoundBlockTag.h"
#include "swf/tag_loaders.h" // for all tag loaders..
#include "sound_handler.h" // for get_sound_handler
#ifdef GNASH_USE_GC
#include "GC.h"
#endif

#include <string>
#include <map>
#include <memory> // for auto_ptr

namespace gnash
{

static void clear_library();

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
static media::sound_handler* _sound_handler = 0;
void  set_sound_handler(media::sound_handler* s) { _sound_handler = s; }
media::sound_handler* get_sound_handler() { return _sound_handler; }

void
set_base_url(const URL& url)
{
  // can call this only once during a single run
  assert(!globals::baseurl.get());
  globals::baseurl.reset(new URL(url));
  log_debug(_("Base url set to: %s"), globals::baseurl->str());
}

const URL&
get_base_url()
{
  // Don't call me if you haven't set me !
  assert(globals::baseurl.get());
  return *globals::baseurl;
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

static void ensure_loaders_registered()
{
    using namespace SWF::tag_loaders;
    using namespace SWF;

    static bool s_registered = false;

    if (s_registered) return;

    // Register the standard loaders.
    s_registered = true;

    // End tag doesn't really need to exist.
    // TODO: use null_loader here ?
    register_tag_loader(SWF::END, end_loader);

    register_tag_loader(SWF::DEFINESHAPE, define_shape_loader);
    register_tag_loader(SWF::FREECHARACTER, fixme_loader); // 03
    register_tag_loader(SWF::PLACEOBJECT, PlaceObject2Tag::loader);
    register_tag_loader(SWF::REMOVEOBJECT,  RemoveObjectTag::loader); // 05
    register_tag_loader(SWF::DEFINEBITS,  define_bits_jpeg_loader);
    register_tag_loader(SWF::DEFINEBUTTON,  button_character_loader);
    register_tag_loader(SWF::JPEGTABLES,  jpeg_tables_loader);
    register_tag_loader(SWF::SETBACKGROUNDCOLOR, SetBackgroundColorTag::loader);
    register_tag_loader(SWF::DEFINEFONT,  define_font_loader);
    register_tag_loader(SWF::DEFINETEXT,  define_text_loader);
    register_tag_loader(SWF::DOACTION,  DoActionTag::doActionLoader);
    register_tag_loader(SWF::DEFINEFONTINFO, define_font_info_loader);
    register_tag_loader(SWF::DEFINEFONTINFO2, define_font_info_loader); // 62
    register_tag_loader(SWF::DEFINESOUND, define_sound_loader);
    register_tag_loader(SWF::STARTSOUND,  StartSoundTag::loader);

    register_tag_loader(SWF::STOPSOUND,     fixme_loader); // 16 

    register_tag_loader(SWF::DEFINEBUTTONSOUND, button_sound_loader);
    register_tag_loader(SWF::SOUNDSTREAMHEAD, sound_stream_head_loader); // 18
    register_tag_loader(SWF::SOUNDSTREAMBLOCK, StreamSoundBlockTag::loader); // 19
    register_tag_loader(SWF::DEFINELOSSLESS, define_bits_lossless_2_loader);
    register_tag_loader(SWF::DEFINEBITSJPEG2, define_bits_jpeg2_loader);
    register_tag_loader(SWF::DEFINESHAPE2,  define_shape_loader);
    register_tag_loader(SWF::DEFINEBUTTONCXFORM, DefineButtonCxformTag::loader); // 23
    // "protect" tag; we're not an authoring tool so we don't care.
    // (might be nice to dump the password instead..)
    register_tag_loader(SWF::PROTECT, null_loader);
    register_tag_loader(SWF::PATHSAREPOSTSCRIPT, fixme_loader); // 25
    register_tag_loader(SWF::PLACEOBJECT2,  PlaceObject2Tag::loader);
    // 27 - _UNKNOWN_ unimplemented
    register_tag_loader(SWF::REMOVEOBJECT2, RemoveObjectTag::loader); // 28
    register_tag_loader(SWF::SYNCFRAME, fixme_loader); // 29
    // 30 - _UNKNOWN_ unimplemented
    register_tag_loader(SWF::FREEALL, fixme_loader); // 31
    register_tag_loader(SWF::DEFINESHAPE3,  define_shape_loader);
    register_tag_loader(SWF::DEFINETEXT2, define_text_loader);
    register_tag_loader(SWF::DEFINEBUTTON2, button_character_loader);
    register_tag_loader(SWF::DEFINEBITSJPEG3, define_bits_jpeg3_loader);
    register_tag_loader(SWF::DEFINELOSSLESS2, define_bits_lossless_2_loader);
    register_tag_loader(SWF::DEFINEEDITTEXT, define_edit_text_loader);
    register_tag_loader(SWF::DEFINEVIDEO, fixme_loader); // 38
    register_tag_loader(SWF::DEFINESPRITE,  sprite_loader);
    register_tag_loader(SWF::NAMECHARACTER, fixme_loader); // 40
    register_tag_loader(SWF::SERIALNUMBER,  serialnumber_loader); // 41
    register_tag_loader(SWF::DEFINETEXTFORMAT, fixme_loader); // 42
    register_tag_loader(SWF::FRAMELABEL,  frame_label_loader); // 43

    // TODO: Implement, but fixme_loader breaks tests.
    register_tag_loader(SWF::DEFINEBEHAVIOR, fixme_loader); // 44

    register_tag_loader(SWF::SOUNDSTREAMHEAD2, sound_stream_head_loader); // 45
    register_tag_loader(SWF::DEFINEMORPHSHAPE, define_shape_morph_loader);
    register_tag_loader(SWF::FRAMETAG,  fixme_loader); // 47
    register_tag_loader(SWF::DEFINEFONT2, define_font_loader); // 48
    register_tag_loader(SWF::GENCOMMAND,  fixme_loader); // 49
    register_tag_loader(SWF::DEFINECOMMANDOBJ, fixme_loader); // 50
    register_tag_loader(SWF::CHARACTERSET,  fixme_loader); // 51
    register_tag_loader(SWF::FONTREF, fixme_loader); // 52

    // TODO: Implement, but fixme_loader breaks tests.
    register_tag_loader(SWF::DEFINEFUNCTION, fixme_loader); // 53 
    register_tag_loader(SWF::PLACEFUNCTION, fixme_loader); // 54 
    register_tag_loader(SWF::GENTAGOBJECT, fixme_loader); // 55 

    register_tag_loader(SWF::EXPORTASSETS, export_loader); // 56
    register_tag_loader(SWF::IMPORTASSETS, import_loader); // 57

    //  We're not an authoring tool so we don't care.
    // (might be nice to dump the password instead..)
    register_tag_loader(SWF::ENABLEDEBUGGER, null_loader);    // 58

    register_tag_loader(SWF::INITACTION, DoInitActionTag::doInitActionLoader);  // 59  

    register_tag_loader(SWF::DEFINEVIDEOSTREAM, define_video_loader); // 60
    register_tag_loader(SWF::VIDEOFRAME, video_loader); // 61

    // 62, DEFINEFONTINFO2 is done above.
    // We're not an authoring tool.
    register_tag_loader(SWF::DEBUGID, null_loader); // 63

    //  We're not an authoring tool so we don't care.
    // (might be nice to dump the password instead..)
    register_tag_loader(SWF::ENABLEDEBUGGER2, null_loader);    // 64
    register_tag_loader(SWF::SCRIPTLIMITS, ScriptLimitsTag::loader); //65

    // TODO: Fix this, but probably not critical.
    register_tag_loader(SWF::SETTABINDEX, fixme_loader); //66 

    // TODO: Alexis reference says these are 83, 84. The 67,68 comes from Tamarin.
    // Figure out which one is correct (possibly both are).
    register_tag_loader(SWF::DEFINESHAPE4_, define_shape_loader); // 67
    register_tag_loader(SWF::DEFINEMORPHSHAPE2_, define_shape_morph_loader); // 68

    register_tag_loader(SWF::FILEATTRIBUTES, file_attributes_loader); // 69
    register_tag_loader(SWF::PLACEOBJECT3, PlaceObject2Tag::loader); // 70
    register_tag_loader(SWF::IMPORTASSETS2, import_loader); // 71

    register_tag_loader(SWF::DOABC, abc_loader); // 72 -- AS3 codeblock.
    register_tag_loader(SWF::DEFINEALIGNZONES, DefineFontAlignZonesTag::loader); // 73

    register_tag_loader(SWF::CSMTEXTSETTINGS, CSMTextSettingsTag::loader); // 74
    register_tag_loader(SWF::DEFINEFONT3, define_font_loader); // 75
    register_tag_loader(SWF::SYMBOLCLASS, fixme_loader); // 76 
    register_tag_loader(SWF::METADATA, metadata_loader); // 77
    register_tag_loader(SWF::DEFINESCALINGGRID, fixme_loader); // 78
    register_tag_loader(SWF::DOABCDEFINE, abc_loader); // 82 -- AS3 codeblock.
    register_tag_loader(SWF::DEFINESHAPE4, define_shape_loader); // 83
    register_tag_loader(SWF::DEFINEMORPHSHAPE2, define_shape_morph_loader); // 84
    register_tag_loader(SWF::DEFINESCENEANDFRAMELABELDATA,define_scene_frame_label_loader); //86
    register_tag_loader(SWF::DEFINEFONTNAME, define_font_name_loader); // 88

    register_tag_loader(SWF::REFLEX, reflex_loader); // 777
}

// Create a movie_definition from a png stream
// NOTE: this method assumes this *is* a png stream
// TODO: The pp won't display PNGs for SWF7 or below.
static movie_definition*
createBitmapMovie(std::auto_ptr<IOChannel> in, const std::string& url, FileType type)
{
    assert (in.get());

    // readImageData takes a shared pointer because JPEG streams sometimes need
    // to transfer ownership.
    boost::shared_ptr<IOChannel> imageData(in.release());

    try
    {
        std::auto_ptr<image::ImageBase> im(image::readImageData(imageData, type));
        if (!im.get())
        {
            log_error(_("Can't read image file from %s"), url);
            return NULL;
        } 

        BitmapMovieDefinition* mdef = new BitmapMovieDefinition(im, url);
        return mdef;

    }
    catch (ParserException& e)
    {
        log_error(_("Parsing error: %s"), e.what());
        return NULL;
    }

}

// Get type of file looking at first bytes
// return "jpeg", "png", "swf" or "unknown"
//
FileType
getFileType(IOChannel* in)
{
  in->seek(0);

  unsigned char buf[3];
  
  if ( 3 < in->read(buf, 3) )
  {
    log_error(_("Can't read file header"));
    in->seek(0);
    return GNASH_FILETYPE_UNKNOWN;
  }
  
  // This is the magic number for any JPEG format file
  if ((buf[0] == 0xff) && (buf[1] == 0xd8) && (buf[2] == 0xff))
  {
    in->seek(0);
    return GNASH_FILETYPE_JPEG;
  }

  // This is the magic number for any PNG format file
  if ((buf[0] == 137) && (buf[1] == 'P') && (buf[2] == 'N')) // buf[3] == 'G' (we didn't read so far)
  {
    in->seek(0);
    return GNASH_FILETYPE_PNG;
  }

  // This is the magic number for any GIF format file
  if ((buf[0] == 'G') && (buf[1] == 'I') && (buf[2] == 'F'))
  {
    in->seek(0);
    return GNASH_FILETYPE_GIF;
  }

  // This is for SWF (FWS or CWS)
  if (  (buf[0] == 'F' || buf[0] == 'C') &&
    (buf[1] == 'W') &&
    (buf[2] == 'S') )
  {
    in->seek(0);
    return GNASH_FILETYPE_SWF;
  }

  if ((buf[0] == 'F') && (buf[1] == 'L') && (buf[2] == 'V') ) {
    return GNASH_FILETYPE_FLV;
  }
  
  // Check if it is an swf embedded in a player (.exe-file)
  if ((buf[0] == 'M') && (buf[1] == 'Z')) {

    if ( 3 < in->read(buf, 3) )
    {
      log_error(_("Can't read 3 bytes after an MZ (.exe) header"));
      in->seek(0);
      return GNASH_FILETYPE_UNKNOWN;
    }

    while ((buf[0]!='F' && buf[0]!='C') || buf[1]!='W' || buf[2]!='S')
    {
      buf[0] = buf[1];
      buf[1] = buf[2];
      buf[2] = in->read_byte();
      if (in->eof())
      {
        log_error(_("Could not find SWF inside an exe file"));
        in->seek(0);
        return GNASH_FILETYPE_UNKNOWN;
      }
    }
    in->seek(in->tell()-3); // position to start of the swf itself
    return GNASH_FILETYPE_SWF;
  }

  log_error("unknown file type, buf is %c%c%c", buf[0], buf[1], buf[2]);
  return GNASH_FILETYPE_UNKNOWN;
}

// Create a SWFMovieDefinition from an SWF stream
// NOTE: this method assumes this *is* an SWF stream
//
static SWFMovieDefinition*
create_swf_movie(std::auto_ptr<IOChannel> in, const std::string& url, bool startLoaderThread)
{

  // Avoid leaks on error 
  std::auto_ptr<SWFMovieDefinition> m ( new SWFMovieDefinition() );

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
create_movie(std::auto_ptr<IOChannel> in, const std::string& url, bool startLoaderThread)
{
  assert(in.get());

  ensure_loaders_registered();

  // see if it's a jpeg or an swf
  FileType type = getFileType(in.get());

    switch (type)
    {
        case GNASH_FILETYPE_JPEG:
        case GNASH_FILETYPE_PNG:
        case GNASH_FILETYPE_GIF:
        {
            if ( startLoaderThread == false )
            {
              log_unimpl(_("Requested to keep from completely loading "
                           "a movie, but the movie in question is an "
                           "image, for which we don't yet have the "
                           "concept of a 'loading thread'"));
            }
            return createBitmapMovie(in, url, type);
        }


        case GNASH_FILETYPE_SWF:
            return create_swf_movie(in, url, startLoaderThread);

        case GNASH_FILETYPE_FLV:
            log_unimpl(_("FLV can't be loaded directly as a movie"));
            return NULL;
        default:
            log_error(_("unknown file type (%s)"), type);
            break;
    }

    return NULL;
}

movie_definition*
create_movie(const URL& url, const char* reset_url, bool startLoaderThread, const std::string* postdata)
{

  const std::string swfurl = url.str();

  std::auto_ptr<IOChannel> in;
  if ( postdata ) in.reset( globals::streamProvider.getStream(url, *postdata) );
  else in.reset( globals::streamProvider.getStream(url) );
  if ( ! in.get() )
  {
      log_error(_("failed to open '%s'; can't create movie"), swfurl);
      return NULL;
  }
  else if ( in->get_error() )
  {
      log_error(_("streamProvider opener can't open '%s'"), swfurl);
      return NULL;
  }

  const char* movie_url = reset_url ? reset_url : swfurl.c_str();
  movie_definition* ret = create_movie(in, movie_url, startLoaderThread);

  return ret;


}

//
// global gnash management
//


void  clear()
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

    if ( VM::isInitialized() )
    {
        VM::get().getRoot().clear();
    }

#ifdef GNASH_USE_GC 
    GC::get().collect();

    GC::cleanup();
#endif

    // By setting the soundhandler to NULL we avoid it being used
    // after it's been de-referenced
    set_sound_handler(NULL);

    // By setting the render handler to NULL we avoid it being used
    // after it's been de-referenced (fixes bug #21310)
    set_render_handler(NULL);
}

//
// library stuff, for sharing resources among different movies.
//


/// Library of SWF movies indexed by URL strings
//
/// Elements are actually SWFMovieDefinition, the ones
/// associated with URLS. Dunno why, but we were using
/// movie_definition here before so this didn't change
/// when the new class was introduced.
///
class MovieLibrary
{
private:

  typedef struct {
    boost::intrusive_ptr<movie_definition> def;
    unsigned hit_count;
  } item;

  typedef std::map< std::string, item > container;

  container _map;
  unsigned _limit;

public:

  MovieLibrary() : 
    _limit(8) 
  {
	RcInitFile& rcfile = RcInitFile::getDefaultInstance();
	set_limit(rcfile.getMovieLibraryLimit());
  }
  
  /// Sets the maximum number of items to hold in the library. When adding new
  /// items, the one with the least hit count is being removed in that case.
  /// Zero is a valid limit (disables library). 
  void set_limit(unsigned limit)
  {
    _limit = limit;  
    limit_size(_limit);  
  }

  bool get(const std::string& key, boost::intrusive_ptr<movie_definition>* ret)
  {
    container::iterator it = _map.find(key);
    if ( it != _map.end() )
    {
      *ret = it->second.def;
      it->second.hit_count++;
      
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
      i->second.def->setReachable();
    }
  }
#endif

  void add(const std::string& key, movie_definition* mov)
  {
  
    if (_limit)
    {
      limit_size(_limit-1);
    }
    else return;  // zero limit, library is a no-op
  
    item temp;
    
    temp.def = mov;
    temp.hit_count=0;
    
    _map[key] = temp;
  }
  

  void clear() { _map.clear(); }
  
private:

  void limit_size(unsigned max) 
  {

    if (max < 1) 
    {
      clear();
      return;
    }

    while (_map.size() > max) 
    {

      // find the item with the worst hit count and remove it
      container::iterator worst = _map.begin();
      
      for ( container::iterator i=_map.begin(), e=_map.end(); i!=e; ++i)
      {
        if (i->second.hit_count < worst->second.hit_count) 
          worst = i;
      }
      
      _map.erase(worst);
    
    }
    
  } //limit_size
  
};

static MovieLibrary s_movie_library;

static void clear_library()
    // Drop all library references to movie_definitions, so they
    // can be cleaned up.
{
    s_movie_library.clear();
}

// Try to load a movie from the given url, if we haven't
// loaded it already.  Add it to our library on success, and
// return a pointer to it.
//
movie_definition* create_library_movie(const URL& url, const char* real_url, bool startLoaderThread, const std::string* postdata)
{
//    log_debug(_("%s: url is %s"), __PRETTY_FUNCTION__, url.str());

    // Use real_url as label for cache if available 
    std::string cache_label = real_url ? URL(real_url).str() : url.str();

    // Is the movie already in the library? (don't check if we have post data!)
    if ( ! postdata )
    {
  boost::intrusive_ptr<movie_definition>  m;
  if ( s_movie_library.get(cache_label, &m) )
      {
        log_debug(_("Movie %s already in library"), cache_label);
    return m.get();
      }
    }

  // Try to open a file under the filename, but DO NOT start
  // the loader thread now to avoid IMPORT tag loaders from 
  // calling create_library_movie() again and NOT finding
  // the just-created movie.
  movie_definition* mov = create_movie(url, real_url, false, postdata);
  //log_debug(_("create_movie(%s, %s, false) returned %p"), url.str(), real_url, mov);

  if (mov == NULL)
  {
    log_error(_("Couldn't load library movie '%s'"),
      url.str());
    return NULL;
  }

  // Movie is good, add to the library 
  if ( ! postdata ) // don't add if we POSTed
  {
  	s_movie_library.add(cache_label, mov);
        log_debug(_("Movie %s (SWF%d) added to library"), cache_label, mov->get_version());
  }
  else
  {
        log_debug(_("Movie %s (SWF%d) NOT added to library (resulted from a POST)"), cache_label, mov->get_version());
  }

  // Now complete the load if the movie is an SWF movie
  // 
  // FIXME: add completeLoad() to movie_definition class
  //        to allow loads of JPEG to use a loader thread
  //        too...
  //
  if ( startLoaderThread )
  {
    SWFMovieDefinition* mdi = dynamic_cast<SWFMovieDefinition*>(mov);
    if ( mdi ) {
      mdi->completeLoad();
    }
  }

  //log_debug(_("create_library_movie(%s, %s, startLoaderThread=%d) about to return %p"), url.str(), real_url, startLoaderThread, mov);
  return mov;
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
