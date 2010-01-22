// GnashVaapiImage.cpp: GnashImage class used with VA API
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

#include "GnashVaapiImage.h"
#include "VaapiImage.h"
#include "vaapi.h"
#include <time.h>

#define DEBUG 0
#include "vaapi_debug.h"

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

/// Get scanline pitch for the specified image type
static inline int get_pitch(int width, ImageType type)
{
    int bytes_per_pixel;

    switch (type) {
    case GNASH_IMAGE_RGB:
	bytes_per_pixel = 3;
	break;
    case GNASH_IMAGE_RGBA:
	bytes_per_pixel = 4;
	break;
    default:
	assert(0);
	bytes_per_pixel = 0;
	break;
    }
    return width * bytes_per_pixel;
}

GnashVaapiImage::GnashVaapiImage(boost::shared_ptr<VaapiSurface> surface, ImageType type)
    : GnashImage(NULL, surface->width(), surface->height(), get_pitch(surface->width(), type),
		 type, GNASH_IMAGE_GPU)
    , _surface(surface)
    , _creation_time(get_ticks_usec())
{
    D(bug("GnashVaapiImage::GnashVaapiImage(): surface 0x%08x, size %dx%d\n",
	  _surface->get(), _width, _height));
}

GnashVaapiImage::GnashVaapiImage(const GnashVaapiImage& o)
    : GnashImage(NULL, o.width(), o.height(), get_pitch(o.width(), o.type()),
		 o.type(), GNASH_IMAGE_GPU)
    , _surface(o.surface())
    , _creation_time(get_ticks_usec())
{
    D(bug("GnashVaapiImage::GnashVaapiImage(): VA image %p\n", &o));

    update(o);
}

GnashVaapiImage::~GnashVaapiImage()
{
    D(bug("GnashVaapiImage::~GnashVaapiImage(): surface 0x%08x\n",
	  _surface->get()));
}

std::auto_ptr<GnashImage> GnashVaapiImage::clone()
{
    D(bug("GnashVaapiImage::clone(): image %p\n", this));

    return std::auto_ptr<GnashImage>(new GnashVaapiImage(*this));
}

void GnashVaapiImage::update(boost::shared_ptr<VaapiSurface> surface)
{
    _surface = surface;
    _creation_time = get_ticks_usec();
}

void GnashVaapiImage::update(boost::uint8_t* data)
{
    D(bug("GnashVaapi::update(): data %p\n", data));

    /* XXX: use vaPutImage() */
    _creation_time = get_ticks_usec();
}

void GnashVaapiImage::update(const GnashImage& from)
{
    assert(_pitch == from.pitch());
    assert(_size <= from.size());
    assert(_type == from.type());

    switch (from.location()) {
    case GNASH_IMAGE_CPU:
	this->update(const_cast<boost::uint8_t *>(from.data()));
	break;
    case GNASH_IMAGE_GPU:
	this->update(static_cast<const GnashVaapiImage &>(from).surface());
	break;
    default:
	assert(0);
	break;
    }
}

// Transfer (and convert) VA surface to CPU image data
bool GnashVaapiImage::transfer()
{
    boost::uint8_t *pixels;

    switch (_type) {
    case GNASH_IMAGE_RGB:
	pixels = _surface->getPixelsRGB();
	break;
    case GNASH_IMAGE_RGBA:
	pixels = _surface->getPixelsRGBA();
	break;
    default:
	assert(0);
	pixels = NULL;
    }

    if (!pixels)
	return false;

    _data.reset(pixels);
    return true;
}

// Get access to the underlying data
boost::uint8_t* GnashVaapiImage::data()
{
    D(bug("GnashVaapiImage::data(): surface 0x%08x\n", _surface->get()));
    D(bug("  -> %u usec from creation\n",
	  (boost::uint32_t)(get_ticks_usec() - _creation_time)));

    if (!transfer())
	return NULL;

    return _data.get();
}

// Get read-only access to the underlying data
const boost::uint8_t* GnashVaapiImage::data() const
{
    D(bug("GnashVaapiImage::data() const: surface 0x%08x\n", _surface->get()));
    D(bug("  -> %u usec from creation\n",
	  (boost::uint32_t)(get_ticks_usec() - _creation_time)));

    /* XXX: awful hack... */
    if (!const_cast<GnashVaapiImage *>(this)->transfer())
	return NULL;

    return _data.get();
}

} // gnash namespace
