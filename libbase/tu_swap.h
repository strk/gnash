// tu_swap.h	-- Ignacio Castaño, Thatcher Ulrich 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Basic swapping stuff and endian-dependent code.

#ifndef TU_SWAP_H
#define TU_SWAP_H

#include "tu_config.h"
//#include "tu_types.h"

template<class T>
void	swap(T* a, T* b)
// Convenient swap function.
{
	T	temp(*a);
	*a = *b;
	*b = temp;
}

#endif // TU_SWAP_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
