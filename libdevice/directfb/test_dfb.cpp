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
#include "DirectFBDevice.h"

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

    directfb::DirectFBDevice dfb;

#if 0
    string result = "The buffer is empty";
    DFBResult code = DR_BUFFEREMPTY;
    if (dfb.getErrorString(code) == result) {
        runtest.pass("DirectFBDevice::getErrorString()");
    } else {
        runtest.fail("DirectFBDevice::getErrorString()");
    }    
#endif

    bool ret = dfb.initDevice(argc, argv);
    if (ret <= 0) {
        exit(0);
    }
    
    if (ret) {
        runtest.pass("DirectFBDevice:InitDevice()");
    } else {
        runtest.fail("DirectFBDevice:InitDevice()");
    }

    if (dfb.getWidth()) {
        runtest.pass("DirectFBDevice::getWidth()");
    } else {
        runtest.fail("DirectFBDevice::getWidth()");
    }
    
    if (dfb.getHeight()) {
        runtest.pass("DirectFBDevice::getHeight()");
    } else {
        runtest.fail("DirecTFBDevice::getHeight()");
    }
    
    if (dfb.getVerticalRes()) {
        runtest.pass("DirectFBDevice::getVerticalRes()");
    } else {
        runtest.fail("DirectFBDevice::getVerticalRes()");
    }
    
    if (dfb.getHorzRes()) {
        runtest.pass("DirectFBDevice::getHorzRes()");
    } else {
        runtest.fail("DirectFBDevice::getHorzRes()");
    }    

    // DirectFB is double buffered by default
    if (dfb.isSingleBuffered() == false) {
        runtest.pass("DirectFBDevice::is*Buffered()");
    } else {
        runtest.fail("DirectFBDevice::is*Buffered()");
    }

#if 0
    if (dfb.isContextSingleBuffered() != dfb.isContextBackBuffered()) {
        runtest.pass("DirectFBDevice::isContextBuffered()");
    } else {
        runtest.fail("DirectFBDevice::isContextBuffered()");
    }
#endif
    
    // Context accessor tests
    if (dfb.getContextID() >= 0) {
        runtest.pass("DirectFBDevice::getContextID()");
    } else {
        runtest.fail("DirectFBDevice::getContextID()");
    }    

    if (dfb.getSurfaceID() >= 0) {
        runtest.pass("DirectFBDevice::getSurfaceID()");
    } else {
        runtest.fail("DirectFBDevice::getSurfaceID()");
    }

    if (dfb.getDepth()) {
        runtest.pass("DirectFBDevice::getDepth()");
    } else {
        runtest.fail("DirectFBDevice::getDepth()");
    }    
    
#if 0
    dfb.printDirectFB();
    dfb.printFBSurface();
    dfb.printFBFont();
    dfb.printFBDisplay();
    dfb.printFBScreen();

    std::cerr << "----------------------" << std::endl;
#endif

}

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
