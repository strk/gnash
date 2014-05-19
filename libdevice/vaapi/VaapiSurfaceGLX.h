// VaapiSurfaceGLX.h: VA/GLX surface abstraction
// 
// Copyright (C) 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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

#ifndef GNASH_VAAPISURFACEGLX_H
#define GNASH_VAAPISURFACEGLX_H

#include <GL/gl.h>

#include "dsodefs.h"
#include "vaapi_common.h"
#include "VaapiSurface.h"

namespace gnash {

/// VA/GLX surface abstraction
class DSOEXPORT VaapiSurfaceGLX 
{
    std::unique_ptr<VaapiSurfaceImplBase> _impl;

public:
    VaapiSurfaceGLX(GLenum target, GLuint texture);

    /// Return VA surface id
    void *get() const { return reinterpret_cast<void *>(_impl->surface()); }

    /// Get surface width
    unsigned int width() const { return _impl->width(); }

    /// Get surface height
    unsigned int height() const { return _impl->height(); }

    /// Update VA/GLX surface from VA surface
    bool update(std::shared_ptr<VaapiSurface> surface);
};

} // gnash namespace

#endif // GNASH_VAAPISURFACEGLX_H

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:

