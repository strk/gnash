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
#include <fcntl.h>

#include "GnashSleep.h"
#include "log.h"
#include "InputDevice.h"

namespace gnash {

static const char *MOUSE_DEVICE = "/dev/input/mice";

bool
MouseDevice::init(const std::string &filespec, size_t size)
{
    GNASH_REPORT_FUNCTION;

    _type = PS2_MOUSE;
    _filespec = filespec;    
    _buffer.reset(new boost::uint8_t[size]);

    // see http://www.computer-engineering.org/ps2mouse/ 
    
    // Try to open mouse device, be error tolerant (FD is kept open all the time)
    _fd = open(MOUSE_DEVICE, O_RDWR);
    
    if (_fd < 0) {
        log_debug("Could not open %s: %s", gnash::MOUSE_DEVICE, strerror(errno));
        return false;
    }
    
    unsigned char buf[10], byte;
    
    if (fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL) | O_NONBLOCK) < 0) {
        log_error("Could not set non-blocking mode for mouse device: %s", strerror(errno));
        close(_fd);
        _fd = -1;
        return false; 
    }
    
    // Clear input buffer
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
}

bool
MouseDevice::command(unsigned char cmd, unsigned char *buf, int count)
{
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
