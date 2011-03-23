// swf_event.h -- clip events (PlaceObject-defined)
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#ifndef GNASH_SWF_EVENT_H
#define GNASH_SWF_EVENT_H

#include "event_id.h" // for composition

namespace gnash {
    class action_buffer;
}

namespace gnash {

//
// swf_event
//

/// For embedding event handlers in place_object_2
//
/// TODO: move under parser dir !
///
class swf_event
{
public:

	swf_event(const event_id& ev, action_buffer& buf)
		:
		m_event(ev),
		m_action_buffer(buf)
	{
	}

	swf_event(const swf_event& o)
		:
		m_event(o.m_event),
		m_action_buffer(o.m_action_buffer)
	{
	}

	const action_buffer& action() const {
		return m_action_buffer;
	}

	const event_id& event() const {
		return m_event;
	}

private:

	/// System event id
	event_id m_event;

	/// Action buffer associated with this event
	//
	/// The buffer is externally owned
	/// (by PlaceObject tag in this design)
	/// and may be shared between multiple swf_events
	///
	const action_buffer& m_action_buffer;

	/// Can't assign to an swf_event
	void operator=(const swf_event& /*s*/);
};


}	// end namespace gnash


#endif // GNASH_SWF_EVENT_H
