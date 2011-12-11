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

#ifdef HAVE_TSLIB_H
# include <tslib.h>
#endif
#if defined(USE_TSLIB) && !defined(HAVE_TSLIB_H)
# warning "No tslib.h! Disabling touchscreen support"
#endif

#include "log.h"
#include "InputDevice.h"

namespace gnash {

#ifdef USE_TSLIB
// Either use environment variable or hardcoded value
// Hint: /dev/ts can be a symlink to the real ts device.
// TSLIB_DEVICE environment variable should point to the
// touchscreen device the library is using.
static const char *TSLIB_DEVICE_ENV = "TSLIB_TSDEVICE";
static const char *TSLIB_DEVICE_NAME = "/dev/ts";
#endif

//static const char *MOUSE_DEVICE = "/dev/usb/tkpanel0";

TouchDevice::TouchDevice()
{
    // GNASH_REPORT_FUNCTION;
}

TouchDevice::~TouchDevice()
{
    // GNASH_REPORT_FUNCTION;

    if (_tsDev) {
        ts_close(_tsDev);
    }
}    

bool
TouchDevice::init()
{
    return init(TSLIB_DEVICE_NAME, DEFAULT_BUFFER_SIZE);
}

bool
TouchDevice::init(const std::string &filespec, size_t /* size */)
{
    // GNASH_REPORT_FUNCTION;

    _type = TouchDevice::TOUCHSCREEN;
    _filespec = filespec;
    
    char *devname = getenv(TSLIB_DEVICE_ENV);
    if (!devname) {
        if (!filespec.empty()) {
            devname = const_cast<char *>(_filespec.c_str());
        } else {
            log_error("No filespec specified for the touchscreen device.");
        }
    }
    
    _tsDev = ts_open(devname, 1);  //Open tslib non-blocking
    if (_tsDev == 0) {
        log_error("Could not open touchscreen %s: %s", devname, strerror(errno));
        return false;
    }
    
    ts_config(_tsDev); 
    if (ts_fd(_tsDev) < 0) {
        log_error("Could not get touchscreen fd %s: %s", devname, strerror(errno));
        return false;
    }
    
    _fd = ts_fd(_tsDev);
    
    log_debug("Using TSLIB on %s", devname);
    
    return true;
}

bool
TouchDevice::check()
{
//    GNASH_REPORT_FUNCTION;

    // Read events from the touchscreen and transport them into Gnash
    // Tslib should be setup so the output is pretty clean.
    struct ts_sample event;
    
    if (_tsDev == 0) {
        return false;           // No tslib device initialized, exit!
    }
    
    int n = ts_read(_tsDev, &event, 1);     // read one event

    // Did we read an event?
    if (n == 1) {
        if (event.pressure > 0) {
            // the screen is touched
            // FIXME: this is a bit of a temporary hack. The last two
            // arguments are a range, so hardcoding them is safe for
            // now. In the future more conversion may be done, making this
            // then be incorrect.
            boost::shared_array<int> coords =
                InputDevice::convertAbsCoords(event.x, event.y,
                                                _screen_width, _screen_height);
            log_debug("Touched x: %d, y: %d", event.x , event.y);
            addData(true, gnash::key::INVALID, 0, event.x, event.y);
        } else {
            addData(false, gnash::key::INVALID, 0, event.x, event.y);
            log_debug("lifted x: %d y: %d", event.x, event.y); //debug
        }
    }

    return true;
}

// FIXME: this currently is lacking the swf program used to generate the
// input data. Instead use the tslib utility 'ts_calibrate', as Gnash now
// has TSlib support.
void
TouchDevice::apply_ts_calibration(float* cx, float* cy, int rawx, int rawy)
{
    GNASH_REPORT_FUNCTION;
    
    // This method use 3 points calibration
    // it is described in http://www.embedded.com/story/OEG20020529S0046
    
    float k,a,b,c,d,e,f;

#if 1
    float ref0x = 800 / 5 * 1;
    float ref0y = 480 / 5 * 1;
    float ref1x = 800 / 5 * 4;

    float ref1y = 480  / 5 * 1;
    float ref2x = 800  / 5 * 1;
    float ref2y = 480  / 5 * 4;
#else   
    float ref0x = _gui->getStage()->getStageWidth() / 5 * 1;
    float ref0y = _gui->getStage()->getStageHeight() / 5 * 1;
    float ref1x = _gui->getStage()->getStageWidth() / 5 * 4;

    float ref1y = _gui->getStage()->getStageHeight() / 5 * 1;
    float ref2x = _gui->getStage()->getStageWidth() / 5 * 4;
    float ref2y = _gui->getStage()->getStageHeight() / 5 * 4;
#endif
    static float cal0x = 2048/5*1;   // very approximative default values
    static float cal0y = 2048/5*4;
    static float cal1x = 2048/5*1;
    static float cal1y = 2048/5*1;
    static float cal2x = 2048/5*4;
    static float cal2y = 2048/5*1;
  
    static bool initialized=false; // woohooo.. better don't look at this code...
    if (!initialized) {
        initialized = true;
    
        char* settings = std::getenv("TSCALIB");
    
        if (settings) {
    
            // expected format: 
            // 491,1635,451,537,1581,646
            // (cal0x,cal0y,cal1x,cal1y,cal2x,cal2y; all integers)
            char buffer[1024];      
            char* p1;
            char* p2;
            bool ok = false;
      
            snprintf(buffer, sizeof buffer, "%s", settings);
            p1 = buffer;
      
            do {
                p2 = strchr(p1, ',');
                if (!p2) continue; // stop here
                *p2 = 0;
                cal0x = atoi(p1);        
                p1=p2+1;
                
                // cal0y        
                p2 = strchr(p1, ',');
                if (!p2) continue; // stop here
                *p2 = 0;
                cal0y = atoi(p1);        
                p1=p2+1;
                
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
      
            log_debug(_("Using touchscreen calibration data: %.0f / %.0f / %.0f / %.0f / %.0f / %.0f"),
                      cal0x, cal0y, cal1x, cal1y, cal2x, cal2y);
        } else {
            log_debug(_("WARNING: No touchscreen calibration settings found. "
                        "The mouse pointer most probably won't work precisely. Set "
                        "TSCALIB environment variable with correct values for better results"));
        }
    
    } //!initialized

      // calcul of K, A, B, C, D, E and F
    k = (cal0x - cal2x) * (cal1y - cal2y) - (cal1x - cal2x) * (cal0y - cal2y);
    a = ((ref0x - ref2x) * (cal1y - cal2y) - (ref1x - ref2x) * (cal0y - cal2y)) / k;
    b = ((ref1x - ref2x) * (cal0x - cal2x) - (ref0x - ref2x) * (cal1x - cal2x)) / k;
    c = (cal0y * (cal2x * ref1x - cal1x * ref2x) + cal1y * (cal0x * ref2x - cal2x * ref0x) + cal2y * (cal1x * ref0x - cal0x * ref1x)) / k;
    d = ((ref0y - ref2y) * (cal1y - cal2y) - (ref1y - ref2y) * (cal0y - cal2y)) / k;
    e = ((ref1y - ref2y) * (cal0x - cal2x) - (ref0y - ref2y) * (cal1x - cal2x)) / k;
    f = (cal0y * (cal2x * ref1y - cal1x * ref2y) + cal1y * (cal0x * ref2y - cal2x * ref0y) + cal2y * (cal1x * ref0y - cal0x * ref1y)) / k;
    
    // real duty:
    *cx = a * rawx + b * rawy + c;
    *cy = d * rawx + e * rawy + f;
}

std::vector<boost::shared_ptr<InputDevice> >
TouchDevice::scanForDevices()
{
    // GNASH_REPORT_FUNCTION;

    struct stat st;

    std::vector<boost::shared_ptr<InputDevice> > devices;

    // Debug strings to make output more readable
    const char *debug[] = {
        "UNKNOWN",
        "KEYBOARD",
        "MOUSE",
        "TABLET",
        "TOUCHSCREEN",
        "TOUCHMOUSE",
        "POWERBUTTON",
        "SLEEPBUTTON",
        "SERIALUSB",
        "INFRARED"
    };
    
    // Look for these files for mouse input
    struct ts_types {
        InputDevice::devicetype_e type;
        const char *filespec;
    };

    struct ts_types touch[] = {
        {InputDevice::TOUCHSCREEN, "/dev/input/ts0"},
        {InputDevice::TOUCHSCREEN, "/dev/ts"},
        {InputDevice::UNKNOWN, 0}
    };

    int i = 0;
    while (touch[i].type != InputDevice::UNKNOWN) {
        int fd = 0;
        // log_debug("Checking for device %s...", touch[i].filespec);
        if (stat(touch[i].filespec, &st) == 0) {
            // Then see if we can open it
            if ((fd = open(touch[i].filespec, O_RDWR)) < 0) {
                log_error("You don't have the proper permissions to open %s",
                          touch[i].filespec);
                i++;
                continue;
            }
            close(fd);
            log_debug("Found a %s device for touchscreen input using %s",
                      debug[touch[i].type], touch[i].filespec);
            boost::shared_ptr<InputDevice> dev
                = boost::shared_ptr<InputDevice>(new TouchDevice());
            if (dev->init(touch[i].filespec, DEFAULT_BUFFER_SIZE)) {
                devices.push_back(dev);
            }
            break;
//            dev->dump();
        }     // stat()
        i++;
    }         // while()
    
    return devices;
}

// end of namespace
}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
