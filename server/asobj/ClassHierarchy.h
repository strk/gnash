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
#include "Namespace.h"

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

	/// The global namespace
	///
	/// Get the global namespace.  This is not the Global object -- it only
	/// contains the classes, not any globally available functions or anything
	/// else.
	Namespace *getGlobalNs() { return &mGlobalNamespace; }

	/// Find a namespace with the given uri.
	///
	/// @return 
	/// The namespace with the given uri or NULL if it doesn't exist.
	Namespace *findNamespace(string_table::key uri)
	{
		if (uri == 0)
			return getGlobalNs();

		std::map<string_table::key, Namespace>::iterator i;
		i = mNamespaces.find(uri);
		if (i == mNamespaces.end())
			return NULL;
		return &(i->second);
	}

	/// \brief
	/// Add a namespace to the set. Don't use to add unnamed namespaces.
	/// Will overwrite existing namespaces 'kind' and 'prefix' values. 
	/// Returns the added space.
	Namespace* addNamespace(string_table::key uri, Namespace::kinds kind)
	{ Namespace &n = mNamespaces[uri]; n.initialize(uri, 0, kind);
	  return &mNamespaces[uri];}

	/// Set the extension object, since it wasn't set on construction.
	void setExtension(Extension *e) { mExtension = e; }

	/// Set the global object, for registrations.
	void setGlobal(as_object *g) { mGlobal = g; }

	/// \brief
	/// Construct the declaration object. Later set the global and
	/// extension objects using setGlobal and setExtension
	ClassHierarchy() :
		mGlobal(NULL), mGlobalNamespace(), mExtension(NULL)
	{/**/}

private:
	as_object *mGlobal;
	Namespace mGlobalNamespace;
	Extension *mExtension;

	std::map<string_table::key, Namespace> mNamespaces;
};

} /* namespace gnash */
#endif /* GNASH_CLASS_HIERARCHY_H */

