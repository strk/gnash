// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#ifndef GNASH_ABC_CLASS_H
#define GNASH_ABC_CLASS_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <list>
#include <map>
#include <vector>
#include "string_table.h"
#include "Property.h"
#include "AbcBlock.h"

namespace gnash {
    namespace abc {
        class Machine;
        class MultiName;
        class abc_function;
        class BoundValue;
        class BoundAccessor;
        class Method;
        class Class;
        class Namespace;
    }
    class ClassHierarchy;
    class Property;
    class as_value;
}

namespace gnash {
namespace abc {

/// A class to represent AS3 Classes.
//
/// Used to store ABC classes. These are not themselves AS-referenceable
/// objects, but can be associated with AS3 Class objects in a way that
/// is yet to be determined.
//
/// TODO: update this documentation when we've worked it out.
//
/// An abc::Class is a static description of an ActionScript Class. Classes
/// have the following important properties:
//
/// 1.  A static initialization method ("cinit"). This is executed during
///     the opcode NewClass, which is generally called only once per class.
/// 2.  A constructor method ("iinit"). This is run every time the Class
///     is constructed. As not all Classes are constructed, the iinit method
///     may never be executed.
/// 3. A set of class Traits.
/// 4. A set of instance Traits.
//
/// Classes are parsed from the "instances" and "classes" section of an
/// ABCBlock. Each of these contains the same number of entries. The iinit
/// methods are found in the instances section, the cinit methods in the
/// classes section.
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

	bool addMemberScript(string_table::key name, Namespace *ns,
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
	Namespace* getProtectedNs() { return _protectedNs; }

	/// Set the protected namespace.
	void setProtectedNs(Namespace *n) { _protectedNs = n; }

    /// The global name of the class.
	string_table::key getName() const { return _name; }
	
    /// Retrieve the Class from which this Class derives.
    Class* getSuper() const { return _super; }

    /// Set the Super Class.
    //
    /// This is the base class for this Class.
	void setSuper(Class *p) { _super = p; }

	/// We implement this interface.
	void pushInterface(Class* p) { _interfaces.push_back(p); }

	/// Set the iinit method.
    //
    /// This is used to construct instances of the Class.
	void setConstructor(Method *m) { _constructor = m; }

    /// Get the iinit method or 'constructor'.
    //
    /// A Class is also valid if it does not have an iinit method, so this
    /// function can return 0.
	Method* getConstructor() const {
        return _constructor;
    }

    /// Set the cinit method
    //
    /// This is used to initialize the Class.
	void setStaticConstructor(Method *m) { _staticConstructor = m; }

    /// Get the cinit method or 'static constructor'.
    //
    /// A Class may have no cinit method, so this function can return 0.
    Method* getStaticConstructor() const { 
        return _staticConstructor;
    }

    void addStaticTrait(const Trait& t) {
        _staticTraits.push_back(t);
    }

    void addInstanceTrait(const Trait& t) {
        _instanceTraits.push_back(t);
    }

	Property* getBinding(string_table::key name)
	{
		BindingContainer::iterator i;
		if (_bindings.empty()) return NULL;
		i = _bindings.find(name);
		if (i == _bindings.end())
			return NULL;
		return &i->second;
	}

	Property* getGetBinding(as_value& v, abc::MultiName& n);
	Property* getSetBinding(as_value& v, abc::MultiName& n);

    /// This initializes all the traits.
    //
    /// Note: this is only necessary because the implementation is bogus.
    /// TODO: fix it.
    void initTraits(AbcBlock& bl);

    /// Necessary for the current bogus implementation.
    void setPrototype(as_object* prototype) {
        _prototype = prototype;
    }

    /// Necessary for the current bogus implementation.
	void initPrototype();

    /// Necessary for the current bogus implementation.
	as_object* getPrototype() { return _prototype; }

private:

	bool addBinding(string_table::key name, const Property& b) {
        _bindings.insert(std::make_pair(name, b));
        return true;
    }

	bool addStaticBinding(string_table::key name, const Property& b) {
        _staticBindings.insert(std::make_pair(name, b));
        return true;
    }

	Property *getStaticBinding(string_table::key name)
	{
		if (_staticBindings.empty()) return 0;
		BindingContainer::iterator i = _staticBindings.find(name);
		if (i == _staticBindings.end()) return 0;
		return &i->second;
	}

    
    /// The Traits for instances of this class
    std::vector<Trait> _instanceTraits;

    /// The static Traits for this class;
    std::vector<Trait> _staticTraits;

	
	typedef std::map<string_table::key, Property> BindingContainer;

    as_object *_prototype;
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
