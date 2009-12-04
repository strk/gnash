// EventDispatcher.cpp:  Implementation of ActionScript int class, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include "smart_ptr.h"
#include "fn_call.h"
#include "Global_as.h"
#include "as_object.h"
#include "builtin_function.h" 

#include "log.h"

#include <string>
#include <sstream>

namespace gnash {

class int_as : public Relay
{
public:
    int_as(int32_t v)
        :
        _int(v)
    {}

private:
    boost::int32_t _int;

};

as_value
int_ctor(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);

    if (fn.nargs) {
        LOG_ONCE( log_unimpl("Arguments passed to int() ctor unhandled") );
    }
	
    obj->setRelay(new int_as(fn.nargs ? fn.arg(0).to_int() : 0));
    return as_value();
}

// Note that this class can be constructed but is not usable!
//
// It needs support in as_value first.
void
int_class_init(as_object& global, const ObjectURI& uri)
{

    Global_as& gl = getGlobal(global);
    as_object* proto = gl.createObject();
    as_object* cl = gl.createClass(&int_ctor, proto);

	// Register _global.DisplayObject
	global.init_member(uri, cl, as_object::DefaultFlags);
}

}
