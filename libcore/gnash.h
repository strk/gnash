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

/// \mainpage
///
///  See Related Pages for movies and sprites informations
///

#ifndef GNASH_H
#define GNASH_H

#include "dsodefs.h"

#include <memory> // for auto_ptr
#include <string>

namespace gnash {
	class IOChannel;
	class movie_definition; // for create_movie
	class render_handler; // for set_render_handler 
	class URL; // for set_base_url
    class RunInfo;
}


/// Freedom bites
namespace gnash {


// The file types that Gnash can handle
enum FileType {
    GNASH_FILETYPE_JPEG,
    GNASH_FILETYPE_PNG,
    GNASH_FILETYPE_GIF,
    GNASH_FILETYPE_SWF,
    GNASH_FILETYPE_FLV,
    GNASH_FILETYPE_UNKNOWN
};

/// The display quality.
//
/// Required for rendering and core.
enum Quality
{
    QUALITY_LOW,
    QUALITY_MEDIUM,
    QUALITY_HIGH,
    QUALITY_BEST
};

// Sound callbacks stuff

/// Set the render handler.  This is one of the first
/// things you should do to initialise the player (assuming you
/// want to display anything).
DSOEXPORT void set_render_handler(render_handler* s);

// Some helpers that may or may not be compiled into your
// version of the library, depending on platform etc.
DSOEXPORT render_handler*   create_render_handler_ogl(bool init = true);

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
/// If real_url is given, the movie's url will be set to that value.
///
/// @param url
/// The URL to load the movie from.
///
/// @param runInfo
/// A RunInfo containing resources needed for parsing, such as the
/// base URL for the run, the sound::sound_handler, and a StreamProvider.
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
/// @param postdata
/// If not NULL, use POST method (only valid for HTTP).
/// NOTE: when POSTing, the movies library won't be used.
///
DSOEXPORT movie_definition* create_library_movie(const URL& url,
        const RunInfo& runInfo, const char* real_url = NULL,
        bool startLoaderThread = true, const std::string* postdata = NULL);

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
DSOEXPORT void clear();

}   // namespace gnash

#endif // GNASH_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
