//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012 Free Software
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

// -----------------------------------------------------------------------------
   
/// \page fb_input FB GUI input devices
///
/// The framebuffer GUI supports various input devices through the modern
/// Linux Input Subsystem (/dev/input/event*). Both standard mice and 
/// touchscreen devices are supported.
///
/// Since there can be multiple input devices in /dev/input/ you have to
/// specify which device to use using the 
//  POINTING_DEVICE environment variable for the mouse and
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
#include "rc.h"
#include "fbsup.h"
#include "log.h"
#include "movie_root.h"
#include "RunResources.h"
#include "GnashSleep.h" // for gnashSleep
#include "Renderer.h"

#include <linux/input.h>    // for /dev/input/event*
#include <events/InputDevice.h>

#ifdef RENDERER_AGG
# include "fb_glue_agg.h"
# include "agg/Renderer_agg.h"
#endif

#ifdef RENDERER_OPENVG
# include "VG/openvg.h"
# include "openvg/OpenVGRenderer.h"
# include "fb_glue_ovg.h"
#endif

#ifdef RENDERER_GLES1
# include "fb_glue_gles1.h"
# include "opengles1/Renderer_gles1.h"
#endif

// We need to declare the std::, as the boost header files want to use ptrdiff_t.
using namespace std;

namespace gnash {

namespace gui {

int terminate_request = false;  // global scope to avoid GUI access

std::auto_ptr<Gui> createFBGui(unsigned long windowid, float scale,
                               bool do_loop, RunResources& r)
{
    // GNASH_REPORT_FUNCTION;
    return std::auto_ptr<Gui>(new FBGui(windowid, scale, do_loop, r));
}

/// Called on CTRL-C and alike
void
terminate_signal(int /*signo*/) {
    terminate_request = true;
}

FBGui::FBGui(unsigned long xid, float scale, bool loop, RunResources& r)
    : Gui(xid, scale, loop, r),
      _fd(-1),
      _original_vt(-1),
      _original_kd(-1),
      _own_vt(-1),
      _xpos(0),
      _ypos(0),  
      _timeout(0),
      _fullscreen(true)
{
    // GNASH_REPORT_FUNCTION;
    
    // initializing to zero helps with debugging and prevents weird bugs
//    memset(mouse_buf, 0, 256);
    signal(SIGINT, terminate_signal);
    signal(SIGTERM, terminate_signal);
}

FBGui::~FBGui()
{  
    // GNASH_REPORT_FUNCTION;
    
    if (_fd > 0) {
        disable_terminal();
        // log_debug("Closing framebuffer device");
        close(_fd);
    }
}

bool
FBGui::init(int argc, char *** argv)
{
//  GNASH_REPORT_FUNCTION;

    // the current renderer as set on the command line or gnashrc file
    std::string renderer = _runResources.getRenderBackend();

#ifdef RENDERER_OPENVG
    if (renderer.empty()) {
        renderer = "openvg";
    }
    if ((renderer == "openvg") || (renderer == "ovg")) {
        renderer = "openvg";
        _glue.reset(new FBOvgGlue(0));
        // Initialize the glue layer between the renderer and the gui toolkit
        if (!_glue->init(argc, argv)) {
            return false;
        }
        
        FBOvgGlue *ovg = reinterpret_cast<FBOvgGlue *>(_glue.get());
        // Set "window" size
        _width =  ovg->getWidth();
        _height = ovg->getHeight();
        log_debug("Width:%d, Height:%d", _width, _height);
        _renderer.reset(renderer::openvg::create_handler(0));     
        renderer::openvg::Renderer_ovg *rend = reinterpret_cast
            <renderer::openvg::Renderer_ovg *>(_renderer.get());
        rend->init(_width, _height);
    }
#endif

#ifdef RENDERER_GLES1
    if ((renderer == "opengles1") || (renderer == "gles1")) {
        renderer = "opengles1";
        _glue.reset(new FBgles1Glue(0));
        // Initialize the glue layer between the renderer and the gui toolkit
        if (!_glue->init(argc, argv)) {
            return false;
        }
        
        FBgles1Glue *gles1 = reinterpret_cast<FBgles1Glue *>(_glue.get());
        // Set "window" size
        _width =  gles1->getWidth();
        _height = gles1->getHeight();
        log_debug("Width:%d, Height:%d", _width, _height);
        //_renderer.reset(create_render_handler_gles1(true, _glue));
        _renderer.reset(renderer::gles1::create_handler(""));
        // renderer::openvg::Renderer_gles1 *rend = reinterpret_cast
        //     <renderer::openvg::Renderer_ovg *>(_renderer.get());
        // rend->init(_width, _height);
    }
#endif
    
    // map framebuffer into memory
    // Create a new Glue layer
#ifdef RENDERER_AGG
    if (renderer.empty()) {
        renderer = "agg";
    }
    if (renderer == "agg") {
        _glue.reset(new FBAggGlue());
        // Initialize the glue layer between the renderer and the gui toolkit
        _glue->init(argc, argv);
        FBAggGlue *agg = reinterpret_cast<FBAggGlue *>(_glue.get());
        // Set "window" size
        _width =  agg->width();
        _height = agg->height();
        log_debug("Width:%d, Height:%d", _width, _height);
        _renderer.reset(agg->createRenderHandler());
    }
#endif
    if ((renderer != "openvg") && (renderer != "agg")) {
        log_error(_("No renderer! %s not supported."), renderer);
    }
    
    disable_terminal();
    
#ifdef HAVE_LINUX_UINPUT_H
    // Look for the User Mode Input (Uinput) device, which is used to
    // control the movement and coordinates of the mouse cursor.
    if (_uinput.scanForDevice()) {
        _uinput.init();
        _uinput.moveTo(0, 0);
    } else {
        log_error(_("Found no accessible User mode input event device"));
    }
#endif
    
    // Initialize all the input devices

    // Look for Mice that use the PS/2 mouse protocol
    std::vector<boost::shared_ptr<InputDevice> > possibles
        = InputDevice::scanForDevices();
    if (possibles.empty()) {
        log_error(_("Found no accessible input event devices"));
    } else {
        log_debug("Found %d input event devices.", possibles.size());
    }
    
    std::vector<boost::shared_ptr<InputDevice> >::iterator it;
    for (it=possibles.begin(); it!=possibles.end(); ++it) {
        // Set the screen size, which is used for calculating absolute
        // mouse locations from relative ones.
        (*it)->setScreenSize(_width, _height);
        // (*it)->dump();
#if defined(USE_MOUSE_PS2) || defined(USE_MOUSE_ETT)
        if ((*it)->getType() == InputDevice::MOUSE) {
            log_debug(_("WARNING: Mouse support may conflict with the input event support."));
            // For now we only want keyboard input events, as the mouse
            // interface default of /dev/input/mice supports hotpluging devices,
            // unlike the regular events.
            _inputs.push_back(*it);
        }
#endif
        if ((*it)->getType() == InputDevice::KEYBOARD) {
            _inputs.push_back(*it);
        }
        // TSLib also supports linux input events, so we
        // use that instead of handling the events directly. The
        // Babbage is configured as a tablet when using input events.
        if ((*it)->getType() == InputDevice::TOUCHSCREEN) {
            log_debug(_("Enabling Touchscreen support."));
            _inputs.push_back(*it);
        }
        if ((*it)->getType() == InputDevice::TABLET) {
#if 1
            log_debug(_("WARNING: Babbage Tablet support disabled as it conflicts with TSlib"));
#else
            log_debug(_("Enabling Babbage Touchscreen support"));
            _inputs.push_back(*it);
#endif
        }
        if ((*it)->getType() == InputDevice::POWERBUTTON) {
            log_debug(_("Enabling Power Button support"));
            _inputs.push_back(*it);
        }
    }

    // Let -j -k override "window" size
    optind = 0; opterr = 0; int c;
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

#if 0
    // FIXME: this allows to draw in a subsection of the screen. OpenVG
    // should be able to support this, but right now it just gets in
    // the way of debugging.
    
    if ( _xpos < 0 ) _xpos += _varinfo.xres - _width;
    _xpos = clamp<int>(_xpos, 0, _varinfo.xres-_width);

    if ( _ypos < 0 ) _ypos += _varinfo.yres - _height;
    _ypos = clamp<int>(_ypos, 0, _varinfo.yres-_height);

    log_debug("X:%d, Y:%d", _xpos, _ypos);
#endif
    
    _validbounds.setTo(0, 0, _width - 1, _height - 1);
    
    return true;
}

bool
FBGui::resize_view(int width, int height)
{
    GNASH_REPORT_FUNCTION;

//   _glue.prepDrawingArea(width, height, 0);
    Gui::resize_view(width, height);
 
    return true;
}

bool
FBGui::run()
{
//  GNASH_REPORT_FUNCTION;

#ifdef USE_TSLIB
    int ts_loop_count = 0;
#endif

    VirtualClock& timer = getClock();
    int delay = 0;
    
    // let the GUI recompute the x/y scale factors to best fit the whole screen
    resize_view(_validbounds.width(), _validbounds.height());

    float fps = getFPS();
    
    // FIXME: this value is arbitrary, and will make any movie with
    // less than 12 frames eat up more of the cpu. It should probably
    // be a much lower value, like 2.
    if (fps > 12) {
        delay = static_cast<int>(100000/fps);
    } else {
        // 10ms per heart beat
        delay = 1000;
    }
    // log_debug(_("Movie Frame Rate is %d, adjusting delay to %dms"), fps,
    //           _interval * delay);
    
    // This loops endlessly at the frame rate
    while (!terminate_request) {  
        // wait the "heartbeat" inteval. _interval is in milliseconds,
        // but gnashSleep() wants nanoseconds, so adjust by 1000.
        gnashSleep(_interval * delay);
        // TODO: Do we need to check the real time slept or is it OK when we woke
        // up early because of some Linux signal sent to our process (and thus
        // "advance" faster than the "heartbeat" interval)? - Udo

#ifdef USE_TSLIB
        ts_loop_count++; //increase loopcount
#endif
        
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
//    GNASH_REPORT_FUNCTION;    

    _glue->render();
}

bool
FBGui::createWindow(const char* /*title*/, int /*width*/, int /*height*/,
                     int /*xPosition*/, int /*yPosition*/)
{
//  GNASH_REPORT_FUNCTION;

    _runResources.setRenderer(_renderer);

    return true;
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

// The Framebuffer GUI usually always runs fullscreen, which is
// the default, but occasionally users want to run the SWF movie
// it's actual size, or in different locations.
void
FBGui::setFullscreen()
{
    _fullscreen = true;
}

void
FBGui::unsetFullscreen()
{
    _fullscreen = false;
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
FBGui::setInvalidatedRegion(const SWFRect& bounds)
{
//  GNASH_REPORT_FUNCTION;

   setInvalidatedRegion(bounds);
}

void
FBGui::setInvalidatedRegions(const InvalidatedRanges& ranges)
 {
//     GNASH_REPORT_FUNCTION;

     _glue->setInvalidatedRegions(ranges);
}

char *
FBGui::find_accessible_tty(int no)
{
    // GNASH_REPORT_FUNCTION;

    char* fn;
    
    fn = find_accessible_tty("/dev/vc/%d", no);   if (fn) return fn;
    fn = find_accessible_tty("/dev/tty%d", no);   if (fn) return fn;
    fn = find_accessible_tty("/dev/tty%02x", no); if (fn) return fn;
    fn = find_accessible_tty("/dev/tty%x", no);   if (fn) return fn;
    fn = find_accessible_tty("/dev/tty%02d", no); if (fn) return fn;
  
    if (no == 0) {
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
    // GNASH_REPORT_FUNCTION;

    _original_kd = -1;
    
    // Find the TTY device name
    
    char* tty = find_accessible_tty(0);

    log_debug("Disabling terminal %s", tty);
    
    int fd;
  
    if (!tty) {
        log_error(_("Could not detect controlling TTY"));
        return false;
    }
    
    // Detect the currently active virtual terminal (so we can switch back to
    // it later)
    
    fd = open(tty, O_RDWR);
    if (fd < 0) {
        log_error(_("Could not open %s"), tty);
        return false;
    }
    
    struct vt_stat vts;
    if (ioctl(fd, VT_GETSTATE, &vts) == -1) {
        log_error(_("Could not get current VT state"));
        close(_fd);
        _fd = -1;
        close(fd);
        return false;
    }
    
    _original_vt = vts.v_active;
    // log_debug("Original TTY NO = %d", _original_vt);
  
#ifdef REQUEST_NEW_VT
    // Request a new VT number
    if (ioctl(fd, VT_OPENQRY, &_own_vt) == -1) {
        log_error(_("Could not request a new VT"));
        close(fd);
        return false;
    }
  
    // log_debug("Own TTY NO = %d", _own_vt);

    if (fd > 0) {
        close(fd);
    }
  
    // Activate our new VT
    tty = find_accessible_tty(_own_vt);
    if (!tty) {
        log_error(_("Could not find device for VT number %d"), _own_vt);
        return false;
    }
  
    _fd = open(tty, O_RDWR);
    if (fd < 0) {
        log_error(_("Could not open %s"), tty);
        return false;
    }
  
    if (ioctl(fd, VT_ACTIVATE, _own_vt) == -1) {
        log_error(_("Could not activate VT number %d"), _own_vt);
        close(fd);
        return false;
    }
  
    if (ioctl(fd, VT_WAITACTIVE, _own_vt) == -1) {
        log_error(_("Error waiting for VT %d becoming active"),
                  _own_vt);
        //close(tty);
        //return false;   don't abort
    }

#else

    _own_vt = _original_vt;   // keep on using the original VT
  
    if (fd > 0) {
        close(fd);
    }
  
    // Activate our new VT
    tty = find_accessible_tty(_own_vt);
    if (!tty) {
        log_error(_("Could not find device for VT number %d"), _own_vt);
        return false;
    }
  
    fd = open(tty, O_RDWR);
    if (fd < 0) {
        log_error(_("Could not open %s"), tty);
        return false;
    }
  
#if 0
    // Become session leader and attach to terminal
    setsid();
    if (ioctl(fd, TIOCSCTTY, 0) == -1) {
    log_error(_("Could not attach controlling terminal (%s)"), tty);
    }
#endif
#endif  // end of if REQUEST_NEW_VT
  
    // Disable keyboard cursor
  
    if (ioctl(fd, KDGETMODE, &_original_kd) == -1) {
        log_error(_("Could not query current keyboard mode on VT"));
    }

    if (ioctl(fd, KDSETMODE, KD_GRAPHICS) == -1) {
        log_error(_("Could not switch to graphics mode on new VT"));
    }

    if (fd > 0) {
        close(fd);
    }
  
    // log_debug("VT %d ready", _own_vt);  
  
    // NOTE: We could also implement virtual console switching by using 
    // VT_GETMODE / VT_SETMODE ioctl calls and handling their signals, but
    // probably nobody will ever want to switch consoles, so I don't bother... 
  
    return true;
}

bool
FBGui::enable_terminal() 
{
    // GNASH_REPORT_FUNCTION;

    // log_debug("Restoring terminal...");

    char* tty = find_accessible_tty(_own_vt);
    log_debug("Enabling terminal %s", tty);

    if (!tty) {
        log_error(_("Could not find device for VT number %d"), _own_vt);
        return false;
    }

    int fd = open(tty, O_RDWR);
    if (fd < 0) {
        log_error(_("Could not open %s"), tty);
        return false;
    }

    if (ioctl(fd, VT_ACTIVATE, _original_vt)) {
        log_error(_("Could not activate VT number %d"), _original_vt);
        close(_fd);
        _fd = -1;
        close(fd);
        return false;
    }

    if (ioctl(fd, VT_WAITACTIVE, _original_vt)) {
        log_error(_("Error waiting for VT %d becoming active"),
                  _original_vt);
        //close(tty);
        //return false;   don't abort
    }  
  
    // Restore keyboard
  
    if (ioctl(fd, KDSETMODE, _original_kd)) {
        log_error(_("Could not restore keyboard mode"));
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
        boost::shared_ptr<InputDevice::input_data_t> ie = (*it)->popData();
        if (ie) {            
            // notifyMouseMove(ie->x, ie->y);
#if 0
            std::cerr << "Got data: " << ((ie->pressed) ? "true" : "false");
            std::cerr << ", " << ie->key << ", " << ie->modifier;
            std::cerr << ", " << ie->x << ", " << ie->y << std::endl;
            // cerr << "X = " << coords[0] << endl;
            // cerr << "Y = " << coords[1] << endl;
#endif
            // Range check and convert the position from relative to
            // absolute
            boost::shared_array<int> coords =
                InputDevice::convertAbsCoords(ie->x, ie->y,
                                              getStage()->getStageWidth(),
                                              getStage()->getStageHeight());
#ifdef HAVE_LINUX_UINPUT_H
            // The mouse was moved
            _uinput.moveTo(coords[0], coords[1]);
            if (coords) {
                notifyMouseMove(coords[0], coords[1]);
            }
#endif
            
            // See if a mouse button was clicked
            if (ie->pressed) {
                notifyMouseClick(true);
#if 1
                double x = 0.655 * ie->x;
                double y = 0.46875 * ie->y;
                // log_debug("Mouse clicked at: %g:%g", x, y);
                notifyMouseMove(int(x), int(y));
#else
                notifyMouseMove(ie->x, ie->y);
#endif  
            }
        }
    }
}

} // end of namespace gui
} // end of namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
