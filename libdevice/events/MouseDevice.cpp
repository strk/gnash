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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <boost/shared_array.hpp>

#include "GnashSleep.h"
#include "log.h"
#include "InputDevice.h"

namespace gnash {

static const char *MOUSE_DEVICE = "/dev/input/mice";

MouseDevice::MouseDevice()
{
    // GNASH_REPORT_FUNCTION;
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
        "Mouse",
        "Touchscreen",
        "Touchscreen Mouse",
        "Power Button",
        "Sleep Button",
        "Serial-USB Adapter",
        "Infrared Receiver"
    };    

    struct mouse_types mice[] = {
        {InputDevice::MOUSE, "/dev/input/mice"},      // PS/2 Mouse
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
            if (dev->init(mice[i].filespec, DEFAULT_BUFFER_SIZE)) {
                devices.push_back(dev);
            }
//            dev->dump();
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

    _type = MOUSE;
    _filespec = filespec;
    _buffer.reset(new boost::uint8_t[size]);

    // see http://www.computer-engineering.org/ps2mouse/ 
    
    // Try to open mouse device, be error tolerant (FD is kept open all the time)
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
    
    // Clear input buffer
    unsigned char buf[10], byte;
    while (read(_fd, buf, sizeof buf) > 0 ) { }

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
#if 0
        // FIXME: don't calculate here, this should be done by the GUI
        xmove = xmove * _gui->getStage()->getStageWidth() / 2048;
        ymove = (2048-ymove) * _gui->getStage()->getStageHeight() / 2048;
#endif
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
        xmove = buf[1];
        ymove = buf[2];
    
        if (buf[0] & 0x10) {
            xmove = -(256-xmove);
        }
        if (buf[0] & 0x20) {
            ymove = -(256-ymove);
        }
        
        ymove *= -1; // vertical movement is upside-down
        
        log_debug(_("x/y %d/%d button %d"), xmove, ymove, btn);
        
        // movement    
        _x += xmove;
        _y += ymove;
        
        if (_x < 0) {
            _x = 0;
        }
        if (_y < 0) {
            _y = 0;
        }
        // FIXME: this is a bit of a temporary hack. The last two
        // arguments are a range, so hardcoding them is safe for
        // now. In the future more conversion may be done, making this
        // then be incorrect.
        boost::shared_array<int> coords =
            MouseDevice::convertCoordinates(_x, _y, 1024, 768);
//          MouseDevice::convertCoordinates(_x, _y,
//                                 _gui->getStage()->getStageWidth(),
//                                 _gui->getStage()->getStageHeight());
        _x = coords[0];
        _y = coords[1];
    } // end of InputDevice::MOUSE
    
    log_debug(_("read mouse @ %d / %d, btn %d"), _x, _y, _button);
    addData(false, gnash::key::INVALID, 0, _x, _y);
    
    // button
    if (btn != _button) {
        _button = btn;
        log_debug("clicked: %d", btn); 
        addData(true, gnash::key::INVALID, 0, _x, _y);
        log_debug(_("mouse click! %d"), btn);
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
        n = read(_fd, trash, sizeof trash);
        if (n > 0) 
            log_debug(_("mouse_command: discarded %d bytes from input buffer"), n);
    } while (n > 0);
    
    // send command
    if ( -1 == write(_fd, &cmd, 1) ) {
        return false;
    }

    // read response (if any)
    while (count > 0) {
        gnashSleep(250*1000); // 250 ms inter-char timeout (simple method)
        // TODO: use select() instead
        
        n = read(_fd, buf, count);
        if (n <= 0) return false;
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
    GNASH_REPORT_FUNCTION;
    
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
