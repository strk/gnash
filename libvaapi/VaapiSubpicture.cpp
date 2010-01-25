// VaapiSubpicture.cpp: VA subpicture abstraction
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

#include "VaapiSubpicture.h"
#include "VaapiGlobalContext.h"
#include "VaapiException.h"
#include "VaapiImage.h"
#include "vaapi_utils.h"
#include <boost/format.hpp>

#define DEBUG 0
#include "vaapi_debug.h"

namespace gnash {

VaapiSubpicture::VaapiSubpicture(boost::shared_ptr<VaapiImage> image)
    : _image(image)
    , _subpicture(VA_INVALID_ID)
{
    D(bug("VaapiSubpicture::VaapiSubpicture(): format '%s'\n", string_of_FOURCC(image->format())));

    if (!create()) {
        boost::format msg;
        msg = boost::format("Could not create %s subpicture")
            % string_of_FOURCC(image->format());
        throw VaapiException(msg.str());
    }
}

VaapiSubpicture::~VaapiSubpicture()
{
    D(bug("VaapiSubpicture::~VaapiSubpicture()\n"));

    destroy();
}

bool VaapiSubpicture::create()
{
    D(bug("VaapiSubpicture::create()\n"));

    if (!_image.get())
	return false;

    VaapiGlobalContext * const gvactx = VaapiGlobalContext::get();
    if (!gvactx)
        return false;

    VASubpictureID subpicture;
    VAStatus status = vaCreateSubpicture(gvactx->display(), _image->get(), &subpicture);
    if (!vaapi_check_status(status, "vaCreateSubpicture()"))
	return false;

    _subpicture = subpicture;
    return true;
}

void VaapiSubpicture::destroy()
{
    VaapiGlobalContext * const gvactx = VaapiGlobalContext::get();
    if (!gvactx)
        return;

    if (_subpicture != VA_INVALID_ID) {
	VAStatus status = vaDestroySubpicture(gvactx->display(), _subpicture);
	if (!vaapi_check_status(status, "vaDestroySubpicture()"))
	    return;
	_subpicture = VA_INVALID_ID;
    }
}

}
