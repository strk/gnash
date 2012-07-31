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
#include "GnashDevice.h"
#include "X11Device.h"

#include <X11/X.h>

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

    if (x11.initDevice(argc, argv)) {
        runtest.pass("X11Device:InitDevice()");
    } else {
        runtest.fail("X11Device:InitDevice()");
    }

    string result = "BadDrawable (invalid Pixmap or Window parameter)";
    int code = BadDrawable;
    if (x11.getErrorString(code) == result) {
        runtest.pass("X11Device::getErrorString()");
    } else {
        runtest.fail("X11Device::getErrorString()");
    }    

    if (x11.getDepth() > 0) {
        runtest.pass("X11Device::getDepth()");
    } else {
        runtest.fail("X11Device::getDepth()");
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
    
    if (x11.isSingleBuffered() >= 0) {
        runtest.pass("X11Device::isSingleBuffered()");
    } else {
        runtest.fail("X11Device::isSingleBuffered()");
    }

    // Context accessor tests
    // std::cerr << "FIXME: " << x11.getContextID() << std::endl;

    if (x11.getID() > 0) {
        runtest.pass("X11Device::getID()");
    } else {
        runtest.fail("X11Device::getID()");
    }    

#if 0
    if (x11.getRedSize() > 0) {
        runtest.pass("X11Device::getRedSize()");
    } else {
        runtest.fail("X11Device::getRedSize()");
    }    

    if (x11.getGreenSize() > 0) {
        runtest.pass("X11Device::getGreenSize()");
    } else {
        runtest.fail("X11Device::getGreenSize()");
    }    
    
    if (x11.getBlueSize() > 0) {
        runtest.pass("X11Device::getBlueSize()");
    } else {
        runtest.fail("X11Device::getBlueSize()");
    }    
#endif
}

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
