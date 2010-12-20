// 
//   Copyright (C) 2010 Free Software Foundation, Inc
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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <fcntl.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <sstream>
#include <map>
#include <signal.h>
#include <unistd.h>
#include <cassert>
#include <regex.h>
#include <boost/assign/list_of.hpp>
#include <boost/date_time/date.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "log.h"
#include "dejagnu.h"
#include "SWFMatrix.h"
#include "Renderer.h"
#include "Transform.h"
#include "CachedBitmap.h"
#include "GnashVaapiImage.h"
#include "GnashVaapiImageProxy.h"
#include "boost/date_time/posix_time/posix_time.hpp"

#ifdef RENDERER_AGG
# ifdef BUILD_RAWFB_DEVICE
#  include "fb/fb_glue_agg.h"
# endif
# ifdef BUILD_X11_DEVICE
#  include "gtk/gtk_glue_agg.h"
# endif
#endif
#ifdef RENDERER_OPENVG
# include "fb/fb_glue_ovg.h"
#endif
#if 0
#ifdef RENDERER_GLES1
# include "fb/fb_glue_gles1.h"
#endif
#ifdef RENDERER_GLES2
# include "fb/fb_glue_gles2.h"
#endif
#ifdef RENDERER_OPENGL
# ifdef BUILD_X11_DEVICE
#  include "gtk/gtk_glue_gtkglext.h"
# endif
#endif
#ifdef RENDERER_CAIRO
# include "gtk/gtk_glue_cairo.h"
#endif
#endif

TestState runtest;

using namespace gnash; 
using namespace renderer; 
using namespace gui; 
using namespace std;
using namespace boost::posix_time;

static void test_render(const std::string &gui, const std::string &render);

// The debug log used by all the gnash libraries.
static LogFile& dbglogfile = LogFile::getDefaultInstance();

//------------------------------------------------------------

// Simple class to do nanosecond based timing for performance analysis
class Timer {
public:
    Timer(const std::string &name, bool flag)
        {
            _print = flag;
            _name = name;
            start();
        }
    Timer(const std::string &name) : _print(false) { _name = name; start(); }
    Timer() : _print(false) { start(); }
    ~Timer()
        {
            stop();
            if (_print) {
                cerr << "Total time for " << _name << " was: " << elapsed() << endl;
            }
        };
    
    void start() {
        _starttime = boost::posix_time::microsec_clock::local_time(); 
    }
    
    void stop() {
        _stoptime = boost::posix_time::microsec_clock::local_time(); 
    }

    std::string elapsed() {
        stringstream ss;
        time_duration td = _stoptime - _starttime;
        ss << td.total_nanoseconds() << "ns elapsed";
        return ss.str();
    }
    void setName(const std::string &name) { _name = name; };
    std::string &getName() { return _name; };
    
private:
    bool _print;
    std::string _name;
    boost::posix_time::ptime _starttime;
    boost::posix_time::ptime _stoptime;
};

int
main(int argc, char *argv[])
{
    // FIXME: for now, always run verbose till this supports command line args
    dbglogfile.setVerbosity();

    // Create fake data for the fake framebuffer, This was copied off a
    // working PC.
    struct fb_var_screeninfo varinfo;
    memset(&varinfo, 0, sizeof(struct fb_var_screeninfo));
    varinfo.xres = 1200;
    varinfo.yres = 1024;
    varinfo.xres_virtual = 1280;
    varinfo.yres_virtual = 1024;
    varinfo.bits_per_pixel = 32;
    varinfo.red.offset = 16;
    varinfo.red.length = 8;
    varinfo.green.offset = 8;
    varinfo.green.length = 8;
    varinfo.blue.offset = 0;
    varinfo.blue.length = 8;
    varinfo.width = 4294967295;
    varinfo.height = 4294967295;
    
    size_t memsize = 5242880;
    struct fb_fix_screeninfo fixinfo = {
        "test_glue",
        0,                      // replaced later by drawing_area
        5242880,                // the memory size
        0,
        0,
        2,
        1,
        1, 0,
        5120,                   // the rowsize
        4289200128,
        524288,                 // the framebuffer memory size
        0,
        {0, 0, 0}};

    test_render("FB", "OpenVG");
}

static void
test_render(const std::string &gui, const std::string &render)
{
    cout << "*** Testing the glue layer for " << gui
         << " and " << render << "*** " << endl;

    FBOvgGlue fbvg(0);

    // if ((gui == "FB") && (render = "OpenVG")) {
    //     // We're not testing the virtual terminals here, so we just pass 0 as
    //     // the file descriptor.
    //     glue = FBAggGlue;
    // }

    // Test the defaults. These need to properly handle an unitialized
    // Glue without segfaulting.
    if (fbvg.width() == 0) {
        runtest.pass("FBOvgGlue::width(0)");
    } else {
        runtest.fail("FBOvgGlue::width(0)");
    }
    
    if (fbvg.height() == 0) {
        runtest.pass("FBOvgGlue::height(0)");
    } else {
        runtest.fail("FBOvgGlue::height(0)");
    }

    Renderer *renderer = fbvg.createRenderHandler();    
    
#if 0
    // FIXME: this appears to be AGG specific
    // These next two will display an error, because the glue isn't
    // set but we know that, we want to test if nothiung crashes when
    // unitilized.
    SWFRect bounds;
    fbvg.setInvalidatedRegion(bounds);
    if (fbvg.getBounds() == 0) {
        runtest.pass("FBOvgGlue::setInvalidatedRegion(0)");
    } else {
        runtest.fail("FBOvgGlue::setInvalidatedRegion(0)");
    }    
#endif
    
    InvalidatedRanges ranges;
    fbvg.setInvalidatedRegions(ranges);
    if (fbvg.getBounds() == 0) {
        runtest.pass("FBOvgGlue::setInvalidatedRegions(0)");
    } else {
        runtest.fail("FBOvgGlue::setInvalidatedRegions(0)");
    }    

#if 0
    fbvg.prepDrawingArea(reinterpret_cast<void *>(fixinfo.smem_start));
    if (fbvg.getBounds() == 0) {
        runtest.pass("FBOvgGlue::setInvalidatedRegions(0)");
    } else {
        runtest.fail("FBOvgGlue::setInvalidatedRegions(0)");
    }
#endif

    // This initlizes the device and glue
    if (fbvg.init(0, 0)) {
        runtest.pass("FBOvgGlue::init()");
    } else {
        runtest.fail("FBOvgGlue::init()");
    }

    if (fbvg.width() > 0) {
        runtest.pass("FBOvgGlue::width()");
    } else {
        runtest.fail("FBOvgGlue::width()");
    }
    
    if (fbvg.height() > 0) {
        runtest.pass("FBOvgGlue::height()");
    } else {
        runtest.fail("FBOvgGlue::height()");
    }

    fbvg.render();
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
