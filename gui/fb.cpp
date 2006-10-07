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

// -----------------------------------------------------------------------------

/// A Framebuffer-based GUI for Gnash. It required the AGG backend to render
/// the frames. It operates in fullscreen mode and uses the screen mode 
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
   

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <unistd.h>

#include "gnash.h"
#include "gui.h"
#include "fbsup.h"
#include "log.h"

//#define DEBUG_SHOW_FPS  // prints number of frames per second to STDOUT

#ifdef DEBUG_SHOW_FPS
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
#endif
//--


namespace gnash
{

// This belongs somewhere else. Just a quick hack to allow compliation!
render_handler*	create_render_handler_agg(char *pixelformat, 
  unsigned char *mem, int memsize, int xres, int yres, int bpp);

//---------------
#ifdef DEBUG_SHOW_FPS
double fps_timer=0;
int fps_counter=0;
void profile() {
  int fd;
  double uptime, idletime;
  char buffer[20];
  int readcount;

  fd = open("/proc/uptime", O_RDONLY);
  if (fd<0) return;
  readcount = read(fd, buffer, sizeof(buffer)-1);
  buffer[readcount]=0;
  sscanf(buffer, "%lf %lf", &uptime, &idletime);
  close(fd);
  
  fps_counter++;

  if (fps_counter<2) {
    fps_timer = uptime;
    return;    
  }
  
  printf("FPS: %.3f (%.2f)\n", fps_counter/(uptime-fps_timer), uptime-fps_timer);
  
}
#endif
//---------------

FBGui::FBGui() : Gui()
{
}

FBGui::FBGui(unsigned long xid, float scale, bool loop, unsigned int depth)
	: Gui(xid, scale, loop, depth)
{
	fd 			= -1;
	fbmem 	= NULL;
	buffer  = NULL;
}

FBGui::~FBGui()
{
	if (fd>0) {
		log_msg("Closing framebuffer device\n");
		close(fd);
	}

	if (buffer) {
		log_msg("Free'ing offscreen buffer\n");
		free(buffer);
	}
}


/// For 8 bit (palette / LUT) modes, sets a grayscale palette. This GUI 
/// currently does not support palette modes. 
bool FBGui::set_grayscale_lut8()
{
  #define TO_16BIT(x) (x | (x<<8))

  struct fb_cmap cmap;
  int i;

  log_msg("LUT8: Setting up colormap\n");

  cmap.start=0;
  cmap.len=256;
  cmap.red = (__u16*)malloc(CMAP_SIZE);
  cmap.green = (__u16*)malloc(CMAP_SIZE);
  cmap.blue = (__u16*)malloc(CMAP_SIZE);
  cmap.transp = NULL;

  for (i=0; i<256; i++) {

    int r = i;
    int g = i;
    int b = i;

    cmap.red[i] = TO_16BIT(r);
    cmap.green[i] = TO_16BIT(g);
    cmap.blue[i] = TO_16BIT(b);

  }

  if (ioctl(fd, FBIOPUTCMAP, &cmap)) {
    log_error("LUT8: Error setting colormap: %s\n", strerror(errno));
    return false;
  }

  return true;

  #undef TO_16BIT
}

bool FBGui::init(int argc, char **argv[])
{
  // Open the framebuffer device
  fd = open("/dev/fb0", O_RDWR);
  if (fd<0) {
    log_error("Could not open framebuffer device: %s", strerror(errno));
    return false;
  }
  
  // Load framebuffer properties
  ioctl(fd, FBIOGET_VSCREENINFO, &var_screeninfo);
  ioctl(fd, FBIOGET_FSCREENINFO, &fix_screeninfo);
  log_msg("Framebuffer device uses %d bytes of memory.\n",
  	fix_screeninfo.smem_len);
  log_msg("Video mode: %dx%d with %d bits per pixel.\n",
    var_screeninfo.xres, var_screeninfo.yres, var_screeninfo.bits_per_pixel);

  // map framebuffer into memory
  fbmem = (unsigned char *)
    mmap(0, fix_screeninfo.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

#ifdef DOUBLE_BUFFER
  // allocate offscreen buffer
  buffer = (unsigned char*)malloc(fix_screeninfo.smem_len);
  memset(buffer, 0, fix_screeninfo.smem_len);
#endif  

#ifdef PIXELFORMAT_LUT8
  // Set grayscale for 8 bit modes
  if (var_screeninfo.bits_per_pixel==8) {
  	if (!set_grayscale_lut8())
  		return false;
  }
#endif

  // Ok, now initialize AGG
  return initialize_renderer();
}

bool FBGui::initialize_renderer() {
  int _width 		= var_screeninfo.xres;
  int _height 	= var_screeninfo.yres;
  int _bpp = var_screeninfo.bits_per_pixel;
  int _size = fix_screeninfo.smem_len;   // TODO: should recalculate!  
  unsigned char *_mem;
  
  
  #ifdef DOUBLE_BUFFER
  _mem = buffer;
  #else
  _mem = fbmem;
  #endif
  
  
  _renderer = NULL;
  
  // choose apropriate pixel format
  
  log_msg("red channel: %d / %d", var_screeninfo.red.offset, 
    var_screeninfo.red.length);
  log_msg("green channel: %d / %d", var_screeninfo.green.offset, 
    var_screeninfo.green.length);
  log_msg("blue channel: %d / %d", var_screeninfo.blue.offset, 
    var_screeninfo.blue.length);
  
  // 15 bits RGB (hicolor)
  if ((var_screeninfo.red.offset==10)
   && (var_screeninfo.red.length==5)
   && (var_screeninfo.green.offset==5)
   && (var_screeninfo.green.length==5)
   && (var_screeninfo.blue.offset==0)
   && (var_screeninfo.blue.length==5) ) {
   
    _renderer = create_render_handler_agg("RGB555", 
      _mem, _size, _width, _height, _bpp);
      
  } else   
  // 16 bits RGB (hicolor)
  if ((var_screeninfo.red.offset=11)
   && (var_screeninfo.red.length==5)
   && (var_screeninfo.green.offset==5)
   && (var_screeninfo.green.length==6)
   && (var_screeninfo.blue.offset==0)
   && (var_screeninfo.blue.length==5) ) {
   
    _renderer = create_render_handler_agg("RGB565", 
      _mem, _size, _width, _height, _bpp);
      
  } else   
  // 24 bits RGB (truecolor)
  if ((var_screeninfo.red.offset==16)
   && (var_screeninfo.red.length==8)
   && (var_screeninfo.green.offset==8)
   && (var_screeninfo.green.length==8)
   && (var_screeninfo.blue.offset==0)
   && (var_screeninfo.blue.length==8) ) {
   
    _renderer = create_render_handler_agg("RGB24", 
      _mem, _size, _width, _height, _bpp);
      
  } else   
  // 24 bits BGR (truecolor)
  if ((var_screeninfo.red.offset==0)
   && (var_screeninfo.red.length==8)
   && (var_screeninfo.green.offset==8)
   && (var_screeninfo.green.length==8)
   && (var_screeninfo.blue.offset==16)
   && (var_screeninfo.blue.length==8) ) {
   
    _renderer = create_render_handler_agg("BGR24", 
      _mem, _size, _width, _height, _bpp);
      
  } else {
    log_msg("ERROR: The pixel format of your framebuffer is not supported.");
  }

  assert(_renderer!=NULL);

  set_render_handler(_renderer);

  return true;
}

bool FBGui::run(void *)
{
	while (true) {
		// sleep for _interval milliseconds
    // TODO: Instead of sleeping, use a timer to compensate render time		
		usleep(_interval*1000);
		Gui::advance_movie(this);
	}
	return true;
}

void FBGui::renderBuffer()
{

#ifdef DOUBLE_BUFFER
	// TODO: Copy only updated parts of the screen!
	memcpy(fbmem, buffer, var_screeninfo.xres*var_screeninfo.yres*
    (var_screeninfo.bits_per_pixel/8));
#endif
	
#ifdef DEBUG_SHOW_FPS
	profile();
#endif
}

bool FBGui::createWindow(int width, int height)
{
  // Framebuffer has no windows... :-)

	return true;
}

bool FBGui::createMenu()
{
  // no menu support! 
	return true;
}

bool FBGui::setupEvents()
{
  // events currently not supported!
	return true;
}

void FBGui::setCallback(unsigned int interval)
{
	_interval = interval;
}

void FBGui::setTimeout(unsigned int timeout)
{

}


// end of namespace gnash
}
