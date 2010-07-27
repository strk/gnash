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
#include <boost/scoped_ptr.hpp>
#include <boost/cstdint.hpp>

#include "gui.h"

namespace gnash {

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
        PS2_MOUSE,
        TOUCHSCREEN
    } devicetype_e;
    InputDevice();
    ~InputDevice();
    
    bool init(devicetype_e type);
    bool init(devicetype_e type, size_t size);
    bool init(devicetype_e type, const std::string &filespec);
    bool init(devicetype_e type, const std::string &filespec, size_t size);
    virtual bool init(const std::string &filespec, size_t size) = 0;
    virtual bool check() = 0;

    // Read data into the Device input buffer.
    int readData();
    
    /// Attach the GUI to this device for propogating events.
    void attachGUI(Gui *gui) { _gui.reset(gui); };

protected:
    devicetype_e        _type;
    std::string         _filespec;
    int                 _fd;
    int                 _x;
    int                 _y;
    // Touchscreens don't have buttons
    int                 _button;
    boost::scoped_array<boost::uint8_t> _buffer;
    boost::scoped_ptr<Gui> _gui;
    size_t              _position;
};

class MouseDevice : public InputDevice
{
    virtual bool init(const std::string &filespec, size_t size);

    virtual bool check();

    /// Sends a command to the mouse and waits for the response
    bool command(unsigned char cmd, unsigned char *buf, int count);
};

class TouchDevice : public InputDevice
{
    virtual bool init(const std::string &filespec, size_t size);

    void apply_ts_calibration(float* cx, float* cy, int rawx, int rawy);

    virtual bool check();
};

class KeyboardDevice : public InputDevice
{
    virtual bool init(const std::string &filespec, size_t size);

    gnash::key::code scancode_to_gnash_key(int code, bool shift);
    
    virtual bool check();
};

} // end of gnash namespace

// end of GNASH_INPUTDEVICE_H
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
