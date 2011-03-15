// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "Property.h"

#include <boost/variant.hpp>
#include <boost/bind.hpp>

#include "VM.h"
#include "as_function.h"
#include "as_environment.h"
#include "fn_call.h"

namespace gnash {

namespace {

// TODO: drop this.
enum Type {
    TYPE_VALUE,
    TYPE_GETTER_SETTER
};

/// Get the cache of a variant member.
struct GetCache : boost::static_visitor<as_value>
{
    result_type operator()(as_value& val) const {
        return val;
    }
    result_type operator()(GetterSetter& gs) const {
        return gs.getCache();
    }
};

/// Set the cache of a variant member.
struct SetCache : boost::static_visitor<>
{
    result_type operator()(as_value& val, const as_value& n) const {
        val = n;
    }
    result_type operator()(GetterSetter& gs, const as_value& n) const {
        gs.setCache(n);
    }
};

}

void
GetterSetter::UserDefinedGetterSetter::markReachableResources() const
{
	if (_getter) _getter->setReachable();
	if (_setter) _setter->setReachable();
	_underlyingValue.setReachable();
}

as_value
GetterSetter::UserDefinedGetterSetter::get(const fn_call& fn) const
{
	ScopedLock lock(*this);
	if (!lock.obtainedLock()) {
		return _underlyingValue;
	}

	if (_getter) return _getter->call(fn);
    
    // should we return underlyingValue here ?
	return as_value(); 
}

void
GetterSetter::UserDefinedGetterSetter::set(const fn_call& fn)
{
	ScopedLock lock(*this);
	if (!lock.obtainedLock() || ! _setter) {
		_underlyingValue = fn.arg(0);
		return;
	}

	_setter->call(fn);
}

as_value
Property::getValue(const as_object& this_ptr) const
{
	switch (_bound.which())
	{
        case TYPE_VALUE:
            return boost::get<as_value>(_bound);
        case TYPE_GETTER_SETTER:
        {
            const GetterSetter* a = boost::get<const GetterSetter>(&_bound);

            as_environment env(getVM(this_ptr));
            fn_call fn(const_cast<as_object*>(&this_ptr), env);
            if (_destructive)
            {
                as_value ret = a->get(fn);
                // The getter might have called the setter, and we
                // should not override.
                if (_destructive) {
                    _bound = ret;
                    _destructive = false;
                }
                return ret;
            }
            return a->get(fn);
        }
	} 
    return as_value();
}

as_value
Property::getCache() const
{
    return boost::apply_visitor(GetCache(), _bound);
}

bool
Property::setValue(as_object& this_ptr, const as_value& value) const
{
    if (readOnly(*this)) {
        if (_destructive) {
            _destructive = false;
            _bound = value;
            return true;
        }
        return false;
    }

	switch (_bound.which()) {
        case TYPE_VALUE: 
            _bound = value;
            return true;
        case TYPE_GETTER_SETTER:
            // Destructive are always overwritten.
            if (_destructive) {
                _destructive = false;
                _bound = value;
            }
            else {
                GetterSetter* a = boost::get<GetterSetter>(&_bound);

                as_environment env(getVM(this_ptr));

                fn_call::Args args;
                args += value;

                fn_call fn(&this_ptr, env, args);

                a->set(fn);
                a->setCache(value);
            }
    }
    return true;
}

void
Property::setCache(const as_value& value)
{
    boost::apply_visitor(boost::bind(SetCache(), _1, value), _bound);
}

} // namespace gnash
