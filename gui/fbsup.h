//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef __FBSUP_H__
#define __FBSUP_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <vector>

#include "gui.h"
#include <linux/fb.h>

#define PIXELFORMAT_LUT8
#define CMAP_SIZE (256*2)

// If defined, an internal software-buffer is used for rendering and is then
// copied to the video RAM. This avoids flicker and is faster for complex 
// graphics, as video RAM access is usually slower. 
// (strongly suggested)
#define DOUBLE_BUFFER


// TODO: Make this configurable via ./configure!


// Define this to read from /dev/input/mice (any PS/2 compatbile mouse or
// emulated by the Kernel) 
//#define USE_MOUSE_PS2

// Define this to read from /dev/input/event0 (new generic input subsystem)
#define USE_INPUT_EVENTS

// Define this to support eTurboTouch / eGalax touchscreens. When reading from
// a serial device, it must be initialized (stty) externally. 
//#define USE_MOUSE_ETT

#ifdef USE_MOUSE_PS2
#define MOUSE_DEVICE "/dev/input/mice"
#endif

#ifdef USE_MOUSE_ETT
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
/// Supported graphics modes:
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
#ifdef DOUBLE_BUFFER
		unsigned char *buffer; // offscreen buffer
#endif		

    std::vector< geometry::Range2d<int> > _drawbounds;

    int m_stage_width;
    int m_stage_height;
    unsigned m_rowsize;

  	int input_fd; /// file descriptor for /dev/input/mice
  	int keyb_fd; /// file descriptor for /dev/input/event* (keyboard)
  	int mouse_x, mouse_y, mouse_btn;
  	unsigned char mouse_buf[256];
  	int mouse_buf_size;
  	
  	// Keyboard SHIFT/CTRL/ALT states (left + right)
  	bool keyb_lshift, keyb_rshift, keyb_lctrl, keyb_rctrl, keyb_lalt, keyb_ralt;

    struct fb_var_screeninfo var_screeninfo;
  	struct fb_fix_screeninfo fix_screeninfo;

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

#ifdef USE_MOUSE_PS2  	
  	/// Sends a command to the mouse and waits for the response
  	bool mouse_command(unsigned char cmd, unsigned char *buf, int count);
#endif

    /// Fills the mouse data input buffer with fresh data
    void read_mouse_data();  	
  	
  	/// Initializes mouse routines
  	bool init_mouse();
  	
  	/// Checks for and processes any mouse activity. Returns true on activity.
  	bool check_mouse();
  	
  	/// Initializes keyboard routines 
  	bool init_keyboard();
  	  	
  	/// Translates a scancode from the Linux Input Subsystem to a Gnash key code 
    gnash::key::code scancode_to_gnash_key(int code, bool shift);
  	
  	/// Checks for and processes any keyboard activity. Returns true on activity.
  	bool check_keyboard();
  	
#ifdef USE_INPUT_EVENTS  	
    /// Applies builtin touchscreen calibration
  	void apply_ts_calibration(float* cx, float* cy, int rawx, int rawy);
#endif
  	
  	int valid_x(int x);
  	int valid_y(int y);
  	  	
	public:
		FBGui(unsigned long xid, float scale, bool loop, RunResources& r);
    virtual ~FBGui();
    virtual bool init(int argc, char ***argv);
    virtual bool createWindow(const char* title, int width, int height);
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
};

// end of namespace gnash
}

#endif

