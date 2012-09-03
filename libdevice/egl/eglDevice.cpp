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

#include "configTemplates.h"

namespace gnash {

namespace renderer {

// The debug log used by all the gnash libraries.
static LogFile& dbglogfile = LogFile::getDefaultInstance();

EGLDevice::EGLDevice()
    : _eglConfig(0),
      _eglContext(EGL_NO_CONTEXT),
      _eglSurface(EGL_NO_SURFACE),
      _eglDisplay(EGL_NO_DISPLAY),
      _quality(LOW),
      _attrib(0),
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
    : _eglConfig(0),
      _eglContext(EGL_NO_CONTEXT),
      _eglSurface(EGL_NO_SURFACE),
      _eglDisplay(EGL_NO_DISPLAY),
      _quality(LOW),
      _attrib(0),
#if BUILD_X11_DEVICE
      _bpp(32)
#else
      _bpp(16)
#endif
{
    GNASH_REPORT_FUNCTION;

    setAttrib(_bpp);

    if (!initDevice(argc, argv)) {
        log_error(_("Couldn't initialize EGL device!"));
    }
}

EGLDevice::EGLDevice(GnashDevice::rtype_t rtype)
    : _eglConfig(0),
      _eglContext(EGL_NO_CONTEXT),
      _eglSurface(EGL_NO_SURFACE),
      _eglDisplay(EGL_NO_DISPLAY),
      _quality(LOW),
      _attrib(0),
#if BUILD_X11_DEVICE
      _bpp(32)
#else
      _bpp(16)
#endif
{
    GNASH_REPORT_FUNCTION;
    
    setAttrib(_bpp);

    if (!initDevice(0, 0)) {
        log_error(_("Couldn't initialize EGL device!"));
    }
    if (!bindClient(rtype)) {
        log_error(_("Couldn't bind client to type %d!"), rtype);
    }
}

void
EGLDevice::setAttrib(int bpp)
{ 
    switch (bpp) {
    case 32:
        _attrib = attrib32_low;
        break;
    case 16:
        _attrib = attrib16_low;
        break;
    case 1:
        _attrib = attrib1_list;
        break;
    }
}

EGLDevice::~EGLDevice()
{
    // GNASH_REPORT_FUNCTION;

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
EGLDevice::initDevice(int /* argc */, char **/*argv[] */)
{
    // EGLDevice::rtype_t rtype;

    dbglogfile.setVerbosity(2);

    GNASH_REPORT_FUNCTION;
    
    // step 1 - get an EGL display
//    _eglDisplay = eglGetDisplay(XOpenDisplay(0));
    _eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (EGL_NO_DISPLAY == _eglDisplay) {
        log_error(_( "eglGetDisplay() failed (error 0x%x)"), eglGetError());
        return false;
    }

    // This can be called multiple times safely
    if (eglInitialize(_eglDisplay, 0, 0) != EGL_TRUE) {
        log_error(_( "eglInitialize() failed (error %s)"),
                   getErrorString(eglGetError()));
        return false;
    }

    // step2 - bind to the wanted client API
    /// This is done by bindClient() later on
    // bindClient(GnashDevice::OPENVG);
    // queryEGLConfig(_eglDisplay);
   
    log_debug(_("EGL_CLIENT_APIS = %s"), eglQueryString(_eglDisplay, EGL_CLIENT_APIS));
    log_debug(_("EGL_EXTENSIONS = %s"), eglQueryString(_eglDisplay, EGL_EXTENSIONS));
    log_debug(_("EGL_VERSION = %s, EGL_VENDOR = %s"),
              eglQueryString(_eglDisplay, EGL_VERSION),
                eglQueryString(_eglDisplay, EGL_VENDOR));

    // step3 - find a suitable config
    EGLint max_num_config = 0;
    
    // Get the number of supported configurations
    if ( EGL_FALSE == eglGetConfigs(_eglDisplay, 0, 0, &max_num_config) ) {
        log_error(_("eglGetConfigs() failed to retrieve the number of configs (error %s)"),
                    getErrorString(eglGetError()));
        return 0;
    }
    if(max_num_config <= 0) {
        log_error(_( "No EGLconfigs found\n" ));
        return 0;
    }
    log_debug(_("Max number of EGL Configs is %d"), max_num_config);

    // The quality of the rendering is controlled by the number of samples
    // and sample buffers as specified in the configuration. Higher quality
    // settings force lower performance.

    // eglChooseConfig() always returns EGL_SUCCESS, so we we just check the
    // returned number of configurations to see if eglChooseConfig() actually
    // found a workable configuration.
    EGLint eglNumOfConfigs = 0;
    switch (_quality) {
      case EGLDevice::LOW:
          eglChooseConfig(_eglDisplay, attrib32_low, &_eglConfig,
                          1, &eglNumOfConfigs);
          if (eglNumOfConfigs) {
              log_debug(_("Using the 32bpp, low quality configuration"));
          } else {
              log_error(_("eglChooseConfig(32-low) failed"));
              eglChooseConfig(_eglDisplay, attrib16_low, &_eglConfig,
                              1, &eglNumOfConfigs);
              if (eglNumOfConfigs) {
                  log_debug(_("Using the 16bpp, low quality configuration"));
              } else {
                  log_error(_("eglChooseConfig(16-low) failed"));
                  return false;
              }
          }
          break;
      case EGLDevice::MEDIUM:
          eglChooseConfig(_eglDisplay, attrib32_medium, &_eglConfig,
                          1, &eglNumOfConfigs);
          if (eglNumOfConfigs) {
              log_debug(_("Using the 32bpp, medium quality configuration"));
          } else {
              log_error(_("eglChooseConfig(32-medium) failed"));
              eglChooseConfig(_eglDisplay, attrib16_medium, &_eglConfig,
                              1, &eglNumOfConfigs);
              if (eglNumOfConfigs) {
                  log_debug(_("Using the 16bpp, medium quality configuration"));
              } else {
                  log_error(_("eglChooseConfig(16-medium) failed"));
                  return false;
              }
          }
          break;
      case EGLDevice::HIGH:
          eglChooseConfig(_eglDisplay, attrib32_high, &_eglConfig,
                          1, &eglNumOfConfigs);
          if (eglNumOfConfigs) {
              log_debug(_("Using the 32bpp, high quality configuration"));
          } else {
              log_error(_("eglChooseConfig(32-high) failed"));
              eglChooseConfig(_eglDisplay, attrib16_high, &_eglConfig,
                              1, &eglNumOfConfigs);
              if (eglNumOfConfigs) {
                  log_debug(_("Using the 16bpp, medium quality configuration"));
              } else {
                  log_error(_("eglChooseConfig(16-high) failed"));
                  return false;
              }
          }
          break;
      default:
          break;
    }

   if (!checkEGLConfig(_eglConfig)) {
       log_error(_("EGL configuration doesn't match!"));
       //return false;
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

#ifdef BUILD_X11_DEVICE
EGLint
EGLDevice::getNativeVisual()
{
    EGLint vid;
    if (_eglDisplay && _eglConfig) {
        if (!eglGetConfigAttrib(_eglDisplay, _eglConfig, EGL_NATIVE_VISUAL_ID, &vid)) {
            log_error(_("eglGetConfigAttrib() failed (error %s)"),
                        getErrorString(eglGetError()));
            return 0;
        } else {
            log_debug(_("EGL native visual is: %d"), vid);
        }
    }

    return vid;
}
#endif

bool
EGLDevice::bindClient(rtype_t rtype)
{
    GNASH_REPORT_FUNCTION;
    
    switch (rtype) {
      case GnashDevice::OPENGLES2:
      {
          log_debug(_("Initializing EGL for OpenGLES2"));
          if(EGL_FALSE == eglBindAPI(EGL_OPENGL_ES_API)) {
              log_error(_("eglBindAPI() failed to retrieve the number of configs (error %s)"),
                          getErrorString(eglGetError()));
              return false;
          }
          break;
      }
      case GnashDevice::OPENGLES1:
      {
          log_debug(_("Initializing EGL for OpenGLES1"));
          if(EGL_FALSE == eglBindAPI(EGL_OPENGL_ES_API)) {
              log_error(_("eglBindAPI() failed to retrive the number of configs (error %s)"),
                          getErrorString(eglGetError()));
              return false;
          }
          break;
      }
      case GnashDevice::OPENVG:
      {
          log_debug(_("Initializing EGL for OpenVG"));
          if(EGL_FALSE == eglBindAPI(EGL_OPENVG_API)) {
              log_error(_("eglBindAPI() failed to retrieve the number of configs (error %s)"),
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
        throw GnashException("bogus window handle!");
    } else {
#ifdef EGL_NATIVE_WINDOW_INT
        _nativeWindow = static_cast<EGLNativeWindowType>(window);
#else
        _nativeWindow = reinterpret_cast<EGLNativeWindowType>(window);
#endif
    }

    if (_eglSurface != EGL_NO_SURFACE) {
        eglDestroySurface(_eglDisplay, _eglSurface);
    }
    
    log_debug(_("Initializing EGL Surface"));
    if (_eglDisplay && _eglConfig) {
        _eglSurface = eglCreateWindowSurface(_eglDisplay, _eglConfig,
                                             _nativeWindow, surface_attributes);
    }
    
    if (EGL_NO_SURFACE == _eglSurface) {
        log_error(_("eglCreateWindowSurface failed (error %s)"),
                    getErrorString(eglGetError()));
    } else {
        printEGLSurface(_eglSurface);
    }
    
    // step5 - create a context
    _eglContext = eglCreateContext(_eglDisplay, _eglConfig, EGL_NO_CONTEXT, NULL);
    if (EGL_NO_CONTEXT == _eglContext) {
        // At least on Ubuntu 10.10, this returns a successful error string
        // with LibeMesa's OpenVG 1.0 implementation. With OpenVG 1.1 on
        // an ARM board, this works fine. Even the libMesa examples fail
        // the same way.
        boost::format fmt = boost::format(
                             _("eglCreateContext failed (error %s)")
                                           ) % getErrorString(eglGetError());
        throw GnashException(fmt.str());
    } else {
        printEGLContext(_eglContext);
    }
    
    // step6 - make the context and surface current
    if (EGL_FALSE == eglMakeCurrent(_eglDisplay, _eglSurface, _eglSurface, _eglContext)) {
        // If for some reason we get a context, but can't make it current,
        // nothing else will work anyway, so don't continue.
        boost::format fmt = boost::format(
                             _("eglMakeCurrent failed (error %s)")
                                           ) % getErrorString(eglGetError());
        throw GnashException(fmt.str());
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
         log_error(_("eglGetConfigs() failed to retrieve the number of configs (error %s)"),
                     getErrorString(eglGetError()));
         return 0;
     }
     if(max_num_config <= 0) {
         log_error(_("No EGLconfigs found\n"));
         return 0;
     }
     log_debug(_("Max number of EGL Configs is %d"), max_num_config);
     
     configs = new EGLConfig[max_num_config];
     if (0 == configs) {
         log_error(_("Out of memory\n"));
         return 0;
     }

     if ( EGL_FALSE == eglGetConfigs(display, configs, max_num_config, &max_num_config)) {
         log_error(_("eglGetConfigs() failed to retrieve the configs (error %s)"),
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
    
    eglGetConfigAttrib(_eglDisplay, config, EGL_BUFFER_SIZE, &value);
    std::cout << "\tEGL_BUFFER_SIZE is " << value  << std::endl;
    eglGetConfigAttrib(_eglDisplay, config, EGL_ALPHA_SIZE, &value);
    std::cout << "\tEGL_ALPHA_SIZE is " << value  << std::endl;
    eglGetConfigAttrib(_eglDisplay, config, EGL_ALPHA_MASK_SIZE, &value);
    std::cout << "\tEGL_ALPHA_MASK_SIZE is " << value  << std::endl;
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

    eglGetConfigAttrib(_eglDisplay, config, EGL_CONFIG_CAVEAT, &value);
    if (value > 0) {
        std::string str;
        if (value & EGL_NONE) {
            str += " EGL_NONE";
        }
        if (value & EGL_SLOW_CONFIG) {
            str += " EGL_SLOW_CONFIG";
        }
        if (value & EGL_NON_CONFORMANT_CONFIG) {
            str += " EGL_NON_CONFORMANT_CONFIG";
        }
        std::cout <<"\tEGL_CONFIG_CAVEAT = " << str << std::endl;
    } else {
        std::cout <<"\tEGL_CONFIG_CAVEAT (default)" << std::endl;
    }

#ifdef BUILD_X11_DEVICE
    eglGetConfigAttrib(_eglDisplay, config, EGL_NATIVE_VISUAL_ID, &value);
    std::cout << "\tX11 Visual is: " << value << std::endl;
#endif
    
    eglGetConfigAttrib(_eglDisplay, config, EGL_BIND_TO_TEXTURE_RGB, &value);
    val = (value)? "true" : "false";
    std::cout << "\tEGL_BIND_TO_TEXTURE_RGB is " << val << std::endl;

    eglGetConfigAttrib(_eglDisplay, config, EGL_BIND_TO_TEXTURE_RGBA, &value);
    val = (value)? "true" : "false";
    std::cout << "\tEGL_BIND_TO_TEXTURE_RGBA is " << val << std::endl;

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
        log_error(_( "eglCreatePbufferSurface() failed (error 0x%x)"), eglGetError());
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
        log_error(_("eglCreatePbufferFromClientBuffer() failed (error 0x%x)"),
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
        log_error(_("eglCreatePbufferFromClientBuffer() failed (error 0x%x)"),
                   eglGetError());
        return EGL_NO_SURFACE;
    }

    _pbuffers.push_back(pbuf);
    
    return pbuf;  
}

// Set the bitmask for the configured renderable types.
EGLint
EGLDevice::getRenderableTypes()
{
    EGLint type = 0;
#ifdef RENDERER_GLES1
    type = type | EGL_OPENGL_ES_BIT;
#endif
#ifdef RENDERER_GLES2
    type = type | EGL_OPENGL_ES2_BIT;
#endif
#ifdef RENDERER_OPENVG
    type = type | EGL_OPENVG_BIT;
#endif
    return type;
}



} // namespace renderer
} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
