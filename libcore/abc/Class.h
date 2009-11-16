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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <list>
#include <map>
#include <vector>
#include <iostream>
#include "string_table.h"
#include "as_value.h"
#include "as_object.h"
#include "Property.h"

#ifdef ENABLE_AVM2
# include "CodeStream.h"
# include "AbcBlock.h"
#endif

namespace gnash {
    namespace abc {
        class Machine;
        class MultiName;
        class abc_function;
        class BoundValue;
        class BoundAccessor;
        class Method;
        class Class;
        typedef Property Binding;
    }
    class Namespace;
    class ClassHierarchy;
    class Property;
}

namespace gnash {
namespace abc {

/// A class to represent, abstractly, ActionScript prototypes.
///
/// This class is intended to be able to capture the structure of an
/// ActionScript prototype as a type, rather than as an object. This is
/// contrary to the spirit of ActionScript as a dynamic language, but it is
/// incredibly helpful to an interpreter for that language.
class Class
{
public:

	Class()
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
	
    void setDeclared() { _declared = true; }
	bool isDeclared() { return _declared; }
	void setInherited() { _inherited = true; }
	bool isInherited() { return _inherited; }

	void setSystem() { _system = true; }
	void unsetSystem() { _system = false; }
	bool isSystem() { return _system; }
	
    /// Set our Name
	void setName(string_table::key name) { _name = name; }

	void dump();

#ifdef ENABLE_AVM2

	bool addValue(string_table::key name, Namespace *ns,
            boost::uint32_t slotID, Class *type, as_value& val,
            bool isconst, bool isstatic);

	bool addSlot(string_table::key name, Namespace *ns,
            boost::uint32_t slotID, Class *type, bool isstatic);

	bool addMethod(string_table::key name, Namespace *ns, Method *method,
		bool isstatic);

	bool addGetter(string_table::key name, Namespace *ns, Method *method,
		bool isstatic);

	bool addSetter(string_table::key name, Namespace *ns, Method *method,
		bool isstatic);

	bool addMemberClass(string_table::key name, Namespace *ns,
		boost::uint32_t slotID, Class *type, bool isstatic);

	// TODO: Figure out how this differs from addMethod
	bool addSlotFunction(string_table::key name, Namespace *ns,
		boost::uint32_t slotID, Method *method, bool isstatic);

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
	Namespace *getProtectedNs() { return _protectedNs; }

	/// Set the protected namespace.
	void setProtectedNs(Namespace *n) { _protectedNs = n; }

	string_table::key getName() const { return _name; }

    void setPrototype(as_object* prototype) {
        _prototype = prototype;
    }

	void initPrototype() {
        _prototype =  new as_object();
    }

	/// What is the type of our parent class?
	Class* getSuper() const { return _super; }

	/// We implement this interface.
	void pushInterface(Class* p) { _interfaces.push_back(p); }

	/// This is our constructor.
	void setConstructor(Method *m) { _constructor = m; }
	Method *getConstructor() { return _constructor; }

	void setStaticConstructor(Method *m) { _staticConstructor = m; }
	
    Method* getStaticConstructor() const { 
        return _staticConstructor;
    }

	void setSuper(Class *p) { _super = p; }

	/// Try to build an Class object from just a prototype.
	void buildFro_prototype(as_object *o, string_table::key name,
		ClassHierarchy *);

	Binding *getBinding(string_table::key name)
	{
		BindingContainer::iterator i;
		if (_bindings.empty()) return NULL;
		i = _bindings.find(name);
		if (i == _bindings.end())
			return NULL;
		return &i->second;
	}

	Binding* getGetBinding(as_value& v, abc::MultiName& n);
	Binding* getSetBinding(as_value& v, abc::MultiName& n);
    std::vector<abc::Trait> _traits;

#endif

private:
	
	typedef std::map<string_table::key, Binding> BindingContainer;

    as_object *_prototype;

	bool addBinding(string_table::key name, Binding b)
	{ _bindings[name] = b; return true; }
	bool addStaticBinding(string_table::key name, Binding b)
	{ _staticBindings[name] = b; return true; }

	Binding *getStaticBinding(string_table::key name)
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
	std::list<Class*> _interfaces;
	Namespace* _protectedNs;
	Class* _super;
	Method* _constructor;
	Method* _staticConstructor;

	BindingContainer _bindings;
	BindingContainer _staticBindings;
	bool _declared;
	bool _inherited;
	bool _system;
};

} // namespace abc 
} // namespace gnash

#endif 
