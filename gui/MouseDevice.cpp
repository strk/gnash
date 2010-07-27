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

#include "GnashSleep.h"
#include "log.h"
#include "InputDevice.h"

namespace gnash {

static const char *MOUSE_DEVICE = "/dev/input/mice";

MouseDevice::MouseDevice()
{
    GNASH_REPORT_FUNCTION;
}

MouseDevice::MouseDevice(Gui *gui)
{
    // GNASH_REPORT_FUNCTION;

    _gui = gui;
}

std::vector<boost::shared_ptr<InputDevice> >
MouseDevice::scanForDevices(Gui *gui)
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
        "PS2 Mouse",
        "eTurboTouch Mouse",
        "Touchscreen",
        "Power Button"
    };
    
    struct mouse_types mice[] = {
        InputDevice::PS2_MOUSE, "/dev/input/mice",      // PS/2 Mouse
#ifdef MULTIPLE_DEVICES
        InputDevice::PS2_MOUSE, "/dev/input/mouse0",
        InputDevice::PS2_MOUSE, "/dev/input/mouse1",
        InputDevice::ETT_MOUSE, "/dev/usb/tkpanel0",    // eTurboTouch touchscreen
#endif
        InputDevice::UNKNOWN, 0
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
            } // open()
            log_debug("Found a %s device for mouse input using %s",
                      debug[mice[i].type], mice[i].filespec);
            
            boost::shared_ptr<InputDevice> dev;
#if defined(USE_MOUSE_PS2) || defined(USE_MOUSE_ETT)
            dev = boost::shared_ptr<InputDevice>(new MouseDevice(gui));
            if (dev->init(mice[i].filespec, DEFAULT_BUFFER_SIZE)) {
                devices.push_back(dev);
            }
            dev->dump();
#endif
        }     // stat()
        close(fd);
        i++;
    }         // while()
    
    return devices;
}

bool
MouseDevice::init()
{
    GNASH_REPORT_FUNCTION;

    return init(MOUSE_DEVICE, DEFAULT_BUFFER_SIZE);
}

bool
MouseDevice::init(const std::string &filespec, size_t size)
{
    GNASH_REPORT_FUNCTION;

    _type = PS2_MOUSE;
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
  
    log_debug(_("Mouse enabled."));
      
    return true;
}

bool
MouseDevice::check()
{
    GNASH_REPORT_FUNCTION;

    return false;
}

bool
MouseDevice::command(unsigned char cmd, unsigned char *buf, int count)
{
    GNASH_REPORT_FUNCTION;

    int n;
    
    // flush input buffer
    char trash[16];
    do {
        n = read(_fd, trash, sizeof trash);
        if (n > 0) 
            log_debug(_("mouse_command: discarded %d bytes from input buffer"), n);
    } while (n > 0);
    
    // send command
    write(_fd, &cmd, 1);

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

// end of namespace
}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
