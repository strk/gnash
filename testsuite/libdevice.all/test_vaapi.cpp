// 
//   Copyright (C) 2010, 2011, 2012 Free Software Foundation, Inc
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <map>
#include <cassert>
#include <regex.h>
#include <boost/assign/list_of.hpp>

#include "log.h"
#include "dejagnu.h"
#include "VaapiDevice.h"

TestState runtest;

using namespace gnash;
using namespace std;
using namespace renderer;

// The debug log used by all the gnash libraries.
static LogFile& dbglogfile = LogFile::getDefaultInstance();

int
main(int argc, char *argv[])
{
    // FIXME: for now, always run verbose till this supports command line args
    dbglogfile.setVerbosity();

    vaapi::VaapiDevice vfb;

#if 0
    string result = "The buffer is empty";
    VFBResult code = DR_BUFFEREMPTY;
    if (vfb.getErrorString(code) == result) {
        runtest.pass("VaapiDevice::getErrorString()");
    } else {
        runtest.fail("VaapiDevice::getErrorString()");
    }    
#endif

    if (vfb.initDevice(argc, argv)) {
        runtest.pass("VaapiDevice:InitDevice()");
    } else {
        runtest.fail("VaapiDevice:InitDevice()");
    }

    if (vfb.getWidth()) {
        runtest.pass("VaapiDevice::getWidth()");
    } else {
        runtest.fail("VaapiDevice::getWidth()");
    }
    
    if (vfb.getHeigth()) {
        runtest.pass("VaapiDevice::getHeigth()");
    } else {
        runtest.fail("VaapiDevice::getHeigth()");
    }
    
    if (vfb.getVerticalRes()) {
        runtest.pass("VaapiDevice::getVerticalRes()");
    } else {
        runtest.fail("VaapiDevice::getVerticalRes()");
    }
    
    if (vfb.getHorzRes()) {
        runtest.pass("VaapiDevice::getHorzRes()");
    } else {
        runtest.fail("VaapiDevice::getHorzRes()");
    }    
    
    if (vfb.isSurfaceSingleBuffered() != vfb.isSurfaceBackBuffered()) {
        runtest.pass("VaapiDevice::isSurface*Buffered()");
    } else {
        runtest.fail("VaapiDevice::isSurface*Buffered()");
    }

    if (vfb.isContextSingleBuffered() != vfb.isContextBackBuffered()) {
        runtest.pass("VaapiDevice::iisContextBuffered()");
    } else {
        runtest.fail("VaapiDevice::isContextBuffered()");
    }

    // Context accessor tests
    if (vfb.getContextID() >= 0) {
        runtest.pass("VaapiDevice::getContextID()");
    } else {
        runtest.fail("VaapiDevice::getContextID()");
    }    

    if (vfb.getSurfaceID() >= 0) {
        runtest.pass("VaapiDevice::getSurfaceID()");
    } else {
        runtest.fail("VaapiDevice::getSurfaceID()");
    }

    if (vfb.getDepth()) {
        runtest.pass("VaapiDevice::getDepth()");
    } else {
        runtest.fail("VaapiDevice::getDepth()");
    }    
    
}

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
