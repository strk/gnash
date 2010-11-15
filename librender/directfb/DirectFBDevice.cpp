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

// #ifdef HAVE_DIRECTFB_H
# include <directfb/directfb.h>
// #else
// # error "This file needs DIRECTFB"
// #endif

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

    // // FIXME: for now, always run verbose till this supports command line args
    dbglogfile.setVerbosity();
    DFBResult result;

    if ((result = DirectFBInit(&argc, &argv)) != DR_OK) {
	log_error("DirectFBInit(): %s", getErrorString(result));
        return false;
    }

    if ((result = DirectFBCreate(&_dfb)) != DR_OK) {
	log_error("DirectFBCreate(): %s", getErrorString(result));
        return false;
    }

#if 0
    // get an interface to the primary keyboard and create an
    // input buffer for it
   _ dfb->GetInputDevice(_dfb, DIDID_KEYBOARD, &_keyboard);
    keyboard->CreateEventBuffer(_keyboard, &_keybuffer);

    // get an interface to the primary keyboard and create an
    // input buffer for it
    DFBCHECK(dfb->GetInputDevice( dfb, DIDID_KEYBOARD, &keyboard ));
    DFBCHECK(keyboard->CreateEventBuffer( keyboard, &keybuffer ));
    
    // set our cooperative level to DFSCL_FULLSCREEN for exclusive access to
    // the primary layer
    dfb->SetCooperativeLevel( dfb, DFSCL_FULLSCREEN );
#endif
    
    DFBSurfaceDescription dsc;

    // get the primary surface, i.e. the surface of the primary layer we have
    // exclusive access to
    dsc.flags = DSDESC_CAPS;
    dsc.caps = static_cast<DFBSurfaceCapabilities>(DSCAPS_PRIMARY | DSCAPS_DOUBLE | DSCAPS_VIDEOONLY);   
    
    if ((result = _dfb->CreateSurface(_dfb, &dsc, &_surface)) != DR_OK) {
	log_error("CreateSurface(): %s", getErrorString(result));
        return false;
    }

    int x, y;
    _surface->GetSize(_surface, &x, &y);
    
    DFBFontDescription fdesc;
    fdesc.flags = DFDESC_HEIGHT;
    fdesc.height = y/10;
    
    if ((result == _dfb->CreateFont(_dfb, FONT, &fdesc, &_font)) != DR_OK) {
	log_error("CreateFont(): %s", getErrorString(result));
    }
    _surface->SetFont(_surface, _font);

#if 0
    DFBSurfaceDescription sdesc;
    if ((result == _dfb->CreateImageProvider(_dfb, "/home/rob/imgp2300.jpg", &_provider)) != DR_OK) {
	log_error("CreateImageProvider(): %s", getErrorString(result));
    }
    _provider->GetSurfaceDescription(_provider, &sdesc);
#endif

    if ((result == _dfb->GetDisplayLayer(_dfb, DLID_PRIMARY, &_layer)) != DR_OK) {
	log_error("GetDisplayLayer(): %s", getErrorString(result));
    }
    
    if ((result == _layer->GetScreen(_layer, &_screen)) != DR_OK) {
	log_error("GetScreen(): %s", getErrorString(result));
    }
    
    return true;
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
           << "\tcan be drawn to. This may not be provided by layers"  << std::endl
           << "\tthat display realtime data, e.g. from an MPEG decoder chip. Playback" << std::endl
           << "\tcontrol may be provided by an external API." << std::endl;
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
DirectFBDevice::getErrorString(DFBResult code)
{
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
        ss << "\tNone of these";
    }
    if (mask & DFXL_FILLRECTANGLE) {
        ss << "\tFillRectangle() is accelerated";
    }
    if (mask & DFXL_DRAWRECTANGLE) {
        ss << "DrawRectangle() is accelerated";
    }
    if (mask & DFXL_DRAWLINE) {
        ss << "\tDrawLine() is accelerated";
    }
    if (mask & DFXL_FILLTRIANGLE) {
        ss << "\tFillTriangle() is accelerated";
    }
    if (mask & DFXL_BLIT) {
        ss << "\tBlit() and TileBlit() are accelerated";
    }
    if (mask & DFXL_STRETCHBLIT) {
        ss << "\tStretchBlit() is accelerated";
    }
    if (mask & DFXL_TEXTRIANGLES) {
        ss << "\tTextureTriangles() is accelerated";
    }
#ifdef DFXL_BLIT2
    if (mask & DFXL_BLIT2) {
        ss << "\tBatchBlit2() is accelerated";
    }
#endif
    if (mask & DFXL_DRAWSTRING) {
        ss << "\tDrawString() and DrawGlyph() are accelerated";
    }
    if (mask & DFXL_ALL) {
        ss << "\tAll drawing/blitting functions";
    }
    if (mask & DFXL_ALL) {
        ss << "\tAll drawing functions";
    }
    if (mask & DFXL_ALL) {
        ss << "\tAll blitting functions. ";
    }

    std::cerr << "Acceleration Mask is: "
              << ss.str() << std::endl;
}

void
DirectFBDevice::printSurfaceBlittingFlags(DFBSurfaceBlittingFlags flags)
{
    // GNASH_REPORT_FUNCTION;

    std::stringstream ss;
    
    if (flags == DSBLIT_NOFX) {
        ss << "\tuses none of the effects" << std::endl;
    }
    if (flags & DSBLIT_BLEND_ALPHACHANNEL) {
        ss << "\tenables blending and uses alphachannel from source" << std::endl;
    }
    if (flags & DSBLIT_BLEND_COLORALPHA) {
        ss << "\tenables blending and uses alpha value from color" << std::endl;
    }
    if (flags & DSBLIT_COLORIZE) {
        ss << "source color with the color's r/g/b values" << std::endl;
    }
    if (flags & DSBLIT_SRC_COLORKEY) {
        ss << "\tdon't blit pixels matching the source color key" << std::endl;
    }
    if (flags & DSBLIT_DST_COLORKEY) {
        ss << "\twrite to destination only if the destination pi matches the destination color key" << std::endl;
    }
    if (flags & DSBLIT_SRC_PREMULTIPLY) {
        ss << "modulates the source color with the (modulated) source alpha" << std::endl;
    }
    if (flags & DSBLIT_DST_PREMULTIPLY) {
        ss << "\tmodulates the dest. color with the dest. alpha" << std::endl;
    }
    if (flags & DSBLIT_DEMULTIPLY) {
        ss << "\tthe color by the alpha before writing data to the destination" << std::endl;
    }
    if (flags & DSBLIT_DEINTERLACE) {
        ss << "deinterlaces the source during blitting by reading" << std::endl;
    }
    if (flags & DSBLIT_SRC_PREMULTCOLOR) {
        ss << "\tmodulates the source color with the color alpha" << std::endl;
    }
    if (flags & DSBLIT_XOR)  {
        ss << "\tbitwise xor the destination pixels with the source pixels after premultiplication" << std::endl;
    }
#ifdef DSBLIT_INDEX_TRANSLATION
    if (flags & DSBLIT_INDEX_TRANSLATION) {
        ss "\tdo fast indexed to indexed translation, this flag is mutual exclusive with all others" << std::endl;
    }
#endif
    if (flags & DSBLIT_ROTATE180) {
        ss << "\trotate the image by 180 degree" << std::endl;
    }
    if (flags & DSBLIT_COLORKEY_PROTECT) {
        ss << "\tmake sure written pixels don't match color key (internal only ATM)" << std::endl;
    }
    if (flags & DSBLIT_SRC_MASK_ALPHA) {
        ss << "\tmodulate source alpha channel with alpha channel from source mask, see also IDirectFBSurface::SetSourceMask()" << std::endl;
    }
    if (flags & DSBLIT_SRC_MASK_COLOR) {
        ss << "\tmodulate source color channels with color channels from source mask, see also IDirectFBSurface::SetSourceMask()" << std::endl;
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
        std::cerr << "ERROR: No surface data!";
    }
}

void
DirectFBDevice::printSurfaceDescriptionFlags(DFBSurfaceDescriptionFlags flags)
{
    std::stringstream ss;

    if (flags == DSDESC_NONE) {
        ss << "none of these" << std::endl;
    }
    if (flags & DSDESC_CAPS) {
        ss << "caps field is valid" << std::endl;
    }
    if (flags & DSDESC_WIDTH) {
        ss << "width field is valid" << std::endl;
    }
    if (flags & DSDESC_HEIGHT) {
        ss << "height field is valid" << std::endl;
    }
    if (flags & DSDESC_PIXELFORMAT) {
        ss << "pixelformat field is valid" << std::endl;
    }
    if (flags & DSDESC_PREALLOCATED) {
        ss << "Surface uses data that has been preallocated by the application" << std::endl;
    }
    if (flags & DSDESC_PALETTE) {
        ss << "Initialize the surfaces palette with the entries specified in the description" << std::endl;
    }
    if (flags & DSDESC_RESOURCE_ID) {
        ss << "user defined resource id for general purpose surfaces is specified, or resource id of window, layer, user is returned" << std::endl;
    }
#ifdef DSDESC_HINTS
    if (flags & DSDESC_HINTS) {
        ss << "Flags for optimized allocation and pixel format selection are set. See also DFBSurfaceHintFlags" << std::endl;
    }
#endif

    if (flags & DSDESC_ALL) {
        ss << "\tall of these" << std::endl;
    }

    std::cerr << "Surfce Description FLags: " << ss.str();
}

void
DirectFBDevice::printSurfaceCapabilities(DFBSurfaceCapabilities caps)
{
    std::stringstream ss;

    if (caps == DSCAPS_NONE) {
        ss << "\tNone of these" << std::endl;
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
        ss << "\St: urface stores data with premultiplied alpha." << std::endl;
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
        ss << "\tDSCAPS_ALL: Sll of these." << std::endl;
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
        ss << "\tunknown or unspecified format" << std::endl;
    }
    if (format & DSPF_ARGB1555) {
        ss << "\t16 bit ARGB (2 byte, alpha 1@15, red 5@10, green 5@5, blue 5@0)" << std::endl;
    }
    if (format & DSPF_RGB16) {
        ss << "\t16 bit RGB (2 byte, red 5@11, green 6@5, blue 5@0)" << std::endl;
    }
    if (format & DSPF_RGB24) {
        ss << "\t24 bit RGB (3 byte, red 8@16, green 8@8, blue 8@0)" << std::endl;
    }
    if (format & DSPF_RGB32) {
        ss << "\t24 bit RGB (4 byte, nothing@24, red 8@16, green 8@8, blue 8@0)" << std::endl;
    }
    if (format & DSPF_ARGB) {
        ss << "\t32 bit ARGB (4 byte, alpha 8@24, red 8@16, green 8@8, blue 8@0)" << std::endl;
    }
    if (format & DSPF_A8) {
        ss << "\t8 bit alpha (1 byte, alpha 8@0), e.g. anti-aliased glyphs" << std::endl;
    }
    if (format & DSPF_YUY2) {
        ss << "\t16 bit YUV (4 byte/ 2 pixel, macropixel contains CbYCrY [31:0])" << std::endl;
    }
    if (format & DSPF_RGB332) {
        ss << "\t8 bit RGB (1 byte, red 3@5, green 3@2, blue 2@0)" << std::endl;
    }
    if (format & DSPF_UYVY) {
        ss << "\t16 bit YUV (4 byte/ 2 pixel, macropixel contains YCbYCr [31:0])" << std::endl;
    }
    if (format & DSPF_I420) {
        ss << "\t12 bit YUV (8 bit Y plane followed by 8 bit quarter size U/V planes)" << std::endl;
    }
    if (format & DSPF_YV12) {
        ss << "\t12 bit YUV (8 bit Y plane followed by 8 bit quarter size V/U planes)" << std::endl;
    }
    if (format & DSPF_LUT8) {
        ss << "\t8 bit LUT (8 bit color and alpha lookup from palette)" << std::endl;
    }
    if (format & DSPF_ALUT44) {
        ss << "\t8 bit ALUT (1 byte, alpha 4@4, color lookup 4@0)" << std::endl;
    }
    if (format & DSPF_AiRGB) {
        ss << "\t32 bit ARGB (4 byte, inv. alpha 8@24, red 8@16, green 8@8, blue 8@0)" << std::endl;
    }
    if (format & DSPF_A1) {
        ss << "\t1 bit alpha (1 byte/ 8 pixel, most significant bit used first)" << std::endl;
    }
    if (format & DSPF_NV12) {
        ss << "\t12 bit YUV (8 bit Y plane followed by one 16 bit quarter size Cb|Cr [7:0|7:0] plane)" << std::endl;
    }
    if (format & DSPF_NV16) {
        ss << "\t16 bit YUV (8 bit Y plane followed by one 16 bit half width Cb|Cr [7:0|7:0] plane)" << std::endl;
    }
    if (format & DSPF_ARGB2554) {
        ss << "\t16 bit ARGB (2 byte, alpha 2@14, red 5@9, green 5@4, blue 4@0)" << std::endl;
    }
    if (format & DSPF_ARGB4444) {
        ss << "\t16 bit ARGB (2 byte, alpha 4@12, red 4@8, green 4@4, blue 4@0)" << std::endl;
    }
#ifdef DSPF_RGBA4444
    if (format & DSPF_RGBA4444) {
        ss << "\t16 bit RGBA (2 byte, red 4@12, green 4@8, blue 4@4, alpha 4@0)" << std::endl;
    }
#endif
    if (format & DSPF_NV21) {
        ss << "\t12 bit YUV (8 bit Y plane followed by one 16 bit quarter size Cr|Cb [7:0|7:0] plane)" << std::endl;
    }
    if (format & DSPF_AYUV) {
        ss << "\t32 bit AYUV (4 byte, alpha 8@24, Y 8@16, Cb 8@8, Cr 8@0)" << std::endl;
    }
    if (format & DSPF_A4) {
        ss << "\t4 bit alpha (1 byte/ 2 pixel, more significant nibble used first)" << std::endl;
    }
    if (format & DSPF_ARGB1666) {
        ss << "\t1 bit alpha (3 byte/ alpha 1@18, red 6@12, green 6@6, blue 6@0)" << std::endl;
    }
    if (format & DSPF_ARGB6666) {
        ss << "\t6 bit alpha (3 byte/ alpha 6@18, red 6@12, green 6@6, blue 6@0)" << std::endl;
    }
    if (format & DSPF_RGB18) {
        ss << "\t6 bit RGB (3 byte/ red 6@12, green 6@6, blue 6@0)" << std::endl;
    }
    if (format & DSPF_LUT2) {
        ss << "\t2 bit LUT (1 byte/ 4 pixel, 2 bit color and alpha lookup from palette)" << std::endl;
    }
    if (format & DSPF_RGB444) {
        ss << "\t16 bit RGB (2 byte, nothing @12, red 4@8, green 4@4, blue 4@0)" << std::endl;
    }
    if (format & DSPF_RGB555) {
        ss << "\t16 bit RGB (2 byte, nothing @15, red 5@10, green 5@5, blue 5@0)" << std::endl;
    }
    if (format & DSPF_BGR555) {
        ss << "\t16 bit BGR (2 byte, nothing @15, blue 5@10, green 5@5, red 5@0)" << std::endl;
    }
#ifdef DSPF_RGBA5551
    if (format & DSPF_RGBA5551) {
        ss << "\t16 bit RGBA (2 byte, red 5@11, green 5@6, blue 5@1, alpha 1@0)" << std::endl;
    }
#endif
#ifdef DSPF_YUV444P
    if (format & DSPF_YUV444P) {
        ss << "\t24 bit full YUV planar (8 bit Y plane followed by an 8 bit Cb and an 8 bit Cr plane)" << std::endl;
    }
#endif
#ifdef DSPF_ARGB8565
    if (format & DSPF_ARGB8565) {
        ss << "\t24 bit ARGB (3 byte, alpha 8@16, red 5@11, green 6@5, blue 5@0)" << std::endl;
    }
#endif
#ifdef DSPF_AVYU
    if (format & DSPF_AVYU) {
        ss << "\t32 bit AVYU 4:4:4 (4 byte, alpha 8@24, Cr 8@16, Y 8@8, Cb 8@0)" << std::endl;
    }
#endif
#ifdef  DSPF_VYU
    if (format & DSPF_VYU) {
        ss << "\t24 bit VYU 4:4:4 (3 byte, Cr 8@16, Y 8@8, Cb 8@0)" << std::endl;
    }
#endif
#ifdef DSPF_A1_LSB
    if (format & DSPF_A1_LSB) {
        ss << "\t1 bit alpha (1 byte/ 8 pixel, LEAST significant bit used first)" << std::endl;
    }
#endif
#ifdef DSPF_YV16
    if (format & DSPF_YV16) {
        ss << "\t16 bit YUV (8 bit Y plane followed by 8 bit 2x1 subsampled V/U planes" << std::endl;
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
