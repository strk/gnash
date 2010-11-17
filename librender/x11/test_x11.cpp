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
#include "X11Device.h"

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

    x11::X11Device x11;

#if 0
    string result = "The buffer is empty";
    X11Result code = DR_BUFFEREMPTY;
    if (x11.getErrorString(code) == result) {
        runtest.pass("X11Device::getErrorString()");
    } else {
        runtest.fail("X11Device::getErrorString()");
    }    
#endif

    if (x11.initDevice(argc, argv)) {
        runtest.pass("X11Device:InitDevice()");
    } else {
        runtest.fail("X11Device:InitDevice()");
    }

    if (x11.getWidth()) {
        runtest.pass("X11Device::getWidth()");
    } else {
        runtest.fail("X11Device::getWidth()");
    }
    
    if (x11.getHeight()) {
        runtest.pass("X11Device::getHeight()");
    } else {
        runtest.fail("X11Device::getHeight()");
    }
    
    if (x11.getVerticalRes()) {
        runtest.pass("X11Device::getVerticalRes()");
    } else {
        runtest.fail("X11Device::getVerticalRes()");
    }
    
    if (x11.getHorzRes()) {
        runtest.pass("X11Device::getHorzRes()");
    } else {
        runtest.fail("X11Device::getHorzRes()");
    }    
    
    if (x11.isSurfaceSingleBuffered() != x11.isSurfaceBackBuffered()) {
        runtest.pass("X11Device::isSurface*Buffered()");
    } else {
        runtest.fail("X11Device::isSurface*Buffered()");
    }

    if (x11.isContextSingleBuffered() != x11.isContextBackBuffered()) {
        runtest.pass("X11Device::iisContextBuffered()");
    } else {
        runtest.fail("X11Device::isContextBuffered()");
    }

    // Context accessor tests
    // std::cerr << "FIXME: " << x11.getContextID() << std::endl;

    if (x11.getContextID() > 0) {
        runtest.pass("X11Device::getContextID()");
    } else {
        runtest.fail("X11Device::getContextID()");
    }    

    if (x11.getSurfaceID() > 0) {
        runtest.pass("X11Device::getSurfaceID()");
    } else {
        runtest.fail("X11Device::getSurfaceID()");
    }

    if (x11.getDepth() > 0) {
        runtest.pass("X11Device::getDepth()");
    } else {
        runtest.fail("X11Device::getDepth()");
    }    

}

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
