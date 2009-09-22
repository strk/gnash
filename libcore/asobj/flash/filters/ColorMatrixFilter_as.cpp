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

namespace gnash {

namespace {
    as_value colormatrixfilter_new(const fn_call& fn);
    as_value colormatrixfilter_matrix(const fn_call& fn);

    void attachColorMatrixFilterInterface(as_object& o);
}

/// TODO: should this inherit from BitmapFilter_as (relay)? This might
/// make cloning easier, but needs some testing first.
class ColorMatrixFilter_as : public Relay, public ColorMatrixFilter
{
public:
    ColorMatrixFilter_as() {}
};

/// The prototype of flash.filters.ColorMatrixFilter is a new BitmapFilter.
void
colormatrixfilter_class_init(as_object& where, const ObjectURI& uri)
{
    Global_as* gl = getGlobal(where);
    string_table& st = getStringTable(where);

    as_function* ctor =
        gl->getMember(st.find("flash.filters.BitmapFilter")).to_as_function();
    
    as_object* proto;
    if (ctor) {
        fn_call::Args args;
        VM& vm = getVM(where);
        proto = ctor->constructInstance(as_environment(vm), args).get();
    }
    else proto = 0;

    as_object* cl = gl->createClass(colormatrixfilter_new, proto);
    attachColorMatrixFilterInterface(*proto);
    where.init_member(getName(uri) , cl, as_object::DefaultFlags,
            getNamespace(uri));
}

namespace {

void
attachColorMatrixFilterInterface(as_object& o)
{
    o.init_property("matrix", colormatrixfilter_matrix, 
        colormatrixfilter_matrix);
}

as_value
colormatrixfilter_matrix(const fn_call& fn)
{
    ColorMatrixFilter_as* ptr = ensureNativeType<ColorMatrixFilter_as>(fn.this_ptr);
    UNUSED(ptr);
    return as_value();
}

as_value
colormatrixfilter_new(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);
    obj->setRelay(new ColorMatrixFilter_as);
    return as_value();
}

}
}

