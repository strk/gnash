// as_object.cpp:  ActionScript Object class and its properties, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "log.h"

#include "smart_ptr.h" // GNASH_USE_GC
#include "as_object.h"
#include "as_function.h"
#include "as_environment.h" 
#include "movie_root.h" 
#include "event_id.h" 
#include "Property.h"
#include "VM.h"
#include "GnashException.h"
#include "fn_call.h" 
#include "Array_as.h"
#include "as_function.h"
#include "Global_as.h" 
#include "GnashAlgorithm.h"
#include "DisplayObject.h"

#include <set>
#include <string>
#include <boost/algorithm/string/case_conv.hpp>
#include <utility> // for std::pair
#include "namedStrings.h"


namespace gnash {
template<typename T>
class
as_object::PrototypeRecursor
{
public:
    PrototypeRecursor(as_object* top, const ObjectURI& uri, T cmp = T())
        :
        _object(top),
        _uri(uri),
        _iterations(0),
        _condition(cmp)
    {
        _visited.insert(top);
    }

    /// Iterate to the next object in the inheritance chain.
    //
    /// This function throws an ActionLimitException when the maximum
    /// number of recursions is reached.
    //
    /// @return     false if there is no next object. In this case calling
    ///             the other functions will abort.
    bool operator()()
    {
        ++_iterations;

        // See swfdec/prototype-recursion-get-?.swf
		if (_iterations > 256) {
			throw ActionLimitException("Lookup depth exceeded.");
        }

        _object = _object->get_prototype();

        // TODO: there is recursion prevention anyway; is this extra 
        // check for circularity really necessary?
        if (!_visited.insert(_object).second) return 0;
        return _object && !_object->displayObject();
    }

    /// Return the wanted property if it exists and satisfies the predicate.
    //
    /// This will abort if there is no current object.
    Property* getProperty(as_object** owner = 0) const {

        assert(_object);
        Property* prop = _object->_members.getProperty(_uri);
        
        if (prop && _condition(*prop)) {
            if (owner) *owner = _object;
            return prop;
        }
        return 0;
    }

private:
    as_object* _object;
    const ObjectURI& _uri;
    std::set<const as_object*> _visited;
    size_t _iterations;
    T _condition;
};

// Anonymous namespace used for module-static defs
namespace {

/// 'super' is a special kind of object
//
/// See http://wiki.gnashdev.org/wiki/index.php/ActionScriptSuper
///
/// We make it derive from as_function instead of as_object
/// to avoid touching too many files (ie: an as_object is not considered
/// something that can be called by current Gnash code). We may want
/// to change this in the future to implement what ECMA-262 refers to
/// as the [[Call]] property of objects.
///
class as_super : public as_object
{
public:

	as_super(Global_as& gl, as_object* super)
		:
        as_object(gl),
		_super(super)
	{
		set_prototype(prototype());
	}

	virtual bool isSuper() const { return true; }

	virtual as_object* get_super(string_table::key fname = 0);

	// Fetching members from 'super' yelds a lookup on the associated prototype
	virtual bool get_member(const ObjectURI& uri, as_value* val)
	{
        as_object* proto = prototype();
		if (proto) return proto->get_member(uri, val);
		log_debug("Super has no associated prototype");
		return false;
	}

	/// Dispatch.
	virtual as_value call(const fn_call& fn)
	{

        // TODO: this is a hack to make sure objects are constructed, not
        // converted (fn.isInstantiation() must be true).
        fn_call::Args::container_type argsIn(fn.getArgs());
        fn_call::Args args;
        args.swap(argsIn);

        fn_call fn2(fn.this_ptr, fn.env(), args, fn.super, true);
        assert(fn2.isInstantiation());
        as_function* ctor = constructor();
		if (ctor) return ctor->call(fn2);
		log_debug("Super has no associated constructor");
		return as_value();
	}

protected:

	virtual void markReachableResources() const
	{
		if (_super) _super->setReachable();
		markAsObjectReachable();
	}

private:

    as_object* prototype() {
        return _super ? _super->get_prototype() : 0;
    }

    as_function* constructor() {
        return _super ? _super->get_constructor() : 0;
    }

	as_object* _super;
};

as_object*
as_super::get_super(string_table::key fname)
{
	// Super references the super class of our class prototype.
	// Our class prototype is __proto__.
	// Our class superclass prototype is __proto__.__proto__

	// Our class prototype is __proto__.
	as_object* proto = get_prototype(); 
	if (!proto) return new as_super(getGlobal(*this), 0);

    if (!fname || getSWFVersion(*this) <= 6) {
        return new as_super(getGlobal(*this), proto);
    }

    as_object* owner = 0;
    proto->findProperty(fname, &owner);
    if (!owner) return 0;

    if (owner == proto) return new as_super(getGlobal(*this), proto);

    as_object* tmp = proto;
    while (tmp && tmp->get_prototype() != owner) {
        tmp = tmp->get_prototype();
    }
    // ok, now 'tmp' should be the object whose __proto__ member
    // contains the actual named method.
    //
    // in the C:B:A:F case this would be B when calling
    // super.myName() from C.prototype.myName()
    
    // well, since we found the property, it must be somewhere!
    assert(tmp); 

    if (tmp != proto) { return new as_super(getGlobal(*this), tmp); }
    return new as_super(getGlobal(*this), owner);

}


/// A PropertyList visitor copying properties to an object
class PropsCopier : public AbstractPropertyVisitor
{

public:

	/// \brief
	/// Initialize a PropsCopier instance associating it
	/// with a target object (an object whose members has to be set)
	///
	PropsCopier(as_object& tgt)
		:
		_tgt(tgt)
	{}

	/// Set *inherited* properties of the given target object
	bool accept(const ObjectURI& uri, const as_value& val) {
		if (getName(uri) == NSV::PROP_uuPROTOuu) return true;
		_tgt.set_member(getName(uri), val);
        return true;
	}
private:
	as_object& _tgt;
};

class PropertyEnumerator : public AbstractPropertyVisitor
{
public:
    PropertyEnumerator(const as_object& this_ptr,
            as_object::SortedPropertyList& to)
        :
        _version(getSWFVersion(this_ptr)),
        _st(getStringTable(this_ptr)),
        _to(to)
    {}

    bool accept(const ObjectURI& uri, const as_value& val) {
		_to.push_front(std::make_pair(_st.value(getName(uri)),
				val.to_string_versioned(_version)));
        return true;
    }

private:
    const int _version;
    string_table& _st;
    as_object::SortedPropertyList& _to;
};

} // end of anonymous namespace


const int as_object::DefaultFlags;

as_object::as_object(Global_as& gl)
	:
    _displayObject(0),
    _array(false),
    _relay(0),
	_vm(getVM(gl)),
	_members(*this)
{
}

as_object::as_object()
	:
    _displayObject(0),
    _array(false),
    _relay(0),
	_vm(VM::get()),
	_members(*this)
{
}

as_value
as_object::call(const fn_call& /*fn*/)
{
    throw ActionTypeError();
}

const std::string&
as_object::stringValue() const
{
    // TODO: AS3 returns a string describing the type of object, e.g.
    // "[object MyObject]"
    if (isAS3(*this)) {
        static const std::string str("[object Object]");
        return str;
    }

    static const std::string str("[object Object]");
    return str;
}

std::pair<bool,bool>
as_object::delProperty(const ObjectURI& uri)
{
	return _members.delProperty(uri);
}


void
as_object::add_property(const std::string& name, as_function& getter,
		as_function* setter)
{
	string_table& st = getStringTable(*this);
	string_table::key k = st.find(name);

	as_value cacheVal;

	Property* prop = _members.getProperty(k);

    if (prop) {
		cacheVal = prop->getCache();
        // Used to return the return value of addGetterSetter, but this
        // is always true.
		_members.addGetterSetter(k, getter, setter, cacheVal);
        return;
		// NOTE: watch triggers not called when adding a new
        // getter-setter property
	}
	else {

		_members.addGetterSetter(k, getter, setter, cacheVal);

        // Nothing more to do if there are no triggers.
        if (!_trigs.get()) return;

		// check if we have a trigger, if so, invoke it
		// and set val to its return
		TriggerContainer::iterator trigIter = _trigs->find(ObjectURI(k, 0));
		if (trigIter != _trigs->end()) {
			Trigger& trig = trigIter->second;

			log_debug("add_property: property %s is being watched, "
                    "current val: %s", name, cacheVal);
			cacheVal = trig.call(cacheVal, as_value(), *this);

			// The trigger call could have deleted the property,
			// so we check for its existence again, and do NOT put
			// it back in if it was deleted
			prop = _members.getProperty(k);
			if (!prop) {
				log_debug("Property %s deleted by trigger on create "
                        "(getter-setter)", name);
				return;
			}
			prop->setCache(cacheVal);
		}
		return;
	}
}


/// Order of property lookup:
//
/// 1. Visible own properties.
/// 2. If DisplayObject, magic properties
/// 3. Visible own properties of all __proto__ objects (a DisplayObject
///    ends the chain).
/// 4. __resolve property of this object and all __proto__ objects (a Display
///    Object ends the chain). This should ignore visibility but doesn't.
bool
as_object::get_member(const ObjectURI& uri, as_value* val)
{
	assert(val);

    const int version = getSWFVersion(*this);

    PrototypeRecursor<IsVisible> pr(this, uri, IsVisible(version));
	
	Property* prop = pr.getProperty();
    if (!prop) {
        if (displayObject()) {
            DisplayObject* d = displayObject();
            if (getDisplayObjectProperty(*d, getName(uri), *val)) return true;
        }
        while (pr()) {
            if ((prop = pr.getProperty())) break;
        }
    }

    // If the property isn't found or doesn't apply to any objects in the
    // inheritance chain, try the __resolve property.
    if (!prop) {

        prop = findProperty(NSV::PROP_uuRESOLVE);
        if (!prop) return false;

        /// If __resolve exists, call it with the name of the undefined
        /// property.
        string_table& st = getStringTable(*this);
        const std::string& undefinedName = st.value(getName(uri));
        log_debug("__resolve exists, calling with '%s'", undefinedName);

        // TODO: we've found the property, don't search for it again.
        *val = callMethod(this, NSV::PROP_uuRESOLVE, undefinedName);
        return true;
    }

	try {
		*val = prop->getValue(*this);
		return true;
	}
	catch (ActionLimitException& exc) {
		// will be logged by outer catcher
		throw;
	}
	catch (ActionTypeError& exc) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Caught exception: %s"), exc.what());
        );
		return false;
	}

}


const Property*
as_object::getByIndex(int index)
{
	// The low byte is used to contain the depth of the property.
	unsigned char depth = index & 0xFF;
	index /= 256; // Signed
	as_object *obj = this;
	while (depth--)
	{
		obj = obj->get_prototype();
		if (!obj)
			return NULL;
	}

	return obj->_members.getPropertyByOrder(index);
}

as_object*
as_object::get_super(string_table::key fname)
{
	// Super references the super class of our class prototype.
	// Our class prototype is __proto__.
	// Our class superclass prototype is __proto__.__proto__

	// Our class prototype is __proto__.
	as_object* proto = get_prototype();

	if (fname && getSWFVersion(*this) > 6) {
		as_object* owner = 0;
		findProperty(fname, &owner);
        // should be 0 if findProperty returned 0
		if (owner != this) proto = owner; 
	}

	as_object* super = new as_super(getGlobal(*this), proto);

	return super;
}

as_function*
as_object::get_constructor()
{
	as_value ctorVal;
	if ( ! get_member(NSV::PROP_uuCONSTRUCTORuu, &ctorVal) )
	{
		//log_debug("Object %p has no __constructor__ member");
		return NULL;
	}
	//log_debug("%p.__constructor__ is %s", ctorVal);
	return ctorVal.to_function();
}

int
as_object::nextIndex(int index, as_object **owner)
{
skip_duplicates:
	unsigned char depth = index & 0xFF;
	unsigned char i = depth;
	index /= 256; // Signed
	as_object *obj = this;
	while (i--)
	{
		obj = obj->get_prototype();
		if (!obj)
			return 0;
	}
	
	const Property *p = obj->_members.getOrderAfter(index);
	if (!p)
	{
		obj = obj->get_prototype();
		if (!obj)
			return 0;
		p = obj->_members.getOrderAfter(0);
		++depth;
	}
	if (p)
	{
		if (findProperty(p->uri()) != p)
		{
			index = p->getOrder() * 256 | depth;
			goto skip_duplicates; // Faster than recursion.
		}
		if (owner)
			*owner = obj;
		return p->getOrder() * 256 | depth;
	}
	return 0;
}

/*private*/
Property*
as_object::findProperty(const ObjectURI& uri, as_object **owner)
{

    const int version = getSWFVersion(*this);

    PrototypeRecursor<IsVisible> pr(this, uri, IsVisible(version));

    do {
        Property* prop = pr.getProperty(owner);
        if (prop) return prop;
    } while (pr());

	// No Property found
	return 0;
}

Property*
as_object::findUpdatableProperty(const ObjectURI& uri)
{

    PrototypeRecursor<Exists> pr(this, uri);

	Property* prop = pr.getProperty();

	// We won't scan the inheritance chain if we find a member,
	// even if invisible.
	if (prop) return prop; 
	
    const int swfVersion = getSWFVersion(*this);

    while (pr()) {
        if ((prop = pr.getProperty())) {
            if ((prop->isStatic() || prop->isGetterSetter()) &&
                    prop->visible(swfVersion)) {
                return prop;
            }
        }
	}
	return 0;
}

void
as_object::set_prototype(const as_value& proto)
{
	// TODO: check what happens if __proto__ is set as a user-defined 
    // getter/setter
	// TODO: check triggers !!
    // Note that this sets __proto__ in namespace 0
	_members.setValue(NSV::PROP_uuPROTOuu, proto, as_object::DefaultFlags);
}

void
as_object::reserveSlot(const ObjectURI& uri, boost::uint16_t slotId)
{
	_members.reserveSlot(uri, slotId);
}

bool
as_object::get_member_slot(int order, as_value* val){
	
	const Property* prop = _members.getPropertyByOrder(order);
	if (prop) {
		return get_member(prop->uri(), val);
	}
    return false;
}


bool
as_object::set_member_slot(int order, const as_value& val, bool ifFound)
{
	const Property* prop = _members.getPropertyByOrder(order);
	if (prop) {
		return set_member(getName(prop->uri()), val, getNamespace(prop->uri()),
                    ifFound);
	}
    return false;
}

void
as_object::executeTriggers(Property* prop, const ObjectURI& uri,
        const as_value& val)
{

    // check if we have a trigger, if so, invoke it
    // and set val to its return
    TriggerContainer::iterator trigIter;
    
    // If there are no triggers or the trigger is not found, just set
    // the property.
    if (!_trigs.get() || (trigIter = _trigs->find(uri)) == _trigs->end()) {
        if (prop) {
            prop->setValue(*this, val);
            prop->clearVisible(getSWFVersion(*this));
        }
        return;
    }

    Trigger& trig = trigIter->second;

    if (trig.dead()) {
        _trigs->erase(trigIter);
        return;
    }

    // WARNING: getValue might itself invoke a trigger
    // (getter-setter)... ouch ?
    // TODO: in this case, return the underlying value !
    as_value curVal = prop ? prop->getCache() : as_value(); 

    log_debug("Existing property %s is being watched: "
            "firing trigger on update (current val:%s, "
            "new val:%s)",
            getStringTable(*this).value(getName(uri)), curVal, val);

    as_value newVal = trig.call(curVal, val, *this);
    
    // This is a particularly clear and concise way of removing dead triggers.
    EraseIf(*_trigs, boost::bind(boost::mem_fn(&Trigger::dead), 
            boost::bind(SecondElement<TriggerContainer::value_type>(), _1)));
                    
    // The trigger call could have deleted the property,
    // so we check for its existence again, and do NOT put
    // it back in if it was deleted
    prop = findUpdatableProperty(uri);
    if (!prop) {
        log_debug("Property %s deleted by trigger on update",
                getStringTable(*this).value(getName(uri)));
        // Return true?
        return;
    }
    prop->setValue(*this, newVal); 
    prop->clearVisible(getSWFVersion(*this));
    
}

/// Order of property lookup:
//
/// 0. MovieClip textfield variables. TODO: this is a hack and should be
///    eradicated.
/// 1. Own properties even if invisible or not getter-setters. 
/// 2. If DisplayObject, magic properties
/// 3. Visible own getter-setter properties of all __proto__ objects
///    (a DisplayObject ends the chain).
bool
as_object::set_member(string_table::key key, const as_value& val,
	string_table::key nsname, bool ifFound)
{

    bool tfVarFound = false;
    if (displayObject()) {
        MovieClip* mc = dynamic_cast<MovieClip*>(displayObject());
        if (mc) tfVarFound = mc->setTextFieldVariables(key, val, nsname);
        // We still need to set the member.
    }

    // Handle the length property for arrays. NB: checkArrayLength() will
    // call this function again if the key is a valid index.
    if (array()) checkArrayLength(*this, key, val, nsname);

    const ObjectURI uri(key, nsname);
    
    PrototypeRecursor<Exists> pr(this, uri);

	Property* prop = pr.getProperty();

	// We won't scan the inheritance chain if we find a member,
	// even if invisible.
	if (!prop) { 

        if (displayObject()) {
            DisplayObject* d = displayObject();
            if (setDisplayObjectProperty(*d, key, val)) return true;
            // TODO: should we execute triggers?
        }

        const int version = getSWFVersion(*this);
        while (pr()) {
            if ((prop = pr.getProperty())) {
                if ((prop->isStatic() || prop->isGetterSetter()) &&
                        prop->visible(version)) {
                    break;
                }
                else prop = 0;
            }
        }
    }

    if (prop) {

		if (prop->isReadOnly()) {
			IF_VERBOSE_ASCODING_ERRORS(
                    log_aserror(_("Attempt to set read-only property '%s'"),
                    getStringTable(*this).value(key));
            );
			return true;
		}

		try {
            executeTriggers(prop, uri, val);
		}
		catch (ActionTypeError& exc) {
			log_aserror(_("%s: Exception %s. Will create a new member"),
				getStringTable(*this).value(key), exc.what());
		}

		return true;
	}

	// Else, add new property...
	if (ifFound) return false;

	// Property does not exist, so it won't be read-only. Set it.
	if (!_members.setValue(key, val)) {

		IF_VERBOSE_ASCODING_ERRORS(
			log_aserror(_("Unknown failure in setting property '%s' on "
			"object '%p'"), getStringTable(*this).value(key), (void*) this);
	    );
		return false;
	}

    executeTriggers(prop, uri, val);

    // Return true if we found a textfield variable.
    if (tfVarFound) return true;

	return false;
}


void
as_object::init_member(const std::string& key1, const as_value& val, int flags,
	string_table::key nsname)
{
	init_member(getStringTable(*this).find(key1), val, flags, nsname);
}

void
as_object::init_member(string_table::key key, const as_value& val, int flags,
	string_table::key nsname, int order)
{

    const ObjectURI uri(key, nsname);

	if (order >= 0 && !_members.reserveSlot(uri,
                static_cast<boost::uint16_t>(order))) {
		log_error(_("Attempt to set a slot for either a slot or a property "
			"which already exists."));
		return;
	}
		
	// Set (or create) a SimpleProperty 
	if (!_members.setValue(uri, val, flags)) {
		log_error(_("Attempt to initialize read-only property ``%s''"
			" on object ``%p'' twice"),
			getStringTable(*this).value(key), (void*)this);
		// We shouldn't attempt to initialize a member twice, should we ?
		abort();
	}
}

void
as_object::init_property(const std::string& key, as_function& getter,
		as_function& setter, int flags, string_table::key nsname)
{
	string_table::key k = getStringTable(*this).find(key);
	init_property(ObjectURI(k, nsname), getter, setter, flags);
}

void
as_object::init_property(const ObjectURI& uri, as_function& getter,
		as_function& setter, int flags)
{
	as_value cacheValue;

    // PropertyList::addGetterSetter always returns true (used to be
    // an assert).
	_members.addGetterSetter(uri, getter, &setter, cacheValue, flags);
}

void
as_object::init_property(const std::string& key, as_c_function_ptr getter,
		as_c_function_ptr setter, int flags, string_table::key nsname)
{
	string_table::key k = getStringTable(*this).find(key);
	init_property(ObjectURI(k, nsname), getter, setter, flags);
}

void
as_object::init_property(const ObjectURI& uri, as_c_function_ptr getter,
		as_c_function_ptr setter, int flags)
{
    // PropertyList::addGetterSetter always returns true (used to be
    // an assert).
	_members.addGetterSetter(uri, getter, setter, flags);
}

bool
as_object::init_destructive_property(const ObjectURI& uri, as_function& getter,
        int flags)
{
	// No case check, since we've already got the key.
	bool success = _members.addDestructiveGetter(uri, getter, flags);
	return success;
}

bool
as_object::init_destructive_property(const ObjectURI& uri,
        as_c_function_ptr getter, int flags)
{
	// No case check, since we've already got the key.
	bool success = _members.addDestructiveGetter(uri, getter, flags);
	return success;
}

void
as_object::init_readonly_property(const std::string& key, as_function& getter,
	int initflags, string_table::key nsname)
{
	string_table::key k = getStringTable(*this).find(key);

	init_property(ObjectURI(k, nsname), getter, getter,
            initflags | PropFlags::readOnly | PropFlags::isProtected);
	assert(_members.getProperty(ObjectURI(k, nsname)));
}

void
as_object::init_readonly_property(const ObjectURI& uri, as_function& getter,
        int initflags)
{
	init_property(uri, getter, getter,
            initflags | PropFlags::readOnly | PropFlags::isProtected);
	assert(_members.getProperty(uri));
}

void
as_object::init_readonly_property(const std::string& key,
        as_c_function_ptr getter, int initflags, string_table::key nsname)
{
	string_table::key k = getStringTable(*this).find(key);

	init_property(ObjectURI(k, nsname), getter, getter,
            initflags | PropFlags::readOnly | PropFlags::isProtected);
	assert(_members.getProperty(ObjectURI(k, nsname)));
}

void
as_object::init_readonly_property(const ObjectURI& uri,
        as_c_function_ptr getter, int initflags)
{
	init_property(uri, getter, getter, initflags | PropFlags::readOnly
		| PropFlags::isProtected);
	assert(_members.getProperty(uri));
}


bool
as_object::set_member_flags(const ObjectURI& uri, int setTrue, int setFalse)
{
	return _members.setFlags(uri, setTrue, setFalse);
}

void
as_object::addInterface(as_object* obj)
{
	assert(obj);
	if (std::find(_interfaces.begin(), _interfaces.end(), obj) ==
            _interfaces.end()) {
		_interfaces.push_back(obj);
    }
}

bool
as_object::instanceOf(as_object* ctor)
{

    /// An object is never an instance of a null prototype.
    if (!ctor) return false;

	as_value protoVal;
	if (!ctor->get_member(NSV::PROP_PROTOTYPE, &protoVal)) {
#ifdef GNASH_DEBUG_INSTANCE_OF
		log_debug("Object %p can't be an instance of an object (%p) "
                "with no 'prototype'",
			(void*)this, (void*)ctor);
#endif
		return false;
	}

	as_object* ctorProto = protoVal.to_object(getGlobal(*this));
	if (!ctorProto) {
#ifdef GNASH_DEBUG_INSTANCE_OF
		log_debug("Object %p can't be an instance of an object (%p) "
                "with non-object 'prototype' (%s)",
			(void*)this, (void*)ctor, protoVal);
#endif
		return false;
	}

	// TODO: cleanup the iteration, make it more readable ...
	std::set<as_object*> visited;

	as_object* obj = this;
	while (obj && visited.insert(obj).second) {
		as_object* thisProto = obj->get_prototype();
		if (!thisProto) {
			break;
		}

		// Check our proto
		if (thisProto == ctorProto) {
#ifdef GNASH_DEBUG_INSTANCE_OF
			log_debug("Object %p is an instance of constructor %p as "
                    "the constructor exposes our __proto__ %p",
				(void*)obj, (void*)ctor, (void*)thisProto);
#endif
			return true;
		}

		// Check our proto interfaces
		if (std::find(thisProto->_interfaces.begin(),
                    thisProto->_interfaces.end(), ctorProto)
                != thisProto->_interfaces.end()) {

#ifdef GNASH_DEBUG_INSTANCE_OF
			log_debug("Object %p __proto__ %p had one interface matching "
                    "with the constructor prototype %p",
				(void*)obj, (void*)thisProto, (void*)ctorProto);
#endif
			return true;
		}

		obj = thisProto;
	}

	return false;
}

bool
as_object::prototypeOf(as_object& instance)
{
	boost::intrusive_ptr<as_object> obj = &instance;

	std::set<as_object*> visited;

	while (obj && visited.insert(obj.get()).second )
	{
		if ( obj->get_prototype() == this ) return true;
		obj = obj->get_prototype(); 
	}

	// See actionscript.all/Inheritance.as for a way to trigger this
	IF_VERBOSE_ASCODING_ERRORS(
        if (obj) log_aserror(_("Circular inheritance chain detected "
                "during isPrototypeOf call"));
	);

	return false;
}

void
as_object::dump_members() 
{
	log_debug(_("%d members of object %p follow"),
		_members.size(), (const void*)this);
	_members.dump();
}

void
as_object::dump_members(std::map<std::string, as_value>& to)
{
	_members.dump(to);
}

void
as_object::setPropFlags(const as_value& props_val, int set_false, int set_true)
{

	if (props_val.is_null()) {
		// Take all the members of the object
		_members.setFlagsAll(set_true, set_false);
		return;
	}

    std::string propstr = props_val.to_string();

    for (;;) {

        std::string prop;
        size_t next_comma=propstr.find(",");
        if (next_comma == std::string::npos) {
            prop = propstr;
        } 
        else {
            prop = propstr.substr(0,next_comma);
            propstr = propstr.substr(next_comma+1);
        }

        // set_member_flags will take care of case conversion
        if (!set_member_flags(getStringTable(*this).find(prop), set_true, set_false) )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Can't set propflags on object "
                "property %s "
                "(either not found or protected)"),	prop);
            );
        }

        if (next_comma == std::string::npos) {
            break;
        }
    }
    return;
}


void
as_object::copyProperties(const as_object& o)
{
	PropsCopier copier(*this);

	// TODO: check if non-visible properties should be also copied !
	o.visitProperties<Exists>(copier);
}

void
as_object::enumeratePropertyKeys(as_environment& env) const
{

	assert(env.top(0).is_undefined());

    // Hack to handle MovieClips.
	if (displayObject()) {
        displayObject()->enumerateNonProperties(env);
    }

	// this set will keep track of visited objects,
	// to avoid infinite loops
	std::set<const as_object*> visited;

    PropertyList::PropertyTracker doneList;
	
	const as_object* current(this);
	while (current && visited.insert(current).second) {
		current->_members.enumerateKeys(env, doneList);
		current = current->get_prototype();
	}
}

void
enumerateProperties(as_object& obj, as_object::SortedPropertyList& to)
{

	// this set will keep track of visited objects,
	// to avoid infinite loops
	std::set<as_object*> visited;

    PropertyEnumerator e(obj, to);
	as_object* current(&obj);

	while (current && visited.insert(current).second) {
		current->visitProperties<IsEnumerable>(e);
		current = current->get_prototype();
	}

}


Property*
as_object::getOwnProperty(const ObjectURI& uri)
{
	return _members.getProperty(uri);
}

bool
as_object::hasOwnProperty(const ObjectURI& uri)
{
	return getOwnProperty(uri);
}

as_object*
as_object::get_prototype() const
{
	int swfVersion = getSWFVersion(*this);

	Property* prop = _members.getProperty(NSV::PROP_uuPROTOuu);
	if (!prop) return 0;
	if (!prop->visible(swfVersion)) return 0;

	as_value tmp = prop->getValue(*this);

	return tmp.to_object(getGlobal(*this));
}

as_value
as_object::getMember(const ObjectURI& uri)
{
	as_value ret;
	get_member(uri, &ret);
	return ret;
}

as_object*
as_object::get_path_element(string_table::key key)
{
//#define DEBUG_TARGET_FINDING 1

	as_value tmp;
	if (!get_member(key, &tmp))
	{
#ifdef DEBUG_TARGET_FINDING 
		log_debug("Member %s not found in object %p",
			getStringTable(*this).value(key), (void*)this);
#endif
		return NULL;
	}
	if (!tmp.is_object())
	{
#ifdef DEBUG_TARGET_FINDING 
		log_debug("Member %s of object %p is not an object (%s)",
			getStringTable(*this).value(key), (void*)this, tmp);
#endif
		return NULL;
	}

	return tmp.to_object(getGlobal(*this));
}

void
getURLEncodedVars(as_object& o, std::string& data)
{
    as_object::SortedPropertyList props;
    enumerateProperties(o, props);

    std::string del;
    data.clear();
    
    for (as_object::SortedPropertyList::const_iterator i=props.begin(),
            e=props.end(); i!=e; ++i) {
        std::string name = i->first;
        std::string value = i->second;
        if (!name.empty() && name[0] == '$') continue; // see bug #22006
        URL::encode(value);

        data += del + name + "=" + value;

        del = "&";
    }
}

bool
as_object::watch(const ObjectURI& uri, as_function& trig,
		const as_value& cust)
{
	
	std::string propname = getStringTable(*this).value(getName(uri));

    if (!_trigs.get()) _trigs.reset(new TriggerContainer);

	TriggerContainer::iterator it = _trigs->find(uri);
	if (it == _trigs->end())
	{
		return _trigs->insert(
                std::make_pair(uri, Trigger(propname, trig, cust))).second;
	}
	it->second = Trigger(propname, trig, cust);
	return true;
}

bool
as_object::unwatch(const ObjectURI& uri)
{
    if (!_trigs.get()) return false; 

	TriggerContainer::iterator trigIter = _trigs->find(uri);
	if (trigIter == _trigs->end()) {
		log_debug("No watch for property %s",
                getStringTable(*this).value(getName(uri)));
		return false;
	}
	Property* prop = _members.getProperty(uri);
	if (prop && prop->isGetterSetter()) {
		log_debug("Watch on %s not removed (is a getter-setter)",
                getStringTable(*this).value(getName(uri)));
		return false;
	}
	trigIter->second.kill();
	return true;
}

#ifdef GNASH_USE_GC
void
as_object::markAsObjectReachable() const
{
	_members.setReachable();

    if (_trigs.get()) {
        for (TriggerContainer::const_iterator it = _trigs->begin();
                it != _trigs->end(); ++it)
        {
            it->second.setReachable();
        }
    }

    // Mark interfaces reachable.
    std::for_each(_interfaces.begin(), _interfaces.end(), 
            std::mem_fun(&as_object::setReachable));

    // Proxy objects can contain references to other as_objects.
    if (_relay) _relay->setReachable();
    if (_displayObject) _displayObject->setReachable();
}
#endif // GNASH_USE_GC

void
Trigger::setReachable() const
{
	_func->setReachable();
	_customArg.setReachable();
}

as_value
Trigger::call(const as_value& oldval, const as_value& newval,
        as_object& this_obj)
{
    assert(!_dead);

	if (_executing) return newval;

	_executing = true;

	try {
		as_environment env(getVM(this_obj));

        fn_call::Args args;
        args += _propname, oldval, newval, _customArg;

		fn_call fn(&this_obj, env, args);

		as_value ret = _func->call(fn);

		_executing = false;

		return ret;

	}
	catch (GnashException&) {
		_executing = false;
		throw;
	}
}

as_object*
getObjectWithPrototype(Global_as& gl, string_table::key c)
{
    as_object* ctor = gl.getMember(c).to_object(gl);
    as_object* proto = ctor ?
        ctor->getMember(NSV::PROP_PROTOTYPE).to_object(gl) : 0;

    as_object* o = gl.createObject();
    o->set_prototype(proto ? proto : as_value());
    return o;
}

/// Get the VM from an as_object
VM&
getVM(const as_object& o)
{
    return o.vm();
}

/// Get the movie_root from an as_object
movie_root&
getRoot(const as_object& o)
{
    return o.vm().getRoot();
}

/// Get the string_table from an as_object
string_table&
getStringTable(const as_object& o)
{
    return o.vm().getStringTable();
}

const RunResources&
getRunResources(const as_object& o)
{
    return o.vm().getRoot().runResources();
}

int
getSWFVersion(const as_object& o)
{
    return o.vm().getSWFVersion();
}

Global_as&
getGlobal(const as_object& o)
{
    return *o.vm().getGlobal();
}

bool
isAS3(const as_object& o)
{
    return isAS3(getVM(o));
}

} // end of gnash namespace
