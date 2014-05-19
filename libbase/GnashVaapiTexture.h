// GnashVaapiTexture.h: GnashImage class used for VA/GLX rendering
// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#ifndef GNASH_GNASHVAAPITEXTURE_H
#define GNASH_GNASHVAAPITEXTURE_H

#include "GnashTexture.h"

namespace gnash {

// Forward declarations
class VaapiSurface;
class VaapiSurfaceGLX;

/// OpenGL texture abstraction
class DSOEXPORT GnashVaapiTexture : public GnashTexture {
    std::unique_ptr<VaapiSurfaceGLX> _surface;

public:
    GnashVaapiTexture(unsigned int width, unsigned int height, 
            image::ImageType type);
    ~GnashVaapiTexture();

    /// Copy texture data from a VA surface.
    //
    /// Note that this surface MUST have the same _pitch, or
    /// unexpected things will happen.
    ///
    /// @param surface VA surface to copy data from.
    void update(boost::shared_ptr<VaapiSurface> surface);
};

} // gnash namespace

#endif // end of GNASH_GNASHVAAPITEXTURE_H


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
