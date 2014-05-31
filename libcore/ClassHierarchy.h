// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#ifndef GNASH_CLASS_HIERARCHY_H
#define GNASH_CLASS_HIERARCHY_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <string>
#include <vector>

#include "ObjectURI.h"

namespace gnash {
    class Extension;
    class as_object;
}

namespace gnash {

/// Register all of the ActionScript classes, with their dependencies.
class ClassHierarchy
{
public:
	struct ExtensionClass
	{
		/// The filename for the library relative to the plugins directory.
		std::string file_name;

		/// Initialization function name
		///
		/// The name of the function which will yield the prototype
		/// object. It should be a function with signature:
		/// void init_name(as_object &obj);
		/// which sets its prototype as the member 'name' in the
		/// object. See extensions/mysql/mysql_db.cpp function
		/// mysql_class_init
		std::string init_name;

        const ObjectURI uri;

		/// The version at which this should be added.
		int version;
	};

	struct NativeClass
	{
		
        /// The type of function to use for initialization
		typedef void (*InitFunc)(as_object& obj, const ObjectURI& uri);

        NativeClass(InitFunc init, ObjectURI u, int ver)
            :
            initializer(init),
            uri(std::move(u)),
            version(ver)
        {}

		/// The initialization function
		///
		/// See ExtensionClass.init_name for the necessary function.
		InitFunc initializer;

		/// The name of the class.
		const ObjectURI uri;

		/// The version at which this should be visible.
		int version;
	};
	
    /// \brief
	/// Construct the declaration object. Later set the global and
	/// extension objects using setGlobal and setExtension
	ClassHierarchy(as_object* global)
        :
		mGlobal(global)
	{}

	/// \brief
	/// Delete our private namespaces.
	~ClassHierarchy();

    typedef std::vector<NativeClass> NativeClasses;

	/// Declare an ActionScript class and how to instantiate it from the core.
	///
	/// @param c
	/// The NativeClass structure which defines the class.
	///
	/// @return true, unless the class with c.name already existed.
	bool declareClass(const NativeClass& c);

	/// Declare a list of native classes.
	void declareAll(const NativeClasses& classes);

	/// Mark objects for garbage collector.
	void markReachableResources() const {}

private:
	as_object* mGlobal;
};

} 
#endif 

