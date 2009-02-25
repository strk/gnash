//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

// Smart (ref-counting) pointer classes.  Uses "intrusive" approach:
// the types pointed to must have add_ref() and drop_ref() methods.
// Typically this is done by inheriting from a ref_counted class,
// although the nice thing about templates is that no particular
// ref-counted class is mandated.


#ifndef SMART_PTR_H
#define SMART_PTR_H

// Define the following macro to use garbage collecting rather
// then ref-counting. Currenlty this would make ref_counted
// derive from GcResource and intrusive_ptr never really messing
// with the stored pointer (not calling add_ref/drop_ref, which would
// be not defined for ref_counted.
// Is is a temporary hack to allow quick switch between GC and REFCOUNT
// mechanism till the GC is stable
//
#define GNASH_USE_GC 1

// TODO: if GNASH_USE_GC is defined have smart_ptr map to intrusive_ptr
//       else have it map to gc_ptr (yet to be defined)

#include "ref_counted.h"
#include "GC.h"

#include <boost/intrusive_ptr.hpp>
#include <typeinfo>

#define COMPILER_SUPPORTS_ARGUMENT_DEPENDENT_LOOKUP 1
#ifdef COMPILER_SUPPORTS_ARGUMENT_DEPENDENT_LOOKUP
namespace gnash {
#else
namespace boost {
#endif

inline void
intrusive_ptr_add_ref(const ref_counted* o)
{
	o->add_ref();
}

inline void
intrusive_ptr_release(const ref_counted* o)
{
	o->drop_ref();
}

// These two should not be needed when we switch all GcResource 
// pointers to use the gc_ptr instead of the intrusive_ptr

inline void intrusive_ptr_add_ref(const GcResource* ) { }
inline void intrusive_ptr_release(const GcResource* ) { }

#ifdef GNASH_USE_GC
class as_object;
inline void intrusive_ptr_add_ref(const as_object* ) { }
inline void intrusive_ptr_release(const as_object* ) { }
#endif

// The below thing won't work. We'll need a real templated class..
//template <typename C> typedef C* gc_ptr;

} 


#endif // SMART_PTR_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
