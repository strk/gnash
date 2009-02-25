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
//

#ifndef __GNASH_ASOBJ_BITMAPFILTER_H__
#define __GNASH_ASOBJ_BITMAPFILTER_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

namespace gnash {

class as_object;

/// Initialize the global BitmapFilter class
void BitmapFilter_class_init(as_object& global);

/// Get the interface, for inheritance.
as_object *bitmapFilter_interface();

} // end of gnash namespace

// __GNASH_ASOBJ_BITMAPFILTER_H__
#endif

#ifdef phelp_helper
#ifndef easy_clone
#define easy_clone(sp_name) \
as_value \
sp_name::bitmap_clone(const fn_call& fn) \
{ \
    boost::intrusive_ptr<sp_name> ptr = ensureType<sp_name>(fn.this_ptr); \
    boost::intrusive_ptr<sp_name> obj = new sp_name(*ptr); \
    boost::intrusive_ptr<as_object> r = obj; \
    r->set_prototype(ptr->get_prototype()); \
    r->copyProperties(*ptr); \
\
    return as_value(r); \
}
#endif /* easy_clone */
#endif /* phelp_helper */
