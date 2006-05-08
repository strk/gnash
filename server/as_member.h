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

#ifndef GNASH_AS_MEMBER_H
#define GNASH_AS_MEMBER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "as_value.h"
#include "as_prop_flags.h"

namespace gnash {


/// Member for as_object: value + flags
class as_member
{
	/// value
	as_value m_value;
	/// Properties flags
	as_prop_flags m_flags;

public:
	/// Default constructor
	as_member() {}

	/// Constructor
	as_member(const as_value &value,const as_prop_flags flags=as_prop_flags())
		:
		m_value(value),
		m_flags(flags)
	{
	}

	/// accessor to the value
	as_value get_member_value() const { return m_value; }

	/// accessor to the properties flags
	as_prop_flags get_member_flags() const { return m_flags; }

	/// set the value
	void set_member_value(const as_value &value)  { m_value = value; }

	/// accessor to the properties flags
	void set_member_flags(const as_prop_flags &flags)  { m_flags = flags; }
};


} // namespace gnash

#endif // GNASH_AS_MEMBER_H
