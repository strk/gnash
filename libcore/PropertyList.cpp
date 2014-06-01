// PropertyList.cpp:  ActionScript property lists, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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
#include "gnashconfig.h" // GNASH_STATS_PROPERTY_LOOKUPS
#endif

#include "PropertyList.h"

#include <utility> 
#include <functional> 
#include <boost/tuple/tuple.hpp>

#include "Property.h" 
#include "as_environment.h"
#include "log.h"
#include "as_function.h"
#include "as_value.h" 
#include "VM.h" 
#include "string_table.h"
#include "GnashAlgorithm.h"

// Define the following to enable printing address of each property added
//#define DEBUG_PROPERTY_ALLOC

// Define this to get verbosity of properties insertion and flags setting
//#define GNASH_DEBUG_PROPERTY 1

// Define this to get stats of property lookups 
//#define GNASH_STATS_PROPERTY_LOOKUPS 1

#ifdef GNASH_STATS_PROPERTY_LOOKUPS
# include "Stats.h"
# include "namedStrings.h"
#endif

namespace gnash {

namespace {

inline
PropertyList::const_iterator
iterator_find(const PropertyList::container& p, const ObjectURI& uri, VM& vm)
{
    const bool caseless = vm.getSWFVersion() < 7;

    if (!caseless) {
        return p.project<PropertyList::CreationOrder>(
                p.get<PropertyList::Case>().find(uri));
    }
        
    return p.project<PropertyList::CreationOrder>(
            p.get<PropertyList::NoCase>().find(uri));
}

}
    
PropertyList::PropertyList(as_object& obj)
    :
    _props(boost::make_tuple(
                boost::tuple<>(),
                boost::make_tuple(
                    KeyExtractor(),
                    ObjectURI::LessThan()
                ),
                boost::make_tuple(
                    KeyExtractor(),
                    ObjectURI::CaseLessThan(getStringTable(obj), true)
                )
            )
        ),
    _owner(obj)
{
}

bool
PropertyList::setValue(const ObjectURI& uri, const as_value& val,
        const PropFlags& flagsIfMissing)
{
	const_iterator found = iterator_find(_props, uri, getVM(_owner));
	
	if (found == _props.end()) {
		// create a new member
		Property a(uri, val, flagsIfMissing);
		// Non slot properties are negative ordering in insertion order
		_props.push_back(a);
#ifdef GNASH_DEBUG_PROPERTY
        ObjectURI::Logger l(getStringTable(_owner));
        log_debug("Simple AS property %s inserted with flags %s",
			l(uri), a.getFlags());
#endif
		return true;
	}

	const Property& prop = *found;
	return prop.setValue(_owner, val);

}

void
PropertyList::setFlags(const ObjectURI& uri, int setFlags, int clearFlags)
{
	iterator found = iterator_find(_props, uri, getVM(_owner));
	if (found == _props.end()) return;
    PropFlags f = found->getFlags();
    f.set_flags(setFlags, clearFlags);
	found->setFlags(f);

}

void
PropertyList::setFlagsAll(int setFlags, int clearFlags)
{
    for (const auto& prop: _props) {
        PropFlags f = prop.getFlags();
        f.set_flags(setFlags, clearFlags);
        prop.setFlags(f);
    }
}

Property*
PropertyList::getProperty(const ObjectURI& uri) const
{
#ifdef GNASH_STATS_PROPERTY_LOOKUPS
    // HINT: can add a final arg to KeyLookup ctor, like NSV::PROP_ON_MOUSE_MOVE
    //       to have *that* property lookup drive dump triggers
    static stats::KeyLookup kcl("getProperty",
        getStringTable(_owner), 10000000, NSV::PROP_uuPROTOuu, 10);
    kcl.check(uri.name);
#endif // GNASH_STATS_PROPERTY_LOOKUPS
	iterator found = iterator_find(_props, uri, getVM(_owner));
	if (found == _props.end()) return nullptr;
	return const_cast<Property*>(&(*found));
}

std::pair<bool,bool>
PropertyList::delProperty(const ObjectURI& uri)
{
	//GNASH_REPORT_FUNCTION;
	iterator found = iterator_find(_props, uri, getVM(_owner));
	if (found == _props.end()) {
		return std::make_pair(false, false);
	}

	// check if member is protected from deletion
	if (found->getFlags().test<PropFlags::dontDelete>()) {
		return std::make_pair(true, false);
	}

	_props.erase(found);
	return std::make_pair(true, true);
}

void
PropertyList::visitKeys(KeyVisitor& visitor, PropertyTracker& donelist)
    const
{
    // We should enumerate in order of creation, not lexicographically.
	for (const auto& prop : _props) {

		if (prop.getFlags().test<PropFlags::dontEnum>()) continue;

        const ObjectURI& uri = prop.uri();

		if (donelist.insert(uri).second) {
			visitor(uri);
		}
	}
}

void
PropertyList::dump()
{
    ObjectURI::Logger l(getStringTable(_owner));
	for (const auto& prop : _props) {
            log_debug("  %s: %s", l(prop.uri()), prop.getValue(_owner));
	}
}

bool
PropertyList::addGetterSetter(const ObjectURI& uri, as_function& getter,
	as_function* setter, const as_value& cacheVal,
	const PropFlags& flagsIfMissing)
{
	Property a(uri, &getter, setter, flagsIfMissing);
	iterator found = iterator_find(_props, uri, getVM(_owner));
    
	if (found != _props.end()) {
		// copy flags from previous member (even if it's a normal member ?)
		a.setFlags(found->getFlags());
		a.setCache(found->getCache());
		_props.replace(found, a);

#ifdef GNASH_DEBUG_PROPERTY
        ObjectURI::Logger l(getStringTable(_owner));
        log_debug("AS GetterSetter %s replaced copying flags %s", l(uri),
                a.getFlags());
#endif

	}
	else {
		a.setCache(cacheVal);
		_props.push_back(a);
#ifdef GNASH_DEBUG_PROPERTY
        ObjectURI::Logger l(getStringTable(_owner));
        log_debug("AS GetterSetter %s inserted with flags %s", l(uri),
                a.getFlags());
#endif
	}

	return true;
}

bool
PropertyList::addGetterSetter(const ObjectURI& uri, as_c_function_ptr getter,
	as_c_function_ptr setter, const PropFlags& flagsIfMissing)
{
	Property a(uri, getter, setter, flagsIfMissing);

	const_iterator found = iterator_find(_props, uri, getVM(_owner));
	if (found != _props.end())
	{
		// copy flags from previous member (even if it's a normal member ?)
		a.setFlags(found->getFlags());
		_props.replace(found, a);

#ifdef GNASH_DEBUG_PROPERTY
        ObjectURI::Logger l(getStringTable(_owner));
        log_debug("Native GetterSetter %s replaced copying flags %s", l(uri),
                a.getFlags());
#endif

	}
	else
	{
		_props.push_back(a);
#ifdef GNASH_DEBUG_PROPERTY
		string_table& st = getStringTable(_owner);
		log_debug("Native GetterSetter %s in namespace %s inserted with "
                          "flags %s", st.value(key), st.value(nsId), a.getFlags());
#endif
	}

	return true;
}

bool
PropertyList::addDestructiveGetter(const ObjectURI& uri, as_function& getter, 
	const PropFlags& flagsIfMissing)
{
	const_iterator found = iterator_find(_props, uri, getVM(_owner));
	if (found != _props.end())
	{
        ObjectURI::Logger l(getStringTable(_owner));
        log_error(_("Property %s already exists, can't addDestructiveGetter"),
                l(uri));
		return false; // Already exists.
	}

	// destructive getter doesn't need a setter
	Property a(uri, &getter, nullptr, flagsIfMissing, true);

	_props.push_back(a);

#ifdef GNASH_DEBUG_PROPERTY
    ObjectURI::Logger l(getStringTable(_owner));
    log_debug("Destructive AS property %s inserted with flags %s",
            l(uri), a.getFlags());
#endif

	return true;
}

bool
PropertyList::addDestructiveGetter(const ObjectURI& uri,
	as_c_function_ptr getter, const PropFlags& flagsIfMissing)
{
	iterator found = iterator_find(_props, uri, getVM(_owner));
	if (found != _props.end()) return false; 

	// destructive getter doesn't need a setter
	Property a(uri, getter, nullptr, flagsIfMissing, true);
	_props.push_back(a);

#ifdef GNASH_DEBUG_PROPERTY
    ObjectURI::Logger l(getStringTable(_owner));
    log_debug("Destructive native property %s with flags %s", l(uri),
            a.getFlags());
#endif
	return true;
}

void
PropertyList::clear()
{
	_props.clear();
}

} // namespace gnash

