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


#ifndef GNASH_MOVIE_FACTORY_H
#define GNASH_MOVIE_FACTORY_H

#include "dsodefs.h"

#include <string>
#include <memory>

namespace gnash {
    class IOChannel;
    class RunResources;
    class movie_definition;
    class URL;
}

namespace gnash {

class MovieFactory
{
public:
   
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
    /// A RunResources containing resources needed for parsing, such as the
    /// base URL for the run, the sound::sound_handler, and a StreamProvider.
    ///
    /// @param real_url
    /// The url to encode as the _url member of the resulting
    /// movie definition. Use NULL if it is not different from
    /// the actual url (default). This is used to simulate a run from
    /// the official publication url.
    ///
    /// @param startLoaderThread
    /// If false only the header will be read, and you'll need to call
    /// completeLoad on the returned movie_definition to actually start it.
    /// This is typically used to postpone parsing until a VirtualMachine
    /// is initialized. Initializing the VirtualMachine requires a target
    /// SWF version, which can be found in the SWF header.
    ///
    /// @param postdata
    /// If not NULL, use POST method (only valid for HTTP).
    /// NOTE: when POSTing, the movies library won't be used.
    static DSOEXPORT movie_definition* makeMovie(const URL& url,
        const RunResources& runInfo, const char* real_url = NULL,
        bool startLoaderThread = true, const std::string* postdata = NULL);
    
    /// Load a movie from an already opened stream.
    //
    /// The movie can be both an SWF or JPEG, the url parameter
    /// will be used to set the _url member of the resulting object.
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
    /// @param runInfo
    /// A RunResources containing resources needed for parsing, such as the
    /// base URL for the run, the sound::sound_handler, and a StreamProvider.
    ///
    /// @param startLoaderThread
    /// If false only the header will be read, and you'll need to call
    /// completeLoad on the returned movie_definition to actually start it.
    /// This is typically used to postpone parsing until a VirtualMachine
    /// is initialized. Initializing the VirtualMachine requires a target
    /// SWF version, which can be found in the SWF header.
    static DSOEXPORT movie_definition* makeMovie(std::auto_ptr<IOChannel> in,
            const std::string& url, const RunResources& runInfo,
            bool startLoaderThread);
};

} // namespace gnash


#endif // GNASH_IMPL_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
