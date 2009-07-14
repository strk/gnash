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
#include "BlurFilter.h"
#include "VM.h"
#include "builtin_function.h"
#include "Global_as.h"

#include "BitmapFilter_as.h"

namespace gnash {

class BlurFilter_as : public as_object, public BlurFilter
{
public:
    static as_value blurX_gs(const fn_call& fn);
    static as_value blurY_gs(const fn_call& fn);
    static as_value quality_gs(const fn_call& fn);
    static as_value bitmap_clone(const fn_call& fn);

    BlurFilter_as(as_object *obj)
	    :
        as_object(obj)
    {
    }
    
    static as_object* Interface();
    static void attachInterface(as_object& o);
    static void attachProperties(as_object& o);
    static void registerCtor(as_object& global);
    static as_value ctor(const fn_call& fn);
private:
    static boost::intrusive_ptr<as_object> s_interface;

};


boost::intrusive_ptr<as_object> BlurFilter_as::s_interface;

as_object*
BlurFilter_as::Interface() {
    if (BlurFilter_as::s_interface == NULL) {
        BlurFilter_as::s_interface = new as_object(getBitmapFilterInterface());
        VM::get().addStatic(BlurFilter_as::s_interface.get());
        BlurFilter_as::attachInterface(*BlurFilter_as::s_interface);
    }
    return BlurFilter_as::s_interface.get();
}

void
BlurFilter_as::registerCtor(as_object& global)
{
    static boost::intrusive_ptr<as_object> cl;
    if (!cl) return;

    Global_as* gl = getGlobal(global);
    cl = gl->createClass(&BlurFilter_as::ctor, BlurFilter_as::Interface());;
    BlurFilter_as::attachInterface(*cl);

    global.init_member("BlurFilter" , cl.get());

}

void
blurfilter_class_init(as_object& global)
{
    BlurFilter_as::registerCtor(global);
}


void
BlurFilter_as::attachInterface(as_object& o) {
    Global_as* gl = getGlobal(o);
    boost::intrusive_ptr<builtin_function> gs;

    o.set_member(VM::get().getStringTable().find("clone"), gl->createFunction(bitmap_clone));

}

void
BlurFilter_as::attachProperties(as_object& o)
{
    boost::intrusive_ptr<builtin_function> gs;
    o.init_property("blurX" , BlurFilter_as::blurX_gs, 
        BlurFilter_as::blurX_gs);
    o.init_property("blurY" , BlurFilter_as::blurY_gs, 
        BlurFilter_as::blurY_gs);
    o.init_property("quality" , BlurFilter_as::quality_gs, 
        BlurFilter_as::quality_gs);

}

as_value
BlurFilter_as::blurX_gs(const fn_call& fn)
{
    
    boost::intrusive_ptr<BlurFilter_as> ptr = ensureType<BlurFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->m_blurX );
    }
    float sp_blurX = fn.arg(0).to_number<float> ();
    ptr->m_blurX = sp_blurX;
    return as_value();
}

as_value
BlurFilter_as::blurY_gs(const fn_call& fn)
{
	boost::intrusive_ptr<BlurFilter_as> ptr = ensureType<BlurFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->m_blurY );
    }
    float sp_blurY = fn.arg(0).to_number<float> ();
    ptr->m_blurY = sp_blurY;
    return as_value();
}

as_value
BlurFilter_as::quality_gs(const fn_call& fn)
{
	boost::intrusive_ptr<BlurFilter_as> ptr = ensureType<BlurFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->m_quality );
    }
    boost::uint8_t sp_quality = fn.arg(0).to_number<boost::uint8_t> ();
    ptr->m_quality = sp_quality;
    return as_value();
}

as_value
BlurFilter_as::bitmap_clone(const fn_call& fn)
{
	boost::intrusive_ptr<BlurFilter_as> ptr = ensureType<BlurFilter_as>(fn.this_ptr);
    boost::intrusive_ptr<BlurFilter_as> obj = new BlurFilter_as(*ptr);
    boost::intrusive_ptr<as_object> r = obj;
    r->set_prototype(ptr->get_prototype());
    r->copyProperties(*ptr);
    return as_value(r);
}

as_value
BlurFilter_as::ctor(const fn_call& )
{
    boost::intrusive_ptr<as_object> obj = new BlurFilter_as(BlurFilter_as::Interface());

    BlurFilter_as::attachProperties(*obj);
    return as_value(obj);

}

}
