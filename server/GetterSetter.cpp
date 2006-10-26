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

#include "GetterSetter.h"

#include "as_environment.h"
#include "fn_call.h"
#include "as_value.h" 
#include "as_function.h"

#include "log.h"


namespace gnash {

void
GetterSetter::getValue(as_object* this_ptr, as_value& ret) const
{
	as_environment env;
	fn_call fn(&ret, this_ptr, &env, 0, 0);
	// TODO: need as_function::operator to be const..
	(*const_cast<as_function*>(_getter))(fn);
}

void
GetterSetter::setValue(as_object* this_ptr, const as_value& val) const
{
	as_environment env;
	env.push(val);
	fn_call fn(NULL, this_ptr, &env, 1, 0);
	// TODO: need as_function::operator to be const..
	(*const_cast<as_function*>(_setter))(fn);
}

GetterSetter&
GetterSetter::operator==(const GetterSetter& s)
{
	if ( s._getter != _getter )
	{
		_getter->drop_ref();
		_getter = s._getter;
		_getter->add_ref();
	}
	if ( s._setter != _setter )
	{
		_setter->drop_ref();
		_setter = s._setter;
		_setter->add_ref();
	}
	return *this;
}

GetterSetter::GetterSetter(const GetterSetter& s)
	:
	_getter(s._getter),
	_setter(s._setter)
{
	_getter->add_ref();
	_setter->add_ref();
}

GetterSetter::
GetterSetter(as_function& getter, as_function& setter)
	:
	_getter(&getter),
	_setter(&setter)
{
	_getter->add_ref();
	_setter->add_ref();
}

GetterSetter::~GetterSetter()
{
	_getter->drop_ref();
	_setter->drop_ref();
}


} // end of gnash namespace

