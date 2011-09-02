// 
//   Copyright (C) 2010, 2011 Free Software Foundation, Inc
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

#include "log.h"
#include "InputDevice.h"
#include "GnashKey.h"
#include "iostream"

namespace gnash {

InputDevice::InputDevice()
    : _type(InputDevice::UNKNOWN),
      _fd(-1)
{
    // GNASH_REPORT_FUNCTION;
    memset(&_input_data, 0, sizeof(InputDevice::input_data_t));
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

void
InputDevice::addData(bool pressed, key::code key, int modifier, int x, int y)
{
    // GNASH_REPORT_FUNCTION;
    
    boost::shared_ptr<input_data_t> _newdata(new input_data_t);
    _newdata->pressed = pressed;
    _newdata->key = key;
    _newdata->modifier = modifier;
    _newdata->x = x;
    _newdata->y = y;
#if 0
    std::cerr << "Adding data: " << _newdata->pressed;
    std::cerr << ", " << _newdata->key << ", " << _newdata->modifier;
    std::cerr << ", " << _newdata->x << ", " << _newdata->y << std::endl;
#endif
    _data.push(_newdata);
}

// Read data into the Device input buffer.
boost::shared_array<boost::uint8_t>
InputDevice::readData(size_t size)
{
    // GNASH_REPORT_FUNCTION;

    boost::shared_array<boost::uint8_t> inbuf;

    if (_fd < 0) {
        return inbuf;   // no mouse available
    }

    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(_fd, &fdset);
    struct timeval tval;
    tval.tv_sec  = 0;
    tval.tv_usec = 1;
    errno = 0;
    int ret = ::select(_fd+1, &fdset, NULL, NULL, &tval);
    if (ret == 0) {
//            log_debug ("The pipe for fd #%d timed out waiting to read", fd);
        return inbuf;
    } else if (ret == 1) {
        // log_debug ("The device for fd #%d is ready", _fd);
    } else {
        log_error("The device has this error: %s", strerror(errno));
        return inbuf;
    }
    
    inbuf.reset(new boost::uint8_t[size]);
    ret = ::read(_fd, inbuf.get(), size);
    if (ret > 0) {
        // log_debug("Read %d bytes, %s", ret, hexify(inbuf.get(), ret, false));
    } else {
        inbuf.reset();
    }

    return inbuf;
}   

void
InputDevice::dump() const
{
    // Debug strings to make output more readable
    const char *debug[] = {
        "UNKNOWN",
        "Keyboard",
        "Mouse",
        "Tablet",
        "Touchscreen",
        "Touchscreen Mouse",
        "Power Button",
        "Sleep Button",
        "Serial-USB Adapter",
        "Infrared Receiver"
    };    

    std::cerr << "Device type is: " << debug[_type];
    std::cerr << ", \tfilespec is: " << _filespec
              << ", fd #" << _fd << std::endl;
//    std::cerr << "\tX is: " << _x << ", Y is: " << _y << std::endl;
}

// The Babbage touchscreen gives is absolute coordinates, but they don't
// match the actual screen resolution. So we convert the coordinates
// to a new absolute location.
// For example, if the LCD is 480 x 800, the tablet thinks this is 1010 x 960.
// This should really use a calibration function, but as we know the numbers...
boost::shared_array<int>
InputDevice::convertAbsCoords(int x, int y, int width, int height)
{
    boost::shared_array<int> coords(new int[2]);

    coords[0] = (x/width) * x;
    coords[1] = (y/height) * y;
    
    return coords;
}

std::vector<boost::shared_ptr<InputDevice> > 
InputDevice::scanForDevices()
{
    // GNASH_REPORT_FUNCTION;
    
    std::vector<boost::shared_ptr<InputDevice> > devices;
    
    std::vector<boost::shared_ptr<InputDevice> > id;
    std::vector<boost::shared_ptr<InputDevice> >::iterator it;
#ifdef USE_INPUT_EVENTS
    id = EventDevice::scanForDevices();
    for (it=id.begin(); it!=id.end(); ++it) {
        devices.push_back(*it);
    }
#endif
#if defined(USE_MOUSE_PS2) || defined(USE_MOUSE_ETT)
    id = MouseDevice::scanForDevices();
    for (it=id.begin(); it!=id.end(); ++it) {
        devices.push_back(*it);
    }
#endif
#if defined(HAVE_TSLIB_H) && defined(USE_TSLIB)
    id = TouchDevice::scanForDevices();
    for (it=id.begin(); it!=id.end(); ++it) {
        devices.push_back(*it);
    }
#endif    
    return devices;
}
    
// end of gnash namespace
}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
