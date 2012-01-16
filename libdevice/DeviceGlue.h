//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
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
//

#ifndef __DEVICE_GLUE_H__
#define __DEVICE_GLUE_H__ 1

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/shared_array.hpp>
#include <boost/scoped_ptr.hpp>

#include "GnashDevice.h"


/// @note This file is a simple base class for any GUI glue layer code
/// That needs to use libdevice devices. Currently this is used by both
/// the GTK and Framebuffer GUIs for OpenVG, OpenGLES1, and OpenGLES2.
namespace gnash {

class DeviceGlue {
public:
    DeviceGlue() {};
    ~DeviceGlue() {};
    
    /// Probe the system to see what types of display devices exist. This
    /// doesn't select a device, it merely returns a list of what is
    /// available.
    ///
    /// @return a list of devices
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

    /// Reset the the current device, which disables output
    void resetDevice() { _device.reset(); };
    
    /// Get the current active device type.
    renderer::GnashDevice::dtype_t getDevice()
    {
        if (_device) {
            return _device->getType();
        }
        return renderer::GnashDevice::NODEV;
    }
    
    /// Set the display device for later use. After this is called,
    /// the display device is active
    void setDevice(renderer::GnashDevice::dtype_t dtype);

    /// Initialze the device
    ///
    /// @param argc The count of arguments from the command line
    /// @param argv The array of command line arguments
    /// @return status
    bool initDevice(int argc, char *argv[]) {
        return (_device) ? _device->initDevice(argc, argv) : false;
    };

    /// Attach the area to draw in to the lower level device. This makes
    /// the drawing area available to the dispaly device when binding the
    /// display to the native windowing system or framebuffer.
    bool attachWindow(renderer::GnashDevice::native_window_t window) {
        return (_device) ? _device->attachWindow(window) : false;
    };

    /// Bind the client API to the device. As EGL can support different
    /// renderers, namely OpenGL, OpenGLES1, OpenGLES2, and OpenVG. This
    /// is how the underlying hardware knows which API to implement.
    bool bindClient(renderer::GnashDevice::rtype_t rtype) {
        return (_device) ? _device->bindClient(rtype) : false;
    };

    ///  Get the Width of the drawing area, in pixels. For framebuffer
    ///  based devices, this is the size of the display screen.
    size_t getWidth()  { return (_device) ? _device->getWidth() : 0; };

    /// Height of the drawing area, in pixels. For framebuffer
    ///  based devices, this is the size of the display screen.
    size_t getHeight() { return (_device) ? _device->getHeight() : 0; };

    /// Depth of the display
    size_t getDepth() { return (_device) ? _device->getDepth() : 0; };

    /// Make the current buffer the active one.
    bool swapBuffers() {
        return (_device) ? _device->swapBuffers() : false;
    }

protected:
    boost::scoped_ptr<renderer::GnashDevice> _device;
};
    
} // namespace gnash

#endif  // end of __DEVICE_GLUE_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
