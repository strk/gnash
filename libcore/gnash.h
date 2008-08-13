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
	class sprite_instance; // for fscommand_callback typedef
	class movie_definition; // for create_movie
	class render_handler; // for set_render_handler 
	class URL; // for set_base_url
 	namespace media {
 		class sound_handler; // for set_sound_handler
 	}
}


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

// Sound callbacks stuff

/// \brief
/// Pass in a sound handler, so you can handle audio on behalf of
/// gnash.  This is optional; if you don't set a handler, or set
/// NULL, then sounds won't be played.
///
/// If you want sound support, you should set this at startup,
/// before loading or playing any movies!
///
DSOEXPORT void  set_sound_handler(media::sound_handler* s);

/// Get currently registered sound handler
DSOEXPORT media::sound_handler* get_sound_handler();

/// Set the render handler.  This is one of the first
/// things you should do to initialise the player (assuming you
/// want to display anything).
DSOEXPORT void set_render_handler(render_handler* s);

/// Set the base url against which to resolve relative urls
DSOEXPORT void set_base_url(const URL& url);

/// Return base url
DSOEXPORT const gnash::URL& get_base_url();

/// Signature of fscommand callback function
typedef void (*fscommand_callback)(sprite_instance* movie,
						const std::string& command, const std::string& arg);

/// Signature of interface event callback.
typedef std::string (*interfaceEventCallback)(const std::string& event,
                                              const std::string& arg);

// Some helpers that may or may not be compiled into your
// version of the library, depending on platform etc.
DSOEXPORT render_handler*   create_render_handler_xbox();
DSOEXPORT render_handler*   create_render_handler_ogl(bool init = true);
//DSOEXPORT render_handler* create_render_handler_cairo(void* cairohandle);

/// Create a gnash::movie_definition from the given URL.
//
/// The URL can correspond to either a JPEG or SWF file.
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
/// If real_url is given, the movie's url will be set to that value.
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
/// @param postdata
/// If not NULL, use POST method (only valid for HTTP)
///
movie_definition* create_movie(const URL& url, const char* real_url=NULL, bool startLoaderThread=true, const std::string* postdata=NULL);

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
/// derived from the IOChannel.
///
/// @param startLoaderThread
/// If false only the header will be read, and you'll need to call completeLoad
/// on the returned movie_definition to actually start it. This is tipically 
/// used to postpone parsing until a VirtualMachine is initialized.
/// Initializing the VirtualMachine requires a target SWF version, which can
/// be found in the SWF header.
///
DSOEXPORT movie_definition* create_movie(std::auto_ptr<IOChannel> in, const std::string& url, bool startLoaderThread=true);

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
	const char* real_url=NULL, bool startLoaderThread=true,
	const std::string* postdata=NULL);
    


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
