// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


#include "sound_handler.h"

namespace gnash {

namespace globals {

// Callback interface to host, for handling sounds.  If it's NULL,
// sound is ignored.
sound_handler*	s_sound_handler = 0;

} // namespace gnash::global


void	set_sound_handler(sound_handler* s)
// Called by host, to set a handler for all sounds.
// Can pass in 0 to disable sound.
{
	globals::s_sound_handler = s;
}


sound_handler*	get_sound_handler()
{
	return globals::s_sound_handler;
}


} // namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
