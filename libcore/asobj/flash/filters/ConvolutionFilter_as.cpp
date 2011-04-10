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

#include <boost/intrusive_ptr.hpp>

#include "ConvolutionFilter_as.h"

#include "as_object.h"
#include "VM.h"
#include "Global_as.h"
#include "BitmapFilter_as.h"
#include "Filters.h"

namespace gnash {

namespace {
    as_value convolutionfilter_new(const fn_call& fn);
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

/// TODO: should this inherit from BitmapFilter_as (relay)? This might
/// make cloning easier, but needs some testing first.
class ConvolutionFilter_as : public Relay, public ConvolutionFilter
{
public:
    ConvolutionFilter_as() {}
};

/// The prototype of flash.filters.ConvolutionFilter is a new BitmapFilter.
void
convolutionfilter_class_init(as_object& where, const ObjectURI& uri)
{
    registerBitmapClass(where, convolutionfilter_new,
            attachConvolutionFilterInterface, uri);
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
convolutionfilter_matrixX(const fn_call& fn)
{
    ConvolutionFilter_as* ptr = ensure<ThisIsNative<ConvolutionFilter_as> >(fn);
    UNUSED(ptr);
    log_unimpl(__PRETTY_FUNCTION__);
    return as_value();
}

as_value
convolutionfilter_matrixY(const fn_call& fn)
{
    ConvolutionFilter_as* ptr = ensure<ThisIsNative<ConvolutionFilter_as> >(fn);
    UNUSED(ptr);
    log_unimpl(__PRETTY_FUNCTION__);
    return as_value();
}

as_value
convolutionfilter_divisor(const fn_call& fn)
{
    ConvolutionFilter_as* ptr = ensure<ThisIsNative<ConvolutionFilter_as> >(fn);
    UNUSED(ptr);
    log_unimpl(__PRETTY_FUNCTION__);
    return as_value();
}

as_value
convolutionfilter_bias(const fn_call& fn)
{
    ConvolutionFilter_as* ptr = ensure<ThisIsNative<ConvolutionFilter_as> >(fn);
    UNUSED(ptr);
    log_unimpl(__PRETTY_FUNCTION__);
    return as_value();
}

as_value
convolutionfilter_preserveAlpha(const fn_call& fn)
{
    ConvolutionFilter_as* ptr = ensure<ThisIsNative<ConvolutionFilter_as> >(fn);
    UNUSED(ptr);
    log_unimpl(__PRETTY_FUNCTION__);
    return as_value();
}

as_value
convolutionfilter_clamp(const fn_call& fn)
{
    ConvolutionFilter_as* ptr = ensure<ThisIsNative<ConvolutionFilter_as> >(fn);
    UNUSED(ptr);
    log_unimpl(__PRETTY_FUNCTION__);
    return as_value();
}

as_value
convolutionfilter_color(const fn_call& fn)
{
    ConvolutionFilter_as* ptr = ensure<ThisIsNative<ConvolutionFilter_as> >(fn);
    UNUSED(ptr);
    log_unimpl(__PRETTY_FUNCTION__);
    return as_value();
}

as_value
convolutionfilter_alpha(const fn_call& fn)
{
    ConvolutionFilter_as* ptr = ensure<ThisIsNative<ConvolutionFilter_as> >(fn);
    UNUSED(ptr);
    log_unimpl(__PRETTY_FUNCTION__);
    return as_value();
}

as_value
convolutionfilter_matrix(const fn_call& fn)
{
    ConvolutionFilter_as* ptr = ensure<ThisIsNative<ConvolutionFilter_as> >(fn);
    UNUSED(ptr);
    log_unimpl(__PRETTY_FUNCTION__);
    return as_value();
}

as_value
convolutionfilter_new(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);
    obj->setRelay(new ConvolutionFilter_as);
    return as_value();
}


} // anonymous namespace
} // namespace gnash
