//
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
#include "movie_root.h"

#include "render_handler.h"
#include "render_handler_agg.h"

//#define DEBUG_SHOW_FPS  // prints number of frames per second to STDOUT

#ifdef DEBUG_SHOW_FPS
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
#endif

// workaround until fatal_error() is implemented
// that is not silent without -v switch
//
// The do { } while (0) strangeness here is the only general way to get a
// compound statement that is lexically equivalent to a single one. Example:
//   #define foo() { this(); that(); }
//   if (a) foo();
//   else bar();
// would become
//   if (a) { this(); that() ; } ; else bar()
// which is a syntax error.

#define fatal_error(args ...) \
	do { fprintf(stderr, args); putc('\n', stderr); } while(0)

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
void terminate_signal(int /*signo*/) {
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
	#ifdef DOUBLE_BUFFER
	buffer  = NULL;
	#endif

  input_fd=-1;
	
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
	
	close(input_fd);

  #ifdef DOUBLE_BUFFER
	if (buffer) {
		log_msg("Free'ing offscreen buffer\n");
		free(buffer);
	}
	#endif
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

  // Initialize mouse (don't abort if no mouse found)
  if (!init_mouse()) {
    // just report to the user, keep on going...
    log_msg("You won't have any input device, sorry.");
  }

  // Open the framebuffer device
  fd = open("/dev/fb0", O_RDWR);
  if (fd<0) {
    fatal_error("Could not open framebuffer device: %s", strerror(errno));
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
  
  _validbounds.setTo(0, 0, _width-1, _height-1);
    
  
  #ifdef DOUBLE_BUFFER
  log_msg("Double buffering enabled");
  _mem = buffer;
  #else
  log_msg("Double buffering disabled");
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
  log_msg("Total bits per pixel: %d", var_screeninfo.bits_per_pixel);
  
  /* NOTE: agg_detect_pixel_format() assumes bit positions in host byte order.
  I don't know if this matches the information provided by var_screeninfo, so
  you know what to do when colors look wrong (or pixel format can't be detected)
  on big-endian machines! - Udo */   
  
  const char* pixelformat = agg_detect_pixel_format(
    var_screeninfo.red.offset, var_screeninfo.red.length,
    var_screeninfo.green.offset, var_screeninfo.green.length,
    var_screeninfo.blue.offset, var_screeninfo.blue.length,
    _bpp
  );
  
  if (pixelformat) {    
    agg_handler = create_render_handler_agg(pixelformat);      
  } else {
    fatal_error("The pixel format of your framebuffer is not supported.");
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
  struct timeval tv;

  double timer = 0.0;
  double start_timer;
  
  if (!gettimeofday(&tv, NULL))
    start_timer = (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
  else
    start_timer = 0.0;
    
  
  // let the GUI recompute the x/y scale factors to best fit the whole screen
  resize_view(_validbounds.width(), _validbounds.height());

	while (!terminate_request) {
	
	  double prevtimer = timer; 
		
		while ((timer-prevtimer)*1000 < _interval) {
		
		  usleep(1); // task switch
		  
		  check_mouse(); // TODO: Exit delay loop on mouse events! 
		        
      if (!gettimeofday(&tv, NULL))
        timer = (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
    }
  
	// advance movie	
	Gui::advance_movie(this);
	}
	return true;
}

void FBGui::renderBuffer()
{
	if ( _drawbounds.size() == 0 ) return; // nothing to do..

#ifdef DOUBLE_BUFFER

  // Size of a pixel in bytes
  // NOTE: +7 to support 15 bpp
  const unsigned int pixel_size = (var_screeninfo.bits_per_pixel+7)/8;

  // Size, in bytes, of a framebuffer row
  const unsigned int scanline_size =
    var_screeninfo.xres * pixel_size;
    
    
  for (unsigned int bno=0; bno < _drawbounds.size(); bno++) {
	
		geometry::Range2d<int>& bounds = _drawbounds[bno];
		
		assert ( ! bounds.isWorld() );  
    
	  // Size, in bytes, of a row that has to be copied
	  const unsigned int row_size = (bounds.width()+1) * pixel_size;
	    
	  // copy each row
	  const int minx = bounds.getMinX();
	  const int maxy = bounds.getMaxY();
	  
	  for (int y=bounds.getMinY(); y<=maxy; ++y) {
	  
	    const unsigned int pixel_index = y * scanline_size + minx*pixel_size;
	    
	    memcpy(&fbmem[pixel_index], &buffer[pixel_index], row_size);
	    
	  }
	}  
	     
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

void FBGui::setInvalidatedRegions(const InvalidatedRanges& ranges)
{
	_renderer->set_invalidated_regions(ranges);

	_drawbounds.clear();
		
	for (unsigned int rno=0; rno<ranges.size(); rno++) {
	
		geometry::Range2d<int> bounds = Intersection(
	    _renderer->world_to_pixel(ranges.getRange(rno)),
	    _validbounds);
			
		// it may happen that a particular range is out of the screen, which 
		// will lead to bounds==null. 
		if (bounds.isNull()) continue; 
		
		_drawbounds.push_back(bounds);
	    
	}
	
}

void FBGui::disable_terminal() 
{
  /*
  --> doesn't work as this hides the cursor of the *current* terminal (which
  --> doesn't have to be the fb one). Maybe just detach from terminal?
  printf("\033[?25l"); 
  fflush(stdout);*/
}

void FBGui::enable_terminal() 
{
  /*printf("\033[?25h");  
  fflush(stdout);*/
}

void FBGui::read_mouse_data()
{
  if (input_fd<0) return;   // no mouse available
  
  int count;  
  
  unsigned char *ptr;
  
  ptr = mouse_buf + mouse_buf_size;
  
  count = read(input_fd, mouse_buf + mouse_buf_size, 
    sizeof(mouse_buf) - mouse_buf_size);
    
  if (count<=0) return;
  
  /*
  printf("read data: ");
  int i;
  for (i=0; i<count; i++) 
    printf("%02x ", ptr[i]);
  printf("\n");
  */
  
  mouse_buf_size += count;
  
}

#ifdef USE_MOUSE_PS2  	
bool FBGui::mouse_command(unsigned char cmd, unsigned char *buf, int count) {
  int n;
  
  write(input_fd, &cmd, 1);
  
  while (count>0) {
    usleep(250*1000); // 250 ms inter-char timeout (simple method)
    // TODO: use select() instead
    
    n = read(input_fd, buf, count);
    if (n<=0) return false;
    count-=n;
    buf+=n;
  }
  return true;
  
} //command()
#endif

#ifdef USE_MOUSE_PS2  	
bool FBGui::init_mouse() 
{

  // see http://www.computer-engineering.org/ps2mouse/
  

  // Try to open mouse device, be error tolerant (FD is kept open all the time)
  input_fd = open(MOUSE_DEVICE, O_RDWR);
  
  if (input_fd<0) {
    log_msg("Could not open " MOUSE_DEVICE ": %s", strerror(errno));    
    return false;
  }
  
  unsigned char buf[10], byte;

  if (fcntl(input_fd, F_SETFL, fcntl(input_fd, F_GETFL) | O_NONBLOCK)<0) {
    log_error("Could not set non-blocking mode for mouse device: %s", strerror(errno));
    close(input_fd);
    input_fd=-1;
    return false; 
  }
  
  // Clear input buffer
  while ( read(input_fd, buf, sizeof buf) > 0 ) { }
  
  // Reset mouse
  if ((!mouse_command(0xFF, buf, 3)) || (buf[0]!=0xFA)) {
    log_msg("Mouse reset failed");
    close(input_fd);
    input_fd=-1;
    return false; 
  }
  
  // Enable mouse data reporting
  if ((!mouse_command(0xF4, &byte, 1)) || (byte!=0xFA)) {
    log_msg("Could not activate Data Reporting mode for mouse");
    close(input_fd);
    input_fd=-1;
    return false; 
  }
  
  
  log_msg("Mouse enabled.");
      
  mouse_x = 0;
  mouse_y = 0;
  mouse_btn = 0;
  
  return true;
}
#endif

#ifdef USE_MOUSE_PS2  	
void FBGui::check_mouse() 
{
  if (input_fd<0) return;   // no mouse available
  
  int i;
  int xmove, ymove, btn, btn_changed;
  
  read_mouse_data();
  
  // resync
  int pos = -1;
  for (i=0; i<mouse_buf_size; i++)
  if (mouse_buf[i] & 8) { // bit 3 must be high for the first byte
    pos = i;
    break;    
  }
  if (pos<0) return; // no sync or no data
  
  if (pos>0) {
    // remove garbage:
    memmove(mouse_buf, mouse_buf + pos, mouse_buf_size - pos);
    mouse_buf_size -= pos;
  }
  
  
  if (mouse_buf_size >= 3) {
  
    xmove = mouse_buf[1];
    ymove = mouse_buf[2];
    btn = mouse_buf[0] & 1;
    
    if (mouse_buf[0] & 0x10) xmove = -(256-xmove);
    if (mouse_buf[0] & 0x20) ymove = -(256-ymove);
    
    ymove *= -1; // vertical movement is upside-down
    
    log_msg("x/y %d/%d btn %d", xmove, ymove, btn);

    // movement    
    mouse_x += xmove;
    mouse_y += ymove;
    
    if (mouse_x<0) mouse_x=0;
    if (mouse_y<0) mouse_y=0;
    if (mouse_x>m_stage_width) mouse_x=m_stage_width;
    if (mouse_y>m_stage_height) mouse_y=m_stage_height;
    
    //log_msg("mouse @ %d / %d, btn %d", mouse_x, mouse_y, mouse_btn);
    
    float xscale = getXScale();
    float yscale = getYScale();
    notify_mouse_moved(int(mouse_x / xscale), int(mouse_y / yscale));
    
    // button
    if (btn != mouse_btn) {
      mouse_btn = btn;
      
      notify_mouse_clicked(btn, 1);  // mark=??
      //log_msg("mouse click! %d", btn);
    }    

    // remove from buffer
    pos=3;
    memmove(mouse_buf, mouse_buf + pos, mouse_buf_size - pos);
    mouse_buf_size -= pos;  
    
  
  }
  
}
#endif

#ifdef USE_MOUSE_ETT  	
bool FBGui::init_mouse()
{
  // Try to open mouse device, be error tolerant (FD is kept open all the time)
  input_fd = open(MOUSE_DEVICE, O_RDWR);
  
  if (input_fd<0) {
    log_msg("Could not open " MOUSE_DEVICE ": %s", strerror(errno));    
    return false;
  }
  
  unsigned char buf[10];

  if (fcntl(input_fd, F_SETFL, fcntl(input_fd, F_GETFL) | O_NONBLOCK)<0) {
    log_error("Could not set non-blocking mode for touchpad device: %s", strerror(errno));
    close(input_fd);
    input_fd=-1;
    return false; 
  }
  
  // Clear input buffer
  while ( read(input_fd, buf, sizeof buf) > 0 ) { }
  
  mouse_buf_size=0;
  
  log_msg("Touchpad enabled.");
  return true;
} 
#endif

#ifdef USE_MOUSE_ETT  	
void FBGui::check_mouse() 
{
  if (input_fd<0) return;   // no mouse available
  
  read_mouse_data();
  
  // resync
  int pos = -1;
  int i;
  for (i=0; i<mouse_buf_size; i++)
  if (mouse_buf[i] & 0x80) { 
    pos = i;
    break;    
  }
  if (pos<0) return; // no sync or no data
  
  if (pos>0) {
    //printf("touchscreen: removing %d bytes garbage!\n", pos);  
    memmove(mouse_buf, mouse_buf + pos, mouse_buf_size - pos);
    mouse_buf_size -= pos;
  }
    
  // packet complete?
  while (mouse_buf_size > 4) {
    /*
    eTurboTouch version??
    mouse_btn = ((mouse_buf[0] >> 4) & 1);
    mouse_x = (mouse_buf[1] << 4) | (mouse_buf[2] >> 3);
    mouse_y = (mouse_buf[3] << 4) | (mouse_buf[4] >> 3);
    */

    int new_btn = (mouse_buf[0] & 1);
    int new_x = (mouse_buf[1] << 7) | (mouse_buf[2]);
    int new_y = (mouse_buf[3] << 7) | (mouse_buf[4]);
    
    /*
    printf("touchscreen: %02x %02x %02x %02x %02x | status %d, pos: %d/%d\n",
      mouse_buf[0], mouse_buf[1], mouse_buf[2], mouse_buf[3], mouse_buf[4],   
      new_btn, new_x, new_y);
    */
    
    
    new_x = (int)(((double)new_x - 355) / (1702 - 355) * 1536 + 256);
    new_y = (int)(((double)new_y - 482) / (1771 - 482) * 1536 + 256);
    
    
    new_x = new_x * m_stage_width / 2048;
    new_y = (2048-new_y) * m_stage_height / 2048;
    
    if ((new_x!=mouse_x) || (new_y!=mouse_y)) {
      float xscale = getXScale();
      float yscale = getYScale();
      mouse_x = new_x;
      mouse_y = new_y;
      notify_mouse_moved(int(mouse_x / xscale), int(mouse_y / yscale));
    }
    
    if (new_btn != mouse_btn) {
      mouse_btn = new_btn;      
      notify_mouse_clicked(mouse_btn, 1);  // mask=?
    }
    
    // remove from buffer
    pos=5;
    memmove(mouse_buf, mouse_buf + pos, mouse_buf_size - pos);
    mouse_buf_size -= pos;    
  }
  
}
#endif

// end of namespace gnash
}
