// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#ifndef GNASH_ASOBJ_STAGE_H
#define GNASH_ASOBJ_STAGE_H

namespace gnash {

struct ObjectURI;
class as_object;

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

/// Register native functions with the VM.
void registerStageNative(as_object& o);

/// Initialize the global Stage class
void stage_class_init(as_object& where, const ObjectURI& uri);

} // end of gnash namespace

#endif

