//
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#include <string>
#include <iostream>

#include "GnashKey.h" // key::code
#include "network.h"
#include "log.h"
#include "lirc.h"

using std::string;

namespace gnash {

// this number camne from the lirc irw program. If this size works for
// them, it should work for us.
const int LIRC_PACKET_SIZE = 128;
const int TIMEOUT = 10;
const int BUTTONSIZE = 10;

Lirc::Lirc() 
    : _sockname("/tmp/lircd"), _button(nullptr)
{
//    GNASH_REPORT_FUNCTION;
    _button = new char[BUTTONSIZE];
}

Lirc::~Lirc()
{
//    GNASH_REPORT_FUNCTION;
    if (_button != nullptr) {
	delete _button;
    }
    closeNet();
}

bool
Lirc::init()
{
//    GNASH_REPORT_FUNCTION;
    return connectSocket(_sockname);
}

bool
Lirc::init(const char *sockpath)
{
//    GNASH_REPORT_FUNCTION;
    _connected = connectSocket(sockpath);
    return _connected;
}

// Whenever lircd receives a IR signal it will broadcast the
// following string to each client:
// <code> <repeat count> <button name> <remote control name>
// 0000000000000003 1 PREV LIRCEMU
// 0000000000000006 1 NEXT LIRCEMU
// 0000000000000012 1 A LIRCEMU

gnash::key::code
Lirc::getKey()
{
//    GNASH_REPORT_FUNCTION;
    key::code key = gnash::key::INVALID;
    
    byte_t buf[LIRC_PACKET_SIZE];
    memset(buf, 0, LIRC_PACKET_SIZE);
    
    // read the data if there is any
    readNet(buf, LIRC_PACKET_SIZE, TIMEOUT);
    
    string packet = reinterpret_cast<char *>(buf);
    string::size_type space1 = packet.find(" ") +1;
    string::size_type space2 = packet.find(" ", space1) + 1;
    string::size_type space3 = packet.find(" ", space2) +1;

    string code_str = packet.substr(0, space1);
    string count_str = packet.substr(space1, space2-space1);    
    string button_str = packet.substr(space2,space3-space2);
    string control_str = packet.substr(space3);

    if (button_str[0] > 'A' && button_str[0] < 'Z') {
        std::cerr << "Character: " << button_str << std::endl;
        key = (gnash::key::code)button_str[0];
    }

    return key;
}

const char *
Lirc::getButton()
{
//    GNASH_REPORT_FUNCTION;
 
    byte_t buf[LIRC_PACKET_SIZE];
    memset(buf, 0, LIRC_PACKET_SIZE);
    
    // read the data if there is any
    readNet(buf, LIRC_PACKET_SIZE, TIMEOUT);
    
    string packet = reinterpret_cast<char *>(buf);
    string::size_type space1 = packet.find(" ") + 1;
    string::size_type space2 = packet.find(" ", space1) + 1;
    string::size_type space3 = packet.find(" ", space2) + 1;
    
    string button_str = packet.substr(space2, space3-space2-1);

    memset(_button, 0, BUTTONSIZE);
    strncpy(_button, button_str.c_str(), BUTTONSIZE);
    return _button;
}

} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
