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

#define phelp_helper BitmapFilter_as
#define phelp_class BitmapFilter
#include "prophelper.h"

namespace gnash {

class BitmapFilter_as : public as_object, public BitmapFilter
{
public:
    phelp_i(bitmap_clone);

    virtual boost::intrusive_ptr<as_object> clone();

private:
    phelp_base_def;
};

phelp_base_imp( , BitmapFilter)

phelp_i_attach_begin
phelp_i_attach(clone, bitmap_clone);
phelp_i_attach_end

// Clone this object.
boost::intrusive_ptr<as_object> BitmapFilter_as::clone()
{
    boost::intrusive_ptr<as_object> o = new BitmapFilter_as(BitmapFilter_as::Interface());
    return o;
}

as_value
BitmapFilter_as::ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new BitmapFilter_as(BitmapFilter_as::Interface());
    return as_value(obj);
}

as_value BitmapFilter_as::bitmap_clone(const fn_call& fn)
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

} // Namespace gnash

