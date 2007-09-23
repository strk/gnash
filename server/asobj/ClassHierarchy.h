// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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

#include "as_object.h"

namespace gnash {

class Extension;

/// Register all of the ActionScript classes, with their dependencies.
class ClassHierarchy
{
public:
	struct extensionClass
	{
		/// \brief
		/// The file name which contains the library, relative to the 
		/// plugins directory.
		std::string file_name;

		/// \brief Initialization function name
		///
		/// The name of the function which will yield the prototype
		/// object. It should be a function with signature:
		/// void init_name(as_object &obj);
		/// which sets its prototype as the member 'name' in the
		/// object. See extensions/mysql/mysql_db.cpp function
		/// mysql_class_init
		std::string init_name;

		/// \brief The name of the class.
		string_table::key name;

		/// \brief
		/// The name of the inherited class.
		/// Ordinarily should be CLASS_OBJECT
		string_table::key super_name;

		/// \brief
		/// The version at which this should be added.
		int version;
	};

	struct nativeClass
	{
		/// The type of function to use for initing.
		typedef void (*init_func)(as_object& obj);

		/// \brief
		/// The initialization function
		///
		/// See extensionClass.init_name for the necessary function.
		init_func initializer;

		/// The name of the class.
		string_table::key name;

		/// \brief
		/// The name of the inherited class. Object is assumed if
		/// none is given. (Unless name is itself Object)
		string_table::key super_name;

		/// \brief
		/// The version at which this should be added.
		int version;
	};

	/// \brief
	/// Declare an ActionScript class, with information on how
	/// to load it from an extension.
	///
	/// @param c
	/// The extensionClass structure which defines the class.
	///
	/// @return true, unless the class with c.name already existed.
	bool declareClass(extensionClass& c);

	/// \brief
	/// Declare an ActionScript class, with information on how
	/// to instantiate it from the core.
	///
	/// @param c
	/// The nativeClass structure which defines the class.
	///
	/// @return true, unless the class with c.name already existed.
	bool declareClass(nativeClass& c);

	/// \brief
	/// Declare all of the native and extension classes from the
	/// tables contained in the source file.
	///
	void massDeclare(int version);

	/// \brief
	/// Construct the declaration object with the given global object
	/// and extension object.
	ClassHierarchy(as_object *global, Extension *e) :
		mGlobal(global), mExtension(e)
	{/**/}

private:
	as_object *mGlobal;
	Extension *mExtension;
};

} /* namespace gnash */
#endif /* GNASH_CLASS_HIERARCHY_H */

