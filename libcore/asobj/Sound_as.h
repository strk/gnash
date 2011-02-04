// Sound_as.h:  ActionScript 3 "Sound" class, for Gnash.
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

#ifndef GNASH_ASOBJ3_SOUND_H
#define GNASH_ASOBJ3_SOUND_H

namespace gnash {
    struct ObjectURI;
    class as_object;
}

namespace gnash {

void sound_class_init(as_object& where, const ObjectURI& uri);

void registerSoundNative(as_object& global);

} // gnash namespace

#endif

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:

