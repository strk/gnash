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

#ifndef __DIRECTFB_DEVICE_H__
#define __DIRECTFB_DEVICE_H__ 1

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>

#ifdef HAVE_DIRECTFB_DIRECTFB_H
# include <directfb/directfb.h>
#else
# error "This file needs DIRECTFB"
#endif

#include "GnashDevice.h"

namespace gnash {

namespace renderer {

namespace directfb {

class DirectFBDevice : public GnashDevice
{
  public:
    DirectFBDevice();
    DirectFBDevice(int argc, char *argv[]);

    ~DirectFBDevice();

    dtype_t getType() { return DIRECTFB; };

    // Initialize DirectFB Device layer
    bool initDevice(int argc, char *argv[]);

    // Initialize DirectFB Window layer
    //    bool initDirectFB(DirectFBNativeWindowType window);
    bool attachWindow(GnashDevice::native_window_t window);
        
    // Utility methods not in the base class
    /// Return a string with the error code as text, instead of a numeric value
    const char *getErrorString(int error);    
    
    // Accessors for the settings needed by higher level code.
    // Surface accessors
    size_t getWidth() {
        return getWidth(_surface);
    }
    
    size_t getHeight() {
        return getHeight(_surface);
    }
    
    int getDepth() {
        DFBSurfacePixelFormat format;
        if (_surface) {
            _surface->GetPixelFormat(_surface, &format);
            return getDepth(format);
        }
        return 0;
    }

    int getRedSize() {
        return 0;
    };
    int getGreenSize() {
        return 0;
    };
    int getBlueSize() {
        return 0;
    };
    
    bool isSingleBuffered() {
        if (_surface) {
            DFBSurfaceCapabilities caps;
            _surface->GetCapabilities(_surface, &caps);
            if (caps & DSCAPS_DOUBLE) {
                return false;
            }
        }
        return true;
    }

    int getID() {
        return static_cast<int>(getSurfaceID());
    }
    
    bool isBufferDestroyed() {
        // return isBufferDestroyed(_directfbSurface);
        return false;
    }

    int getSurfaceID() {
	if (_layer) {
            DFBDisplayLayerID id;
            _screen->GetID(_screen, &id);
            return static_cast<int>(id);
	}
        return 0;
    }

    virtual bool supportsRenderer(rtype_t /* rtype */) { return true; };

    // Overload some of the base class methods to deal with Device specific
    // data types.
    int getDepth(DFBSurfacePixelFormat format);

    size_t getWidth(IDirectFBSurface *surface) {
	int x, y;
	if (surface) {
	    surface->GetSize(surface, &x, &y);
	    return static_cast<size_t>(x);
	}
	return 0;
    };
    size_t getHeight(IDirectFBSurface *surface) {
	int x, y;
	if (surface) {
	    surface->GetSize(surface, &x, &y);
	    return static_cast<size_t>(y);
	}
	return 0;
    }
    bool isSurfaceBackBuffered() {
        if (_surface) {
            DFBSurfaceCapabilities caps;
            _surface->GetCapabilities(_surface, &caps);
            if (caps & DSCAPS_DOUBLE) {
                return true;
            }        
            return false;
        }
    }
    // Context accessors
    int getContextID() {
	if (_layer) {
            DFBDisplayLayerID id;
            _layer->GetID(_layer, &id);
            return static_cast<int>(id);
	}
	return 0;
    }

    bool isContextSingleBuffered() {
        if (_layer) {
            DFBDisplayLayerConfig config;
            _layer->GetConfiguration(_layer, &config);
            if (config.buffermode & DLBM_FRONTONLY) {
                return true;
            }        
            return false;
        }
        return false;
    }
    bool isContextBackBuffered() {
        if (_layer) {
            DFBDisplayLayerConfig config;
            _layer->GetConfiguration(_layer, &config);
            if (config.buffermode & DLBM_FRONTONLY) {
                return false;
            }        
            return true;
        }
        return true;
    }

    bool isNativeRender() {
        return true;
    }
    
    size_t getVerticalRes() {
        return getVerticalRes(_screen);
    }
    size_t getVerticalRes(IDirectFBScreen *screen) {
	int x, y;
	if (screen) {
	    screen->GetSize(screen, &x, &y);
	    return static_cast<size_t>(x);
	}
        return 0;
    }
    size_t getHorzRes() {
        return getHorzRes(_screen);
    }
    size_t getHorzRes(IDirectFBScreen *screen) {
	int x, y;
	if (screen) {
	    screen->GetSize(screen, &x, &y);
	    return static_cast<size_t>(y);
	}
        return 0;
    }
    
    /// print the data in a DirectFB
    void printDirectFB() {
        printDirectFB(_dfb);
    };
    void printDirectFB(IDirectFB *fb);

    /// print the data in a DirectFBSurface
    void printFBSurface() {
        printFBSurface(_surface);
    };
    void printFBSurface(IDirectFBSurface *surface);

    /// print the data in a DirectFBFont
    void printFBFont() {
        printFBFont(_font);
    };
    void printFBFont(IDirectFBFont *font);

    /// print the data in a DirectFBDisplay
    void printFBDisplay() {
        printFBDisplay(_layer);
    };
    void printFBDisplay(IDirectFBDisplayLayer *display);

    void printFBLayer() {
        printFBDisplayLayer(_layer);
    };
    void printFBDisplayLayer(IDirectFBDisplayLayer *layer);

    /// print the data in a DirectFBFont
    void printFBScreen() {
        printFBScreen(_screen);
    };
    void printFBScreen(IDirectFBScreen *screen);

    /// print the data in a DirectFBInputDevice
    void printFBInputDevice() {
        printFBInputDevice(_keyboard);
    };
    void printFBInputDevice(IDirectFBInputDevice *input);

protected:
    void printAccelerationMask(DFBAccelerationMask mask);
    void printSurfaceBlittingFlags(DFBSurfaceBlittingFlags flags);
    void printSurfaceDrawingFlags(DFBSurfaceDrawingFlags flags);
    void printGrapbicsDriverIndo(DFBGraphicsDriverInfo *driver);
    void printSurfaceDescriptionFlags(DFBSurfaceDescriptionFlags flags);
    void printSurfaceCapabilities(DFBSurfaceCapabilities caps);
    void printSurfacePixelFormat(DFBSurfacePixelFormat format);
    void printDisplayLayerTypeFlags(DFBDisplayLayerTypeFlags flags);
    void printDisplayLayerCapabilities(DFBDisplayLayerCapabilities caps);
    void printfScreenCapabilities(DFBScreenCapabilities caos);
    void printDisplayLayerConfig(DFBDisplayLayerConfig *config);
    void printDisplayLayerBufferMode(DFBDisplayLayerBufferMode mode);

    void printColor(DFBColor color);
    //    void printFBSurfaceHintFlags(DFBSurfaceHintFlags flags);
    
    IDirectFB		     *_dfb;
    IDirectFBSurface         *_surface;
    IDirectFBInputDevice     *_keyboard;
    IDirectFBEventBuffer     *_keybuffer;
    IDirectFBImageProvider   *_provider;
    IDirectFBFont            *_font;
    IDirectFBDisplayLayer    *_layer;
    IDirectFBScreen          *_screen;
};

} // namespace directfb
} // namespace renderer
} // namespace gnash

#endif  // end of __DIRECTFB_DEVICE_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
