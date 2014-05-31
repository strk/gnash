// as_object.cpp:  ActionScript Object class and its properties, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include "as_object.h"

#include <set>
#include <string>
#include <boost/algorithm/string/case_conv.hpp>
#include <utility> // for std::pair

#include "RunResources.h"
#include "log.h"
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
        _condition(std::move(cmp))
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
    Property* getProperty(as_object** owner = nullptr) const {

        assert(_object);
        Property* prop = _object->_members.getProperty(_uri);
        
        if (prop && _condition(*prop)) {
            if (owner) *owner = _object;
            return prop;
        }
        return nullptr;
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

as_function*
getConstructor(as_object& o)
{
	as_value ctorVal;
	if (!o.get_member(NSV::PROP_uuCONSTRUCTORuu, &ctorVal)) {
		return nullptr;
	}
	return ctorVal.to_function();
}

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

    virtual as_object* get_super(const ObjectURI& fname);

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
        as_object::markReachableResources();
	}

private:

    as_object* prototype() {
        return _super ? _super->get_prototype() : nullptr;
    }

    as_function* constructor() {
        return _super ? getConstructor(*_super) : nullptr;
    }

    as_object* _super;
};

as_object*
as_super::get_super(const ObjectURI& fname)
{
    // Super references the super class of our class prototype.
    // Our class prototype is __proto__.
    // Our class superclass prototype is __proto__.__proto__

    // Our class prototype is __proto__.
    as_object* proto = get_prototype(); 
    if (!proto) return new as_super(getGlobal(*this), nullptr);

    if (fname.empty() || getSWFVersion(*this) <= 6) {
        return new as_super(getGlobal(*this), proto);
    }

    as_object* owner = nullptr;
    proto->findProperty(fname, &owner);
    if (!owner) return nullptr;

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
class PropsCopier : public PropertyVisitor
{

public:

    /// \brief
    /// Initialize a PropsCopier instance associating it
    /// with a target object (an object whose members has to be set)
    ///
    PropsCopier(as_object& tgt)
        :
        _tgt(tgt)
	{
    }

    /// Set *inherited* properties of the given target object
    bool accept(const ObjectURI& uri, const as_value& val) {
        if (getName(uri) == NSV::PROP_uuPROTOuu) return true;
        _tgt.set_member(uri, val);
        return true;
    }
private:
    as_object& _tgt;
};

class PropertyEnumerator : public PropertyVisitor
{
public:
    PropertyEnumerator(SortedPropertyList& to)
        :
        _to(to)
    {}

    bool accept(const ObjectURI& uri, const as_value& val) {
        _to.push_back(std::make_pair(uri, val));
        return true;
    }
private:
    SortedPropertyList& _to;
};

} // anonymous namespace


const int as_object::DefaultFlags;

as_object::as_object(const Global_as& gl)
    :
    GcResource(getRoot(gl).gc()),
    _displayObject(nullptr),
    _array(false),
    _vm(getVM(gl)),
    _members(*this)
{
}

as_object::as_object(VM& vm)
    :
    GcResource(vm.getRoot().gc()),
    _displayObject(nullptr),
    _array(false),
    _vm(vm),
    _members(*this)
{
}

as_value
as_object::call(const fn_call& /*fn*/)
{
    throw ActionTypeError();
}

std::string
as_object::stringValue() const
{
    return "[object Object]";
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
    const ObjectURI& uri = getURI(vm(), name);

    Property* prop = _members.getProperty(uri);

    if (prop) {
        const as_value& cacheVal = prop->getCache();
        // Used to return the return value of addGetterSetter, but this
        // is always true.
        _members.addGetterSetter(uri, getter, setter, cacheVal);
        return;
        // NOTE: watch triggers not called when adding a new
        // getter-setter property
    }
    else {

        _members.addGetterSetter(uri, getter, setter, as_value());

        // Nothing more to do if there are no triggers.
        if (!_trigs.get()) return;

        // check if we have a trigger, if so, invoke it
        // and set val to its return
        TriggerContainer::iterator trigIter = _trigs->find(uri);

        if (trigIter != _trigs->end()) {

            Trigger& trig = trigIter->second;

            log_debug("add_property: property %s is being watched", name);
            as_value v = trig.call(as_value(), as_value(), *this);

            // The trigger call could have deleted the property,
            // so we check for its existence again, and do NOT put
            // it back in if it was deleted
            prop = _members.getProperty(uri);
            if (!prop) {
                log_debug("Property %s deleted by trigger on create (getter-setter)", name);
                return;
            }
            prop->setCache(v);
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
            if (getDisplayObjectProperty(*d, uri, *val)) return true;
        }
        while (pr()) {
            if ((prop = pr.getProperty())) break;
        }
    }

    // If the property isn't found or doesn't apply to any objects in the
    // inheritance chain, try the __resolve property.
    if (!prop) {

        PrototypeRecursor<Exists> pr(this, NSV::PROP_uuRESOLVE);

        as_value resolve;

        for (;;) {
            Property* res = pr.getProperty();
            if (res) {
                resolve = res->isGetterSetter() ? res->getCache() :
                                                  res->getValue(*this);
                if (version < 7) break;
                if (resolve.is_object()) break;
            }
            // Finished searching.
            if (!pr()) return false;
        }

        // If __resolve exists, call it with the name of the undefined
        // property.
        string_table& st = getStringTable(*this);
        const std::string& undefinedName = st.value(getName(uri));

        fn_call::Args args;
        args += undefinedName;

        // Invoke the __resolve property.
        *val = invoke(resolve, as_environment(getVM(*this)), this, args);

        return true;
    }

    try {
        *val = prop->getValue(*this);
        return true;
    }
    catch (const ActionTypeError& exc) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Caught exception: %s"), exc.what());
            );
        return false;
    }
}


as_object*
as_object::get_super(const ObjectURI& fname)
{
    // Super references the super class of our class prototype.
    // Our class prototype is __proto__.
    // Our class superclass prototype is __proto__.__proto__

    // Our class prototype is __proto__.
    as_object* proto = get_prototype();

    if ( ! fname.empty() && getSWFVersion(*this) > 6) {
        as_object* owner = nullptr;
        findProperty(fname, &owner);
        // should be 0 if findProperty returned 0
        if (owner != this) proto = owner; 
    }

    as_object* super = new as_super(getGlobal(*this), proto);

    return super;
}

as_object*
as_object::get_super()
{
    // Our class prototype is __proto__.
    as_object* proto = get_prototype();
    as_object* super = new as_super(getGlobal(*this), proto);

    return super;
}

Property*
as_object::findProperty(const ObjectURI& uri, as_object** owner)
{

    const int version = getSWFVersion(*this);

    PrototypeRecursor<IsVisible> pr(this, uri, IsVisible(version));

    do {
        Property* prop = pr.getProperty(owner);
        if (prop) return prop;
    } while (pr());

    // No Property found
    return nullptr;
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
            if (prop->isGetterSetter() && visible(*prop, swfVersion)) {
                return prop;
            }
        }
    }
    return nullptr;
}

void
as_object::set_prototype(const as_value& proto)
{
    // TODO: check what happens if __proto__ is set as a user-defined 
    // getter/setter
    // TODO: check triggers !!
    _members.setValue(NSV::PROP_uuPROTOuu, proto, as_object::DefaultFlags);
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
    const as_value& curVal = prop ? prop->getCache() : as_value(); 
    const as_value& newVal = trig.call(curVal, val, *this);
    
    // This is a particularly clear and concise way of removing dead triggers.
    EraseIf(*_trigs, std::bind(std::mem_fn(&Trigger::dead),
             std::bind(&TriggerContainer::value_type::second,
             std::placeholders::_1)));
                    
    // The trigger call could have deleted the property,
    // so we check for its existence again, and do NOT put
    // it back in if it was deleted
    prop = findUpdatableProperty(uri);
    if (!prop) return;

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
as_object::set_member(const ObjectURI& uri, const as_value& val, bool ifFound)
{

    bool tfVarFound = false;
    if (displayObject()) {
        MovieClip* mc = dynamic_cast<MovieClip*>(displayObject());
        if (mc) tfVarFound = mc->setTextFieldVariables(uri, val);
        // We still need to set the member.
    }

    // Handle the length property for arrays. NB: checkArrayLength() will
    // call this function again if the key is a valid index.
    if (array()) checkArrayLength(*this, uri, val);

    PrototypeRecursor<Exists> pr(this, uri);

    Property* prop = pr.getProperty();

    // We won't scan the inheritance chain if we find a member,
    // even if invisible.
    if (!prop) { 
            
        if (displayObject()) {
            DisplayObject* d = displayObject();
            if (setDisplayObjectProperty(*d, uri, val)) return true;
            // TODO: should we execute triggers?
        }
            
        const int version = getSWFVersion(*this);
        while (pr()) {
            if ((prop = pr.getProperty())) {
                if ((prop->isGetterSetter()) && visible(*prop, version)) {
                    break;
                }
                else prop = nullptr;
            }
        }
    }
        
    if (prop) {
        if (readOnly(*prop)) {
            IF_VERBOSE_ASCODING_ERRORS(
                ObjectURI::Logger l(getStringTable(*this));
                log_aserror(_("Attempt to set read-only property '%s'"),
                            l(uri));
                );
            return true;
        }
            
        try {
            executeTriggers(prop, uri, val);
        }
        catch (const ActionTypeError& exc) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(
                _("%s: %s"), getStringTable(*this).value(getName(uri)), exc.what());
            );
        }
            
        return true;
    }
        
    // Else, add new property...
    if (ifFound) return false;
        
    // Property does not exist, so it won't be read-only. Set it.
    if (!_members.setValue(uri, val)) {
            
        IF_VERBOSE_ASCODING_ERRORS(
            ObjectURI::Logger l(getStringTable(*this));
            log_aserror(_("Unknown failure in setting property '%s' on "
                          "object '%p'"), l(uri), (void*) this);
            );
        return false;
    }
        
    executeTriggers(prop, uri, val);
        
    // Return true if we found a textfield variable.
    if (tfVarFound) return true;
        
    return false;
}


void
as_object::init_member(const std::string& key1, const as_value& val, int flags)
{
    const ObjectURI& uri(getURI(vm(), key1));
    init_member(uri, val, flags);
}

void
as_object::init_member(const ObjectURI& uri, const as_value& val, int flags)
{

    // Set (or create) a SimpleProperty 
    if (!_members.setValue(uri, val, flags)) {
        ObjectURI::Logger l(getStringTable(*this));
        log_error(_("Attempt to initialize read-only property '%s'"
                    " on object '%p' twice"), l(uri), (void*)this);
        // We shouldn't attempt to initialize a member twice, should we ?
        abort();
    }
}

void
as_object::init_property(const std::string& name, as_function& getter,
                         as_function& setter, int flags)
{
    const ObjectURI& uri = getURI(vm(), name);
    init_property(uri, getter, setter, flags);
}

void
as_object::init_property(const ObjectURI& uri, as_function& getter,
                         as_function& setter, int flags)
{
    _members.addGetterSetter(uri, getter, &setter, as_value(), flags);
}

void
as_object::init_property(const std::string& name, as_c_function_ptr getter,
                         as_c_function_ptr setter, int flags)
{
    const ObjectURI& uri = getURI(vm(), name);
    init_property(uri, getter, setter, flags);
}

void
as_object::init_property(const ObjectURI& uri, as_c_function_ptr getter,
                         as_c_function_ptr setter, int flags)
{
    _members.addGetterSetter(uri, getter, setter, flags);
}

bool
as_object::init_destructive_property(const ObjectURI& uri, as_function& getter,
                                     int flags)
{
    return _members.addDestructiveGetter(uri, getter, flags);
}

bool
as_object::init_destructive_property(const ObjectURI& uri,
                                     as_c_function_ptr getter, int flags)
{
    return _members.addDestructiveGetter(uri, getter, flags);
}

void
as_object::init_readonly_property(const std::string& name, as_function& getter,
                                  int initflags)
{
    const ObjectURI& uri = getURI(vm(), name);

    init_property(uri, getter, getter, initflags | PropFlags::readOnly);
    assert(_members.getProperty(uri));
}

void
as_object::init_readonly_property(const std::string& name,
                                  as_c_function_ptr getter, int initflags)
{
    const ObjectURI& uri = getURI(vm(), name);
    init_property(uri, getter, getter, initflags | PropFlags::readOnly);
    assert(_members.getProperty(uri));
}

void
as_object::set_member_flags(const ObjectURI& uri, int setTrue, int setFalse)
{
    _members.setFlags(uri, setTrue, setFalse);
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
        log_debug("Object %p can't be an instance of an object (%p) with no 'prototype'",
                  (void*)this, (void*)ctor);
#endif
        return false;
    }

    as_object* ctorProto = toObject(protoVal, getVM(*this));
    if (!ctorProto) {
#ifdef GNASH_DEBUG_INSTANCE_OF
        log_debug("Object %p can't be an instance of an object (%p) with non-object 'prototype' (%s)",
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
            log_debug("Object %p is an instance of constructor %p as the constructor exposes our __proto__ %p",
                      (void*)obj, (void*)ctor, (void*)thisProto);
#endif
            return true;
        }

        // Check our proto interfaces
        if (std::find(thisProto->_interfaces.begin(),
                      thisProto->_interfaces.end(), ctorProto)
            != thisProto->_interfaces.end()) {

#ifdef GNASH_DEBUG_INSTANCE_OF
            log_debug("Object %p __proto__ %p had one interface matching with the constructor prototype %p",
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
    as_object* obj = &instance;

    std::set<as_object*> visited;

    while (obj && visited.insert(obj).second ) {
        if (obj->get_prototype() == this) return true;
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
    log_debug("%d members of object %p follow", _members.size(),
            static_cast<const void*>(this));
    _members.dump();
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
        set_member_flags(getURI(vm(), prop), set_true, set_false);

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
as_object::visitKeys(KeyVisitor& visitor) const
{
    // Hack to handle MovieClips.
    if (displayObject()) {
        displayObject()->visitNonProperties(visitor);
    }

    // this set will keep track of visited objects,
    // to avoid infinite loops
    std::set<const as_object*> visited;

    PropertyList::PropertyTracker doneList;
	
    const as_object* current(this);
    while (current && visited.insert(current).second) {
        current->_members.visitKeys(visitor, doneList);
        current = current->get_prototype();
    }
}


Property*
as_object::getOwnProperty(const ObjectURI& uri)
{
    return _members.getProperty(uri);
}

as_object*
as_object::get_prototype() const
{
    int swfVersion = getSWFVersion(*this);
    
    Property* prop = _members.getProperty(NSV::PROP_uuPROTOuu);
    if (!prop) return nullptr;
    if (!visible(*prop, swfVersion)) return nullptr;
    
    const as_value& proto = prop->getValue(*this);
    
    return toObject(proto, getVM(*this));
}

std::string
getURLEncodedVars(as_object& o)
{
    SortedPropertyList props = enumerateProperties(o);

    std::string data;
    string_table& st = getStringTable(o);
    
    for (SortedPropertyList::const_reverse_iterator i = props.rbegin(),
            e = props.rend(); i != e; ++i) {

        const std::string& name = i->first.toString(st);
        const std::string& value = i->second.to_string();
        
        // see bug #22006
        if (!name.empty() && name[0] == '$') continue; 

        URL::encode(value);
        if (i != props.rbegin()) data += '&';

        data += name + "=" + value;

    }
    return data;
}

bool
as_object::watch(const ObjectURI& uri, as_function& trig,
		const as_value& cust)
{
	
    std::string propname = getStringTable(*this).value(getName(uri));

    if (!_trigs.get()) _trigs.reset(new TriggerContainer);

    TriggerContainer::iterator it = _trigs->find(uri);
    if (it == _trigs->end()) {
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

void
as_object::markReachableResources() const
{
    _members.setReachable();

    if (_trigs.get()) {
        for (TriggerContainer::const_iterator it = _trigs->begin();
             it != _trigs->end(); ++it) {
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

        const as_environment env(getVM(this_obj));
        
        fn_call::Args args;
        args += _propname, oldval, newval, _customArg;
        
        fn_call fn(&this_obj, env, args);
        as_value ret = _func->call(fn);
        _executing = false;
        
        return ret;
        
    }
    catch (const GnashException&) {
        _executing = false;
        throw;
    }
}

SortedPropertyList
enumerateProperties(as_object& obj)
{

    // this set will keep track of visited objects,
    // to avoid infinite loops
    std::set<as_object*> visited;

    SortedPropertyList to;
    PropertyEnumerator e(to);
    as_object* current(&obj);

    while (current && visited.insert(current).second) {
        current->visitProperties<IsEnumerable>(e);
        current = current->get_prototype();
    }
    return to;

}

as_object*
getPathElement(as_object& o, const ObjectURI& uri)
{
    as_value tmp;
    if (!o.get_member(uri, &tmp)) return nullptr;
    if (!tmp.is_object()) return nullptr;
    return toObject(tmp, getVM(o));
}


void
sendEvent(as_object& o, const as_environment& env, const ObjectURI& name)
{
    Property* prop = o.findProperty(name);
    if (prop) {
        fn_call::Args args;
        invoke(prop->getValue(o), env, &o, args);
    }
}

as_object*
getObjectWithPrototype(Global_as& gl, const ObjectURI& c)
{
    as_object* ctor = toObject(getMember(gl, c), getVM(gl));
    as_object* proto = ctor ? 
        toObject(getMember(*ctor, NSV::PROP_PROTOTYPE), getVM(gl)) : nullptr;

    as_object* o = createObject(gl);
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

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
