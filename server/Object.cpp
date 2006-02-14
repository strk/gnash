// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"

//#include "Object.h"
#include "action.h" // when we've finished, Object.h will stay, action.h away
#include "Function.h"

namespace gnash {

bool
as_object::get_member(const tu_stringi& name, as_value* val)
{
	IF_VERBOSE_ACTION(
		log_msg("  get member: %s (at %p) for object %p\n", name.c_str(), val, this);
	);
	if (name == "__proto__")
	{
		if ( m_prototype == NULL ) log_msg("as_object %p has no prototype\n", this);
		val->set_as_object(m_prototype);
		return true;
	}
	else {
		as_member m;

		if (m_members.get(name, &m) == false)
		{
			IF_VERBOSE_ACTION(log_msg("  not found on first level\n"));
			if (m_prototype == NULL)
			{
				IF_VERBOSE_ACTION(log_msg("  no __proto__ (m_prototype) defined\n"));
				return false;
			}
			else
			{
				IF_VERBOSE_ACTION(log_msg("  checkin in __proto__ (m_prototype) %p\n",m_prototype));
				return m_prototype->get_member(name, val);
			}
		} else {
			IF_VERBOSE_ACTION(log_msg("  found on first level\n"));
			*val=m.get_member_value();
			return true;
		}
	}
	return true;
}

bool
as_object::get_member(const tu_stringi& name, as_member* member) const
{
	//printf("GET MEMBER: %s at %p for object %p\n", name.c_str(), member, this);
	assert(member != NULL);
	return m_members.get(name, member);
}

void
as_object::set_member(const tu_stringi& name, const as_value& val )
{
	//printf("SET MEMBER: %s at %p for object %p\n", name.c_str(), val.to_object(), this);
	if (name == "__proto__") 
	{
		if (m_prototype) m_prototype->drop_ref();
		m_prototype = val.to_object();
		if (m_prototype) m_prototype->add_ref();
	}
	else
	{
		stringi_hash<as_member>::const_iterator it = this->m_members.find(name);
		
		if ( it != this->m_members.end() ) {

			const as_prop_flags flags = (it.get_value()).get_member_flags();

			// is the member read-only ?
			if (!flags.get_read_only()) {
				m_members.set(name, as_member(val, flags));
			}

		} else {
			m_members.set(name, as_member(val));
		}
	}
}

bool
as_object::set_member_flags(const tu_stringi& name, const int flags)
{
	as_member member;
	if (this->get_member(name, &member)) {
		as_prop_flags f = member.get_member_flags();
		f.set_flags(flags);
		member.set_member_flags(f);

		m_members.set(name, member);

		return true;
	}

	return false;
}

void
as_object::clear()
{
	m_members.clear();
	if (m_prototype)
	{
		m_prototype->drop_ref();
		m_prototype = NULL;
	}
}

bool
as_object::instanceOf(function_as_object* ctor)
{
	as_object* proto=m_prototype;
	do {
		if ( proto == ctor->m_properties ) return true;
		proto = ctor->m_properties;
	} while (proto);

	return false;
}

} // end of gnash namespace

