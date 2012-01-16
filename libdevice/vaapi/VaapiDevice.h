//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010. 2011, 2012
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

#ifndef __VAAPI_DEVICE_H__
#define __VAAPI_DEVICE_H__ 1

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>

#include <va/va.h>

namespace gnash
{

namespace renderer {

namespace vaapi {

class VaapiDevice
{
  public:
    VaapiDevice();
    ~VaapiDevice();

    // Initialize Vaapi Device layer
    bool initDevice(int argc, char *argv[]);

    // Initialize Vaapi Window layer
    //    bool initVaapi(VaapiNativeWindowType window);
    
    // Utility methods not in the base class
    /// Return a string with the error code as text, instead of a numeric value
    const char *getErrorString(DFBResult error);    
    
    // Accessors for the settings needed by higher level code.
    // Surface accessors
    size_t getWidth() {
        // return getWidth(_surface);
    }
    
    size_t getWidth(IVaapiSurface *surface) {
	return 0;
    };
    size_t getHeigth() {
        // return getHeigth(_surface);
    }
    
    size_t getHeigth(IVaapiSurface *surface) {
	return 0;
    }
    size_t getVerticalRes() {
        return getVerticalRes(_screen);
    }
    size_t getVerticalRes(IVaapiScreen *screen) {
        return 0;
    }
    size_t getHorzRes() {
        return getHorzRes(_screen);
    }
    size_t getHorzRes(IVaapiScreen *screen) {
        return 0;
    }

    bool isSurfaceSingleBuffered() {
        return true;
    }
    
    bool isSurfaceBackBuffered() {
        if (_surface) {
            DFBSurfaceCapabilities caps;
            _surface->GetCapabilities(_surface, &amp;caps);
            if (caps &amp; DSCAPS_DOUBLE) {
                return true;
            }        
            return false;
        }
    }
    bool isBufferDestroyed() {
        // return isBufferDestroyed(_vaapiSurface);
        return false;
    }
    bool isBufferDestroyed(IVaapiSurface surface) {
        return false;
    }
    bool isMultiSample() {
        return false;
    }
    int getSurfaceID() {
        return 0;
    }

    // Context accessors
    int getContextID() {
	return 0;
    }

    bool isContextSingleBuffered() {
        return false;
    }
    bool isContextBackBuffered() {
        return true;
    }

    int getDepth() {
    }
    int getDepth(DFBSurfacePixelFormat format);

private:
    
};

} // namespace vaapi
} // namespace renderer
} // namespace gnash

#endif  // end of __VAAPI_DEVICE_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
