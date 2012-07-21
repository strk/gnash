//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011
//   Free Software Foundation, Inc
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "log.h"
#include "fb_glue_ovg.h"
#include "GnashDevice.h"
#include "GnashException.h"

#ifdef BUILD_EGL_DEVICE
# include <egl/eglDevice.h>
#endif

#ifdef BUILD_RAWFB_DEVICE
# include <rawfb/RawFBDevice.h>
#endif

#include "GnashDevice.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace gnash {

namespace gui {

FBOvgGlue::FBOvgGlue(int /* fd */)
    : _stride(0)
{
    // GNASH_REPORT_FUNCTION;    
}

FBOvgGlue::~FBOvgGlue()
{
    // GNASH_REPORT_FUNCTION;
}

bool
FBOvgGlue::init(int argc, char **argv[])
{
    // GNASH_REPORT_FUNCTION;

#if 0
    bool egl = false;
    bool rawfb = false;
    bool dfb = false;
    bool x11 = false;
    // Probe to see what display devices we have that could be used.
    boost::shared_array<renderer::GnashDevice::dtype_t> devs = probeDevices();
    if (devs) {
        int i = 0;
        while (devs[i] != renderer::GnashDevice::NODEV) {
            switch (devs[i++]) {
              case renderer::GnashDevice::EGL:
                  log_debug(_("Probing found an EGL display device"));
                  egl = true;
                  break;
              case renderer::GnashDevice::RAWFB:
                  log_debug(_("Probing found a raw Framebuffer display device"));
                  rawfb = true;
                  break;
              case renderer::GnashDevice::X11:
                  log_debug(_("Probing found an X11 display device"));
                  x11 = true;
                  break;
              case renderer::GnashDevice::DIRECTFB:
                  log_debug(_("Probing found a DirectFB display device"));
                  dfb = true;
                  break;
              case renderer::GnashDevice::NODEV:
              default:
                  log_error(_("No display devices found by probing!"));
                  break;
            }
        }

    }
    
    // Now that we know what exists, we have to decide which one to
    // use, as OpenVG can work with anything. We can only have one
    // display device operating at a time.
    if (egl) {
        setDevice(renderer::GnashDevice::EGL);
    } else {
        // OpenVG requires EGL, so if we don't have it, Gnash won't run
        log_error("OpenVG needs EGL to work!");
        return false;
    }
#endif

    _device.reset(new renderer::EGLDevice(argc, *argv));

    // Initialize the display device
    // EGL still reqires us to open the framebuffer
    _device->bindClient(renderer::GnashDevice::OPENVG);

#ifndef __ANDROID__
    _display.initDevice(0, 0);

    _width = getWidth();
    _height = getHeight();
    
    // Some linux distros like ltib have more information available
    // about the framebuffer
    int fd = ::open("/sys/class/graphics/fb0/stride", O_RDONLY);
    char number[10];
    if (::read(fd, &number, 10)) {
        _stride = strtol(number, NULL, 0);
    } else {
        if (getDepth() == 32) {
            _stride = _width * 4;
        } else {
            _stride = _width * 2;
        }
    }
    close(fd);
    
    // You must pass in the file descriptor to the opened
    // framebuffer when creating a window. Under X11, this is
    // actually the XID of the created window.
    return _device->attachWindow(_display.getHandle());
#else
    return _device->attachWindow(0);
#endif    
}

Renderer*
FBOvgGlue::createRenderHandler()
{
//    GNASH_REPORT_FUNCTION;

    // Create the renderer
    _renderer.reset(renderer::openvg::create_handler(0));

    // Print the description
    if (!_renderer->description().empty()) {
        log_debug("Renderer is: %s", _renderer->description());
    }
    
    if (!_renderer) {
        boost::format fmt = boost::format(
            _("Could not create OpenVG renderer"));
        throw GnashException(fmt.str());
    }

    return _renderer.get();
}

/// Not implemented, Fixme
void
FBOvgGlue::setInvalidatedRegions(const InvalidatedRanges& ranges)
{
    if (!_renderer) {
        log_error(_("No renderer set!"));
        return;
    }

    _renderer->set_invalidated_regions(ranges);
    
    _drawbounds.clear();

    for (size_t rno = 0; rno<ranges.size(); rno++) {
        geometry::Range2d<int> bounds = Intersection(
            _renderer->world_to_pixel(ranges.getRange(rno)),
            _validbounds);
        // it may happen that a particular range is out of the screen, which 
        // will lead to bounds==null. 
        if (bounds.isNull()) continue; 
        
        _drawbounds.push_back(bounds);   
    }
    
    // GNASH_REPORT_FUNCTION;
    // if (_renderer) {
    //     _renderer->setInvalidatedRegions(ranges);
    // }
}

void
FBOvgGlue::prepDrawingArea(void * /*drawing_area */)
{
    // GNASH_REPORT_FUNCTION;

    // _device->attachWindow(reinterpret_cast
    //         <renderer::GnashDevice::native_window_t>(drawing_area));
}

void
FBOvgGlue::render()
{
    // GNASH_REPORT_FUNCTION;

    _device->swapBuffers();
}

} // end of namespace gui
} // end of namespace gnash
    
// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
