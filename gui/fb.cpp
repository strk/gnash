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

// -----------------------------------------------------------------------------
   
/// \page fb_input FB GUI input devices
///
/// The framebuffer GUI supports various input devices through the modern
/// Linux Input Subsystem (/dev/input/event*). Both standard mice and 
/// touchscreen devices are supported.
///
/// Make sure the USE_INPUT_EVENTS macro is defined in fbsup.h so that
/// the events system is enabled for the FB GUI (this may be configurable
/// at runtime sometime).
///
/// Since there can be multiple input devices in /dev/input/ you have to
/// specify which device to use using the 
///  POINTING_DEVICE environment variable for the mouse and
///  KEYBOARD_DEVICE environment variable for the keyboard
   

/// \page fb_calibration FB GUI Touchscreen Calibration
///
/// The touchscreen drivers (like "usbtouchscreen") provide raw data from the
/// devices. It is up to the user space program to translate this data to
/// screen coordinates. Normally this is done by the X server so this 
/// conversion needs to be done internally by Gnash itself.
///
/// The current implementation uses a very simple 2-point calibration where
/// the first point is at one fifth of the screen width and height and the
/// second point is at the exact opposite part of the screen (at it's lower
/// right). The SWF file calibrate.swf provides a simple graphical reference
/// for 4:3 sized screens (don't use it for other formats!).
/// 
/// With the current preliminary implementation it's a bit uncomfortable to do 
/// the calibration:
/// 
/// 1) starting gnash with DUMP_RAW environment variable will show raw 
///    coordinates on STDOUT:
///      DUMP_RAW=1 gnash calibrate.swf
/// 
/// 2) Keep touching the upper left reference point for a while. You'll get
///    lots of (similar) coordinates printed. Choose a X/Y coordinate pair you 
///    think is the best one (ie. the average) and write it down.
///
/// 3) Do the same for the lower right reference point.
///
/// From now on, start gnash with the TSCALIB enivronment variable set to
/// the coordinates you just found out. Write the coordinates separated by
/// commas (X,Y,X,Y), like this: 
///   TSCALIB=491,1635,1581,639 gnash yourmovie.swf    

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <cerrno>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include "GnashSystemIOHeaders.h"
#include <csignal>
#include <cstdlib> // getenv

#include "gnash.h"
#include "gui.h"
#include "fbsup.h"
#include "log.h"
#include "movie_root.h"

#include "render_handler.h"
#include "render_handler_agg.h"
#include "GnashSleep.h" // for gnashSleep

#include <linux/input.h>    // for /dev/input/event*

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
  fd      = -1;
  fbmem   = NULL;
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
    log_debug(_("Closing framebuffer device"));
    close(fd);
  }
  
  close(input_fd);

  #ifdef DOUBLE_BUFFER
  if (buffer) {
    log_debug(_("Free'ing offscreen buffer"));
    free(buffer);
  }
  #endif
}


bool FBGui::set_grayscale_lut8()
{
  #define TO_16BIT(x) (x | (x<<8))

  struct fb_cmap cmap;
  int i;

  log_debug(_("LUT8: Setting up colormap"));

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
    log_error(_("LUT8: Error setting colormap: %s"), strerror(errno));
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
    log_debug(_("You won't have any pointing input device, sorry."));
  }
  
  // Initialize keyboard (still not critical)
  if (!init_keyboard()) {   
    log_debug(_("You won't have any keyboard input device, sorry."));
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
  log_debug(_("Framebuffer device uses %d bytes of memory."),
    fix_screeninfo.smem_len);
  log_debug(_("Video mode: %dx%d with %d bits per pixel."),
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
  int _width    = var_screeninfo.xres;
  int _height   = var_screeninfo.yres;
  int _bpp = var_screeninfo.bits_per_pixel;
  int _size = fix_screeninfo.smem_len;   // TODO: should recalculate!  
  unsigned char *_mem;
  render_handler_agg_base *agg_handler;
  
  m_stage_width = _width;
  m_stage_height = _height;
  
  _validbounds.setTo(0, 0, _width-1, _height-1);
    
  
  #ifdef DOUBLE_BUFFER
  log_debug(_("Double buffering enabled"));
  _mem = buffer;
  #else
  log_debug(_("Double buffering disabled"));
  _mem = fbmem;
  #endif
  
  
  agg_handler = NULL;
  
  // choose apropriate pixel format
  
  log_debug(_("red channel: %d / %d"), var_screeninfo.red.offset, 
    var_screeninfo.red.length);
  log_debug(_("green channel: %d / %d"), var_screeninfo.green.offset, 
    var_screeninfo.green.length);
  log_debug(_("blue channel: %d / %d"), var_screeninfo.blue.offset, 
    var_screeninfo.blue.length);
  log_debug(_("Total bits per pixel: %d"), var_screeninfo.bits_per_pixel);
  
  const char* pixelformat = agg_detect_pixel_format(
    var_screeninfo.red.offset, var_screeninfo.red.length,
    var_screeninfo.green.offset, var_screeninfo.green.length,
    var_screeninfo.blue.offset, var_screeninfo.blue.length,
    _bpp
  );

  if (pixelformat) {    
    agg_handler = create_render_handler_agg(pixelformat);      
  } else {
    fatal_error("The pixel format of your framebuffer could not be detected.");
    return false;
  }

  assert(agg_handler!=NULL);
  _renderer = agg_handler;

  set_render_handler(agg_handler);
  
  m_rowsize = var_screeninfo.xres_virtual*((_bpp+7)/8);
  
  agg_handler->init_buffer(_mem, _size, _width, _height, m_rowsize);
  
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
    
      gnashSleep(1); // task switch
      
      check_mouse(); // TODO: Exit delay loop on mouse events! 
      check_keyboard(); // TODO: Exit delay loop on keyboard events!
            
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

    
  for (unsigned int bno=0; bno < _drawbounds.size(); bno++) {
  
    geometry::Range2d<int>& bounds = _drawbounds[bno];
    
    assert ( ! bounds.isWorld() );  
    
    // Size, in bytes, of a row that has to be copied
    const unsigned int row_size = (bounds.width()+1) * pixel_size;
      
    // copy each row
    const int minx = bounds.getMinX();
    const int maxy = bounds.getMaxY();
    
    for (int y=bounds.getMinY(); y<=maxy; ++y) {
    
      const unsigned int pixel_index = y * m_rowsize + minx*pixel_size;
      
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

void FBGui::setFullscreen()
{
  // FB GUI always runs fullscreen; ignore...
}

void FBGui::unsetFullscreen()
{
  // FB GUI always runs fullscreen; ignore...
}

void FBGui::showMenu(bool /*show*/)
{
  log_unimpl(_("This GUI does not yet support menus"));
}

bool FBGui::showMouse(bool /*show*/)
{
  log_unimpl(_("This GUI does not yet support a mouse pointer"));
  // Should return true if the pointer was visible before call,
  // otherwise false;
  return true;
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

char* FBGui::find_accessible_tty(int no) {

  char* fn;
  
  fn = find_accessible_tty("/dev/vc/%d", no);   if (fn) return fn;
  fn = find_accessible_tty("/dev/tty%d", no);   if (fn) return fn;
  fn = find_accessible_tty("/dev/tty%02x", no); if (fn) return fn;
  fn = find_accessible_tty("/dev/tty%x", no);   if (fn) return fn;
  fn = find_accessible_tty("/dev/tty%02d", no); if (fn) return fn;
  
  if (no==0) {
    fn = find_accessible_tty("/dev/tty", no);  // just "/dev/tty" 
    if (fn) return fn;
  }
    
  return NULL;

}

char* FBGui::find_accessible_tty(const char* format, int no) {

  static char fname[1024];
  
  snprintf(fname, sizeof fname, format, no);
    
  if (access(fname, R_OK|W_OK) != -1) {
    return fname;
  }

  return NULL; 
}

bool FBGui::disable_terminal() 
{
  original_kd = -1;
  
  struct vt_stat vts;
  
  // Find the TTY device name
  
  char* tty = find_accessible_tty(0);
  
  int fd;
  
  if (!tty) {
    log_debug(_("WARNING: Could not detect controlling TTY"));
    return false;
  }
  
  
  // Detect the currently active virtual terminal (so we can switch back to
  // it later)
  
  fd = open(tty, O_RDWR);
  if (fd<0) {
    log_debug(_("WARNING: Could not open %s"), tty);
    return false;
  }
  
  if (ioctl(fd, VT_GETSTATE, &vts) == -1) {
    log_debug(_("WARNING: Could not get current VT state"));
    close(fd);
    return false;
  }
    
  original_vt = vts.v_active;
  log_debug(_("Original TTY NO = %d"), original_vt);   
  
#ifdef REQUEST_NEW_VT

  // Request a new VT number
  if (ioctl(fd, VT_OPENQRY, &own_vt) == -1) {
    log_debug(_("WARNING: Could not request a new VT"));
    close(fd);
    return false;
  }
  
  log_debug(_("Own TTY NO = %d"), own_vt);
  
  close(fd);
  
  // Activate our new VT
  tty = find_accessible_tty(own_vt);
  if (!tty) {
    log_debug(_("WARNING: Could not find device for VT number %d"), own_vt);
    return false;
  }
  
  fd = open(tty, O_RDWR);
  if (fd<0) {
    log_debug(_("WARNING: Could not open %s"), tty);
    return false;
  }
  
  if (ioctl(fd, VT_ACTIVATE, own_vt) == -1) {
    log_debug(_("WARNING: Could not activate VT number %d"), own_vt);
    close(fd);
    return false;
  }
  
  if (ioctl(fd, VT_WAITACTIVE, own_vt) == -1) {
    log_debug(_("WARNING: Error waiting for VT %d becoming active"), own_vt);
    //close(tty);
    //return false;   don't abort
  }

#else

  own_vt = original_vt;   // keep on using the original VT
  
  close(fd);
  
  // Activate our new VT
  tty = find_accessible_tty(own_vt);
  if (!tty) {
    log_debug(_("WARNING: Could not find device for VT number %d"), own_vt);
    return false;
  }
  
  fd = open(tty, O_RDWR);
  if (fd<0) {
    log_debug(_("WARNING: Could not open %s"), tty);
    return false;
  }
  
  /*
  // Become session leader and attach to terminal
  setsid();
  if (ioctl(fd, TIOCSCTTY, 0) == -1) {
    log_debug(_("WARNING: Could not attach controlling terminal (%s)"), tty);
  }
  */
  

#endif  
  
  // Disable keyboard cursor
  
  if (ioctl(fd, KDGETMODE, &original_kd) == -1) {
    log_debug(_("WARNING: Could not query current keyboard mode on VT"));
  }

  if (ioctl(fd, KDSETMODE, KD_GRAPHICS) == -1) {
    log_debug(_("WARNING: Could not switch to graphics mode on new VT"));
  }
   
  close(fd);
  
  log_debug(_("VT %d ready"), own_vt);
  
  
  // NOTE: We could also implement virtual console switching by using 
  // VT_GETMODE / VT_SETMODE ioctl calls and handling their signals, but
  // probably nobody will ever want to switch consoles, so I don't bother... 
  
  return true;
}

bool FBGui::enable_terminal() 
{

  log_debug(_("Restoring terminal..."));

  char* tty = find_accessible_tty(own_vt);
  if (!tty) {
    log_debug(_("WARNING: Could not find device for VT number %d"), own_vt);
    return false;
  }

  int fd = open(tty, O_RDWR);
  if (fd<0) {
    log_debug(_("WARNING: Could not open %s"), tty);
    return false;
  }

  if (ioctl(fd, VT_ACTIVATE, original_vt)) {
    log_debug(_("WARNING: Could not activate VT number %d"), original_vt);
    close(fd);
    return false;
  }

  if (ioctl(fd, VT_WAITACTIVE, original_vt)) {
    log_debug(_("WARNING: Error waiting for VT %d becoming active"), original_vt);
    //close(tty);
    //return false;   don't abort
  }

  
  
  // Restore keyboard
  
  if (ioctl(fd, KDSETMODE, original_kd)) {
    log_debug(_("WARNING: Could not restore keyboard mode"));
  }  
  
  close(fd);
  
  return true;
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
  
  // flush input buffer
  char trash[16];
  do {
    n = read(input_fd, trash, sizeof trash);
    if (n>0) 
      log_debug(_("mouse_command: discarded %d bytes from input buffer"), n);
  } while (n>0);
  
  // send command
  write(input_fd, &cmd, 1);
  
  // read response (if any)
  while (count>0) {
    gnashSleep(250*1000); // 250 ms inter-char timeout (simple method)
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
    log_debug(_("Could not open " MOUSE_DEVICE ": %s"), strerror(errno));    
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
    log_debug(_("Mouse reset failed"));
    close(input_fd);
    input_fd=-1;
    return false; 
  }
  
  // Get Device ID (not crucial, debug only)
  if ((!mouse_command(0xF2, buf, 2)) || (buf[0]!=0xFA)) {
    log_debug(_("WARNING: Could not detect mouse device ID"));
  } else {
    unsigned char devid = buf[1];
    if (devid!=0)
      log_debug(_("WARNING: Non-standard mouse device ID %d"), devid);
  }
  
  // Enable mouse data reporting
  if ((!mouse_command(0xF4, &byte, 1)) || (byte!=0xFA)) {
    log_debug(_("Could not activate Data Reporting mode for mouse"));
    close(input_fd);
    input_fd=-1;
    return false; 
  }
  
  
  log_debug(_("Mouse enabled."));
      
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
    
    log_debug(_("x/y %d/%d btn %d"), xmove, ymove, btn);

    // movement    
    mouse_x += xmove;
    mouse_y += ymove;
    
    if (mouse_x<0) mouse_x=0;
    if (mouse_y<0) mouse_y=0;
    if (mouse_x>m_stage_width) mouse_x=m_stage_width;
    if (mouse_y>m_stage_height) mouse_y=m_stage_height;
    
    //log_debug(_("mouse @ %d / %d, btn %d"), mouse_x, mouse_y, mouse_btn);
    
    notify_mouse_moved(mouse_x, mouse_y);
    
    // button
    if (btn != mouse_btn) {
      mouse_btn = btn;
      
      notify_mouse_clicked(btn, 1);  // mark=??
      //log_debug(_("mouse click! %d"), btn);
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
    log_debug(_("Could not open " MOUSE_DEVICE ": %s"), strerror(errno));    
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
  
  log_debug(_("Touchpad enabled."));
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
      mouse_x = new_x;
      mouse_y = new_y;
      notify_mouse_moved(mouse_x, mouse_y);
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

#ifdef USE_INPUT_EVENTS   
bool FBGui::init_mouse()
{
  std::string dev;

  char* devname = std::getenv("POINTING_DEVICE");
  if (devname) dev = devname;
  else dev = "/dev/input/event0";

  // Try to open mouse device, be error tolerant (FD is kept open all the time)
  input_fd = open(dev.c_str(), O_RDONLY);
  
  if (input_fd<0) {
    log_debug(_("Could not open %s: %s"), dev.c_str(), strerror(errno));    
    return false;
  }
  
  log_debug(_("Pointing device %s open"), dev.c_str());
  
  if (fcntl(input_fd, F_SETFL, fcntl(input_fd, F_GETFL) | O_NONBLOCK)<0) {
    log_error(_("Could not set non-blocking mode for pointing device: %s"), strerror(errno));
    close(input_fd);
    input_fd=-1;
    return false; 
  }
  
  return true;

} //init_mouse

void FBGui::apply_ts_calibration(float* cx, float* cy, int rawx, int rawy) {

  /*
  <UdoG>:
  This is a *very* simple method to translate raw touchscreen coordinates to
  the screen coordinates. We simply to linear interpolation between two points.
  Note this won't work well when the touchscreen is not perfectly aligned to
  the screen (ie. slightly rotated). Standard touchscreen calibration uses
  5 calibration points (or even 25). If someone can give me the formula, tell
  me! I'm too lazy right now to do the math myself... ;)  
  
  And sorry for the quick-and-dirty implementation! I'm in a hurry...
  */

  float ref1x = m_stage_width  / 5 * 1;
  float ref1y = m_stage_height / 5 * 1;
  float ref2x = m_stage_width  / 5 * 4;
  float ref2y = m_stage_height / 5 * 4;
  
  static float cal1x = 2048/5*1;   // very approximative default values
  static float cal1y = 2048/5*4;
  static float cal2x = 2048/5*4;
  static float cal2y = 2048/5*1;
  
  static bool initialized=false; // woohooo.. better don't look at this code...
  if (!initialized) {
    initialized=true;
    
    char* settings = std::getenv("TSCALIB");
    
    if (settings) {
    
      // expected format: 
      // 491,1635,1581,646      (cal1x,cal1y,cal2x,cal2y; all integers)

      char buffer[1024];      
      char* p1;
      char* p2;
      bool ok = false;
      
      snprintf(buffer, sizeof buffer, "%s", settings);
      p1 = buffer;
      
      do {
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
      
      log_debug(_("Using touchscreen calibration data: %.0f / %.0f / %.0f / %.0f"),
        cal1x, cal1y, cal2x, cal2y);
    
    } else {
      log_debug(_("WARNING: No touchscreen calibration settings found. "
        "The mouse pointer most probably won't work precisely. Set "
        "TSCALIB environment variable with correct values for better results"));
    }
    
  } //!initialized


  // real duty: 
  *cx = (rawx-cal1x) / (cal2x-cal1x) * (ref2x-ref1x) + ref1x;
  *cy = (rawy-cal1y) / (cal2y-cal1y) * (ref2y-ref1y) + ref1y;
}

void FBGui::check_mouse()
{

  if (input_fd < 0) return;

  struct input_event ev;  // time,type,code,value
  
  static int new_mouse_x = 0; // all uncalibrated!
  static int new_mouse_y = 0;
  static int new_mouse_btn = 0;
  
  int notify_x=0;     // coordinate to be sent via notify_mouse_moved()
  int notify_y=0;
  bool move_pending = false;  // true: notify_mouse_moved() should be called
  
  // this is necessary for our quick'n'dirty touchscreen calibration: 
  static int coordinatedebug = std::getenv("DUMP_RAW")!=NULL;
  
  // The while loop is limited because the kernel tends to send us hundreds
  // of events while the touchscreen is touched. We don't loose any 
  // information if we stop reading because the kernel will stop
  // sending redundant information.
  int loops=0;  
  
  // Assuming we will never read less than one full struct...  
  
  while ((loops++ < 100) && (read(input_fd, &ev, sizeof ev) == (sizeof ev))) {
  
    if (ev.type == EV_SYN) {    // synchronize (apply information)
    
      if ((new_mouse_x != mouse_x) || (new_mouse_y != mouse_y)) {
      
        mouse_x = new_mouse_x;
        mouse_y = new_mouse_y;
        
        float cx, cy;
        
        if (std::getenv("TSCALIB"))  // ONLY convert when requested
          apply_ts_calibration(&cx, &cy, mouse_x, mouse_y);
        else
          { cx=mouse_x; cy=mouse_y; }
              
        // Don't call notify_mouse_moved() here because this would lead to
        // lots of calls, especially for touchscreens. Instead we save the
        // coordinate and call notify_mouse_moved() only once.
        notify_x = cx;
        notify_y = cy;
        move_pending = true;        
      }
      
      if (new_mouse_btn != mouse_btn) {
      
        if (move_pending) {
          notify_mouse_moved(notify_x, notify_y);
          move_pending = false;
        }
      
        mouse_btn = new_mouse_btn;
        notify_mouse_clicked(mouse_btn, 1);  // mark=??
      }

      if (coordinatedebug)
        printf("% 5d / % 5d / % 5d\n", mouse_x, mouse_y, mouse_btn);
      
    }
  
    if (ev.type == EV_KEY) {    // button down/up
    
      // don't care which button, we support only one...
      new_mouse_btn = ev.value;      
      
    }
      
    if (ev.type == EV_ABS) {    // absolute coordinate
      if (ev.code == ABS_X) new_mouse_x = ev.value;
      if (ev.code == ABS_Y) new_mouse_y = ev.value;
    }
    
    if (ev.type == EV_REL) {    // relative movement
      if (ev.code == REL_X) new_mouse_x += ev.value;
      if (ev.code == REL_Y) new_mouse_y += ev.value;
      
      if (new_mouse_x < 0) new_mouse_x=0;
      if (new_mouse_y < 0) new_mouse_y=0;
      
      if (new_mouse_x > m_stage_width ) new_mouse_x = m_stage_width;
      if (new_mouse_y > m_stage_height) new_mouse_y = m_stage_height;
    }      
  
  } 
  
  if (move_pending) 
    notify_mouse_moved(notify_x, notify_y);
 
} //check_mouse
#endif


bool FBGui::init_keyboard() 
{
  std::string dev;

  char* devname = std::getenv("KEYBOARD_DEVICE");
  if (devname) dev = devname;
  else dev = "/dev/input/event0";

  // Try to open keyboard device, be error tolerant (FD is kept open all the time)
  keyb_fd = open(dev.c_str(), O_RDONLY);
  
  if (keyb_fd<0) {
    log_debug(_("Could not open %s: %s"), dev.c_str(), strerror(errno));    
    return false;
  }
  
  log_debug(_("Keyboard device %s open"), dev.c_str());
  
  if (fcntl(keyb_fd, F_SETFL, fcntl(keyb_fd, F_GETFL) | O_NONBLOCK)<0) {
    log_error(_("Could not set non-blocking mode for keyboard device: %s"), strerror(errno));
    close(keyb_fd);
    keyb_fd=-1;
    return false; 
  }
  
  return true;
}

gnash::key::code FBGui::scancode_to_gnash_key(int code, bool shift) {
 
  // NOTE: Scancodes are mostly keyboard oriented (ie. Q, W, E, R, T, ...)
  // while Gnash codes are mostly ASCII-oriented (A, B, C, D, ...) so no
  // direct conversion is possible.
  
  // TODO: This is a very *incomplete* list and I also dislike this method
  // very much because it depends on the keyboard layout (ie. pressing "Z"
  // on a german keyboard will print "Y" instead). So we need some 
  // alternative...
  
  switch (code) {
  
    case KEY_1      : return !shift ? gnash::key::_1 : gnash::key::EXCLAM;
    case KEY_2      : return !shift ? gnash::key::_2 : gnash::key::DOUBLE_QUOTE; 
    case KEY_3      : return !shift ? gnash::key::_3 : gnash::key::HASH; 
    case KEY_4      : return !shift ? gnash::key::_4 : gnash::key::DOLLAR; 
    case KEY_5      : return !shift ? gnash::key::_5 : gnash::key::PERCENT; 
    case KEY_6      : return !shift ? gnash::key::_6 : gnash::key::AMPERSAND; 
    case KEY_7      : return !shift ? gnash::key::_7 : gnash::key::SINGLE_QUOTE; 
    case KEY_8      : return !shift ? gnash::key::_8 : gnash::key::PAREN_LEFT; 
    case KEY_9      : return !shift ? gnash::key::_9 : gnash::key::PAREN_RIGHT; 
    case KEY_0      : return !shift ? gnash::key::_0 : gnash::key::ASTERISK;
                            
    case KEY_A      : return shift ? gnash::key::A : gnash::key::a;
    case KEY_B      : return shift ? gnash::key::B : gnash::key::b;
    case KEY_C      : return shift ? gnash::key::C : gnash::key::c;
    case KEY_D      : return shift ? gnash::key::D : gnash::key::d;
    case KEY_E      : return shift ? gnash::key::E : gnash::key::e;
    case KEY_F      : return shift ? gnash::key::F : gnash::key::f;
    case KEY_G      : return shift ? gnash::key::G : gnash::key::g;
    case KEY_H      : return shift ? gnash::key::H : gnash::key::h;
    case KEY_I      : return shift ? gnash::key::I : gnash::key::i;
    case KEY_J      : return shift ? gnash::key::J : gnash::key::j;
    case KEY_K      : return shift ? gnash::key::K : gnash::key::k;
    case KEY_L      : return shift ? gnash::key::L : gnash::key::l;
    case KEY_M      : return shift ? gnash::key::M : gnash::key::m;
    case KEY_N      : return shift ? gnash::key::N : gnash::key::n;
    case KEY_O      : return shift ? gnash::key::O : gnash::key::o;
    case KEY_P      : return shift ? gnash::key::P : gnash::key::p;
    case KEY_Q      : return shift ? gnash::key::Q : gnash::key::q;
    case KEY_R      : return shift ? gnash::key::R : gnash::key::r;
    case KEY_S      : return shift ? gnash::key::S : gnash::key::s;
    case KEY_T      : return shift ? gnash::key::T : gnash::key::t;
    case KEY_U      : return shift ? gnash::key::U : gnash::key::u;
    case KEY_V      : return shift ? gnash::key::V : gnash::key::v;
    case KEY_W      : return shift ? gnash::key::W : gnash::key::w;
    case KEY_X      : return shift ? gnash::key::X : gnash::key::x;
    case KEY_Y      : return shift ? gnash::key::Y : gnash::key::y;
    case KEY_Z      : return shift ? gnash::key::Z : gnash::key::z;

    case KEY_F1     : return gnash::key::F1; 
    case KEY_F2     : return gnash::key::F2; 
    case KEY_F3     : return gnash::key::F3; 
    case KEY_F4     : return gnash::key::F4; 
    case KEY_F5     : return gnash::key::F5; 
    case KEY_F6     : return gnash::key::F6; 
    case KEY_F7     : return gnash::key::F7; 
    case KEY_F8     : return gnash::key::F8; 
    case KEY_F9     : return gnash::key::F9;
    case KEY_F10    : return gnash::key::F10;
    case KEY_F11    : return gnash::key::F11;
    case KEY_F12    : return gnash::key::F12;
    
    case KEY_KP0    : return gnash::key::KP_0; 
    case KEY_KP1    : return gnash::key::KP_1; 
    case KEY_KP2    : return gnash::key::KP_2; 
    case KEY_KP3    : return gnash::key::KP_3; 
    case KEY_KP4    : return gnash::key::KP_4; 
    case KEY_KP5    : return gnash::key::KP_5; 
    case KEY_KP6    : return gnash::key::KP_6; 
    case KEY_KP7    : return gnash::key::KP_7; 
    case KEY_KP8    : return gnash::key::KP_8; 
    case KEY_KP9    : return gnash::key::KP_9;

    case KEY_KPMINUS       : return gnash::key::KP_SUBTRACT;
    case KEY_KPPLUS        : return gnash::key::KP_ADD;
    case KEY_KPDOT         : return gnash::key::KP_DECIMAL;
    case KEY_KPASTERISK    : return gnash::key::KP_MULITPLY;  // typo in GnashKey.h
    case KEY_KPENTER       : return gnash::key::KP_ENTER;
    
    case KEY_ESC           : return gnash::key::ESCAPE;
    case KEY_MINUS         : return gnash::key::MINUS;
    case KEY_EQUAL         : return gnash::key::EQUALS;
    case KEY_BACKSPACE     : return gnash::key::BACKSPACE;
    case KEY_TAB           : return gnash::key::TAB;
    case KEY_LEFTBRACE     : return gnash::key::LEFT_BRACE;
    case KEY_RIGHTBRACE    : return gnash::key::RIGHT_BRACE;
    case KEY_ENTER         : return gnash::key::ENTER;
    case KEY_LEFTCTRL      : return gnash::key::CONTROL;
    case KEY_SEMICOLON     : return gnash::key::SEMICOLON;
    //case KEY_APOSTROPHE    : return gnash::key::APOSTROPHE;  
    //case KEY_GRAVE         : return gnash::key::GRAVE;
    case KEY_LEFTSHIFT     : return gnash::key::SHIFT;
    case KEY_BACKSLASH     : return gnash::key::BACKSLASH;
    case KEY_COMMA         : return gnash::key::COMMA;
    case KEY_SLASH         : return gnash::key::SLASH;
    case KEY_RIGHTSHIFT    : return gnash::key::SHIFT;
    case KEY_LEFTALT       : return gnash::key::ALT;
    case KEY_SPACE         : return gnash::key::SPACE;
    case KEY_CAPSLOCK      : return gnash::key::CAPSLOCK;
    case KEY_NUMLOCK       : return gnash::key::NUM_LOCK;
    //case KEY_SCROLLLOCK    : return gnash::key::SCROLLLOCK;
    
    case KEY_UP            : return gnash::key::UP;
    case KEY_DOWN          : return gnash::key::DOWN;
    case KEY_LEFT          : return gnash::key::LEFT;
    case KEY_RIGHT         : return gnash::key::RIGHT;
    case KEY_PAGEUP        : return gnash::key::PGUP;
    case KEY_PAGEDOWN      : return gnash::key::PGDN;
    case KEY_INSERT        : return gnash::key::INSERT;
    case KEY_DELETE        : return gnash::key::DELETEKEY;
    case KEY_HOME          : return gnash::key::HOME;
    case KEY_END           : return gnash::key::END;
    
  }
  
  return gnash::key::INVALID;  
}

void FBGui::check_keyboard()
{

  if (keyb_fd < 0) return;

  struct input_event ev;  // time,type,code,value

  
  while (read(keyb_fd, &ev, sizeof ev) == (sizeof ev)) {
  
    if (ev.type == EV_KEY) {
    
      // code == scan code of the key (KEY_xxxx defines in input.h)
      
      // value == 0  key has been released
      // value == 1  key has been pressed
      // value == 2  repeated key reporting (while holding the key) 

      if (ev.code==KEY_LEFTSHIFT) 
        keyb_lshift = ev.value;
      else
      if (ev.code==KEY_RIGHTSHIFT) 
        keyb_rshift = ev.value;
      else
      if (ev.code==KEY_LEFTCTRL) 
        keyb_lctrl = ev.value;
      else
      if (ev.code==KEY_RIGHTCTRL) 
        keyb_rctrl = ev.value;
      else
      if (ev.code==KEY_LEFTALT) 
        keyb_lalt = ev.value;
      else
      if (ev.code==KEY_RIGHTALT) 
        keyb_ralt = ev.value;
      else {
      
        gnash::key::code  c = scancode_to_gnash_key(ev.code, 
          keyb_lshift || keyb_rshift);
      
        // build modifier
      
        int modifier = gnash::key::GNASH_MOD_NONE;
        
        if (keyb_lshift || keyb_rshift)
          modifier = modifier | gnash::key::GNASH_MOD_SHIFT;

        if (keyb_lctrl || keyb_rctrl)
          modifier = modifier | gnash::key::GNASH_MOD_CONTROL;

        if (keyb_lalt || keyb_ralt)
          modifier = modifier | gnash::key::GNASH_MOD_ALT;
          
          
        // send event
        if (c != gnash::key::INVALID) 
            Gui::notify_key_event(c, modifier, ev.value);
              
      } //if normal key

    } //if EV_KEY      
  
  } //while

}

// end of namespace gnash
}
