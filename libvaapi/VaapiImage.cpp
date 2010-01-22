// VaapiImage.cpp: VA image abstraction
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

#include "VaapiImage.h"
#include "VaapiSurface.h"
#include "VaapiGlobalContext.h"
#include "VaapiException.h"
#include "vaapi_utils.h"
#include <string.h>

#define DEBUG 0
#include "vaapi_debug.h"

namespace gnash {

VaapiImage::VaapiImage(const VaapiSurface *surface)
    : _data(NULL)
{
    D(bug("VaapiImage::VaapiImage(): surface 0x%08x\n", surface.get()));

    memset(&_image, 0, sizeof(_image));
    _image.image_id = VA_INVALID_ID;

    update(surface);
}

VaapiImage::~VaapiImage()
{
    D(bug("VaapiImage::~VaapiImage()\n"));

    destroy();
}

bool VaapiImage::map()
{
    VaapiGlobalContext * const gvactx = VaapiGlobalContext::get();
    if (!gvactx)
	return false;

    VAStatus status;
    status = vaMapBuffer(gvactx->display(), _image.buf, (void **)&_data);
    return vaapi_check_status(status, "vaMapBuffer()");
}

bool VaapiImage::unmap()
{
    if (!_data)
	return true;

    _data = NULL;

    VaapiGlobalContext * const gvactx = VaapiGlobalContext::get();
    if (!gvactx)
	return false;

    VAStatus status;
    status = vaUnmapBuffer(gvactx->display(), _image.buf);
    return vaapi_check_status(status, "vaUnmapBuffer()");
}

void VaapiImage::destroy()
{
    unmap();

    if (_image.image_id == VA_INVALID_ID)
	return;

    VaapiGlobalContext * const gvactx = VaapiGlobalContext::get();
    if (!gvactx)
	return;

    VAStatus status;
    status = vaDestroyImage(gvactx->display(), _image.image_id);
    if (!vaapi_check_status(status, "vaDestroyImage()"))
	return;

    _image.image_id = VA_INVALID_ID;
}

bool VaapiImage::update(const VaapiSurface *surface)
{
    VaapiGlobalContext * const gvactx = VaapiGlobalContext::get();
    if (!gvactx)
	return false;

    VAStatus status;
    status = vaSyncSurface(gvactx->display(), surface->get());
    if (!vaapi_check_status(status, "vaSyncSurface()"))
	return false;

    static const boost::uint32_t image_formats[] = {
	VA_FOURCC('N','V','1','2'),
	VA_FOURCC('Y','V','1','2'),
	VA_FOURCC('U','Y','V','Y'),
	VA_FOURCC('Y','U','Y','V'),
	VA_FOURCC('R','G','B','A'),
	0
    };

    const VAImageFormat *image_format = NULL;
    for (int i = 0; image_formats[i] != 0; i++) {
	const VAImageFormat *m = gvactx->getImageFormat(image_formats[i]);
	if (m) {
	    image_format = m;
	    break;
	}
    }
    if (!image_format)
	return false;

    unmap();

    if (_image.width != surface->width() ||
	_image.height != surface->height() ||
	_image.format.fourcc != image_format->fourcc) {
	/* XXX: check RGBA formats further */

	destroy();

	status = vaCreateImage(gvactx->display(),
			       const_cast<VAImageFormat *>(image_format),
			       surface->width(), surface->height(),
			       &_image);
	if (!vaapi_check_status(status, "vaCreateImage()"))
	    return false;
    }

    status = vaGetImage(gvactx->display(), surface->get(),
			0, 0, surface->width(), surface->height(),
			_image.image_id);
    if (!vaapi_check_status(status, "vaGetImage()"))
	return false;

    return map();
}

// Get pixels for the specified plane
boost::uint8_t *VaapiImage::getPlane(int plane) const
{
    assert(_image.image_id != VA_INVALID_ID);
    return _data ? &_data[_image.offsets[plane]] : NULL;
}

// Get scanline pitch for the specified plane
unsigned int VaapiImage::getPitch(int plane) const
{   
    assert(_image.image_id != VA_INVALID_ID);
    return _image.pitches[plane];
}

} // gnash namespace
