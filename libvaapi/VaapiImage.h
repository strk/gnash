// VaapiImage.h: VA image abstraction
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

#ifndef GNASH_VAAPIIMAGE_H
#define GNASH_VAAPIIMAGE_H

#include "vaapi_common.h"

namespace gnash {

// Forward declarations
class VaapiSurface;

/// VA image data
class VaapiImage {
    VAImage		_image;
    boost::uint8_t *	_data;

    bool map();
    bool unmap();
    void destroy();

public:
    VaapiImage(const VaapiSurface *surface);
    ~VaapiImage();

    /// Update VA image with surface
    bool update(const VaapiSurface *surface);

    /// Get image type (FOURCC)
    boost::uint32_t fourcc() const
	{ return _image.format.fourcc; }

    /// Get VA image format
    const VAImageFormat &format() const
	{ return _image.format; }

    /// Get number of planes
    unsigned int getPlaneCount() const
	{ return _image.num_planes; }

    /// Get pixels for the specified plane
    boost::uint8_t *getPlane(int plane) const;

    /// Get scanline pitch for the specified plane
    unsigned int getPitch(int plane) const;
};

} // gnash namespace

#endif /* GNASH_VAAPIIMAGE_H */
