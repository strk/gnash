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

/* $Id: TextFormat.cpp,v 1.1 2007/04/05 01:28:40 nihilus Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "TextFormat.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function


as_value textformat_tostring(const fn_call& fn);
as_value textformat_ctor(const fn_call& fn);

static void
attachTextFormatInterface(as_object& o)
{
	// is this really needed ? shouldn't toString be
	// derived from Object inheritance ?
	o.init_member("toString", new builtin_function(textformat_tostring));
}

static as_object*
getTextFormatInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object();
		attachTextFormatInterface(*o);
	}
	return o.get();
}

class textformat_as_object: public as_object
{

public:

	textformat_as_object()
		:
		as_object(getTextFormatInterface())
	{}

	// override from as_object ?
	//const char* get_text_value() const { return "TextFormat"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};

as_value textformat_tostring(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
    return as_value();
}

as_value
textformat_ctor(const fn_call& /* fn */)
{
	boost::intrusive_ptr<as_object> obj = new textformat_as_object;
	
	return as_value(obj.get()); // will keep alive
}

// extern (used by Global.cpp)
void textformat_class_init(as_object& global)
{
	// This is going to be the global TextFormat "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&textformat_ctor, getTextFormatInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachTextFormatInterface(*cl);
		     
	}

	// Register _global.TextFormat
	global.init_member("TextFormat", cl.get());

}


} // end of gnash namespace

