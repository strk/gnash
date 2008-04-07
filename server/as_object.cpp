// as_object.cpp:  ActionScript Object class and its properties, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include "as_object.h"
#include "as_function.h"
#include "as_environment.h" // for enumerateProperties
#include "Property.h" // for findGetterSetter
#include "VM.h"
#include "GnashException.h"
#include "fn_call.h" // for generic methods
#include "Object.h" // for getObjectInterface
#include "action.h" // for call_method
#include "array.h" // for setPropFlags
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
	}

	virtual bool isSuper() const { return true; }

	std::string get_text_value() const
	{
		return "[object Object]";
	}

	// Fetching members from 'super' yelds a lookup on the associated prototype
	virtual bool get_member(string_table::key name, as_value* val,
		string_table::key nsname = 0)
	{
		//log_debug("as_super::get_member %s called - _proto is %p", getVM().getStringTable().value(name).c_str(), _proto);
		if ( _proto ) return _proto->get_member(name, val, nsname);
		log_debug("Super has no associated prototype");
		return false;
	}

	// Setting members on 'super' is a no-op
	virtual void set_member(string_table::key /*key*/, const as_value& /*val*/,
		string_table::key /*nsname*/ = 0)
	{
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


// A PropertyList visitor copying properties to an object
class PropsCopier {

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
	void operator() (string_table::key name, const as_value& val)
	{
		if (name == NSV::PROP_uuPROTOuu) return;
		//log_debug(_("Setting member '%s' to value '%s'"), name.c_str(), val.to_debug_string().c_str());
		_tgt.set_member(name, val);
	}
};

} // end of anonymous namespace


namespace gnash {

bool
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
		return _members.addGetterSetter(k, getter, setter, cacheVal);

		// NOTE: watch triggers not called when adding a new getter-setter property
	}
	else
	{

		bool ret = _members.addGetterSetter(k, getter, setter, cacheVal);
		if (!ret) return false;

#if 1
		// check if we have a trigger, if so, invoke it
		// and set val to it's return
		TriggerContainer::iterator trigIter = _trigs.find(std::make_pair(k, 0));
		if ( trigIter != _trigs.end() )
		{
			Trigger& trig = trigIter->second;

			log_debug("add_property: property %s is being watched, current val: %s", name, cacheVal.to_debug_string());
			cacheVal = trig.call(cacheVal, as_value(), *this);

			// The trigger call could have deleted the property,
			// so we check for its existance again, and do NOT put
			// it back in if it was deleted
			prop = _members.getProperty(k);
			if ( ! prop )
			{
				log_debug("Property %s deleted by trigger on create (getter-setter)", name);
				return false; // or true ?
			}
			prop->setCache(cacheVal);
			//prop->setValue(*this, cacheVal);
		}
#endif

		return ret;
	}
}

bool
as_object::add_property(const std::string& name, as_c_function_ptr getter,
		as_c_function_ptr setter)
{
	string_table &st = _vm.getStringTable();
	return _members.addGetterSetter(st.find(PROPNAME(name)), getter, setter);
}

/*protected*/
bool
as_object::get_member_default(string_table::key name, as_value* val,
	string_table::key nsname)
{
	assert(val);

	Property* prop = findProperty(name, nsname);
	if (!prop)
		return false;

	try 
	{
		*val = prop->getValue(*this);
		return true;
	}
	catch (ActionLimitException& exc)
	{
		// will be logged by outer catcher
		throw;
	}
	catch (ActionException& exc)
	{
		// TODO: check if this should be an 'as' error.. (log_aserror)
		log_error(_("Caught exception: %s"), exc.what());
		return false;
	}
}

Property*
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

	return const_cast<Property *>(obj->_members.getPropertyByOrder(index));
}

as_object*
as_object::get_super()
{
#if 1
	// Assuming we're a prototype...

	// __constructor__ is superCtor
	as_function* superCtor = get_constructor();     

	// __proto__ is superProto
	// NOTE: we don't use get_prototype as it would skip the get_member 
	// override of as_super.. A solution would likely be having as_super
	// register the associated prototype as it's own __proto__ member
	// so that get_prototype() returns it and can be threated exactly
	// the same as other objects in ActionCallMethod
	// TODO: try this, for simplification
	//
	as_object* superProto = NULL; // get_prototype().get();
	as_value val;
	if ( get_member(NSV::PROP_uuPROTOuu, &val) )
	{
		superProto = val.to_object().get();
	}

	as_object* super = new as_super(superCtor, superProto);

	return super;
#else

	static bool getting = false;
	as_object *owner = NULL;

	Property *p = NULL;

	if (getting)
	{
		log_debug("Already getting super, return NULL");
		return NULL;
	}

	getting = true;

#if 1
	// Super is prototype.__constructor__
	p = findProperty(NSV::PROP_PROTOTYPE, 0, &owner);
	if (!p)
	{
		log_debug("This object (%s @ %p) has no 'prototype' get_super returns NULL", typeName(*this), (void*)this);
		getting = false;
		return NULL;
	}

	as_value protoval = p->getValue(*owner);
	as_object *proto = protoval.to_object().get();
	if (!proto)
	{
		log_debug("This object's (%s @ %p) 'prototype' member is not an object (%s) - get_super returns NULL", typeName(*this), (void*)this, protoval.to_debug_string());
		getting = false;
		return NULL;
	}

	as_function* constructor = proto->get_constructor();
	if ( ! constructor )
	{
		log_debug("This object's (%s @ %p) 'prototype' has no constructor", typeName(*this), (void*)this);
	}

	// prototype of the constructor is constructor.prototype (I think)
#if 1
	as_object* prototype = NULL;
	if ( constructor && constructor->get_member(NSV::PROP_PROTOTYPE, &protoval) )
	{
		prototype = protoval.to_object().get();
	}
#else
	as_object* prototype = get_prototype().get();
	if ( ! prototype )
	{
		log_debug("This object (%s @ %p) has no __proto__", typeName(*this), (void*)this);
	}
#endif

	getting = false;

	return new as_super(constructor, prototype);
#else
	// Super is this.__proto__.__constructor__.prototype
	as_object *proto = get_prototype().get();
	if (!proto)
	{
		getting = false;
		return NULL;
	}

	// If an object is its own prototype, we stop looking.
	if (proto == this)
	{
		getting = false;
		return this;
	}

	p = proto->findProperty(NSV::PROP_uuCONSTRUCTORuu, 0, &owner);
	if (!p)
	{
		getting = false;
		return NULL;
	}

	as_value ctor = p->getValue(*owner);
	as_object *ctor_obj = ctor.to_object().get();
	if (!ctor_obj)
	{
		getting = false;
		return NULL;
	}

	p = ctor_obj->findProperty(NSV::PROP_PROTOTYPE, 0, &owner);
	if (!p)
	{
		getting = false;
		return NULL;
	}

	as_value ctor_proto = p->getValue(*owner);
	as_object *super = ctor_proto.to_object().get();

	getting = false;

	return super;
#endif
#endif
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
	//log_debug("%p.__constructor__ is %s", ctorVal.to_debug_string().c_str());
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
		// TODO: add ignoreVisibility parameter to allow using __proto__ even when not visible ?
		if (prop && prop->isVisible(swfVersion))
		{
			if (owner != NULL)
				*owner = this;
			return prop;
		}
		return NULL;
	}

	// keep track of visited objects, avoid infinite loops.
	std::set<as_object*> visited;

	int i = 0;

	boost::intrusive_ptr<as_object> obj = this;
	while (obj && visited.insert(obj.get()).second)
	{
		++i;
		if ((i > 255 && swfVersion == 5) || i > 257)
			throw ActionLimitException("Lookup depth exceeded.");

		Property* prop = obj->_members.getProperty(key);
		if (prop && prop->isVisible(swfVersion) )
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
	if ( prop )	return prop;  // TODO: what about isVisible ?

	// don't enter an infinite loop looking for __proto__ ...
	if (key == NSV::PROP_uuPROTOuu) return NULL;

	std::set<as_object*> visited;
	visited.insert(this);

	int i = 0;

	boost::intrusive_ptr<as_object> obj = get_prototype();
	while (obj && visited.insert(obj.get()).second)
	{
		++i;
		if ((i > 255 && swfVersion == 5) || i > 257)
			throw ActionLimitException("Property lookup depth exceeded.");

		Property* p = obj->_members.getProperty(key, nsname);
		if (p && (p->isGetterSetter() | p->isStatic()) && p->isVisible(swfVersion))
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
	static string_table::key key = NSV::PROP_uuPROTOuu;

	// TODO: check what happens if __proto__ is set as a user-defined getter/setter
	// TODO: check triggers !!
	_members.setValue(key, as_value(proto.get()), *this, 0, flags);
}

void
as_object::reserveSlot(string_table::key name, string_table::key nsId,
	unsigned short slotId)
{
	_members.reserveSlot(name, nsId, slotId);
}

// Handles read_only and static properties properly.
void
as_object::set_member_default(string_table::key key, const as_value& val,
	string_table::key nsname)
{
	//log_debug(_("set_member_default(%s)"), key.c_str());
	Property* prop = findUpdatableProperty(key, nsname);
	if (prop)
	{
		if (prop->isReadOnly())
		{
			IF_VERBOSE_ASCODING_ERRORS(log_aserror(_(""
				"Attempt to set read-only property '%s'"),
				_vm.getStringTable().value(key).c_str()););
			return;
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
					_vm.getStringTable().value(key), curVal.to_debug_string(), val.to_debug_string());
				as_value newVal = trig.call(curVal, val, *this);
				// The trigger call could have deleted the property,
				// so we check for its existance again, and do NOT put
				// it back in if it was deleted
				prop = findUpdatableProperty(key, nsname);
				if ( ! prop )
				{
					log_debug("Property %s deleted by trigger on update", _vm.getStringTable().value(key));
					return;
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
			return;
		}
		catch (ActionException& exc)
		{
			log_aserror(_("%s: Exception %s. Will create a new member"),
				_vm.getStringTable().value(key).c_str(), exc.what());
		}

		return;
	}

	// Else, add new property...

	// Property does not exist, so it won't be read-only. Set it.
	if (!_members.setValue(key, const_cast<as_value&>(val), *this, nsname))
	{
		IF_VERBOSE_ASCODING_ERRORS(
			log_aserror(_("Unknown failure in setting property '%s' on "
			"object '%p'"), _vm.getStringTable().value(key).c_str(),
			(void*) this););
		return;
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
			log_debug("Property %s deleted by trigger on create", _vm.getStringTable().value(key));
			return;
		}
		prop->setValue(*this, newVal);
	}

}

std::pair<bool,bool>
as_object::update_member(string_table::key key, const as_value& val,
	string_table::key nsname)
{
	std::pair<bool,bool> ret; // first is found, second is updated

	//log_debug(_("set_member_default(%s)"), key.c_str());
	Property* prop = findUpdatableProperty(key, nsname);
	if (prop)
	{
		if (prop->isReadOnly())
		{
			IF_VERBOSE_ASCODING_ERRORS(log_aserror(_(""
				"Attempt to set read-only property '%s'"),
				_vm.getStringTable().value(key).c_str()););
			return std::make_pair(true, false);
		}

		try
		{
			as_value newVal = val;

			// check if we have a trigger, if so, invoke it
			// and set val to it's return
			TriggerContainer::iterator trigIter = _trigs.find(std::make_pair(key, nsname));
			if ( trigIter != _trigs.end() )
			{
				Trigger& trig = trigIter->second;
				// WARNING: getValue might itself invoke a trigger (getter-setter)... ouch ?
				as_value curVal = prop->getCache(); // Value(*this); 
				log_debug("Property %s is being watched: firing trigger on update (current val:%s, new val:%s",
					_vm.getStringTable().value(key),
					curVal.to_debug_string(), val.to_debug_string());
				newVal = trig.call(curVal, val, *this);
				// The trigger call could have deleted the property,
				// so we check for its existance again, and do NOT put
				// it back in if it was deleted
				prop = findUpdatableProperty(key, nsname);
				if ( ! prop )
				{
					return std::make_pair(true, true);
				}
			}

			prop->setValue(*this, newVal);
			return std::make_pair(true, true);
		}
		catch (ActionException& exc)
		{
			log_debug(_("%s: Exception %s. Will create a new member"),
				_vm.getStringTable().value(key).c_str(), exc.what());
		}

		return std::make_pair(true, false);
	}

	return std::make_pair(false, false);

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
	//log_debug(_("Initializing member %s for object %p"), _vm.getStringTable().value(key).c_str(), (void*) this);

	if (order >= 0 && !_members.
		reserveSlot(static_cast<unsigned short>(order), key, nsname))
	{
		log_error(_("Attempt to set a slot for either a slot or a property "
			"which already exists."));
		return;
	}
		
	// Set (or create) a SimpleProperty 
	if (! _members.setValue(key, const_cast<as_value&>(val), *this, nsname, flags) )
	{
		log_error(_("Attempt to initialize read-only property ``%s''"
			" on object ``%p'' twice"),
			_vm.getStringTable().value(key).c_str(), (void*)this);
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
	bool success;
	success = _members.addGetterSetter(key, getter, &setter, nsname);

	// We shouldn't attempt to initialize a property twice, should we ?
	assert(success);

	//log_debug(_("Initialized property '%s'"), name.c_str());

	// TODO: optimize this, don't scan again !
	_members.setFlags(key, flags, nsname);

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
	bool success;
	success = _members.addGetterSetter(key, getter, setter, nsname);

	// We shouldn't attempt to initialize a property twice, should we ?
	assert(success);

	//log_debug(_("Initialized property '%s'"), name.c_str());

	// TODO: optimize this, don't scan again !
	_members.setFlags(key, flags, nsname);

}

bool
as_object::init_destructive_property(string_table::key key, as_function& getter,
	as_function& setter, int flags, string_table::key nsname)
{
	bool success;

	// No case check, since we've already got the key.
	success = _members.addDestructiveGetterSetter(key, getter, setter, nsname);
	_members.setFlags(key, flags, nsname);
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
as_object::init_readonly_property(const std::string& key, as_c_function_ptr getter,
	int initflags, string_table::key nsname)
{
	string_table::key k = _vm.getStringTable().find(PROPNAME(key));

	init_property(k, getter, getter, initflags | as_prop_flags::readOnly
		| as_prop_flags::isProtected, nsname);
	assert(_members.getProperty(k, nsname));
}

std::string
as_object::asPropName(string_table::key name)
{
	std::string orig = _vm.getStringTable().value(name);

	return PROPNAME(orig); // why is PROPNAME needed here ?
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
as_object::instanceOf(as_function* ctor)
{
	if (this == ctor->getPrototype())
		return true;

	if (!mInterfaces.empty())
	{
		// TODO: Make this work.
		if (std::find(mInterfaces.begin(), mInterfaces.end(), ctor->getPrototype()) != mInterfaces.end())
		{
			return true;
		}
	}

	as_object *proto = this->get_prototype().get();
	if (proto)
		return proto->instanceOf(ctor);
	else
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
	log_debug(_(SIZET_FMT " members of object %p follow"),
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
					"(either not found or protected)"),
					prop.c_str());
				);
			}

			if ( next_comma == std::string::npos )
			{
				break;
			}
		}
		return;
	}

	// Evan: it seems that if set_true == 0 and set_false == 0,
	// this function acts as if the parameters were (object, null, 0x1, 0)
#if 0 // bullshit, see actionscript.all/Global.as
	if (set_false == 0 && set_true == 0)
	{
	    props_val.set_null();
	    set_false = 0;
	    set_true = 0x1;
	}
#endif

	if (props_val.is_null())
	{
		// Take all the members of the object
		//std::pair<size_t, size_t> result = 
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
	as_array_object* ary = dynamic_cast<as_array_object*>(props.get());
	if ( ! ary )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Invalid call to AsSetPropFlags: "
			"invalid second argument %s "
			"(expected string, null or an array)"),
			props_val.to_debug_string().c_str());
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
	assert( env.top(0).is_null() );

	enumerateNonProperties(env);

	// this set will keep track of visited objects,
	// to avoid infinite loops
	std::set< as_object* > visited;
	PropertyList::propNameSet named;

	boost::intrusive_ptr<as_object> obj = const_cast<as_object*>(this);
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
as_object::enumerateProperties(std::map<std::string, std::string>& to)
{

	// this set will keep track of visited objects,
	// to avoid infinite loops
	std::set< as_object* > visited;

	boost::intrusive_ptr<as_object> obj = this;
	while ( obj && visited.insert(obj.get()).second )
	{
		obj->_members.enumerateKeyValue(*this, to);
		obj = obj->get_prototype();
	}

}

as_object::as_object()
	:
	_members(),
	_vm(VM::get())
	//, m_prototype(NULL)
{
}

as_object::as_object(as_object* proto)
	:
	_members(),
	_vm(VM::get())
	//, m_prototype(proto)
{
	init_member(NSV::PROP_uuPROTOuu, as_value(proto));
}

as_object::as_object(boost::intrusive_ptr<as_object> proto)
	:
	_members(),
	_vm(VM::get())
	//, m_prototype(proto)
{
	//set_prototype(proto);
	init_member(NSV::PROP_uuPROTOuu, as_value(proto));
}

as_object::as_object(const as_object& other)
	:
#ifndef GNASH_USE_GC
	ref_counted(),
#else
	GcResource(), 
#endif
	_members(other._members),
	_vm(VM::get())
	//, m_prototype(other.m_prototype) // done by _members copy
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
#if 0
	as_value val;
	if ( ! get_member(NSV::PROP_uuPROTOuu, &val) )
	{
		//log_debug("Object %p has no __proto__ member");
		return NULL;
	}
	//log_debug("%p.__proto__ is %s", val.to_debug_string().c_str());
	return val.to_object().get();
#else
	static string_table::key key = NSV::PROP_uuPROTOuu;

	int swfVersion = _vm.getSWFVersion();

	boost::intrusive_ptr<as_object> nullRet = NULL;

	Property* prop = _members.getProperty(key);
	if ( ! prop ) return nullRet;
	if ( ! prop->isVisible(swfVersion) ) return nullRet;

	as_value tmp = prop->getValue(*this);

	return tmp.to_object();
#endif
}

bool
as_object::on_event(const event_id& id )
{
	as_value event_handler;

	if (get_member(id.get_function_key(), &event_handler) )
	{
		call_method(event_handler, NULL, this, 0, 0);
		return true;
	}

	return false;
}

as_value
as_object::getMember(string_table::key name, string_table::key nsname)
{
	as_value ret;
	get_member(name, &ret, nsname);
	//get_member(PROPNAME(name), &ret);
	return ret;
}

as_value
as_object::callMethod(string_table::key methodName)
{
	as_value ret;
	as_value method;

	if (! get_member(methodName, &method))
	{
		return ret;
	}

	as_environment env;

	return call_method(method, &env, this, 0, env.stack_size());
}

as_value
as_object::callMethod(string_table::key methodName, const as_value& arg0)
{
	as_value ret;
	as_value method;

	if (!get_member(methodName, &method))
	{
		return ret;
	}

	as_environment env;
	env.push(arg0);

	ret = call_method(method, &env, this, 1, env.stack_size()-1);

	env.drop(1);

	return ret;
}

as_value
as_object::callMethod(string_table::key methodName,
	const as_value& arg0, const as_value& arg1)
{
	as_value ret;
	as_value method;

	if (! get_member(methodName, &method))
	{
		return ret;
	}

	as_environment env;

#ifndef NDEBUG
	size_t origStackSize = env.stack_size();
#endif

	env.push(arg1);
	env.push(arg0);

	ret = call_method(method, &env, this, 2, env.stack_size()-1);

	env.drop(2);

#ifndef NDEBUG
	assert(origStackSize == env.stack_size());
#endif

	return ret;
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

	as_environment env;

#ifndef NDEBUG
	size_t origStackSize = env.stack_size();
#endif

	env.push(arg2);
	env.push(arg1);
	env.push(arg0);

	ret = call_method(method, &env, this, 3, env.stack_size()-1);

	env.drop(3);

#ifndef NDEBUG
	assert(origStackSize == env.stack_size());
#endif

	return ret;
}

as_value
as_object::callMethod(string_table::key methodName,
	const as_value& arg0, const as_value& arg1,
	const as_value& arg2, const as_value& arg3)
{
	as_value ret;
	as_value method;

	if (! get_member(methodName, &method))
	{
		return ret;
	}

	as_environment env;

#ifndef NDEBUG
	size_t origStackSize = env.stack_size();
#endif

	env.push(arg3);
	env.push(arg2);
	env.push(arg1);
	env.push(arg0);

	ret = call_method(method, &env, this, 4, env.stack_size()-1);

	env.drop(4);

#ifndef NDEBUG
	assert(origStackSize == env.stack_size());
#endif

	return ret;
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
			_vm.getStringTable().value(key).c_str(),
			(void*)this);
#endif
		return NULL;
	}
	if ( ! tmp.is_object() )
	{
#ifdef DEBUG_TARGET_FINDING 
		log_debug("Member %s of object %p is not an object (%s)",
			_vm.getStringTable().value(key).c_str(), (void*)this,
			tmp.to_debug_string().c_str());
#endif
		return NULL;
	}

	return tmp.to_object().get();
}

void
as_object::getURLEncodedVars(std::string& data)
{
    typedef std::map<std::string, std::string> PropMap;
    PropMap props;
    enumerateProperties(props);

    std::string del;
    data.clear();
    
    for (PropMap::const_iterator i=props.begin(), e=props.end(); i!=e; ++i)
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
		return _trigs.insert(std::make_pair(k, Trigger(propname, trig, cust))).second;
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
		log_debug("No watch for property %s", getVM().getStringTable().value(key));
		return false;
	}
	Property* prop = _members.getProperty(key, ns);
	if ( prop && prop->isGetterSetter() )
	{
		log_debug("Watch on %s not removed (is a getter-setter)", getVM().getStringTable().value(key));
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
		as_environment env;

#ifndef NDEBUG
		size_t origStackSize = env.stack_size();
#endif

		env.push(_customArg);
		env.push(newval);
		env.push(oldval);
		env.push(_propname);
		fn_call fn(const_cast<as_object*>(&this_obj), &env, 4, env.stack_size()-1);
		as_value ret = _func->call(fn);
		env.drop(4);

#ifndef NDEBUG
		assert(origStackSize == env.stack_size());
#endif

		_executing = false;

		return ret;

	}
	catch (...)
	{
		_executing = false;
		throw;
	}
}



} // end of gnash namespace
