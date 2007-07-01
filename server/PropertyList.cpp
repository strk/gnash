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

#include <utility> // for std::make_pair

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
PropertyList::getValue(const std::string& key, as_value& val,
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
PropertyList::setValue(const std::string& key, const as_value& val,
		as_object& this_ptr)
{
	iterator found = _props.find( key );
	if ( found == _props.end() )
	{
		// create a new member
		_props[key] = new SimpleProperty(val);
		return true;
	}

	Property* prop = found->second;

	if ( prop->isReadOnly() )
	{
		log_error(_("Property %s is read-only, not setting it to %s"), key.c_str(), val.to_string().c_str());
		return false;
	}

	//log_msg(_("Property %s set to value %s"), key.c_str(), val.to_string());
	prop->setValue(this_ptr, val);
	return true;
}

bool
PropertyList::setFlags(const std::string& key,
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
PropertyList::getProperty(const std::string& key)
{
	iterator it=find(key);
	if ( it == end() ) return NULL;
	return it->second;
}

std::pair<bool,bool>
PropertyList::delProperty(const std::string& key)
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
		const std::string& name = it->first;

		if ( setFlags(name, flagsSet, flagsClear) ) ++success;
		else ++failure;
	}

	return std::make_pair(success,failure);

}

void
PropertyList::enumerateKeys(as_environment& env) const
{
	for ( const_iterator i=begin(), ie=end(); i != ie; ++i)
	{
		const Property* prop = i->second;

		if ( prop->getFlags().get_dont_enum() ) continue;

		env.push(as_value(i->first.c_str()));
	}
}

void
PropertyList::enumerateKeyValue(as_object& this_ptr, std::map<std::string, std::string>& to) 
{
	for ( const_iterator i=begin(), ie=end(); i != ie; ++i)
	{
		const Property* prop = i->second;

		if ( prop->getFlags().get_dont_enum() ) continue;

		to.insert(make_pair(i->first,
				prop->getValue(this_ptr).to_string()));
	}
}

void
PropertyList::dump(as_object& this_ptr)
{
	for ( const_iterator it=begin(), itEnd=end(); it != itEnd; ++it )
	{
		log_msg("  %s: %s", it->first.c_str(),
			it->second->getValue(this_ptr).to_string().c_str());
	}
}

void
PropertyList::import(const PropertyList& o) 
{
	for (const_iterator it = o.begin(), itEnd = o.end(); it != itEnd; ++it)
	{
		const std::string& name = it->first;
		const Property* prop = it->second;

		// Delete any previous property with this name
		iterator found = _props.find( name );
		if ( found != _props.end() )
		{
			delete found->second;
			found->second = prop->clone();
		}
		else
		{
			_props[name] = prop->clone();
		}
	}
}

bool
PropertyList::addGetterSetter(const std::string& key, as_function& getter,
	as_function& setter)
{
	iterator found = _props.find( key );
	if ( found != _props.end() ) return false; // already exists !!

	_props[key] = new GetterSetterProperty(GetterSetter(getter, setter));
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

