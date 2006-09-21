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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//

/* $Id: ref_counted.h,v 1.4 2006/09/21 06:15:15 nihilus Exp $ */

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
/// boost::shared_ptr<>
///
class DSOEXPORT ref_counted
{
private:
	mutable int	m_ref_count;
	mutable weak_proxy*	m_weak_proxy;
public:
	ref_counted();
	virtual ~ref_counted();
	void	add_ref() const;
	void	drop_ref() const;
	int	get_ref_count() const { return m_ref_count; }
	weak_proxy*	get_weak_proxy() const;
};

} // namespace gnash

#endif // GNASH_REF_COUNTED_H
