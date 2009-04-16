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

namespace gnash {

class BitmapFilter_as : public as_object, public BitmapFilter
{
public:
    static as_value bitmap_clone(const fn_call& fn);

    virtual boost::intrusive_ptr<as_object> clone();

    BitmapFilter_as(as_object *obj)
	    :
        as_object(obj)
    {}
    static as_object* Interface();
    static void attachInterface(as_object& o);
    static void attachProperties(as_object& o);
    static void registerCtor(as_object& global);
    static as_value ctor(const fn_call& fn);
private:
    static boost::intrusive_ptr<as_object> s_interface;
    static boost::intrusive_ptr<builtin_function> s_ctor;
};

boost::intrusive_ptr<as_object> BitmapFilter_as::s_interface;
boost:: intrusive_ptr<builtin_function> BitmapFilter_as::s_ctor;

as_object*
BitmapFilter_as::Interface() {
    if (BitmapFilter_as::s_interface == NULL) {
        BitmapFilter_as::s_interface = new as_object;
        VM::get().addStatic(BitmapFilter_as::s_interface.get());
        BitmapFilter_as::attachInterface(*BitmapFilter_as::s_interface);
    }
    return BitmapFilter_as::s_interface.get();
}

void
BitmapFilter_as::registerCtor(as_object& global) {
    if (BitmapFilter_as::s_ctor != NULL) return;
    BitmapFilter_as::s_ctor = new builtin_function(&BitmapFilter_as::ctor, BitmapFilter_as::Interface());
    VM::get().addStatic(BitmapFilter_as::s_ctor.get());
    BitmapFilter_as::attachInterface(*BitmapFilter_as::s_ctor);
    global.init_member("BitmapFilter" , BitmapFilter_as::s_ctor.get());
}

void
BitmapFilter_class_init(as_object& global) {
    BitmapFilter_as::registerCtor(global);
}

void
BitmapFilter_as::attachInterface(as_object& o)
{
    boost::intrusive_ptr<builtin_function> gs;
    o.init_member("clone" , new builtin_function(bitmap_clone));
}


boost::intrusive_ptr<as_object> BitmapFilter_as::clone()
{
    boost::intrusive_ptr<as_object> o = new BitmapFilter_as(BitmapFilter_as::Interface());
    return o;
}

as_value
BitmapFilter_as::ctor(const fn_call& )
{
    boost::intrusive_ptr<as_object> obj = new BitmapFilter_as(BitmapFilter_as::Interface());
    return as_value(obj);
}

as_value
BitmapFilter_as::bitmap_clone(const fn_call& fn)
{
    boost::intrusive_ptr<BitmapFilter_as> to_copy = ensureType<BitmapFilter_as> (fn.this_ptr);
    boost::intrusive_ptr<BitmapFilter_as> filter = new BitmapFilter_as(*to_copy);
    filter->set_prototype(filter->get_prototype());
    filter->copyProperties(*filter);
    boost::intrusive_ptr<as_object> r = filter;

    return as_value(r);
}

as_object*
bitmapFilter_interface()
{
    return BitmapFilter_as::Interface();
}

}
