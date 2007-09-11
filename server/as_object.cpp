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
	void operator() (const std::string& name, const as_value& val)
	{
		//log_msg(_("Setting member '%s' to value '%s'"), name.c_str(), val.to_string());
		_tgt.set_member(name, val);
	}
};

} // end of anonymous namespace


namespace gnash {

bool
as_object::add_property(const std::string& key, as_function& getter,
		as_function& setter)
{
	if ( _vm.getSWFVersion() < 7 )
	{
		std::string name = key;
		boost::to_lower(name, _vm.getLocale());
		return _members.addGetterSetter(name, getter, setter);
	}
	else
	{
		return _members.addGetterSetter(key, getter, setter);
	}
}

/*protected*/
bool
as_object::get_member_default(const std::string& name, as_value* val)
{
	assert(val);

	Property* prop = findProperty(name);
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

/*private*/
Property*
as_object::findProperty(const std::string& key)
{
	// don't enter an infinite loop looking for __proto__ ...
	if ( key == "__proto__" )
	{
		return _members.getProperty(key);
	}

	// this set will keep track of visited objects,
	// to avoid infinite loops
	std::set< as_object* > visited;

	boost::intrusive_ptr<as_object> obj = this;
	while ( obj && visited.insert(obj.get()).second )
	{
		Property* prop = obj->_members.getProperty(key);
		if ( prop ) return prop;
		else obj = obj->get_prototype();
	}

	// No Property found
	return NULL;

}

/*private*/
Property*
as_object::findGetterSetter(const std::string& key)
{
	// don't enter an infinite loop looking for __proto__ ...
	if ( key == "__proto__" )
	{
		Property* prop = _members.getProperty(key);
		if ( ! prop ) return NULL;
		if ( ! prop->isGetterSetter() ) return NULL;
		return prop;
	}

	// this set will keep track of visited objects,
	// to avoid infinite loops
	std::set< as_object* > visited;

	boost::intrusive_ptr<as_object> obj = this;
	while ( obj && visited.insert(obj.get()).second )
	{
		Property* prop = obj->_members.getProperty(key);
		if ( prop && prop->isGetterSetter() )
		{
			// what if a property is found which is
			// NOT a getter/setter ?
			return prop;
		}
		obj = obj->get_prototype();
	}

	// No Getter/Setter property found
	return NULL;

}

/*protected*/
void
as_object::set_prototype(boost::intrusive_ptr<as_object> proto, int flags)
{
	static std::string key ( "__proto__" );

	// TODO: check what happens if __proto__ is set as a user-defined getter/setter
	if ( _members.setValue(key, as_value(proto.get()), *this) )
	{
		// TODO: optimize this, don't scan again !
		_members.setFlags(key, flags, 0);
	}
}

void
as_object::set_member_default(const std::string& key, const as_value& val )
{
	//log_msg(_("set_member_default(%s)"), key.c_str());

	// found a getter/setter property in the inheritance chain
	// so set that and return
	Property* prop = findGetterSetter(key);
	if ( prop )
	{
		try 
		{
			//log_msg(_("Found a getter/setter property for key %s"), key.c_str());
			if (prop->isReadOnly())
			{
				IF_VERBOSE_ASCODING_ERRORS(
                			log_aserror(_("Attempt to set read-only property '%s'"),
						    key.c_str());
	                	);
			} else
			{
				prop->setValue(*this, val);
			}
			return;
		}
		catch (ActionException& exc)
		{
			log_msg(_("%s: Exception %s.  Will create a new member"), key.c_str(), exc.what());
		}
	}

	//log_msg(_("Found no getter/setter property for key %s"), key.c_str());

	// No getter/setter property found, so set (or create) a
	// SimpleProperty (if possible)
	if ( ! _members.setValue(key, val, *this) )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Attempt to set read-only property ``%s''"
			" on object ``%p''"),
			key.c_str(), (void*)this);
		);
	}

}

void
as_object::init_member(const std::string& key, const as_value& val, int flags)
{

	//log_msg(_("Setting member %s (SWF version:%d)"), key.c_str(), vm.getSWFVersion());

	VM& vm = _vm;

	if ( vm.getSWFVersion() < 7 )
	{
		std::string keylower = key;
		boost::to_lower(keylower, vm.getLocale());

		// Set (or create) a SimpleProperty 
		if ( ! _members.setValue(keylower, val, *this) )
		{
			log_error(_("Attempt to initialize read-only property ``%s''"
				" (%s) on object ``%p'' twice"),
				keylower.c_str(), key.c_str(), (void*)this);
			// We shouldn't attempt to initialize a member twice, should we ?
			assert(0);
		}
		// TODO: optimize this, don't scan again !
		_members.setFlags(keylower, flags, 0);
	}
	else
	{
		// Set (or create) a SimpleProperty 
		if ( ! _members.setValue(key, val, *this) )
		{
			log_error(_("Attempt to initialize read-only property ``%s''"
				" on object ``%p'' twice"),
				key.c_str(), (void*)this);
			// We shouldn't attempt to initialize a member twice, should we ?
			assert(0);
		}
		// TODO: optimize this, don't scan again !
		_members.setFlags(key, flags, 0);
	}




}

void
as_object::init_property(const std::string& key, as_function& getter,
		as_function& setter, int flags)
{
	bool success;
	if ( _vm.getSWFVersion() < 7 )
	{
		std::string name = key;
		boost::to_lower(name, _vm.getLocale());
		success = _members.addGetterSetter(name, getter, setter);
		//log_msg(_("Initialized property '%s'"), name.c_str());
		// TODO: optimize this, don't scan again !
		_members.setFlags(name, flags, 0);
	}
	else
	{
		success = _members.addGetterSetter(key, getter, setter);
		//log_msg(_("Initialized property '%s'"), key.c_str());
		// TODO: optimize this, don't scan again !
		_members.setFlags(key, flags, 0);
	}

	// We shouldn't attempt to initialize a property twice, should we ?
	assert(success);
}

void
as_object::init_readonly_property(const std::string& key, as_function& getter, int initflags)
{
	init_property(key, getter, getter, initflags);

	as_prop_flags& flags = getOwnProperty(key)->getFlags();

	// ActionScript must not change the flags of this builtin property.
	flags.set_is_protected(true);

	// Make the property read-only; that is, the default no-op handler will
	// be triggered when ActionScript tries to set it.
	flags.set_read_only();
}

std::string
as_object::asPropName(std::string name)
{
	std::string orig = name;
	if ( _vm.getSWFVersion() < 7 )
	{
		boost::to_lower(orig, _vm.getLocale());
	}

	return orig;
}


bool
as_object::set_member_flags(const std::string& name,
		int setTrue, int setFalse)
{
	// TODO: accept a std::string directly
	return _members.setFlags(name, setTrue, setFalse);
}

bool
as_object::instanceOf(as_function* ctor)
{
	boost::intrusive_ptr<as_object> obj = this;

	std::set< as_object* > visited;

	while (obj && visited.insert(obj.get()).second )
	{
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
as_object::setPropFlags(as_value& props_val, int set_false, int set_true)
{
	if ( props_val.is_string() )
	{
		std::string propstr = props_val.to_string();
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
			if ( ! set_member_flags(prop.c_str(), set_true, set_false) )
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

	boost::intrusive_ptr<as_object> obj = const_cast<as_object*>(this);
	while ( obj && visited.insert(obj.get()).second )
	{
		obj->_members.enumerateKeys(env);
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
as_object::delProperty(const std::string& name)
{
	if ( _vm.getSWFVersion() < 7 )
	{
        	std::string key = name;
		boost::to_lower(key, _vm.getLocale());
		return _members.delProperty(key);
	}
	else
	{
		return _members.delProperty(name);
	}
}

Property*
as_object::getOwnProperty(const std::string& name)
{
	if ( _vm.getSWFVersion() < 7 )
	{
        	std::string key = name;
		boost::to_lower(key, _vm.getLocale());
		return _members.getProperty(key);
	}
	else
	{
		return _members.getProperty(name);
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
	static std::string key ( "__proto__" );
	as_value tmp;
	// I don't think any subclass should override getting __proto__ anyway...
	//if ( ! get_member(key, &tmp) ) return NULL;
	if ( ! _members.getValue(key, tmp, *this) ) return NULL;
	return tmp.to_object();

#if 0 // the inheritance chain MUST end somewhere, handle the SWF4 thing in some other way
	if ( m_prototype ) return m_prototype.get();
	//log_msg(_("as_object::get_prototype(): Hit top of inheritance chain"));

	// if SWF version < 5 the Object interface won't keep alive !
	if ( _vm.getSWFVersion() > 4 )
	{
		return getObjectInterface();
	}

	return NULL;
#endif
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

	if (get_member(handler_name, &event_handler) )
	{
		call_method(event_handler, NULL, this, 0, 0);
		return true;
	}

	return false;
}
#endif 

as_value
as_object::getMember(const std::string& name)
{
	as_value ret;
	get_member(PROPNAME(name), &ret);
	return ret;
}

as_value
as_object::callMethod(const std::string& methodName, as_environment& env)
{
	as_value ret;
	as_value method;

	if ( ! get_member(methodName, &method) )
	{
		return ret;
	}

	return call_method(method, &env, this, 0, env.stack_size());
}

as_value
as_object::callMethod(const std::string& methodName, as_environment& env, const as_value& arg0)
{
	as_value ret;
	as_value method;

	if ( ! get_member(methodName, &method) )
	{
		return ret;
	}

	env.push(arg0);

	ret = call_method(method, &env, this, 1, env.stack_size()-1);

	env.drop(1);

	return ret;
}

as_value
as_object::callMethod(const std::string& methodName, as_environment& env, const as_value& arg0, const as_value& arg1)
{
	as_value ret;
	as_value method;

	if ( ! get_member(methodName, &method) )
	{
		return ret;
	}

	env.push(arg0);
	env.push(arg1);

	ret = call_method(method, &env, this, 2, env.stack_size()-2);

	env.drop(2);

	return ret;
}

} // end of gnash namespace
