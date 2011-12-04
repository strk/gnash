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

#ifndef __GNASH_DEVICE_H__
#define __GNASH_DEVICE_H__ 1

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/scoped_array.hpp>

#include "Geometry.h"

/// @note This file is the base class for all low level rendering and display
/// devices. These devices must be probed and initialized first, before any
/// renderering or window creation happens.
/// The derived classes for this base class are then used by the renderer to
/// determine functionality.
namespace gnash {

namespace renderer {

struct GnashDevice
{
    typedef std::vector<const Path*> PathRefs;
    typedef std::vector<Path> PathVec;
    typedef std::vector<geometry::Range2d<int> > ClipBounds;
    typedef std::vector<const Path*> PathPtrVec;
    
    /// Handle multiple window types. The derived classes will cast this to
    /// the proper data type.
    typedef long native_window_t;
    
    /// The list of supported renders that use devices
    typedef enum {OPENVG, OPENGL, OPENGLES1, OPENGLES2, XORG, VAAPI} rtype_t;
    /// The list of supported device types
    typedef enum {NODEV, EGL, DIRECTFB, X11, RAWFB} dtype_t;
    
    GnashDevice(int argc, char *argv[]);
    GnashDevice() { };
    
    virtual ~GnashDevice() { };

    /// Get the type of the instatiated device. Since the
    /// renderer has a single value for the supported device,
    /// this is used to see which device has been loaded.
    virtual dtype_t getType() = 0;
    
    /// Initialize GNASH Device layer. This mostly just
    /// initializes all the private data.
    virtual bool initDevice(int argc, char *argv[]) = 0;

    /// Attach Native Window to device. This connects a
    /// Native Window to the device so surfaces can be created.
    virtual bool attachWindow(native_window_t window) = 0;
    
    // Utility methods not in the base class
    
    /// Return a string with the error code as text, instead of a numeric value
    virtual const char *getErrorString(int error) = 0;
    
    /// Query the system for all supported configs
    // int queryGNASHConfig() { return queryGNASHConfig(_gnashDisplay); };
    // int queryGNASHConfig(GNASHDisplay display);

    /// Get the stride of the device
    virtual size_t getStride() = 0;
 
    /// Get the width of the device
    virtual size_t getWidth() = 0;

    /// Get the Height of the device
    virtual size_t getHeight() = 0;

    /// Get the depth of the device
    virtual int getDepth() = 0;

    /// Get the size of the Red pixel
    virtual int getRedSize() = 0;

    /// Get the size of the Green pixel
    virtual int getGreenSize() = 0;

    /// Get the size of the Blue pixel
    virtual int getBlueSize() = 0;

    /// Is this device single buffered
    virtual bool isSingleBuffered() = 0;

    /// Are buffers destroyed ?
    virtual bool isBufferDestroyed() = 0;

    /// Get the window ID handle
    virtual int getID() = 0;

    /// Is the specified renderer supported by this hardware ?
    virtual bool supportsRenderer(rtype_t rtype) = 0;

    /// Is this renderering natively
    virtual bool isNativeRender() = 0;

    // These are only used for the Framebuffer

    /// Get the memory from the real framebuffer
    virtual boost::uint8_t *getFBMemory() { return 0; };
    
    /// Get the memory from an offscreen buffer to support Double Buffering
    virtual boost::uint8_t *getOffscreenBuffer() { return 0; };

    virtual size_t getFBMemSize() { return 0; };

    // bindClient() is used by OpenVG, OpenGLES1, and OpenGLES2
    // to bind the client type to the EGL surface. This method
    // is unused by the RawFB, DirectFB, and X11 Devices.
    virtual bool bindClient(GnashDevice::rtype_t) { return false; };
    
    virtual bool swapBuffers() {
        GNASH_REPORT_FUNCTION;
        return false;
    };
};
    
} // namespace renderer
} // namespace gnash

#endif  // end of __GNASH_DEVICE_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
