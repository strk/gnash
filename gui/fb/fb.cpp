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

// -----------------------------------------------------------------------------
   
/// \page fb_input FB GUI input devices
///
/// The framebuffer GUI supports various input devices through the modern
/// Linux Input Subsystem (/dev/input/event*). Both standard mice and 
/// touchscreen devices are supported.
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

///
/// 10-4-2008  N. Coesel
/// Added support for tslib. Tslib is a library which allows to create
/// a stack / cascade of filters to filter and scale touch-screen output.
/// The source doesn't come with much documentation, but writing your
/// own filter based on an existing filter is really easy. Tslib can 
/// deal with old style H3600 style touchscreens and the newer event 
/// interface. See http://tslib.berlios.de/ to get the source. 
///
/// The check_tslib() routine assumes filtering for movement and presses
/// is properly setup. Scaling is supposed to be performed by tslib or
/// the underlying touchscreen driver.

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "GnashSystemIOHeaders.h"
#include "GnashNumeric.h"

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
#include <csignal>
#include <cstdlib> // getenv

#ifdef HAVE_TSLIB_H
# include <tslib.h>
#endif
#if defined(ENABLE_TSLIB) && !defined(HAVE_TSLIB_H)
# warning "No tslib.h! Disabling touchscreen support"
# undef ENABLE_TSLIB
#endif

#include "gui.h"
#include "fbsup.h"
#include "log.h"
#include "movie_root.h"
#include "RunResources.h"

#include "Renderer.h"
#include "Renderer_agg.h"
#include "GnashSleep.h" // for gnashSleep

#include <linux/input.h>    // for /dev/input/event*


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

int terminate_request = false;  // global scope to avoid GUI access

/// Called on CTRL-C and alike
void
terminate_signal(int /*signo*/) {
    terminate_request = true;
}

FBGui::FBGui(unsigned long xid, float scale, bool loop, RunResources& r)
    : Gui(xid, scale, loop, r),
      fd(-1),
      original_vt(-1),
      original_kd(-1),
      own_vt(-1),
      fbmem(0),
      buffer(0),                // the real value is set by ENABLE_DOUBLE_BUFFERING
      _xpos(0),
      _ypos(0),  
      m_rowsize(0),
      _timeout(0)
{
    // initializing to zero helps with debugging and prevents weird bugs
//    memset(mouse_buf, 0, 256);
    memset(&var_screeninfo, 0, sizeof(fb_var_screeninfo));
    memset(&fix_screeninfo, 0, sizeof(fb_fix_screeninfo));

    signal(SIGINT, terminate_signal);
    signal(SIGTERM, terminate_signal);
}

FBGui::~FBGui()
{  
    if (fd > 0) {
        enable_terminal();
        log_debug(_("Closing framebuffer device"));
        close(fd);
    }

#ifdef ENABLE_DOUBLE_BUFFERING
    if (buffer) {
        log_debug(_("Free'ing offscreen buffer"));
        free(buffer);
    }
#endif
}

bool
FBGui::set_grayscale_lut8()
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

#ifdef ENABLE_FAKE_FRAMEBUFFER
    if (fakefb_ioctl(fd, FBIOPUTCMAP, &cmap))
#else
    if (ioctl(fd, FBIOPUTCMAP, &cmap))
#endif
    {
        log_error(_("LUT8: Error setting colormap: %s"), strerror(errno));
        return false;
    }

    return true;

#undef TO_16BIT
}

bool
FBGui::init(int argc, char *** argv)
{
    // GNASH_REPORT_FUNCTION;

    // Initialize all the input devices

    // Look for Mice that use the PS/2 mouse protocol
    _inputs = InputDevice::scanForDevices(this);
    if (_inputs.empty()) {
        log_error("Found no accessible input event devices");
    }
    
    // Open the framebuffer device
#ifdef ENABLE_FAKE_FRAMEBUFFER
    fd = open(FAKEFB, O_RDWR);
#else
    fd = open("/dev/fb0", O_RDWR);
#endif
    if (fd < 0) {
        fatal_error("Could not open framebuffer device: %s", strerror(errno));
        return false;
    }
  
    // Load framebuffer properties
#ifdef ENABLE_FAKE_FRAMEBUFFER
    fakefb_ioctl(fd, FBIOGET_VSCREENINFO, &var_screeninfo);
    fakefb_ioctl(fd, FBIOGET_FSCREENINFO, &fix_screeninfo);
#else
    ioctl(fd, FBIOGET_VSCREENINFO, &var_screeninfo);
    ioctl(fd, FBIOGET_FSCREENINFO, &fix_screeninfo);
#endif
    log_debug(_("Framebuffer device uses %d bytes of memory."),
              fix_screeninfo.smem_len);
    log_debug(_("Video mode: %dx%d with %d bits per pixel."),
              var_screeninfo.xres, var_screeninfo.yres, var_screeninfo.bits_per_pixel);

    // map framebuffer into memory
    fbmem = (unsigned char *)
        mmap(0, fix_screeninfo.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

#ifdef ENABLE_DOUBLE_BUFFERING
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

    // Set "window" size
    _width    = var_screeninfo.xres;
    _height   = var_screeninfo.yres;

    // Let -j -k override "window" size
    optind = 0; opterr = 0; char c;
    while ((c = getopt (argc, *argv, "j:k:X:Y:")) != -1) {
        switch (c) {
            case 'j':
                _width = clamp<int>(atoi(optarg), 1, _width);
                break;
            case 'k':
                _height = clamp<int>(atoi(optarg), 1, _height);
                break;
            case 'X':
                _xpos = atoi(optarg);
                break;
            case 'Y':
                _ypos = atoi(optarg);
                break;
        }
    }

    if ( _xpos < 0 ) _xpos += var_screeninfo.xres - _width;
    _xpos = clamp<int>(_xpos, 0, var_screeninfo.xres-_width);

    if ( _ypos < 0 ) _ypos += var_screeninfo.yres - _height;
    _ypos = clamp<int>(_ypos, 0, var_screeninfo.yres-_height);

    log_debug("Width:%d, Height:%d", _width, _height);
    log_debug("X:%d, Y:%d", _xpos, _ypos);

    _validbounds.setTo(0, 0, _width - 1, _height - 1);    

    return true;
}

bool
FBGui::initialize_renderer()
{
    // GNASH_REPORT_FUNCTION;

    const int bpp = var_screeninfo.bits_per_pixel;
    const int size = fix_screeninfo.smem_len; 

    // TODO: should recalculate!  
    unsigned char* mem;
    Renderer_agg_base* agg_handler;
  
#ifdef ENABLE_DOUBLE_BUFFERING
    log_debug(_("Double buffering enabled"));
    mem = buffer;
#else
    log_debug(_("Double buffering disabled"));
    mem = fbmem;
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
        bpp
        );

    if (pixelformat) {    
        agg_handler = create_Renderer_agg(pixelformat);      
    } else {
        fatal_error("The pixel format of your framebuffer could not be detected.");
        return false;
    }

    assert(agg_handler);

    _renderer.reset(agg_handler);

    _runResources.setRenderer(_renderer);
  
    m_rowsize = var_screeninfo.xres_virtual*((bpp+7)/8);
  
    agg_handler->init_buffer(mem, size, _width, _height, m_rowsize);
  
    disable_terminal();

    return true;
}

bool
FBGui::run()
{

    VirtualClock& timer = getClock();
    
    // let the GUI recompute the x/y scale factors to best fit the whole screen
    resize_view(_validbounds.width(), _validbounds.height());

    // This loops endlessly at the frame rate
    while (!terminate_request) {  
        // wait the "heartbeat" inteval
        gnashSleep(_interval * 1000);    
        // TODO: Do we need to check the real time slept or is it OK when we woke
        // up early because of some Linux signal sent to our process (and thus
        // "advance" faster than the "heartbeat" interval)? - Udo

        // check input devices
        checkForData();
        
        // advance movie  
        Gui::advance_movie(this);

        // check if we've reached a timeout
        if (_timeout && timer.elapsed() >= _timeout ) {
            break;
        }
    }
  
    return true;
}

void
FBGui::renderBuffer()
{
    // GNASH_REPORT_FUNCTION;

    if ( _drawbounds.size() == 0 ) return; // nothing to do..

#ifdef ENABLE_DOUBLE_BUFFERING

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

        const int minx1 = minx+_xpos;
        for (int y=bounds.getMinY(), y1=y+_ypos; y<=maxy; ++y, ++y1) {
            const unsigned int pix_idx_in = y*m_rowsize + minx*pixel_size;
            const unsigned int pix_idx_out = y1*m_rowsize + minx1*pixel_size;
            memcpy(&fbmem[pix_idx_out], &buffer[pix_idx_in], row_size);
        }
    }  
       
#endif
  
}

bool
FBGui::createWindow(const char* /*title*/, int /*width*/, int /*height*/,
                     int /*xPosition*/, int /*yPosition*/)
{
    // Now initialize AGG
    return initialize_renderer();
}

bool
FBGui::createMenu()
{
    // no menu support! 
    return true;
}

bool
FBGui::setupEvents()
{
    // events currently not supported!
    return true;
}

void
FBGui::setInterval(unsigned int interval)
{
    _interval = interval;
}

void
FBGui::setTimeout(unsigned int timeout)
{
    _timeout = timeout;
}

void
FBGui::setFullscreen()
{
    // FB GUI always runs fullscreen; ignore...
}

void
FBGui::unsetFullscreen()
{
  // FB GUI always runs fullscreen; ignore...
}

void
FBGui::showMenu(bool /*show*/)
{
    log_unimpl(_("This GUI does not yet support menus"));
}

bool
FBGui::showMouse(bool /*show*/)
{
    log_unimpl(_("This GUI does not yet support a mouse pointer"));
    // Should return true if the pointer was visible before call,
    // otherwise false;
    return true;
}

void
FBGui::setInvalidatedRegions(const InvalidatedRanges& ranges)
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

char *
FBGui::find_accessible_tty(int no)
{
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

char *
FBGui::find_accessible_tty(const char* format, int no)
{
    static char fname[1024];
    
    snprintf(fname, sizeof fname, format, no);
    
    if (access(fname, R_OK|W_OK) != -1) {
        return fname;
    }
    
    return NULL; 
}

bool
FBGui::disable_terminal() 
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
    if (fd < 0) {
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

    if (fd > 0) {
        close(fd);
    }
  
    // Activate our new VT
    tty = find_accessible_tty(own_vt);
    if (!tty) {
        log_debug(_("WARNING: Could not find device for VT number %d"), own_vt);
        return false;
    }
  
    fd = open(tty, O_RDWR);
    if (fd < 0) {
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
  
    if (fd > 0) {
        close(fd);
    }
  
    // Activate our new VT
    tty = find_accessible_tty(own_vt);
    if (!tty) {
        log_debug(_("WARNING: Could not find device for VT number %d"), own_vt);
        return false;
    }
  
    fd = open(tty, O_RDWR);
    if (fd < 0) {
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

    if (fd > 0) {
        close(fd);
    }
  
    log_debug(_("VT %d ready"), own_vt);  
  
    // NOTE: We could also implement virtual console switching by using 
    // VT_GETMODE / VT_SETMODE ioctl calls and handling their signals, but
    // probably nobody will ever want to switch consoles, so I don't bother... 
  
    return true;
}

bool
FBGui::enable_terminal() 
{
    log_debug(_("Restoring terminal..."));

    char* tty = find_accessible_tty(own_vt);
    if (!tty) {
        log_debug(_("WARNING: Could not find device for VT number %d"), own_vt);
        return false;
    }

    int fd = open(tty, O_RDWR);
    if (fd < 0) {
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

    if (fd > 0) {
        close(fd);
    }
  
    return true;
}

void
FBGui::checkForData()
{
    // GNASH_REPORT_FUNCTION;

    std::vector<boost::shared_ptr<InputDevice> >::iterator it;

    for (it=_inputs.begin(); it!=_inputs.end(); ++it) {
        (*it)->check();
    }
}

// end of namespace gnash
}

#ifdef ENABLE_FAKE_FRAMEBUFFER
// Simulate the ioctls used to get information from the framebuffer
// driver. Since this is an emulator, we have to set these fields
// to a reasonable default.
int
fakefb_ioctl(int /* fd */, int request, void *data)
{
    // GNASH_REPORT_FUNCTION;
    
    switch (request) {
      case FBIOGET_VSCREENINFO:
      {
          struct fb_var_screeninfo *ptr =
              reinterpret_cast<struct fb_var_screeninfo *>(data);
          // If we are using a simulated framebuffer, the default for
          // fbe us 640x480, 8bits. So use that as a sensible
          // default. Note that the fake framebuffer is only used for
          // debugging and development.
          ptr->xres          = 640; // visible resolution
          ptr->xres_virtual  = 640; // virtual resolution
          ptr->yres          = 480; // visible resolution
          ptr->yres_virtual  = 480; // virtual resolution
          ptr->width         = 640; // width of picture in mm
          ptr->height        = 480; // height of picture in mm

          // Android and fbe use a 16bit 5/6/5 framebuffer
          ptr->bits_per_pixel = 16;
          ptr->red.length    = 5;
          ptr->red.offset    = 11;
          ptr->green.length  = 6;
          ptr->green.offset  = 5;
          ptr->blue.length   = 5;
          ptr->blue.offset   = 0;
          ptr->transp.offset = 0;
          ptr->transp.length = 0;
          // 8bit framebuffer
          // ptr->bits_per_pixel = 8;
          // ptr->red.length    = 8;
          // ptr->red.offset    = 0;
          // ptr->green.length  = 8;
          // ptr->green.offset  = 0;
          // ptr->blue.length   = 8;
          // ptr->blue.offset   = 0;
          // ptr->transp.offset = 0;
          // ptr->transp.length = 0;
          ptr->grayscale     = 1; // != 0 Graylevels instead of color
          
          break;
      }
      case FBIOGET_FSCREENINFO:
      {
          struct fb_fix_screeninfo *ptr =
              reinterpret_cast<struct fb_fix_screeninfo *>(data);
          ptr->smem_len = 307200; // Length of frame buffer mem
          ptr->type = FB_TYPE_PACKED_PIXELS; // see FB_TYPE_*
          ptr->visual = FB_VISUAL_PSEUDOCOLOR; // see FB_VISUAL_*
          ptr->xpanstep = 0;      // zero if no hardware panning
          ptr->ypanstep = 0;      // zero if no hardware panning
          ptr->ywrapstep = 0;     // zero if no hardware panning
          ptr->accel = FB_ACCEL_NONE; // Indicate to driver which specific
                                  // chip/card we have
          break;
      }
      case FBIOPUTCMAP:
      {
          // Fbe uses this name for the fake framebuffer, so in this
          // case assume we're using fbe, so write to the known fbe
          // cmap file.
          std::string str = FAKEFB;
          if (str == "/tmp/fbe_buffer") {
              int fd = open("/tmp/fbe_cmap", O_WRONLY);
              if (fd) {
                  write(fd, data, sizeof(struct fb_cmap));
                  close(fd);
              } else {
                  gnash::log_error("Couldn't write to the fake cmap!");
                  return -1;
              }
          } else {
              gnash::log_error("Couldn't write to the fake cmap, unknown type!");
              return -1;
          }
          // If we send a SIGUSR1 signal to fbe, it'll reload the
          // color map.
          int fd = open("/tmp/fbe.pid", O_RDONLY);
          char buf[10];
          if (fd) {
              if (read(fd, buf, 10) == 0) {
                  close(fd);
                  return -1;
              } else {
                  pid_t pid = strtol(buf, 0, NULL);
                  kill(pid, SIGUSR1);
                  gnash::log_debug("Signaled fbe to reload it's colormap.");
              }
              close(fd);
          }
          break;
      }
      default:
          gnash::log_unimpl("fakefb_ioctl(%d)", request);
          break;
    }

    return 0;
}
#endif  // ENABLE_FAKE_FRAMEBUFFER

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
