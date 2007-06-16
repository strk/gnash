// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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

/* $Id: ref_counted.h,v 1.8 2007/06/16 07:59:19 strk Exp $ */

#ifndef GNASH_REF_COUNTED_H
#define GNASH_REF_COUNTED_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "container.h"
#include "smart_ptr.h"

#ifdef GNASH_USE_GC
# include "GC.h"
# ifdef GNASH_GC_DEBUG
#  include "log.h"
#  include <typeinfo>
# endif // GNASH_GC_DEBUG
#endif // GNASH_USE_GC

namespace gnash {

/// \brief
/// For stuff that's tricky to keep track of w/r/t ownership & cleanup.
/// The only use for this class seems to be for putting derived
/// classes in smart_ptr
///
#ifdef GNASH_USE_GC
class DSOEXPORT ref_counted : public GcResource
#else
class DSOEXPORT ref_counted
#endif
{

private:

#ifndef GNASH_USE_GC
	mutable int		m_ref_count;
#endif // ndef GNASH_USE_GC
	
protected:

	// A ref-counted object only deletes self,
	// must never be explicitly deleted !
	virtual ~ref_counted()
	{
#ifndef GNASH_USE_GC
		assert(m_ref_count == 0);
#endif // ndef GNASH_USE_GC
	}

public:
	ref_counted()
#ifndef GNASH_USE_GC
		:
		m_ref_count(0)
#endif // ndef GNASH_USE_GC
	{
	}

#ifndef GNASH_USE_GC
	void	add_ref() const
	{
		assert(m_ref_count >= 0);
		m_ref_count++;
	}

	void	drop_ref() const
	{
		assert(m_ref_count > 0);
		m_ref_count--;
		if (m_ref_count <= 0)
		{
			// Delete me!
			delete this;
		}
	}

	int	get_ref_count() const { return m_ref_count; }
#endif // ndef GNASH_USE_GC
};

} // namespace gnash

#endif // GNASH_REF_COUNTED_H
