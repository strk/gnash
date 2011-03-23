// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#include "BitmapFilter_as.h"

#include "namedStrings.h"
#include "as_object.h"
#include "VM.h"
#include "NativeFunction.h"
#include "Global_as.h"
#include "Filters.h"

namespace gnash {

namespace {
    
    as_value bitmapfilter_new(const fn_call& fn);
    as_value bitmapfilter_clone(const fn_call& fn);
    as_value getBitmapFilterConstructor(const fn_call& fn);
    void attachBitmapFilterInterface(as_object& o);
}
 
/// This may need a reference to its owner as_object
//
/// TODO: is BitmapFilter GC collected? Not currently (see libcore/Filters.h)
class BitmapFilter_as : public Relay
{
public:
    BitmapFilter_as()
        //: _filter(new BitmapFilter)
    {}

    // ~BitmapFilter_as() { delete _filter; }

private:
    //BitmapFilter* _filter;
};


void
bitmapfilter_class_init(as_object& where, const ObjectURI& uri)
{
    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
    where.init_destructive_property(uri, getBitmapFilterConstructor, flags);
}

void
registerBitmapFilterNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(bitmapfilter_new, 1112, 0);
    vm.registerNative(bitmapfilter_clone, 1112, 1);
}

void
registerBitmapClass(as_object& where, Global_as::ASFunction ctor,
        Global_as::Properties p, const ObjectURI& uri)
{
    Global_as& gl = getGlobal(where);

    VM& vm = getVM(where);

    // We should be looking for flash.filters.BitmapFilter, but as this
    // triggers a lookup of the flash.filters package while we are creating
    // it, so entering infinite recursion, we'll cheat and assume that
    // the object 'where' is the filters package.
    as_function* constructor =
        getMember(where, getURI(vm, "BitmapFilter")).to_function();
    
    as_object* proto;
    if (constructor) {
        fn_call::Args args;
        VM& vm = getVM(where);
        proto = constructInstance(*constructor, as_environment(vm), args);
    }
    else proto = 0;

    as_object* cl = gl.createClass(ctor, createObject(gl));
    if (proto) p(*proto);

    // The startup script overwrites the prototype assigned by ASconstructor,
    // so the new prototype doesn't have a constructor property. We do the
    // same here.
    cl->set_member(NSV::PROP_PROTOTYPE, proto);
    where.init_member(uri , cl, as_object::DefaultFlags);

}

namespace {

void
attachBitmapFilterInterface(as_object& o)
{
    const int flags = PropFlags::onlySWF8Up;
    VM& vm = getVM(o);
    o.init_member("clone", vm.getNative(1112, 1), flags);
}

as_value
getBitmapFilterConstructor(const fn_call& fn)
{
    log_debug("Loading flash.filters.BitmapFilter class");
    Global_as& gl = getGlobal(fn);
    VM& vm = getVM(fn);
    
    as_object* proto = createObject(gl);
    as_object* cl = vm.getNative(1112, 0);
    cl->init_member(NSV::PROP_PROTOTYPE, proto);
    proto->init_member(NSV::PROP_CONSTRUCTOR, cl);
    
    attachBitmapFilterInterface(*proto);
    return cl;
}

as_value
bitmapfilter_new(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);
    obj->setRelay(new BitmapFilter_as);
    return as_value();
}

/// TODO: there are no tests for how this works, so it's not implemented.
as_value
bitmapfilter_clone(const fn_call& fn)
{
    BitmapFilter_as* relay = ensure<ThisIsNative<BitmapFilter_as> >(fn);
    UNUSED(relay);
    return as_value();
}
} // anonymous namespace
} // gnash namespace
