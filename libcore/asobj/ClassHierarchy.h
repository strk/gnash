// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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

#include <list>
#include <vector>
#include <ostream>

#include "as_object.h"
#include "asClass.h"
#include "SafeStack.h"

namespace gnash {

class Extension;
class asClass;
class asMethod;
class asException;
class asBoundValue;
class asBoundAccessor;
class as_object;

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
		/// The name of the namespace in which this belongs.
		string_table::key namespace_name;

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
		/// The name of the namespace in which this belongs.
		string_table::key namespace_name;

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
	bool declareClass(const nativeClass& c);

	/// \brief
	/// Declare all of the native and extension classes from the
	/// tables contained in the source file.
	///
	void massDeclare();

	/// The global namespace
	///
	/// Get the global namespace.  This is not the Global object -- it only
	/// contains the classes, not any globally available functions or anything
	/// else.
	asNamespace* getGlobalNs() { return mGlobalNamespace; }

	// Chad: Document
	as_object* newOfType(string_table::key /*whattype*/) { return NULL; }

	/// Find a namespace with the given uri.
	///
	/// @return 
	/// The namespace with the given uri or NULL if it doesn't exist.
	asNamespace *findNamespace(string_table::key uri)
	{
		namespacesContainer::iterator i;
		if (mNamespaces.empty())
			return NULL;
		i = mNamespaces.find(uri);
		if (i == mNamespaces.end())
			return NULL;
		return &i->second;
	}

	/// \brief
	/// Obtain a new anonymous namespace. Use this to let the object keep track
	/// of all namespaces, even private ones. Namespaces obtained in this way
	/// can't ever be found. (They must be kept and passed to the appropriate
	/// objects.)
	///
	asNamespace* anonNamespace(string_table::key uri)
	{
		mAnonNamespaces.grow(1); 
		asNamespace *n = &mAnonNamespaces.top(0); 
		n->setURI(uri); 
		return n; 
	}

	/// \brief
	/// Add a namespace to the set. Don't use to add unnamed namespaces.
	/// Will overwrite existing namespaces 'kind' and 'prefix' values. 
	/// Returns the added space.
	asNamespace* addNamespace(string_table::key uri)
	{
		asNamespace *n = findNamespace(uri);
		if (n)
			return n;
		// The set should create it automatically here. TODO: Make sure
		mNamespaces[uri].setURI(uri);
		return &mNamespaces[uri];
	}

	/// Set the extension object, since it wasn't set on construction.
	void setExtension(Extension *e) { mExtension = e; }

	/// Set the global object, for registrations.
	void setGlobal(as_object *g) { mGlobal = g; }

	/// Mark objects for garbage collector.
	void markReachableResources() const;

	/// Create a new asClass object for use.
	asClass *newClass()
	{ mClassMemory.grow(1); return &mClassMemory.top(0); }

	asException *newException()
	{ mExceptionMemory.grow(1); return &mExceptionMemory.top(0); }

	/// Create a new asMethod object for use.
	asMethod *newMethod()
	{ mMethodMemory.grow(1); return &mMethodMemory.top(0); }

	asBoundValue *newBoundValue()
	{ mBoundValueMemory.grow(1); return &mBoundValueMemory.top(0); }

	asBoundAccessor *newBoundAccessor()
	{ mBoundAccessorMemory.grow(1); return &mBoundAccessorMemory.top(0); }

	/// \brief
	/// Construct the declaration object. Later set the global and
	/// extension objects using setGlobal and setExtension
	ClassHierarchy() :
		mGlobal(NULL), mGlobalNamespace(NULL), mExtension(NULL),
		mAnonNamespaces(),
		mClassMemory(), mExceptionMemory(),
		mMethodMemory(),
		mBoundValueMemory(), mBoundAccessorMemory()
	{ mGlobalNamespace = anonNamespace(0); }

	/// \brief
	/// Delete our private namespaces.
	~ClassHierarchy();

private:
	as_object *mGlobal;
	asNamespace *mGlobalNamespace;
	Extension *mExtension;

	typedef std::map<string_table::key, asNamespace> namespacesContainer;
	namespacesContainer mNamespaces;
	SafeStack<asNamespace> mAnonNamespaces;
	SafeStack<asClass> mClassMemory;
	SafeStack<asException> mExceptionMemory;
	SafeStack<asMethod> mMethodMemory;
	SafeStack<asBoundValue> mBoundValueMemory;
	SafeStack<asBoundAccessor> mBoundAccessorMemory;
};

std::ostream& operator << (std::ostream& os, const ClassHierarchy::nativeClass& c);
std::ostream& operator << (std::ostream& os, const ClassHierarchy::extensionClass& c);

} /* namespace gnash */
#endif /* GNASH_CLASS_HIERARCHY_H */

