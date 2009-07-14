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
#include "Object.h"
#include "Global_as.h"

namespace gnash {

namespace {
    
    as_value bitmapfilter_ctor(const fn_call& fn);
    as_value bitmapfilter_clone(const fn_call& fn);
    as_value getBitmapFilterConstructor(const fn_call& fn);
    void attachBitmapFilterInterface(as_object& o);
}
 
// TODO: Use composition, not inheritance.
class BitmapFilter_as : public as_object, public BitmapFilter
{
public:
    BitmapFilter_as(as_object *obj)
	    :
        as_object(obj)
    {}
};


void
bitmapfilter_class_init(as_object& global)
{
    string_table& st = getStringTable(global);
    
    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
    global.init_destructive_property(st.find("BitmapFilter"),
            getBitmapFilterConstructor, flags);
}

as_object*
getBitmapFilterInterface()
{
    static as_object* o;
    if (!o) {
        o = new as_object(getObjectInterface());
        VM::get().addStatic(o);
        attachBitmapFilterInterface(*o);
    }
    return o;
}

namespace {

void
attachBitmapFilterInterface(as_object& o)
{
    const int flags = 0;
    Global_as* gl = getGlobal(o);
    o.init_member("clone", gl->createFunction(bitmapfilter_clone), flags);
}

as_value
getBitmapFilterConstructor(const fn_call& fn)
{
    static builtin_function* cl;
    if (!cl) {
        cl = new builtin_function(&bitmapfilter_ctor,
                getBitmapFilterInterface());
        getVM(fn).addStatic(cl);
    }
    return cl;
}

as_value
bitmapfilter_ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj =
        new BitmapFilter_as(getBitmapFilterInterface());
    return as_value(obj);
}

as_value
bitmapfilter_clone(const fn_call& fn)
{
    boost::intrusive_ptr<BitmapFilter_as> to_copy = ensureType<BitmapFilter_as> (fn.this_ptr);
    boost::intrusive_ptr<BitmapFilter_as> filter = new BitmapFilter_as(*to_copy);
    filter->set_prototype(filter->get_prototype());
    filter->copyProperties(*filter);
    boost::intrusive_ptr<as_object> r = filter;

    return as_value(r);
}
} // anonymous namespace
} // gnash namespace
