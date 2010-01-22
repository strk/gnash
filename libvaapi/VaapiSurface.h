// VaapiSurface.h: VA surface abstraction
// 
//   Copyright (C) 2009 Splitted-Desktop Systems
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

#ifndef GNASH_VAAPISURFACE_H
#define GNASH_VAAPISURFACE_H

#include "vaapi_common.h"

namespace gnash {

// Forward declarations
class VaapiImage;

/// VA surface base representation
class VaapiSurfaceImplBase {
    uintptr_t		_surface;
    unsigned int	_width;
    unsigned int	_height;

protected:
    void reset(uintptr_t surface)
	{ _surface = surface; }

public:
    VaapiSurfaceImplBase(unsigned int width, unsigned int height);
    virtual ~VaapiSurfaceImplBase() { }

    /// Get VA surface
    uintptr_t surface() const
        { return _surface; }

    /// Get surface width
    unsigned int width() const
        { return _width; }

    /// Get surface height
    unsigned int height() const
        { return _height; }

    /// Get surface pixels in RGB or RGBA format
    //
    /// NOTE: data array is allocated with new[] and shall be freed
    virtual boost::uint8_t *getPixels(bool rgba = true)
	{ return NULL; }
};

/// VA surface abstraction
class VaapiSurface {
    std::auto_ptr<VaapiSurfaceImplBase> _impl;

public:
    VaapiSurface(unsigned int width, unsigned int height);

    /// Return VA surface id
    VASurfaceID get() const
        { return static_cast<VASurfaceID>(_impl->surface()); }

    /// Get surface width
    unsigned int width() const
        { return _impl->width(); }

    /// Get surface height
    unsigned int height() const
        { return _impl->height(); }

    /// Get surface pixels in RGB format
    //
    /// NOTE: data array is allocated with new[] and shall be freed
    boost::uint8_t *getPixelsRGB()
	{ return _impl->getPixels(false); }

    /// Get surface pixels in RGBA format
    //
    /// NOTE: data array is allocated with new[] and shall be freed
    boost::uint8_t *getPixelsRGBA()
	{ return _impl->getPixels(true); }
};

} // gnash namespace

#endif /* GNASH_VAAPISURFACE_H */
