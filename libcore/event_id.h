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
//


#ifndef GNASH_EVENT_ID_H
#define GNASH_EVENT_ID_H

#include <string>
#include "GnashKey.h"

// Forward declarations
namespace gnash {
    struct ObjectURI;
}

namespace gnash {


/// A class to identify 'static' SWF events (system events).
//
/// The event process in a SWF comprises the raising of the event itself and
/// its receipt by a handler. Events may be either dynamically or statically
/// defined. A dynamic event is handled in ActionScript: an AS-defined
/// function is called when the event is raised. Dynamic events do not need
/// event_id.
//
/// Event handlers may also be defined statically, for instance in a
/// PlaceObject2 tag, or by default. System events such as mouse handling,
/// load milestones, or keyboard events should be sent to appropriate
/// DisplayObjects. This process uses event_id.
//
/// Static events may additionally be handled dynamically (using ActionScript).
//
/// The event_id class is used as an identifier for actual events and
/// and for the signature of an expected event.
class event_id
{
public:

    /// The types of events that are handled by DisplayObjects.
    enum EventCode
    {
        INVALID,

        // These are for buttons and sprites.
        PRESS,
        RELEASE,
        RELEASE_OUTSIDE,
        ROLL_OVER,
        ROLL_OUT,
        DRAG_OVER,
        DRAG_OUT,
        KEY_PRESS,

        // These are for sprites only.
        INITIALIZE,
        LOAD,
        UNLOAD,
        ENTER_FRAME,
        MOUSE_DOWN,
        MOUSE_UP,
        MOUSE_MOVE,
        KEY_DOWN,
        KEY_UP,
        DATA,
        CONSTRUCT
    };
    
    /// Construct an invalid event_id.
    //
    /// This is not useful until its values have been set.
    event_id()
        :
        _id(INVALID),
        _keyCode(key::INVALID)
    {}

    /// Construct an event_id.
    //
    /// @param id       The type of event
    /// @param c        The key associated with an event (only if this
    ///                 is a keyboard event).
    explicit event_id(EventCode id, key::code c = key::INVALID)
        :
        _id(id),
        _keyCode(c)
    {
        // We do have a testcase with _id == KEY_PRESS,
        // and keyCode==0(KEY_INVALID)
        // see key_event_test.swf(produced by Ming)
    }

    /// Set the key associated with this event.
    //
    /// @param SWFKey   The SWF code matched to the event. This
    ///                 must be converted to a unique gnash::key::code.
    void setKeyCode(boost::uint8_t SWFkey)
    {
        // Lookup the SWFcode in the gnash::key::code table.
        // Some are not unique (keypad numbers are the
        // same as normal numbers), so we take the first match.
        // As long as we can work out the SWFCode from the
        // gnash::key::code it's all right.
        int i = 0;
        while (key::codeMap[i][key::SWF] != SWFkey && i < key::KEYCOUNT) i++;

        if (i == key::KEYCOUNT) _keyCode = key::INVALID;
        else _keyCode = static_cast<key::code>(i);
    }

    /// Return the name of a method-handler function
    /// corresponding to this event.
    const std::string& functionName() const;

    /// Return the ObjectURI of a method-handler function
    /// corresponding to this event.
    const ObjectURI& functionURI() const;
    
    /// Return the keycode associated with this event_id.
    //
    /// This should be key::INVALID if the event_id is not a keyboard event.
    key::code keyCode() const { return _keyCode; }

    /// Return the identifier for this event type.
    EventCode id() const { return _id; }

private:

    EventCode _id;
    
    // keyCode must be the unique gnash key identifier
    // gnash::key::code.
    // TextField has to be able to work out the
    // ASCII value from keyCode, while other users need 
    // the SWF code or the Flash key code.
    key::code _keyCode;


};

/// Return whether two event_ids are equal
//
/// event_ids are equal if both id and keycode match. Keycode is only
/// relevant for keyboard events, and must be key::INVALID for other
/// event types.
inline bool
operator==(const event_id& a, const event_id& b)
{
    return a.id() == b.id() && a.keyCode() == b.keyCode();
}

/// Comparator for use in stdlib containers.
inline bool
operator<(const event_id& a, const event_id& b)
{
    if (a.id() == b.id()) return a.keyCode() < b.keyCode();
    return a.id() < b.id();
}


/// Check whether an event is a button-like event.
//
/// @param e        The event to check
/// @return         True if it is
bool isButtonEvent(const event_id& e);

/// Check whether an event is a keyboard event.
//
/// @param e        The event to check
/// @return         True if it is
bool isKeyEvent(const event_id& e);

std::ostream& operator<< (std::ostream& o, const event_id& ev);

} // namespace gnash


#endif 


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
