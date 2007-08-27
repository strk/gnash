// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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

/* $Id: BitmapFilter_as.cpp,v 1.1 2007/08/27 03:06:42 cmusick Exp $ */

#include "BitmapFilter.h"
#include "VM.h"
#include "builtin_function.h"

namespace gnash {

class BitmapFilter_as
{
public:
    static as_object* Interface(); // To a BitmapFilter
    static void attachInterface(as_object& o); // Attach the interface.
    static void registerCtor(as_object& global); // public ctor
    static as_value ctor(const fn_call& fn); // constructor for BitmapFilter

    static as_value bitmap_clone(const fn_call& fn);

private:
    static boost::intrusive_ptr<as_object> s_interface;
    static boost::intrusive_ptr<builtin_function> s_ctor;
};

boost::intrusive_ptr<as_object> BitmapFilter_as::s_interface;
boost::intrusive_ptr<builtin_function> BitmapFilter_as::s_ctor;

as_object*
BitmapFilter_as::Interface()
{
    if (BitmapFilter_as::s_interface == NULL)
    {
        BitmapFilter_as::s_interface = new as_object;
        VM::get().addStatic(BitmapFilter_as::s_interface.get());

        BitmapFilter_as::attachInterface(*BitmapFilter_as::s_interface);
    }

    return BitmapFilter_as::s_interface.get();
}

void
BitmapFilter_as::attachInterface(as_object& o)
{
    o.init_member("clone", new builtin_function(bitmap_clone));
}

void
BitmapFilter_as::registerCtor(as_object& global)
{
    if (BitmapFilter_as::s_ctor != NULL)
        return;

    BitmapFilter_as::s_ctor = new builtin_function(&BitmapFilter_as::ctor,
        BitmapFilter_as::Interface());
    VM::get().addStatic(BitmapFilter_as::s_ctor.get());

    // TODO: Is this correct?
    BitmapFilter_as::attachInterface(*BitmapFilter_as::s_ctor);

    global.init_member("BitmapFilter", BitmapFilter_as::s_ctor.get());
}

as_value
BitmapFilter_as::ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new BitmapFilter(BitmapFilter_as::Interface());
    return as_value(obj.get());
}

as_value BitmapFilter_as::bitmap_clone(const fn_call& fn)
{
    boost::intrusive_ptr<BitmapFilter> filter = ensureType<BitmapFilter> (fn.this_ptr);
    boost::intrusive_ptr<as_object> retval = filter->clone();
    retval->set_prototype(filter->get_prototype());

    return as_value(retval);
}

void bitmapFilter_class_init(as_object& global)
{
   BitmapFilter_as::registerCtor(global);
}

as_object*
bitmapFilter_interface()
{
    return BitmapFilter_as::Interface();
}

} // Namespace gnash

