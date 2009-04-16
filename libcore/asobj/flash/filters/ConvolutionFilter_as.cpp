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
#include "ConvolutionFilter.h"
#include "VM.h"
#include "builtin_function.h"

#include "BitmapFilter_as.h"

namespace gnash {

class ConvolutionFilter_as : public as_object, public ConvolutionFilter
{
public:
    static as_value matrixX_gs(const fn_call& fn);
    static as_value matrixY_gs(const fn_call& fn);
    static as_value matrix_gs(const fn_call& fn);
    static as_value divisor_gs(const fn_call& fn);
    static as_value bias_gs(const fn_call& fn);
    static as_value preserveAlpha_gs(const fn_call& fn);
    static as_value clamp_gs(const fn_call& fn);
    static as_value color_gs(const fn_call& fn);
    static as_value alpha_gs(const fn_call& fn);
    static as_value bitmap_clone(const fn_call& fn);

    ConvolutionFilter_as(as_object *obj)
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
static boost::intrusive_ptr<builtin_function> s_ctor;

};

boost::intrusive_ptr<as_object> ConvolutionFilter_as::s_interface;

boost::intrusive_ptr<builtin_function> ConvolutionFilter_as::s_ctor;

as_object*
ConvolutionFilter_as::Interface()
{
    if (ConvolutionFilter_as::s_interface == NULL) {
        ConvolutionFilter_as::s_interface = new as_object (bitmapFilter_interface());
         VM::get().addStatic(ConvolutionFilter_as::s_interface.get());
         ConvolutionFilter_as::attachInterface(*ConvolutionFilter_as::s_interface);
    }
    return ConvolutionFilter_as::s_interface.get();
}

void
ConvolutionFilter_as::registerCtor(as_object& global)
{
    if (ConvolutionFilter_as::s_ctor != NULL) return;
    ConvolutionFilter_as::s_ctor = new builtin_function(&ConvolutionFilter_as::ctor, ConvolutionFilter_as::Interface());
    VM::get().addStatic(ConvolutionFilter_as::s_ctor.get());
    ConvolutionFilter_as::attachInterface(*ConvolutionFilter_as::s_ctor);
    global.init_member("ConvolutionFilter" , ConvolutionFilter_as::s_ctor.get());
}

void
ConvolutionFilter_class_init(as_object& global)
{
    ConvolutionFilter_as::registerCtor(global);
}

void
ConvolutionFilter_as::attachInterface(as_object& o)
{
    boost::intrusive_ptr<builtin_function> gs;
    o.set_member(VM::get().getStringTable().find("clone"), new builtin_function(bitmap_clone));
}

void
ConvolutionFilter_as::attachProperties(as_object& o) {
    boost::intrusive_ptr<builtin_function> gs;

    gs = new builtin_function(ConvolutionFilter_as::matrixX_gs, NULL);
    o.init_property("matrixX" , *gs, *gs);

    gs = new builtin_function(ConvolutionFilter_as::matrixY_gs, NULL);
    o.init_property("matrixY" , *gs, *gs);

    gs = new builtin_function(ConvolutionFilter_as::matrix_gs, NULL);
    o.init_property("matrix" , *gs, *gs);

    gs = new builtin_function(ConvolutionFilter_as::divisor_gs, NULL);
    o.init_property("divisor" , *gs, *gs);

    gs = new builtin_function(ConvolutionFilter_as::bias_gs, NULL);
    o.init_property("bias" , *gs, *gs);

    gs = new builtin_function(ConvolutionFilter_as::preserveAlpha_gs, NULL);
    o.init_property("preserveAlpha" , *gs, *gs);

    gs = new builtin_function(ConvolutionFilter_as::clamp_gs, NULL);
    o.init_property("clamp" , *gs, *gs);

    gs = new builtin_function(ConvolutionFilter_as::color_gs, NULL);
    o.init_property("color" , *gs, *gs);

    gs = new builtin_function(ConvolutionFilter_as::alpha_gs, NULL);
    o.init_property("alpha" , *gs, *gs);

}

as_value
ConvolutionFilter_as::matrixX_gs(const fn_call& fn)
{
	boost::intrusive_ptr<ConvolutionFilter_as> ptr = ensureType<ConvolutionFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->_matrixX );
    }
    boost::uint8_t sp_matrixX = fn.arg(0).to_number<boost::uint8_t> ();
    ptr->_matrixX = sp_matrixX;
    return as_value();
}

as_value
ConvolutionFilter_as::matrixY_gs(const fn_call& fn)
{
	boost::intrusive_ptr<ConvolutionFilter_as> ptr = ensureType<ConvolutionFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->_matrixY );
    }
    boost::uint8_t sp_matrixY = fn.arg(0).to_number<boost::uint8_t> ();
    ptr->_matrixY = sp_matrixY;
    return as_value();
}

as_value
ConvolutionFilter_as::divisor_gs(const fn_call& fn)
{
	boost::intrusive_ptr<ConvolutionFilter_as> ptr = ensureType<ConvolutionFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->_divisor );
    }
    float sp_divisor = fn.arg(0).to_number<float> ();
    ptr->_divisor = sp_divisor;
    return as_value();
}

as_value
ConvolutionFilter_as::bias_gs(const fn_call& fn)
{
	boost::intrusive_ptr<ConvolutionFilter_as> ptr = ensureType<ConvolutionFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) { return as_value(ptr->_bias );
    }
    float sp_bias = fn.arg(0).to_number<float> ();
    ptr->_bias = sp_bias;
    return as_value();
}

as_value
ConvolutionFilter_as::preserveAlpha_gs(const fn_call& fn)
{
	boost::intrusive_ptr<ConvolutionFilter_as> ptr = ensureType<ConvolutionFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->_preserveAlpha);
    }
    bool sp_preserveAlpha = fn.arg(0).to_bool();
    ptr->_preserveAlpha = sp_preserveAlpha;
    return as_value();
}

as_value
ConvolutionFilter_as::clamp_gs(const fn_call& fn)
{
	boost::intrusive_ptr<ConvolutionFilter_as> ptr = ensureType<ConvolutionFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->_clamp );
    }
    bool sp_clamp = fn.arg(0).to_bool ();
    ptr->_clamp = sp_clamp;
    return as_value();
}

as_value
ConvolutionFilter_as::color_gs(const fn_call& fn)
{
	boost::intrusive_ptr<ConvolutionFilter_as> ptr = ensureType<ConvolutionFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->_color );
    } 
    boost::uint32_t sp_color = fn.arg(0).to_number<boost::uint32_t> ();
    ptr->_color = sp_color;
    return as_value();
}

as_value
ConvolutionFilter_as::alpha_gs(const fn_call& fn)
{
	boost::intrusive_ptr<ConvolutionFilter_as> ptr = ensureType<ConvolutionFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->_alpha );
    }
    boost::uint8_t sp_alpha = fn.arg(0).to_number<boost::uint8_t> ();
    ptr->_alpha = sp_alpha;
    return as_value();
}

as_value
ConvolutionFilter_as::matrix_gs(const fn_call& fn)
{
	boost::intrusive_ptr<ConvolutionFilter_as> ptr = ensureType<ConvolutionFilter_as>(fn.this_ptr);
    return as_value();
}

as_value
ConvolutionFilter_as::bitmap_clone(const fn_call& fn)
{
	boost::intrusive_ptr<ConvolutionFilter_as> ptr = ensureType<ConvolutionFilter_as>(fn.this_ptr);
    boost::intrusive_ptr<ConvolutionFilter_as> obj = new ConvolutionFilter_as(*ptr);
    boost::intrusive_ptr<as_object> r = obj;
    r->set_prototype(ptr->get_prototype());
    r->copyProperties(*ptr);
    return as_value(r);
}

as_value
ConvolutionFilter_as::ctor(const fn_call& )
{
    boost::intrusive_ptr<as_object> obj = new ConvolutionFilter_as(ConvolutionFilter_as::Interface());
    ConvolutionFilter_as::attachProperties(*obj);
    return as_value(obj.get());
}

}
