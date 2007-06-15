// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// 
//
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "GetterSetter.h"

#include "as_environment.h"
#include "fn_call.h"
#include "as_value.h" 
#include "as_function.h"

#include "log.h"


namespace gnash {

as_value
GetterSetter::getValue(as_object* this_ptr) const
{
	as_environment env;
	fn_call fn(this_ptr, &env, 0, 0);
	// TODO: need as_function::operator to be const..
	return (*const_cast<as_function*>(_getter))(fn);
}

void
GetterSetter::setValue(as_object* this_ptr, const as_value& val) const
{
	as_environment env;
	env.push(val);
	fn_call fn(this_ptr, &env, 1, 0);
	// TODO: need as_function::operator to be const..
	(*const_cast<as_function*>(_setter))(fn);
}

GetterSetter&
GetterSetter::operator=(const GetterSetter& s)
{
	if ( s._getter != _getter )
	{
#ifndef GNASH_USE_GC
		_getter->drop_ref();
#endif // ndef GNASH_USE_GC
		_getter = s._getter;
#ifndef GNASH_USE_GC
		_getter->add_ref();
#endif // ndef GNASH_USE_GC
	}
	if ( s._setter != _setter )
	{
#ifndef GNASH_USE_GC
		_setter->drop_ref();
#endif // ndef GNASH_USE_GC
		_setter = s._setter;
#ifndef GNASH_USE_GC
		_setter->add_ref();
#endif // ndef GNASH_USE_GC
	}
	return *this;
}

GetterSetter::GetterSetter(const GetterSetter& s)
	:
	_getter(s._getter),
	_setter(s._setter)
{
#ifndef GNASH_USE_GC
	_getter->add_ref();
	_setter->add_ref();
#endif // ndef GNASH_USE_GC
}

GetterSetter::
GetterSetter(as_function& getter, as_function& setter)
	:
	_getter(&getter),
	_setter(&setter)
{
#ifndef GNASH_USE_GC
	_getter->add_ref();
	_setter->add_ref();
#endif // ndef GNASH_USE_GC
}

GetterSetter::~GetterSetter()
{
#ifndef GNASH_USE_GC
	_getter->drop_ref();
	_setter->drop_ref();
#endif // ndef GNASH_USE_GC
}

void
GetterSetter::setReachable() const
{
#ifdef GNASH_USE_GC
	_getter->setReachable();
	_setter->setReachable();
#endif
}

} // end of gnash namespace

