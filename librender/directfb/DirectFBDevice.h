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

// #ifdef HAVE_DIRECTFB_H
# include <directfb/directfb.h>
// #else
// # error "This file needs DIRECTFB"
// #endif

namespace gnash
{

namespace renderer {

namespace directfb {

class DirectFBDevice
{
  public:
    DirectFBDevice();
    ~DirectFBDevice();

    // Initialize DirectFB Device layer
    bool initDevice(int argc, char *argv[]);

    // Initialize DirectFB Window layer
    //    bool initDirectFB(DirectFBNativeWindowType window);
    
    // Utility methods not in the base class
    /// Return a string with the error code as text, instead of a numeric value
    const char *getErrorString(DFBResult error);    
    
    // Accessors for the settings needed by higher level code.
    // Surface accessors
    size_t getWidth() {
	return getWidth(_surface.get());
    }
    
    size_t getWidth(IDirectFBSurface *surface) {
	int x, y;
	if (surface) {
	    surface->GetSize(surface, &x, &y);
	    return static_cast<size_t>(x);
	}
	return 0;
    };
    size_t getHeigth() {
        return getHeigth(_surface.get());
    }
    
    size_t getHeigth(IDirectFBSurface *surface) {
	int x, y;
	if (surface) {
	    surface->GetSize(surface, &x, &y);
	    return static_cast<size_t>(y);
	}
	return 0;
    }
    size_t getVerticalRes() {
    }
    size_t getHorzRes() {
    }
    bool isSurfaceSingleBuffered() {
        // return isSurfaceSingleBuffered(_directfbSurface);
    }
    
    bool isSurfaceSingleBuffered(IDirectFBSurface surface) {
    }
    
    bool isSurfaceBackBuffered() {
        // return isSurfaceBackBuffered(_directfbSurface);
    }
    bool isSurfaceBackBuffered(IDirectFBSurface surface) {
    }

    bool isBufferDestroyed() {
        // return isBufferDestroyed(_directfbSurface);
    }
    bool isBufferDestroyed(IDirectFBSurface surface) {
    }
    bool isMultiSample() {
    }
    int getSurfaceID() {
    }

    // Context accessors
    int getContextID() {
	if (_dfb) {
	    IDirectFBDisplayLayer layer;
	    // _dfb->GetID(_dfb, &layer);
	}
	return 0;
    }

    bool isContextSingleBuffered() {
    }
    bool isContextBackBuffered() {
    }

    bool isNativeRender() {
    }
    int getSamples() {
    }
    int getSampleBuffers() {
    }
    int getDepth() {
    }
    int getMaxSwapInterval() {
    }
    int getMinSwapInterval() {
    }
    
    /// print the data in a DirectFB
    void printDirectFB() {
        printDirectFB(_dfb);
    };
    void printDirectFB(IDirectFB *fb);

    /// print the data in a DirectFBSurface
    void printFBSurface() {
        printFBSurface(_surface.get());
    };
    void printFBSurface(IDirectFBSurface *surface);

    /// print the data in a DirectFBFont
    void printFBFont() {
        printFBFont(_font.get());
    };
    void printFBFont(IDirectFBFont *font);

    /// print the data in a DirectFBInputDevice
    void printFBInputDevice() {
        printFBInputDevice(_keyboard.get());
    };
    void printFBInputDevice(IDirectFBInputDevice *input);

protected:
    void printAccelerationMask(DFBAccelerationMask mask);
    void printSurfaceBlittingFlags(DFBSurfaceBlittingFlags flags);
    void printSurfaceDrawingFlags(DFBSurfaceDrawingFlags flags);
    void printGrapbicsDriverIndo(DFBGraphicsDriverInfo *driver);
    
    IDirectFB				      *_dfb;
    boost::scoped_ptr<IDirectFBSurface>        _surface;
    boost::scoped_ptr<IDirectFBInputDevice>    _keyboard;
    boost::scoped_ptr<IDirectFBEventBuffer>    _keybuffer;
    boost::scoped_ptr<IDirectFBImageProvider>  _provider;
    boost::scoped_ptr<IDirectFBFont>           _font;
    boost::scoped_ptr<IDirectFBDisplayLayer>   _id;
};

} // namespace directfb
} // namespace renderer
} // namespace gnash

#endif  // end of __DIRECTFB_DEVICE_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
