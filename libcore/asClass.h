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
#include <iostream>
#include "string_table.h"
#include "as_value.h"
#include "CodeStream.h"
#include "Property.h"
#include "as_function.h"
#include "abc_block.h"


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
class Machine;
class abc_function;

namespace abc_parsing {
class abc_Trait;
}

using namespace abc_parsing;
class asException
{
public:
	void setStart(boost::uint32_t i) { _start = i; }
	void setEnd(boost::uint32_t i) { mEnd = i; }
	void setCatch(boost::uint32_t i) { mCatch = i; }
	void catchAny() { mCatchAny = true; }
	void setCatchType(asClass* p) { mCatchType = p; }
	void setNamespace(asNamespace* n) { _namespace = n; }
	void setName(string_table::key name) { _name = name; }

private:
	boost::uint32_t _start;
	boost::uint32_t mEnd;
	boost::uint32_t mCatch;
	bool mCatchAny;
	asClass *mCatchType;
	asNamespace *_namespace;
	string_table::key _name;
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

	void setStatic() { _static = true; }
	void unsetStatic() { _static = false; }
	bool isStatic() { return _static; }

	boost::uint32_t getSlotId() { return _slotId; }
	void setSlotId(boost::uint32_t s) { _slotId = s; }

	// Chad: Document
	string_table::key getName() { return _name; }
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
		_namespace(ns), mType(T_METHOD), _slotId(0), mConst(true),
		_static(isstatic),	_method(pMethod)
	{/**/}

	// As an assignable function.
	asBinding(asNamespace *ns, asMethod *pMethod, boost::uint32_t sid, bool isstatic) :
		_namespace(ns), mType(T_METHOD), _slotId(sid), mConst(false),
		_static(isstatic),	_method(pMethod)
	{/**/}

	asBinding(asNamespace *ns, asBoundValue *pValue, boost::uint32_t sid, bool isconstant,
		bool isstatic) : _namespace(ns), mType(T_VALUE), _slotId(sid), mConst(isconstant),
		_static(isstatic), mValue(pValue)
	{/**/}

	asBinding(asNamespace *ns, asBoundValue *pValue, boost::uint32_t sid, bool isstatic) :
		_namespace(ns), mType(T_VALUE), _slotId(sid), mConst(false), _static(isstatic),
		mValue(pValue)
	{/**/}

	asBinding(asNamespace *ns, asBoundAccessor *pAccess, bool isstatic) :
		_namespace(ns), mType(T_ACCESS), _slotId(0), mConst(true), _static(isstatic),
		_access(pAccess)
	{/**/}

	asBinding(asNamespace *ns, asClass *pClass, boost::uint32_t sid, bool isstatic) :
		_namespace(ns), mType(T_CLASS), _slotId(sid), mConst(true), _static(isstatic),
		mClass(pClass)
	{/**/}

	asBinding() : _namespace(NULL), mType(T_CLASS), _slotId(0), mConst(false), _static(false),
		mClass(NULL)
	{/**/}

	asBinding(asMethod *);
	asBinding(as_function *);

	void reset(asBoundAccessor *pAccess, bool isstatic)
	{
		mType = T_ACCESS;
		_access = pAccess;
		mConst = true;
		_static = isstatic;
	}

	asBoundAccessor* getAccessor()
	{ return mType == T_ACCESS ? _access : NULL; }

	asBoundValue* getValue()
	{ return mType == T_VALUE ? mValue : NULL; }

	asMethod* getMethod()
	{ return mType == T_METHOD ? _method : NULL; }

	asClass* getClass()
	{ return mType == T_CLASS ? mClass : NULL; }

	as_function* getASFunction();

	asNamespace *_namespace;

	typedef enum
	{
		T_CLASS,
		T_METHOD,
		T_AS_FUNCTION,
		T_VALUE,
		T_ACCESS
	} types;
	types mType;

	boost::uint32_t _slotId;
	bool mConst;
	bool _static;
	string_table::key _name;
	as_object* _owner;

	union
	{
		asClass *mClass;
		asMethod *_method;
		asBoundValue *mValue;
		asBoundAccessor *_access;
	};
};
#endif // comment out of asBinding

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
	bool setSetter(asMethod *p) { _setter = p; return true; }
	bool setValue(asBoundValue *p) { mValue = p; return true; }

	asBoundValue* getValue() { return mValue; }
	asMethod *getGetter() { return mGetter; }
	asMethod *getSetter() { return _setter; }

private:
	asMethod *mGetter;
	asMethod *_setter;
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
public:
	
    typedef std::list<asClass*> ArgumentList;

	asMethod();

    boost::uint32_t methodID() const {
        return _methodID;
    }

    void setMethodID(boost::uint32_t m) {
        _methodID = m;
    }

	void initPrototype(Machine* machine);

	boost::uint32_t getMaxRegisters(){ return _maxRegisters;}

	void setMaxRegisters(boost::uint32_t maxRegisters) { 
        _maxRegisters = maxRegisters;
    }

	boost::uint32_t getBodyLength(){ return _bodyLength;}

	void setBodyLength(boost::uint32_t length){ _bodyLength = length;}

	abc_function* getPrototype() { return _prototype; }

	asBinding* getBinding(string_table::key name);

	bool isNative() { return _isNative; }
	bool hasBody() const { return _body != NULL; }

	as_object* construct(as_object* /*base_scope*/) { /* TODO */ return NULL; }

	bool hasActivation();

	CodeStream *getBody() { return _body; }
	void setBody(CodeStream *b) { _body = b; }

	bool addValue(string_table::key name, asNamespace *ns, boost::uint32_t slotID,
		asClass *type, as_value& val, bool isconst);

	bool addSlot(string_table::key name, asNamespace *ns, boost::uint32_t slotID,
		asClass *type);

	bool addMethod(string_table::key name, asNamespace *ns, asMethod *method);

	bool addGetter(string_table::key name, asNamespace *ns, asMethod *method);

	bool addSetter(string_table::key name, asNamespace *ns, asMethod *method);

	bool addMemberClass(string_table::key name, asNamespace *ns,
		boost::uint32_t slotID, asClass *type);
	
	bool addSlotFunction(string_table::key name, asNamespace *ns,
		boost::uint32_t slotID, asMethod *method);

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
	bool isFinal() const { return _flags & FLAGS_FINAL; }

	/// \brief
	/// Set the method as final.
	void setFinal() { _flags = _flags | FLAGS_FINAL; }

	/// \brief
	/// Unset the method as final. Not final anymore.
	void unsetFinal() { _flags = _flags & ~FLAGS_FINAL; }

	/// \brief
	/// Is the method private?
	bool isPrivate() const { return _flags & FLAGS_PRIVATE; }

	/// \brief
	/// Make the method private.
	void setPrivate() {
        _flags = (_flags & ~(FLAGS_PUBLIC | FLAGS_PROTECTED)) | FLAGS_PRIVATE;
    }

	/// \brief
	/// Is the method protected?
	bool isProtected() const {
        return _flags & FLAGS_PROTECTED;
    }

	/// \brief
	/// Make the method protected.
	void setProtected() {
        _flags = (_flags & ~(FLAGS_PUBLIC | FLAGS_PRIVATE)) | FLAGS_PROTECTED; }

	/// \brief
	/// Is the method public?
	bool isPublic() const { return _flags & FLAGS_PUBLIC; }

	/// \brief
	/// Make the method public.
	void setPublic() {
        _flags = (_flags & ~(FLAGS_PRIVATE | FLAGS_PROTECTED)) | FLAGS_PUBLIC;
    }

	/// \brief
	/// How many arguments are required? -1 means unknown.
	int minArgumentCount() const { return _minArguments; }

	/// \brief
	/// Set the required minimum arguments.
	void setMinArgumentCount(int i) { _minArguments = i; }

	/// \brief
	/// How many arguments are allowed? -1 means unknown.
	int maxArgumentCount() const { return _maxArguments; }

	/// Set the required maximum arguments.
	void setMaxArgumentCount(int i) { _maxArguments = i; }

	/// \brief
	/// Push an argument of type t into the method definition
	void pushArgument(asClass *t) { _arguments.push_back(t); }

	/// \brief
	/// Push an optional argument's default value.
	void pushOptional(const as_value& v) { _optionalArguments.push_back(v); }

	/// \brief
	/// Are any of the arguments optional?
	bool optionalArguments() const {
        return minArgumentCount() != maxArgumentCount();
    }

	/// \brief
	/// Get a reference to a list of argument types.
	ArgumentList& getArgumentList() { return _arguments; }

	/// \brief
	/// Get an object capable of executing this function.
	/// Note: This may be NULL, because we might have information about this
	/// function but not actually have it yet.
	as_function* getImplementation() { return _implementation; }

	/// \brief
	/// Print the opcodes that define a method using log_parse.
	void print_body();

	void print_static_constructor(){

	}

private:

	enum Flag
	{
		FLAGS_FINAL = 0x01,
		FLAGS_PROTECTED = 0x02,
		FLAGS_PUBLIC = 0x04,
		FLAGS_PRIVATE = 0x08
	};

	/// A list of type identifiers
	typedef std::map<string_table::key, asBinding> BindingContainer;

	bool addBinding(string_table::key name, asBinding b);
	
    boost::uint32_t _methodID;

    abc_function* _prototype;
	int _minArguments;
	int _maxArguments;
	boost::uint32_t _bodyLength;
	bool _isNative;
	ArgumentList _arguments;
	std::list<as_value> _optionalArguments;
	as_function* _implementation;
	unsigned char _flags;
	CodeStream* _body;
	boost::uint32_t _maxRegisters;

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

	asClass()
        :
        _prototype(0),
        _final(false),
        _sealed(false),
        _dynamic(false),
		_interface(false),
        _name(0),
        _interfaces(),
        _protectedNs(0),
		_super(0),
        _constructor(0),
        _staticConstructor(0),
		_bindings(),
        _staticBindings(),
        _declared(false),
        _inherited(false),
		_system(false)
	{}

	as_object* getPrototype() { return _prototype; }

	void dump();

	bool addValue(string_table::key name, asNamespace *ns,
            boost::uint32_t slotID, asClass *type, as_value& val,
            bool isconst, bool isstatic);

	bool addSlot(string_table::key name, asNamespace *ns,
            boost::uint32_t slotID, asClass *type, bool isstatic);

	bool addMethod(string_table::key name, asNamespace *ns, asMethod *method,
		bool isstatic);

	bool addGetter(string_table::key name, asNamespace *ns, asMethod *method,
		bool isstatic);

	bool addSetter(string_table::key name, asNamespace *ns, asMethod *method,
		bool isstatic);

	bool addMemberClass(string_table::key name, asNamespace *ns,
		boost::uint32_t slotID, asClass *type, bool isstatic);

	// TODO: Figure out how this differs from addMethod
	bool addSlotFunction(string_table::key name, asNamespace *ns,
		boost::uint32_t slotID, asMethod *method, bool isstatic);

	/// Is the class final?
	bool isFinal() const { return _final; }

	/// Set the class as final.
	void setFinal() { _final = true; }

	/// Set the class as not final.
	void unsetFinal() { _final = false; }

	/// Is the class sealed?
	bool isSealed() const { return _sealed; }

	/// Set the class as sealed.
	void setSealed() { _sealed = true; }

	// Set the class as not sealed.
	void unsetSealed() { _sealed = false; }

	/// Is the class an interface type?
	bool isInterface() const { return _interface; }

	/// Set the class as an interface.
	void setInterface() { _interface = true; }

	/// Set the class as not an interface.
	void unsetInterface() { _interface = false; }

	/// Is the class dynamic?
	bool isDynamic() const { return _dynamic; }

	/// Set the class as dynamic.
	void setDynamic() { _dynamic = true; }

	/// Set the class as not dynamic.
	void unsetDynamic() { _dynamic = false; }

	/// Does the class have a protected namespace to be inherited?
	bool hasProtectedNs() const { return _protectedNs; }

	/// Get the protected namespace.
	asNamespace *getProtectedNs() { return _protectedNs; }

	/// Set the protected namespace.
	void setProtectedNs(asNamespace *n) { _protectedNs = n; }

	string_table::key getName() const { return _name; }

	void initPrototype(){ _prototype = new as_object();}

	/// Set our Name
	void setName(string_table::key name) { _name = name; }

	/// What is the type of our parent class?
	asClass* getSuper() const { return _super; }

	/// We implement this interface.
	void pushInterface(asClass* p) { _interfaces.push_back(p); }

	/// This is our constructor.
	void setConstructor(asMethod *m) { _constructor = m; }
	asMethod *getConstructor() { return _constructor; }

	void setStaticConstructor(asMethod *m) { _staticConstructor = m; }
	asMethod *getStaticConstructor(){return _staticConstructor;}
	void setSuper(asClass *p) { _super = p; }

	/// Try to build an asClass object from just a prototype.
	void buildFro_prototype(as_object *o, string_table::key name,
		ClassHierarchy *);

	void setDeclared() { _declared = true; }
	bool isDeclared() { return _declared; }
	void setInherited() { _inherited = true; }
	bool isInherited() { return _inherited; }

	void setSystem() { _system = true; }
	void unsetSystem() { _system = false; }
	bool isSystem() { return _system; }

	asBinding *getBinding(string_table::key name)
	{
		BindingContainer::iterator i;
		if (_bindings.empty()) return NULL;
		i = _bindings.find(name);
		if (i == _bindings.end())
			return NULL;
		return &i->second;
	}

	asBinding* getGetBinding(as_value& v, asName& n);
	asBinding* getSetBinding(as_value& v, asName& n);

	std::vector<abc_Trait> _traits;

private:

	typedef std::map<string_table::key, asBinding> BindingContainer;

    as_object *_prototype;

	bool addBinding(string_table::key name, asBinding b)
	{ _bindings[name] = b; return true; }
	bool addStaticBinding(string_table::key name, asBinding b)
	{ _staticBindings[name] = b; return true; }

	asBinding *getStaticBinding(string_table::key name)
	{
		BindingContainer::iterator i;
		if (_staticBindings.empty())
			return NULL;
		i = _staticBindings.find(name);
		if (i == _staticBindings.end())
			return NULL;
		return &i->second;
	}

	bool _final;
	bool _sealed;
	bool _dynamic;
	bool _interface;
	string_table::key _name;
	std::list<asClass*> _interfaces;
	asNamespace* _protectedNs;
	asClass* _super;
	asMethod* _constructor;
	asMethod* _staticConstructor;

	BindingContainer _bindings;
	BindingContainer _staticBindings;
	bool _declared;
	bool _inherited;
	bool _system;
};

} /* namespace gnash */

#endif /* GNASH_AS_CLASS_H */
