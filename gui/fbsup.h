//
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
//
//

#ifndef __FBSUP_H__
#define __FBSUP_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gui.h"
#include <linux/fb.h>

#define PIXELFORMAT_LUT8
#define CMAP_SIZE (256*2)

#define DOUBLE_BUFFER


namespace gnash
{


/// A Framebuffer-based GUI for Gnash.
//
/// It currently requires the AGG backend to render the frames.
/// It operates in fullscreen mode and uses the screen mode 
/// currently active.
///
/// Supported graphics modes:
///
///   Resolution: any
///
///   Pixel formats:
///     8 bit: none yet
///     15 bit: R5/G5/B5
///     16 bit: R5/G6/B5
///     24 bit: R8/G8/B8, B8/G8/R8
///
///
/// Supported input devices:
///
///   None, yet. But may be added easily.    
///
class FBGui : public Gui
{
	private:
		int fd;
		unsigned char *fbmem;  // framebuffer memory
#ifdef DOUBLE_BUFFER
		unsigned char *buffer; // offscreen buffer
#endif		
    int m_draw_minx;
    int m_draw_miny;
    int m_draw_maxx;
    int m_draw_maxy;
    
    int m_stage_width;
    int m_stage_height;

    struct fb_var_screeninfo var_screeninfo;
  	struct fb_fix_screeninfo fix_screeninfo;

	/// For 8 bit (palette / LUT) modes, sets a grayscale palette.
	//
	/// This GUI currently does not support palette modes. 
	///
  	bool set_grayscale_lut8();
  	
  	bool initialize_renderer();
  	
  	int valid_x(int x);
  	int valid_y(int y);
  	
	public:
		FBGui();
		FBGui(unsigned long xid, float scale, bool loop, unsigned int depth);
    virtual ~FBGui();
    virtual bool init(int argc, char **argv[]);
    virtual bool createWindow(const char* title, int width, int height);
    virtual bool run();
    virtual bool createMenu();
    virtual bool setupEvents();
    virtual void renderBuffer();
    virtual void setInterval(unsigned int interval);
    virtual void setTimeout(unsigned int timeout);
    
    virtual void set_invalidated_region(const rect& bounds);
};

// end of namespace gnash
}

#endif

