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
#include "as_function.h"
#include "as_environment.h" // for enumerateProperties

#include <set>

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
	//log_action("  get member: %s (at %p) for object %p\n", name.c_str(), (void*)val, (void*)this);
	if (name == "__proto__")
	{
		if ( m_prototype == NULL )
		{
			log_msg("as_object %p has no prototype\n", (void*)this);
			return false;
		}
		val->set_as_object(m_prototype);
		return true;
	}
	else
	{
		as_member m;

		if (m_members.get(name, &m) == false)
		{
			//log_action("  not found on first level\n");
			if (m_prototype == NULL)
			{
				//log_action("  no __proto__ (m_prototype) defined\n");
				return false;
			}
			else
			{
				//log_action("  checkin in __proto__ (m_prototype) %p\n", (void*)m_prototype);
				return m_prototype->get_member(name, val);
			}
		} else {
			//log_action("  found on first level\n");
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
as_object::set_prototype(as_object* proto)
{
	if (m_prototype) m_prototype->drop_ref();
	m_prototype = proto;
	if (m_prototype) m_prototype->add_ref();
}

void
as_object::set_member_default(const tu_stringi& name, const as_value& val )
{
	//printf("SET MEMBER: %s = %s for object %p\n", name.c_str(), val.to_string(), this);
	if (name == "__proto__") 
	{
		set_prototype(val.to_object());
		return;
	}

	stringi_hash<as_member>::const_iterator it = this->m_members.find(name);
	
	if ( it == this->m_members.end() )
	{
		m_members[name] = as_member(val);
		return;
	}

	const as_prop_flags flags = (it->second).get_member_flags();

	// is the member read-only ?
	if (!flags.get_read_only()) {
		m_members[name] = as_member(val, flags);
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
as_object::instanceOf(as_function* ctor)
{
	as_object* proto=m_prototype;
	do {
		if ( proto == ctor->getPrototype() ) return true;
		proto = ctor->getPrototype();
	} while (proto);

	return false;
}

void
as_object::dump_members() const
{
	typedef stringi_hash<as_member>::const_iterator members_iterator;

	//Vitaly: temporarily commented because of problems with the VC++ compiler
//	log_msg("%d Members of object %p follow",
//		m_members..size(), (void*)this);
	for ( members_iterator it=m_members.begin(), itEnd=m_members.end();
		it != itEnd; ++it )
	{
		log_msg("  %s: %s", it->first.c_str(), it->second.get_member_value().to_string());
	}
}

void
as_object::setPropFlags(as_value& props_val, int set_false, int set_true)
{
	if ( props_val.get_type() == as_value::STRING )
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

		stringi_hash<as_member>::iterator it = \
			m_members.find(prop.c_str());
		if ( it != m_members.end() )
		{
			as_member& member = it->second;
			as_prop_flags f = member.get_member_flags();
			f.set_flags(set_true, set_false);
			member.set_member_flags(f);
		}
		else
		{
			log_warning("Unknown object property %s, "
				"can't set propflags on it", prop.c_str());
		}


			if ( next_comma == std::string::npos )
			{
				break;
			}
		}
		return;
	}

	as_object* props = props_val.to_object();

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

		stringi_hash<as_member>::iterator it = m_members.begin();
		while (it != m_members.end())
		{
			as_member& member = it->second;
			as_prop_flags f = member.get_member_flags();
			f.set_flags(set_true, set_false);
			member.set_member_flags(f);
			++it;
		}

		// Are we sure we need to descend to __proto__ ?
		// should we recurse then ?

		if (m_prototype != NULL)
		{
			as_object* prototype = m_prototype;

			it = prototype->m_members.begin();
			while (it != prototype->m_members.end())
			{
				as_member& member = it->second;
				as_prop_flags f = member.get_member_flags();
				f.set_flags(set_true, set_false);
				member.set_member_flags(f);

				++it;
			}
		}
	}
	else
	{
		as_object* object_props = props;

		stringi_hash<as_member>::iterator it = object_props->m_members.begin();
		while(it != object_props->m_members.end())
		{
			const tu_stringi key = (it->second).get_member_value().to_string();
			stringi_hash<as_member>::iterator it2 = m_members.find(key);

			if (it2 != m_members.end())
			{
				as_member& member = it2->second;

				as_prop_flags f = member.get_member_flags();
				f.set_flags(set_true, set_false);
				member.set_member_flags(f);
			}

			++it;
		}
	}
}

void
as_object::copyProperties(const as_object& o)
{
	typedef stringi_hash<as_member>::const_iterator members_iterator;
	for (members_iterator it = o.m_members.begin(),
				itEnd = o.m_members.end();
				it != itEnd;
				++it )
	{
		const tu_stringi name = it->first;
		const as_member	member = it->second;
		// TODO: don't call get_member_value, we
		//       must copy also 'getset' members ...
		set_member(name, member.get_member_value());
	}
}

void
as_object::enumerateProperties(as_environment& env) const
{
	assert( env.top(0).get_type() == as_value::NULLTYPE );


	// this set will keep track of visited objects,
	// to avoid infinite loops
	std::set<const as_object*> visited;

	typedef stringi_hash<as_member>::const_iterator members_iterator;

	const as_object* obj = this;
	while ( obj && visited.insert(obj).second )
	{
		for ( members_iterator
			it=obj->m_members.begin(), itEnd=obj->m_members.end();
			it!=itEnd;
			++it )
		{
			const as_member& member = it->second;
		
			if (! member.get_member_flags().get_dont_enum())
			{
				// shouldn't this be a tu_string instead ?
				// we need to support UTF8 too I guess
				const char* val = it->first.c_str();

				env.push(as_value(val));
			}  
		}

		obj = obj->m_prototype;
	}

	if ( obj ) log_warning("prototype loop during Enumeration");
}

} // end of gnash namespace

