// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include "MovieFactory.h"

#include <string>
#include <map>
#include <memory> 
#include <algorithm>

#include "GnashEnums.h"
#include "GnashImage.h"
#include "IOChannel.h"
#include "utility.h"
#include "log.h"
#include "SWFMovieDefinition.h"
#include "BitmapMovieDefinition.h"
#include "RunResources.h"
#include "URL.h"
#include "StreamProvider.h"
#include "MovieLibrary.h"
#include "fontlib.h"

namespace gnash {

namespace {
    /// Get type of file looking at first bytes
    FileType getFileType(IOChannel& in);

    boost::intrusive_ptr<SWFMovieDefinition> createSWFMovie(
            std::auto_ptr<IOChannel> in, const std::string& url,
            const RunResources& runResources, bool startLoaderThread);

    boost::intrusive_ptr<BitmapMovieDefinition> createBitmapMovie(
            std::auto_ptr<IOChannel> in, const std::string& url,
            const RunResources& r, FileType type);

    boost::intrusive_ptr<movie_definition> createNonLibraryMovie(
            const URL& url, const RunResources& runResources,
            const char* reset_url, bool startLoaderThread,
            const std::string* postdata);
}

MovieLibrary MovieFactory::movieLibrary;

boost::intrusive_ptr<movie_definition>
MovieFactory::makeMovie(std::auto_ptr<IOChannel> in, const std::string& url,
        const RunResources& runResources, bool startLoaderThread)
{
    boost::intrusive_ptr<movie_definition> ret;

    assert(in.get());

    // see if it's a jpeg or an swf
    FileType type = getFileType(*in);

    switch (type) {
        case GNASH_FILETYPE_JPEG:
        case GNASH_FILETYPE_PNG:
        case GNASH_FILETYPE_GIF:
        {
            if (!startLoaderThread) {
              log_unimpl(_("Requested to keep from completely loading "
                           "a movie, but the movie in question is an "
                           "image, for which we don't yet have the "
                           "concept of a 'loading thread'"));
            }
            ret = createBitmapMovie(in, url, runResources, type);
            break;
        }


        case GNASH_FILETYPE_SWF:
            ret = createSWFMovie(in, url, runResources, startLoaderThread);
            break;

        case GNASH_FILETYPE_FLV:
            log_unimpl(_("FLV can't be loaded directly as a movie"));
            return ret;

        default:
            log_error(_("Unknown file type"));
            break;
    }

    return ret;
}

// Try to load a movie from the given url, if we haven't
// loaded it already.  Add it to our library on success, and
// return a pointer to it.
boost::intrusive_ptr<movie_definition>
MovieFactory::makeMovie(const URL& url, const RunResources& runResources,
        const char* real_url, bool startLoaderThread,
        const std::string* postdata)
{
    boost::intrusive_ptr<movie_definition> mov;

    // Use real_url as label for cache if available 
    const std::string& cache_label = real_url ? URL(real_url).str() : url.str();

    // Is the movie already in the library? (don't check if we have post data!)
    if (!postdata) {
        if (movieLibrary.get(cache_label, &mov)) {
            log_debug("Movie %s already in library", cache_label);
            return mov;
        }
    }

    // Try to open a file under the filename, but DO NOT start
    // the loader thread now to avoid IMPORT tag loaders from 
    // calling createMovie() again and NOT finding
    // the just-created movie.
    mov = createNonLibraryMovie(url, runResources, real_url, false, postdata);

    if (!mov) {
        log_error(_("Couldn't load library movie '%s'"), url.str());
        return mov;
    }

    // Movie is good, add to the library, but not if we used POST
    if (!postdata) {
        movieLibrary.add(cache_label, mov.get());
        log_debug("Movie %s (SWF%d) added to library",
                cache_label, mov->get_version());
    }
    else {
        log_debug("Movie %s (SWF%d) NOT added to library (resulted from "
                    "a POST)", cache_label, mov->get_version());
    }

    /// Now complete the load if the movie is an SWF movie
    // 
    /// This is a no-op except for SWF movies.
    if (startLoaderThread) mov->completeLoad();

    return mov;
}

void
MovieFactory::clear()
{
    movieLibrary.clear();
}

namespace {

/// Get type of file looking at first bytes
FileType
getFileType(IOChannel& in)
{
    in.seek(0);

    char buf[3];
    
    if (in.read(buf, 3) < 3) {
        log_error(_("Can't read file header"));
        in.seek(0);
        return GNASH_FILETYPE_UNKNOWN;
    }
    
    // This is the magic number {0xff, 0xd8, 0xff} for JPEG format files
    if (std::equal(buf, buf + 3, "\xff\xd8\xff")) {
        in.seek(0);
        return GNASH_FILETYPE_JPEG;
    }

    // This is the magic number for any PNG format file
    // buf[3] == 'G' (we didn't read so far)
    if (std::equal(buf, buf + 3, "\x89PN")) {
        in.seek(0);
        return GNASH_FILETYPE_PNG;
    }

    // This is the magic number for any GIF format file
    if (std::equal(buf, buf + 3, "GIF")) {
        in.seek(0);
        return GNASH_FILETYPE_GIF;
    }

    // This is for SWF (FWS or CWS)
    if (std::equal(buf, buf + 3, "FWS") || std::equal(buf, buf + 3, "CWS")) {
        in.seek(0);
        return GNASH_FILETYPE_SWF;
    }

    // Take one guess at what this is. (It's an FLV-format file).
    if (std::equal(buf, buf + 3, "FLV")) {
        return GNASH_FILETYPE_FLV;
    }
    
    // Check if it is an swf embedded in a player (.exe-file)
    if (std::equal(buf, buf + 2, "MZ")) {

        if (in.read(buf, 3) < 3) {
            log_error(_("Can't read 3 bytes after an MZ (.exe) header"));
            in.seek(0);
            return GNASH_FILETYPE_UNKNOWN;
        }

        while ((buf[0]!='F' && buf[0]!='C') || buf[1]!='W' || buf[2]!='S') {
            buf[0] = buf[1];
            buf[1] = buf[2];
            buf[2] = in.read_byte();
            if (in.eof()) {
                log_error(_("Could not find SWF inside an .exe file"));
                in.seek(0);
                return GNASH_FILETYPE_UNKNOWN;
            }
        }
        in.seek(in.tell() - static_cast<std::streamoff>(3));
        return GNASH_FILETYPE_SWF;
    }

    log_error(_("unknown file type, buffer is %c%c%c"), buf[0], buf[1], buf[2]);
    return GNASH_FILETYPE_UNKNOWN;
}

// Create a SWFMovieDefinition from an SWF stream
// NOTE: this method assumes this *is* an SWF stream
boost::intrusive_ptr<SWFMovieDefinition>
createSWFMovie(std::auto_ptr<IOChannel> in, const std::string& url,
        const RunResources& runResources, bool startLoaderThread)
{

    boost::intrusive_ptr<SWFMovieDefinition> m = new SWFMovieDefinition(runResources);

    const std::string& absURL = URL(url).str();

    if (!m->readHeader(in, absURL)) return 0;
    if (startLoaderThread && !m->completeLoad()) return 0;

    return m;
}

// Create a movie_definition from an image format stream
// NOTE: this method assumes this *is* the format described in the
// FileType type
// TODO: The pp won't display PNGs for SWF7 or below.
boost::intrusive_ptr<BitmapMovieDefinition>
createBitmapMovie(std::auto_ptr<IOChannel> in, const std::string& url,
        const RunResources& r, FileType type)
{
    assert (in.get());

    boost::intrusive_ptr<BitmapMovieDefinition> ret;

    // readImageData takes a shared pointer because JPEG streams sometimes need
    // to transfer ownership.
    boost::shared_ptr<IOChannel> imageData(in.release());

    try {
        std::auto_ptr<image::GnashImage> im(
                image::Input::readImageData(imageData, type));

        if (!im.get()) {
            log_error(_("Can't read image file from %s"), url);
            return ret;
        }

        Renderer* renderer = r.renderer();
        ret = new BitmapMovieDefinition(im, renderer, url);
        return ret;

    }
    catch (const ParserException& e) {
        log_error(_("Parsing error: %s"), e.what());
        return ret;
    }

}

boost::intrusive_ptr<movie_definition>
createNonLibraryMovie(const URL& url, const RunResources& runResources,
        const char* reset_url, bool startLoaderThread,
        const std::string* postdata)
{
  
    boost::intrusive_ptr<movie_definition> ret;
  
    std::auto_ptr<IOChannel> in;
  
    const StreamProvider& streamProvider = runResources.streamProvider();
  
    const RcInitFile& rcfile = RcInitFile::getDefaultInstance();
  
    if (postdata) {
        in = streamProvider.getStream(url, *postdata, rcfile.saveLoadedMedia());
    }
    else in = streamProvider.getStream(url, rcfile.saveLoadedMedia());
  
    if (!in.get()) {
        log_error(_("failed to open '%s'; can't create movie"), url);
        return ret;
    }
    
    if (in->bad()) {
        log_error(_("streamProvider opener can't open '%s'"), url);
        return ret;
    }
  
    const std::string& movie_url = reset_url ? reset_url : url.str();
    ret = MovieFactory::makeMovie(in, movie_url, runResources,
            startLoaderThread);
  
    return ret;
  
}

} // unnamed namespace

} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
