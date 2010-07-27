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

#include "log.h"
#include "InputDevice.h"

namespace gnash {

static const char *MOUSE_DEVICE = "/dev/usb/tkpanel0";

bool
TouchDevice::init(InputDevice::devicetype_e type, const std::string &filespec, size_t size)
{
    GNASH_REPORT_FUNCTION;
    
    // Try to open mouse device, be error tolerant (FD is kept open all the time)
    _fd = open(MOUSE_DEVICE, O_RDWR);
    
    if (_fd < 0) {
        log_debug("Could not open %s: %s", MOUSE_DEVICE, strerror(errno));
        return false;
    }
    
    unsigned char buf[10];
    
    if (fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL) | O_NONBLOCK) < 0) {
        log_error("Could not set non-blocking mode for touchpad device: %s", strerror(errno));
        close(_fd);
        _fd = -1;
        return false; 
    }

    if (_buffer) {
        // Clear input buffer
        while (read(_fd, _buffer.get(), DEFAULT_BUFFER_SIZE) > 0 ) { }
    }
    
    log_debug(_("Touchpad enabled."));
    
    return true;
}

bool
TouchDevice::check()
{
    GNASH_REPORT_FUNCTION;

    // If we don't have a GUi attached, then we can't propogate the events
    if (!_gui) {
        return false;
    }
    
    bool activity = false;
  
    if (_fd < 0) {
        return false;   // no mouse available
    }
  
    readData();
  
    // resync
    int pos = -1;
    int i;
    for (i=0; i<_position; i++)
        if (_buffer[i] & 0x80) { 
            pos = i;
            break;    
        }
    if (pos < 0) {
        return false; // no sync or no data
    }
  
    if (pos > 0) {
        //printf("touchscreen: removing %d bytes garbage!\n", pos);  
        memmove(_buffer.get(), _buffer.get() + pos, _position - pos);
        _position -= pos;
    }
    
    // packet complete?
    while (DEFAULT_BUFFER_SIZE > 4) {
        /*
          eTurboTouch version??
          mouse_btn = ((mouse_buf[0] >> 4) & 1);
          mouse_x = (mouse_buf[1] << 4) | (mouse_buf[2] >> 3);
          mouse_y = (mouse_buf[3] << 4) | (mouse_buf[4] >> 3);
        */

        int new_btn = (_buffer[0] & 1);
        int new_x = (_buffer[1] << 7) | (_buffer[2]);
        int new_y = (_buffer[3] << 7) | (_buffer[4]);
    
        /*
          printf("touchscreen: %02x %02x %02x %02x %02x | status %d, pos: %d/%d\n",
          mouse_buf[0], mouse_buf[1], mouse_buf[2], mouse_buf[3], mouse_buf[4],   
          new_btn, new_x, new_y);
        */    
    
        new_x = static_cast<int>(((static_cast<double>(new_x )- 355) / (1702 - 355)
                                  * 1536 + 256));
        new_y = static_cast<int>(((static_cast<double>(new_y) - 482) / (1771 - 482)
                                  * 1536 + 256));
    
        new_x = new_x * _gui->getScreenResX() / 2048;
        new_y = (2048-new_y) * _gui->getScreenResY() / 2048;

        if ((new_x != _x) || (new_y != _y)) {
            _x = new_x;
            _y = new_y;
            _gui->notifyMouseMove(_x, _y);
            activity = true;
        }
    
        if (new_btn != _button) {
            _button = new_btn;      
            printf("clicked: %d\n", _button);
            _gui->notifyMouseClick(_button);  // mask=?
            activity = true;
        }
    
        // remove from buffer
        pos=5;
        memmove(_buffer.get(), _buffer.get() + pos, _position - pos);
        _position -= pos;    
    }
  
    return activity;  
}

void
TouchDevice::apply_ts_calibration(float* cx, float* cy, int rawx, int rawy)
{
    /*
      <UdoG>:
      This is a *very* simple method to translate raw touchscreen coordinates to
      the screen coordinates. We simply to linear interpolation between two points.
      Note this won't work well when the touchscreen is not perfectly aligned to
      the screen (ie. slightly rotated). Standard touchscreen calibration uses
      5 calibration points (or even 25). If someone can give me the formula, tell
      me! I'm too lazy right now to do the math myself... ;)  
  
      And sorry for the quick-and-dirty implementation! I'm in a hurry...
    */

    float ref1x = _gui->getScreenResX() / 5 * 1;
    float ref1y = _gui->getScreenResY() / 5 * 1;
    float ref2x = _gui->getScreenResX() / 5 * 4;
    float ref2y = _gui->getScreenResY() / 5 * 4;
  
    static float cal1x = 2048/5*1;   // very approximative default values
    static float cal1y = 2048/5*4;
    static float cal2x = 2048/5*4;
    static float cal2y = 2048/5*1;
  
    static bool initialized=false; // woohooo.. better don't look at this code...
    if (!initialized) {
        initialized = true;
    
        char* settings = std::getenv("TSCALIB");
    
        if (settings) {
    
            // expected format: 
            // 491,1635,1581,646      (cal1x,cal1y,cal2x,cal2y; all integers)

            char buffer[1024];      
            char* p1;
            char* p2;
            bool ok = false;
      
            snprintf(buffer, sizeof buffer, "%s", settings);
            p1 = buffer;
      
            do {
                // cal1x        
                p2 = strchr(p1, ',');
                if (!p2) continue; // stop here
                *p2 = 0;
                cal1x = atoi(p1);        
                p1=p2+1;
        
                // cal1y        
                p2 = strchr(p1, ',');
                if (!p2) continue; // stop here
                *p2 = 0;
                cal1y = atoi(p1);        
                p1=p2+1;
        
                // cal2x        
                p2 = strchr(p1, ',');
                if (!p2) continue; // stop here
                *p2 = 0;
                cal2x = atoi(p1);        
                p1=p2+1;
        
                // cal2y        
                cal2y = atoi(p1);
        
                ok = true;        
        
            } while (0);
      
            if (!ok)
                log_debug(_("WARNING: Error parsing calibration data!"));
      
            log_debug(_("Using touchscreen calibration data: %.0f / %.0f / %.0f / %.0f"),
                      cal1x, cal1y, cal2x, cal2y);
        } else {
            log_debug(_("WARNING: No touchscreen calibration settings found. "
                        "The mouse pointer most probably won't work precisely. Set "
                        "TSCALIB environment variable with correct values for better results"));
        }
    
    } //!initialized

    // real duty: 
    *cx = (rawx-cal1x) / (cal2x-cal1x) * (ref2x-ref1x) + ref1x;
    *cy = (rawy-cal1y) / (cal2y-cal1y) * (ref2y-ref1y) + ref1y;
}

// end of namespace
}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
