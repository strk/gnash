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

#include "HostInterface.h"

#include <boost/variant.hpp>
#include <boost/any.hpp>
#include <string>
#include <ostream>

namespace gnash {

std::ostream&
operator<<(std::ostream& os, const HostMessage& m)
{
    return os << m.event();
}

std::ostream&
operator<<(std::ostream& os, const CustomMessage& m)
{
    return os << m.name();
}

std::ostream&
operator<<(std::ostream& os, HostMessage::KnownEvent e)
{
    struct Wrapper {
        Wrapper(std::ostream& os) : _os(os << "<") {}
        ~Wrapper() { _os << ">"; }
        std::ostream& _os;
    } a(os);

    switch (e) {
        case HostMessage::SHOW_MOUSE:
            return os << "show mouse";
        case HostMessage::RESIZE_STAGE:
            return os << "resize stage";
        case HostMessage::SHOW_MENU:
            return os << "show menu";
        case HostMessage::UPDATE_STAGE:
            return os << "update stage";
        case HostMessage::SET_DISPLAYSTATE:
            return os << "set display state";
        case HostMessage::SET_CLIPBOARD:
            return os << "set clipboard";
        case HostMessage::SCREEN_RESOLUTION:
            return os << "screen resolution";
        case HostMessage::SCREEN_DPI:
            return os << "screen DPI";
        case HostMessage::PIXEL_ASPECT_RATIO:
            return os << "pixel aspect ratio";
        case HostMessage::PLAYER_TYPE:
            return os << "player type";
        case HostMessage::SCREEN_COLOR:
            return os << "screen color";
        case HostMessage::EXTERNALINTERFACE_ISPLAYING:
            return os << "ExternalInterface.isPlaying";
        case HostMessage::EXTERNALINTERFACE_PAN:
            return os << "ExternalInterface.pan";
        case HostMessage::EXTERNALINTERFACE_PLAY:
            return os << "ExternalInterface.play";
        case HostMessage::EXTERNALINTERFACE_REWIND:
            return os << "ExternalInterface.rewind";
        case HostMessage::EXTERNALINTERFACE_SETZOOMRECT:
            return os << "ExternalInterface.setZoomRect";
        case HostMessage::EXTERNALINTERFACE_STOPPLAY:
            return os << "ExternalInterface.stopPlay";
        case HostMessage::EXTERNALINTERFACE_ZOOM:
            return os << "ExternalInterface.zoom";
        default:
            return os << "Unknown event " << +e;
    }
}

} // namespace gnash

