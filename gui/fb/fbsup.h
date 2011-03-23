//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#ifndef GNASH_FBSUP_H
#define GNASH_FBSUP_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/scoped_array.hpp>
#include <vector>
#include <linux/fb.h>

#include "gui.h"
#include "InputDevice.h"

#define PIXELFORMAT_LUT8
#define CMAP_SIZE (256*2)

#ifdef USE_MOUSE_PS2
# define MOUSE_DEVICE "/dev/input/mice"
#endif

// FIXME: this should really be TSLIB_DEVICE_NAME, but I don't have the
// ETT SDK, so for now, leave it the way it was.
#ifdef USE_ETT_TSLIB
#define MOUSE_DEVICE "/dev/usb/tkpanel0"
#endif

// Define this to request a new virtual terminal at startup. This doesn't always
// work and probably is not necessary anyway
//#define REQUEST_NEW_VT

namespace gnash
{

/// A Framebuffer-based GUI for Gnash.
/// ----------------------------------
///
/// This is a simple "GUI" that works with any framebuffer device (/dev/fb0).
/// No window system is required, it will run straigt from a console. 
///
/// The current version requires that your system boots in graphics mode (that
/// is, with a framebuffer driver - like vesafb - and most probably the 
/// virtual console enabled). Which graphics mode Gnash runs in depends on the 
/// mode your machine boots in and can be choosen using the kernel command line.
/// With other words: Gnash does not change the graphics mode.
/// Refer to the framebuffer docs for more information.
///
/// The fb gui now also supports pointing devices like mice or touchscreens, but
/// it is not required. It works with /dev/input/mice so any PS/2 compatible 
/// mouse should do. Your kernel can emulate the "mice" device for other devices
/// (like touchscreens and tablets) so this method is very flexible.
///
/// There is currently no visible mouse pointer built in, which is fine for 
/// touchscreens but will make it difficult for standard mice. This will be 
/// fixed in near time.
//
// Supported graphics modes:
///
///   Resolution: any
///
///   Pixel formats:
///     8 bit: none yet
///     15 bit: R5/G5/B5
///     16 bit: R5/G6/B5
///     24 bit: R8/G8/B8, B8/G8/R8
///     32 bit: R8/G8/B8/A8, B8/G8/R8/A8
///
///
/// Supported input devices:
///
///   any PS/2 compatible mouse (may be emulated by the kernel) talking 
///   to /dev/input/mice
class FBGui : public Gui
{
private:
    int fd;
    int original_vt;       // virtual terminal that was active at startup
    int original_kd;       // keyboard mode at startup
    int own_vt;            // virtual terminal we are running in   
    unsigned char *fbmem;  // framebuffer memory
    unsigned char *buffer; // offscreen buffer
    
    std::vector< geometry::Range2d<int> > _drawbounds;

    // X position of the output window
    int _xpos;

    // Y position of the output window
    int _ypos;
    
    unsigned m_rowsize;
    
    std::vector<boost::shared_ptr<InputDevice> > _inputs;

    struct fb_var_screeninfo var_screeninfo;
    struct fb_fix_screeninfo fix_screeninfo;

    unsigned int _timeout; /* TODO: should we move this to base class ? */
    
    /// For 8 bit (palette / LUT) modes, sets a grayscale palette.
    //
    /// This GUI currently does not support palette modes. 
    ///
    bool set_grayscale_lut8();
    
    bool initialize_renderer();
    
    /// Tries to find a accessible tty
    char* find_accessible_tty(int no);
    char* find_accessible_tty(const char* format, int no);
    
    /// switches from text mode to graphics mode (disables the text terminal)
    bool disable_terminal();
    
    /// reverts disable_terminal() changes
    bool enable_terminal();
    
public:
    FBGui(unsigned long xid, float scale, bool loop, RunResources& r);
    virtual ~FBGui();
    virtual bool init(int argc, char ***argv);
    virtual bool createWindow(const char *title, int width, int height,
                              int xPosition = 0, int yPosition = 0);
    virtual bool run();
    virtual bool createMenu();
    virtual bool setupEvents();
    virtual void renderBuffer();
    virtual void setInterval(unsigned int interval);
    virtual void setTimeout(unsigned int timeout);
    
    virtual void setFullscreen();
    virtual void unsetFullscreen();
    
    virtual void showMenu(bool show);
    virtual bool showMouse(bool show);
    
    virtual void setInvalidatedRegions(const InvalidatedRanges& ranges);
    virtual bool want_multiple_regions() { return true; }

    void checkForData();    
};

// end of namespace gnash
}

#ifdef ENABLE_FAKE_FRAMEBUFFER
/// Simulate the ioctls used to get information from the framebuffer driver.
///
/// Since this is an emulator, we have to set these fields to a reasonable default.
int fakefb_ioctl(int fd, int request, void *data);
#endif

#endif  // end of GNASH_FBSUP_H

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
