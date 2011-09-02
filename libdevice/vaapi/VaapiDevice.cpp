//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <iostream>
#include <cerrno>
#include <exception>
#include <sstream>

#include "log.h"
#include "Renderer.h"
#include "GnashException.h"

#include "VaapiDevice.h"

namespace gnash {

namespace renderer {

namespace vaapi {
    
// The debug log used by all the gnash libraries.
static LogFile&amp; dbglogfile = LogFile::getDefaultInstance();

// FIXME: this font name shouldn't be hardcoded!
const char *FONT = "/usr/share/fonts/truetype/freefont/FreeSerif.ttf";

VaapiDevice::VaapiDevice()
{
    GNASH_REPORT_FUNCTION;
}

VaapiDevice::~VaapiDevice()
{
    // GNASH_REPORT_FUNCTION;
}

bool
VaapiDevice::initDevice(int argc, char *argv[])
{
    GNASH_REPORT_FUNCTION;

    return true;
}

} // namespace vaapi
} // namespace renderer
} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
