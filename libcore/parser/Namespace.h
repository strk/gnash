// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_NAMESPACE_H
#define GNASH_NAMESPACE_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <map>
#include "string_table.h"
#include "as_object.h"

namespace gnash {

class abc_block;

/// A namespace for ActionScript. Not really functional in AS2.
///
/// A namespace contains the prototypes of the objects in its
/// namespace and nothing else.  In AS3 and up, namespaces are
/// not semantically nested. (Though they may look nested by their
/// names, this is not a true nesting as done by the bytecode.)
class Namespace
{
public:
	friend class abc_block; // Parser for abc_blocks.

	typedef enum
	{
		KIND_NORMAL = 0x08,
		KIND_PRIVATE = 0x05,
		KIND_PACKAGE = 0x16,
		KIND_PACKAGE_INTERNAL = 0x17,
		KIND_PROTECTED = 0x18,
		KIND_EXPLICIT = 0x19,
		KIND_STATIC_PROTECTED = 0x1A
	} kinds;

	/// the Uri is the name of the nameSpace.
	string_table::key getUri() const { return mUri; }

	/// the Prefix is used for XML, but not otherwise.
	string_table::key getPrefix() const { return mPrefix; }

	/// \brief
	/// Create a namespace with the given DisplayObjectistics. Such a namespace
	/// will be empty upon creation.
	Namespace(string_table::key uri, string_table::key prefix, kinds kind) :
		mUri(uri), mPrefix(prefix), mKind(kind), mMembers()
	{/**/}

	/// \brief
	/// Default constructor so that Namespaces can appear in containers.
	Namespace() : mUri(0), mPrefix(0), mKind(KIND_NORMAL), mMembers()
	 {/**/}

	/// \brief
	/// Initialize the namespace with the given values.  Does not clear
	/// the list of members of the namespace.
	void initialize(string_table::key uri, string_table::key prefix, kinds kind)
	{ mUri = uri; mPrefix = prefix; mKind = kind; }

	/// \brief
	/// Add a prototype
	///
	/// It is intentional that there is no facility for deleting a prototype.
	/// It doesn't make any sense, and this would cause multiple problems.
	/// If this functionality is needed, the whole namespace is probably
	/// invalidated, and the namespace can be discarded.
	///
	/// @param name
	/// The string table key corresponding to the unqualified name of the
	/// class.
	///
	/// @param obj
	/// The object which is the prototype of the named class.
	///
	/// @return
	/// true if the object is added, false if a different prototype object
	/// already existed.
	bool addPrototype(string_table::key name, as_object* obj)
	{
		std::map<string_table::key, as_object*>::iterator i =
			mMembers.find(name);

		if (i != mMembers.end())
			return false;

		mMembers[name] = obj;
		return true;
	}

	/// \brief
	/// Declare a class stub.
	bool stubPrototype(string_table::key name)
	{ return addPrototype(name, NULL); }

	/// \brief
	/// Has a prototype been either stubbed or entered?
	bool prototypeExists(string_table::key name)
	{ return mMembers.find(name) != mMembers.end(); }

	/// \brief
	/// Set a prototype for an existing stub.
	///
	/// The difference between this and addPrototype is that addPrototype
	/// will not allow the addition of the object if there is already an
	/// entry, and this will.
	///
	/// @param name
	/// The string table key of the unqualified name of the class.
	///
	/// @param obj
	/// The object which is the prototype of the named class.
	///
	/// @return
	/// true if the stub existed, otherwise false.
	/// The prototype is added either way.
	bool setPrototype(string_table::key name, as_object *obj)
	{
		std::map<string_table::key, as_object*>::iterator i =
			mMembers.find(name);
		if (i == mMembers.end())
		{
			mMembers[name] = obj;
			return false;
		}
		i->second = obj;
		return true;
	}

	/// \brief
	/// Get a prototype
	///
	/// Find a prototype object given its string table key. Returns an empty
	/// pointer if there is no such object. Because prototypes are unique,
	/// returns the actual prototype, not a copy. Might return NULL if the
	/// prototype is stubbed but not set.
	as_object* getPrototype(string_table::key name)
	{
		std::map<string_table::key, as_object*>::iterator i = 
			mMembers.find(name);

		if (i == mMembers.end())
			return NULL;

		return i->second;
	}

	/// \brief
	/// Mark reachable resources for GC.
	void markReachableResources() const;

private:
	string_table::key mUri;
	string_table::key mPrefix;
	kinds mKind;

	std::map<string_table::key, as_object*> mMembers;
};

inline void
Namespace::markReachableResources() const
{
	std::map<string_table::key, as_object*>::const_iterator i =
		mMembers.begin();

	for ( ; i != mMembers.end(); ++i)
	{
		if (i->second != NULL)
			i->second->setReachable();
	}
}

}; /* namespace gnash */
#endif /* GNASH_NAMESPACE_H */
