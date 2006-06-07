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

#include "log.h"

#include "as_object.h"
//#include "action.h" // when we've finished, Object.h will stay, action.h away
#include "Function.h"

namespace gnash {

/*public virtual*/
bool
as_object::get_member(const tu_stringi& name, as_value* val)
{
	return get_member_default(name, val);
}

/*protected*/
bool
as_object::get_member_default(const tu_stringi& name, as_value* val)
{
    log_action("  get member: %s (at %p) for object %p\n", name.c_str(), (void*)val, (void*)this);
	if (name == "__proto__")
	{
		if ( m_prototype == NULL ) log_msg("as_object %p has no prototype\n", (void*)this);
		val->set_as_object(m_prototype);
		return true;
	}
	else {
		as_member m;

		if (m_members.get(name, &m) == false)
		{
			log_action("  not found on first level\n");
			if (m_prototype == NULL)
			{
				log_action("  no __proto__ (m_prototype) defined\n");
				return false;
			}
			else
			{
				log_action("  checkin in __proto__ (m_prototype) %p\n", (void*)m_prototype);
				return m_prototype->get_member(name, val);
			}
		} else {
			log_action("  found on first level\n");
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
as_object::set_member(const tu_stringi& name, const as_value& val)
{
	return set_member_default(name, val);
}

void
as_object::set_member_default(const tu_stringi& name, const as_value& val )
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

			const as_prop_flags flags = (it->second).get_member_flags();

			// is the member read-only ?
			if (!flags.get_read_only()) {
				m_members[name] = as_member(val, flags);
			}

		} else {
			m_members[name] = as_member(val);
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

		m_members[name] = member;

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

