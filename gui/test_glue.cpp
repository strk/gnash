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
# include "fb/fb_glue_agg.h"
//# include "gtk/gtk_glue_agg.h"
#endif
#ifdef RENDERER_OPENGL
#endif
#ifdef RENDERER_OPENVG
#endif
#ifdef RENDERER_GLES1
#endif
#ifdef RENDERER_GLES2
#endif
#ifdef RENDERER_CAIRO
#endif

TestState runtest;

using namespace gnash; 
using namespace renderer; 
using namespace gui; 
using namespace std;
using namespace boost::posix_time;

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

    // We're not testing the virtual terminals here, so we just pass 0 as
    // the file descriptor.
    FBAggGlue fbag(0);

    // Test the defaults. These need to properly handle an unitialized
    // Renderer without segfaulting.
    if (fbag.width() == 0) {
        runtest.pass("FBAggGlue::width(0)");
    } else {
        runtest.fail("FBAggGlue::width(0)");
    }
    
    if (fbag.height() == 0) {
        runtest.pass("FBAggGlue::height(0)");
    } else {
        runtest.fail("FBAggGlue::height(0)");
    }    

    // These next two will display an error, because the renderer isn't
    // set but we know that, we want to test if nothiung crashes when
    // unitilized.
    SWFRect bounds;
    fbag.setInvalidatedRegion(bounds);
    if (fbag.getBounds() == 0) {
        runtest.pass("FBAggGlue::setInvalidatedRegion(0)");
    } else {
        runtest.fail("FBAggGlue::setInvalidatedRegion(0)");
    }    

    InvalidatedRanges ranges;
    fbag.setInvalidatedRegions(ranges);
    if (fbag.getBounds() == 0) {
        runtest.pass("FBAggGlue::setInvalidatedRegions(0)");
    } else {
        runtest.fail("FBAggGlue::setInvalidatedRegions(0)");
    }    

#if 0
    fbag.prepDrawingArea(reinterpret_cast<void *>(fixinfo.smem_start));
    if (fbag.getBounds() == 0) {
        runtest.pass("FBAggGlue::setInvalidatedRegions(0)");
    } else {
        runtest.fail("FBAggGlue::setInvalidatedRegions(0)");
    }
#endif

    // This initlizes the device and renderer
    if (fbag.init(argc, &argv)) {
        runtest.pass("FBAggGlue::init()");
    } else {
        runtest.fail("FBAggGlue::init()");
    }

    if (fbag.width() > 0) {
        runtest.pass("FBAggGlue::width()");
    } else {
        runtest.fail("FBAggGlue::width()");
    }
    
    if (fbag.height() > 0) {
        runtest.pass("FBAggGlue::height()");
    } else {
        runtest.fail("FBAggGlue::height()");
    }

    fbag.render();
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
