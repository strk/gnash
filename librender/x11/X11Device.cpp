//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <iostream>
#include <cerrno>
#include <exception>
#include <sstream>

#include "log.h"
#include "GnashException.h"

#ifdef HAVE_GTK2
#include "gdk/gdkx.h"
#include "X11/Xlib.h"
#include "X11/Xutil.h"
#endif

// #ifdef HAVE_X11_H
// # include <x11/x11.h>
// #else
// # error "This file needs X11"
// #endif

#include "X11Device.h"

namespace gnash {

namespace renderer {

namespace x11 {
    
// The debug log used by all the gnash libraries.
static LogFile& dbglogfile = LogFile::getDefaultInstance();

// FIXME: this font name shouldn't be hardcoded!
const char *FONT = "/usr/share/fonts/truetype/freefont/FreeSerif.ttf";

X11Device::X11Device()
{
    GNASH_REPORT_FUNCTION;
}

X11Device::~X11Device()
{
    // GNASH_REPORT_FUNCTION;
    
// #ifdef HAVE_GTK2_XX
//     gdk_exit(0);
// #else
//     XDestroyWindow(_display, _window);
//     XCloseDisplay(_display);
// #endif
}

bool
X11Device::initDevice(int argc, char *argv[])
{
    GNASH_REPORT_FUNCTION;

#ifdef HAVE_GTK2_XX
    // As gdk_init() wants the command line arguments, we have to create
    // fake ones, as we don't care about the X11 options at this point.
    gdk_init(&argc, &argv);

#if 0
    GdkVisual* wvisual = gdk_drawable_get_visual(_drawing_area->window);

    GdkImage* tmpimage = gdk_image_new (GDK_IMAGE_FASTEST, wvisual, 1, 1);

    const GdkVisual* visual = tmpimage->visual;

    // FIXME: we use bpp instead of depth, because depth doesn't appear to
    // include the padding byte(s) the GdkImage actually has.
    const char *pixelformat = agg_detect_pixel_format(
        visual->red_shift, visual->red_prec,
        visual->green_shift, visual->green_prec,
        visual->blue_shift, visual->blue_prec,
        tmpimage->bpp * 8);

    gdk_image_destroy(tmpimage);
#endif  // end of 0
#else
    char *dpyName = NULL;
    int num_visuals;
    XSetWindowAttributes attr;
    unsigned long mask;    
    Window win;
    int width, height;
    int x, y;
    const char *name = "Foo";
 
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-display") == 0) {
            dpyName = argv[i+1];
            i++;
        }
    }

    _display = XOpenDisplay(dpyName);
    if (!_display) {
        log_error("couldn't open X11 display!");
        return false;
    }

    Window root = XDefaultRootWindow(_display);
    _screennum = XDefaultScreen(_display);

    _depth = DefaultDepth(_display, _screennum);
    _colormap = DefaultColormap(_display, _screennum);

    _visual = XDefaultVisual(_display, _screennum);

    _screen = DefaultScreenOfDisplay(_display);

    VisualID vid = XVisualIDFromVisual(_visual);
    
    XVisualInfo *visInfo, visTemplate;
    visTemplate.visualid = vid;
    visInfo = XGetVisualInfo(_display, VisualIDMask, &visTemplate, &num_visuals);
    // std::cerr << "Num Visuals: " << num_visuals << std::endl;
    if (!visInfo) {
         log_error("Error: couldn't get X visual\n");
         exit(1);
    }

    int re = visInfo[0].bits_per_rgb;
    
    XFree(visInfo);
    
    XWindowAttributes gattr;
    XGetWindowAttributes(_display, root, &gattr);
    
    // std::cerr << "Width: " << gattr.backing_store << std::endl;
    // std::cerr << "Width: " << gattr.depth << std::endl;

#if 0
    
    // window attributes
    attr.background_pixel = 0;
    attr.border_pixel = 0;
    attr.colormap = XCreateColormap(_display, root, visInfo->visual, AllocNone);
    attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
    mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;
    
    win = XCreateWindow(_display, root, 0, 0, width, height,
                         0, visInfo->depth, InputOutput,
                         visInfo->visual, mask, &attr );
    
#endif

#if 0
    // set hints and properties
    XSizeHints sizehints;
    sizehints.x = x;
    sizehints.y = y;
    sizehints.width  = width;
    sizehints.height = height;
    sizehints.flags = USSize | USPosition;
    XSetNormalHints(_display, win, &sizehints);
    XSetStandardProperties(_display, win, name, name,
                           None, (char **)NULL, 0, sizehints);
#endif
    
   // XFree(visInfo);
#endif
    
    return true;
}

// Return a string with the error code as text, instead of a numeric value
const char *
X11Device::getErrorString(int error)
{
    static char msg[80];
    
    if (_display) {
        XGetErrorText(_display, error, msg, 80);
    } else {
        log_error("The default Display is not set!");
    }

    return msg;
}

#if 0
int
X11Device::getDepth(DFBSurfacePixelFormat format)
{
    return 0;
}
#endif

} // namespace x11
} // namespace renderer
} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
