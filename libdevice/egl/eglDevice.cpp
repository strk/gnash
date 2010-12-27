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

#include "log.h"
// #include "RunResources.h"
#include "Renderer.h"
#include "GnashException.h"

#ifdef HAVE_EGL_EGL_H
# include <EGL/egl.h>
#else
# error "This file needs EGL, which is part of OpenGL-ES"
#endif

//#include <GL/gl.h>

#include "eglDevice.h"

namespace gnash {

namespace renderer {

static const EGLint attrib32_list[] = {
    EGL_RED_SIZE,       8,
    EGL_GREEN_SIZE,     8,
    EGL_BLUE_SIZE,      8,
//  EGL_ALPHA_SIZE,     0,
//    EGL_DEPTH_SIZE,     24,
// #ifdef RENDERER_GLES1
//     EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
// #endif
// #ifdef RENDERER_GLES2
//     EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
// #endif
#ifdef RENDERER_OPENVG
     EGL_RENDERABLE_TYPE, EGL_OPENVG_BIT,
//     EGL_STENCIL_SIZE,   8,
#endif
//    EGL_RENDERABLE_TYPE, EGL_OPENVG_BIT|EGL_OPENGL_ES_BIT|EGL_OPENGL_ES2_BIT,
    EGL_SURFACE_TYPE,   EGL_WINDOW_BIT|EGL_PBUFFER_BIT|EGL_PIXMAP_BIT,
//    EGL_SAMPLE_BUFFERS, 1,
// FIXME: Single Buffering appears not to work on X11, you get no visual. This is
// the default though.    
//    EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER,
    EGL_NONE
};

static EGLint const attrib16_list[] = {
    EGL_RED_SIZE,       5,
    EGL_GREEN_SIZE,     6,
    EGL_BLUE_SIZE,      5,
    EGL_ALPHA_SIZE,     0,
// #ifdef RENDERER_GLES1
//     EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
// #endif
// #ifdef RENDERER_GLES2
//     EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
// #endif
    EGL_LUMINANCE_SIZE,     EGL_DONT_CARE,
    EGL_SURFACE_TYPE,       EGL_VG_COLORSPACE_LINEAR_BIT,
    EGL_SAMPLES,            0,
#ifdef RENDERER_OPENVG
    EGL_RENDERABLE_TYPE, EGL_WINDOW_BIT|EGL_PBUFFER_BIT|EGL_PIXMAP_BIT,
    EGL_DEPTH_SIZE,     16,
#endif
    EGL_NONE
};

// These are the same EGL config settings as used by the Mesa
// examples, which run on X11
static const EGLint attrib1_list[] = {
    EGL_RED_SIZE, 1,
    EGL_GREEN_SIZE, 1,
    EGL_BLUE_SIZE, 1,
    EGL_RENDERABLE_TYPE, EGL_OPENVG_BIT,
    EGL_NONE
};

const EGLint window_attrib_list[] = {
    // Back buffering is used for window and pbuffer surfaces. Windows
    // require eglSwapBuffers() to become visible, and pbuffers don't.   
    // EGL_SINGLE_BUFFER is by pixmap surfaces. With OpenVG, windows
    // can also be single buffered. eglCopyBuffers() can be used to copy
    // both back and single buffered surfaces to a pixmap.
    EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER,
    EGL_COLORSPACE,    EGL_COLORSPACE_sRGB,
    EGL_NONE
};

// From the EGL 1.4 spec:
//
// EGL defines several types of drawing surfaces collectively referred
// to as EGLSurfaces. These include windows, used for onscreen
// rendering; pbuffers, used for offscreen rendering; and pixmaps,
// used for offscreen rendering into buffers that may be accessed
// through native APIs. EGL windows and pixmaps are tied to native
// window system windows and pixmaps.
//
// depth, multisample, and stencil buffers are currently used only by
// OpenGL-ES.

// EGL and OpenGL ES supports two rendering models: back buffered and
// single buffered. Back buffered rendering is used by window and
// pbuffer surfaces. Memory for the color buffer used during rendering
// is allocated and owned by EGL. When the client is finished drawing
// a frame, the back buffer may be copied to a visible window using
// eglSwapBuffers. Pbuffer surfaces have a back buffer but no
// associated window, so the back buffer need not be copied.
//
// Single buffered rendering is used by pixmap surfaces. Memory for
// the color buffer is specified at surface creation time in the form
// of a native pixmap, and client APIs are required to use that memory
// during rendering. When the client is finished drawing a frame, the
// native pixmap contains the final image. Pixmap surfaces typically
// do not support multisampling, since the native pixmap used as the
// color buffer is unlikely to provide space to store multisample
// information. Some client APIs , such as OpenGL and OpenVG , also
// support single buffered rendering to window surfaces. This behavior
// can be selected when creating the window surface, as defined in
// section 3.5.1. When mixing use of client APIs which do not support
// single buffered rendering into windows, like OpenGL ES , with
// client APIs which do support it, back color buffers and visible
// window contents must be kept consistent when binding window
// surfaces to contexts for each API type. Both back and single
// buffered surfaces may also be copied to a specified native pixmap
// using eglCopyBuffers.

// Native rendering will always be supported by pixmap surfaces (to
// the extent that native rendering APIs can draw to native
// pixmaps). Pixmap surfaces are typically used when mixing native and
// client API rendering is desirable, since there is no need to move
// data between the back buffer visible to the client APIs and the
// native pixmap visible to native rendering APIs. However, pixmap
// surfaces may, for the same reason, have restricted capabilities and
// performance relative to window and pbuffer surfaces.

// The debug log used by all the gnash libraries.
static LogFile& dbglogfile = LogFile::getDefaultInstance();

EGLDevice::EGLDevice()
    : _eglConfig(0),
      _eglContext(EGL_NO_CONTEXT),
      _eglSurface(EGL_NO_SURFACE),
      _eglDisplay(EGL_NO_DISPLAY),
      _eglNumOfConfigs(0),
      _max_num_config(1),
#if BUILD_X11_DEVICE
      _bpp(32)
#else
      _bpp(16)
#endif
{
    GNASH_REPORT_FUNCTION;
    
    setAttrib(_bpp);
}

EGLDevice::EGLDevice(int argc, char *argv[])
    :  _attrib(0),
       _eglConfig(0),
      _eglContext(EGL_NO_CONTEXT),
      _eglSurface(EGL_NO_SURFACE),
      _eglDisplay(EGL_NO_DISPLAY),
      _eglNumOfConfigs(0),
      _max_num_config(1),
#if BUILD_X11_DEVICE
      _bpp(32)
#else
      _bpp(16)
#endif
{
    GNASH_REPORT_FUNCTION;

    setAttrib(_bpp);

    if (!initDevice(argc, argv)) {
        log_error("Couldn't initialize EGL device!");
    }
}

EGLDevice::EGLDevice(GnashDevice::rtype_t rtype)
    :  _attrib(0),
       _eglConfig(0),
      _eglContext(EGL_NO_CONTEXT),
      _eglSurface(EGL_NO_SURFACE),
      _eglDisplay(EGL_NO_DISPLAY),
      _eglNumOfConfigs(0),
      _max_num_config(1),
#if BUILD_X11_DEVICE
      _bpp(32)
#else
      _bpp(16)
#endif
{
    GNASH_REPORT_FUNCTION;
    
    setAttrib(_bpp);

    if (!initDevice(0, 0)) {
        log_error("Couldn't initialize EGL device!");
    }
    if (!bindClient(rtype)) {
        log_error("Couldn't bind client to type %d!", rtype);
    }
}

void
EGLDevice::setAttrib(int bpp)
{ 
    switch (bpp) {
    case 32:
        _attrib = attrib32_list;
        break;
    case 16:
        _attrib = attrib16_list;
        break;
    case 1:
        _attrib = attrib1_list;
        break;
    }
}

EGLDevice::~EGLDevice()
{
    GNASH_REPORT_FUNCTION;

    if (_eglDisplay != EGL_NO_DISPLAY) {  
        eglMakeCurrent(_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        std::vector<EGLSurface>::iterator it;
        for (it = _pbuffers.begin(); it != _pbuffers.end(); ++it) {
            eglDestroySurface(_eglDisplay, *it);
        }
        
        if (_eglContext != EGL_NO_CONTEXT)
            eglDestroyContext(_eglDisplay, _eglContext);
        
        if (_eglSurface != EGL_NO_SURFACE)
            eglDestroySurface(_eglDisplay, _eglSurface);
        
        eglTerminate(_eglDisplay);
    }
}

/// @note: There are a few steps required to initialize an EGL
/// Device. This uses threee methods to do so, two are defaults from
/// the base class, one is an additional class that is EGL specific.
///
/// To start, initialize the device with the command line
/// arguments. These are ignored by EGL, but passed through here to
/// follow the way most other Devices need to be initialized.
///
/// Once initialized, EGL must be told which Client API to use, this
/// is either OpenVG, OpenGLES1, or OpenGLES2. To do this, we bind the
/// EGL device to the client API.
///
/// Once bound, the last step attaches the window surface of the
/// desktop or framebuffer to EGL. This is what binds EGL to the
/// desktop or framebuffer.
bool
EGLDevice::initDevice(int argc, char *argv[])
{
    EGLDevice::rtype_t rtype;

    dbglogfile.setVerbosity(2);

    GNASH_REPORT_FUNCTION;
    
    // see egl_config.c for a list of supported configs, this looks for
    // a 5650 (rgba) config, supporting OpenGL ES and windowed surfaces

    // step 1 - get an EGL display

//    _eglDisplay = eglGetDisplay(XOpenDisplay(0));
    _eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (EGL_NO_DISPLAY == _eglDisplay) {
        log_error( "eglGetDisplay() failed (error 0x%x)", eglGetError());
        return false;
    }

    // This can be called multiple times safely
    if (eglInitialize(_eglDisplay, 0, 0) != EGL_TRUE) {
        log_error( "eglInitialize() failed (error %s)",
                   getErrorString(eglGetError()));
        return false;
    }

    // step2 - bind to the wanted client API
    /// This is done by bindClient() later on
    // bindClient(GnashDevice::OPENVG);
    queryEGLConfig(_eglDisplay);
   
    log_debug("EGL_CLIENT_APIS = %s", eglQueryString(_eglDisplay, EGL_CLIENT_APIS));
    log_debug("EGL_EXTENSIONS = %s",  eglQueryString(_eglDisplay, EGL_EXTENSIONS));
    log_debug("EGL_VERSION = %s, EGL_VENDOR = %s",
              eglQueryString(_eglDisplay, EGL_VERSION),
              eglQueryString(_eglDisplay, EGL_VENDOR));

    // step3 - find a suitable config
#if 1
    printEGLAttribs(_attrib);
    if (EGL_FALSE == eglChooseConfig(_eglDisplay, _attrib, &_eglConfig,
                                     1, &_eglNumOfConfigs)) {
        log_error("eglChooseConfig(32) failed (error %s)", 
                  getErrorString(eglGetError()));
        return false;
    }
#else
    if (_bpp == 32) {
        printEGLAttribs(attrib32_list);
        if (EGL_FALSE == eglChooseConfig(_eglDisplay, attrib32_list, &_eglConfig,
                                          1, &_eglNumOfConfigs)) {
            log_error("eglChooseConfig(32) failed (error %s)", 
                       getErrorString(eglGetError()));
            return false;
        }
    } else if (_bpp == 16) {
        printEGLAttribs(attrib16_list);
        if (EGL_FALSE == eglChooseConfig(_eglDisplay, attrib16_list, &_eglConfig,
                                         1, &_eglNumOfConfigs)) {
            log_error("eglChooseConfig(16) failed (error %s)",
                       getErrorString(eglGetError()));
            return false;
        }
    } else {
        log_error("No supported bpp value!");
    }
#endif

    if (0 == _eglNumOfConfigs) {
        log_error("eglChooseConfig() was unable to find a suitable config");
        return false;
    }
    
   if (!checkEGLConfig(_eglConfig)) {
       log_error("EGL configuration doesn't match!");
       return false;
   } else {
       printEGLConfig(_eglConfig);
   }
    
    return true;
}

bool
EGLDevice::supportsRenderer(rtype_t rtype)
{
    GNASH_REPORT_FUNCTION;
    
    if (_eglDisplay && _eglContext) {
        EGLint value;
        eglQueryContext(_eglDisplay, _eglContext, EGL_CONTEXT_CLIENT_TYPE, &value);
        std::string str;
        if ((value == EGL_OPENGL_ES_API) && (rtype == EGLDevice::OPENGLES2)) {
            return true;
        } else if ((value == EGL_OPENGL_ES_API) && (rtype == EGLDevice::OPENGLES1)) {
            return true;
        } else if ((value == EGL_OPENVG_API) && (rtype == EGLDevice::OPENVG)){
            return true;
        }
    }
    return false;
}

EGLint
EGLDevice::getNativeVisual()
{
    EGLint vid;
    if (_eglDisplay && _eglConfig) {
        if (!eglGetConfigAttrib(_eglDisplay, _eglConfig, EGL_NATIVE_VISUAL_ID, &vid)) {
            log_error("eglGetConfigAttrib() failed (error %s)",
                      getErrorString(eglGetError()));
            return 0;
        } else {
            log_debug("EGL native visual is: %d", vid);
        }
    }

    return vid;
}

bool
EGLDevice::bindClient(rtype_t rtype)
{
    GNASH_REPORT_FUNCTION;
    
    EGLint value;

    switch (rtype) {
      case GnashDevice::OPENGLES2:
      {
          log_debug("Initializing EGL for OpenGLES2");
          if(EGL_FALSE == eglBindAPI(EGL_OPENGL_ES_API)) {
              log_error("eglBindAPI() failed to retrive the number of configs (error %s)",
                        getErrorString(eglGetError()));
              return false;
          }
          break;
      }
      case GnashDevice::OPENGLES1:
      {
          log_debug("Initializing EGL for OpenGLES1");
          if(EGL_FALSE == eglBindAPI(EGL_OPENGL_ES_API)) {
              log_error("eglBindAPI() failed to retrive the number of configs (error %s)",
                        getErrorString(eglGetError()));
              return false;
          }
          break;
      }
      case GnashDevice::OPENVG:
      {
          log_debug("Initializing EGL for OpenVG");
          if(EGL_FALSE == eglBindAPI(EGL_OPENVG_API)) {
              log_error("eglBindAPI() failed to retrive the number of configs (error %s)",
                        getErrorString(eglGetError()));
              return false;
          }
          break;
      }
      case GnashDevice::XORG:
      case GnashDevice::VAAPI:
      default:
          break;
    }
    return true;
}

bool
EGLDevice::attachWindow(GnashDevice::native_window_t window)
{
    GNASH_REPORT_FUNCTION;
    
    if (!window) {
        return false;
    } else {
        _nativeWindow = static_cast<EGLNativeWindowType>(window);
    }

    log_debug("Initializing EGL Surface");
    if (_eglDisplay && _eglConfig) {
        _eglSurface = eglCreateWindowSurface(_eglDisplay, _eglConfig,
                                             _nativeWindow, NULL);
    }
    
    if (EGL_NO_SURFACE == _eglSurface) {
        log_error("eglCreateWindowSurface failed (error %s)", 
                  getErrorString(eglGetError()));
    } else {
        printEGLSurface(_eglSurface);
    }
    
    // step5 - create a context
    _eglContext = eglCreateContext(_eglDisplay, _eglConfig, EGL_NO_CONTEXT, NULL);
    if (EGL_NO_CONTEXT == _eglContext) {
        log_error("eglCreateContext failed (error %s)",
                  getErrorString(eglGetError()));
    } else {
        printEGLContext(_eglContext);
    }
    
    // step6 - make the context and surface current
    if (EGL_FALSE == eglMakeCurrent(_eglDisplay, _eglSurface, _eglSurface, _eglContext)) {
        log_error("eglMakeCurrent failed (error %s)",
                  getErrorString(eglGetError()));
    }       // begin user code

    return true;
}   

const char *
EGLDevice::getErrorString(int error)
{
    switch (error) {
    case EGL_SUCCESS:
        return "EGL_SUCCESS";
    case EGL_NOT_INITIALIZED:
        return "EGL_NOT_INITIALIZED";
    case EGL_BAD_ACCESS:
        return "EGL_BAD_ACCESS";
    case EGL_BAD_ALLOC:
        return "EGL_BAD_ALLOC";
    case EGL_BAD_ATTRIBUTE:
        return "EGL_BAD_ATTRIBUTE";
    case EGL_BAD_CONFIG:
        return "EGL_BAD_CONFIG";
    case EGL_BAD_CONTEXT:
        return "EGL_BAD_CONTEXT";
    case EGL_BAD_CURRENT_SURFACE:
        return "EGL_BAD_CURRENT_SURFACE";
    case EGL_BAD_DISPLAY:
        return "EGL_BAD_DISPLAY";
    case EGL_BAD_MATCH:
        return "EGL_BAD_MATCH";
    case EGL_BAD_NATIVE_PIXMAP:
        return "EGL_BAD_NATIVE_PIXMAP";
    case EGL_BAD_NATIVE_WINDOW:
        return "EGL_BAD_NATIVE_WINDOW";
    case EGL_BAD_PARAMETER:
        return "EGL_BAD_PARAMETER";
    case EGL_BAD_SURFACE:
        return "EGL_BAD_SURFACE";
    case EGL_CONTEXT_LOST:
        return "EGL_CONTEXT_LOST";
    default:
        return "unknown error code";
    }
}

bool
EGLDevice::checkEGLConfig(EGLConfig config)
{
    // GNASH_REPORT_FUNCTION;
    
    // Use this to explicitly check that the EGL config has the expected color depths
    EGLint value;
    if (_bpp == 32) {            
        eglGetConfigAttrib(_eglDisplay, config, EGL_RED_SIZE, &value);
        if (8 != value) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_GREEN_SIZE, &value);
        if (8 != value) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_BLUE_SIZE, &value);
        if (8 != value) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_ALPHA_SIZE, &value);
        if (0 != value) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_SAMPLES, &value);
        if (0 != value) {
            return false;
        }
    } else if (_bpp == 16) {
        eglGetConfigAttrib(_eglDisplay, config, EGL_RED_SIZE, &value);
        if ( 5 != value ) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_GREEN_SIZE, &value);
        if (6 != value) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_BLUE_SIZE, &value);
        if (5 != value) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_ALPHA_SIZE, &value);
        if (0 != value) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_SAMPLES, &value);
#ifdef  RENDERER_GLES            
        if (4 != value) {
            return false;
        }
#endif
#ifdef  RENDERER_OPENVG
        if (0 != value) {
            return false;
            }
#endif
    } else {
        return false;
    }

    return true;
}

/// Query the system for all supported configs
int
EGLDevice::queryEGLConfig(EGLDisplay display)
{
     GNASH_REPORT_FUNCTION;
     EGLConfig *configs = 0;
     EGLint max_num_config = 0;

     // Get the number of supported configurations
     if ( EGL_FALSE == eglGetConfigs(display, 0, 0, &max_num_config) ) {
         log_error("eglGetConfigs() failed to retrive the number of configs (error %s)",
                   getErrorString(eglGetError()));
         return 0;
     }
     if(max_num_config <= 0) {
         printf( "No EGLconfigs found\n" );
         return 0;
     }
     log_debug("Max number of EGL Configs is %d", max_num_config);     
     
     configs = new EGLConfig[max_num_config];
     if (0 == configs) {
         log_error( "Out of memory\n" );
         return 0;
     }

     if ( EGL_FALSE == eglGetConfigs(display, configs, max_num_config, &max_num_config)) {
         log_error("eglGetConfigs() failed to retrive the configs (error %s)",
                   getErrorString(eglGetError()));
         return 0;
     }
     
#if 0
     // This prints out all the configurations, so it can be quite large
     for (int i=0; i<max_num_config; i++ ) {
         std::cerr << "Config[" << i << "] is:" << std::endl;
         printEGLConfig(configs[i]);
     }
#endif
     
     return max_num_config;
}

void
EGLDevice::printEGLAttribs(const EGLint *attrib)
{
    if (attrib) {
        std::cout << "Printing EGL Attributes list" << std::endl;
        int i = 0;
        while (attrib[i] != EGL_NONE) {
            switch (attrib[i]) {
            case EGL_RED_SIZE:
                std::cout << "\tRed: " << attrib[i+1];
                break;
            case EGL_GREEN_SIZE:
                std::cout << ", Green: " << attrib[i+1];
                break;
            case EGL_BLUE_SIZE:
                std::cout << ", Blue: " << attrib[i+1] << std::endl;
                break;
            case EGL_DEPTH_SIZE:
                std::cout << ", Depth: " << attrib[i+1] << std::endl;
                break;
            case EGL_RENDERABLE_TYPE:
                if (attrib[i+1] & EGL_OPENVG_BIT) {
                    std::cout << "\tOpenVG Renderable" << std::endl;
                }
                if (attrib[i+1] & EGL_OPENGL_ES_BIT) {
                    std::cout << "\tOpenGLES1 Renderable" << std::endl;
                }
                if (attrib[i+1] & EGL_OPENGL_ES2_BIT) {
                    std::cout << "\tOpenGLES2 Renderable" << std::endl;
                }
                break;
            default:
                break;
            }
            i+=2;
        }
    }
    std::cout << "----------------------------------" << std::endl;
}

void
EGLDevice::printEGLConfig(EGLConfig config)
{
    EGLint red, blue, green;
    EGLint value;
    eglGetConfigAttrib(_eglDisplay, config, EGL_RED_SIZE, &red);
    eglGetConfigAttrib(_eglDisplay, config, EGL_GREEN_SIZE, &green);
    eglGetConfigAttrib(_eglDisplay, config, EGL_BLUE_SIZE, &blue);
    std::cout << "\tConfig has RED = " << red << ", GREEN = " << green
              << ", BLUE = " << blue  << std::endl;
    
    eglGetConfigAttrib(_eglDisplay, config, EGL_ALPHA_SIZE, &value);
    std::cout << "\tEGL_ALPHA_SIZE is " << value  << std::endl;
    eglGetConfigAttrib(_eglDisplay, config, EGL_STENCIL_SIZE, &value);
    std::cout << "\tEGL_STENCIL_SIZE is " << value  << std::endl;
    eglGetConfigAttrib(_eglDisplay, config, EGL_SAMPLES, &value);
    std::cout << "\tEGL_SAMPLES is " << value  << std::endl;
    eglGetConfigAttrib(_eglDisplay, config, EGL_DEPTH_SIZE, &value);
    std::cout << "\tEGL_DEPTH_SIZE is " << value  << std::endl;
    eglGetConfigAttrib(_eglDisplay, config, EGL_MAX_SWAP_INTERVAL, &value);
    std::cout << "\tEGL_MAX_SWAP_INTERVAL is " << value << std::endl;
    eglGetConfigAttrib(_eglDisplay, config, EGL_MIN_SWAP_INTERVAL, &value);
    std::cout << "\tEGL_MIN_SWAP_INTERVAL is " << value << std::endl;
    eglGetConfigAttrib(_eglDisplay, config, EGL_NATIVE_RENDERABLE, &value);
    std::string val = (value)? "true" : "false";
    std::cout << "\tEGL_NATIVE_RENDERABLE is " << val << std::endl;
    eglGetConfigAttrib(_eglDisplay, config, EGL_SAMPLE_BUFFERS, &value);
    std::cout << "\tEGL_SAMPLE_BUFFERS is " << value << std::endl;
    eglGetConfigAttrib(_eglDisplay, config, EGL_RENDERABLE_TYPE, &value);
    if (value > 0) {
        std::string str;
        if (value & EGL_OPENGL_ES2_BIT) {
            str += " OpenGL-ES 2.0";
        }
        if (value & EGL_OPENGL_ES_BIT) {
            str += " OpenGL-ES 1.1";
        }
        if (value & EGL_OPENVG_BIT) {
            str += " OpenVG";
        }
        if (value & EGL_OPENGL_BIT) {
            str += " OpenGL";
        }
        std::cout <<"\tEGL_RENDERABLE_TYPE = " << str << std::endl;
    } else {
        std::cout <<"\tEGL_RENDERABLE_TYPE (default)" << std::endl;
    }
    eglGetConfigAttrib(_eglDisplay, config, EGL_SURFACE_TYPE, &value);
    if (value > 0) {
        std::string str;
        if (value & EGL_WINDOW_BIT) {
            str += " Window";
        }
        if (value & EGL_PIXMAP_BIT) {
            str += " Pixmap";
        }
        if (value & EGL_PBUFFER_BIT) {
            str += " Pbuffer";
        }
        std::cout <<"\tEGL_SURFACE_TYPE = " << str  << std::endl;
    } else {
          std::cout <<"\tEGL_SURFACE_TYPE (default)" << std::endl;
    }
    eglGetConfigAttrib(_eglDisplay, config, EGL_NATIVE_VISUAL_ID, &value);
    std::cout << "\tX11 Visual is: " << value << std::endl;
}

void
EGLDevice::printEGLContext(EGLContext context)
{
    EGLint value;
    eglQueryContext(_eglDisplay, context, EGL_CONFIG_ID, &value);
    std::cout << "Context EGL_CONFIG_ID is " << value << std::endl;
    eglQueryContext(_eglDisplay, context, EGL_CONTEXT_CLIENT_TYPE, &value);
    std::cout << "\tEGL_CONTEXT_CLIENT_TYPE is "
              << std::string((value == EGL_OPENVG_API)
              ? "EGL_OPENVG_API" : "EGL_OPENGL_ES_API") << std::endl;
    // eglQueryContext(_eglDisplay, context, EGL_CONTEXT_CLIENT_VERSION, &value);
    // log_debug("EGL_CONTEXT_CLIENT_VERSION is %d", value);
    eglQueryContext(_eglDisplay, context, EGL_RENDER_BUFFER, &value);
    std::cout << "\tEGL_RENDER_BUFFER is " << std::string((value == EGL_BACK_BUFFER)
              ? "EGL_BACK_BUFFER" : "EGL_SINGLE_BUFFER") << std::endl;
}

void
EGLDevice::printEGLSurface(EGLSurface surface)
{
    EGLint value;
    eglQuerySurface(_eglDisplay, surface, EGL_CONFIG_ID, &value);
    std::cout << "Surface EGL_CONFIG_ID is " << value << std::endl;
    eglQuerySurface(_eglDisplay, surface, EGL_HEIGHT, &value);
    std::cout << "\tEGL_HEIGHT is " << value<< std::endl;
    eglQuerySurface(_eglDisplay, surface, EGL_WIDTH, &value);
    std::cout << "\tEGL_WIDTH is " << value << std::endl;
    eglQuerySurface(_eglDisplay, surface, EGL_RENDER_BUFFER, &value);
    std::cout << "\tEGL_RENDER_BUFFER is " << std::string((value == EGL_BACK_BUFFER)
              ? "EGL_BACK_BUFFER" : "EGL_SINGLE_BUFFER") << std::endl;
    eglQuerySurface(_eglDisplay, surface, EGL_VERTICAL_RESOLUTION, &value);
    std::cout << "\tEGL_VERTICAL_RESOLUTION is " << value << std::endl;
    eglQuerySurface(_eglDisplay, surface, EGL_HORIZONTAL_RESOLUTION, &value);
    std::cout << "\tEGL_HORIZONTAL_RESOLUTION is " << value << std::endl;
    eglQuerySurface(_eglDisplay, surface, EGL_SWAP_BEHAVIOR, &value);
    std::cout << "\tEGL_SWAP_BEHAVIOR is "
              << std::string((value == EGL_BUFFER_DESTROYED)
                 ? "EGL_BUFFER_DESTROYED" : "EGL_BUFFER_PRESERVED") << std::endl;
    eglQuerySurface(_eglDisplay, surface, EGL_MULTISAMPLE_RESOLVE, &value);
    std::cout << "\tEGL_MULTISAMPLE_RESOLVE is "
              << std::string((value == EGL_MULTISAMPLE_RESOLVE_BOX)
                 ? "EGL_MULTISAMPLE_RESOLVE_BOX" : "EGL_MULTISAMPLE_RESOLVE_DEFAULT") << std::endl;
}

// EGL WIDTH, EGL HEIGHT, EGL LARGEST PBUFFER, EGL TEXTURE FORMAT, 
// EGL TEXTURE TARGET, EGL MIPMAP TEXTURE, EGL COLORSPACE, and EGL ALPHA FORMAT.
EGLSurface
EGLDevice::createPbuffer(int width, int height)
{
    const EGLint attribs[] = {
        EGL_WIDTH,      width,
        EGL_HEIGHT,     height,
        EGL_NONE
    };

    EGLSurface pbuf = eglCreatePbufferSurface(_eglDisplay, _eglConfig, attribs);
    if (pbuf == EGL_NO_SURFACE) {
        log_error( "eglCreatePbufferSurface() failed (error 0x%x)", eglGetError());
        return EGL_NO_SURFACE;
    }

    _pbuffers.push_back(pbuf);
    
    return pbuf;
}
EGLSurface
EGLDevice::createPbuffer(int width, int height, EGLClientBuffer buf, EGLenum type)
{
    const EGLint attribs[] = {
        EGL_WIDTH,      width,
        EGL_HEIGHT,     height,
        EGL_NONE
    };

    EGLSurface pbuf = eglCreatePbufferFromClientBuffer(_eglDisplay, type, buf,
                                              _eglConfig, attribs);
    if (pbuf == EGL_NO_SURFACE) {
        log_error( "eglCreatePbufferFromClientBuffer() failed (error 0x%x)",
                   eglGetError());
        return EGL_NO_SURFACE;
    }

    _pbuffers.push_back(pbuf);
    
    return pbuf;
}

EGLSurface
EGLDevice::createPixmap(int width, int height, NativePixmapType buf)
{
      const EGLint attribs[] = {
        EGL_WIDTH,      width,
        EGL_HEIGHT,     height,
        EGL_NONE
    };

      EGLSurface pbuf = eglCreatePixmapSurface(_eglDisplay, _eglConfig, buf, attribs);
    if (pbuf == EGL_NO_SURFACE) {
        log_error( "eglCreatePbufferFromClientBuffer() failed (error 0x%x)",
                   eglGetError());
        return EGL_NO_SURFACE;
    }

    _pbuffers.push_back(pbuf);
    
    return pbuf;  
}

} // namespace renderer
} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
