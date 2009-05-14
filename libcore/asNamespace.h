
#ifndef GNASH_AS_NAMESPACE_H
#define GNASH_AS_NAMESPACE_H

#include "string_table.h"

// Forward declarations
namespace gnash {
    class asClass;
}

namespace gnash {

/// Represent an ActionScript namespace
class asNamespace
{
public:
	void markReachableResources() const { /* TODO */ }

	/// Our parent (for protected)
	void setParent(asNamespace* p) { _parent = p; }

	asNamespace* getParent() { return _parent; }

	/// Set the uri
	void setURI(string_table::key name) { mUri = name; }

	/// What is the Uri of the namespace?
	string_table::key getURI() const { return mUri; }

	string_table::key getAbcURI() const {return _abcURI;}
	void setAbcURI(string_table::key n){ _abcURI = n; }

	/// What is the XML prefix?
	string_table::key getPrefix() const { return _prefix; }

	/// Create an empty namespace
	asNamespace()
        :
        _parent(0),
        mUri(0),
        _prefix(0),
        _abcURI(0),
        mClasses(),
		mRecursePrevent(false),
        _private(false),
        _protected(false)
	{/**/}

	/// Add a class to the namespace. The namespace stores this, but
	/// does not take ownership. (So don't delete it.)
	bool addClass(string_table::key name, asClass *a)
	{
		if (getClassInternal(name))
			return false;
		mClasses[static_cast<std::size_t>(name)] = a;
		return true;
	}

	void stubPrototype(string_table::key name);

	/// Get the named class. Returns NULL if information is not known
	/// about the class. (Stubbed classes still return NULL here.)
	asClass *getClass(string_table::key name) 
	{
		if (mRecursePrevent)
			return NULL;

		asClass *found = getClassInternal(name);
		if (found || !getParent())
			return found;

		mRecursePrevent = true;
		found = getParent()->getClass(name);
		mRecursePrevent = false;
		return found;
	}

	void setPrivate() { _private = true; }
	void unsetPrivate() { _private = false; }
	bool isPrivate() { return _private; }

	void setProtected() { _protected = true; }
	void unsetProtected() { _protected = false; }
	bool isProtected() { return _protected; }
	
private:
	asNamespace *_parent;
	string_table::key mUri;
	string_table::key _prefix;

	string_table::key _abcURI;

	typedef std::map<string_table::key, asClass*> container;
	container mClasses;
	mutable bool mRecursePrevent;

	bool _private;
	bool _protected;

	asClass* getClassInternal(string_table::key name) const
	{
		container::const_iterator i;
		if (mClasses.empty())
			return NULL;

		i = mClasses.find(name);
		if (i == mClasses.end())
			return NULL;
		return i->second;
	}
};

} // namespace gnash

#endif
