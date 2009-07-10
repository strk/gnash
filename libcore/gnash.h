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

#include <string>

namespace gnash {
	class IOChannel;
	class movie_definition; // for create_movie
	class Renderer; // for set_Renderer 
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
DSOEXPORT void set_Renderer(Renderer* s);

// Some helpers that may or may not be compiled into your
// version of the library, depending on platform etc.
DSOEXPORT Renderer*   create_Renderer_ogl(bool init = true);

/// Initialize gnash core library
//
DSOEXPORT void gnashInit();

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
