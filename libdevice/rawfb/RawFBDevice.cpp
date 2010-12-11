//
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
#include "GnashException.h"

#include "RawFBDevice.h"
#include "GnashDevice.h"

namespace gnash {

namespace renderer {

namespace rawfb {
    
// The debug log used by all the gnash libraries.
static LogFile& dbglogfile = LogFile::getDefaultInstance();


RawFBDevice::RawFBDevice()
{
    GNASH_REPORT_FUNCTION;
}

RawFBDevice::RawFBDevice(int vid)
{
    GNASH_REPORT_FUNCTION;

    if (!initDevice(0, 0)) {
        log_error("Couldn't initialize RAWFB device!");
    }
}

RawFBDevice::RawFBDevice(int argc, char *argv[])
{
    GNASH_REPORT_FUNCTION;
    
}

RawFBDevice::~RawFBDevice()
{
    GNASH_REPORT_FUNCTION;
}

bool
RawFBDevice::initDevice(int argc, char *argv[])
{
    GNASH_REPORT_FUNCTION;
}

// Initialize RAWFB Window layer
bool
RawFBDevice::attachWindow(GnashDevice::native_window_t window)
{
    GNASH_REPORT_FUNCTION;
}
    

// Return a string with the error code as text, instead of a numeric value
const char *
RawFBDevice::getErrorString(int error)
{
}

// Create an RAWFB window to render in. This is only used by testing
void
RawFBDevice::createWindow(const char *name, int x, int y, int width, int height)
{
    GNASH_REPORT_FUNCTION;
}

void
RawFBDevice::eventLoop(size_t passes)
{
}

} // namespace rawfb
} // namespace renderer
} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
