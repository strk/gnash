// GnashVaapiImageProxy.h: GnashVaapiImage proxy class used for delayed rendering
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

#ifndef GNASH_GNASHVAAPIIMAGEPROXY_H
#define GNASH_GNASHVAAPIIMAGEPROXY_H

#include <boost/shared_ptr.hpp>

namespace gnash {

// Forward declarations
class GnashVaapiImage;
class VaapiSurface;

/// GnashVaapiImage proxy for delayed rendering
/// XXX: call it GnashRenderedVaapiImage instead?
class DSOEXPORT GnashVaapiImageProxy
{
    /* XXX: Should Renderers use boost::shared_ptr<> we could simple
       derive from a GnashImageProxy base that would itself contain a
       shared pointer to the image */
    boost::shared_ptr<VaapiSurface> _surface;

    /// X-position of the rendered image, in pixels
    const int _x;

    /// Y-position of the rendered image, in pixels
    const int _y;

    /// Width of the rendered image, in pixels
    const size_t _width;

    /// Height of the rendered image, in pixels
    const size_t _height;

public:
    GnashVaapiImageProxy(GnashVaapiImage *image, int x, int y, size_t w, size_t h)
        : _surface(image->surface()), _x(x), _y(y), _width(w), _height(h)
        { }

    GnashVaapiImageProxy(const GnashVaapiImageProxy& o)
        : _surface(o.surface())
        , _x(o.x()), _y(o.y()), _width(o.width()), _height(o.height())
        { }

    /// Get access to the underlying surface
    //
    /// @return     A pointer to the VA surface.
    boost::shared_ptr<VaapiSurface> surface() const
        { return _surface; }

    /// Get the rendered image's x position
    //
    /// @return     The rendered image's x position in pixels.
    int x() const { return _x; }

    /// Get the rendered image's y position
    //
    /// @return     The rendered image's y position in pixels.
    int y() const { return _y; }

    /// Get the rendered image's width
    //
    /// @return     The rendered image's width in pixels.
    size_t width() const { return _width; }

    /// Get the rendered image's width
    //
    /// @return     The rendered image's height in pixels.
    size_t height() const { return _height; }
};

} // gnash namespace

#endif // end of GNASH_GNASHVAAPIIMAGEPROXY_H


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
