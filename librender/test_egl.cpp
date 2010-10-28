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

#ifdef HAVE_EGL_EGL_H
# include <EGL/egl.h>
#else
# error "This file needs EGL, which is part of OpenGL-ES"
#endif

#include "log.h"
#include "dejagnu.h"
#include "eglDevice.h"

TestState runtest;

using namespace gnash;
using namespace std;
using namespace renderer;

void test_egl(EGLDevice &egl, EGLDevice::rtype_t);

// The debug log used by all the gnash libraries.
static LogFile& dbglogfile = LogFile::getDefaultInstance();

const char *estrs[] = {
    "OpenVG",
    "OpenGLES1",
    "OpenGLES2"
};
    
int
main(int argc, char *argv[])
{
    // FIXME: for now, always run verbose till this supports command line args
    dbglogfile.setVerbosity();
    
    EGLDevice egl1, egl2, egl3;

#ifdef RENDERER_OPENVG
    test_egl(egl1, EGLDevice::OPENVG);
    egl1.printEGLConfig();
#endif
    
#ifdef RENDERER_GLES1
    test_egl(egl2, EGLDevice::OPENGLES1);
//    egl2.printEGLConfig();
#endif
    
#ifdef RENDERER_GLES2
    test_egl(egl3, EGLDevice::OPENGLES2);
//    egl3.printEGLConfig();
#endif
}

void
test_egl(EGLDevice &egl, EGLDevice::rtype_t rtype)
{
    cout << "Testing " << estrs[rtype] << endl;

    // There isn't a whole lot to test, if init works, most
    // everything else has to be correct.
    if (egl.initDevice(rtype)) {
        runtest.pass("EGLDevice::init()");
    } else {
        runtest.fail("EGLDevice::init()");
    }

    // Init'ing to zero uses the root screen as the display. Otherwise
    // the argument should be an EGLNativeWindowType.
    if (egl.initEGL(0)) {
        runtest.pass("EGLDevice::initEGL(0)");
    } else {
        runtest.fail("EGLDevice::initEGL(0)");
    }

    // If there are more than zero configurations, something beyond
    // initializing is working
    if (egl.queryEGLConfig()) {
        runtest.pass("EGLDevice::queryEGLConfig()");
    } else {
        runtest.fail("EGLDevice::queryEGLConfig()");
    }

    // This is a utility method for converting integer error codes to
    // something human readable for debugging.
    string result = "EGL_BAD_CONFIG";
    if (egl.getErrorString(EGL_BAD_CONFIG) == result) {
        runtest.pass("EGLDevice::getErrorString()");
    } else {
        runtest.fail("EGLDevice::getErrorString()");
    }
    
}

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
