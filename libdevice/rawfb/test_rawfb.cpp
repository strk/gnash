// 
//   Copyright (C) 2010 Free Software Foundation, Inc
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
#include "GnashDevice.h"
#include "RawFBDevice.h"

TestState runtest;

using namespace gnash;
using namespace std;
using namespace device;

// The debug log used by all the gnash libraries.
static LogFile& dbglogfile = LogFile::getDefaultInstance();

int
main(int argc, char *argv[])
{
    // FIXME: for now, always run verbose till this supports command line args
    dbglogfile.setVerbosity();

    rawfb::RawFBDevice rfb;

    bool ret = rfb.initDevice(argc, argv);
    if (ret <= 0) {
        exit(0);
    }
    
    if (ret) {
        runtest.pass("RawFBDevice:InitDevice()");
    } else {
        runtest.fail("RawFBDevice:InitDevice()");
    }

    if (rfb.getWidth()) {
        runtest.pass("RawFBDevice::getWidth()");
    } else {
        runtest.fail("RawFBDevice::getWidth()");
    }
    
    if (rfb.getHeight()) {
        runtest.pass("RawFBDevice::getHeight()");
    } else {
        runtest.fail("DirecTFBDevice::getHeight()");
    }

#if 0
    if (rfb.getVerticalRes()) {
        runtest.pass("RawFBDevice::getVerticalRes()");
    } else {
        runtest.fail("RawFBDevice::getVerticalRes()");
    }
    
    if (rfb.getHorzRes()) {
        runtest.pass("RawFBDevice::getHorzRes()");
    } else {
        runtest.fail("RawFBDevice::getHorzRes()");
    }    
#endif
    
    if (rfb.isSingleBuffered()) {
        runtest.pass("RawFBDevice::is*Buffered()");
    } else {
        runtest.fail("RawFBDevice::is*Buffered()");
    }

#if 0
    if (rfb.isContextSingleBuffered() != rfb.isContextBackBuffered()) {
        runtest.pass("RawFBDevice::iisContextBuffered()");
    } else {
        runtest.fail("RawFBDevice::isContextBuffered()");
    }

    // Context accessor tests
    std::cerr << "FIXME: " << rfb.getContextID() << std::endl;

    if (rfb.getContextID() >= 0) {
        runtest.pass("RawFBDevice::getContextID()");
    } else {
        runtest.fail("RawFBDevice::getContextID()");
    }    

    if (rfb.getSurfaceID() >= 0) {
        runtest.pass("RawFBDevice::getSurfaceID()");
    } else {
        runtest.fail("RawFBDevice::getSurfaceID()");
    }
#endif
    
    if (rfb.getDepth()) {
        runtest.pass("RawFBDevice::getDepth()");
    } else {
        runtest.fail("RawFBDevice::getDepth()");
    }
}

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
