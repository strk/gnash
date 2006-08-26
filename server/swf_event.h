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
//

#ifndef GNASH_SWF_EVENT_H
#define GNASH_SWF_EVENT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "as_value.h" // for composition
#include "action_buffer.h" // for composition
#include "action.h" // for event_id

#include <cassert>

namespace gnash {

//
// swf_event
//

/// For embedding event handlers in place_object_2
class swf_event
{
public:
    // NOTE: DO NOT USE THESE AS VALUE TYPES IN AN
    // std::vector<>!  They cannot be moved!  The private
    // operator=(const swf_event&) should help guard
    // against that.

    event_id	m_event;
    action_buffer	m_action_buffer;
    as_value	m_method;

    swf_event()
	{
	}

    void	attach_to(character* ch) const
	{
	    ch->set_event_handler(m_event, m_method);
	}

private:
    // DON'T USE THESE
    swf_event(const swf_event& /*s*/) { assert(0); }
    void	operator=(const swf_event& /*s*/) { assert(0); }
};


}	// end namespace gnash


#endif // GNASH_SWF_EVENT_H
