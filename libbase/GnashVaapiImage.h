// GnashVaapiImage.h: GnashImage class used with VA API
// 
// Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#ifndef GNASH_GNASHVAAPIIMAGE_H
#define GNASH_GNASHVAAPIIMAGE_H

#include <boost/shared_ptr.hpp>

#include "GnashImage.h"
#include "dsodefs.h"

namespace gnash {

// Forward declarations
class VaapiSurface;
class VaapiSurfaceProxy;

/// GnashImage implementation using a VA surface
class DSOEXPORT GnashVaapiImage : public image::GnashImage
{
    boost::shared_ptr<VaapiSurface> _surface;
    boost::uint64_t _creation_time;

    /// Transfer (and convert) VA surface to CPU image data
    bool transfer();

public:
    GnashVaapiImage(boost::shared_ptr<VaapiSurface> surface,
            image::ImageType type);
    GnashVaapiImage(const GnashVaapiImage& o);
    ~GnashVaapiImage();

    virtual void update(boost::shared_ptr<VaapiSurface> surface);
    virtual void update(boost::uint8_t* data);
    virtual void update(const image::GnashImage& from);

    /// Get access to the underlying surface
    //
    /// @return     A pointer to the VA surface.
    boost::shared_ptr<VaapiSurface> surface() const
        { return _surface; }

    /// Get access to the underlying data
    //
    /// NOTE: This function shall not be used
    //
    /// @return     NULL.
    virtual iterator begin();

    /// Get read-only access to the underlying data
    //
    /// @return     A read-only pointer to the raw image data.
    virtual const_iterator begin() const;
};

} // gnash namespace

#endif // GNASH_GNASHVAAPIIMAGE_H

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
