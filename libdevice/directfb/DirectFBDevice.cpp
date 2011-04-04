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
#include <sstream>

#include "log.h"
// #include "RunResources.h"
#include "Renderer.h"
#include "GnashException.h"

#ifdef HAVE_DIRECTFB_H
# include <directfb/directfb.h>
#else
# error "This file needs DirectFB"
#endif

#include "DirectFBDevice.h"

namespace gnash {

namespace renderer {

namespace directfb {
    
// The debug log used by all the gnash libraries.
static LogFile& dbglogfile = LogFile::getDefaultInstance();

// FIXME: this font name shouldn't be hardcoded!
const char *FONT = "/usr/share/fonts/truetype/freefont/FreeSerif.ttf";

DirectFBDevice::DirectFBDevice()
    : _dfb(0),
      _surface(0),
      _keyboard(0),
      _keybuffer(0),
      _provider(0),
      _font(0),
      _layer(0),
      _screen(0)
{
    GNASH_REPORT_FUNCTION;
}

DirectFBDevice::DirectFBDevice(int argc, char *argv[])
    : _dfb(0),
      _surface(0),
      _keyboard(0),
      _keybuffer(0),
      _provider(0),
      _font(0),
      _layer(0),
      _screen(0)
{
    GNASH_REPORT_FUNCTION;
    
    if (!initDevice(argc, argv)) {
        log_error("Couldn't initialize DirectFB device!");
    }
}

DirectFBDevice::~DirectFBDevice()
{
    // GNASH_REPORT_FUNCTION;

    if (_keybuffer) {
        _keybuffer->Release(_keybuffer);
    }
    if (_surface) {
        _surface->Release(_surface);
    }
    if (_provider) {
        _provider->Release(_provider);
    }
    if (_font) {
        _font->Release(_font);
    }
    if (_layer) {
        _layer->Release(_layer);
    }
    if (_dfb) {
        _dfb->Release(_dfb);
    }
}

bool
DirectFBDevice::initDevice(int argc, char *argv[])
{
    GNASH_REPORT_FUNCTION;

    DFBResult result;

    if ((result = DirectFBInit(&argc, &argv)) != DR_OK) {
	log_error("DirectFBInit(): %s", getErrorString(result));
        return false;
    }

    if ((result = DirectFBCreate(&_dfb)) != DR_OK) {
	log_error("DirectFBCreate(): %s", getErrorString(result));
        return false;
    }

    // get an interface to the primary keyboard and create an
    // input buffer for it
   _dfb->GetInputDevice(_dfb, DIDID_KEYBOARD, &_keyboard);
    _keyboard->CreateEventBuffer(_keyboard, &_keybuffer);

    // get an interface to the primary keyboard and create an
    // input buffer for it
    _dfb->GetInputDevice(_dfb, DIDID_KEYBOARD, &_keyboard);
    _keyboard->CreateEventBuffer(_keyboard, &_keybuffer );
    
    // set our cooperative level to DFSCL_FULLSCREEN for exclusive access to
    // the primary layer
    _dfb->SetCooperativeLevel(_dfb, DFSCL_FULLSCREEN);
    
    DFBSurfaceDescription dsc;

    // get the primary surface, i.e. the surface of the primary layer we have
    // exclusive access to
    dsc.flags = DSDESC_CAPS;
//    dsc.caps = static_cast<DFBSurfaceCapabilities>(DSCAPS_PRIMARY | DSCAPS_DOUBLE | DSCAPS_VIDEOONLY);
    dsc.caps = static_cast<DFBSurfaceCapabilities>(DSCAPS_PRIMARY | DSCAPS_FLIPPING);
    
    if ((result = _dfb->CreateSurface(_dfb, &dsc, &_surface)) != DR_OK) {
	log_error("CreateSurface(): %s", getErrorString(result));
        return false;
    }

    int x, y;
    _surface->GetSize(_surface, &x, &y);
    
    DFBFontDescription fdesc;
    fdesc.flags = DFDESC_HEIGHT;
    fdesc.height = y/10;
    
    if ((result == _dfb->CreateFont(_dfb, FONT, &fdesc, &_font)) == DR_OK) {
	log_error("CreateFont(): %s", getErrorString(result));
    }
    _surface->SetFont(_surface, _font);

#if 0
    DFBSurfaceDescription sdesc;
    if ((result == _dfb->CreateImageProvider(_dfb, "/tmp/img832a.jpg", &_provider)) != DR_OK) {
	log_error("CreateImageProvider(): %s", getErrorString(result));
    }
    _provider->GetSurfaceDescription(_provider, &sdesc);
#endif

    if ((result == _dfb->GetDisplayLayer(_dfb, DLID_PRIMARY, &_layer)) == DR_OK) {
	log_error("GetDisplayLayer(): %s", getErrorString(result));
    }
    
    if ((result == _layer->GetScreen(_layer, &_screen)) == DR_OK) {
	log_error("GetScreen(): %s", getErrorString(result));
    }
    
    DFBSurfacePixelFormat format;
    _surface->GetPixelFormat(_surface, &format);

#if 0
    _surface->FillRectangle (_surface, 0, 0, x, y);
    _surface->SetColor (_surface, 0x80, 0x80, 0xff, 0xff);
    _surface->DrawLine (_surface, 0, x / 2, x - 1, y / 2);
    _surface->Flip (_surface, NULL, DSFLIP_NONE);
    sleep(15);
#endif
    
    return true;
}

bool
DirectFBDevice::attachWindow(GnashDevice::native_window_t window)
{
    GNASH_REPORT_FUNCTION;
}

void
DirectFBDevice::printDisplayLayerConfig(DFBDisplayLayerConfig *config)
{
    std::cerr << "DisplayLayerConfig:" << std::endl;
    
    if (config) {
        std::cerr << "\tLayer Width: " << config->width 
                  << ", Layer Height: " << config->height << std::endl;
        printSurfacePixelFormat(config->pixelformat);
        printDisplayLayerBufferMode(config->buffermode);
        printSurfaceCapabilities(config->surface_caps);
        // FIXME: these two need to be implemented
        // printDisplayLayerSourceID(config->source);
        // printDisplayLayerOptions(config->options);
    }
}

void
DirectFBDevice::printDisplayLayerBufferMode(DFBDisplayLayerBufferMode mode)
{
    std::cerr << "DisplayLayerBufferMode:" << std:: endl;

    if (mode == DLBM_UNKNOWN) {
        std::cerr << "\tDLBM_UNKNOWN: Unknown Display LAyer Buffer Mode" << std::endl;
    }
    if (mode == DLBM_FRONTONLY) {
        std::cerr << "\tDLBM_FRONTONLY: No Backbuffer" << std::endl;
    }
    if (mode & DLBM_BACKVIDEO) {
        std::cerr << "\tDLBM_BACKVIDEO: backbuffer in video memory */";
    }
    if (mode & DLBM_BACKSYSTEM) {
        std::cerr << "\tDLBM_BACKSYSTEM: backbuffer in system memory */";
    }
    if (mode & DLBM_TRIPLE) {
        std::cerr << "\tDLBM_TRIPLE: triple buffering */";
    }
    if (mode & DLBM_WINDOWS) {
        std::cerr << "\tDLBM_WINDOWS: no layer buffers at all,using buffer of each window";
    }

}

int
DirectFBDevice::getDepth(DFBSurfacePixelFormat format)
{
    if (format == DSPF_UNKNOWN) {
        return 0;
    }
    if (format & DSPF_ARGB1555) {
        return 16;
    }
    if (format & DSPF_RGB16) {
        return 16;
    }
    if (format & DSPF_RGB24) {
        return 24;
    }
    if (format & DSPF_RGB32) {
        return 24;
    }
    if (format & DSPF_ARGB) {
        return 32;
    }
    if (format & DSPF_A8) {
        return 8;
    }
    if (format & DSPF_YUY2) {
        return 16;
    }
    if (format & DSPF_RGB332) {
        return 8;
    }
    if (format & DSPF_UYVY) {
        return 16;
    }
    if (format & DSPF_I420) {
        return 12;
    }
    if (format & DSPF_YV12) {
        return 12;
    }
    if (format & DSPF_LUT8) {
        return 8;
    }
    if (format & DSPF_ALUT44) {
        return 8;
    }
    if (format & DSPF_AiRGB) {
        return 32;
    }
    if (format & DSPF_A1) {
        return 1;
    }
    if (format & DSPF_NV12) {
        return 12;
    }
    if (format & DSPF_NV16) {
        return 16;
    }
    if (format & DSPF_ARGB2554) {
        return 16;
    }
    if (format & DSPF_ARGB4444) {
        return 26;
    }
#ifdef DSPF_RGBA4444
    if (format & DSPF_RGBA4444) {
        return 16;
    }
#endif
    if (format & DSPF_NV21) {
        return 12;
    }
    if (format & DSPF_AYUV) {
        return 32;
    }
    if (format & DSPF_A4) {
        return 4;
    }
    if (format & DSPF_ARGB1666) {
        return 1;
    }
    if (format & DSPF_ARGB6666) {
        return 16;
    }
    if (format & DSPF_RGB18) {
        return 16;
    }
    if (format & DSPF_LUT2) {
        return 12;
    }
    if (format & DSPF_RGB444) {
        return 16;
    }
    if (format & DSPF_RGB555) {
        return 16;
    }
    if (format & DSPF_BGR555) {
        return 16;
    }
#ifdef DSPF_RGBA5551
    if (format & DSPF_RGBA5551) {
        return 16;
    }
#endif
#ifdef DSPF_YUV444P
    if (format & DSPF_YUV444P) {
        return 24;
    }
#endif
#ifdef DSPF_ARGB8565
    if (format & DSPF_ARGB8565) {
        return 24;
    }
#endif
#ifdef DSPF_AVYU
    if (format & DSPF_AVYU) {
        return 32;
    }
#endif
#ifdef  DSPF_VYU
    if (format & DSPF_VYU) {
        return 24;
    }
#endif
#ifdef DSPF_A1_LSB
    if (format & DSPF_A1_LSB) {
        return 1;
    }
#endif
#ifdef DSPF_YV16
    if (format & DSPF_YV16) {
        return 16;
    }
#endif

    return 0;
}

void
DirectFBDevice::printFBDisplay(IDirectFBDisplayLayer *layer)
{
    std::stringstream ss;
    
    if (layer) {
        DFBDisplayLayerDescription dldesc;
        layer->GetDescription(layer, &dldesc);
        if (dldesc.name) {
            ss << "\tDisplay Name: " << dldesc.name << std::endl;
        }
        ss << "\tLevel: " << dldesc.level << std::endl;
        ss << "\tEegions: " << dldesc.regions << std::endl;
        ss << "\tSources: " << dldesc.sources << std::endl;
        ss << "\tclip_regions: " << dldesc.clip_regions << std::endl;

        printDisplayLayerTypeFlags(dldesc.type);
        printDisplayLayerCapabilities(dldesc.caps);
    }
    
    std::cerr << ss.str();
}

void
DirectFBDevice::printDisplayLayerCapabilities(DFBDisplayLayerCapabilities caps)
{
    std::stringstream ss;

    if (caps == DLCAPS_NONE) {
        ss << "\tDLCAPS_NONE: No Capabilities" << std::endl;
    }	 	
    if (caps & DLCAPS_SURFACE) {
        ss << "\tDLCAPS_SURFACE: The layer has a surface that " << std::endl
           << "\t\tcan be drawn to. This may not be provided by layers"  << std::endl
           << "\t\tthat display realtime data, e.g. from an MPEG decoder chip. Playback" << std::endl
           << "\t\tcontrol may be provided by an external API." << std::endl;
    }
    if (caps & DLCAPS_OPACITY) {
        ss << "\tDLCAPS_OPACITY: The layer supports blending with layer(s) below based on a global alpha factor." << std::endl;
    }
    if (caps & DLCAPS_ALPHACHANNEL) {
        ss << "\tDLCAPS_ALPHACHANNEL: The layer supports blending with layer(s) below based on each pixel's alpha value." << std::endl;
    }
    if (caps & DLCAPS_SCREEN_LOCATION) {
        ss << "\tDLCAPS_SCREEN: The layer location on the screen can be changed, this includes position and size as" << std::endl
           << "\tnormalized values. The default is 0.0f, 0.0f, 1.0f, 1.0f. Supports IDirectFBDisplayLayer::SetScreenLocation() " << std::endl
           << "\tand IDirectFBDisplayLayer::SetScreenRectangle(). This implies DLCAPS_SCREEN_POSITION and _SIZE." << std::endl;
    }
    if (caps & DLCAPS_FLICKER_FILTERING) {
        ss << "\tDLCAPS_FLICKER: Flicker filtering can be enabled for smooth output on interlaced display devices." << std::endl;
    }
    if (caps & DLCAPS_DEINTERLACING) {
        ss << "\tDLCAPS_DEINTERLACING: The layer provides optional deinterlacing for displaying interlaced video data"  << std::endl
           << "\ton progressive display devices." << std::endl;
    }
    if (caps & DLCAPS_SRC_COLORKEY) {
        ss << "\tDLCAPS_SRC_COLORKEY: A specific color can be declared as transparent." << std::endl;
    }
    if (caps & DLCAPS_DST_COLORKEY) {
        ss << "\tDLCAPS_DST: A specific color of layers below can be specified as the color of the only locations " << std::endl
           << "\twhere the layer is visible." << std::endl;
    }
    if (caps & DLCAPS_BRIGHTNESS) {
        ss << "\tDLCAPS_BRIGHTNESS: Adjustment of brightness is supported." << std::endl;
    }
    if (caps & DLCAPS_CONTRAST) {
        ss << "\tDLCAPS_CONTRAST: Adjustment of contrast is supported." << std::endl;
    }
    if (caps & DLCAPS_HUE) {
        ss << "\tDLCAPS_HUE: Adjustment of hue is supported." << std::endl;
    }
    if (caps & DLCAPS_SATURATION) {
        ss << "\tDLCAPS_SATURATION: Adjustment of saturation is supported." << std::endl;
    }
    if (caps & DLCAPS_LEVELS) {
        ss << "\tDLCAPS_LEVELS: Adjustment of the layer's level (z position) is supported." << std::endl;
    }
    if (caps & DLCAPS_FIELD_PARITY) {
        ss << "\tDLCAPS_FIELD: Field parity can be selected" << std::endl;
    }
    if (caps & DLCAPS_WINDOWS) {
        ss << "\tDLCAPS_WINDOWS: Hardware window support." << std::endl;
    }
    if (caps & DLCAPS_SOURCES) {
        ss << "\tDLCAPS_SOURCES: Sources can be selected." << std::endl;
    }
    if (caps & DLCAPS_ALPHA_RAMP) {
        ss << "\tDLCAPS_ALPHA:Alpha values for formats with one or two alpha bits can be chosen, i.e. using ARGB1555"  << std::endl
           << "\tor ARGB2554 the user can define the meaning of the two or four possibilities. In short, this feature" << std::endl
           << "\tprovides a lookup table for the alpha bits of these formats. See also IDirectFBSurface::SetAlphaRamp()." << std::endl;
    }
    if (caps & DLCAPS_PREMULTIPLIED) {
        ss << "\tDLCAPS_PREMULTIPLIED: Surfaces with premultiplied alpha are supported." << std::endl;
    }
    if (caps & DLCAPS_SCREEN_POSITION) {
        ss << "\tDLCAPS_SCREEN:The layer position on the screen can be changed. Supports IDirectFBDisplayLayer::SetScreenPosition()." << std::endl;
    }
    if (caps & DLCAPS_SCREEN_SIZE) {
        ss << "\tDLCAPS_SCREEN: The layer size (defined by its source rectangle) can be scaled to a different size on" << std::endl
           << "\tthe screen (defined by its screen/destination rectangle or its normalized size) and does not have to be 1:1 with it." << std::endl;
    }
    if (caps & DLCAPS_CLIP_REGIONS) {
        ss << "\tDLCAPS_CLIP_REGIONS: Supports IDirectFBDisplayLayer::SetClipRegions()." << std::endl;
    }
    if (caps & DLCAPS_ALL) {
        ss << "\tDLCAPS_ALL:: ALL capabilities" << std::endl;
    }

    std::cerr << "DisplayLayerCapabilities: " << std::endl << ss.str() << std::endl;
}

void
DirectFBDevice::printFBScreen(IDirectFBScreen *screen)
{
    std::stringstream ss;

    DFBResult result;

    if (screen) {
        // get the screen ID
        DFBScreenID id;
        screen->GetID(screen, &id);
        ss << "Screen ID: " << id << std::endl;

        // Get the size of the screen
        int x, y;
        screen->GetSize(screen, &x, &y);
        ss << "\tWidth: " << x << " Heigth: " << y << std::endl;

        DFBScreenDescription sdesc;
        // FIXME: On Ubuntu, this returns the wrong result
        if ((result == screen->GetDescription(screen, &sdesc)) == DR_OK) {
            log_error("GetDescription(): %s", getErrorString(result));
        }

        if (sdesc.name) {
            ss << "\tScreen Name is: " << sdesc.name << std::endl;
        }
        ss << "\tmkixers: " << sdesc.mixers << std::endl;
        ss << "\tencoders: " << sdesc.encoders << std::endl;
        ss << "\toutputs: " << sdesc.outputs << std::endl;

        printfScreenCapabilities(sdesc.caps);
    }
    
    std::cerr << "FBScreen: " << std::endl << ss.str() << std::endl;
}

void 
DirectFBDevice::printfScreenCapabilities(DFBScreenCapabilities caps)
{
    std::stringstream ss;

    if (caps) {
        if (caps == DSCCAPS_NONE) {
            ss << "\tDSCCAPS_NONE: No Capabilities" << std::endl;
        }
        if (caps == DSCCAPS_VSYNC) {
            ss << "\tDSCCAPS_VSYNC: Synchronization with the vertical retrace supported" << std::endl;
        }
        if (caps == DSCCAPS_POWER_MANAGEMENT) {
            ss << "\tDSCCAPS_POWER_MANGEMENT: power management" << std::endl;
        }
        if (caps == DSCCAPS_ENCODERS) {
            ss << "\tDSCCAPS_ENCODERS: Has encoders" << std::endl;
        }
        if (caps == DSCCAPS_OUTPUTS) {
            ss << "\tDSCCAPS_OUTPUTS: Has outputs" << std::endl;
        }
        if (caps == DSCCAPS_ALL) {
            ss << "\tDSCCAPS_ALL: Has everything" << std::endl;
        }
    }

    std::cerr << "Screen Capabilities:" << ss.str() << std::endl;
}

void
DirectFBDevice::printDisplayLayerTypeFlags(DFBDisplayLayerTypeFlags flags)
{
    std::stringstream ss;

    if (flags == DLTF_NONE) {
        ss << "\tUnclassified, no specific type";
    }
    if (flags & DLTF_GRAPHICS) {
        ss << "\tCan be used for graphics output";
    }
    if (flags & DLTF_VIDEO) {
        ss << "\tCan be used for live video output ";
    }
    if (flags & DLTF_STILL_PICTURE) {
        ss << "\tCan be used for single frames";
    }
    if (flags & DLTF_BACKGROUND) {
        ss << "\tCan be used as a background layer";
    }
    if (flags & DLTF_ALL) {
        ss << "\tAll type flags set";
    }

    std::cerr << "DisplayLayerTypeFlags: " << ss.str() << std::endl;
}

/// Return a string with the error code as text, instead of a numeric value
const char *
DirectFBDevice::getErrorString(int error)
{
    DFBResult code = static_cast<DFBResult>(error);
    
    switch (code) {
    case DR_OK: 
	return "No error occured";
	break;
    case DR_FAILURE: 
	return "A general or unknown error occured";
	break;
    case DR_INIT:
	return "A general initialization error occured";
	break;
    case DR_BUG:
	return "Internal bug or inconsistency has been detected";
	break;
    case DR_DEAD:
	return "Interface has a zero reference counter (available in debug mode)";
	break;
    case DR_UNSUPPORTED:
	return "The requested operation or an argument is (currently) not supported";
	break;
    case DR_UNIMPLEMENTED:
	return "The requested operation is not implemented, yet";
	break;
    case DR_ACCESSDENIED:
	return "Access to the resource is denied";
	break;
    case DR_INVAREA:
	return "An invalid area has been specified or detected";
	break;
    case DR_INVARG:
	return "An invalid argument has been specified";
	break;
    case DR_NOLOCALMEMORY:
	return "There's not enough system memory";
	break;
    case DR_NOSHAREDMEMORY:
	return "There's not enough shared memory";
	break;
    case DR_LOCKED:
	return "The resource is (already) locked";
	break;
    case DR_BUFFEREMPTY:
	return "The buffer is empty";
	break;
    case DR_FILENOTFOUND:
	return "The specified file has not been found";
	break;
    case DR_IO:
	return "A general I/O error occured";
	break;
    case DR_BUSY:
	return "The resource or device is busy";
	break;
    case DR_NOIMPL:
	return "No implementation for this interface or content type has been found";
	break;
    case DR_TIMEOUT:
	return "The operation timed out";
	break;
    case DR_THIZNULL:
	return "thiz' pointer is NULL";
	break;
    case DR_IDNOTFOUND:
	return "No resource has been found by the specified id";
	break;
    case DR_DESTROYED:
	return "The underlying object (e.g. a window or surface) has been destroyed";
	break;
    case DR_FUSION:
	return "Internal fusion error detected, most likely related to IPC resources";
	break;
    case DR_BUFFERTOOLARGE:
	return "Buffer is too large";
	break;
    case DR_INTERRUPTED:
	return "The operation has been interrupted";
	break;
    case DR_NOCONTEXT:
	return "No context available";
	break;
    case DR_TEMPUNAVAIL:
	return "Temporarily unavailable";
	break;
    case DR_LIMITEXCEEDED:
	return "Attempted to exceed limit, i.e. any kind of maximum size, count etc";
	break;
    case DR_NOSUCHMETHOD:
	return "Requested method is not known, e.g. to remote site";
	break;
    case DR_NOSUCHINSTANCE:
	return "Requested instance is not known, e.g. to remote site";
	break;
    case DR_ITEMNOTFOUND:
	return "No such item found";
	break;
    case DR_VERSIONMISMATCH:
	return "Some versions didn't match";
	break;
    case DR_EOF:
	return "Reached end of file";
	break;
    case DR_SUSPENDED:
	return "The requested object is suspended";
	break;
    case DR_INCOMPLETE:
	return "The operation has been executed, but not completely";
	break;
    case DR_NOCORE:
	return "Core part not available";
	break;
    case DFB_NOVIDEOMEMORY:
	return "There's not enough video memory";
	break;
    case DFB_MISSINGFONT:
	return "No font has been set";
	break;
    case DFB_MISSINGIMAGE:
	return "No image has been set";
	break;
    default:
	 return "Unknown error code!";
	 break;
    };
}

void 
DirectFBDevice::printAccelerationMask(DFBAccelerationMask mask)
{
    std::stringstream ss;
    
    if (mask == DFXL_NONE) {
        ss << "\tDFXL_NONE: None of these";
    }
    if (mask & DFXL_FILLRECTANGLE) {
        ss << "\tDFXL_FILLRECTANGLE: FillRectangle() is accelerated";
    }
    if (mask & DFXL_DRAWRECTANGLE) {
        ss << "\tDFXL_DRAWRECTANGLE: DrawRectangle() is accelerated";
    }
    if (mask & DFXL_DRAWLINE) {
        ss << "\tDFXL_DRAWLINE: DrawLine() is accelerated";
    }
    if (mask & DFXL_FILLTRIANGLE) {
        ss << "\tDFXL_FILLTRIANGLE: FillTriangle() is accelerated";
    }
    if (mask & DFXL_BLIT) {
        ss << "\tDFXL_BLIT: Blit() and TileBlit() are accelerated";
    }
    if (mask & DFXL_STRETCHBLIT) {
        ss << "\tDFXL_STRETCHBLIT: StretchBlit() is accelerated";
    }
    if (mask & DFXL_TEXTRIANGLES) {
        ss << "\tDFXL_TEXTRIANGLES: TextureTriangles() is accelerated";
    }
#ifdef DFXL_BLIT2
    if (mask & DFXL_BLIT2) {
        ss << "\tDFXL_BLIT2: BatchBlit2() is accelerated";
    }
#endif
    if (mask & DFXL_DRAWSTRING) {
        ss << "\tDFXL_DRAWSTRING: DrawString() and DrawGlyph() are accelerated";
    }
    if (mask & DFXL_ALL) {
        ss << "\tDFXL_ALL: All drawing/blitting functions";
    }

    std::cerr << "Acceleration Mask is: " << ss.str() << std::endl;
}

void
DirectFBDevice::printSurfaceBlittingFlags(DFBSurfaceBlittingFlags flags)
{
    // GNASH_REPORT_FUNCTION;

    std::stringstream ss;
    
    if (flags == DSBLIT_NOFX) {
        ss << "\tDSBLIT_NOFX: uses none of the effects" << std::endl;
    }
    if (flags & DSBLIT_BLEND_ALPHACHANNEL) {
        ss << "\tDSBLIT_BLEND_ALPHACHANNEL: enables blending and uses alphachannel from source" << std::endl;
    }
    if (flags & DSBLIT_BLEND_COLORALPHA) {
        ss << "\tDSBLIT_BLEND_COLORALPHA: enables blending and uses alpha value from color" << std::endl;
    }
    if (flags & DSBLIT_COLORIZE) {
        ss << "\tDSBLIT_COLORIZE: source color with the color's r/g/b values" << std::endl;
    }
    if (flags & DSBLIT_SRC_COLORKEY) {
        ss << "\tDSBLIT_SRC_COLORKEY: don't blit pixels matching the source color key" << std::endl;
    }
    if (flags & DSBLIT_DST_COLORKEY) {
        ss << "\tDSBLIT_DST_COLORKEY: write to destination only if the destination pi matches the destination color key" << std::endl;
    }
    if (flags & DSBLIT_SRC_PREMULTIPLY) {
        ss << "\tDSBLIT_SRC_PREMULTIPLY: modulates the source color with the (modulated) source alpha" << std::endl;
    }
    if (flags & DSBLIT_DST_PREMULTIPLY) {
        ss << "\tmodulates the dest. color with the dest. alpha" << std::endl;
    }
    if (flags & DSBLIT_DEMULTIPLY) {
        ss << "\tDSBLIT_DEMULTIPLY: the color by the alpha before writing data to the destination" << std::endl;
    }
    if (flags & DSBLIT_DEINTERLACE) {
        ss << "\tDSBLIT_DEINTERLACE: deinterlaces the source during blitting by reading" << std::endl;
    }
    if (flags & DSBLIT_SRC_PREMULTCOLOR) {
        ss << "\tDSBLIT_SRC_PREMULTCOLOR: modulates the source color with the color alpha" << std::endl;
    }
    if (flags & DSBLIT_XOR)  {
        ss << "\tDSBLIT_XOR: bitwise xor the destination pixels with the source pixels after premultiplication" << std::endl;
    }
#ifdef DSBLIT_INDEX_TRANSLATION
    if (flags & DSBLIT_INDEX_TRANSLATION) {
        ss "\tDSBLIT_INDEX_TRANSLATION: do fast indexed to indexed translation, this flag is mutual exclusive with all others" << std::endl;
    }
#endif
    if (flags & DSBLIT_ROTATE180) {
        ss << "\tDSBLIT_ROTATE180: rotate the image by 180 degree" << std::endl;
    }
    if (flags & DSBLIT_COLORKEY_PROTECT) {
        ss << "\tDSBLIT_COLORKEY_PROTECT: make sure written pixels don't match color key (internal only ATM)" << std::endl;
    }
    if (flags & DSBLIT_SRC_MASK_ALPHA) {
        ss << "\tDSBLIT_SRC_MASK_ALPHA: modulate source alpha channel with alpha channel from source mask, see also IDirectFBSurface::SetSourceMask()" << std::endl;
    }
    if (flags & DSBLIT_SRC_MASK_COLOR) {
        ss << "\tDSBLIT_SRC_MASK_COLOR: modulate source color channels with color channels from source mask, see also IDirectFBSurface::SetSourceMask()" << std::endl;
    }

    std::cerr << "Blitting Flags are: " << ss.str();
}

void
DirectFBDevice::printSurfaceDrawingFlags(DFBSurfaceDrawingFlags flags)
{
    // GNASH_REPORT_FUNCTION;
    std::stringstream ss;
    
    if (flags == DSDRAW_NOFX) {
        ss << "\tDSDRAW_NOFX: uses none of the effects";
    }
    if (flags & DSDRAW_BLEND) {
        ss << "\tDSDRAW_BLEND: uses alpha from color";
    }
    if (flags & DSDRAW_DST_COLORKEY) {
        ss << "\tDSDRAW_DST_COLORKEY: write to destination only if the destination pixel matches the destination color key";
    }
    if (flags & DSDRAW_SRC_PREMULTIPLY) {
        ss << "\tDSDRAW_SRC_PREMULTIPLY: multiplies the color's rgb channels by the alpha channel before drawing";
    }
    if (flags & DSDRAW_DST_PREMULTIPLY) {
        ss << "\tDSDRAW_DST_PREMULTIPLYmodulates the dest. color with the dest. alpha";
    }
    if (flags & DSDRAW_DEMULTIPLY) {
        ss << "\tDSDRAW_DEMULTIPLYdivides the color by the alpha before writing the data to the destination";
    }
    if (flags & DSDRAW_XOR) {
        ss << "\tDSDRAW_XOR: bitwise xor the destination pixels with the specified color after premultiplication";
    }

    std::cerr << "Drawing Flags are: " << ss.str() << std::endl;

}

void
DirectFBDevice::printGrapbicsDriverIndo(DFBGraphicsDriverInfo *driver)
{
    // GNASH_REPORT_FUNCTION;
    std::stringstream ss;
    
    if (driver) {
        ss << "Version: " << driver->major << ":" << driver->minor << std::endl;
        ss << "Driver Name: " << driver->name << std::endl
           << "Driver Vendor: " << driver->vendor;
    }

    std::cerr << "Graphics Driver Info: " << ss.str() << std::endl;
}

/// print the data in a DirectFB
void
DirectFBDevice::printDirectFB(IDirectFB *fb)
{
    // GNASH_REPORT_FUNCTION;

    DFBGraphicsDeviceDescription desc;

    if (fb) {
        std::cerr << "DirectFB data" << std::endl;

        _dfb->GetDeviceDescription(fb, &desc);
#if 0
        // Use the values in the GraphicsDriver Info instead
        if (desc.name) {
            std::cerr << "\tName: " << desc.name << std::endl;
        }
        if (desc.vendor) {
            std::cerr << "\tVendor: " << desc.vendor << std::endl;
        }
#endif
        if (desc.video_memory) {
            std::cerr << "\tVideo Memory: " << std::hex << desc.video_memory
                      << std::dec << std::endl;
        }

        printGrapbicsDriverIndo(&desc.driver);
        printAccelerationMask(desc.acceleration_mask);
        printSurfaceBlittingFlags(desc.blitting_flags);
        printSurfaceDrawingFlags(desc.drawing_flags);

    } else {
        log_error("iDirectFB data not set!");
    }
}

/// print the data in a DirectFBSurface
void
DirectFBDevice::printFBSurface(IDirectFBSurface *surface)
{
    if (surface) {
        // DFBSurfaceDescriptionFlags 
        // printSurfaceDescriptionFlags(desc.flags);

	int x, y;
        std::cerr << "FBSurface:" << std::endl;
        surface->GetSize(surface, &x, &y);
        std::cerr << "\tWidth: " << x << ", Height: "<< y << std::endl;
        // ss << "Surface ID: " << surface->resource_id << std::end;
        
        // Get the Capabilities of the Surface
        DFBSurfaceCapabilities caps;
        surface->GetCapabilities(surface, &caps);
        printSurfaceCapabilities(caps);

        // Get the Pixel Format
        DFBSurfacePixelFormat format;
        surface->GetPixelFormat(surface, &format);
        // printSurfacePixelFormat(format); // too verbose

	DFBAccelerationMask mask;
        surface->GetAccelerationMask(_surface, _surface, &mask);
        printAccelerationMask(mask);

        // printFBColor(surface->entries);
        // printFBSurfaceHintFlags(surface->hints);
        return;
    } else {
        std::cerr << "ERROR: No surface data!" << std::endl;
    }
}

void
DirectFBDevice::printSurfaceDescriptionFlags(DFBSurfaceDescriptionFlags flags)
{
    std::stringstream ss;

    if (flags == DSDESC_NONE) {
        ss << "\tDSDESC_NONE: none of these" << std::endl;
    }
    if (flags & DSDESC_CAPS) {
        ss << "\tDSDESC_CAPS: caps field is valid" << std::endl;
    }
    if (flags & DSDESC_WIDTH) {
        ss << "\tDSDESC_WIDTH: width field is valid" << std::endl;
    }
    if (flags & DSDESC_HEIGHT) {
        ss << "\tDSDESC_HEIGHT: height field is valid" << std::endl;
    }
    if (flags & DSDESC_PIXELFORMAT) {
        ss << "\tDSDESC_PIXELFORMAT: pixelformat field is valid" << std::endl;
    }
    if (flags & DSDESC_PREALLOCATED) {
        ss << "\tDSDESC_PREALLOCATED: Surface uses data that has been preallocated by the application" << std::endl;
    }
    if (flags & DSDESC_PALETTE) {
        ss << "\tDSDESC_PALETTE: Initialize the surfaces palette with the entries specified in the description" << std::endl;
    }
    if (flags & DSDESC_RESOURCE_ID) {
        ss << "\tDSDESC_RESOURCE_ID: user defined resource id for general purpose surfaces is specified, or resource id of window, layer, user is returned" << std::endl;
    }
#ifdef DSDESC_HINTS
    if (flags & DSDESC_HINTS) {
        ss << "\tDSDESC_HINTS: Flags for optimized allocation and pixel format selection are set. See also DFBSurfaceHintFlags" << std::endl;
    }
#endif

    if (flags & DSDESC_ALL) {
        ss << "\tDSDESC_ALL: all of these" << std::endl;
    }

    std::cerr << "Surfce Description FLags: " << ss.str();
}

void
DirectFBDevice::printSurfaceCapabilities(DFBSurfaceCapabilities caps)
{
    std::stringstream ss;

    if (caps == DSCAPS_NONE) {
        ss << "\tDSCAPS_NONE: None of these" << std::endl;
    }
    if (caps & DSCAPS_PRIMARY) {
        ss << "\tDSCAPS_PRIMARY: It's the primary surface" << std::endl;
    }
    if (caps & DSCAPS_SYSTEMONLY) {
        ss << "\tDSCAPS_SYSTEMONLY: Surface data is permanently stored in system memory." << std::endl;
        ss << "\t\tThere's no video memory allocation/storage" << std::endl;
    }
    if (caps & DSCAPS_VIDEOONLY) {
        ss << "\tDSCAPS_VIDEOONLY: Surface data is permanently stored in video memory. " << std::endl;
        ss << "\t\tThere's no system memory allocation/storage." << std::endl;
    }
    if (caps & DSCAPS_DOUBLE) {
        ss << "\tDSCAPS_DOUBLE: Surface is double buffered" << std::endl;
    }
    if (caps & DSCAPS_SUBSURFACE) {
        ss << "\tDSCAPS_SUBSURFACE: Surface is just a sub area of another one sharing the surface data." << std::endl;
    }
    if (caps & DSCAPS_INTERLACED) {
        ss << "\tDSCAPS_INTERLACED: Each buffer contains interlaced video (or graphics)" << std::endl
           << "\tdata consisting of two fields. Their lines are stored interleaved. One" << std::endl
           << "\tfield's height is a half of the surface's height." << std::endl;
    }
    if (caps & DSCAPS_SEPARATED) {
        ss << "\tDSCAPS_SEPARATED: For usage with DSCAPS_INTERLACED." << std::endl;
    }
    if (caps & DSCAPS_SEPARATED) {
        ss << "\tDSCAPS_SEPARATED: specifies that the fields are NOT interleaved line" << std::endl
           << "\tby line in the buffer. The first field is followed by the second one." << std::endl;
    }
    if (caps & DSCAPS_STATIC_ALLOC) {
        ss << "\tDSCAPS_STATIC_ALLOC: The amount of video or system memory allocated for " << std::endl
           << "the surface is never less than its initial value. This way a surface can" << std::endl
           << "be resized (smaller and bigger up to the initial size) without reallocation" << std::endl
           << "of the buffers. It's useful for surfaces that need a guaranteed space in" << std::endl
           << "video memory after resizing." << std::endl;
    }
    if (caps & DSCAPS_TRIPLE) {
        ss << "\tDSCAPS_TRIPLE: Surface is triple buffered." << std::endl;
    }
    if (caps & DSCAPS_PREMULTIPLIED) {
        ss << "\t: Surface stores data with premultiplied alpha." << std::endl;
    }
    if (caps & DSCAPS_DEPTH) {
        ss << "\tDSCAPS_DEPTH: A depth buffer is allocated." << std::endl;
    }
    if (caps & DSCAPS_SHARED) {
        ss << "\tDSCAPS_SHARED: The surface will be accessible among processes." << std::endl;
    }
#ifdef DSCAPS_ROTATED
    if (caps & DSCAPS_ROTATED) {
        ss << "\tDSCAPS_ROTATED: The back buffers are allocated with swapped width/height (unimplemented!)" << std::endl;
    }
#endif
    if (caps & DSCAPS_ALL) {
        ss << "\tDSCAPS_ALL: All of these." << std::endl;
    }
    if (caps & DSCAPS_FLIPPING) {
        ss << "\tDSCAPS_FLIPPING: Surface needs Flip() calls to make updates/changes visible/usable: ";
    }

    //     DSCAPS_DOUBLE | DSCAPS_TRIPLE 
    std::cerr << "Surface Capabilities: " << std::endl << ss.str() << std::endl;
}

void
DirectFBDevice::printSurfacePixelFormat(DFBSurfacePixelFormat format)
{
    std::stringstream ss;

    if (format == DSPF_UNKNOWN) {
        ss << "\tDSPF_UNKNOWN: unknown or unspecified format" << std::endl;
    }
    if (format & DSPF_ARGB1555) {
        ss << "\tDSPF_ARGB1555: 16 bit ARGB (2 byte, alpha 1@15, red 5@10, green 5@5, blue 5@0)" << std::endl;
    }
    if (format & DSPF_RGB16) {
        ss << "\tDSPF_RGB1: 16 bit RGB (2 byte, red 5@11, green 6@5, blue 5@0)" << std::endl;
    }
    if (format & DSPF_RGB24) {
        ss << "\tDSPF_RGB24: 24 bit RGB (3 byte, red 8@16, green 8@8, blue 8@0)" << std::endl;
    }
    if (format & DSPF_RGB32) {
        ss << "\tDSPF_RGB32: 24 bit RGB (4 byte, nothing@24, red 8@16, green 8@8, blue 8@0)" << std::endl;
    }
    if (format & DSPF_ARGB) {
        ss << "\tDSPF_ARGB: 32 bit ARGB (4 byte, alpha 8@24, red 8@16, green 8@8, blue 8@0)" << std::endl;
    }
    if (format & DSPF_A8) {
        ss << "\tDSPF_A8: 8 bit alpha (1 byte, alpha 8@0), e.g. anti-aliased glyphs" << std::endl;
    }
    if (format & DSPF_YUY2) {
        ss << "\tDSPF_YUY2: 16 bit YUV (4 byte/ 2 pixel, macropixel contains CbYCrY [31:0])" << std::endl;
    }
    if (format & DSPF_RGB332) {
        ss << "\tDSPF_RGB332: 8 bit RGB (1 byte, red 3@5, green 3@2, blue 2@0)" << std::endl;
    }
    if (format & DSPF_UYVY) {
        ss << "\tDSPF_UYVY: 16 bit YUV (4 byte/ 2 pixel, macropixel contains YCbYCr [31:0])" << std::endl;
    }
    if (format & DSPF_I420) {
        ss << "\tDSPF_I420: 12 bit YUV (8 bit Y plane followed by 8 bit quarter size U/V planes)" << std::endl;
    }
    if (format & DSPF_YV12) {
        ss << "\tDSPF_YV12: 12 bit YUV (8 bit Y plane followed by 8 bit quarter size V/U planes)" << std::endl;
    }
    if (format & DSPF_LUT8) {
        ss << "\tDSPF_LUT8: 8 bit LUT (8 bit color and alpha lookup from palette)" << std::endl;
    }
    if (format & DSPF_ALUT44) {
        ss << "\tDSPF_ALUT44: 8 bit ALUT (1 byte, alpha 4@4, color lookup 4@0)" << std::endl;
    }
    if (format & DSPF_AiRGB) {
        ss << "\tDSPF_AiRGB: 32 bit ARGB (4 byte, inv. alpha 8@24, red 8@16, green 8@8, blue 8@0)" << std::endl;
    }
    if (format & DSPF_A1) {
        ss << "\tDSPF_A1: 1 bit alpha (1 byte/ 8 pixel, most significant bit used first)" << std::endl;
    }
    if (format & DSPF_NV12) {
        ss << "\tDSPF_NV12: 12 bit YUV (8 bit Y plane followed by one 16 bit quarter size Cb|Cr [7:0|7:0] plane)" << std::endl;
    }
    if (format & DSPF_NV16) {
        ss << "\tDSPF_NV16: 16 bit YUV (8 bit Y plane followed by one 16 bit half width Cb|Cr [7:0|7:0] plane)" << std::endl;
    }
    if (format & DSPF_ARGB2554) {
        ss << "\tDSPF_ARGB2554: 16 bit ARGB (2 byte, alpha 2@14, red 5@9, green 5@4, blue 4@0)" << std::endl;
    }
    if (format & DSPF_ARGB4444) {
        ss << "\tDSPF_ARGB4444: 16 bit ARGB (2 byte, alpha 4@12, red 4@8, green 4@4, blue 4@0)" << std::endl;
    }
#ifdef DSPF_RGBA4444
    if (format & DSPF_RGBA4444) {
        ss << "\tDSPF_RGBA4444: 16 bit RGBA (2 byte, red 4@12, green 4@8, blue 4@4, alpha 4@0)" << std::endl;
    }
#endif
    if (format & DSPF_NV21) {
        ss << "\tDSPF_NV21: 12 bit YUV (8 bit Y plane followed by one 16 bit quarter size Cr|Cb [7:0|7:0] plane)" << std::endl;
    }
    if (format & DSPF_AYUV) {
        ss << "\t3DSPF_AYUV: 2 bit AYUV (4 byte, alpha 8@24, Y 8@16, Cb 8@8, Cr 8@0)" << std::endl;
    }
    if (format & DSPF_A4) {
        ss << "\tDSPF_A4: 4 bit alpha (1 byte/ 2 pixel, more significant nibble used first)" << std::endl;
    }
    if (format & DSPF_ARGB1666) {
        ss << "\tDSPF_ARGB1666: 1 bit alpha (3 byte/ alpha 1@18, red 6@12, green 6@6, blue 6@0)" << std::endl;
    }
    if (format & DSPF_ARGB6666) {
        ss << "\tDSPF_ARGB6666: 6 bit alpha (3 byte/ alpha 6@18, red 6@12, green 6@6, blue 6@0)" << std::endl;
    }
    if (format & DSPF_RGB18) {
        ss << "\tDSPF_RGB18: 6 bit RGB (3 byte/ red 6@12, green 6@6, blue 6@0)" << std::endl;
    }
    if (format & DSPF_LUT2) {
        ss << "\tDSPF_LUT2: 2 bit LUT (1 byte/ 4 pixel, 2 bit color and alpha lookup from palette)" << std::endl;
    }
    if (format & DSPF_RGB444) {
        ss << "\tDSPF_RGB444: 16 bit RGB (2 byte, nothing @12, red 4@8, green 4@4, blue 4@0)" << std::endl;
    }
    if (format & DSPF_RGB555) {
        ss << "\tDSPF_RGB555: 16 bit RGB (2 byte, nothing @15, red 5@10, green 5@5, blue 5@0)" << std::endl;
    }
    if (format & DSPF_BGR555) {
        ss << "\tDSPF_BGR555: 16 bit BGR (2 byte, nothing @15, blue 5@10, green 5@5, red 5@0)" << std::endl;
    }
#ifdef DSPF_RGBA5551
    if (format & DSPF_RGBA5551) {
        ss << "\tDSPF_RGBA5551: 16 bit RGBA (2 byte, red 5@11, green 5@6, blue 5@1, alpha 1@0)" << std::endl;
    }
#endif
#ifdef DSPF_YUV444P
    if (format & DSPF_YUV444P) {
        ss << "\tDSPF_YUV444P: 24 bit full YUV planar (8 bit Y plane followed by an 8 bit Cb and an 8 bit Cr plane)" << std::endl;
    }
#endif
#ifdef DSPF_ARGB8565
    if (format & DSPF_ARGB8565) {
        ss << "\tDSPF_ARGB8565: 24 bit ARGB (3 byte, alpha 8@16, red 5@11, green 6@5, blue 5@0)" << std::endl;
    }
#endif
#ifdef DSPF_AVYU
    if (format & DSPF_AVYU) {
        ss << "\tDSPF_AVYU: 32 bit AVYU 4:4:4 (4 byte, alpha 8@24, Cr 8@16, Y 8@8, Cb 8@0)" << std::endl;
    }
#endif
#ifdef  DSPF_VYU
    if (format & DSPF_VYU) {
        ss << "\tDSPF_VYU: 24 bit VYU 4:4:4 (3 byte, Cr 8@16, Y 8@8, Cb 8@0)" << std::endl;
    }
#endif
#ifdef DSPF_A1_LSB
    if (format & DSPF_A1_LSB) {
        ss << "\tDSPF_A1_LSB: 1 bit alpha (1 byte/ 8 pixel, LEAST significant bit used first)" << std::endl;
    }
#endif
#ifdef DSPF_YV16
    if (format & DSPF_YV16) {
        ss << "\tDSPF_YV16: 16 bit YUV (8 bit Y plane followed by 8 bit 2x1 subsampled V/U planes" << std::endl;
    }
#endif

    std::cerr << "Pixel Formats supported: " << std::endl << ss.str();
}

void
DirectFBDevice::printColor(DFBColor color)
{
    std::stringstream ss;

    std::cerr << ss.str() << std::endl;
}

#if 0
void
DirectFBDevice::printFBSurfaceHintFlags(DFBSurfaceHintFlags flags)
{
    std::stringstream ss;

    std::cerr << ss.str() << std::endl;
}
#endif

/// print the data in a DirectFBFont
void
DirectFBDevice::printFBFont(IDirectFBFont *font)
{
    std::stringstream ss;
    
    int ivalue;
    const char *cvalue;

    if (font) {
        _font->GetHeight(_font, &ivalue);
        ss << "\tHeight:\t\t" << ivalue << std::endl;

        _font->GetAscender(_font, &ivalue);
        ss << "\tAscender:\t" << ivalue << std::endl;

        _font->GetDescender(_font, &ivalue);
        ss << "\tDescender:\t" << ivalue << std::endl;

        _font->GetMaxAdvance(_font, &ivalue);
        ss << "\tMaxAdvance:\t" << ivalue << std::endl;

    }

    std::cerr << "FBFont: " << std::endl << ss.str() << std::endl;
}

/// print the data in a DirectFBInputDevice
void
DirectFBDevice::printFBInputDevice(IDirectFBInputDevice *input)
{
    std::stringstream ss;
    

    std::cerr << ss.str() << std::endl;
}

} // namespace directfb
} // namespace renderer
} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
