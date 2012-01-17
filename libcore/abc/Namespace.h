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

#ifndef GNASH_AS_NAMESPACE_H
#define GNASH_AS_NAMESPACE_H

#include "string_table.h"
#include <map>

// Forward declarations
namespace gnash {
    namespace abc {
        class Class;
    }
    class ClassHierarchy;
    class string_table;
}

namespace gnash {
namespace abc {

/// Represent an ActionScript Namespace
//
/// Namespaces are generally global resources, although private Namespaces
/// are only visible inside a single AbcBlock.
//
/// Because there is no guarantee that a Namespace is private to an AbcBlock,
/// they must never store any AbcBlock-internal information, particularly
/// not the AbcURI.
//
/// A Namespace is currently a collection of Scripts, which should be 
/// turned into a Class on first use.
class Namespace
{
public:

	/// Create an empty namespace
	Namespace()
        :
        _parent(0),
        _uri(0),
        _prefix(0),
        _scripts(),
		mRecursePrevent(false),
        _private(false),
        _protected(false),
        _package(false)
	{}

    void markReachableResources() const { /* TODO */ }

	/// Our parent (for protected)
	void setParent(Namespace* p) { _parent = p; }

	Namespace* getParent() { return _parent; }

	/// Set the uri
	void setURI(string_table::key name) { _uri = name; }

	/// What is the Uri of the namespace?
	string_table::key getURI() const { return _uri; }

	/// What is the XML prefix?
	string_table::key getPrefix() const { return _prefix; }

	/// Add a Class to the namespace. The namespace stores this, but
	/// does not take ownership.
	bool addScript(string_table::key name, Class* a)
	{
		if (getScriptInternal(name)) return false;
		_scripts[static_cast<std::size_t>(name)] = a;
		return true;
	}

	void stubPrototype(ClassHierarchy& ch, string_table::key name);

	/// Get the named class. Returns NULL if information is not known
	/// about the class. (Stubbed classes still return NULL here.)
	Class* getScript(string_table::key name) 
	{
		if (mRecursePrevent) return NULL;

		Class* found = getScriptInternal(name);

		if (found || !getParent()) return found;

		mRecursePrevent = true;
		found = getParent()->getScript(name);
		mRecursePrevent = false;
		return found;
	}

    void dump(string_table& st);

	void setPrivate() { _private = true; }
	void unsetPrivate() { _private = false; }
	bool isPrivate() { return _private; }

	void setProtected() { _protected = true; }
	void unsetProtected() { _protected = false; }
	bool isProtected() { return _protected; }
	
    void setPackage() { _package = true; }
	void unsetPackage() { _package = false; }
	bool isPackage() { return _package; }
	
private:

	Namespace* _parent;
	string_table::key _uri;
	string_table::key _prefix;

	typedef std::map<string_table::key, Class*> container;
	container _scripts;
	mutable bool mRecursePrevent;

	bool _private;
	bool _protected;
	bool _package;

	Class* getScriptInternal(string_table::key name) const
	{
		container::const_iterator i;
		
        if (_scripts.empty()) return NULL;

		i = _scripts.find(name);

		if (i == _scripts.end()) return NULL;
		return i->second;
	}
};

} // namespace abc
} // namespace gnash

#endif
