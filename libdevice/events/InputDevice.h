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
#include <vector>
#include <queue>
#include <linux/input.h>

#include "GnashKey.h"

namespace gnash {

// Define if you want to support multiple input devices of the same type.
// The default is to support the devices we prefer for mouse, keyboard,
// and touchscreen.
// #define MULTIPLE_DEVICES 1

// If we have a mouse, but the size isn't specified, then this is the
// default size.
static const int DEFAULT_BUFFER_SIZE = 256;

// This is an InputDevice class to cover the various touchscreens, Mice, or
// keyboards supported.
class InputDevice
{
public:
    typedef struct {
        bool pressed;
        gnash::key::code key;
        int modifier;
        int x;
        int y;
        int button;
        int position;
        int pressure;
        int volumne;
        int distance;
        int rx;
        int ry;
        int rz;
        int throttle;
        int rudder;
        int gas;
        int brake;
        int tiltX;
        int tiltY;        
    } input_data_t;
    typedef enum {
        UNKNOWN,
        KEYBOARD,
        MOUSE,
        TABLET,
        TOUCHSCREEN,
        TOUCHMOUSE,
        POWERBUTTON,
        SLEEPBUTTON,
        SERIALUSB,
        INFRARED
    } devicetype_e;
    InputDevice();
    virtual ~InputDevice();

    virtual const char *id() = 0;
    
    virtual bool init();
    bool init(devicetype_e type);
    bool init(devicetype_e type, size_t size);
    bool init(devicetype_e type, const std::string &filespec);
    bool init(devicetype_e type, const std::string &filespec, size_t size);
    virtual bool init(const std::string &filespec, size_t size) = 0;
    virtual bool check() = 0;

    static std::vector<boost::shared_ptr<InputDevice> > scanForDevices();
    
    InputDevice::devicetype_e getType() { return _type; };

    // Read data into the Device input buffer.
    boost::shared_array<boost::uint8_t> readData(size_t size);
    boost::shared_ptr<input_data_t> popData()
    {
        boost::shared_ptr<InputDevice::input_data_t> input;
        if (_data.size()) {
            // std::cerr << "FIXME: " <<_data.size() << std::endl;
            input = _data.front();
            _data.pop();
        }
        return input;
    }

    static boost::shared_array<int> convertAbsCoords(int x, int y,
                                                     int width, int height);

    void dump();
protected:
    void addData(bool pressed, key::code key, int modifier, int x, int y);
    
    devicetype_e        _type;
    std::string         _filespec;
    int                 _fd;
    input_data_t        _input_data;
    // These hold the data queue
    boost::scoped_array<boost::uint8_t> _buffer;
    std::queue<boost::shared_ptr<input_data_t> > _data;
};

class MouseDevice : public InputDevice
{
public:
    MouseDevice();
    const char *id() { return "Mouse"; };
    bool init();
    bool init(const std::string &filespec, size_t size);
    bool check();

    static std::vector<boost::shared_ptr<InputDevice> > scanForDevices();
    
    /// Sends a command to the mouse and waits for the response
    bool command(unsigned char cmd, unsigned char *buf, int count);

    /// \brief. Mouse movements are relative to the last position, so
    /// this method is used to convert from relative position to
    /// the absolute position Gnash needs.
    static boost::shared_array<int> convertCoordinates(int x, int y,
                                                       int width, int height);
};

class TouchDevice : public InputDevice
{
public:
    const char *id() { return "TouchScreen"; };
    TouchDevice();
    virtual ~TouchDevice();
    bool init();
    bool init(const std::string &filespec, size_t size);
    bool check();

    void apply_ts_calibration(float* cx, float* cy, int rawx, int rawy);
    
    static std::vector<boost::shared_ptr<InputDevice> > scanForDevices();
private:
    // Although the value is only set when using a touchscreen, it takes up little
    // memory to initialize a pointer to avoid lots of messy ifdefs.
    struct tsdev *_tsDev;
};

class EventDevice : public InputDevice
{
public:
    EventDevice();
    const char *id() { return "InputEvent"; };
    virtual bool init();
    virtual bool init(const std::string &filespec, size_t size);
    virtual bool check();

    gnash::key::code scancode_to_gnash_key(int code, bool shift);

    // This looks for all the input event devices.
    static std::vector<boost::shared_ptr<InputDevice> > scanForDevices();
    
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
