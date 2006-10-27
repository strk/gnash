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
#include "Property.h" // for findGetterSetter

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
as_object::get_member_default(const tu_stringi& namei, as_value* val)
{
	assert(val);

	// temp hack, should really update this method's interface instead
	std::string name = namei.c_str();

	// TODO: inspect wheter it is possible to make __proto__ a
	//       getter/setter property instead, to take this into account
	//
	if (namei == "__proto__")
	{
		if ( m_prototype == NULL )
		{
			log_msg("as_object %p has no prototype\n", (void*)this);
			return false;
		}
		val->set_as_object(m_prototype);
		return true;
	}

	Property* prop = findProperty(name);
	if ( ! prop ) return false;

	*val = prop->getValue(*this);
	return true;
	
}

/*private*/
Property*
as_object::findProperty(const std::string& key)
{
	// this set will keep track of visited objects,
	// to avoid infinite loops
	std::set<const as_object*> visited;

	as_object* obj = this;
	while ( obj && visited.insert(obj).second )
	{
		Property* prop = obj->_members.getProperty(key);
		if ( prop ) return prop;
		else obj = obj->m_prototype;
	}

	// No Property found
	return NULL;

}

/*private*/
Property*
as_object::findGetterSetter(const std::string& key)
{
	// this set will keep track of visited objects,
	// to avoid infinite loops
	std::set<const as_object*> visited;

	as_object* obj = this;
	while ( obj && visited.insert(obj).second )
	{
		Property* prop = obj->_members.getProperty(key);
		if ( prop && prop->isGetterSetter() )
		{
			// what if a property is found which is
			// NOT a getter/setter ?
			return prop;
		}
		obj = obj->m_prototype;
	}

	// No Getter/Setter property found
	return NULL;

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
	// TODO: make __proto__ a getter/setter ?
	if (name == "__proto__") 
	{
		set_prototype(val.to_object());
		return;
	}

        std::string key = name.c_str();

	// found a getter/setter property in the inheritance chain
	// so set that and return
	Property* prop = findGetterSetter(key);
	if ( prop )
	{
		//log_msg("Found a getter/setter property for key %s", key.c_str());
		// TODO: have setValue check for read-only property 
		//       and warn if failed
		prop->setValue(*this, val);
		return;
	}

	//log_msg("Found NO getter/setter property for key %s", key.c_str());

	// No getter/setter property found, so set (or create) a
	// SimpleProperty (if possible)
	if ( ! _members.setValue(key, val, *this) )
	{
		log_warning("Attempt to set Read-Only property ``%s''"
			" on object ``%p''",
			key.c_str(), (void*)this);
	}

}

bool
as_object::set_member_flags(const tu_stringi& name, int setTrue, int setFalse)
{
	// TODO: accept a std::string directly
	return _members.setFlags(std::string(name.c_str()), setTrue, setFalse);
}

void
as_object::clear()
{
	_members.clear();
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
as_object::dump_members() 
{
	log_msg("%d Members of object %p follow",
		_members.size(), (const void*)this);
	_members.dump(*this);
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

			if ( ! _members.setFlags(prop, set_true, set_false) )
			{
				log_warning("Can't set propflags on object "
					"property %s "
					"(either not found or protected)",
					prop.c_str());
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
		//std::pair<size_t, size_t> result = 
		_members.setFlagsAll(set_true, set_false);

		// Are we sure we need to descend to __proto__ ?
		// should we recurse then ?

		if (m_prototype != NULL)
		{
			m_prototype->_members.setFlagsAll(set_true, set_false);
		}
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
	_members.import(o._members);
}

void
as_object::enumerateProperties(as_environment& env) const
{
	assert( env.top(0).get_type() == as_value::NULLTYPE );


	// this set will keep track of visited objects,
	// to avoid infinite loops
	std::set<const as_object*> visited;

	const as_object* obj = this;
	while ( obj && visited.insert(obj).second )
	{
		obj->_members.enumerateValues(env);
		obj = obj->m_prototype;
	}

	if ( obj ) log_warning("prototype loop during Enumeration");
}

} // end of gnash namespace

