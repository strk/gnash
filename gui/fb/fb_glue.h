//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
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

#ifndef GNASH_FB_GLUE_H
#define GNASH_FB_GLUE_H

#include <cassert>

namespace gnash {
    class Renderer;
    class movie_root;
}

namespace gnash {

namespace gui {
    
typedef void FbWidget;
  
class FBGlue
{
public:
    FBGlue();
    virtual ~FBGlue();
    
    virtual bool init(int argc, char **argv[]) = 0;
    
    virtual void prepDrawingArea(FbWidget *drawing_area) = 0;
    virtual Renderer* createRenderHandler() = 0;
    virtual void setRenderHandlerSize(int /*width*/, int /*height*/) {}
    virtual void render() = 0;
    
    virtual int width() = 0;
    virtual int height() = 0;
    
    virtual void render(void* const region);

    virtual void beforeRendering(movie_root *) {};

    boost::shared_array<renderer::GnashDevice::dtype_t> probeDevices() {
        GNASH_REPORT_FUNCTION;
        
        size_t total = 0;
#ifdef BUILD_EGL_DEVICE
        total++;
#endif
#ifdef BUILD_RAWFB_DEVICE
        total++;
#endif
#ifdef BUILD_DIRECTFB_DEVICE
        total++;
#endif
#ifdef BUILD_X11_DEVICE
        total++;
#endif
        boost::shared_array<renderer::GnashDevice::dtype_t> devs
            (new renderer::GnashDevice::dtype_t[total]);
#ifdef BUILD_X11_DEVICE
        devs[--total] = renderer::GnashDevice::X11;
#endif
#ifdef BUILD_EGL_DEVICE
        devs[--total] = renderer::GnashDevice::EGL;
#endif
#ifdef BUILD_RAWFB_DEVICE
        devs[--total] = renderer::GnashDevice::RAWFB;
#endif
#ifdef BUILD_DIRECTFB_DEVICE
        devs[--total] = renderer::GnashDevice::DIRECTFB;
#endif
        
        return devs;
    }

    void resetDevice() { _device.reset(); };
    
    renderer::GnashDevice::dtype_t getDevice()
    {
        if (_device) {
            return _device->getType();
        }
        return renderer::GnashDevice::NODEV;
    }
    
    void setDevice(renderer::GnashDevice::dtype_t dtype) {
        switch (dtype) {
#ifdef BUILD_EGL_DEVICE
          case renderer::GnashDevice::EGL:
          {
              _device.reset(new renderer::EGLDevice);
              break;
          }
#endif
#ifdef BUILD_RAWFB_DEVICE_XX
          case renderer::GnashDevice::RAWFB:
          {
              _device.reset(new renderer::rawfb::RawFBDevice);
              break;
          }
#endif
#ifdef BUILD_DIRECTFB_DEVICE
          case renderer::GnashDevice::DIRECTFB:
          {
              _device.reset(new renderer::directfb::DirectFBDevice);
              break;
          }
#endif
#ifdef BUILD_X11_DEVICE
          case renderer::GnashDevice::X11:
          {
              _device.reset(new renderer::x11::X11Device);
              break;
          }
#endif
          default:
              log_error("unsupported Display Device!");
        }
        // // EGL doesn't care about command line argument, so pass NULL
        // _device->initDevice(0, 0);
        // renderer::EGLDevice *egl = (renderer::EGLDevice*)_device.get();
        // egl->printEGLConfig();
        // egl->printEGLSurface();
    }

    size_t getWidth()  { return (_device) ? _device->getWidth() : 0; };
    size_t getHeight() { return (_device) ? _device->getWidth() : 0; };
    size_t getDepth()  { return (_device) ? _device->getDepth() : 0; };
    
protected:
    boost::scoped_ptr<renderer::GnashDevice> _device;    
};

} // end of namespace gui
} // end of namespace gnash

// end of GNASH_FB_GLUE_H
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
