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

#include "namedStrings.h"
#include "Boolean_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "NativeFunction.h"
#include "GnashException.h"
#include "VM.h" 

namespace gnash {

namespace {    
    as_value boolean_tostring(const fn_call& fn);
    as_value boolean_valueof(const fn_call& fn);
    as_value boolean_ctor(const fn_call& fn);
    void attachBooleanInterface(as_object& o);
}

class Boolean_as: public Relay
{

public:

    explicit Boolean_as(bool val)
        :
        _val(val)
    {
    }
    
    bool value() const { return _val; }

private:

    bool _val;
    
};

// extern (used by Global.cpp)
void
boolean_class_init(as_object& where, const ObjectURI& uri)
{
    VM& vm = getVM(where);
    Global_as& gl = getGlobal(where);

    as_object* proto = createObject(gl);
    as_object* cl = vm.getNative(107, 2);
    cl->init_member(NSV::PROP_PROTOTYPE, proto);
    proto->init_member(NSV::PROP_CONSTRUCTOR, cl);

    attachBooleanInterface(*proto);
    
    // Register _global.Boolean
    where.init_member(uri, cl, as_object::DefaultFlags);

}

void
registerBooleanNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(boolean_valueof, 107, 0);
    vm.registerNative(boolean_tostring, 107, 1);
    vm.registerNative(boolean_ctor, 107, 2);
}

namespace {


void
attachBooleanInterface(as_object& o)
{
    VM& vm = getVM(o);
    o.init_member("valueOf", vm.getNative(107, 0));
    o.init_member("toString", vm.getNative(107, 1));
}

as_value
boolean_tostring(const fn_call& fn)
{
    Boolean_as* obj = ensure<ThisIsNative<Boolean_as> >(fn);
    if (obj->value()) return as_value("true");
    return as_value("false");
}


as_value
boolean_valueof(const fn_call& fn) 
{
    Boolean_as* obj = ensure<ThisIsNative<Boolean_as> >(fn);
    return as_value(obj->value());
}

as_value
boolean_ctor(const fn_call& fn)
{

    if (!fn.isInstantiation()) {
        if (!fn.nargs) return as_value();
        return as_value(toBool(fn.arg(0), getVM(fn)));
    }

    const bool val = fn.nargs ? toBool(fn.arg(0), getVM(fn)) : false;

    as_object* obj = fn.this_ptr;
    obj->setRelay(new Boolean_as(val));
    return as_value();

}

} // anonymous namespace
} // gnash namespace

