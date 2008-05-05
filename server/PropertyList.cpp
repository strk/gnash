// PropertyList.cpp:  ActionScript property lists, for Gnash.
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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "PropertyList.h"
#include "Property.h" 

#include "log.h"

#include "as_function.h"
#include "as_value.h" // for enumerateValues
#include "VM.h" // For string_table

#include <utility> // for std::make_pair

// Define the following to enable printing address of each property added
//#define DEBUG_PROPERTY_ALLOC

namespace gnash {

PropertyList::PropertyList(const PropertyList& pl)
	:
	mDefaultOrder(pl.mDefaultOrder) // correct ?
{
	import(pl);
}

PropertyList&
PropertyList::operator=(const PropertyList& pl)
{
	if ( this != &pl )
	{
		clear();
		mDefaultOrder = pl.mDefaultOrder;
		import(pl);
	}
	return *this;
}

// Should find in any namespace if nsId is 0, and any namespace should find
// something in namespace 0.
static inline
PropertyList::container::iterator
iterator_find(PropertyList::container &p, string_table::key name,
	string_table::key nsId)
{
	if (nsId)
	{
		PropertyList::container::iterator i =
			p.find(boost::make_tuple(name, nsId));
		if (i != p.end())
			return i;
		return p.find(boost::make_tuple(name, 0));
	}

	return p.find(boost::make_tuple(name));
}

typedef PropertyList::container::index<PropertyList::oType>::type::iterator
	orderIterator;

static inline
orderIterator
iterator_find(PropertyList::container &p, int order)
{
	return p.get<1>().find(order);
}

bool
PropertyList::getValueByOrder(int order, as_value& val,
	as_object& this_ptr)
{
	orderIterator i = iterator_find(_props, order);
	if (i == _props.get<1>().end())
		return false;

	val = i->getValue(this_ptr);
	return true;
}

const Property*
PropertyList::getPropertyByOrder(int order)
{
	orderIterator i = iterator_find(_props, order);
	if (i == _props.get<1>().end())
		return NULL;

	return &(*i);
}

const Property*
PropertyList::getOrderAfter(int order)
{
	orderIterator i = iterator_find(_props, order);

	if (i == _props.get<1>().end())
		return NULL; // Not found at all.

	do
	{
		++i;
		if (i == _props.get<1>().end())
			return NULL;
	} while (i->getFlags().get_dont_enum());

	return &(*i);
}

bool
PropertyList::reserveSlot(unsigned short slotId, string_table::key name,
	string_table::key nsId)
{
	orderIterator found = iterator_find(_props, slotId + 1);
	if (found != _props.get<1>().end())
		return false;

	Property a(name, nsId, as_value());
	a.setOrder(slotId + 1);
	_props.insert(a);
	return true;
}

bool
PropertyList::getValue(const string_table::key key, as_value& val,
		as_object& this_ptr, const string_table::key nsId) 
{
	container::iterator found = iterator_find(_props, key, nsId);
	if (found == _props.end())
		return false;

	val = found->getValue(this_ptr);
	return true;
}

bool
PropertyList::setValue(string_table::key key, as_value val,
		as_object& this_ptr, string_table::key nsId,
		const as_prop_flags& flagsIfMissing)
{
	container::iterator found = iterator_find(_props, key, nsId);
	
	if (found == _props.end())
	{
		// create a new member
		Property a(key, nsId, val, flagsIfMissing);
		// Non slot properties are negative ordering in insertion order
		a.setOrder(- ++mDefaultOrder - 1);
		_props.insert(a);
		return true;
	}
	if (found->isReadOnly())
	{
		log_error(_("Property %s is read-only, not setting it to %s"), 
			VM::get().getStringTable().value(key).c_str(), val.to_string().c_str());
		return false;
	}

	const_cast<Property*>(&(*found))->setValue(this_ptr, val);
	return true;
}

bool
PropertyList::setFlags(string_table::key key,
		int setFlags, int clearFlags, string_table::key nsId)
{
	container::iterator found = iterator_find(_props, key, nsId);
	if ( found == _props.end() ) return false;

	as_prop_flags& f = const_cast<as_prop_flags&>(found->getFlags());
	return f.set_flags(setFlags, clearFlags);
}

std::pair<size_t,size_t>
PropertyList::setFlagsAll(int setFlags, int clearFlags)
{
	size_t success=0;
	size_t failure=0;

	for (container::iterator it=_props.begin(), far=_props.end(); it != far; ++it)
	{
		as_prop_flags& f = const_cast<as_prop_flags&>(it->getFlags());
		if (f.set_flags(setFlags, clearFlags))
			++success;
		else
			++failure;
	}

	return std::make_pair(success,failure);
}

Property*
PropertyList::getProperty(string_table::key key, string_table::key nsId)
{
	container::iterator found = iterator_find(_props, key, nsId);
	if (found == _props.end())
	{
		//log_error("getProperty(%s): not found", VM::get().getStringTable().value(key).c_str());
		return NULL;
	}
	return const_cast<Property*>(&(*found));
}

std::pair<bool,bool>
PropertyList::delProperty(string_table::key key, string_table::key nsId)
{
	//GNASH_REPORT_FUNCTION;
	container::iterator found = iterator_find(_props, key, nsId);
	if (found == _props.end())
	{
		return std::make_pair(false,false);
	}

	// check if member is protected from deletion
	if (found->getFlags().get_dont_delete())
	{
		return std::make_pair(true,false);
	}

	_props.erase(found);
	return std::make_pair(true,true);
}

std::pair<size_t,size_t>
PropertyList::setFlagsAll(const PropertyList& props,
		int flagsSet, int flagsClear)
{
	size_t success=0;
	size_t failure=0;

	for (container::const_iterator it = props._props.begin(),
		itEnd = props._props.end(); it != itEnd; ++it )
	{
		string_table::key key = it->mName;

		if (setFlags(key, flagsSet, flagsClear, it->mNamespace)) ++success;
		else ++failure;
	}

	return std::make_pair(success,failure);

}

void
PropertyList::enumerateKeys(as_environment& env, propNameSet& donelist) const
{
	string_table& st = VM::get().getStringTable();
	for (container::const_iterator i=_props.begin(), ie=_props.end(); i != ie; ++i)
	{
		if (i->getFlags().get_dont_enum())
			continue;

		if (donelist.insert(std::make_pair(i->mName, i->mNamespace)).second)
		{
			if (i->mNamespace)
				env.push(as_value(st.value(i->mName) + "." + st.value(i->mNamespace)));
			else
				env.push(as_value(st.value(i->mName)));
		}
	}
}

void
PropertyList::enumerateKeyValue(as_object& this_ptr, std::map<std::string, std::string>& to) 
{
	string_table& st = VM::get().getStringTable();
	for (container::const_iterator i=_props.begin(), ie=_props.end(); i != ie; ++i)
	{
		if (i->getFlags().get_dont_enum())
			continue;

		to.insert(make_pair(st.value(i->mName),
				i->getValue(this_ptr).to_string()));
	}
}

void
PropertyList::dump(as_object& this_ptr, std::map<std::string, as_value>& to) 
{
	string_table& st = VM::get().getStringTable();
	for (container::const_iterator i=_props.begin(), ie=_props.end(); i != ie; ++i)
	{
		to.insert(make_pair(st.value(i->mName), i->getValue(this_ptr)));
	}
}

void
PropertyList::dump(as_object& this_ptr)
{
	string_table& st = VM::get().getStringTable();
	for (container::const_iterator it=_props.begin(), itEnd=_props.end(); it != itEnd; ++it )
	{
		log_debug("  %s: %s", st.value(it->mName).c_str(),
			it->getValue(this_ptr).to_string().c_str());
	}
}

void
PropertyList::import(const PropertyList& o) 
{
	for (container::const_iterator it = o._props.begin(),
		itEnd = o._props.end(); it != itEnd; ++it)
	{
		// overwrite any previous property with this name
		container::iterator found = iterator_find(_props, it->mName, it->mNamespace);
		if (found != _props.end())
		{
			Property a = *it;
			a.setOrder(found->getOrder());
			_props.replace(found, a);
		}
		else
		{
			Property a = *it;
			a.setOrder(- ++mDefaultOrder - 1);
			_props.insert(a);
		}
	}
}

bool
PropertyList::addGetterSetter(string_table::key key, as_function& getter,
	as_function* setter, const as_value& cacheVal, string_table::key nsId)
{
	Property a(key, nsId, &getter, setter);
	a.setCache(cacheVal);
	a.setOrder(- ++mDefaultOrder - 1);

	container::iterator found = iterator_find(_props, key, nsId);
	if (found != _props.end())
	{
		// copy flags from previous member (even if it's a normal member ?)
		as_prop_flags& f = a.getFlags();
		f = found->getFlags();

		_props.replace(found, a);
		assert ( iterator_find(_props, key, nsId) != _props.end() );

	}
	else
	{
		_props.insert(a);
        	assert ( iterator_find(_props, key, nsId) != _props.end() );
	}


	return true;
}

bool
PropertyList::addGetterSetter(string_table::key key, as_c_function_ptr getter,
	as_c_function_ptr setter, string_table::key nsId)
{
	Property a(key, nsId, getter, setter);
	a.setOrder(- ++mDefaultOrder - 1);

	container::iterator found = iterator_find(_props, key, nsId);
	if (found != _props.end())
	{
		// copy flags from previous member (even if it's a normal member ?)
		as_prop_flags& f = a.getFlags();
		f = found->getFlags();

		_props.replace(found, a);
		assert ( iterator_find(_props, key, nsId) != _props.end() );

	}
	else
	{
		_props.insert(a);
        	assert ( iterator_find(_props, key, nsId) != _props.end() );
	}


	return true;
}

bool
PropertyList::addDestructiveGetter(string_table::key key,
	as_function& getter, string_table::key nsId,
	const as_prop_flags& flagsIfMissing)
{
	container::iterator found = iterator_find(_props, key, nsId);
	if (found != _props.end())
		return false; // Already exists.

	// destructive getter don't need a setter
	Property a(key, nsId, &getter, (as_function*)0, flagsIfMissing, true);
	a.setOrder(- ++mDefaultOrder - 1);
	_props.insert(a);
	return true;
}

void
PropertyList::clear()
{
	_props.clear();
}

void
PropertyList::setReachable() const
{
	for (container::const_iterator it = _props.begin();
			it != _props.end(); ++it)
	{
		it->setReachable();
	}
}

} // end of gnash namespace

