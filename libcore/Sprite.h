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

//------------------------------------------------------------------
// NOTE: this header is still here just as a place-holder for
//       Doxygen page about Sprites
//------------------------------------------------------------------

/// \page Sprite Sprites and MovieClips
///
/// A Sprite, or MovieClip, is a mini movie-within-a-movie. 
///
/// It doesn't define its own DisplayObjects;
/// it uses the DisplayObjects from the parent
/// movie, but it has its own frame counter, display list, etc.
///
/// @@ are we sure it doesn't define its own chars ?
///
/// The sprite implementation is divided into 
/// gnash::sprite_definition and gnash::MovieClip.
///
/// The _definition holds the immutable data for a sprite (as read
/// from an SWF stream), while the _instance contains the state for
/// a specific run of if (frame being played, mouse state, timers,
/// display list as updated by actions, ...)
///

