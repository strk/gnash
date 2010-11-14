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

DirectFBDevice::DirectFBDevice()
{
    GNASH_REPORT_FUNCTION;
}

DirectFBDevice::~DirectFBDevice()
{
    GNASH_REPORT_FUNCTION;

    if (_keybuffer) {
        _keybuffer->Release(_keybuffer.get());
    }
    if (_surface) {
        _surface->Release(_surface.get());
    }
    if (_provider) {
        _provider->Release(_provider.get());
    }
    if (_font) {
        _font->Release(_font.get());
    }
    if (_id) {
        _id->Release(_id.get());
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
#endif

//    log_debug("Gnash DirectFB Frame width %d height %d bpp %d \n", _width, _height, _bpp);
    
    return true;
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
        ss << "uses none of the effects";
    }
    if (flags & DSDRAW_BLEND) {
        ss << "uses alpha from color";
    }
    if (flags & DSDRAW_DST_COLORKEY) {
        ss << "write to destination only if the destination pixel matches the destination color key";
    }
    if (flags & DSDRAW_SRC_PREMULTIPLY) {
        ss << "multiplies the color's rgb channels by the alpha channel before drawing";
    }
    if (flags & DSDRAW_DST_PREMULTIPLY) {
        ss << "modulates the dest. color with the dest. alpha";
    }
    if (flags & DSDRAW_DEMULTIPLY) {
        ss << "divides the color by the alpha before writing the data to the destination";
    }
    if (flags & DSDRAW_XOR) {
        ss << "bitwise xor the destination pixels with the specified color after premultiplication";
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
        _dfb->GetDeviceDescription(fb, &desc);

        if (desc.name) {
            std::cerr << "Name: " << desc.name << std::endl;
        }
        if (desc.vendor) {
            std::cerr << "Vendor: " << desc.vendor << std::endl;
        }
        printAccelerationMask(desc.acceleration_mask);
        printSurfaceBlittingFlags(desc.blitting_flags);
        printSurfaceDrawingFlags(desc.drawing_flags);
        printGrapbicsDriverIndo(&desc.driver);
    } else {
        log_error("iDirectFB data not set!");
    }
}

/// print the data in a DirectFBSurface
void
DirectFBDevice::printFBSurface(IDirectFBSurface *surface)
{
    
}

 /// print the data in a DirectFBFont
void
DirectFBDevice::printFBFont(IDirectFBFont *font)
{
}

/// print the data in a DirectFBInputDevice
void
DirectFBDevice::printFBInputDevice(IDirectFBInputDevice *input)
{
}

} // namespace directfb
} // namespace renderer
} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
