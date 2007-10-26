// as_object.cpp:  ActionScript Object class and its properties, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
#include "config.h"
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
		//log_msg(_("Setting member '%s' to value '%s'"), name.c_str(), val.to_debug_string().c_str());
		_tgt.set_member(name, val);
	}
};

} // end of anonymous namespace


namespace gnash {

bool
as_object::add_property(const std::string& key, as_function& getter,
		as_function& setter)
{
	string_table &stringTable = _vm.getStringTable();
	if ( _vm.getSWFVersion() < 7 )
	{
		std::string name = key;
		boost::to_lower(name, _vm.getLocale());
		return _members.addGetterSetter(stringTable.find(name), getter, setter);
	}
	else
	{
		return _members.addGetterSetter(stringTable.find(key), getter, setter);
	}
}

/*protected*/
bool
as_object::get_member_default(string_table::key name, as_value* val,
	string_table::key nsname)
{
	assert(val);

	Property* prop = findProperty(name, nsname);
	if ( ! prop ) return false;

	try 
	{
		*val = prop->getValue(*this);
		return true;
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
	// don't enter an infinite loop looking for __proto__ ...
	if (key == NSV::PROP_uuPROTOuu)
	{
		if (owner != NULL)
			*owner = this;
		return _members.getProperty(key, nsname);
	}

	// keep track of visited objects, avoid infinite loops.
	std::set<as_object*> visited;

	int swfVersion = _vm.getSWFVersion();

	boost::intrusive_ptr<as_object> obj = this;
	while (obj && visited.insert(obj.get()).second)
	{
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
	if ( prop && prop->isVisible(swfVersion) ) return prop;

	// don't enter an infinite loop looking for __proto__ ...
	if (key == NSV::PROP_uuPROTOuu) return NULL;

	// this set will keep track of visited objects,
	// to avoid infinite loops
	std::set< as_object* > visited;
	visited.insert(this);

	boost::intrusive_ptr<as_object> obj = get_prototype();
	while ( obj && visited.insert(obj.get()).second )
	{
		Property* prop = obj->_members.getProperty(key, nsname);
		if ( prop && prop->isGetterSetter() && prop->isVisible(swfVersion) )
		{
			// what if a property is found which is
			// NOT a getter/setter ?
			return prop;
		}
		obj = obj->get_prototype();
	}

	// No Getter/Setter property found in inheritance chain
	return NULL;
}

/*protected*/
void
as_object::set_prototype(boost::intrusive_ptr<as_object> proto, int flags)
{
	static string_table::key key = NSV::PROP_uuPROTOuu;

	// TODO: check what happens if __proto__ is set as a user-defined getter/setter
	if (_members.setValue(key, as_value(proto.get()), *this, 0) )
	{
		// TODO: optimize this, don't scan again !
		_members.setFlags(key, flags, 0);
	}
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
	//log_msg(_("set_member_default(%s)"), key.c_str());
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

		// TODO: add isStatic() check in findUpdatableProperty ?
		//if (prop->isGetterSetter() || prop->isStatic()) {
			try
			{
				prop->setValue(*this, val);
				return;
			}
			catch (ActionException& exc)
			{
				log_msg(_("%s: Exception %s. Will create a new member"),
					_vm.getStringTable().value(key).c_str(), exc.what());
			}
		//}

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
	}
}

std::pair<bool,bool>
as_object::update_member(string_table::key key, const as_value& val,
	string_table::key nsname)
{
	std::pair<bool,bool> ret; // first is found, second is updated

	//log_msg(_("set_member_default(%s)"), key.c_str());
	Property* prop = findUpdatableProperty(key, nsname);
	if (prop)
	{
		if (prop->isReadOnly())
		{
			IF_VERBOSE_ASCODING_ERRORS(log_aserror(_(""
				"Attempt to set read-only property '%s'"),
				_vm.getStringTable().value(key).c_str()););
			return make_pair(true, false);
		}

		try
		{
			prop->setValue(*this, val);
			return make_pair(true, true);
		}
		catch (ActionException& exc)
		{
			log_msg(_("%s: Exception %s. Will create a new member"),
				_vm.getStringTable().value(key).c_str(), exc.what());
		}

		return make_pair(true, false);
	}

	return make_pair(false, false);

}

void
as_object::init_member(const std::string& key1, const as_value& val, int flags,
	string_table::key nsname)
{
	if ( _vm.getSWFVersion() < 7 )
	{
		std::string keylower = key1;
		boost::to_lower(keylower, _vm.getLocale());

		init_member(_vm.getStringTable().find(keylower), val, flags, nsname);

	}
	else
	{
		init_member(_vm.getStringTable().find(key1), val, flags, nsname);
	}
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
	if (! _members.setValue(key, const_cast<as_value&>(val), *this, nsname) )
	{
		log_error(_("Attempt to initialize read-only property ``%s''"
			" on object ``%p'' twice"),
			_vm.getStringTable().value(key).c_str(), (void*)this);
		// We shouldn't attempt to initialize a member twice, should we ?
		assert(0);
	}
	// TODO: optimize this, don't scan again !
	_members.setFlags(key, flags, nsname);
}

void
as_object::init_property(const std::string& key, as_function& getter,
		as_function& setter, int flags, string_table::key nsname)
{
	if ( _vm.getSWFVersion() < 7 )
	{
		std::string name = key;
		boost::to_lower(name, _vm.getLocale());
		string_table::key k = _vm.getStringTable().find(name);
		init_property(k, getter, setter, flags, nsname);
	}
	else
	{
		string_table::key k = _vm.getStringTable().find(key);
		init_property(k, getter, setter, flags, nsname);
	}

}

void
as_object::init_property(string_table::key key, as_function& getter,
		as_function& setter, int flags, string_table::key nsname)
{
	bool success;
	success = _members.addGetterSetter(key, getter, setter, nsname);

	// We shouldn't attempt to initialize a property twice, should we ?
	assert(success);

	//log_msg(_("Initialized property '%s'"), name.c_str());

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
	string_table::key k;
	if ( _vm.getSWFVersion() < 7 )
	{
		std::string name = key;
		boost::to_lower(name, _vm.getLocale());
		k = _vm.getStringTable().find(name);
	}
	else
	{
		k = _vm.getStringTable().find(key);
	}
	init_property(k, getter, getter, initflags | as_prop_flags::readOnly
		| as_prop_flags::isProtected, nsname);
	assert(_members.getProperty(k, nsname));
}

std::string
as_object::asPropName(string_table::key name)
{
	std::string orig = _vm.getStringTable().value(name);
	if ( _vm.getSWFVersion() < 7 )
	{
		boost::to_lower(orig, _vm.getLocale());
	}

	return orig;
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
	if (std::find(mInterfaces.begin(), mInterfaces.end(), obj) == mInterfaces.end())
		mInterfaces.push_back(obj);
	else
		fprintf(stderr, "Not adding duplicate interface.\n");
}

bool
as_object::instanceOf(as_function* ctor)
{
	boost::intrusive_ptr<as_object> obj = this;

	std::set< as_object* > visited;

	if (this == ctor)
	{ assert(0); }
	while (obj && visited.insert(obj.get()).second )
	{
		if (!mInterfaces.empty() &&
			std::find(mInterfaces.begin(), mInterfaces.end(), obj) != mInterfaces.end())
			return true;
		if ( obj->get_prototype() == ctor->getPrototype() ) return true;
		obj = obj->get_prototype(); 
	}

	// See actionscript.all/Inheritance.as for a way to trigger this
	IF_VERBOSE_ASCODING_ERRORS(
	if ( obj ) log_aserror(_("Circular inheritance chain detected during instanceOf call"));
	);

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
	log_msg(_(SIZET_FMT " members of object %p follow"),
		_members.size(), (const void*)this);
	_members.dump(*this);
}

void
as_object::dump_members(std::map<std::string, as_value>& to)
{
	_members.dump(*this, to);
}

void
as_object::setPropFlags(as_value& props_val, int set_false, int set_true)
{
	if (props_val.is_string())
	{
		std::string propstr = props_val.to_string(); // no need for calling toString here..
		if ( _vm.getSWFVersion() < 7 ) // convert to lower case if required
		{
			boost::to_lower(propstr);
		}

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
				propstr=propstr.substr(next_comma);
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

	boost::intrusive_ptr<as_object> props = props_val.to_object();

	// Evan: it seems that if set_true == 0 and set_false == 0,
	// this function acts as if the parameters were (object, null, 0x1, 0)
	if (set_false == 0 && set_true == 0)
	{
	    props = NULL;
	    set_false = 0;
	    set_true = 0x1;
	}

	if (props == NULL)
	{
		// TODO: this might be a comma-separated list
		//       of props as a string !!
		//

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
	}
	else
	{
		//std::pair<size_t, size_t> result = 
		_members.setFlagsAll(props->_members, set_true, set_false);
	}
}


void
as_object::copyProperties(const as_object& o)
{
	PropsCopier copier(*this);

	// TODO: check if non-visible properties should be also copied !
	o._members.visitValues(copier,
			// Need const_cast due to getValue getting non-const ...
			const_cast<as_object&>(o));
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
	init_member("__proto__", as_value(proto));
}

as_object::as_object(boost::intrusive_ptr<as_object> proto)
	:
	_members(),
	_vm(VM::get())
	//, m_prototype(proto)
{
	//set_prototype(proto);
	init_member("__proto__", as_value(proto));
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
	if ( _vm.getSWFVersion() < 7 )
	{
       	std::string key = _vm.getStringTable().value(name);
		boost::to_lower(key, _vm.getLocale());
		return _members.delProperty(_vm.getStringTable().find(key), nsname);
	}
	else
	{
		return _members.delProperty(name, nsname);
	}
}

Property*
as_object::getOwnProperty(string_table::key name, string_table::key nsname)
{
	if ( _vm.getSWFVersion() < 7 )
	{
       	std::string key = _vm.getStringTable().value(name);
		boost::to_lower(key, _vm.getLocale());
		return _members.getProperty(_vm.getStringTable().find(key), nsname);
	}
	else
	{
		return _members.getProperty(name, nsname);
	}
}

as_value
as_object::tostring_method(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = fn.this_ptr;

	std::string text_val = obj->get_text_value();
	if ( ! text_val.empty() ) // TODO: check if still possible
	{
		return as_value(text_val);
	}
	else
	{
		return as_value("[object Object]");
	}
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
	static string_table::key key = NSV::PROP_uuPROTOuu;

	int swfVersion = _vm.getSWFVersion();

	boost::intrusive_ptr<as_object> nullRet;

	Property* prop = _members.getProperty(key);
	if ( ! prop ) return nullRet;
	if ( ! prop->isVisible(swfVersion) ) return nullRet;

	as_value tmp = prop->getValue(*this);

	return tmp.to_object();
}

#ifdef NEW_KEY_LISTENER_LIST_DESIGN
bool
as_object::on_event(const event_id& id )
{
	as_value event_handler;

	std::string handler_name = id.get_function_name();

	if ( _vm.getSWFVersion() < 7 )
	{
		boost::to_lower(handler_name, _vm.getLocale());
	}

	if (get_member(_vm.getStringTable().find(handler_name), &event_handler) )
	{
		call_method(event_handler, NULL, this, 0, 0);
		return true;
	}

	return false;
}
#endif 

as_value
as_object::getMember(string_table::key name, string_table::key nsname)
{
	as_value ret;
	get_member(name, &ret, nsname);
	//get_member(PROPNAME(name), &ret);
	return ret;
}

as_value
as_object::callMethod(string_table::key methodName, as_environment& env)
{
	as_value ret;
	as_value method;

	if (! get_member(methodName, &method))
	{
		return ret;
	}

	return call_method(method, &env, this, 0, env.stack_size());
}

as_value
as_object::callMethod(string_table::key methodName, as_environment& env, const as_value& arg0)
{
	as_value ret;
	as_value method;

	if (!get_member(methodName, &method))
	{
		return ret;
	}

	env.push(arg0);

	ret = call_method(method, &env, this, 1, env.stack_size()-1);

	env.drop(1);

	return ret;
}

as_value
as_object::callMethod(string_table::key methodName, as_environment& env,
	const as_value& arg0, const as_value& arg1)
{
	as_value ret;
	as_value method;

	if (! get_member(methodName, &method))
	{
		return ret;
	}

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

} // end of gnash namespace
