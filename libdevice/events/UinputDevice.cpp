// 
//   Copyright (C) 2010, 2011, 2012 Free Software Foundation, Inc
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

#include "GnashSleep.h"
#include "log.h"
#include "InputDevice.h"

namespace gnash {

UinputDevice::UinputDevice()
    : _fd(-1)
{
    // GNASH_REPORT_FUNCTION;
}

UinputDevice::~UinputDevice()
{
    // GNASH_REPORT_FUNCTION;
    if (_fd) {
        if (ioctl(_fd, UI_DEV_DESTROY, 0) < 0) {
            log_error(_("ioctl(UI_DEV_DESTROY)"));
        }
    }
}

bool
UinputDevice::scanForDevice()
{
    // GNASH_REPORT_FUNCTION;

    struct stat st;

    // Look for these files for mouse input
    struct mouse_types {
        InputDevice::devicetype_e type;
        const char *filespec;
    };    

    // The Uinput device is found in one of these two locations.
    const char *mice[] = {
        "/dev/input/event4",
        "/dev/uinput",
        "/dev/input/uinput",
        nullptr
    };

    int i = 0;
    while (mice[i]) {
        if (stat(mice[i], &st) == 0) {
            // Then see if we can open it, this is a write only device
            if ((_fd = open(mice[i], O_WRONLY)) < 0) {
                log_error(_("You don't have the proper permissions to open %s"),
                          mice[i]);
                i++;
                continue;
            }
            log_debug(_("Found a User mode input device at %s"), mice[i]);
            return true;
            
        }     // stat()      
        i++;
    }         // while()
    
    return false;
}

bool
UinputDevice::init()
{
    // GNASH_REPORT_FUNCTION;

    if (_fd < 0) {
        log_error(_("User Mode Input device not initialized yet!"));
        return false;
    }
    
    if (ioctl(_fd, UI_SET_EVBIT, EV_KEY) < 0) {
        log_error(_("ioctl(UI_SET_EVBIT, EV_KEY)"));
        // return false;
    }

#if 0 // USE_RELATIVE_POINTER
    if (ioctl(_fd, UI_SET_EVBIT, EV_REL) < 0) {
        log_error(_("ioctl(UI_SET_EVBIT, EV_REL)"));
        // return false;
    }
    if (ioctl(_fd, UI_SET_RELBIT, REL_X) < 0) {
        log_error(_("ioctl(UI_SET_RELBIT, REL_X)"));
        // return false;
    }
    if (ioctl(_fd, UI_SET_RELBIT, REL_Y) < 0) {
        log_error(_("ioctl( UI_SET_RELBIT, REL_Y)"));
        // return false;
    }
#else
#if 1 // USE_ABSOLUTE_POINTER
    uinput_user_dev uidev = uinput_user_dev();

    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput");
    uidev.id.bustype = BUS_USB;
    // uidev.id.vendor  = 0x1;
    // uidev.id.product = 0x1;
    // uidev.id.version = 1;

    uidev.absmin[ABS_X] = 0;
    uidev.absmax[ABS_X] = 1024; //_screen_width;
    uidev.absfuzz[ABS_X] = 0;
    uidev.absflat[ABS_X] = 0;
    uidev.absmin[ABS_Y] = 0;
    uidev.absmax[ABS_Y] = 768; //_screen_height;
    uidev.absfuzz[ABS_Y] = 0;
    uidev.absflat[ABS_Y ] = 0;
    uidev.absmax[ABS_PRESSURE]=400;

    if (::write(_fd, (char *)&uidev, sizeof(uidev)) < 0) {
        log_error(_("write uidev"));
        // return false;
    }
#endif
    if (ioctl(_fd, UI_SET_EVBIT, EV_ABS) < 0) {
        log_error(_("ioctl(UI_SET_EVBIT, EV_ABS): %s"), strerror(errno));
        // return false;
    }
    if (ioctl(_fd, UI_SET_ABSBIT,ABS_X) < 0) {
        log_error(_("ioctl(UI_SET_ABSBIT,ABS_X): %s"), strerror(errno));
        // return false;
    }
    if (ioctl(_fd, UI_SET_ABSBIT, ABS_Y) < 0) {
        log_error(_("ioctl(UI_SET_ABSBIT, ABS_Y): %s"), strerror(errno));
        // return false;
    }
#endif
    
    if (ioctl(_fd, UI_SET_KEYBIT, BTN_LEFT) < 0) {
        log_error(_("ioctl(UI_SET_KEYBIT, BTN_LEFT)): %s"), strerror(errno));
        // return false;
    }
    if (ioctl(_fd, UI_SET_KEYBIT, BTN_RIGHT) < 0) {
        log_error(_("ioctl(UI_SET_KEYBIT, BTN_RIGHT): %s"), strerror(errno));
        // return false;
    }
    if (ioctl(_fd, UI_SET_EVBIT, ABS_PRESSURE) < 0) {
        log_error(_("ioctl(UI_SET_EVBIT, ABS_PRESSURE): %s"), strerror(errno));
        // return false;
    }
    // if (ioctl(_fd, UI_SET_EVBIT, ABS_TOUCH) < 0) {
    //     log_error("ioctl(UI_SET_EVBIT, ABS_TOUCH)");
    //     return false;
    // }
    if (ioctl(_fd, UI_SET_KEYBIT, BTN_MOUSE) < 0) {
        log_error(_("ioctl(UI_SET_KEYBIT, BTN_MOUSE): %s"), strerror(errno));
        // return false;
    }

    if (ioctl(_fd, UI_DEV_CREATE, 0) < 0) {
        log_error(_("ioctl(UI_DEV_CREATED) failed!"),  strerror(errno));
        // return false;
    }
    
    return true;
}

// Move the mouse cursor to a specified location
bool
UinputDevice::moveTo(int x, int y)
{
    // GNASH_REPORT_FUNCTION;

    input_event ev = input_event();
    
    gettimeofday(&ev.time, nullptr);
    ev.type = EV_ABS;
    ev.code = ABS_X;
    ev.value = x;
    if (::write(_fd, &ev, sizeof(struct input_event)) < 0) {
        log_error("write ABS_X");
        return false;
    }
    
    ev.code = SYN_REPORT;
    ev.type = EV_SYN;
    if (::write(_fd, &ev, sizeof(struct input_event)) < 0) {
        log_error("write SYN");
        return false;
    }

    ev.type = EV_ABS;
    ev.code = ABS_Y;
    ev.value = y;
    if (::write(_fd, &ev, sizeof(struct input_event)) < 0) {
        log_error("write ABS_Y");
        return false;
    }

    ev.code = SYN_REPORT;
    ev.type = EV_SYN;
    if (::write(_fd, &ev, sizeof(struct input_event)) < 0) {
        log_error("write SYN");
        return false;
    }

    return true;
}   

// end of namespace
}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
