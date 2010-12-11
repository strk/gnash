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
using namespace renderer;

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
    
    if ((ret) && (rfb.getFBMemory() > 0) && (rfb.getFBMemSize() > 0)) {
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

    if (rfb.isSingleBuffered()) {
        runtest.pass("RawFBDevice::is*Buffered()");
    } else {
        runtest.fail("RawFBDevice::is*Buffered()");
    }
    
    if (rfb.getDepth()) {
        runtest.pass("RawFBDevice::getDepth()");
    } else {
        runtest.fail("RawFBDevice::getDepth()");
    }
    
    if (rfb.getRedSize() > 0) {
        runtest.pass("RawFBDevice::getRedSize()");
    } else {
        runtest.fail("RawFBDevice::getRedSize()");
    }

    if (rfb.getGreenSize() > 0) {
        runtest.pass("RawFBDevice::getGreenSize()");
    } else {
        runtest.fail("RawFBDevice::getGreenSize()");
    }

    if (rfb.getBlueSize() > 0) {
        runtest.pass("RawFBDevice::getBlueSize()");
    } else {
        runtest.fail("RawFBDevice::getBlueSize()");
    }

    if (rfb.setGrayscaleLUT8()) {
        runtest.pass("RawFBDevice::setGrayscaleLUT8()");
    } else {
        runtest.fail("RawFBDevice::setGrayscaleLUT8()");
    }
        
    // AGG uses these to calculate the poixel format
#ifdef RENDERER_AGG
    if (rfb.getRedOffset() > 0) {
        runtest.pass("RawFBDevice::getRedOffset()");
    } else {
        runtest.fail("RawFBDevice::getRedOffset()");
    }
    
    if (rfb.getGreenOffset() > 0) {
        runtest.pass("RawFBDevice::getGreenOffset()");
    } else {
        runtest.fail("RawFBDevice::getGreenOffset()");
    }
    
    if (rfb.getBlueOffset() == 0) {
        runtest.pass("RawFBDevice::getBlueOffset()");
    } else {
        runtest.fail("RawFBDevice::getBlueOffset()");
    }
#endif
}

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
