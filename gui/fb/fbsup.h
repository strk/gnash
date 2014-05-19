//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Free Software
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

#ifndef GNASH_FBSUP_H
#define GNASH_FBSUP_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>
#include <vector>

#include <linux/fb.h>

#include "gui.h"
#include "events/InputDevice.h"
#include "Renderer.h"

#define PIXELFORMAT_LUT8

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

namespace gnash {

namespace gui {

class FBGlue;

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
public:
    FBGui(unsigned long xid, float scale, bool loop, RunResources& r);
    virtual ~FBGui();
    /// \brief Initialize the framebuffer
    ///
    /// This opens the framebuffer device,
    virtual bool init(int argc, char ***argv);
    /// \brief
    /// Create and display our window.
    ///
    /// @param title The window title.
    /// @param width The desired window width in pixels.
    /// @param height The desired window height in pixels.
    /// @param xPosition The desired window X position from the top left corner.
    /// @param yPosition The desired window Y position from the top left corner.
    bool createWindow(const char *title, int width, int height,
                              int xPosition = 0, int yPosition = 0);

    /// Render the current buffer.
    /// For OpenGL, this means that the front and back buffers are swapped.
    void renderBuffer();
    
    /// Start main rendering loop.
    bool run();

    // Resize the viewing area within the total framebuffer display
    bool resize_view(int width, int height);
    
    /// Gives the GUI a *hint* which region of the stage should be redrawn.
    //
    /// There is *no* restriction what the GUI might do with these coordinates. 
    /// Normally the GUI forwards the information to the renderer so that
    /// it avoids rendering regions that did not change anyway. The GUI can
    /// also alter the bounds before passing them to the renderer and it's
    /// absolutely legal for the GUI to simply ignore the call.
    ///
    /// Coordinates are in TWIPS!
    ///
    /// Note this information is given to the GUI and not directly to the 
    /// renderer because both of them need to support this feature for 
    /// correct results. It is up to the GUI to forward this information to
    /// the renderer.
    ///
    // does not need to be implemented (optional feature),
    // but still needs to be available.
    //
    void setInvalidatedRegion(const SWFRect& bounds);
    void setInvalidatedRegions(const InvalidatedRanges& ranges);

    /// Should return TRUE when the GUI/Renderer combination supports multiple
    /// invalidated bounds regions. 
    bool want_multiple_regions() { return true; }

    // Information for System.capabilities to be reimplemented in
    // each gui.
    double getPixelAspectRatio() const { return 0; }
    int getScreenResX() { return 0; }
    int getScreenResY() { return 0; }
    double getScreenDPI() const { return 0; }
    std::string getScreenColor() const { return ""; }

    // For the framebuffer, these are mostly just stubs.

    void setFullscreen();
    void unsetFullscreen();
    
    bool createMenu();
    bool setupEvents();
    void setInterval(unsigned int interval);
    void setTimeout(unsigned int timeout);
    
    void showMenu(bool show);
    bool showMouse(bool show);    

    // Poll this to see if there is any input data.
    void checkForData();
    
private:
    // bool initialize_renderer();
    
    /// Tries to find a accessible tty
    char* find_accessible_tty(int no);
    char* find_accessible_tty(const char* format, int no);
    
    /// switches from text mode to graphics mode (disables the text terminal)
    bool disable_terminal();
    
    /// reverts disable_terminal() changes
    bool enable_terminal();
    
    int         _fd;
    int         _original_vt;   // virtual terminal that was active at startup
    int         _original_kd;   // keyboard mode at startup
    int         _own_vt;        // virtual terminal we are running in   
    
    int         _xpos;          // X position of the output window
    int         _ypos;          // Y position of the output window
    size_t      _timeout;       // timeout period for the event loop
    bool        _fullscreen;

    std::shared_ptr<FBGlue>   _glue;

    /// This is the array of functioning input devices.
    std::vector<std::shared_ptr<InputDevice> > _inputs;

    std::shared_ptr<Renderer> _renderer;
#ifdef HAVE_LINUX_UINPUT_H
    UinputDevice                _uinput;
#endif
};

} // end of namespace gui
} // end of namespace gnash

#endif  // end of GNASH_FBSUP_H

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
