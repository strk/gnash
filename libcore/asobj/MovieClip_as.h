// MovieClip_as.h:  ActionScript "MovieClip" class, for Gnash.
//
//   Copyright (C) 2009, 2010, 2011 Free Software Foundation, Inc.
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

#ifndef GNASH_ASOBJ3_MOVIECLIP_H
#define GNASH_ASOBJ3_MOVIECLIP_H


namespace gnash {

// Forward declarations
class as_object;
struct ObjectURI;
class DisplayObject;
class SWFMatrix;

/// Initialize the global MovieClip class
void movieclip_class_init(as_object& where, const ObjectURI& uri);

/// Register ASNative MovieClip methods (AS2 only).
void registerMovieClipNative(as_object& where);

} // gnash namespace

#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

