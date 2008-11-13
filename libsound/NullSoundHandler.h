// NullSoundHandler - fake sound handler, for testing gnash
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


#ifndef NULL_SOUND_HANDLER_H
#define NULL_SOUND_HANDLER_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "sound_handler.h" // for inheritance
#include "SoundInfo.h" 
#include "dsodefs.h" // for DSOEXPORT

#include <vector>
#include <memory>
#include <cassert>
#include <cstring>

namespace gnash {
namespace sound {

/// Null sound_handler, for testing 
//
/// @todo start a thread to fetch samples ?
///
class DSOEXPORT NullSoundHandler : public sound_handler
{
public:
};
	
} // gnash.sound namespace 
} // namespace gnash

#endif // NULL_SOUND_HANDLER_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
