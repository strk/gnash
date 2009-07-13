// impl.cpp:  Implement ActionScript tags, movie loading, library, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
# include "gnashconfig.h"
#endif

#include "MovieFactory.h"
#include "smart_ptr.h" // GNASH_USE_GC
#include "IOChannel.h"
#include "utility.h"
#include "fontlib.h"
#include "log.h"
#include "GnashImage.h"
#include "sprite_definition.h"
#include "SWFMovieDefinition.h"
#include "BitmapMovieDefinition.h"
#include "RunResources.h"
#include "URL.h"
#include "StreamProvider.h"
#include "MovieClip.h"
#include "VM.h"
#include "MovieLibrary.h"

#ifdef GNASH_USE_GC
#include "GC.h"
#endif

#include <string>
#include <map>
#include <memory> // for auto_ptr
#include <algorithm>

namespace gnash
{

// Forward declarations
namespace {
    FileType getFileType(IOChannel& in);
    SWFMovieDefinition* createSWFMovie(std::auto_ptr<IOChannel> in,
            const std::string& url, const RunResources& runInfo,
            bool startLoaderThread);
}

static void clear_library();

// Create a movie_definition from an image format stream
// NOTE: this method assumes this *is* the format described in the
// FileType type
// TODO: The pp won't display PNGs for SWF7 or below.
static movie_definition*
createBitmapMovie(std::auto_ptr<IOChannel> in, const std::string& url,
        const RunResources& r, FileType type)
{
    assert (in.get());

    // readImageData takes a shared pointer because JPEG streams sometimes need
    // to transfer ownership.
    boost::shared_ptr<IOChannel> imageData(in.release());

    try
    {
        std::auto_ptr<GnashImage> im(
                ImageInput::readImageData(imageData, type));

        if (!im.get()) {
            log_error(_("Can't read image file from %s"), url);
            return NULL;
        }

        Renderer* renderer = r.renderer();

        BitmapMovieDefinition* mdef =
            new BitmapMovieDefinition(im, renderer, url);

        return mdef;

    }
    catch (ParserException& e)
    {
        log_error(_("Parsing error: %s"), e.what());
        return NULL;
    }

}


movie_definition*
MovieFactory::makeMovie(std::auto_ptr<IOChannel> in, const std::string& url,
        const RunResources& runInfo, bool startLoaderThread)
{
  assert(in.get());

  // see if it's a jpeg or an swf
  FileType type = getFileType(*in);

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
            return createBitmapMovie(in, url, runInfo, type);
        }


        case GNASH_FILETYPE_SWF:
            return createSWFMovie(in, url, runInfo, startLoaderThread);

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
createNonLibraryMovie(const URL& url, const RunResources& runInfo,
        const char* reset_url, bool startLoaderThread,
        const std::string* postdata)
{

  std::auto_ptr<IOChannel> in;

  const StreamProvider& streamProvider = runInfo.streamProvider();

  const RcInitFile& rcfile = RcInitFile::getDefaultInstance();

  if (postdata) {
      in = streamProvider.getStream(url, *postdata, rcfile.saveLoadedMedia());
  }
  else in = streamProvider.getStream(url, rcfile.saveLoadedMedia());

  if ( ! in.get() )
  {
      log_error(_("failed to open '%s'; can't create movie"), url);
      return NULL;
  }
  
  if (in->bad())
  {
      log_error(_("streamProvider opener can't open '%s'"), url);
      return NULL;
  }

  std::string movie_url = reset_url ? reset_url : url.str();
  movie_definition* ret = MovieFactory::makeMovie(in, movie_url, runInfo,
          startLoaderThread);

  return ret;


}


namespace {

/// Get type of file looking at first bytes
FileType
getFileType(IOChannel& in)
{
    in.seek(0);

    char buf[3];
    
    if (in.read(buf, 3) < 3)
    {
        log_error(_("Can't read file header"));
        in.seek(0);
        return GNASH_FILETYPE_UNKNOWN;
    }
    
    // This is the magic number {0xff, 0xd8, 0xff} for JPEG format files
    if (std::equal(buf, buf + 3, "\xff\xd8\xff"))
    {
        in.seek(0);
        return GNASH_FILETYPE_JPEG;
    }

    // This is the magic number for any PNG format file
    // buf[3] == 'G' (we didn't read so far)
    if (std::equal(buf, buf + 3, "\x89PN")) 
    {
        in.seek(0);
        return GNASH_FILETYPE_PNG;
    }

    // This is the magic number for any GIF format file
    if (std::equal(buf, buf + 3, "GIF"))
    {
        in.seek(0);
        return GNASH_FILETYPE_GIF;
    }

    // This is for SWF (FWS or CWS)
    if (std::equal(buf, buf + 3, "FWS") || std::equal(buf, buf + 3, "CWS"))
    {
        in.seek(0);
        return GNASH_FILETYPE_SWF;
    }

    // Take one guess at what this is. (It's an FLV-format file).
    if (std::equal(buf, buf + 3, "FLV")) {
        return GNASH_FILETYPE_FLV;
    }
    
    // Check if it is an swf embedded in a player (.exe-file)
    if (std::equal(buf, buf + 2, "MZ")) {

        if ( 3 > in.read(buf, 3) )
        {
            log_error(_("Can't read 3 bytes after an MZ (.exe) header"));
            in.seek(0);
            return GNASH_FILETYPE_UNKNOWN;
        }

        while ((buf[0]!='F' && buf[0]!='C') || buf[1]!='W' || buf[2]!='S')
        {
            buf[0] = buf[1];
            buf[1] = buf[2];
            buf[2] = in.read_byte();
            if (in.eof())
            {
                log_error(_("Could not find SWF inside an exe file"));
                in.seek(0);
                return GNASH_FILETYPE_UNKNOWN;
            }
        }
        in.seek(in.tell() - static_cast<std::streamoff>(3));
        return GNASH_FILETYPE_SWF;
    }

    log_error("unknown file type, buf is %c%c%c", buf[0], buf[1], buf[2]);
    return GNASH_FILETYPE_UNKNOWN;
}

// Create a SWFMovieDefinition from an SWF stream
// NOTE: this method assumes this *is* an SWF stream
//
SWFMovieDefinition*
createSWFMovie(std::auto_ptr<IOChannel> in, const std::string& url,
        const RunResources& runInfo, bool startLoaderThread)
{

    std::auto_ptr<SWFMovieDefinition> m (new SWFMovieDefinition(runInfo));

    const std::string& absURL = URL(url).str();

    if (!m->readHeader(in, absURL)) return 0;
    if (startLoaderThread && !m->completeLoad()) return 0;

    return m.release();
}

}

//
// global gnash management
//


// Maximum release of resources.
void  clear()
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
    log_debug("Any segfault past this message is likely due to improper "
            "threads cleanup.");

    VM::get().clear();

    clear_library();
    fontlib::clear();

#ifdef GNASH_USE_GC 
    GC::get().collect();

    GC::cleanup();
#endif

}

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
movie_definition*
MovieFactory::makeMovie(const URL& url, const RunResources& runInfo,
        const char* real_url, bool startLoaderThread,
        const std::string* postdata)
{

    // Use real_url as label for cache if available 
    std::string cache_label = real_url ? URL(real_url).str() : url.str();

    // Is the movie already in the library? (don't check if we have post data!)
    if (!postdata)
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
    // calling createMovie() again and NOT finding
    // the just-created movie.
    movie_definition* mov = createNonLibraryMovie(url, runInfo, real_url, false,
            postdata);

    if (!mov)
    {
        log_error(_("Couldn't load library movie '%s'"), url.str());
        return NULL;
    }

    // Movie is good, add to the library 
    if (!postdata) // don't add if we POSTed
    {
        s_movie_library.add(cache_label, mov);
        log_debug(_("Movie %s (SWF%d) added to library"),
                cache_label, mov->get_version());
    }
    else
    {
        log_debug(_("Movie %s (SWF%d) NOT added to library (resulted from "
                    "a POST)"), cache_label, mov->get_version());
    }

    /// Now complete the load if the movie is an SWF movie
    // 
    /// This is a no-op except for SWF movies.
    if (startLoaderThread) mov->completeLoad();

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
