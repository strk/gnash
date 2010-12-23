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
//

#ifndef __DEVICE_GLUE_H__
#define __DEVICE_GLUE_H__ 1

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/scoped_array.hpp>
#include <boost/shared_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "GnashDevice.h"

#ifdef BUILD_EGL_DEVICE
#include "egl/eglDevice.h"
#endif
#ifdef BUILD_RAWFB_DEVICE
#include "rawfb/RawFBDevice.h"
#endif
#ifdef BUILD_DIRECTFB_DEVICE
#include "directfb/DirectFBDevice.h"
#endif
#ifdef BUILD_X11_DEVICE
#include "x11/X11Device.h"
#endif

/// @note This file is a simple base class for any GUI glue layer code
/// That needs to use libdevice devices. Currently this is used by both
/// the GTK and Framebufffer GUIs for OpenVG, OpenGLES1, and OpenGLES2.
namespace gnash {

class DeviceGlue {
public:
    DeviceGlue() {};
    ~DeviceGlue() {};
    
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
        total++;                // add one more for the list terminator
        boost::shared_array<renderer::GnashDevice::dtype_t> devs
            (new renderer::GnashDevice::dtype_t[total]);
        // terminate the list so it can easily be walked through later.
        devs[--total] = renderer::GnashDevice::NODEV;
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
#ifdef BUILD_RAWFB_DEVICE
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

    bool initDevice(int argc, char *argv[]) { return
            _device->initDevice(argc, argv); };

    bool attachWindow(renderer::GnashDevice::native_window_t window) { return
            _device->attachWindow(window); };
    
protected:
    boost::scoped_ptr<renderer::GnashDevice> _device;
};
    
} // namespace gnash

#endif  // end of __DEVICE_GLUE_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
