// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#ifndef GNASH_REF_COUNTED_H
#define GNASH_REF_COUNTED_H

#include "dsodefs.h" // for DSOEXPORT

#include <cassert>
#include <boost/detail/atomic_count.hpp>

namespace gnash {

/// \brief
/// For stuff that's tricky to keep track of w/r/t ownership & cleanup.
/// The only use for this class seems to be for putting derived
/// classes in smart_ptr
///
class DSOEXPORT ref_counted
{

private:

	// We use an atomic counter to make ref-counting 
	// thread-safe. The drop_ref() method must be
	// carefully designed for this to be effective
	// (decrement & check in a single statement)
	//
	typedef boost::detail::atomic_count Counter;

	mutable Counter m_ref_count;
	
protected:

	// A ref-counted object only deletes self,
	// must never be explicitly deleted !
	virtual ~ref_counted()
	{
		assert(m_ref_count == 0);
	}

public:
	ref_counted()
		:
		m_ref_count(0)
	{
	}

	ref_counted(const ref_counted&)
		:
		m_ref_count(0)
	{
	}

	void add_ref() const {
		assert(m_ref_count >= 0);
		++m_ref_count;
	}

	void drop_ref() const {

		assert(m_ref_count > 0);
		if (!--m_ref_count) {
			// Delete me!
			delete this;
		}
	}

	long get_ref_count() const { return m_ref_count; }
};

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

} // namespace gnash

#endif // GNASH_REF_COUNTED_H
