// Number.cpp:  ActionScript Number class, for Gnash.
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


#include "log.h"
#include "Number_as.h"
#include "fn_call.h"
#include "Global_as.h"
#include "as_object.h" // for inheritance
#include "as_value.h" // for doubleToString
#include "NativeFunction.h"
#include "VM.h"

#include "namedStrings.h"
#include <sstream>
#include <cmath>


namespace gnash {

class Number_as : public Relay
{
public:

    Number_as(double val)
        :
        _val(val)
    {
    }

    double value() const {
        return _val;
    }

private:
    
    // the number value
    double _val;

};

namespace {

as_value
number_toString(const fn_call& fn)
{
    // Number.toString must only work for number object, not generic ones.
    // This is so trace(Number.prototype) doesn't return 0 ...
    Number_as* obj = ensure<ThisIsNative<Number_as> >(fn);

    double val = obj->value();
    unsigned radix = 10;

    if ( fn.nargs ) 
    {
        int userRadix = toInt(fn.arg(0), getVM(fn));
        if ( userRadix >= 2 && userRadix <= 36 ) radix=userRadix;
        else
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Number.toString(%s): "
                "radix must be in the 2..36 range (%d is invalid)"),
                fn.arg(0), userRadix)
            )
        }

    }
    return doubleToString(val, radix); 
}

as_value
number_valueOf(const fn_call& fn)
{
    // Number.valueOf must only work for number object, not generic ones.
    // This is so trace(Number.prototype == Object) return true in swf5 ?
    Number_as* obj = ensure<ThisIsNative<Number_as> >(fn);

    return obj->value();
}

as_value
number_ctor(const fn_call& fn)
{
    double val = 0;
    if (fn.nargs > 0) {
        val = toNumber(fn.arg(0), getVM(fn));
    }

    if (!fn.isInstantiation()) {
        return as_value(val);
    }

    fn.this_ptr->setRelay(new Number_as(val));
    
    return as_value(); 
}

void
attachNumberInterface(as_object& o)
{
    VM& vm = getVM(o);
    o.init_member("valueOf", vm.getNative(106, 0));
    o.init_member("toString", vm.getNative(106, 1));
}

void
attachNumberStaticInterface(as_object& o)
{
    // constant flags
    const int cflags = as_object::DefaultFlags | PropFlags::readOnly;

    // Set __proto__ and constructor to constant.
    as_value null; null.set_null();
    o.setPropFlags(null, 0, cflags);

    // Not quite the same as numeric_limits<double>::max()
    o.init_member("MAX_VALUE", 1.79769313486231e+308, cflags);
    // This is generally numeric_limits<double>::denorm_min().
    o.init_member("MIN_VALUE", 4.94065645841247e-324, cflags);
    o.init_member("NaN", as_value(NaN), cflags);
    o.init_member("POSITIVE_INFINITY",
            as_value(std::numeric_limits<double>::infinity()), cflags);
    o.init_member("NEGATIVE_INFINITY",
            as_value(-std::numeric_limits<double>::infinity()), cflags);
}

} // anonymous namespace


// extern (used by Global.cpp)
void
number_class_init(as_object& where, const ObjectURI& uri)
{
    VM& vm = getVM(where);
    Global_as& gl = getGlobal(where);

    as_object* proto = createObject(gl);
    as_object* cl = vm.getNative(106, 2);
    cl->init_member(NSV::PROP_PROTOTYPE, proto);
    proto->init_member(NSV::PROP_CONSTRUCTOR, cl);

    attachNumberInterface(*proto);
    attachNumberStaticInterface(*cl);

    // Register _global.Number
    where.init_member(uri, cl, as_object::DefaultFlags);

}

void
registerNumberNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(number_valueOf, 106, 0);
    vm.registerNative(number_toString, 106, 1);
    vm.registerNative(number_ctor, 106, 2);
}

} // namespace gnash
