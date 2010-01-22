// VaapiContext.cpp: VA context abstraction
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

#include "VaapiContext.h"
#include "VaapiGlobalContext.h"
#include "VaapiDisplay.h"
#include "VaapiSurface.h"
#include "vaapi_utils.h"

#define DEBUG 0
#include "vaapi_debug.h"

namespace gnash {

/// Translates VAProfile to VaapiCodec
static VaapiCodec get_codec(VAProfile profile)
{
    switch (profile) {
    case VAProfileMPEG2Simple:
    case VAProfileMPEG2Main:
	return VAAPI_CODEC_MPEG2;
    case VAProfileMPEG4Simple:
    case VAProfileMPEG4AdvancedSimple:
    case VAProfileMPEG4Main:
	return VAAPI_CODEC_MPEG4;
    case VAProfileH264Baseline:
    case VAProfileH264Main:
    case VAProfileH264High:
	return VAAPI_CODEC_H264;
    case VAProfileVC1Simple:
    case VAProfileVC1Main:
    case VAProfileVC1Advanced:
	return VAAPI_CODEC_VC1;
    default:
	break;
    }
    abort();
    return VAAPI_CODEC_UNKNOWN;
}

/// Returns number of VA surfaces to create for a specified codec
static unsigned int get_max_surfaces(VaapiCodec codec)
{
    // Number of scratch surfaces beyond those used as reference
    const unsigned int SCRATCH_SURFACES_COUNT = 8;

    // Make sure pool of created surfaces for H.264 is under 64 MB for 1080p
    const unsigned int MAX_SURFACE_SIZE   = (1920 * 1080 * 3) / 2;
    const unsigned int MAX_VIDEO_MEM_SIZE = 64 * 1024 * 1024;
    const unsigned int MAX_SURFACES_COUNT = MAX_VIDEO_MEM_SIZE / MAX_SURFACE_SIZE;

    unsigned int max_surfaces;
    max_surfaces = (codec == VAAPI_CODEC_H264 ? 16 : 2) + SCRATCH_SURFACES_COUNT;
    if (max_surfaces > MAX_SURFACES_COUNT)
	max_surfaces = MAX_SURFACES_COUNT;

    return max_surfaces;
}

VaapiContext::VaapiContext(VAProfile profile, VAEntrypoint entrypoint)
    : _config(VA_INVALID_ID)
    , _context(VA_INVALID_ID)
    , _codec(get_codec(profile))
    , _profile(profile)
    , _entrypoint(entrypoint)
    , _picture_width(0), _picture_height(0)
{
    D(bug("VaapiContext::VaapiContext(): profile %d, entrypoint %d\n", profile, entrypoint));
    construct();
}

VaapiContext::~VaapiContext()
{
    D(bug("VaapiContext::~VaapiContext(): context 0x%08x\n", _context));
    destruct();
}

bool VaapiContext::construct()
{
    VaapiGlobalContext * const gvactx = VaapiGlobalContext::get();
    if (!gvactx)
	return false;

    _display = gvactx->display();
    if (!_display)
	return false;

    VAStatus status;
    VAConfigAttrib attrib;
    attrib.type = VAConfigAttribRTFormat;
    status = vaGetConfigAttributes(_display, _profile, _entrypoint, &attrib, 1);
    if (!vaapi_check_status(status, "vaGetConfigAttributes()"))
        return false;
    if ((attrib.value & VA_RT_FORMAT_YUV420) == 0)
        return false;

    VAConfigID config;
    status = vaCreateConfig(_display, _profile, _entrypoint, &attrib, 1, &config);
    if (!vaapi_check_status(status, "vaCreateConfig()"))
	return false;

    _config = config;
    return true;
}

void VaapiContext::destruct()
{
    destroyContext();

    if (_config != VA_INVALID_ID) {
	VAStatus status = vaDestroyConfig(_display, _config);
	vaapi_check_status(status, "vaDestroyConfig()");
    }
}

bool VaapiContext::createContext(unsigned int width, unsigned int height)
{
    if (_config == VA_INVALID_ID)
	return false;

    const unsigned int num_surfaces = get_max_surfaces(_codec);
    std::vector<VASurfaceID> surface_ids;
    surface_ids.reserve(num_surfaces);
    for (unsigned int i = 0; i < num_surfaces; i++) {
	VaapiSurfaceSP surface(new VaapiSurface(width, height));
	_surfaces.push(surface);
	surface_ids.push_back(surface->get());
    }

    VAStatus status;
    VAContextID context;
    status = vaCreateContext(_display,
			     _config,
			     width, height,
			     VA_PROGRESSIVE,
			     &surface_ids[0], surface_ids.size(),
			     &context);
    if (!vaapi_check_status(status, "vaCreateContext()"))
	return false;

    _context		= context;
    _picture_width	= width;
    _picture_height	= height;
    D(bug("  -> context 0x%08x\n", _context));
    return true;
}

void VaapiContext::destroyContext()
{
    VAStatus status;

    if (_context != VA_INVALID_ID) {
	status = vaDestroyContext(_display,_context);
	if (!vaapi_check_status(status, "vaDestroyContext()"))
	    return;
	_context = VA_INVALID_ID;
    }

    for (unsigned int i = 0; i < _surfaces.size(); i++)
	_surfaces.pop();
    _picture_width  = 0;
    _picture_height = 0;
}

bool VaapiContext::initDecoder(unsigned int width, unsigned int height)
{
    if (_picture_width == width && _picture_height == height)
	return true;

    destroyContext();
    return createContext(width, height);
}

/// Get a free surface
boost::shared_ptr<VaapiSurface> VaapiContext::acquireSurface()
{
    boost::shared_ptr<VaapiSurface> surface = _surfaces.front();
    _surfaces.pop();
    D(bug("VaapiContext::acquireSurface(): surface 0x%08x\n", surface->get()));
    return surface;
}

/// Release surface
void VaapiContext::releaseSurface(boost::shared_ptr<VaapiSurface> surface)
{
    D(bug("VaapiContext::releaseSurface(): surface 0x%08x\n", surface->get()));
    _surfaces.push(surface);
}

} // gnash namespace
