// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// 
//

/* $Id: ref_counted.h,v 1.2 2006/10/29 18:34:11 rsavoye Exp $ */

#ifndef GNASH_REF_COUNTED_H
#define GNASH_REF_COUNTED_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "container.h"
#include "smart_ptr.h"

namespace gnash {

/// \brief
/// For stuff that's tricky to keep track of w/r/t ownership & cleanup.
/// The only use for this class seems to be for putting derived
/// classes in smart_ptr
/// TODO: remove use of this base class in favor of using
/// boost::shared_ptr<> ???? boost::intrusive_ptr(?)
///

class DSOEXPORT ref_counted
{
private:
	mutable int		m_ref_count;
	mutable weak_proxy*	m_weak_proxy;
public:
	ref_counted()
	:
	m_ref_count(0),
	m_weak_proxy(0)
	{
	}
	virtual ~ref_counted()
	{
	assert(m_ref_count == 0);
	if (m_weak_proxy){
		m_weak_proxy->notify_object_died();
		m_weak_proxy->drop_ref();
		}
	}
	void	add_ref() const {
	assert(m_ref_count >= 0);
	m_ref_count++;
	}
	void	drop_ref() const {
	assert(m_ref_count > 0);
	m_ref_count--;
	if (m_ref_count <= 0){
		// Delete me!
		delete this;
		}
	}

	int	get_ref_count() const { return m_ref_count; }
	weak_proxy*	get_weak_proxy() const {
	
	assert(m_ref_count > 0);	// By rights, somebody should be holding a ref to us.
	if (m_weak_proxy == NULL)	// Host calls this to register a function for progress bar handling
					// during loading movies.

		{
		m_weak_proxy = new weak_proxy;
		m_weak_proxy->add_ref();
		}
	return m_weak_proxy;
	}
};

} // namespace gnash

#endif // GNASH_REF_COUNTED_H
