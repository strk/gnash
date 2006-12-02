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

//
//

// -----------------------------------------------------------------------------
   

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <unistd.h>
#include <signal.h>

#include "gnash.h"
#include "gui.h"
#include "fbsup.h"
#include "log.h"

#include "render_handler.h"
#include "render_handler_agg.h"

//#define DEBUG_SHOW_FPS  // prints number of frames per second to STDOUT

#ifdef DEBUG_SHOW_FPS
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
#endif
//--


namespace gnash
{


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

int terminate_request=false;  // global scope to avoid GUI access

/// Called on CTRL-C and alike
void terminate_signal(int signo) {
  terminate_request=true;
}


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
	
	signal(SIGINT, terminate_signal);
	signal(SIGTERM, terminate_signal);
}

FBGui::~FBGui()
{
  
	if (fd>0) {
	  enable_terminal();
		log_msg("Closing framebuffer device\n");
		close(fd);
	}

	if (buffer) {
		log_msg("Free'ing offscreen buffer\n");
		free(buffer);
	}
}


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


bool FBGui::init(int /*argc*/, char *** /*argv*/)
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
  render_handler_agg_base *agg_handler;
  
  m_stage_width = _width;
  m_stage_height = _height;
  
  
  #ifdef DOUBLE_BUFFER
  _mem = buffer;
  #else
  _mem = fbmem;
  #endif
  
  
  agg_handler = NULL;
  
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
   
    agg_handler = create_render_handler_agg("RGB555");
      
  } else   
  // 16 bits RGB (hicolor)
  if ((var_screeninfo.red.offset=11)
   && (var_screeninfo.red.length==5)
   && (var_screeninfo.green.offset==5)
   && (var_screeninfo.green.length==6)
   && (var_screeninfo.blue.offset==0)
   && (var_screeninfo.blue.length==5) ) {
   
    agg_handler = create_render_handler_agg("RGB565");
      
  } else   
  // 24 bits RGB (truecolor)
  if ((var_screeninfo.red.offset==16)
   && (var_screeninfo.red.length==8)
   && (var_screeninfo.green.offset==8)
   && (var_screeninfo.green.length==8)
   && (var_screeninfo.blue.offset==0)
   && (var_screeninfo.blue.length==8) ) {
   
    agg_handler = create_render_handler_agg("RGB24");
      
  } else   
  // 24 bits BGR (truecolor)
  if ((var_screeninfo.red.offset==0)
   && (var_screeninfo.red.length==8)
   && (var_screeninfo.green.offset==8)
   && (var_screeninfo.green.length==8)
   && (var_screeninfo.blue.offset==16)
   && (var_screeninfo.blue.length==8) ) {
   
    agg_handler = create_render_handler_agg("BGR24");
      
  } else {
    log_error("The pixel format of your framebuffer is not supported.");
    return false;
  }

  assert(agg_handler!=NULL);
  _renderer = agg_handler;

  set_render_handler(agg_handler);
  
  agg_handler->init_buffer(_mem, _size, _width, _height);
  
  disable_terminal();

  return true;
}

bool FBGui::run()
{
  double timer = 0.0;

	while (!terminate_request) {
	
	  double prevtimer = timer; 
		
		while ((timer-prevtimer)*1000 < _interval) {
		
		  usleep(1); // task switch
		  
      struct timeval tv;
      if (!gettimeofday(&tv, NULL))
        timer = (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
    }
	
		Gui::advance_movie(this);
	}
	return true;
}

void FBGui::renderBuffer()
{

	if ( _drawbounds.isNull() ) return; // nothing to do..

	assert ( ! _drawbounds.isWorld() );

#ifdef DOUBLE_BUFFER
  
  // Size of a pixel in bytes
  // NOTE: +7 to support 15 bpp
  const unsigned int pixel_size = (var_screeninfo.bits_per_pixel+7)/8;

  // Size, in bytes, of a framebuffer row
  const unsigned int scanline_size =
    var_screeninfo.xres * pixel_size;
    
  // Size, in bytes, of a row that has to be copied
  const unsigned int row_size = _drawbounds.width() * pixel_size;
    
  // copy each row
  const int minx = _drawbounds.getMinX();
  const int maxy = _drawbounds.getMaxY();
  for (int y=_drawbounds.getMinY(); y<maxy; ++y) {
  
    const unsigned int pixel_index = y * scanline_size + minx*pixel_size;
    
    memcpy(&fbmem[pixel_index], &buffer[pixel_index], row_size);
    
  }  
     

	/*memcpy(fbmem, buffer, var_screeninfo.xres*var_screeninfo.yres*
    (var_screeninfo.bits_per_pixel/8));*/
#endif
	
#ifdef DEBUG_SHOW_FPS
	profile();
#endif
}

bool FBGui::createWindow(const char* /*title*/, int /*width*/, int /*height*/)
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

void FBGui::setInterval(unsigned int interval)
{
	_interval = interval;
}

void FBGui::setTimeout(unsigned int /*timeout*/)
{

}


int FBGui::valid_x(int x) {
  if (x<0) x=0;
  if (x>=m_stage_width) x=m_stage_width-1;
  return x;
}

int FBGui::valid_y(int y) {
  if (y<0) y=0;
  if (y>=m_stage_height) y=m_stage_height-1;
  return y;
}

void FBGui::set_invalidated_region(const rect& bounds) {

#ifdef DOUBLE_BUFFER
  
	// forward to renderer
	_renderer->set_invalidated_region(bounds);

	// update _drawbounds, which are the bounds that need to
	// be rerendered (??)
	//
	_drawbounds = Intersection(
			_renderer->world_to_pixel(bounds),
			_validbounds);
  
	// TODO: add two pixels because of anti-aliasing...

#endif
  
}  // set_invalidated_region

void FBGui::disable_terminal() {
  /*
  --> doesn't work as this hides the cursor of the *current* terminal (which
  --> doesn't have to be the fb one). Maybe just detach from terminal?
  printf("\033[?25l"); 
  fflush(stdout);*/
}

void FBGui::enable_terminal() {
  /*printf("\033[?25h");  
  fflush(stdout);*/
}

// end of namespace gnash
}
