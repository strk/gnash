// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

/// \page interface Host Interface
///
/// The core Gnash libraries support a flexible way of interacting with
/// hosting applications. This is necessary for ActionScript execution, as
/// well as for user notifications.
///
/// 1. The hosting application may ignore any message without dangerous
///    side effects.
///
/// 2. A set of expected messages exist, which should be supported for
///    proper ActionScript compatibility.
///
/// 3. Users must return the exact type expected (see KnownEvent), as
///    no implicit conversion is used. This means, for instance, that
///    where a std::string is expected, a const char* may not be used.
///
/// 4. There is the possibility to add custom messages for use in
///    ActionScript extensions.
///
/// The rationale for the current design is that hosting applications
/// should be able to support as many of the expected messages types as
/// they choose using a single interface: gnash::HostInterface::call(). This
/// should support various types of argument and various types of
/// return, so that the different 
///
/// The drawback of this flexibility is that there is no compile-time
/// check for the correct use of the interface. Within the core
/// libraries, this host interface is accessed only through
/// gnash::movie_root::callInterface(),
/// ensuring that any mistakes in the hosting application are handled
/// safely and logged.

#ifndef GNASH_HOST_INTERFACE_H
#define GNASH_HOST_INTERFACE_H

#include <boost/variant.hpp>
#include <boost/any.hpp>
#include <string>
#include <iosfwd>

#include "dsodefs.h"

namespace gnash {

/// A custom form of communication with the host application.
//
/// This comprises a string and any type of argument.
class CustomMessage
{
public:
    explicit CustomMessage(const std::string& s,
            const boost::any& arg = boost::blank())
        :
        _name(s),
        _arg(arg)
    {}
    const std::string& name() const { return _name; }
    const boost::any& arg() const { return _arg; }
private:
    std::string _name;
    boost::any _arg;
};

/// Built-in forms of communication with the host application.
//
/// These messages should be supported for ActionScript compatibility.
class HostMessage
{
public:

    /// The messages that a hosting application should handle.
    //
    /// 
    enum KnownEvent {

        /// Whether to display a mouse pointer
        /// - Argument type: bool
        /// - Effects: show mouse if true, hide if false
        /// - Return: boolean, true if the mouse was visible before
        SHOW_MOUSE,

        /// Resize the stage
        /// - Argument type: std::pair<int, int>(x, y)
        /// - Effects: resize stage to x pixels by y pixels
        /// - Return: none
        RESIZE_STAGE,

        /// Update the stage
        /// - Argument type: none
        /// - Effects: update the stage
        /// - Return: none
        UPDATE_STAGE,

        /// Whether to show the menu or not
        /// - Argument type: bool 
        /// - Effects: show menu if true, hide if false
        /// - Return: none
        /// @todo   This is probably insufficient.
        SHOW_MENU,

        /// Whether to show in fullscreen or not
        /// - Argument type: movie_root::DisplayState
        /// - Effects: display fullscreen if movie_root::DISPLAYSTATE_FULLSCREEN,
        ///            otherwise normal
        /// - Return: none
        SET_DISPLAYSTATE,

        /// Set system clipboard
        /// - Argument type: std::string
        /// - Effects: set system clipboard
        /// - Return: none
        SET_CLIPBOARD,

        /// Get screen resolution
        /// - Argument type: none
        /// - Effects: get screen resolution
        /// - Return: std::pair<int, int>(x, y)
        SCREEN_RESOLUTION,

        /// Get screen DPI.
        /// - Argument type: none
        /// - Effects: get screen DPI
        /// - Return: double
        SCREEN_DPI,

        /// Get pixel aspect ratio
        /// - Argument type: none
        /// - Effects: return pixel aspect ratio
        /// - Return: double
        PIXEL_ASPECT_RATIO,

        /// Get player type
        /// - Argument type: none
        /// - Effects: return "Plugin" or "StandAlone"
        /// - Return: std::string
        PLAYER_TYPE,

        /// Get screen color
        /// - Argument type: none
        /// - Effects: return "Color" or something else
        /// - Return: std::string
        SCREEN_COLOR,

        /// Notify an error
        /// - Argument type: std::string
        /// - Effects: notify the user of an error
        /// - Return: none
        NOTIFY_ERROR,

        /// Ask a question
        /// - Argument type: std::string
        /// - Effects: get the answer to a question
        /// - Return: bool
        QUERY,

        /// @todo check if the following types are appropriate.
        EXTERNALINTERFACE_ISPLAYING,
        EXTERNALINTERFACE_PAN,
        EXTERNALINTERFACE_PLAY,
        EXTERNALINTERFACE_REWIND,
        EXTERNALINTERFACE_SETZOOMRECT,
        EXTERNALINTERFACE_STOPPLAY,
        EXTERNALINTERFACE_ZOOM
    };

    explicit HostMessage(KnownEvent e, const boost::any& arg = boost::blank())
        :
        _event(e),
        _arg(arg)
    {}

    KnownEvent event() const { return _event; }
    const boost::any& arg() const { return _arg; }

private:
    KnownEvent _event;
    boost::any _arg;
};

/// Abstract base class for FS handlers
class FsCallback
{
public:
    virtual void notify(const std::string& cmd, const std::string& arg) = 0;
    virtual ~FsCallback() {}
};

/// Abstract base class for hosting app handler
class HostInterface
{
public:

    virtual ~HostInterface() {}

    typedef boost::variant<HostMessage, CustomMessage> Message;

    /// Pass a message to the hosting application with an optional return
    //
    /// The core library should access this function through
    /// movie_root::callInterface() or movie_root::callInterface<>()
    //
    /// @param e    The message to pass
    /// @return     A return of any type. Both callers and users
    ///             should know the expected type.
    virtual boost::any call(const Message& e) = 0;

    /// Instruct the hosting application to exit.
    //
    /// The hosting application may ignore this: do not rely on it
    /// to exit the program.
    virtual void exit() = 0;

};

/// Stream a description of any host interface message type.
DSOEXPORT std::ostream& operator<<(std::ostream& os, const HostMessage& m);
DSOEXPORT std::ostream& operator<<(std::ostream& os, const CustomMessage& m);

/// Stream a description of an expected message.
DSOEXPORT std::ostream& operator<<(std::ostream& os, HostMessage::KnownEvent e);


} // namespace gnash

#endif
