// VaapiSurface.cpp: VA surface abstraction
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

#include "VaapiSurface.h"
#include "VaapiGlobalContext.h"
#include "VaapiImage.h"
#include "vaapi_utils.h"

extern "C" {
#include <libswscale/swscale.h>
}

#define DEBUG 0
#include "vaapi_debug.h"

namespace gnash {

/// A wrapper around an SwsContet that ensures it's freed on destruction.
class SwsContextWrapper {
    SwsContext *_context;

public:
    SwsContextWrapper(SwsContext *context)
	: _context(context)
	{ }

    ~SwsContextWrapper()
	{ sws_freeContext(_context); }

    SwsContext *get() const
	{ return _context; }
};

/// Get scanline pitch for the specified image type
static inline int get_pitch(int width, bool rgba)
{
    return (rgba ? 4 : 3) * width;
}

/// Translates VA image format to FFmpeg PixelFormat
static PixelFormat get_pixel_format(VAImageFormat const &image_format)
{
    switch (image_format.fourcc) {
    case VA_FOURCC('N','V','1','2'): return PIX_FMT_NV12;
    case VA_FOURCC('Y','V','1','2'): return PIX_FMT_YUV420P;
    case VA_FOURCC('U','Y','V','Y'): return PIX_FMT_UYVY422;
    case VA_FOURCC('Y','U','Y','V'): return PIX_FMT_YUYV422;
    }
    if (image_format.fourcc == VA_FOURCC('R','G','B','A')) {
	enum {
#ifdef WORDS_BIGENDIAN
	    BO_NATIVE = VA_MSB_FIRST,
	    BO_NONNAT = VA_LSB_FIRST,
#else
	    BO_NATIVE = VA_LSB_FIRST,
	    BO_NONNAT = VA_MSB_FIRST,
#endif
	};
	static const struct {
	    enum PixelFormat	pix_fmt;
	    unsigned char	byte_order;
	    unsigned char	bits_per_pixel;
	    unsigned int	red_mask;
	    unsigned int	green_mask;
	    unsigned int	blue_mask;
	}
	pix_fmt_map[] = {
	    { PIX_FMT_BGR32, BO_NATIVE, 32, 0x000000ff, 0x0000ff00, 0x00ff0000 },
	    { PIX_FMT_BGR32, BO_NONNAT, 32, 0xff000000, 0x00ff0000, 0x0000ff00 },
	    { PIX_FMT_RGB32, BO_NATIVE, 32, 0x00ff0000, 0x0000ff00, 0x000000ff },
	    { PIX_FMT_RGB32, BO_NONNAT, 32, 0x0000ff00, 0x00ff0000, 0xff000000 },
	    { PIX_FMT_NONE, 0, 0, 0, 0, 0 }
	};
	for (int i = 0; pix_fmt_map[i].pix_fmt != PIX_FMT_NONE; i++) {
	    if (pix_fmt_map[i].byte_order == image_format.byte_order &&
		pix_fmt_map[i].bits_per_pixel == image_format.bits_per_pixel &&
		pix_fmt_map[i].red_mask == image_format.red_mask &&
		pix_fmt_map[i].green_mask == image_format.green_mask &&
		pix_fmt_map[i].blue_mask == image_format.blue_mask)
		return pix_fmt_map[i].pix_fmt;
	}
    }
    return PIX_FMT_NONE;
}

VaapiSurfaceImplBase::VaapiSurfaceImplBase(unsigned int width, unsigned int height)
    : _surface(VA_INVALID_SURFACE), _width(width), _height(height)
{
}

class VaapiSurfaceImpl: public VaapiSurfaceImplBase {
    const VaapiSurface *		_surface;
    std::auto_ptr<VaapiImage>		_image;
    bool				_image_rgba;
    std::auto_ptr<SwsContextWrapper>	_swsContext;

public:
    VaapiSurfaceImpl(const VaapiSurface *surface, unsigned int width, unsigned int height);
    ~VaapiSurfaceImpl();

    /// Get surface pixels in RGB or RGBA format
    //
    /// NOTE: data array is allocated with new[] and shall be freed
    virtual boost::uint8_t *getPixels(bool rgba);
};

VaapiSurfaceImpl::VaapiSurfaceImpl(const VaapiSurface *surface,
				   unsigned int width, unsigned int height)
    : VaapiSurfaceImplBase(width, height)
    , _surface(surface)
{
    D(bug("VaapiSurface::VaapiSurface()\n"));

    if (width == 0 || height == 0)
	return;

    VaapiGlobalContext * gvactx = VaapiGlobalContext::get();
    if (!gvactx)
	return;

    VAStatus status;
    VASurfaceID surface_id;
    status = vaCreateSurfaces(gvactx->display(),
			      width, height, VA_RT_FORMAT_YUV420,
			      1, &surface_id);
    if (!vaapi_check_status(status, "vaCreateSurfaces()"))
	return;

    reset(surface_id);
    D(bug("  -> surface 0x%08x\n", surface()));
}

VaapiSurfaceImpl::~VaapiSurfaceImpl()
{
    D(bug("VaapiSurface::~VaapiSurface(): surface 0x%08x\n", surface()));

    if (surface() == VA_INVALID_SURFACE)
	return;

    VaapiGlobalContext * gvactx = VaapiGlobalContext::get();
    if (!gvactx)
	return;

    VAStatus status;
    VASurfaceID surface_id = surface();
    status = vaDestroySurfaces(gvactx->display(), &surface_id, 1);
    if (!vaapi_check_status(status, "vaDestroySurfaces()"))
	return;

    reset(VA_INVALID_SURFACE);
}

boost::uint8_t *VaapiSurfaceImpl::getPixels(bool rgba)
{
    if (_image.get())
	_image->update(_surface);
    else
	_image.reset(new VaapiImage(_surface));

    const PixelFormat srcPixFmt = get_pixel_format(_image->format());
    const PixelFormat dstPixFmt = rgba ? PIX_FMT_RGBA : PIX_FMT_RGB24;

    if (!_swsContext.get() || (_image_rgba ^ rgba) != 0)
	_swsContext.reset(new SwsContextWrapper(
			      sws_getContext(width(), height(), srcPixFmt,
					     width(), height(), dstPixFmt,
					     SWS_BILINEAR,
					     NULL, NULL, NULL)));
    _image_rgba = rgba;

    boost::int32_t srcPitch[4];
    boost::uint8_t *srcData[4];

    for (unsigned int i = 0; i < _image->getPlaneCount(); i++) {
	srcData[i]  = _image->getPlane(i);
	srcPitch[i] = _image->getPitch(i);
    }

    boost::int32_t dstPitch = get_pitch(width(), rgba);
    boost::uint8_t *dstData = new boost::uint8_t[height() * dstPitch];

    if (sws_scale(_swsContext->get(),
		  srcData, srcPitch, 0, height(),
		  &dstData, &dstPitch) < 0)
	return NULL;

    return dstData;
}

VaapiSurface::VaapiSurface(unsigned width, unsigned int height)
    : _impl(new VaapiSurfaceImpl(this, width, height))
{
}

} // gnash namespace
