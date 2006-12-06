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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "check.h"
#include "GetterSetter.h"
#include "builtin_function.h"
#include "as_object.h"
#include "as_value.h"
#include "fn_call.h"
#include "log.h"
#include "VM.h"
#include "DummyMovieDefinition.h"

#include "check.h"
#include <iostream>
#include <sstream>
#include <cassert>
#include <string>

using namespace std;
using namespace gnash;

/// return the object's text value
static void getter(const fn_call& fn)
{
	as_object* o = fn.this_ptr;
	assert(fn.nargs == 0);
	const char* txt = o->get_text_value();
	fn.result->set_string(txt);
}

/// set a new member to the object
static void setter(const fn_call& fn)
{
	as_object* o = fn.this_ptr;
	assert(fn.nargs == 1);
	as_value& val = fn.arg(0);
	o->set_member(val.to_string(), val);
}

struct test_object: public as_object {
	std::string textval;

	test_object(const char* initial_val)
		:
		textval(initial_val)
	{
	}

	const string& getText() const { return textval; }

	const char* get_text_value() const { return textval.c_str(); }
	void set_member(const tu_stringi& , const as_value& val )
	{
		textval = val.to_string();
	}
};

int
main(int /*argc*/, char** /*argv*/)
{

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity();

	boost::intrusive_ptr<movie_definition> md6 ( new DummyMovieDefinition(6) );
	VM& vm = VM::init(*md6);

	test_object obj("initial text");
	test_object obj2("other obj");

	builtin_function* get = new builtin_function(&getter);
	builtin_function* set = new builtin_function(&setter);

	GetterSetter getset(*get, *set);

	as_value val;
	getset.getValue(&obj, val);
	check_equals(obj.getText(), string("initial text"));
	check_equals(val, as_value("initial text"));

	val.set_string("second try");
	getset.setValue(&obj, val);
	check_equals(obj.getText(), string("second try"));
	val.set_string("");

	getset.getValue(&obj, val);
	check_equals(val, as_value("second try"));

	// Test copy ctor

	GetterSetter getset2(getset);

	getset2.getValue(&obj2, val);
	check_equals(obj2.getText(), string("other obj"));
	check_equals(val, as_value("other obj"));

	val.set_string("second try for other");
	getset2.setValue(&obj2, val);
	check_equals(obj2.getText(), string("second try for other"));
	val.set_string("");

	getset2.getValue(&obj2, val);
	check_equals(val, as_value("second try for other"));

	getset2.getValue(&obj, val);
	check_equals(val, as_value("second try"));

	// Test assignment
	
	GetterSetter tmp(getset);
	tmp.getValue(&obj, val);
	check_equals(val, as_value("second try"));
	tmp = getset2;
	tmp.getValue(&obj2, val);
	check_equals(val, as_value("second try for other"));
}

