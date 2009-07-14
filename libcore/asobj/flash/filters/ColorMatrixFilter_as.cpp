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
#include "ColorMatrixFilter.h"
#include "VM.h"
#include "builtin_function.h"
#include "Global_as.h"
#include "BitmapFilter_as.h"

namespace gnash {

class ColorMatrixFilter_as : public as_object, public ColorMatrixFilter
{
public:
    static as_value matrix_gs(const fn_call& fn);
    static as_value bitmap_clone(const fn_call& fn);

    ColorMatrixFilter_as(as_object *obj)
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


boost::intrusive_ptr<as_object> ColorMatrixFilter_as::s_interface;
boost:: intrusive_ptr<builtin_function> ColorMatrixFilter_as::s_ctor;

as_object* ColorMatrixFilter_as::Interface() {
    if (ColorMatrixFilter_as::s_interface == NULL) {
        ColorMatrixFilter_as::s_interface = new as_object (getBitmapFilterInterface());
        VM::get().addStatic(ColorMatrixFilter_as::s_interface.get());
        ColorMatrixFilter_as::attachInterface(*ColorMatrixFilter_as::s_interface);
    }
    return ColorMatrixFilter_as::s_interface.get();
}

void
ColorMatrixFilter_as::registerCtor(as_object& global)
{
    if (ColorMatrixFilter_as::s_ctor != NULL) return;
    ColorMatrixFilter_as::s_ctor = new builtin_function(&ColorMatrixFilter_as::ctor, ColorMatrixFilter_as::Interface());
    VM::get().addStatic(ColorMatrixFilter_as::s_ctor.get());
    ColorMatrixFilter_as::attachInterface(*ColorMatrixFilter_as::s_ctor);
    global.init_member("ColorMatrixFilter" , ColorMatrixFilter_as::s_ctor.get());
}

void
colormatrixfilter_class_init(as_object& global)
{
    ColorMatrixFilter_as::registerCtor(global);
}


void
ColorMatrixFilter_as::attachInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
	boost::intrusive_ptr<builtin_function> gs;

    o.set_member(VM::get().getStringTable().find("clone"), gl->createFunction(bitmap_clone));

}

void
ColorMatrixFilter_as::attachProperties(as_object& o)
{
	boost::intrusive_ptr<builtin_function> gs;

    gs = new builtin_function(ColorMatrixFilter_as::matrix_gs, NULL);
    o.init_property("matrix" , *gs, *gs);
}

as_value
ColorMatrixFilter_as::matrix_gs(const fn_call& fn)
{
	boost::intrusive_ptr<ColorMatrixFilter_as> ptr = ensureType<ColorMatrixFilter_as>(fn.this_ptr);
    return as_value();
}

as_value
ColorMatrixFilter_as::bitmap_clone(const fn_call& fn)
{
	boost::intrusive_ptr<ColorMatrixFilter_as> ptr = ensureType<ColorMatrixFilter_as>(fn.this_ptr);
    boost::intrusive_ptr<ColorMatrixFilter_as> obj = new ColorMatrixFilter_as(*ptr);
    boost::intrusive_ptr<as_object> r = obj;
    r->set_prototype(ptr->get_prototype());
    r->copyProperties(*ptr);
    return as_value(r);
}

as_value
ColorMatrixFilter_as::ctor(const fn_call& )
{
    boost::intrusive_ptr<as_object> obj = new ColorMatrixFilter_as(ColorMatrixFilter_as::Interface());

    ColorMatrixFilter_as::attachProperties(*obj);
    return as_value(obj.get());

}

}
