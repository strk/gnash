// event_id.cpp:  static ActionScript events for Gnash.
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
//


#include "log.h"
#include "event_id.h"
#include "namedStrings.h"
#include "ObjectURI.h"

#include <map>
#include <string>
#include <algorithm>
#include <boost/format.hpp>
#include <cassert>
#include <boost/assign/list_of.hpp>

namespace gnash {

const std::string&
event_id::functionName() const
{
    typedef std::map<EventCode, std::string> EventFunctionNameMap;
    static const EventFunctionNameMap e = boost::assign::map_list_of
        (INVALID, "INVALID")
		(PRESS, "onPress")
		(RELEASE, "onRelease")
		(RELEASE_OUTSIDE, "onReleaseOutside")
		(ROLL_OVER, "onRollOver")
		(ROLL_OUT, "onRollOut")	
		(DRAG_OVER, "onDragOver")
		(DRAG_OUT, "onDragOut")	
		(KEY_PRESS, "onKeyPress")
		(INITIALIZE, "onInitialize")
		(LOAD, "onLoad")
		(UNLOAD, "onUnload")
		(ENTER_FRAME, "onEnterFrame")
		(MOUSE_DOWN, "onMouseDown")	
		(MOUSE_UP, "onMouseUp")
		(MOUSE_MOVE, "onMouseMove")
		(KEY_DOWN, "onKeyDown")
		(KEY_UP, "onKeyUp")	
		(DATA, "onData")
		(CONSTRUCT, "onConstruct");

    EventFunctionNameMap::const_iterator it = e.find(_id);
    assert(it != e.end());
    return it->second;
}

const ObjectURI&
event_id::functionURI() const
{
    typedef std::map<EventCode, ObjectURI> EventFunctionMap;

    // TODO: make this table non-static, as
    //       it contains string_table-dependent
    //       mutable entries
    //
    static const EventFunctionMap e = boost::assign::map_list_of
		(PRESS, NSV::PROP_ON_PRESS)
		(RELEASE, NSV::PROP_ON_RELEASE)
		(RELEASE_OUTSIDE, NSV::PROP_ON_RELEASE_OUTSIDE)
		(ROLL_OVER, NSV::PROP_ON_ROLL_OVER )
		(ROLL_OUT, NSV::PROP_ON_ROLL_OUT)
		(DRAG_OVER, NSV::PROP_ON_DRAG_OVER)
		(DRAG_OUT, NSV::PROP_ON_DRAG_OUT)
		(KEY_PRESS, NSV::PROP_ON_KEY_PRESS)
		(INITIALIZE, NSV::PROP_ON_INITIALIZE)
		(LOAD, NSV::PROP_ON_LOAD)
		(UNLOAD, NSV::PROP_ON_UNLOAD)
		(ENTER_FRAME, NSV::PROP_ON_ENTER_FRAME)
		(MOUSE_DOWN, NSV::PROP_ON_MOUSE_DOWN)
		(MOUSE_UP, NSV::PROP_ON_MOUSE_UP)
		(MOUSE_MOVE, NSV::PROP_ON_MOUSE_MOVE)
		(KEY_DOWN, NSV::PROP_ON_KEY_DOWN)
		(KEY_UP, NSV::PROP_ON_KEY_UP)
		(DATA, NSV::PROP_ON_DATA)
		(CONSTRUCT, NSV::PROP_ON_CONSTRUCT);

    EventFunctionMap::const_iterator it = e.find(_id);
    assert(it != e.end());
    return it->second;
}

bool
isKeyEvent(const event_id& e) 
{
	switch (e.id())
	{
		case event_id::KEY_DOWN:
		case event_id::KEY_PRESS:
		case event_id::KEY_UP:
			return true;
		default:
			return false;
	}
}

bool
isButtonEvent(const event_id& e)
{
	switch (e.id())
	{
		case event_id::PRESS:
		case event_id::RELEASE:
		case event_id::RELEASE_OUTSIDE:
		case event_id::ROLL_OVER:
		case event_id::ROLL_OUT:
		case event_id::DRAG_OVER:
		case event_id::DRAG_OUT:
		case event_id::KEY_PRESS:
			return true;
		default:
			return false;
	}
}

std::ostream& operator<< (std::ostream& o, const event_id& ev)
{
    return (o << ev.functionName());
}

} // end of namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
