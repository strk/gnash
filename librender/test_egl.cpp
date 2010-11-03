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
    egl1.printEGLContext();
    egl1.printEGLSurface();
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
    
    // Surface config info tests
    if (egl.getSurfaceID()) {
        runtest.pass("EGLDevice::getSurfaceID()");
    } else {
        runtest.fail("EGLDevice::getSurfaceID()");
    }
    
    if (egl.getWidth()) {
        runtest.pass("EGLDevice::getWidth()");
    } else {
        runtest.fail("EGLDevice::getWidth()");
    }
    
    if (egl.getHeigth()) {
        runtest.pass("EGLDevice::getHeigth()");
    } else {
        runtest.fail("EGLDevice::getHeigth()");
    }
    
    if (egl.getVerticalRes()) {
        runtest.pass("EGLDevice::getVerticalRes()");
    } else {
        runtest.fail("EGLDevice::getVerticalRes()");
    }
    
    if (egl.getHorzRes()) {
        runtest.pass("EGLDevice::getHorzRes()");
    } else {
        runtest.fail("EGLDevice::getHorzRes()");
    }
    
    if (egl.supportsRenderer(rtype)) {
        runtest.pass("EGLDevice::supportsRenderer()");
    } else {
        runtest.fail("EGLDevice::supportsRenderer()");
    }
    
    if (egl.isSurfaceSingleBuffered() != egl.isSurfaceBackBuffered()) {
        runtest.pass("EGLDevice::isSurface*Buffered()");
    } else {
        runtest.fail("EGLDevice::isSurface*Buffered()");
    }

    if (egl.isBufferDestroyed()) {
        runtest.pass("EGLDevice::isBufferDestroyed()");
    } else {
        runtest.fail("EGLDevice::isBufferDestroyed()");
    }
    
    if (!egl.isMultiSample()) {
        runtest.pass("EGLDevice::isMultiSample()");
    } else {
        runtest.fail("EGLDevice::isMultiSample()");
    }

    // Context accessor tests
    if (egl.getContextID()) {
        runtest.pass("EGLDevice::getContextID()");
    } else {
        runtest.fail("EGLDevice::getContextID()");
    }
    
    if (egl.supportsClient(rtype)) {
        runtest.pass("EGLDevice::supportsClient()");
    } else {
        runtest.fail("EGLDevice::supportsClient()");
    }
    
    if (egl.isContextSingleBuffered() != egl.isContextBackBuffered()) {
        runtest.pass("EGLDevice::isContext*Buffered()");
    } else {
        runtest.fail("EGLDevice::isContext*Buffered()");
    }
    
    if (egl.isNativeRender()) {
        runtest.pass("EGLDevice::isNativeRender()");
    } else {
        runtest.fail("EGLDevice::isNativeRender()");
    }

    if (egl.getSamples() == 0) {
        runtest.pass("EGLDevice::getSamples()");
    } else {
        runtest.fail("EGLDevice::getSamples()");
    }
    
    if (egl.getSampleBuffers() == 0) {
        runtest.pass("EGLDevice::getSampleBuffers()");
    } else {
        runtest.fail("EGLDevice::getSampleBuffers()");
    }
    
    if (egl.getDepth()) {
        runtest.pass("EGLDevice::getDepth()");
    } else {
        runtest.fail("EGLDevice::getDepth()");
    }
    
    if (egl.getMaxSwapInterval() == 0) {
        runtest.pass("EGLDevice::getMaxSwapInterval()");
    } else {
        runtest.fail("EGLDevice::getMaxSwapInterval()");
    }
    
    if (egl.getMinSwapInterval() == 0) {
        runtest.pass("EGLDevice::getMinSwapInterval()");
    } else {
        runtest.fail("EGLDevice::getMinSwapInterval()");
    }

    // Test Pbuffers
    EGLSurface surf = egl.createPbuffer(200, 200);
    if ((surf != EGL_NO_SURFACE) && (egl.getWidth(surf) == 200)) {
        runtest.pass("EGLDevice::createPbuffer(int, int)");
    } else {
        runtest.fail("EGLDevice::createPbuffer(int, int)");
    }
    
    EGLSurface surf1 = egl[0];
    if ((surf1 != EGL_NO_SURFACE) && (egl.getWidth(surf1) == 200)
        && (egl.getHeigth(surf1) == 200)) {
        runtest.pass("EGLDevice::operator[]()");
    } else {
        runtest.fail("EGLDevice::operator[]()");
    }

    EGLSurface surf2 = egl.createPbuffer(300, 300);
    EGLSurface surf3 = egl.createPbuffer(400, 400);

    if (egl.totalPbuffers() == 3) {
        runtest.pass("EGLDevice::totalPbuffers(2)");
    } else {
        runtest.fail("EGLDevice::totalPbuffers(2)");
    }

    // Since we're EGL_SINGLE_BUFFER'd, this is a nop
    if (egl.swapPbuffers()) {
        runtest.pass("EGLDevice::swapPbuffers()");
    } else {
        runtest.fail("EGLDevice::swapPbuffers()");
    }

    egl.makePbufferCurrent(1);
    EGLSurface surf4 = eglGetCurrentSurface(EGL_DRAW);
    if ((egl.getWidth(surf4) == 300) && ((egl.getHeigth(surf4) == 300))) {
        runtest.pass("EGLDevice::makePbufferCurrent(int)");
    } else {
        runtest.fail("EGLDevice::makePbufferCurrent(int)");
    }

    // This should trigger an error as the number is more than we
    // have created
    if (!egl.makePbufferCurrent(10)) {
        runtest.pass("EGLDevice::makePbufferCurrent(maxed)");
    } else {
        runtest.fail("EGLDevice::makePbufferCurrent(maxed)");
    }
    
#if 0
    if (!egl.copyPbuffers(1)) {
        runtest.pass("EGLDevice::copyPbuffers()");
    } else {
        runtest.fail("EGLDevice::copyPbuffers()");
    }
#endif
    
    // EGLSurface surf5 = eglGetCurrentSurface(EGL_DRAW);
    // egl.printEGLSurface(surf5);
}

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
