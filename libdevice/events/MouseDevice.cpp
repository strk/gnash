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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <boost/shared_array.hpp>

#include "GnashSleep.h"
#include "log.h"
#include "InputDevice.h"

namespace gnash {

static const char *MOUSE_DEVICE = "/dev/input/mice";

MouseDevice::MouseDevice()
    : _previous_x(0),
      _previous_y(0)
{
    // GNASH_REPORT_FUNCTION;
}

MouseDevice::~MouseDevice()
{
    // GNASH_REPORT_FUNCTION;
    if (_type == InputDevice::UMOUSE) {
        if (ioctl(_fd, UI_DEV_DESTROY) < 0) {
            log_error("ioctl(UI_DEV_DESTROY)");
        }
    }
}

std::vector<boost::shared_ptr<InputDevice> >
MouseDevice::scanForDevices()
{
    // GNASH_REPORT_FUNCTION;

    struct stat st;

    std::vector<boost::shared_ptr<InputDevice> > devices;

    // Look for these files for mouse input
    struct mouse_types {
        InputDevice::devicetype_e type;
        const char *filespec;
    };

    // Debug strings to make output more readable
    const char *debug[] = {
        "UNKNOWN",
        "Keyboard",
        "User Mode Mouse",
        "PS/2 Mouse",
        "Touchscreen",
        "Touchscreen Mouse",
        "Power Button",
        "Sleep Button",
        "Serial-USB Adapter",
        "Infrared Receiver"
    };    

    struct mouse_types mice[] = {
        {InputDevice::UMOUSE, "/dev/uinput"},          // User mode Mouse
        {InputDevice::UMOUSE, "/dev/input/uinput"},    // User mode Mouse
        {InputDevice::MOUSE, "/dev/input/mice"},       // PS/2 Mouse
#ifdef MULTIPLE_DEVICES
        {InputDevice::MOUSE, "/dev/input/mouse0"},
        {InputDevice::MOUSE, "/dev/input/mouse1"},
        {InputDevice::MOUSE, "/dev/usb/tkpanel0"},    // eTurboTouch touchscreen
#endif
        {InputDevice::UNKNOWN, 0}
    };

    int i = 0;
    while (mice[i].type != InputDevice::UNKNOWN) {
        int fd = 0;
        if (stat(mice[i].filespec, &st) == 0) {
            // Then see if we can open it
            if ((fd = open(mice[i].filespec, O_RDWR)) < 0) {
                log_error("You don't have the proper permissions to open %s",
                          mice[i].filespec);
                i++;
                continue;
            }
            close(fd);
            log_debug("Found a %s device for mouse input using %s",
                      debug[mice[i].type], mice[i].filespec);
            
            boost::shared_ptr<InputDevice> dev;
#if defined(USE_MOUSE_PS2) || defined(USE_MOUSE_ETT)
            dev = boost::shared_ptr<InputDevice>(new MouseDevice());
            dev->setType(mice[i].type);
            if (dev->init(mice[i].filespec, DEFAULT_BUFFER_SIZE)) {
                devices.push_back(dev);
            }
            // dev->dump();
#endif
#if 0
            // The User mode Mouse driver is preferred over the others.
            if ((mice[i].type = InputDevice::UMOUSE)) {
                log_debug("Defaulting to User Mode Mouse support");
                return devices;
            }
#endif
        }     // stat()
        
        i++;
    }         // while()
    
    return devices;
}

bool
MouseDevice::init()
{
    // GNASH_REPORT_FUNCTION;

    return init(MOUSE_DEVICE, DEFAULT_BUFFER_SIZE);
}

bool
MouseDevice::init(const std::string &filespec, size_t size)
{
    // GNASH_REPORT_FUNCTION;

    _filespec = filespec;
    _buffer.reset(new boost::uint8_t[size]);

    // see http://www.computer-engineering.org/ps2mouse/ 
    
    // Try to open mouse device, be error tolerant (FD is kept open all the time
    if (_type == InputDevice::UMOUSE) {
        _fd = open(filespec.c_str(), O_WRONLY | O_NDELAY);       
        if (initUMouse()) {
            moveToUMouse(400, 400);
            return true;
        }
    } else {
        _fd = open(filespec.c_str(), O_RDWR);
    
        if (_fd < 0) {
            log_debug("Could not open %s: %s", filespec, strerror(errno));
            return false;
        }
        
        if (fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL) | O_NONBLOCK) < 0) {
            log_error("Could not set non-blocking mode for mouse device: %s", strerror(errno));
            close(_fd);
            _fd = -1;
            return false; 
        }
    }
    
    // Clear input buffer
    unsigned char buf[10], byte;
    while (read(_fd, buf, size) > 0 ) { }

    // A touchscreen works similar to a Mouse, but not exactly...
    if (_type == InputDevice::MOUSE) {
        // Reset mouse
        if ((!command(0xFF, buf, 3)) || (buf[0] != 0xFA)) {
            log_debug(_("Mouse reset failed"));
            close(_fd);
            _fd = -1;
            return false; 
        }
        
        // Get Device ID (not crucial, debug only)
        if ((!command(0xF2, buf, 2)) || (buf[0] != 0xFA)) {
            log_debug(_("WARNING: Could not detect mouse device ID"));
        } else {
            unsigned char devid = buf[1];
            if (devid != 0)
                log_debug(_("WARNING: Non-standard mouse device ID %d"), devid);
        }
        
        // Enable mouse data reporting
        if ((!command(0xF4, &byte, 1)) || (byte != 0xFA)) {
            log_debug(_("Could not activate Data Reporting mode for mouse"));
            close(_fd);
            _fd = -1;
            return false; 
        }  
        
        log_debug("Mouse enabled for %s on fd #%d", _filespec, _fd);
    }
      
    return true;
}

bool
MouseDevice::initUMouse()
{
    GNASH_REPORT_FUNCTION;

    _type = UMOUSE;

    struct uinput_user_dev uidev;
    
    if (ioctl(_fd, UI_SET_EVBIT, EV_KEY) < 0) {
        log_error("ioctl(UI_SET_EVBIT, EV_KEY)");
        return false;
    }
    if (ioctl(_fd, UI_SET_KEYBIT, BTN_MOUSE) < 0) {
        log_error("ioctl(UI_SET_KEYBIT, BTN_MOUSE)");
        return false;
    }
    if (ioctl(_fd, UI_SET_KEYBIT, BTN_LEFT) < 0) {
        log_error("ioctl(UI_SET_KEYBIT, BTN_LEFT))");
        return false;
    }
    if (ioctl(_fd, UI_SET_KEYBIT, BTN_RIGHT) < 0) {
        log_error("ioctl(UI_SET_KEYBIT, BTN_RIGHT)");
        return false;
    }
    if (ioctl(_fd, UI_SET_EVBIT, EV_REL) < 0) {
        log_error("ioctl(UI_SET_EVBIT, EV_REL)");
        return false;
    }
    if (ioctl(_fd, UI_SET_RELBIT, REL_X) < 0) {
        log_error("ioctl(UI_SET_RELBIT, REL_X)");
        return false;
    }
    if (ioctl(_fd, UI_SET_RELBIT, REL_Y) < 0) {
        log_error("ioctl( UI_SET_RELBIT, REL_Y)");
        return false;
    }
    if (ioctl(_fd, UI_SET_EVBIT, EV_ABS) < 0) {
        log_error("ioctl(UI_SET_EVBIT, EV_ABS)");
        return false;
    }
    if (ioctl(_fd, UI_SET_ABSBIT,ABS_X) < 0) {
        log_error("ioctl(UI_SET_ABSBIT,ABS_X)");
        return false;
    }
    if (ioctl(_fd, UI_SET_ABSBIT, ABS_Y) < 0) {
        log_error("ioctl(UI_SET_ABSBIT, ABS_Y)");
        return false;
    }

    for (int i=0; i<256; i++) {
        if (ioctl(_fd, UI_SET_KEYBIT, i) < 0) {
            log_error("ioctl(UI_SET_KEYBIT, i)");
            return false;
        }
    }
    
    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x1;
    uidev.id.product = 0x1;
    uidev.id.version = 1;

    uidev.absmin[ABS_X] = 0;
    uidev.absmax[ABS_X] = _screen_width;
    uidev.absfuzz[ABS_X] = 0;
    uidev.absflat[ABS_X] = 0;
    uidev.absmin[ABS_Y] = 0;
    uidev.absmax[ABS_Y] = _screen_height;
    uidev.absfuzz[ABS_Y] = 0;
    uidev.absflat[ABS_Y ] = 0;

    if (::write(_fd, (char *)&uidev, sizeof(uidev)) < 0) {
        log_error("write uidev");
        return false;
    }

    if (ioctl(_fd, UI_DEV_CREATE) < 0) {
        log_error("ioctl(UI_DEV_CREATED)");
        return false;
    }

    return true;
}

// From http://www.computer-engineering.org/ps2mouse
//
// PS/2 Mouse mouse data is always in a 3 byte packet that looks like this:
//
//  	       Bit 7       Bit 6         Bit 5        Bit 4      Bit 3     Bit 2    Bit 1   Bit 0
// Byte 1 | Y overflow | X overflow | Y sign bit | X sign bit | Always 1 | Middle | Right | Left
// Byte 2                               X movement
// Byte 3                               Y movement
//
// The movement values are 9-bit 2's complement integers, where the
// most significant bit appears as a "sign" bit in byte 1 of the
// movement data packet. Their value represents the mouse's offset
// relative to its position when the previous packet was sent, in
// units determined by the current resolution. The range of values
// that can be expressed is -255 to +255. If this range is exceeded,
// the appropriate overflow bit is set.
//
// Note that reporting is disabled by default. The mouse will not
// actually issue any movement data packets until it receives the
// "Enable Data Reporting" (0xF4) command. 
//
// Stream mode is the default operating mode, and is otherwise set
// using the "Set Stream Mode" (0xEA) command.
//
// In remote mode the mouse reads its inputs and updates its
// counters/flags at the current sample rate, but it does not
// automatically issue data packets when movement has
// occured. Instead, the host polls the mouse using the "Read Data"
// (0xEB) command. Upon receiving this command the mouse will issue a
// single movement data packet and reset its movement counters. 

// The mouse enters remote mode upon receiving the "Set Remote Mode"
// (0xF0) command.
bool
MouseDevice::check()
{
    // GNASH_REPORT_FUNCTION;

    if (_fd < 0) {
        return false;   // no mouse available
    }
    
    int xmove, ymove, btn;
    boost::shared_array<boost::uint8_t> buf;
    if (_type == InputDevice::TOUCHMOUSE) {
        // The eTurboTouch has a 4 byte packet
        buf = readData(4);
    } else {
        // PS/2 Mouse packets are always 3 bytes
        buf = readData(3);
    }
    
    if (!buf) {
        return false;
    }
    
    // resync
    if (!buf[0] & 8) { // bit 3 us always set in the first byte
        log_error("No sync in first byte!");
        return false;
    }

    // A Touchscreen works similar to a Mouse, but not exactly.
    // At least for the eTurboTouch, it has a different layout
    // in the packet for the location as it has an additional byte
    btn   = buf[0] & 0x7;
    if (_type == InputDevice::TOUCHMOUSE) {
        xmove = (buf[1] << 7) | (buf[2]);
        ymove = (buf[3] << 7) | (buf[4]);
        /*
          printf("touchscreen: %02x %02x %02x %02x %02x | status %d, pos: %d/%d\n",
          mouse_buf[0], mouse_buf[1], mouse_buf[2], mouse_buf[3], mouse_buf[4],   
          new_btn, new_x, new_y);
        */    
        
        xmove = static_cast<int>(((static_cast<double>(xmove)- 355) / (1702 - 355)
                                  * 1536 + 256));
        ymove = static_cast<int>(((static_cast<double>(ymove) - 482) / (1771 - 482)
                                  * 1536 + 256));
        // FIXME: don't calculate here, this should be done by the GUI
        xmove = xmove * _screen_width / 2048;
        ymove = (2048-ymove) * _screen_height / 2048;
    } else {                    // end of InputDevice::TOUCHMOUSE
        // PS/2 Mouse
        // The movement values are 9-bit 2's complement integers,
        // where the most significant bit appears as a "sign" bit in
        // byte 1 of the movement data packet. Their value represents
        // the mouse's offset relative to its position when the
        // previous packet was sent, in units determined by the
        // current resolution. The range of values that can be
        // expressed is -255 to +255. If this range is exceeded, the
        // appropriate overflow bit is set.
        // (from the manpage for the psm driver)
        // Byte 1
        //     bit 7  One indicates overflow in the vertical movement count.
        //     bit 6  One indicates overflow in the horizontal movement count.
        //     bit 5  Set if the vertical movement count is negative.
        //     bit 4  Set if the horizontal movement count is negative.
        //     bit 3  Always one.
        //     bit 2  Middle button status; set if pressed.  For devices without
        //             the middle button, this bit is always zero.
        //     bit 1  Right button status; set if pressed.
        //     bit 0  Left button status; set if pressed.
        // Byte 2  Horizontal movement count in two’s complement; -256 through 255.
        //     Note that the sign bit is in the first byte.
        // Byte 3  Vertical movement count in two’s complement; -256 through 255.
        //     Note that the sign bit is in the first byte.
        xmove = buf[1];
        ymove = buf[2];
    
        if (buf[0] & 0x10) {
            xmove = -(256-xmove);
        }
        if (buf[0] & 0x20) {
            ymove = -(256-ymove);
        }
        
        ymove *= -1; // vertical movement is upside-down
        
        log_debug(_("PS/2 Mouse: Xmove=%d, Ymove=%d,  Button %d"),
                  xmove, ymove, btn);
        
        // movement    
        _input_data.x += xmove;
        _input_data.y += ymove;
        
        if (_input_data.x < 0) {
            _input_data.x = 0;
        }
        if (_input_data.y < 0) {
            _input_data.y = 0;
        }
        // FIXME: this is a bit of a temporary hack. The last two
        // arguments are a range, so hardcoding them is safe for
        // now. In the future more conversion may be done, making this
        // then be incorrect.
        boost::shared_array<int> coords =
            InputDevice::convertAbsCoords(_input_data.x, _input_data.y,
                                          _screen_width, _screen_height);
            // MouseDevice::convertCoordinates(_input_data.x, _input_data.y,
            //                          _screen_width, _screen_height);
        log_debug(_("convert: Xin=%d, Yin=%d, Xout=%d, Yout=%d"),
                  _input_data.x, _input_data.y, coords[0],coords[1]);
        
        _input_data.x = coords[0];
        _input_data.y = coords[1];
    } // end of InputDevice::MOUSE
    
    log_debug(_("read mouse: X=%d, Y=%d, Btn: btn %d"), _input_data.x,
              _input_data.y, _input_data.button);
    addData(false, gnash::key::INVALID, 0, _input_data.x, _input_data.y);
    
    // button
    if (btn != _input_data.button) {
        _input_data.button = btn;
        addData(true, gnash::key::INVALID, 0, _input_data.x, _input_data.y);
        log_debug(_("mouse click! %d"), btn);
    }
    
    return true;
}

// Move the mouse cursor to a specified location
bool
MouseDevice::moveToUMouse(int x, int y)
{
    GNASH_REPORT_FUNCTION;

    struct input_event ev;
    
    memset(&ev, 0, sizeof(ev));
    gettimeofday(&ev.time, NULL);
    ev.type = EV_ABS;
    ev.code = ABS_X;
    ev.value = x;
    if (::write(_fd, &ev, sizeof(struct input_event)) < 0) {
        log_error("write ABS_X");
        return false;
    }
    
    gettimeofday(&ev.time, NULL);
    ev.code = SYN_REPORT;
    ev.type = EV_SYN;
    if (::write(_fd, &ev, sizeof(struct input_event)) < 0) {
        log_error("write SYN");
        return false;
    }

    gettimeofday(&ev.time, NULL);
    ev.type = EV_ABS;
    ev.code = ABS_Y;
    ev.value = y;
    if (::write(_fd, &ev, sizeof(struct input_event)) < 0) {
        log_error("write ABS_Y");
        return false;
    }

    gettimeofday(&ev.time, NULL);
    ev.code = SYN_REPORT;
    ev.type = EV_SYN;
    if (::write(_fd, &ev, sizeof(struct input_event)) < 0) {
        log_error("write SYN");
        return false;
    }

    ////////////////////////////
    gettimeofday(&ev.time, NULL);
    ev.type = EV_KEY;
    ev.code = BTN_LEFT;
    ev.value = 0;
    if (::write(_fd, &ev, sizeof(struct input_event)) < 0) {
        log_error("write SYN");
        return false;
    }
    gettimeofday(&ev.time, NULL);
    ev.type = EV_KEY;
    ev.code = BTN_LEFT;
    ev.value = 1;
    if (::write(_fd, &ev, sizeof(struct input_event)) < 0) {
        log_error("write SYN");
        return false;
    }
    
    gettimeofday(&ev.time, NULL);
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    if (::write(_fd, &ev, sizeof(struct input_event)) < 0) {
        log_error("write SYN");
        return false;
    }

    return true;
}   

bool
MouseDevice::command(unsigned char cmd, unsigned char *buf, int count)
{
    // GNASH_REPORT_FUNCTION;

    int n;
    
    // flush input buffer
    char trash[16];
    do {
        n = ::read(_fd, trash, sizeof trash);
        if (n > 0) 
            log_debug(_("mouse_command: discarded %d bytes from input buffer"), n);
    } while (n > 0);
    
    // send command
    if ( -1 == ::write(_fd, &cmd, 1) ) {
        return false;
    }

    // read response (if any)
    while (count > 0) {
        gnashSleep(250*1000); // 250 ms inter-char timeout (simple method)
        // TODO: use select() instead
        
        n = read(_fd, buf, count);
        if (n <= 0) {
            return false;
        }
        count -= n;
        buf += n;
    }
    
    return true;
    
} // command()

/// \brief. Mouse movements are relative to the last position, so
/// this method is used to convert from relative position to
/// the absolute position Gnash needs.
boost::shared_array<int>
MouseDevice::convertCoordinates(int x, int y, int width, int height)
{
//    GNASH_REPORT_FUNCTION;
    
    boost::shared_array<int> coords(new int[2]);
    
    if (x > width) {
        coords[0] = width;
    } else {
        coords[0] = x;
    }
    if (y > height) {
        coords[1] = height;
    } else {
        coords[1] = y;
    }

    return coords;
}

// end of namespace
}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
