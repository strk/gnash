// GnashVaapiImage.cpp: GnashImage class used with VA API
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

#include <time.h>

#include "log.h"
#include "GnashVaapiImage.h"
#include "VaapiSurface.h"

namespace gnash {

/// Get current value of microsecond timer
static boost::uint64_t get_ticks_usec(void)
{
#ifdef HAVE_CLOCK_GETTIME
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return (boost::uint64_t)t.tv_sec * 1000000 + t.tv_nsec / 1000;
#else
    struct timeval t;
    gettimeofday(&t, NULL);
    return (boost::uint64_t)t.tv_sec * 1000000 + t.tv_usec;
#endif
}

GnashVaapiImage::GnashVaapiImage(boost::shared_ptr<VaapiSurface> surface,
        image::ImageType type)
    :
    image::GnashImage(NULL, surface->width(), surface->height(), type,
            image::GNASH_IMAGE_GPU),
    _surface(surface),
    _creation_time(get_ticks_usec())
{
    log_debug("GnashVaapiImage::GnashVaapiImage(): surface 0x%08x, size %dx%d\n",
          _surface->get(), _width, _height);
}

GnashVaapiImage::~GnashVaapiImage()
{
    log_debug("GnashVaapiImage::~GnashVaapiImage(): surface 0x%08x\n",
          _surface->get());
}

void GnashVaapiImage::update(boost::shared_ptr<VaapiSurface> surface)
{
    _surface = surface;
    _creation_time = get_ticks_usec();
}

void GnashVaapiImage::update(boost::uint8_t* data)
{
    log_debug("GnashVaapi::update(): data %p\n", data);

    // XXX: use vaPutImage()
    _creation_time = get_ticks_usec();
}

void GnashVaapiImage::update(const image::GnashImage& from)
{
    assert(stride() == from.stride());
    assert(size() <= from.size());
    assert(type() == from.type());

    switch (from.location()) {
        case image::GNASH_IMAGE_CPU:
            this->update(const_cast<boost::uint8_t*>(from.begin()));
            break;
        case image::GNASH_IMAGE_GPU:
            this->update(static_cast<const GnashVaapiImage&>(from).surface());
            break;
        default:
            assert(0);
            break;
    }
}

// Transfer (and convert) VA surface to CPU image data
bool GnashVaapiImage::transfer()
{
    // NOTE: if VAAPI is used, we have a dedicated backend, so we
    //       should not have to retrieve the VA surface underlying pixels.
    //       Mark this usage scenario as a fatal error and fix the code
    //       instead.
    log_error("GnashVaapiImage: VA surface to SW pixels are not supported\n");
    assert(0);

    _data.reset();
    return _data.get() != NULL;
}

// Get access to the underlying data
image::GnashImage::iterator
GnashVaapiImage::begin()
{
    log_debug("GnashVaapiImage::data(): surface 0x%08x\n", _surface->get());
    log_debug("  -> %u usec from creation\n",
              (boost::uint32_t)(get_ticks_usec() - _creation_time));

    if (!transfer()) {
        return NULL;
    }

    return _data.get();
}

// Get read-only access to the underlying data
image::GnashImage::const_iterator
GnashVaapiImage::begin() const
{
    log_debug("GnashVaapiImage::data() const: surface 0x%08x\n", _surface->get());
    log_debug("  -> %u usec from creation\n",
          (boost::uint32_t)(get_ticks_usec() - _creation_time));

    /* XXX: awful hack... */
    if (!const_cast<GnashVaapiImage *>(this)->transfer()) {
        return NULL;
    }

    return _data.get();
}

} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
