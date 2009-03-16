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

#ifndef GNASH_AS_CLASS_H
#define GNASH_AS_CLASS_H

#include <list>
#include <map>
#include <vector>
#include "string_table.h"
#include "as_value.h"
#include "CodeStream.h"
#include "Property.h"
#include "as_function.h"

namespace gnash {

class as_function;
class asNamespace;
class asMethod;
class asClass;
class asException;
typedef Property asBinding;
//class asBinding;
class asBoundValue;
class asBoundAccessor;
class ClassHierarchy;
class Property;

class asName;

class asException
{
public:
	void setStart(boost::uint32_t i) { mStart = i; }
	void setEnd(boost::uint32_t i) { mEnd = i; }
	void setCatch(boost::uint32_t i) { mCatch = i; }
	void catchAny() { mCatchAny = true; }
	void setCatchType(asClass* p) { mCatchType = p; }
	void setNamespace(asNamespace* n) { mNamespace = n; }
	void setName(string_table::key name) { mName = name; }

private:
	boost::uint32_t mStart;
	boost::uint32_t mEnd;
	boost::uint32_t mCatch;
	bool mCatchAny;
	asClass *mCatchType;
	asNamespace *mNamespace;
	string_table::key mName;
};
#if 0
/// An abstract binding for ActionScript.
class asBinding
{
public:
	void dump(string_table::key name);

	void setConst() { mConst = true; }
	void unsetConst() { mConst = false; }
	bool isConst() { return mConst; }

	void setStatic() { mStatic = true; }
	void unsetStatic() { mStatic = false; }
	bool isStatic() { return mStatic; }

	boost::uint32_t getSlotId() { return mSlotId; }
	void setSlotId(boost::uint32_t s) { mSlotId = s; }

	// Chad: Document
	string_table::key getName() { return mName; }
	void setLexOnly(bool) { /* TODO */ }

	/// If true, the attribute has a setter, but no getter.
	bool isWriteOnly();

	/// If true, the attribute can be read, but not written. Might be
	/// constant or might have a getter but no setter.
	bool isReadOnly(); 

	bool isGetSet() { return getAccessor() != NULL; }

	// Conversion from Property
	asBinding(Property *, string_table::key name);

	// As a member method.
	asBinding(asNamespace *ns, asMethod *pMethod, bool isstatic = false) :
		mNamespace(ns), mType(T_METHOD), mSlotId(0), mConst(true),
		mStatic(isstatic),	mMethod(pMethod)
	{/**/}

	// As an assignable function.
	asBinding(asNamespace *ns, asMethod *pMethod, boost::uint32_t sid, bool isstatic) :
		mNamespace(ns), mType(T_METHOD), mSlotId(sid), mConst(false),
		mStatic(isstatic),	mMethod(pMethod)
	{/**/}

	asBinding(asNamespace *ns, asBoundValue *pValue, boost::uint32_t sid, bool isconstant,
		bool isstatic) : mNamespace(ns), mType(T_VALUE), mSlotId(sid), mConst(isconstant),
		mStatic(isstatic), mValue(pValue)
	{/**/}

	asBinding(asNamespace *ns, asBoundValue *pValue, boost::uint32_t sid, bool isstatic) :
		mNamespace(ns), mType(T_VALUE), mSlotId(sid), mConst(false), mStatic(isstatic),
		mValue(pValue)
	{/**/}

	asBinding(asNamespace *ns, asBoundAccessor *pAccess, bool isstatic) :
		mNamespace(ns), mType(T_ACCESS), mSlotId(0), mConst(true), mStatic(isstatic),
		mAccess(pAccess)
	{/**/}

	asBinding(asNamespace *ns, asClass *pClass, boost::uint32_t sid, bool isstatic) :
		mNamespace(ns), mType(T_CLASS), mSlotId(sid), mConst(true), mStatic(isstatic),
		mClass(pClass)
	{/**/}

	asBinding() : mNamespace(NULL), mType(T_CLASS), mSlotId(0), mConst(false), mStatic(false),
		mClass(NULL)
	{/**/}

	asBinding(asMethod *);
	asBinding(as_function *);

	void reset(asBoundAccessor *pAccess, bool isstatic)
	{
		mType = T_ACCESS;
		mAccess = pAccess;
		mConst = true;
		mStatic = isstatic;
	}

	asBoundAccessor* getAccessor()
	{ return mType == T_ACCESS ? mAccess : NULL; }

	asBoundValue* getValue()
	{ return mType == T_VALUE ? mValue : NULL; }

	asMethod* getMethod()
	{ return mType == T_METHOD ? mMethod : NULL; }

	asClass* getClass()
	{ return mType == T_CLASS ? mClass : NULL; }

	as_function* getASFunction();

	asNamespace *mNamespace;

	typedef enum
	{
		T_CLASS,
		T_METHOD,
		T_AS_FUNCTION,
		T_VALUE,
		T_ACCESS
	} types;
	types mType;

	boost::uint32_t mSlotId;
	bool mConst;
	bool mStatic;
	string_table::key mName;
	as_object* mOwner;

	union
	{
		asClass *mClass;
		asMethod *mMethod;
		asBoundValue *mValue;
		asBoundAccessor *mAccess;
	};
};
#endif // comment out of asBinding

/// Represent an ActionScript namespace
class asNamespace
{
public:
	void markReachableResources() const { /* TODO */ }

	/// Our parent (for protected)
	void setParent(asNamespace* p) { mParent = p; }

	asNamespace* getParent() { return mParent; }

	/// Set the uri
	void setURI(string_table::key name) { mUri = name; }

	/// What is the Uri of the namespace?
	string_table::key getURI() const { return mUri; }

	/// What is the XML prefix?
	string_table::key getPrefix() const { return mPrefix; }

	/// Create an empty namespace
	asNamespace() : mParent(NULL), mUri(0), mPrefix(0), mClasses(),
		mRecursePrevent(false), mPrivate(false), mProtected(false)
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

	void setPrivate() { mPrivate = true; }
	void unsetPrivate() { mPrivate = false; }
	bool isPrivate() { return mPrivate; }

	void setProtected() { mProtected = true; }
	void unsetProtected() { mProtected = false; }
	bool isProtected() { return mProtected; }
	
private:
	asNamespace *mParent;
	string_table::key mUri;
	string_table::key mPrefix;

	typedef std::map<string_table::key, asClass*> container;
	container mClasses;
	mutable bool mRecursePrevent;

	bool mPrivate;
	bool mProtected;

	asClass *getClassInternal(string_table::key name) const
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

class asBoundValue;

class asBoundAccessor
{
public:
	bool setGetter(asMethod *p) { mGetter = p; return true; }
	bool setSetter(asMethod *p) { mSetter = p; return true; }
	bool setValue(asBoundValue *p) { mValue = p; return true; }

	asBoundValue* getValue() { return mValue; }
	asMethod *getGetter() { return mGetter; }
	asMethod *getSetter() { return mSetter; }

private:
	asMethod *mGetter;
	asMethod *mSetter;
	asBoundValue *mValue;
};

class asBoundValue 
{
public:
	asBoundValue() : mConst(false), mValue()
	{ mValue.set_undefined(); }
	void setValue(as_value &v) { mValue = v; }
	as_value getCurrentValue() { return mValue; }

	void setType(asClass *t) { mType = t; }
	asClass *getType() { return mType; }

private:
	bool mConst;
	asClass *mType;
	as_value mValue;
};

/// A class to represent, abstractly, an ActionScript method.
///
/// Methods are unnamed until they are bound to an object.
class asMethod
{
private:
	as_function* mPrototype;

	typedef enum
	{
		FLAGS_FINAL = 0x01,
		FLAGS_PROTECTED = 0x02,
		FLAGS_PUBLIC = 0x04,
		FLAGS_PRIVATE = 0x08
	} flags;
	/// A list of type identifiers
	typedef std::list<asClass*> argumentList;
	typedef std::map<string_table::key, asBinding> binding_container;

	int mMinArguments;
	int mMaxArguments;
	bool mIsNative;
	argumentList mArguments;
	std::list<as_value> mOptionalArguments;
	as_function *mImplementation;
	unsigned char mFlags;
	CodeStream *mBody;

	bool addBinding(string_table::key name, asBinding b);

public:
	as_function* getPrototype() { return mPrototype; }

	asBinding* getBinding(string_table::key name);

	asMethod();

	bool isNative() { return mIsNative; }
	bool hasBody() const { return mBody != NULL; }

	as_object* construct(as_object* /*base_scope*/) { /* TODO */ return NULL; }

	bool hasActivation();

	CodeStream *getBody() { return mBody; }
	void setBody(CodeStream *b) { mBody = b; }

	bool addValue(string_table::key name, asNamespace *ns, boost::uint32_t slotId,
		asClass *type, as_value& val, bool isconst);

	bool addSlot(string_table::key name, asNamespace *ns, boost::uint32_t slotId,
		asClass *type);

	bool addMethod(string_table::key name, asNamespace *ns, asMethod *method);

	bool addGetter(string_table::key name, asNamespace *ns, asMethod *method);

	bool addSetter(string_table::key name, asNamespace *ns, asMethod *method);

	bool addMemberClass(string_table::key name, asNamespace *ns,
		boost::uint32_t slotId, asClass *type);
	
	bool addSlotFunction(string_table::key name, asNamespace *ns,
		boost::uint32_t slotId, asMethod *method);

	/// \brief
	/// Set the owner of this method.
	void setOwner(asClass* s);

	/// \brief
	/// Get the unique identifier for the return type. 0 is 'anything'.
	/// (This is the value of any dynamic property.)
	/// Id reference: Type
	asClass* getReturnType() const;

	/// Set the return type
	void setReturnType(asClass* t);

	asMethod *getSuper();

	void setSuper(asMethod* s);

	/// \brief
	/// Is the method final? If so, it may not be overridden.
	bool isFinal() const { return mFlags & FLAGS_FINAL; }

	/// \brief
	/// Set the method as final.
	void setFinal() { mFlags = mFlags | FLAGS_FINAL; }

	/// \brief
	/// Unset the method as final. Not final anymore.
	void unsetFinal() { mFlags = mFlags & ~FLAGS_FINAL; }

	/// \brief
	/// Is the method private?
	bool isPrivate() const { return mFlags & FLAGS_PRIVATE; }

	/// \brief
	/// Make the method private.
	void setPrivate()
	{ mFlags = (mFlags & ~(FLAGS_PUBLIC | FLAGS_PROTECTED)) | FLAGS_PRIVATE; }

	/// \brief
	/// Is the method protected?
	bool isProtected() const { return mFlags & FLAGS_PROTECTED; }

	/// \brief
	/// Make the method protected.
	void setProtected()
	{ mFlags = (mFlags & ~(FLAGS_PUBLIC | FLAGS_PRIVATE)) | FLAGS_PROTECTED; }

	/// \brief
	/// Is the method public?
	bool isPublic() const { return mFlags & FLAGS_PUBLIC; }

	/// \brief
	/// Make the method public.
	void setPublic()
	{ mFlags = (mFlags & ~(FLAGS_PRIVATE | FLAGS_PROTECTED)) | FLAGS_PUBLIC; }

	/// \brief
	/// How many arguments are required? -1 means unknown.
	int minArgumentCount() const { return mMinArguments; }

	/// \brief
	/// Set the required minimum arguments.
	void setMinArgumentCount(int i) { mMinArguments = i; }

	/// \brief
	/// How many arguments are allowed? -1 means unknown.
	int maxArgumentCount() const { return mMaxArguments; }

	/// Set the required maximum arguments.
	void setMaxArgumentCount(int i) { mMaxArguments = i; }

	/// \brief
	/// Push an argument of type t into the method definition
	void pushArgument(asClass *t) { mArguments.push_back(t); }

	/// \brief
	/// Push an optional argument's default value.
	void pushOptional(const as_value& v) { mOptionalArguments.push_back(v); }

	/// \brief
	/// Are any of the arguments optional?
	bool optionalArguments() const
	{ return minArgumentCount() != maxArgumentCount(); }

	/// \brief
	/// Get a reference to a list of argument types.
	argumentList& getArgumentList() { return mArguments; }

	/// \brief
	/// Get an object capable of executing this function.
	/// Note: This may be NULL, because we might have information about this
	/// function but not actually have it yet.
	as_function* getImplementation() { return mImplementation; }
};

/// A class to represent, abstractly, ActionScript prototypes.
///
/// This class is intended to be able to capture the structure of an
/// ActionScript prototype as a type, rather than as an object. This is
/// contrary to the spirit of ActionScript as a dynamic language, but it is
/// incredibly helpful to an interpreter for that language.
class asClass
{
public:
	as_object* getPrototype() { return mPrototype; }

	void dump();

	bool addValue(string_table::key name, asNamespace *ns, boost::uint32_t slotId,
		asClass *type, as_value& val, bool isconst, bool isstatic);

	bool addSlot(string_table::key name, asNamespace *ns, boost::uint32_t slotId,
		asClass *type, bool isstatic);

	bool addMethod(string_table::key name, asNamespace *ns, asMethod *method,
		bool isstatic);

	bool addGetter(string_table::key name, asNamespace *ns, asMethod *method,
		bool isstatic);

	bool addSetter(string_table::key name, asNamespace *ns, asMethod *method,
		bool isstatic);

	bool addMemberClass(string_table::key name, asNamespace *ns,
		boost::uint32_t slotId, asClass *type, bool isstatic);

	// TODO: Figure out how this differs from addMethod
	bool addSlotFunction(string_table::key name, asNamespace *ns,
		boost::uint32_t slotId, asMethod *method, bool isstatic);

	/// Is the class final?
	bool isFinal() const { return mFinal; }

	/// Set the class as final.
	void setFinal() { mFinal = true; }

	/// Set the class as not final.
	void unsetFinal() { mFinal = false; }

	/// Is the class sealed?
	bool isSealed() const { return mSealed; }

	/// Set the class as sealed.
	void setSealed() { mSealed = true; }

	// Set the class as not sealed.
	void unsetSealed() { mSealed = false; }

	/// Is the class an interface type?
	bool isInterface() const { return mInterface; }

	/// Set the class as an interface.
	void setInterface() { mInterface = true; }

	/// Set the class as not an interface.
	void unsetInterface() { mInterface = false; }

	/// Is the class dynamic?
	bool isDynamic() const { return mDynamic; }

	/// Set the class as dynamic.
	void setDynamic() { mDynamic = true; }

	/// Set the class as not dynamic.
	void unsetDynamic() { mDynamic = false; }

	/// Does the class have a protected namespace to be inherited?
	bool hasProtectedNs() const { return mProtectedNs != NULL; }

	/// Get the protected namespace.
	asNamespace *getProtectedNs() { return mProtectedNs; }

	/// Set the protected namespace.
	void setProtectedNs(asNamespace *n) { mProtectedNs = n; }

	string_table::key getName() const { return mName; }

	/// Set our Name
	void setName(string_table::key name) { mName = name; }

	/// What is the type of our parent class?
	asClass* getSuper() const { return mSuper; }

	/// We implement this interface.
	void pushInterface(asClass* p) { mInterfaces.push_back(p); }

	/// This is our constructor.
	void setConstructor(asMethod *m) { mConstructor = m; }
	asMethod *getConstructor() { return mConstructor; }

	void setStaticConstructor(asMethod *m) { mStaticConstructor = m; }

	void setSuper(asClass *p) { mSuper = p; }

	/// Try to build an asClass object from just a prototype.
	void buildFromPrototype(as_object *o, string_table::key name,
		ClassHierarchy *);

	void setDeclared() { mDeclared = true; }
	bool isDeclared() { return mDeclared; }
	void setInherited() { mInherited = true; }
	bool isInherited() { return mInherited; }

	void setSystem() { mSystem = true; }
	void unsetSystem() { mSystem = false; }
	bool isSystem() { return mSystem; }

	asClass() : mFinal(false), mSealed(false), mDynamic(false),
		mInterface(false), mName(0), mInterfaces(), mProtectedNs(NULL),
		mSuper(NULL), mConstructor(NULL), mStaticConstructor(NULL),
		mBindings(), mStaticBindings(), mDeclared(false), mInherited(false),
		mSystem(false)
	{/**/}


	asBinding *getBinding(string_table::key name)
	{
		binding_container::iterator i;
		if (mBindings.empty())
			return NULL;
		i = mBindings.find(name);
		if (i == mBindings.end())
			return NULL;
		return &i->second;
	}

	asBinding* getGetBinding(as_value& v, asName& n);
	asBinding* getSetBinding(as_value& v, asName& n);

private:
	as_object *mPrototype;

	bool addBinding(string_table::key name, asBinding b)
	{ mBindings[name] = b; return true; }
	bool addStaticBinding(string_table::key name, asBinding b)
	{ mStaticBindings[name] = b; return true; }

	asBinding *getStaticBinding(string_table::key name)
	{
		binding_container::iterator i;
		if (mStaticBindings.empty())
			return NULL;
		i = mStaticBindings.find(name);
		if (i == mStaticBindings.end())
			return NULL;
		return &i->second;
	}

	bool mFinal;
	bool mSealed;
	bool mDynamic;
	bool mInterface;
	string_table::key mName;
	std::list<asClass*> mInterfaces;
	asNamespace *mProtectedNs;
	asClass *mSuper;
	asMethod *mConstructor;
	asMethod *mStaticConstructor;

	typedef std::map<string_table::key, asBinding> binding_container;

	binding_container mBindings;
	binding_container mStaticBindings;
	bool mDeclared;
	bool mInherited;
	bool mSystem;
};

} /* namespace gnash */

#endif /* GNASH_AS_CLASS_H */
