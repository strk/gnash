// 
//   Copyright (C) 2010 Free Software Foundation, Inc
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "gui.h"
#include "log.h"
#include "InputDevice.h"

namespace gnash {

InputDevice::InputDevice()
    : _type(InputDevice::UNKNOWN),
      _fd(-1),
      _x(0),
      _y(0),
      _button(0),
      _position(0),
      _gui(0)
{
    // GNASH_REPORT_FUNCTION;
}

InputDevice::InputDevice(Gui *gui)
    : _type(InputDevice::UNKNOWN),
      _fd(-1),
      _x(0),
      _y(0),
      _button(0),
      _position(0),
      _gui(gui)
{
    // GNASH_REPORT_FUNCTION;
}

InputDevice::~InputDevice()
{
    // GNASH_REPORT_FUNCTION;
}

bool
InputDevice::init()
{
    // GNASH_REPORT_FUNCTION;
    
    return true;
}

bool
InputDevice::init(InputDevice::devicetype_e type)
{
    GNASH_REPORT_FUNCTION;
    return init(type, std::string(), DEFAULT_BUFFER_SIZE);
}

bool
InputDevice::init(InputDevice::devicetype_e type, size_t size)
{
    GNASH_REPORT_FUNCTION;

    return init(type, std::string(), size);
}

bool
InputDevice::init(InputDevice::devicetype_e type, const std::string &filespec,
                  size_t size)
{
    GNASH_REPORT_FUNCTION;

    _type = type;
    _filespec = filespec;
    
    return init(filespec, size);
}

// Read data into the Device input buffer.
int
InputDevice::readData()
{
    GNASH_REPORT_FUNCTION;

    if (_fd < 0) {
        return 0;   // no mouse available
    }
    
    int count;  
    
    unsigned char *ptr;
    
    ptr = _buffer.get() + _position;
    
    count = read(_fd, _buffer.get() + _position, _position - DEFAULT_BUFFER_SIZE);
    
    if (count <= 0) {
        return count;
    }
    
    /*
      printf("read data: ");
      int i;
      for (i=0; i<count; i++) 
      printf("%02x ", ptr[i]);
      printf("\n");
    */
    
    _position += count;

    return count;
}   

void
InputDevice::dump()
{
    // Debug strings to make output more readable
    const char *debug[] = {
        "UNKNOWN",
        "Keyboard",
        "PS2 Mouse",
        "eTurboTouch Mouse",
        "Touchscreen",
        "Power Button"
    };    

    std::cerr << "Device type is: " << debug[_type] << std::endl;
    std::cerr << "\tfilespec is: " << _filespec
              << ", fd #" << _fd << std::endl;
    std::cerr << "\tX is: " << _x << ", Y is: " << _y << std::endl;
}

// Scan for all the possible input devices. This aggregates all
// the devices from each type into a single big vector.
std::vector<boost::shared_ptr<InputDevice> >
InputDevice::scanForDevices(Gui *gui)
{
    // GNASH_REPORT_FUNCTION;

    std::vector<boost::shared_ptr<InputDevice> > devices;
    
    std::vector<boost::shared_ptr<InputDevice> > id;
    std::vector<boost::shared_ptr<InputDevice> >::iterator it;
#ifdef USE_INPUT_EVENT
    id = EventDevice::scanForDevices(gui);
    for (it=id.begin(); it!=id.end(); ++it) {
        devices.push_back(*it);
    }
#endif
#if defined(USE_MOUSE_PS2) || defined(USE_MOUSE_ETT)
    id = MouseDevice::scanForDevices(gui);
    for (it=id.begin(); it!=id.end(); ++it) {
        devices.push_back(*it);
    }
#endif
#ifdef USE_TSLIB
    id = TouchDevice::scanForDevices(gui);
    for (it=id.begin(); it!=id.end(); ++it) {
        devices.push_back(*it);
    }
#endif

    return devices;
}

#ifdef USE_TSLIB
boost::shared_ptr<InputDevice>
createTSlibDevice(Gui *gui)
{
    return boost::shared_ptr<InputDevice>(new TouchDevice(gui));
}
#endif

#ifdef USE_INPUT_EVENT
boost::shared_ptr<InputDevice>
createEventDevice(Gui *gui)
{
    return boost::shared_ptr<InputDevice>(new EventDevice(gui));
}
#endif

// end of gnash namespace
}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
