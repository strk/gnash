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
#include "Global_as.h"
#include "builtin_function.h"
#include "Object.h"
#include "BitmapFilter_as.h"

namespace gnash {

namespace {
    
    as_value convolutionfilter_ctor(const fn_call& fn);
    as_value convolutionfilter_clone(const fn_call& fn);
    as_value getConvolutionFilterConstructor(const fn_call& fn);
    as_value convolutionfilter_matrixX(const fn_call& fn);
    as_value convolutionfilter_matrixY(const fn_call& fn);
    as_value convolutionfilter_matrix(const fn_call& fn);
    as_value convolutionfilter_divisor(const fn_call& fn);
    as_value convolutionfilter_bias(const fn_call& fn);
    as_value convolutionfilter_preserveAlpha(const fn_call& fn);
    as_value convolutionfilter_clamp(const fn_call& fn);
    as_value convolutionfilter_color(const fn_call& fn);
    as_value convolutionfilter_alpha(const fn_call& fn);

    void attachConvolutionFilterInterface(as_object& o);
}
 
// TODO: Use composition, not inheritance.
class ConvolutionFilter_as : public as_object, public ConvolutionFilter
{
public:
    ConvolutionFilter_as(as_object *obj)
        :
        as_object(obj)
    {}

};


void
convolutionfilter_class_init(as_object& where, const ObjectURI& uri)
{
    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
    where.init_destructive_property(getName(uri),
            getConvolutionFilterConstructor, flags, getNamespace(uri));
}

as_object*
getConvolutionFilterInterface()
{
    static as_object* o;
    if (!o) {
        o = new as_object(getBitmapFilterInterface());
        VM::get().addStatic(o);
        attachConvolutionFilterInterface(*o);
    }
    return o;
}

namespace {

void
attachConvolutionFilterInterface(as_object& o)
{
    const int flags = 0;
    o.init_property("matrixX" , convolutionfilter_matrixX,
            convolutionfilter_matrixX, flags);
    o.init_property("divisor" , convolutionfilter_divisor,
            convolutionfilter_divisor, flags);
    o.init_property("matrix" , convolutionfilter_matrix,
            convolutionfilter_matrix, flags);
    o.init_property("matrixY" , convolutionfilter_matrixY,
            convolutionfilter_matrixY, flags);
    o.init_property("alpha" , convolutionfilter_alpha,
            convolutionfilter_alpha, flags);
    o.init_property("clamp" , convolutionfilter_clamp,
            convolutionfilter_clamp, flags);
    o.init_property("preserveAlpha" , convolutionfilter_preserveAlpha,
            convolutionfilter_preserveAlpha, flags);
    o.init_property("bias" , convolutionfilter_bias,
            convolutionfilter_bias, flags);
    o.init_property("color" , convolutionfilter_color,
            convolutionfilter_color, flags);
}

as_value
getConvolutionFilterConstructor(const fn_call& fn)
{
    Global_as* gl = getGlobal(fn);
    return gl->createClass(&convolutionfilter_ctor,
            getConvolutionFilterInterface());
}

as_value
convolutionfilter_ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj =
        new ConvolutionFilter_as(getConvolutionFilterInterface());
    return as_value(obj);
}

as_value
convolutionfilter_matrixX(const fn_call& fn)
{
    // TODO: check whether this is necessary (probably is).
    boost::intrusive_ptr<ConvolutionFilter_as> ptr =
        ensureType<ConvolutionFilter_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl(__PRETTY_FUNCTION__);
    return as_value();
}

as_value
convolutionfilter_matrixY(const fn_call& fn)
{
    boost::intrusive_ptr<ConvolutionFilter_as> ptr =
        ensureType<ConvolutionFilter_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl(__PRETTY_FUNCTION__);
    return as_value();
}

as_value
convolutionfilter_divisor(const fn_call& fn)
{
    // TODO: check whether this is necessary (probably is).
    boost::intrusive_ptr<ConvolutionFilter_as> ptr =
        ensureType<ConvolutionFilter_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl(__PRETTY_FUNCTION__);
    return as_value();
}

as_value
convolutionfilter_bias(const fn_call& fn)
{
    // TODO: check whether this is necessary (probably is).
    boost::intrusive_ptr<ConvolutionFilter_as> ptr =
        ensureType<ConvolutionFilter_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl(__PRETTY_FUNCTION__);
    return as_value();
}

as_value
convolutionfilter_preserveAlpha(const fn_call& fn)
{
    // TODO: check whether this is necessary (probably is).
    boost::intrusive_ptr<ConvolutionFilter_as> ptr =
        ensureType<ConvolutionFilter_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl(__PRETTY_FUNCTION__);
    return as_value();
}

as_value
convolutionfilter_clamp(const fn_call& fn)
{
    // TODO: check whether this is necessary (probably is).
    boost::intrusive_ptr<ConvolutionFilter_as> ptr =
        ensureType<ConvolutionFilter_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl(__PRETTY_FUNCTION__);
    return as_value();
}

as_value
convolutionfilter_color(const fn_call& fn)
{
    // TODO: check whether this is necessary (probably is).
    boost::intrusive_ptr<ConvolutionFilter_as> ptr =
        ensureType<ConvolutionFilter_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl(__PRETTY_FUNCTION__);
    return as_value();
}

as_value
convolutionfilter_alpha(const fn_call& fn)
{
    // TODO: check whether this is necessary (probably is).
    boost::intrusive_ptr<ConvolutionFilter_as> ptr =
        ensureType<ConvolutionFilter_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl(__PRETTY_FUNCTION__);
    return as_value();
}

as_value
convolutionfilter_matrix(const fn_call& fn)
{
    // TODO: check whether this is necessary (probably is).
    boost::intrusive_ptr<ConvolutionFilter_as> ptr =
        ensureType<ConvolutionFilter_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl(__PRETTY_FUNCTION__);
    return as_value();
}

} // anonymous namespace
} // namespace gnash
