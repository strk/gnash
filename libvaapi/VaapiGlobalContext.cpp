// VaapiGlobalContext.cpp: VA API global context
// 
// Copyright (C) 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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

#include "VaapiGlobalContext.h"
#include "VaapiDisplayX11.h"
#if USE_VAAPI_GLX
#include "VaapiDisplayGLX.h"
#endif
#include "vaapi_utils.h"

namespace gnash {

VaapiGlobalContext::VaapiGlobalContext(std::auto_ptr<VaapiDisplay> display)
    : _display(display)
{
    init();
}

VaapiGlobalContext::~VaapiGlobalContext()
{
}

bool
VaapiGlobalContext::init()
{
    VADisplay dpy = display();
    VAStatus status;

    int num_profiles = 0;
    if (vaMaxNumProfiles(dpy) == 0) {
        return false;
    }
    
    _profiles.resize(vaMaxNumProfiles(dpy));
    status = vaQueryConfigProfiles(dpy, &_profiles[0], &num_profiles);
    if (!vaapi_check_status(status, "vaQueryConfigProfiles()"))
	return false;
    _profiles.resize(num_profiles);

    int num_image_formats = 0;
    _image_formats.resize(vaMaxNumImageFormats(dpy));
    status = vaQueryImageFormats(dpy, &_image_formats[0], &num_image_formats);
    if (!vaapi_check_status(status, "vaQueryImageFormats()"))
	return false;
    _image_formats.resize(num_image_formats);
    return true;
}

bool
VaapiGlobalContext::hasProfile(VAProfile profile) const
{
    for (unsigned int i = 0; i < _profiles.size(); i++) {
	if (_profiles[i] == profile)
	    return true;
    }
    return false;
}

const VAImageFormat *
VaapiGlobalContext::getImageFormat(boost::uint32_t fourcc) const
{
    for (unsigned int i = 0; i < _image_formats.size(); i++) {
	if (_image_formats[i].fourcc == fourcc)
	    return &_image_formats[i];
    }
    return NULL;
}

/// A wrapper around a VaapiGlobalContext to ensure it's free'd on destruction.
VaapiGlobalContext *VaapiGlobalContext::get()
{
    static std::auto_ptr<VaapiGlobalContext> vaapi_global_context;

    if (!vaapi_global_context.get()) {
	std::auto_ptr<VaapiDisplay> dpy;
	/* XXX: this won't work with multiple renders built-in */
#if USE_VAAPI_GLX
	dpy.reset(new VaapiDisplayGLX());
#else
	dpy.reset(new VaapiDisplayX11());
#endif
	if (!dpy.get())
	    return NULL;
	vaapi_global_context.reset(new VaapiGlobalContext(dpy));
    }
    return vaapi_global_context.get();
}

} // gnash namespace
