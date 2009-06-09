// Stage_as.h:  ActionScript 3 "Stage" class, for Gnash.
//
//   Copyright (C) 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_ASOBJ3_STAGE_H
#define GNASH_ASOBJ3_STAGE_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

// ADDED
#include "as_object.h" // for inheritance
#include "movie_root.h" // for access to scaleMode

#include <list>

// Forward declarations
class as_object;

namespace gnash {
	
// ADDED
/// This is the Stage ActionScript object.
//
/// Some Stage methods are implemented in movie_root, because
/// it provides the interface to the Gui and/or all the values
/// required are necessarily in movie_root:
///
/// - scaleMode
/// - width
/// - height
/// - displayState
/// - alignMode
//
/// Most functions are ASnative, which means they cannot rely on
/// the existence of a load-on-demand Stage object. Only resize events
/// appear to need this (not ASnative). The ASnative functions
/// are available from SWF5

class Stage_as: public as_object
{

public:
    
	Stage_as();
	
	/// Notify all listeners about a resize event
	void notifyResize();
	
	void notifyFullScreen(bool fs);

};

/// Initialize the global Stage class
void stage_class_init(as_object& global);

// ADDED
void registerStageNative(as_object& o);

} // gnash namespace

// GNASH_ASOBJ3_STAGE_H
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

