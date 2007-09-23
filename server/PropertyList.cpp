// PropertyList.cpp:  ActionScript property lists, for Gnash.
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
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "PropertyList.h"
#include "Property.h" 


#include "log.h"

#include "as_function.h"
#include "as_environment.h" // for enumerateKeys
#include "as_value.h" // for enumerateValues
#include "VM.h" // For string_table

#include <utility> // for std::make_pair

// Define the following to enable printing address of each property added
//#define DEBUG_PROPERTY_ALLOC

namespace gnash {

PropertyList::PropertyList()
{
}

PropertyList::PropertyList(const PropertyList& pl)
{
	import(pl);
}

PropertyList&
PropertyList::operator=(const PropertyList& pl)
{
	if ( this != &pl )
	{
		clear();
		import(pl);
	}
	return *this;
}



bool
PropertyList::getValue(const string_table::key key, as_value& val,
		as_object& this_ptr) 
{
	const_iterator found = _props.find( key );
	if ( found == _props.end() )
	{
		return false;
	}

	val=found->second->getValue(this_ptr);

	//log_msg(_("Property %s found, value is (%s)"), key.c_str(), val.to_string());

	return true;
}

bool
PropertyList::setValue(string_table::key key, const as_value& val,
		as_object& this_ptr)
{
	iterator found = _props.find( key );
	if ( found == _props.end() )
	{
		// create a new member
		SimpleProperty* prop = new SimpleProperty(val);
#ifdef DEBUG_PROPERTY_ALLOC
		log_debug("SimpleProperty %s = %p", VM::get().getStringTable().value(key).c_str(), (void*)prop);
#endif // DEBUG_PROPERTY_ALLOC
		_props[key] = prop;
		return true;
	}

	Property* prop = found->second;

	if ( prop->isReadOnly() )
	{
		log_error(_("Property %s is read-only, not setting it to %s"), 
			VM::get().getStringTable().value(key).c_str(), val.to_string().c_str());
		return false;
	}

	//log_msg(_("Property %s set to value %s"), key.c_str(), val.to_string());
	prop->setValue(this_ptr, val);
	return true;
}

bool
PropertyList::setFlags(string_table::key key,
		int setFlags, int clearFlags)
{
	iterator found = _props.find( key );
	if ( found == _props.end() ) return false;

	Property* prop = found->second;

	as_prop_flags& f = prop->getFlags();
	return f.set_flags(setFlags, clearFlags);
}

std::pair<size_t,size_t>
PropertyList::setFlagsAll(int setFlags, int clearFlags)
{
	size_t success=0;
	size_t failure=0;

	for ( iterator it=_props.begin(), far=_props.end(); it != far; ++it)
	{
		Property* prop = it->second;
		as_prop_flags& f = prop->getFlags();
		if ( f.set_flags(setFlags, clearFlags) ) ++success;
		else ++failure;
	}

	return std::make_pair(success,failure);
}

Property*
PropertyList::getProperty(string_table::key key)
{
	iterator it=find(key);
	if ( it == end() ) return NULL;
	return it->second;
}

std::pair<bool,bool>
PropertyList::delProperty(string_table::key key)
{
	//GNASH_REPORT_FUNCTION;
	iterator it=find(key);
	if ( it == end() ){
		return std::make_pair(false,false);
	}

	// check if member is protected from deletion
	if ( it->second->getFlags().get_dont_delete() )
	{
		return std::make_pair(true,false);
	}

	delete it->second;
	_props.erase(it);
	return std::make_pair(true,true);
}

std::pair<size_t,size_t>
PropertyList::setFlagsAll(const PropertyList& props,
		int flagsSet, int flagsClear)
{
	size_t success=0;
	size_t failure=0;

	for (const_iterator it = props.begin(), itEnd = props.end(); it != itEnd; ++it )
	{
		string_table::key key = it->first;

		if ( setFlags(key, flagsSet, flagsClear) ) ++success;
		else ++failure;
	}

	return std::make_pair(success,failure);

}

void
PropertyList::enumerateKeys(as_environment& env) const
{
	string_table& st = VM::get().getStringTable();
	for ( const_iterator i=begin(), ie=end(); i != ie; ++i)
	{
		const Property* prop = i->second;

		if ( prop->getFlags().get_dont_enum() ) continue;

		env.push(as_value(st.value(i->first).c_str()));
	}
}

void
PropertyList::enumerateKeyValue(as_object& this_ptr, std::map<std::string, std::string>& to) 
{
	string_table& st = VM::get().getStringTable();
	for ( const_iterator i=begin(), ie=end(); i != ie; ++i)
	{
		const Property* prop = i->second;

		if ( prop->getFlags().get_dont_enum() ) continue;

		to.insert(make_pair(st.value(i->first),
				prop->getValue(this_ptr).to_string()));
	}
}

void
PropertyList::dump(as_object& this_ptr)
{
	string_table& st = VM::get().getStringTable();
	for ( const_iterator it=begin(), itEnd=end(); it != itEnd; ++it )
	{
		log_msg("  %s: %s", st.value(it->first).c_str(),
			it->second->getValue(this_ptr).to_string().c_str());
	}
}

void
PropertyList::import(const PropertyList& o) 
{
	for (const_iterator it = o.begin(), itEnd = o.end(); it != itEnd; ++it)
	{
		string_table::key key = it->first;
		const Property* prop = it->second;

		// Delete any previous property with this name
		iterator found = _props.find(key);
		if ( found != _props.end() )
		{
			delete found->second;
			found->second = prop->clone();
		}
		else
		{
			_props[key] = prop->clone();
		}
	}
}

bool
PropertyList::addGetterSetter(string_table::key key, as_function& getter,
	as_function& setter)
{
	iterator found = _props.find( key );
	if ( found != _props.end() ) return false; // already exists !!

	GetterSetterProperty* prop = new GetterSetterProperty(GetterSetter(getter, setter));
#ifdef DEBUG_PROPERTY_ALLOC
	log_debug("GetterSetterProperty %s = %p", V::get().getStringTable().value(key).c_str(), (void*)prop);
#endif // DEBUG_PROPERTY_ALLOC
	_props[key] = prop;
	return true;
}

bool
PropertyList::addDestructiveGetterSetter(string_table::key key,
	as_function& getter, as_function& setter)
{
	iterator found = _props.find(key);
	if (found != _props.end())
		return false; // Already exists.

	DestructiveGetterSetterProperty* prop =
		new DestructiveGetterSetterProperty(GetterSetter(getter, setter));
	_props[key] = prop;
	return true;
}

void
PropertyList::clear()
{
	for (iterator it = begin(), itEnd = end(); it != itEnd; ++it)
		delete it->second;
	_props.clear();
}

PropertyList::~PropertyList()
{
	for (iterator it = begin(), itEnd = end(); it != itEnd; ++it)
		delete it->second;
}

void
PropertyList::setReachable() const
{
	for (const_iterator it = begin(), itEnd = end(); it != itEnd; ++it)
	{
		it->second->setReachable();
	}
}

} // end of gnash namespace

