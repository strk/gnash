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

#ifndef GNASH_INPUTDEVICE_H
#define GNASH_INPUTDEVICE_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/scoped_array.hpp>
#include <boost/shared_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

#include "gui.h"

#include <linux/input.h>

namespace gnash {

// Define if you want to support multiple input devices of the same type.
// The default is to support the devices we prefer for mouse, keyboard,
// and touchscreen.
// #define MULTIPLE_DEVICES 1

// Forward declarations
class Gui;

// If we have a mouse, but the size isn't specified, then this is the
// default size.
static const int DEFAULT_BUFFER_SIZE = 256;

// This is an InputDevice class to cover the various touchscreens, Mice, or
// keyboards supported.
class InputDevice
{
public:
    typedef enum {
        UNKNOWN,
        KEYBOARD,
        MOUSE,
        TOUCHSCREEN,
        TOUCHMOUSE,
        POWERBUTTON,
        SLEEPBUTTON,
        SERIALUSB,
        INFRARED
    } devicetype_e;
    InputDevice();
    InputDevice(Gui *gui);
    virtual ~InputDevice();
    
    virtual bool init();
    bool init(devicetype_e type);
    bool init(devicetype_e type, size_t size);
    bool init(devicetype_e type, const std::string &filespec);
    bool init(devicetype_e type, const std::string &filespec, size_t size);
    virtual bool init(const std::string &filespec, size_t size) = 0;
    virtual bool check() = 0;

    static std::vector<boost::shared_ptr<InputDevice> > scanForDevices(Gui *gui);

    InputDevice::devicetype_e getType() const { return _type; };

    // Read data into the Device input buffer.
    boost::shared_array<boost::uint8_t> readData(size_t size);

    void dump() const;
    
protected:
    devicetype_e        _type;
    std::string         _filespec;
    int                 _fd;
    int                 _x;
    int                 _y;
    // Touchscreens don't have buttons
    int                 _button;
    size_t              _position;
    boost::scoped_array<boost::uint8_t> _buffer;
    // We don't control the memory associated with the Gui, we just use
    // it to propogate the events from this device.
    Gui                 *_gui;
};

class MouseDevice : public InputDevice
{
public:    
    MouseDevice();
    MouseDevice(Gui *gui);
    virtual bool init();
    virtual bool init(const std::string &filespec, size_t size);
    virtual bool check();

    static std::vector<boost::shared_ptr<InputDevice> > scanForDevices(Gui *gui);
    
    /// Sends a command to the mouse and waits for the response
    bool command(unsigned char cmd, unsigned char *buf, int count);
};

class TouchDevice : public InputDevice
{
public:
    TouchDevice();
    TouchDevice(Gui *gui);
    virtual ~TouchDevice();
    virtual bool init();
    virtual bool init(const std::string &filespec, size_t size);
    virtual bool check();

    void apply_ts_calibration(float* cx, float* cy, int rawx, int rawy);
    
    static std::vector<boost::shared_ptr<InputDevice> > scanForDevices(Gui *gui);
private:
    // Although the value is only set when using a touchscreen, it takes up little
    // memory to initialize a pointer to avoid lots of messy ifdefs.
    struct tsdev *_tsDev;
};

class EventDevice : public InputDevice
{
public:    
    EventDevice();
    EventDevice(Gui *gui);
    virtual bool init();
    virtual bool init(const std::string &filespec, size_t size);
    virtual bool check();

    gnash::key::code scancode_to_gnash_key(int code, bool shift);

    // This looks for all the input event devices.
    static std::vector<boost::shared_ptr<InputDevice> > scanForDevices(Gui *gui);
    
private:
    // Keyboard SHIFT/CTRL/ALT states (left + right)
     bool keyb_lshift, keyb_rshift, keyb_lctrl, keyb_rctrl, keyb_lalt, keyb_ralt;
    struct input_id _device_info;
};

} // end of gnash namespace

// end of GNASH_INPUTDEVICE_H
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
