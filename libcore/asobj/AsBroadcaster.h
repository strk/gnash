// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#ifndef GNASH_ASBROADCASTER_H
#define GNASH_ASBROADCASTER_H

// Forward declarations
namespace gnash {
	class as_value;
	class as_object;
	class fn_call;
    struct ObjectURI;
}

namespace gnash {
  
/// AsBroadcaster facilities
class AsBroadcaster {

public:

	/// Initialize the given object as an AsBroadcaster
	//
	/// This method set the addListener,removeListener and broadcastMessage
	/// AS methods with the object, and set the _listners array member.
	///
	/// It is exposed so that Stage,TextField,Key,Mouse and Selection
	/// can call this internally.
	///
	/// The AsBroadcaster_init will take care of registering
	/// the _global.AsBroadcaster object and its 'initialize'
	/// method for user-defined broadcasters initialization
	///
	static void initialize(as_object& obj);

	/// Return the global AsBroadcaster
	/// (the native one, immune to any override)
	///
	static as_object* getAsBroadcaster();

    static void registerNative(as_object &global);

    static void init(as_object& global, const ObjectURI& uri);

};

} // end of gnash namespace

#endif

