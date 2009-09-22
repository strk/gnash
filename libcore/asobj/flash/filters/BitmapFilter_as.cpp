// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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


#include "as_object.h"
#include "BitmapFilter.h"
#include "VM.h"
#include "builtin_function.h"
#include "NativeFunction.h"
#include "Object.h"
#include "Global_as.h"

namespace gnash {

namespace {
    
    as_value bitmapfilter_new(const fn_call& fn);
    as_value bitmapfilter_clone(const fn_call& fn);
    as_value getBitmapFilterConstructor(const fn_call& fn);
    void attachBitmapFilterInterface(as_object& o);
}
 
/// This may need a reference to its owner as_object
//
/// TODO: is BitmapFilter GC collected?
class BitmapFilter_as : public Relay
{
public:
    BitmapFilter_as()
        :
        _filter(new BitmapFilter)
    {}

private:
    BitmapFilter* _filter;
};


void
bitmapfilter_class_init(as_object& where, const ObjectURI& uri)
{
    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
    where.init_destructive_property(getName(uri), getBitmapFilterConstructor,
		    flags, getNamespace(uri));
}

void
registerBitmapFilterNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(bitmapfilter_new, 1112, 0);
    vm.registerNative(bitmapfilter_clone, 1112, 1);
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
    Global_as* gl = getGlobal(fn);
    as_object* proto = gl->createObject();
    as_object* cl = gl->createClass(&bitmapfilter_new, proto);
    attachBitmapFilterInterface(*proto);
    return cl;
}

as_value
bitmapfilter_new(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);
    obj->setRelay(new BitmapFilter_as);
}

/// TODO: there are no tests for how this works, so it's not implemented.
as_value
bitmapfilter_clone(const fn_call& fn)
{
    BitmapFilter_as* relay = ensureNativeType<BitmapFilter_as>(fn.this_ptr);
    UNUSED(relay);
    return as_value();
}
} // anonymous namespace
} // gnash namespace
