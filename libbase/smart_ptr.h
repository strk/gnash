// smart_ptr.h	-- by Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Smart (ref-counting) pointer classes.  Uses "intrusive" approach:
// the types pointed to must have add_ref() and drop_ref() methods.
// Typically this is done by inheriting from a ref_counted class,
// although the nice thing about templates is that no particular
// ref-counted class is mandated.

/* $Id: smart_ptr.h,v 1.15 2006/11/11 22:44:54 strk Exp $ */

#ifndef SMART_PTR_H
#define SMART_PTR_H

#include "tu_config.h"
#include "utility.h"

#include <boost/intrusive_ptr.hpp>

#define COMPILER_SUPPORTS_ARGUMENT_DEPENDENT_LOOKUP 1
#ifdef COMPILER_SUPPORTS_ARGUMENT_DEPENDENT_LOOKUP
namespace gnash {
#else
namespace boost {
#endif

template <class T>
void
intrusive_ptr_add_ref(T* o)
{
	o->add_ref();
}

template <class T>
void
intrusive_ptr_release(T* o)
{
	o->drop_ref();
}

}; 


#endif // SMART_PTR_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
