//
//   Copyright (C) 2011 Free Software Foundation, Inc
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

#include "DeviceGlue.h"

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

namespace gnash {

void
DeviceGlue::setDevice(renderer::GnashDevice::dtype_t dtype) 
{
    switch (dtype) {
#ifdef BUILD_EGL_DEVICE
        case renderer::GnashDevice::EGL:
        {
            _device.reset(new renderer::EGLDevice(0, 0));
            break;
        }
#endif
#ifdef BUILD_RAWFB_DEVICE
        case renderer::GnashDevice::RAWFB:
        {
            _device.reset(new renderer::rawfb::RawFBDevice(0, 0));
            break;
        }
#endif
#ifdef BUILD_DIRECTFB_DEVICE
        case renderer::GnashDevice::DIRECTFB:
        {
            _device.reset(new renderer::directfb::DirectFBDevice(0, 0));
            break;
        }
#endif
#ifdef BUILD_X11_DEVICE
        case renderer::GnashDevice::X11:
        {
            _device.reset(new renderer::x11::X11Device(0, 0));
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

} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
