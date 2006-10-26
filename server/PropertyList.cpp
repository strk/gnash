// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "PropertyList.h"

#include "log.h"

#include "as_function.h"
#include "as_environment.h" // for enumerateValues
#include "as_value.h" // for enumerateValues

namespace gnash {

//PropertyList::PropertyList(as_object& owner)
//	:
//	_owner(owner)
//{
//}


bool
PropertyList::getValue(const std::string& key, as_value& val) const
{
	const_iterator found = _props.find( key );
	if ( found == _props.end() )
	{
		return false;
	}

	val=found->second.get_member_value();

	//log_msg("Property %s found, assigning to return (%s)", key.c_str(), val.to_string());

	return true;
}

bool
PropertyList::setValue(const std::string& key, const as_value& val) 
{
	iterator found = _props.find( key );
	if ( found == _props.end() )
	{
		// create a new member
		_props[key] = as_member(val);
		return true;
	}

	as_member& member = found->second;

	if ( member.is_read_only() )
	{
		log_warning("Property %s is read-only, not setting it", key.c_str());
		return false;
	}

	//log_msg("Property %s set to value %s", key.c_str(), val.to_string());
	member.set_member_value(val);
	return true;
}

bool
PropertyList::setFlags(const std::string& key,
		int setFlags, int clearFlags)
{
	iterator found = _props.find( key );
	if ( found == _props.end() ) return false;

	as_member& member = found->second;

	as_prop_flags& f = member.get_member_flags();
	return f.set_flags(setFlags, clearFlags);
}

std::pair<size_t,size_t>
PropertyList::setFlagsAll(int setFlags, int clearFlags)
{
	size_t success=0;
	size_t failure=0;

	for ( iterator it=_props.begin(), far=_props.end(); it != far; ++it)
	{
		as_member& member = it->second;
		as_prop_flags& f = member.get_member_flags();
		if ( f.set_flags(setFlags, clearFlags) ) ++success;
		else ++failure;
	}

	return std::make_pair(success,failure);
}

std::pair<size_t,size_t>
PropertyList::setFlagsAll(const PropertyList& props,
		int flagsSet, int flagsClear)
{
	size_t success=0;
	size_t failure=0;

	for (const_iterator it = begin(), itEnd = end(); it != itEnd; ++it )
	{
		const std::string& name = it->first;

		if ( setFlags(name, flagsSet, flagsClear) ) ++success;
		else ++failure;
	}

	return std::make_pair(success,failure);

}

void
PropertyList::enumerateValues(as_environment& env) const
{
	for ( const_iterator i=begin(), ie=end(); i != ie; ++i)
	{
		const as_member& member = i->second;

		if ( member.get_member_flags().get_dont_enum() ) continue;

		env.push(as_value(i->first.c_str()));
	}
}

void
PropertyList::dump() const
{
	for ( const_iterator it=begin(), itEnd=end(); it != itEnd; ++it )
	{
		log_msg("  %s: %s", it->first.c_str(),
			it->second.get_member_value().to_string());
	}
}

void
PropertyList::import(const PropertyList& o) 
{
	for (const_iterator it = o.begin(), itEnd = o.end(); it != itEnd; ++it)
	{
		const std::string& name = it->first;
		const as_member& member = it->second;

		// TODO: don't call get_member_value, we
		//       must copy also 'getset' members ...
		setValue(name, member.get_member_value());
	}
}

} // end of gnash namespace

