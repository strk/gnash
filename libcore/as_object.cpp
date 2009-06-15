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
#include "as_environment.h" // for enumerateProperties
#include "movie_root.h" // for enumerateProperties
#include "event_id.h" // for enumerateProperties
#include "Property.h" // for findGetterSetter
#include "VM.h"
#include "GnashException.h"
#include "fn_call.h" // for generic methods
#include "Object.h" // for getObjectInterface
#include "action.h" // for call_method
#include "Array_as.h" // for setPropFlags
#include "as_function.h" // for inheritance of as_super

#include <set>
#include <string>
#include <boost/algorithm/string/case_conv.hpp>
#include <utility> // for std::pair
#include "namedStrings.h"
#include "asName.h"
#include "asClass.h"

// Anonymous namespace used for module-static defs
namespace {

using namespace gnash;

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
class as_super : public as_function
{
public:

	as_super(as_function* ctor, as_object* proto)
		:
		_ctor(ctor),
		_proto(proto)
	{
		set_prototype(proto);
		//log_debug("as_super %p constructed with ctor %p and proto %p", this, ctor, proto);
	}

	virtual bool isSuper() const { return true; }

	virtual as_object* get_super(const char* fname=0);

	std::string get_text_value() const
	{
		return "[object Object]";
	}

	// Fetching members from 'super' yelds a lookup on the associated prototype
	virtual bool get_member(string_table::key name, as_value* val,
		string_table::key nsname = 0)
	{
		log_debug("as_super::get_member %s called - _proto is %p", getVM().getStringTable().value(name), _proto);
		if ( _proto ) return _proto->get_member(name, val, nsname);
		log_debug("Super has no associated prototype");
		return false;
	}

	// Setting members on 'super' is a no-op
	virtual void set_member(string_table::key /*key*/, const as_value& /*val*/,
		string_table::key /*nsname*/ = 0)
	{
		log_debug("set_member.");
		// can't assign to super
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("Can't set members on the 'super' object");
		);
	}

	/// Dispatch.
	virtual as_value operator()(const fn_call& fn)
	{
		//log_debug("Super call operator. fn.this_ptr is %p", fn.this_ptr);
		if ( _ctor ) return _ctor->call(fn);
		log_debug("Super has no associated constructor");
		return as_value();
	}

protected:

	virtual void markReachableResources() const
	{
		if ( _ctor ) _ctor->setReachable();
		if ( _proto ) _proto->setReachable();
		markAsFunctionReachable();
	}

private:

	as_function* _ctor;
	as_object* _proto;
};

as_object*
as_super::get_super(const char* fname)
{
	// Super references the super class of our class prototype.
	// Our class prototype is __proto__.
	// Our class superclass prototype is __proto__.__proto__

	// Our class prototype is __proto__.
	as_object* proto = get_prototype().get(); 
	if ( ! proto )
	{
		//log_debug("We (a super) have no associated prototype, returning a null-referencing as_super from get_super()");
		return new as_super(0, 0);
	}

	// proto's __proto__ is superProto 
	as_object* superProto = proto->get_prototype().get();

	// proto's __constructor__ is superCtor
	as_function* superCtor = proto->get_constructor();
	assert(superCtor == get_constructor());

	//log_debug("super %p proto is %p, its prototype %p", this, proto, proto->get_prototype());

	VM& vm = getVM();
	if ( fname && vm.getSWFVersion() > 6)
	{
		as_object* owner = 0;
		string_table& st = vm.getStringTable();
		string_table::key k = st.find(fname);

		proto->findProperty(k, 0, &owner);
		if ( ! owner )
		{
			//log_debug("get_super: can't find property %s", fname);
			return 0;
		}

		//log_debug("object containing method %s is %p, its __proto__ is %p", fname, owner, owner->get_prototype());

		assert(owner);

		if ( owner != proto )
		{
			as_object* tmp = proto;
			while (tmp && tmp->get_prototype() != owner) tmp = tmp->get_prototype().get();
			// ok, now 'tmp' should be the object whose __proto__ member contains
			// the actual named method.
			//
			// in the C:B:A:F case this would be B when calling super.myName() from
			// C.prototype.myName()
			//

			assert(tmp); // well, since we found the property, it must be somewhere!

			//log_debug("tmp is %p", tmp);

			if ( tmp != proto )
			{
				//assert(superProto == tmp->get_prototype().get());

				//superCtor = superProto->get_constructor();
				superCtor = tmp->get_constructor();
				//if ( ! superCtor ) log_debug("superProto (owner) has no __constructor__");
			}
			else
			{
				//log_debug("tmp == proto");
				superCtor = owner->get_constructor(); // most likely..
				if ( superProto ) superProto = superProto->get_prototype().get();
			}
		}
		else
		{
			// TODO: check if we've anything to do here...
			//log_debug("owner == proto == %p", owner);
			//if ( superProto ) superProto = superProto->get_prototype().get();
			//superCtor = superProto->get_constructor();
			//if ( superProto )
			//{
			//	superCtor = superProto->get_constructor();
			//} // else superCtor = NULL ?
		}
	}

	as_object* super = new as_super(superCtor, superProto);

	return super;
}


/// A PropertyList visitor copying properties to an object
class PropsCopier : public AbstractPropertyVisitor {

	as_object& _tgt;

public:

	/// \brief
	/// Initialize a PropsCopier instance associating it
	/// with a target object (an object whose members has to be set)
	///
	PropsCopier(as_object& tgt)
		:
		_tgt(tgt)
	{}

	/// \brief
	/// Use the set_member function to properly set *inherited* properties
	/// of the given target object
	///
	void accept(string_table::key name, const as_value& val)
	{
		if (name == NSV::PROP_uuPROTOuu) return;
		//log_debug(_("Setting member '%s' to value '%s'"), name, val);
		_tgt.set_member(name, val);
	}
};

} // end of anonymous namespace


namespace gnash {

void
as_object::add_property(const std::string& name, as_function& getter,
		as_function* setter)
{
	string_table &st = _vm.getStringTable();
	string_table::key k = st.find(name);

	as_value cacheVal;

	Property* prop = _members.getProperty(k);
	if ( prop )
	{
		cacheVal = prop->getCache();

        // Used to return the return value of addGetterSetter, but this
        // is always true.
		_members.addGetterSetter(k, getter, setter, cacheVal);

        return;
		// NOTE: watch triggers not called when adding a new
        // getter-setter property
	}
	else
	{

		_members.addGetterSetter(k, getter, setter, cacheVal);

#if 1
		// check if we have a trigger, if so, invoke it
		// and set val to its return
		TriggerContainer::iterator trigIter = _trigs.find(std::make_pair(k, 0));
		if ( trigIter != _trigs.end() )
		{
			Trigger& trig = trigIter->second;

			log_debug("add_property: property %s is being watched, "
                    "current val: %s", name, cacheVal);
			cacheVal = trig.call(cacheVal, as_value(), *this);

			// The trigger call could have deleted the property,
			// so we check for its existance again, and do NOT put
			// it back in if it was deleted
			prop = _members.getProperty(k);
			if ( ! prop )
			{
				log_debug("Property %s deleted by trigger on create "
                        "(getter-setter)", name);
				return;
			}
			prop->setCache(cacheVal);
		}
#endif
		return;
	}
}


bool
as_object::get_member(string_table::key name, as_value* val,
	string_table::key nsname)
{
	assert(val);
	Property* prop = findProperty(name, nsname);
	
    if (!prop) {

        /// If the property isn't found, try the __resolve property.
        prop = findProperty(NSV::PROP_uuRESOLVE, nsname);
        if (!prop) return false;

        /// If __resolve exists, call it with the name of the undefined
        /// property.
        string_table& st = _vm.getStringTable();
        const std::string& undefinedName = st.value(name);
        log_debug("__resolve exists, calling with '%s'", undefinedName);

        *val = callMethod(NSV::PROP_uuRESOLVE, undefinedName);
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
		// TODO: check if this should be an 'as' error.. (log_aserror)
		log_error(_("Caught exception: %s"), exc.what());
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
		obj = obj->get_prototype().get();
		if (!obj)
			return NULL;
	}

	return obj->_members.getPropertyByOrder(index);
}

as_object*
as_object::get_super(const char* fname)
{
	// Super references the super class of our class prototype.
	// Our class prototype is __proto__.
	// Our class superclass prototype is __proto__.__proto__

	// Our class prototype is __proto__.
	as_object* proto = get_prototype().get();

	VM& vm = getVM();
	if ( fname && vm.getSWFVersion() > 6)
	{
		as_object* owner = 0;
		string_table& st = vm.getStringTable();
		string_table::key k = st.find(fname);
		findProperty(k, 0, &owner);
        // should be 0 if findProperty returned 0
		if (owner != this) proto = owner; 
	}

	// proto's __proto__ is superProto 
	as_object* superProto = proto ? proto->get_prototype().get() : 0;

	// proto's __constructor__ is superCtor
	as_function* superCtor = proto ? proto->get_constructor() : 0;

	as_object* super = new as_super(superCtor, superProto);

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
	return ctorVal.to_as_function();
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
		obj = obj->get_prototype().get();
		if (!obj)
			return 0;
	}
	
	const Property *p = obj->_members.getOrderAfter(index);
	if (!p)
	{
		obj = obj->get_prototype().get();
		if (!obj)
			return 0;
		p = obj->_members.getOrderAfter(0);
		++depth;
	}
	if (p)
	{
		if (findProperty(p->getName(), p->getNamespace()) != p)
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
as_object::findProperty(string_table::key key, string_table::key nsname, 
	as_object **owner)
{
	int swfVersion = _vm.getSWFVersion();

	// don't enter an infinite loop looking for __proto__ ...
	if (key == NSV::PROP_uuPROTOuu && !nsname)
	{
		Property* prop = _members.getProperty(key, nsname);
		// TODO: add ignoreVisibility parameter to allow using 
        // __proto__ even when not visible ?
		if (prop && prop->visible(swfVersion))
		{
			if (owner) *owner = this;
			return prop;
		}
		return 0;
	}

	// keep track of visited objects, avoid infinite loops.
	std::set<as_object*> visited;

	int i = 0;

	boost::intrusive_ptr<as_object> obj = this;
		
    // This recursion prevention seems not to exist in the PP.
    // Instead, it stops when its general timeout for the
    // execution of scripts is reached.
	while (obj && visited.insert(obj.get()).second)
	{
		++i;
		if ((i > 255 && swfVersion == 5) || i > 257)
			throw ActionLimitException("Lookup depth exceeded.");

		Property* prop = obj->_members.getProperty(key);
		if (prop && prop->visible(swfVersion) )
		{
			if (owner != NULL)
				*owner = obj.get();
			return prop;
		}
		else
			obj = obj->get_prototype();
	}

	// No Property found
	return NULL;
}

Property*
as_object::findUpdatableProperty(string_table::key key, string_table::key nsname)
{
	int swfVersion = _vm.getSWFVersion();

	Property* prop = _members.getProperty(key, nsname);
	// 
	// We won't scan the inheritance chain if we find a member,
	// even if invisible.
	// 
	if ( prop )	return prop;  // TODO: what about visible ?

	// don't enter an infinite loop looking for __proto__ ...
	if (key == NSV::PROP_uuPROTOuu) return NULL;

	std::set<as_object*> visited;
	visited.insert(this);

	int i = 0;

	boost::intrusive_ptr<as_object> obj = get_prototype();

    // TODO: does this recursion protection exist in the PP?
	while (obj && visited.insert(obj.get()).second)
	{
		++i;
		if ((i > 255 && swfVersion == 5) || i > 257)
			throw ActionLimitException("Property lookup depth exceeded.");

		Property* p = obj->_members.getProperty(key, nsname);
		if (p && (p->isGetterSetter() | p->isStatic()) && p->visible(swfVersion))
		{
			return p; // What should we do if this is not a getter/setter ?
		}
		obj = obj->get_prototype();
	}
	return NULL;
}

/*protected*/
void
as_object::set_prototype(boost::intrusive_ptr<as_object> proto, int flags)
{
	// TODO: check what happens if __proto__ is set as a user-defined 
    // getter/setter
	// TODO: check triggers !!
	_members.setValue(NSV::PROP_uuPROTOuu, as_value(proto.get()), *this, 0,
            flags);
}

void
as_object::reserveSlot(string_table::key name, string_table::key nsId,
	unsigned short slotId)
{
	_members.reserveSlot(name, nsId, slotId);
}

bool
as_object::get_member_slot(int order, as_value* val){
	
	const Property* prop = _members.getPropertyByOrder(order);
	if(prop){
		return get_member(prop->getName(), val, prop->getNamespace());
	}
	else{
		return false;
	}
}


bool
as_object::set_member_slot(int order, const as_value& val, bool ifFound)
{
	const Property* prop = _members.getPropertyByOrder(order);
	if(prop){
		return set_member(prop->getName(), val, prop->getNamespace(), ifFound);
	}
	else{
		return false;
	}
}

// Handles read_only and static properties properly.
bool
as_object::set_member(string_table::key key, const as_value& val,
	string_table::key nsname, bool ifFound)
{
	//log_debug(_("set_member_default(%s)"), key);
	Property* prop = findUpdatableProperty(key, nsname);
	if (prop)
	{
		if (prop->isReadOnly())
		{
			IF_VERBOSE_ASCODING_ERRORS(log_aserror(_(""
				"Attempt to set read-only property '%s'"),
				_vm.getStringTable().value(key)););
			return true;
		}

		try
		{
			// check if we have a trigger, if so, invoke it
			// and set val to it's return
			TriggerContainer::iterator trigIter = _trigs.find(std::make_pair(key, nsname));
			if ( trigIter != _trigs.end() )
			{
				Trigger& trig = trigIter->second;

				// WARNING: getValue might itself invoke a trigger
				// (getter-setter)... ouch ?
				// TODO: in this case, return the underlying value !
				as_value curVal = prop->getCache(); // getValue(*this); 

				log_debug("Existing property %s is being watched: firing trigger on update (current val:%s, new val:%s)",
					_vm.getStringTable().value(key), curVal, val);
				as_value newVal = trig.call(curVal, val, *this);
				// The trigger call could have deleted the property,
				// so we check for its existance again, and do NOT put
				// it back in if it was deleted
				prop = findUpdatableProperty(key, nsname);
				if ( ! prop )
				{
					log_debug("Property %s deleted by trigger on update", _vm.getStringTable().value(key));
					return true;
				}

				//if ( prop->isGetterSetter() ) prop->setCache(newVal); 
				prop->setValue(*this, newVal); 
			}
			else
			{
				// log_debug("No trigger for key %d ns %d", key, nsname);
				prop->setValue(*this, val);
			}

			prop->clearVisible(_vm.getSWFVersion());
		}
		catch (ActionTypeError& exc)
		{
			log_aserror(_("%s: Exception %s. Will create a new member"),
				_vm.getStringTable().value(key), exc.what());
		}

		return true;
	}

	// Else, add new property...
	if ( ifFound ) return false;

	// Property does not exist, so it won't be read-only. Set it.
	if (!_members.setValue(key, val, *this, nsname))
	{
		IF_VERBOSE_ASCODING_ERRORS(
			log_aserror(_("Unknown failure in setting property '%s' on "
			"object '%p'"), _vm.getStringTable().value(key), (void*) this);
	    );
		return false;
	}

	// Now check if we have a trigger, if so, invoke it
	// and reset val to it's return
	// NOTE that we do this *after* setting it in first place 
	// as the trigger seems allowed to delete the property again
	TriggerContainer::iterator trigIter = _trigs.find(std::make_pair(key, nsname));
	if ( trigIter != _trigs.end() )
	{
		Trigger& trig = trigIter->second;

		log_debug("Property %s is being watched, calling trigger on create", _vm.getStringTable().value(key));

		// NOTE: the trigger call might delete the propery being added
		//       so we first add the property, then call the trigger
		//       and finally check if the property still exists (ufff...)
		//

		as_value curVal; // undefined, didn't exist...
		as_value newVal = trig.call(curVal, val, *this);
		Property* prop = _members.getProperty(key);
		if ( ! prop )
		{
			log_debug("Property %s deleted by trigger on create", 
                    _vm.getStringTable().value(key));
		}
		else
		{
			prop->setValue(*this, newVal);
		}
	}

	return false;
}


void
as_object::init_member(const std::string& key1, const as_value& val, int flags,
	string_table::key nsname)
{
	init_member(_vm.getStringTable().find(PROPNAME(key1)), val, flags, nsname);
}

void
as_object::init_member(string_table::key key, const as_value& val, int flags,
	string_table::key nsname, int order)
{

	if (order >= 0 && !_members.
		reserveSlot(static_cast<unsigned short>(order), key, nsname))
	{
		log_error(_("Attempt to set a slot for either a slot or a property "
			"which already exists."));
		return;
	}
		
	// Set (or create) a SimpleProperty 
	if (! _members.setValue(key, val, *this, nsname, flags) )
	{
		log_error(_("Attempt to initialize read-only property ``%s''"
			" on object ``%p'' twice"),
			_vm.getStringTable().value(key), (void*)this);
		// We shouldn't attempt to initialize a member twice, should we ?
		abort();
	}
}

void
as_object::init_property(const std::string& key, as_function& getter,
		as_function& setter, int flags, string_table::key nsname)
{
	string_table::key k = _vm.getStringTable().find(PROPNAME(key));
	init_property(k, getter, setter, flags, nsname);
}

void
as_object::init_property(string_table::key key, as_function& getter,
		as_function& setter, int flags, string_table::key nsname)
{
	as_value cacheValue;

    // PropertyList::addGetterSetter always returns true (used to be
    // an assert).
	_members.addGetterSetter(key, getter, &setter, cacheValue, flags, nsname);
}

void
as_object::init_property(const std::string& key, as_c_function_ptr getter,
		as_c_function_ptr setter, int flags, string_table::key nsname)
{
	string_table::key k = _vm.getStringTable().find(PROPNAME(key));
	init_property(k, getter, setter, flags, nsname);
}

void
as_object::init_property(string_table::key key, as_c_function_ptr getter,
		as_c_function_ptr setter, int flags, string_table::key nsname)
{
    // PropertyList::addGetterSetter always returns true (used to be
    // an assert).
	_members.addGetterSetter(key, getter, setter, flags, nsname);
}

bool
as_object::init_destructive_property(string_table::key key, as_function& getter,
	int flags, string_table::key nsname)
{
	// No case check, since we've already got the key.
	bool success = _members.addDestructiveGetter(key, getter, nsname, flags);
	return success;
}

bool
as_object::init_destructive_property(string_table::key key,
        as_c_function_ptr getter, int flags, string_table::key nsname)
{
	// No case check, since we've already got the key.
	bool success = _members.addDestructiveGetter(key, getter, nsname, flags);
	return success;
}

void
as_object::init_readonly_property(const std::string& key, as_function& getter,
	int initflags, string_table::key nsname)
{
	string_table::key k = _vm.getStringTable().find(PROPNAME(key));

	init_property(k, getter, getter, initflags | as_prop_flags::readOnly
		| as_prop_flags::isProtected, nsname);
	assert(_members.getProperty(k, nsname));
}

void
as_object::init_readonly_property(const string_table::key& k,
        as_function& getter, int initflags, string_table::key nsname)
{
	init_property(k, getter, getter, initflags | as_prop_flags::readOnly
		| as_prop_flags::isProtected, nsname);
	assert(_members.getProperty(k, nsname));
}

void
as_object::init_readonly_property(const std::string& key,
        as_c_function_ptr getter, int initflags, string_table::key nsname)
{
	string_table::key k = _vm.getStringTable().find(PROPNAME(key));

	init_property(k, getter, getter, initflags | as_prop_flags::readOnly
		| as_prop_flags::isProtected, nsname);
	assert(_members.getProperty(k, nsname));
}

void
as_object::init_readonly_property(const string_table::key& k,
        as_c_function_ptr getter, int initflags, string_table::key nsname)
{
	init_property(k, getter, getter, initflags | as_prop_flags::readOnly
		| as_prop_flags::isProtected, nsname);
	assert(_members.getProperty(k, nsname));
}


bool
as_object::set_member_flags(string_table::key name,
		int setTrue, int setFalse, string_table::key nsname)
{
	return _members.setFlags(name, setTrue, setFalse, nsname);
}

void
as_object::add_interface(as_object* obj)
{
	assert(obj);

	if (std::find(mInterfaces.begin(), mInterfaces.end(), obj) == mInterfaces.end())
		mInterfaces.push_back(obj);
}

bool
as_object::instanceOf(as_object* ctor)
{
//#define GNASH_DEBUG_INSTANCE_OF 1

	as_value protoVal;
	if ( ! ctor->get_member(NSV::PROP_PROTOTYPE, &protoVal) )
	{
#ifdef GNASH_DEBUG_INSTANCE_OF
		log_debug("Object %p can't be an instance of an object (%p) w/out 'prototype'",
			(void*)this, (void*)ctor);
#endif
		return false;
	}
	as_object* ctorProto = protoVal.to_object().get();
	if ( ! ctorProto )
	{
#ifdef GNASH_DEBUG_INSTANCE_OF
		log_debug("Object %p can't be an instance of an object (%p) with non-object 'prototype' (%s)",
			(void*)this, (void*)ctor, protoVal);
#endif
		return false;
	}

	// TODO: cleanup the iteration, make it more readable ...

	std::set< as_object* > visited;

	as_object* obj = this;
	while (obj && visited.insert(obj).second )
	{
		as_object* thisProto = obj->get_prototype().get();
		if ( ! thisProto )
		{
			break;
		}

		// Check our proto
		if ( thisProto == ctorProto )
		{
#ifdef GNASH_DEBUG_INSTANCE_OF
			log_debug("Object %p is an instance of constructor %p as the constructor exposes our __proto__ %p",
				(void*)obj, (void*)ctor, (void*)thisProto);
#endif
			return true;
		}

		// Check our proto interfaces
		if (std::find(thisProto->mInterfaces.begin(), thisProto->mInterfaces.end(), ctorProto) != thisProto->mInterfaces.end())
		{
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
	boost::intrusive_ptr<as_object> obj = &instance;

	std::set< as_object* > visited;

	while (obj && visited.insert(obj.get()).second )
	{
		if ( obj->get_prototype() == this ) return true;
		obj = obj->get_prototype(); 
	}

	// See actionscript.all/Inheritance.as for a way to trigger this
	IF_VERBOSE_ASCODING_ERRORS(
	if ( obj ) log_aserror(_("Circular inheritance chain detected during isPrototypeOf call"));
	);

	return false;
}

void
as_object::dump_members() 
{
	log_debug(_("%d members of object %p follow"),
		_members.size(), (const void*)this);
	_members.dump(*this);
}

void
as_object::dump_members(std::map<std::string, as_value>& to)
{
	_members.dump(*this, to);
}

class FlagsSetterVisitor {
	string_table& _st;
	PropertyList& _pl;
	int _setTrue;
	int _setFalse;
public:
	FlagsSetterVisitor(string_table& st, PropertyList& pl, int setTrue, int setFalse)
		:
		_st(st),
		_pl(pl),
		_setTrue(setTrue),
		_setFalse(setFalse)
	{}

	void visit(as_value& v)
	{
		string_table::key key = _st.find(v.to_string());
		_pl.setFlags(key, _setTrue, _setFalse);
	}
};

void
as_object::setPropFlags(const as_value& props_val, int set_false, int set_true)
{
	if (props_val.is_string())
	{
		std::string propstr = PROPNAME(props_val.to_string()); 

		for(;;)
		{
			std::string prop;
			size_t next_comma=propstr.find(",");
			if ( next_comma == std::string::npos )
			{
				prop=propstr;
			} 
			else
			{
				prop=propstr.substr(0,next_comma);
				propstr=propstr.substr(next_comma+1);
			}

			// set_member_flags will take care of case conversion
			if (!set_member_flags(_vm.getStringTable().find(prop), set_true, set_false) )
			{
				IF_VERBOSE_ASCODING_ERRORS(
				log_aserror(_("Can't set propflags on object "
					"property %s "
					"(either not found or protected)"),	prop);
				);
			}

			if ( next_comma == std::string::npos )
			{
				break;
			}
		}
		return;
	}

	if (props_val.is_null())
	{
		// Take all the members of the object
		_members.setFlagsAll(set_true, set_false);

		// Are we sure we need to descend to __proto__ ?
		// should we recurse then ?
#if 0
		if (m_prototype)
		{
			m_prototype->_members.setFlagsAll(set_true, set_false);
		}
#endif
		return;
	}

	boost::intrusive_ptr<as_object> props = props_val.to_object();
	Array_as* ary = dynamic_cast<Array_as*>(props.get());
	if ( ! ary )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Invalid call to AsSetPropFlags: "
			"invalid second argument %s "
			"(expected string, null or an array)"),
			props_val);
		);
		return;
	}

	// The passed argument has to be considered an array
	//std::pair<size_t, size_t> result = 
	FlagsSetterVisitor visitor(getVM().getStringTable(), _members, set_true, set_false);
	ary->visitAll(visitor);
	//_members.setFlagsAll(props->_members, set_true, set_false);
}


void
as_object::copyProperties(const as_object& o)
{
	PropsCopier copier(*this);

	// TODO: check if non-visible properties should be also copied !
	o.visitPropertyValues(copier);
}

void
as_object::enumerateProperties(as_environment& env) const
{
	assert(env.top(0).is_undefined());

	enumerateNonProperties(env);

	// this set will keep track of visited objects,
	// to avoid infinite loops
	std::set< const as_object* > visited;
	PropertyList::propNameSet named;

	boost::intrusive_ptr<const as_object> obj(this);
	
	while ( obj && visited.insert(obj.get()).second )
	{
		obj->_members.enumerateKeys(env, named);
		obj = obj->get_prototype();
	}

	// This happens always since top object in hierarchy
	// is always Object, which in turn derives from itself
	//if ( obj ) log_error(_("prototype loop during Enumeration"));
}

void
as_object::enumerateProperties(SortedPropertyList& to) const
{

	// this set will keep track of visited objects,
	// to avoid infinite loops
	std::set< const as_object* > visited;

	boost::intrusive_ptr<const as_object> obj(this);
	while ( obj && visited.insert(obj.get()).second )
	{
		obj->_members.enumerateKeyValue(*this, to);
		obj = obj->get_prototype();
	}

}

as_object::as_object(movie_root& mr)
	:
	_vm(mr.getVM()),
	_members(_vm)
{
}


as_object::as_object()
	:
	_vm(VM::get()),
	_members(_vm)
{
}

as_object::as_object(as_object* proto)
	:
	_vm(VM::get()),
	_members(_vm)
{
	init_member(NSV::PROP_uuPROTOuu, as_value(proto));
}

as_object::as_object(boost::intrusive_ptr<as_object> proto)
	:
	_vm(VM::get()),
	_members(_vm)
{
	init_member(NSV::PROP_uuPROTOuu, as_value(proto));
}

as_object::as_object(const as_object& other)
	:
#ifndef GNASH_USE_GC
	ref_counted(),
#else
	GcResource(), 
#endif
	_vm(VM::get()),
	_members(other._members)
{
}

std::pair<bool,bool>
as_object::delProperty(string_table::key name, string_table::key nsname)
{
	return _members.delProperty(name, nsname);
}

Property*
as_object::getOwnProperty(string_table::key key, string_table::key nsname)
{
	return _members.getProperty(key, nsname);
}

bool
as_object::hasOwnProperty(string_table::key key, string_table::key nsname)
{
	return getOwnProperty(key, nsname) != NULL;
}

as_value
as_object::tostring_method(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = fn.this_ptr;

	std::string text_val = obj->get_text_value();
	return as_value(text_val);
}

as_value
as_object::valueof_method(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = fn.this_ptr;

	return obj->get_primitive_value();
}

boost::intrusive_ptr<as_object>
as_object::get_prototype()
{
	int swfVersion = _vm.getSWFVersion();

	Property* prop = _members.getProperty(NSV::PROP_uuPROTOuu);
	if ( ! prop ) return 0;
	if ( ! prop->visible(swfVersion) ) return 0;

	as_value tmp = prop->getValue(*this);

	return tmp.to_object();
}

bool
as_object::on_event(const event_id& id )
{
	as_value event_handler;

	if (get_member(id.functionKey(), &event_handler) )
	{
		call_method0(event_handler, as_environment(_vm), this);
		return true;
	}

	return false;
}

as_value
as_object::getMember(string_table::key name, string_table::key nsname)
{
	as_value ret;
	get_member(name, &ret, nsname);
	return ret;
}

as_value
as_object::callMethod(string_table::key methodName)
{
	as_value method;

	if (! get_member(methodName, &method))
	{
		return as_value();
	}

	as_environment env(_vm);

	return call_method0(method, env, this);
}

as_value
as_object::callMethod(string_table::key methodName, const as_value& arg0)
{
	as_value method;

	if (!get_member(methodName, &method))
	{
		return as_value();
	}

	as_environment env(_vm);

	std::auto_ptr< std::vector<as_value> > args ( new std::vector<as_value> );
	args->push_back(arg0);

	return call_method(method, env, this, args);
}

as_value
as_object::callMethod(string_table::key methodName, const as_value& arg0,
        const as_value& arg1)
{
	as_value method;

	if (! get_member(methodName, &method))
	{
		return as_value();
	}

	as_environment env(_vm);

	std::auto_ptr< std::vector<as_value> > args ( new std::vector<as_value> );
	args->push_back(arg0);
	args->push_back(arg1);

	return call_method(method, env, this, args);
}

as_value
as_object::callMethod(string_table::key methodName,
	const as_value& arg0, const as_value& arg1, const as_value& arg2)
{
	as_value ret;
	as_value method;

	if (! get_member(methodName, &method))
	{
		return ret;
	}

	as_environment env(_vm);

	std::auto_ptr< std::vector<as_value> > args ( new std::vector<as_value> );
	args->push_back(arg0);
	args->push_back(arg1);
	args->push_back(arg2);

	ret = call_method(method, env, this, args);

	return ret;
}

as_value
as_object::callMethod(string_table::key methodName, const as_value& arg0,
        const as_value& arg1, const as_value& arg2, const as_value& arg3)
{
	as_value method;

	if (! get_member(methodName, &method))
	{
		return as_value();
	}

	as_environment env(_vm);

	std::auto_ptr< std::vector<as_value> > args ( new std::vector<as_value> );
	args->push_back(arg0);
	args->push_back(arg1);
	args->push_back(arg2);
	args->push_back(arg3);

	return call_method(method, env, this, args);

}

as_object*
as_object::get_path_element(string_table::key key)
{
//#define DEBUG_TARGET_FINDING 1

	as_value tmp;
	if ( ! get_member(key, &tmp ) )
	{
#ifdef DEBUG_TARGET_FINDING 
		log_debug("Member %s not found in object %p",
			_vm.getStringTable().value(key), (void*)this);
#endif
		return NULL;
	}
	if ( ! tmp.is_object() )
	{
#ifdef DEBUG_TARGET_FINDING 
		log_debug("Member %s of object %p is not an object (%s)",
			_vm.getStringTable().value(key), (void*)this, tmp);
#endif
		return NULL;
	}

	return tmp.to_object().get();
}

void
as_object::getURLEncodedVars(std::string& data)
{
    SortedPropertyList props;
    enumerateProperties(props);

    std::string del;
    data.clear();
    
    for (SortedPropertyList::const_iterator i=props.begin(),
            e=props.end(); i!=e; ++i)
    {
      std::string name = i->first;
      std::string value = i->second;
      if ( ! name.empty() && name[0] == '$' ) continue; // see bug #22006
      URL::encode(value);
      
      data += del + name + "=" + value;
      
      del = "&";
        
    }
    
}

bool
as_object::watch(string_table::key key, as_function& trig,
		const as_value& cust, string_table::key ns)
{
	
	FQkey k = std::make_pair(key, ns);
	std::string propname = VM::get().getStringTable().value(key);

	TriggerContainer::iterator it = _trigs.find(k);
	if ( it == _trigs.end() )
	{
		return _trigs.insert(
                std::make_pair(k, Trigger(propname, trig, cust))).second;
	}
	it->second = Trigger(propname, trig, cust);
	return true;
}

bool
as_object::unwatch(string_table::key key, string_table::key ns)
{
	TriggerContainer::iterator trigIter = _trigs.find(std::make_pair(key, ns));
	if ( trigIter == _trigs.end() )
	{
		log_debug("No watch for property %s",
                getVM().getStringTable().value(key));
		return false;
	}
	Property* prop = _members.getProperty(key, ns);
	if ( prop && prop->isGetterSetter() )
	{
		log_debug("Watch on %s not removed (is a getter-setter)",
                getVM().getStringTable().value(key));
		return false;
	}
	_trigs.erase(trigIter);
	return true;
}

#ifdef GNASH_USE_GC
void
as_object::markAsObjectReachable() const
{
	_members.setReachable();

	for (TriggerContainer::const_iterator it = _trigs.begin();
			it != _trigs.end(); ++it)
	{
		it->second.setReachable();
	}
}
#endif // GNASH_USE_GC

void
Trigger::setReachable() const
{
	_func->setReachable();
	_customArg.setReachable();
}

as_value
Trigger::call(const as_value& oldval, const as_value& newval, as_object& this_obj)
{
	if ( _executing ) return newval;

	_executing = true;

	try {
		as_environment env(VM::get()); // TODO: get VM in some other way 

		std::auto_ptr<std::vector<as_value> > args (new std::vector<as_value>);
		args->push_back(_propname);
		args->push_back(oldval);
		args->push_back(newval);
		args->push_back(_customArg);

		fn_call fn(&this_obj, env, args);

		as_value ret = _func->call(fn);

		_executing = false;

		return ret;

	}
	catch (GnashException&)
	{
		_executing = false;
		throw;
	}
}

void
as_object::visitPropertyValues(AbstractPropertyVisitor& visitor) const
{
    _members.visitValues(visitor, *this);
}

void
as_object::visitNonHiddenPropertyValues(AbstractPropertyVisitor& visitor) const
{
    _members.visitNonHiddenValues(visitor, *this);
}



} // end of gnash namespace
