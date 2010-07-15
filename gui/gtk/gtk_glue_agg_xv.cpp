//
//   Copyright (C) 2008, 2009, 2010 Free Software Foundation, Inc.
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



/// \page gtk_shm_support GTK shared memory extension support
/// 
/// Xv glue for AGG.
//
/// Xv does hardware scaling for us.
/// Thus, we should let the renderer render at a scale of 1 and we'll let
/// Xv scale the rendered image to window size. Of course this destroys
/// aspect ratio potentially set by the SWF.
 


#include "gnash.h"
#include "log.h"
#include "Renderer.h"
#include "Renderer_agg.h"
#include "Movie.h"
#include "movie_root.h"
#include "gtk_glue_agg_xv.h"
#include "VideoConverter.h"
#include "VM.h"

#include <cerrno>
#include <gdk/gdkx.h>
#include <sys/ipc.h>
#include <sys/shm.h>

namespace gnash
{

GtkAggXvGlue::GtkAggXvGlue()
  : _agg_renderer(0),
    _stride(0),
    _xv_image(0),
    _xv_image_is_shared(false),
    _xv_port(std::numeric_limits<XvPortID>::max()),
    _xv_max_width(0),
    _xv_max_height(0),
    _window_width(0),
    _window_height(0),
    _movie_width(0),
    _movie_height(0),
    //_mediaHandler(media::MediaHandler::get()),
    _shm_info(0)    
{
    memset(&_xv_format, 0, sizeof(XvImageFormatValues));
}

GtkAggXvGlue::~GtkAggXvGlue()
{
    destroy_x_image();
}

bool
GtkAggXvGlue::init(int /*argc*/, char **/*argv*/[])
{
    int dummy;
    unsigned int p_version, p_release, p_request_base, p_event_base,
                 p_error_base;
      
    if (!XQueryExtension(gdk_display, "XVideo", &dummy, &dummy, &dummy)) {
        log_debug(_("WARNING: No XVideo extension available."));
        return false;
    }
    if (XvQueryExtension(gdk_display, &p_version, &p_release, &p_request_base,
                         &p_event_base, &p_error_base) != Success) {
        log_debug(_("WARNING: XVideo extension is available, but is not currently ready."));
        return false;
    }

    log_debug(_("GTK-AGG: XVideo available (version: %d, release: %d, "
              "request base: %d, event base: %d, error base: %d)"),
              p_version, p_release, p_request_base, p_event_base,
              p_error_base);

    return findXvPort(gdk_display);
}

void
GtkAggXvGlue::prepDrawingArea(GtkWidget *drawing_area)
{
    _drawing_area = drawing_area;

    gtk_widget_set_double_buffered(_drawing_area, FALSE);
}

Renderer*
GtkAggXvGlue::createRenderHandler()
{
    std::string pixelformat = findPixelFormat(_xv_format);
    _agg_renderer = create_Renderer_agg(pixelformat.c_str());
    if (_agg_renderer == NULL) {
        boost::format fmt = boost::format(
            _("Could not create AGG renderer with pixelformat %s")
            ) % pixelformat;
        throw GnashException(fmt.str());
    }

     return _agg_renderer;
}

void
GtkAggXvGlue::setupRendering()
{
    static bool first = true;
    if (first && VM::isInitialized()) {
        first = false;
        
        const Movie& mi = VM::get().getRoot().getRootMovie();
    
        _movie_width = mi.widthPixels();
        _movie_height = mi.heightPixels();        
    
        if (!create_xv_shmimage(_movie_width, _movie_height)) {
            if (!create_xv_image(_movie_width, _movie_height)) {
                log_error(_("GTK-AGG: Could not create the Xv image."));
                first = true;
                return;
            }
        }
        
        if (_xv_format.type == XvRGB) {
            // init renderer to write directly to xv_image
            static_cast<Renderer_agg_base *>(_agg_renderer)->init_buffer
                  ((unsigned char*) _xv_image->data, _xv_image->data_size,
                  _movie_width, _movie_height, _xv_image->pitches[0]);
    
        } else {        
        
    	    int _bpp = 24;
    	    int depth_bytes = _bpp / 8;

            // 4 byte alignment. Gst expects this and ffmpeg doesn't object.
    	    _stride = ( (_movie_width*depth_bytes) + 4 - 1 ) & ~( 4 - 1 );  
    	    
    	    int bufsize = _stride * _movie_height;

    	    _offscreenbuf.reset(new unsigned char[bufsize]);
    	    
    	    Renderer_agg_base * renderer =
    	      static_cast<Renderer_agg_base *>(_agg_renderer);
    	    renderer->init_buffer(_offscreenbuf.get(), bufsize, _movie_width,
    	                          _movie_height, _stride);

        }
    }
}


void 
GtkAggXvGlue::beforeRendering()
{
    setupRendering();
    
    // We force the scale to its original state in case the GUI changed it (in
    // the event of a resize), because we want Xv to do the scaling for us.    
    _agg_renderer->set_scale(1.0, 1.0);
}

void
GtkAggXvGlue::render()
{
    render(0, 0, _movie_width, _movie_height);
}

void
GtkAggXvGlue::render(int /* minx */, int /* miny */, int /* maxx */, int /* maxy */)
{
    if (!_drawing_area || !_xv_image) {
        return;
    }
    
    if (_xv_format.type == XvYUV) {

        boost::uint32_t imgtype = 0;
        media::ImgBuf img(imgtype, _offscreenbuf.get(),
                          _stride*_movie_height,
                           _movie_width, _movie_height);
        img.dealloc = media::ImgBuf::noop;
        img.stride[0] = _stride;

        std::auto_ptr<media::ImgBuf> buf = _video_converter->convert(img);
        if (!buf.get()) {
            log_error(_("RGB->YUV conversion failed."));
            return;
        }
        if ((size_t)_xv_image->data_size != buf->size) {
            log_error(_("Converter returned invalid YUV data size (exp: %d, got %d)"),
                      _xv_image->data_size, buf->size);
            return;
        }
        

        memcpy(_xv_image->data, buf->data, buf->size);
    }

    if (_xv_image_is_shared) {
        XLockDisplay(gdk_display);

        XvShmPutImage(
            gdk_display,
            _xv_port,
            GDK_WINDOW_XWINDOW(_drawing_area->window), 
            GDK_GC_XGC(_drawing_area->style->fg_gc[GTK_STATE_NORMAL]),  // ???
            _xv_image,
            //minx, miny, w, h,
            0, 0, _movie_width, _movie_height, /* source */
            //minx2, miny2, w2, h2,
            0, 0, _window_width, _window_height, /* destination */
            False);            
            
        XSync(gdk_display, False);

        XUnlockDisplay (gdk_display);
    
    } else {
        XvPutImage(
            gdk_display,
             _xv_port,
             GDK_WINDOW_XWINDOW(_drawing_area->window), 
             GDK_GC_XGC(_drawing_area->style->fg_gc[GTK_STATE_NORMAL]),  // ???
             _xv_image,
             //minx, miny, w, h,
             0, 0, _movie_width, _movie_height, /* source */
             //minx2, miny2, w2, h2,
             0, 0, _window_width, _window_height /* destination */);
    }

}

void
GtkAggXvGlue::configure(GtkWidget *const /*widget*/, GdkEventConfigure *const event)
{

    _window_width = event->width;
    _window_height = event->height;
}


void
get_max_xv_image(Display *display, XvPortID xv_port,
			     unsigned int *width, unsigned int *height)
{
    XvEncodingInfo * encodings;
    unsigned int num_encodings, idx;

    XvQueryEncodings(display, xv_port, &num_encodings, &encodings);
    if ( encodings ) {
        for ( idx = 0; idx < num_encodings; ++idx ) {
            if (std::equal(encodings[idx].name, encodings[idx].name+8, "XV_IMAGE")) {
                *width  = encodings[idx].width;
                *height = encodings[idx].height;
                break;
            }
        }
    }
    log_debug("GTK-AGG: Maximum XVideo dimensions: %ux%u\n", *width, *height );
    XvFreeEncodingInfo( encodings );
}

bool
GtkAggXvGlue::findXvPort(Display* display)
{
    unsigned int num_adaptors;
    XvAdaptorInfo* adaptor_info;

    if ((XvQueryAdaptors(display, DefaultRootWindow(display), &num_adaptors,
                         &adaptor_info) != Success)) {
        log_debug("GTK-AGG: WARNING: No XVideo adapters. Falling back to non-Xv.");
    
        return false;    
    }
  
    log_debug("GTK-AGG: NOTICE: Found %d XVideo adapter(s) on GPU.", num_adaptors);
    
    for (unsigned int i = 0; i < num_adaptors; ++i) {

        const XvAdaptorInfo& adaptor = adaptor_info[i];
        
        if (!((adaptor.type & XvInputMask) &&
              (adaptor.type & XvImageMask))) {
            continue;
        }
        
        for (XvPortID port = adaptor.base_id;
             port < adaptor.base_id + adaptor.num_ports;
             ++port) {
             
             int num_formats;
             XvImageFormatValues* formats = XvListImageFormats(display, port, &num_formats);

             for (int j=0; j < num_formats; j++) {
                 const XvImageFormatValues& format = formats[j];
                 
                 if (!isFormatBetter(_xv_format, format)) {
                     continue;
                 }

                 std::string agg_pixelformat = findPixelFormat(format);
                 if (agg_pixelformat.empty()) {
                     continue;
                 }
                 
                 if (!ensurePortGrabbed(display, port)) {
                     continue;
                 }
                 
                 _xv_format = format;
             }
             
             XFree(formats);             
        }
    }
    
    XvFreeAdaptorInfo(adaptor_info);    

    if (_xv_port != std::numeric_limits<XvPortID>::max()) {
        const char fourcc[] = {(_xv_format.id & 0xFF),
                               (_xv_format.id >> 8)  & 0xFF,
                               (_xv_format.id >> 16) & 0xFF, 
                               (_xv_format.id >> 24) & 0xFF, 0};
        log_debug(_("GTK-AGG: Selected format %s for Xv rendering."), fourcc);
        get_max_xv_image(display, _xv_port, &_xv_max_width, &_xv_max_height);
    }
    
    return _xv_format.id != 0;
}

std::string
GtkAggXvGlue::findPixelFormat(const XvImageFormatValues& format)
{
    std::string rv;

    if ((format.type == XvRGB) && (format.format == XvPacked)) {

        unsigned int red_shift, red_prec;
        unsigned int green_shift, green_prec;
        unsigned int blue_shift, blue_prec;

        decode_mask(format.red_mask,   &red_shift,   &red_prec);
        decode_mask(format.green_mask, &green_shift, &green_prec);
        decode_mask(format.blue_mask,  &blue_shift,  &blue_prec);

        const char *pixelformat = agg_detect_pixel_format(
            red_shift, red_prec,
            green_shift, green_prec,
            blue_shift, blue_prec,
            format.bits_per_pixel);

        if (!pixelformat) {
            log_debug("GTK-AGG: Unknown RGB format "
                    "'%d:%d:%d:%x:%x:%x' reported by Xv."
                    "  Please report this to the gnash-dev "
                    "mailing list.", format.id,
                    format.bits_per_pixel, format.depth,
                    format.red_mask, format.green_mask,
                    format.blue_mask);
            return rv;
        }

        rv = pixelformat;

    } else {
        std::auto_ptr<media::VideoConverter> converter =
            _mediaHandler->createVideoConverter(0x32424752 /* RGB */, format.id);

        if (!converter.get()) {
            return rv;
        }

        _video_converter = converter;
        
        rv = "RGB24";
    }
    
    return rv;
}

bool
GtkAggXvGlue::ensurePortGrabbed(Display *display, XvPortID port)
{
    if (port == _xv_port) {
        return true;
    }
    
    if (!grabXvPort(display, port)) {
        return false;
    }

    if (_xv_port != std::numeric_limits<XvPortID>::max()) {
        XvUngrabPort(display, _xv_port, CurrentTime);
    }
    
    _xv_port = port;
    
    return true;;
}

bool
GtkAggXvGlue::isFormatBetter(const XvImageFormatValues& oldformat,
                             const XvImageFormatValues& newformat)
{
    if ( (newformat.type == XvRGB) ) {
        if (oldformat.type != XvRGB) {
            return true;
        }
        
        return (newformat.depth > oldformat.depth);
    }

    return newformat.bits_per_pixel > oldformat.bits_per_pixel;
}

void 
GtkAggXvGlue::decode_mask(unsigned long mask, unsigned int *shift, unsigned int *size)
{
    *shift = 0;
    *size = 0;
    
    if (mask==0) return; // invalid mask
    
    while (!(mask & 1)) {
        (*shift)++;
        mask = mask >> 1;
    }
    
    while (mask & 1) {
        (*size)++;
        mask = mask >> 1;
    }
}


bool
GtkAggXvGlue::grabXvPort(Display *display, XvPortID port) 
{
    int status;

    if ((status = XvGrabPort(display, port, CurrentTime)) == Success){
        log_debug("GTK-AGG: XVideo successfully grabbed port %ld.",
                (ptrdiff_t)port);
        return true;
    } else {
        const char* reason;
        switch (status) {
        case XvInvalidTime:
            reason = "XvInvalidTime";
            break;
        case XvAlreadyGrabbed:
            reason = "XvAlreadyGrabbed";
            break;
        case XvBadExtension:
            reason = "XvBadExtension";
            break;
        case XvBadAlloc:
            reason = "XvBadAlloc";
            break;
        default:
            reason = "Unknown";
        }
        log_debug("GTK-AGG: WARNING: Unable to XvGrabPort(%ld).  Error: %s", 
                 (unsigned long)port, reason);
    }
    return false;
}

bool
GtkAggXvGlue::create_xv_image(unsigned int width, unsigned int height)
{
    destroy_x_image();
        
    _xv_image = XvCreateImage (gdk_display, _xv_port, 
                               _xv_format.id,
                               NULL, width, height);
    if (!_xv_image) {
        printf("GTK-AGG: XvCreateImage failed!");
        return false;
    }
    if ((_xv_image->width < (int)width) || (_xv_image->height < (int)height)) {
        log_debug("GTK-AGG: xv_image => XVideo requested %dx%d, got %dx%d.  Aborting.\n",
                width, height, _xv_image->width, _xv_image->height);
        destroy_x_image();
        return false;
    }
    _xv_image->data = (char*)malloc(_xv_image->data_size);
    if (!_xv_image->data) {
        printf("GTK-AGG: Could not allocate %i bytes for Xv buffer: %s\n",
                _xv_image->data_size, strerror(errno));
        return false;
    }
    memset(_xv_image->data, 0, _xv_image->data_size);
    _xv_image_is_shared = false;
    
    log_debug(_("GTK-AGG: Created non-shared XvImage %dx%d@%#x, data=%#x, %d bytes, %d planes."), 
              width, height, (ptrdiff_t)_xv_image, (ptrdiff_t)_xv_image->data, 
              _xv_image->data_size, _xv_image->num_planes);
    return true;
}

bool
GtkAggXvGlue::create_xv_shmimage(unsigned int width, unsigned int height)
{
    // First, we make sure that we can actually use XShm. Gdk seems to do a
    // better job at detecting this than we traditionally have, so we'll just
    // try to create small shared GdkImage.

    assert(_drawing_area && _drawing_area->window);
    
    GdkVisual* wvisual = gdk_drawable_get_visual(_drawing_area->window);

    GdkImage* tmpimage = gdk_image_new (GDK_IMAGE_SHARED, wvisual, 1, 1);
    
    if (tmpimage) {
        gdk_image_destroy(tmpimage);
    } else {
        log_debug(_("GTK-AGG: XShm not supported; will use non-shared memory."));
        return false;
    }

    // destroy any already existing structures
    destroy_x_image();

    // prepare segment info (populated by XvShmCreateImage)
    _shm_info = (XShmSegmentInfo*) malloc(sizeof(XShmSegmentInfo));  
    assert(_shm_info != NULL);

    // create shared memory XImage
    if ((width > _xv_max_width) || (height > _xv_max_height)) {
        log_debug("GTK-AGG: xv_shmimage => %dx%d too big for XVideo", width, height);
        return false;
    }
    _xv_image = XvShmCreateImage(gdk_display, _xv_port, 
                                 _xv_format.id,
                                 NULL, width, height, _shm_info);
    if (!_xv_image) {
        printf("GTK-AGG: XvShmCreateImage failed!");
        return false;
    }
    if ((_xv_image->width < (int)width) || (_xv_image->height < (int)height)) {
        log_debug("GTK-AGG: xv_shmimage => XVideo requested %dx%d, got %dx%d.  Aborting.\n",
                width, height, _xv_image->width, _xv_image->height);
        destroy_x_image();
        return false;
    }
    _xv_image_is_shared = true;

    // create shared memory segment
    _shm_info->shmid = shmget(IPC_PRIVATE, 
                            _xv_image->data_size,
                            IPC_CREAT|0777);
    if (_shm_info->shmid == -1) {
        printf("GTK-AGG: xv_shmimage => Failed requesting Xv shared memory segment "
                    "(%s). Perhaps the "
                    "required memory size is bigger than the limit set by the kernel.",
                    strerror(errno));
        destroy_x_image();
        return false;
    }

    // attach the shared memory segment to our process
    _shm_info->shmaddr = _xv_image->data = (char*) shmat(_shm_info->shmid, NULL, 0);
    
    if (_shm_info->shmaddr == (char*)-1) {
        printf("GTK-AGG: xv_shmimage => Failed attaching to Xv shared memory segment: %s",
                    strerror(errno));
        destroy_x_image();
        return false;
    }
    
    // clear memory
    memset(_xv_image->data, 0, _xv_image->data_size);
    
    log_debug("GTK-AGG: Created shared XvImage %dx%d@%#x, data=%#x, %d bytes.", 
              width, height, (ptrdiff_t)_xv_image, (ptrdiff_t)_xv_image->data, 
              _xv_image->data_size);

    // Give the server full access to our memory segment. We just follow
    // the documentation which recommends this, but we could also give him
    // just read-only access since we don't need XShmGetImage...
    _shm_info->readOnly = False;
    
    // Finally, tell the server to attach to our shared memory segment  
    if (!XShmAttach(gdk_display, _shm_info)) {
        printf("GTK-AGG: xv_shmimage => Server failed attaching to the shared "
                    "memory segment");
        destroy_x_image();
        return false;
    }

    XSync(gdk_display, False);

    // mark segment for automatic destruction after last process detaches
    shmctl(_shm_info->shmid, IPC_RMID, 0);

    return true;
}

void 
GtkAggXvGlue::destroy_x_image()
{
    if (_xv_image) {
        log_debug("GTK-AGG: destroy => Using XFree (XVideo) to dispose of "
                  "shared memory (%#x,%#x).", (ptrdiff_t)_xv_image, (ptrdiff_t)_xv_image->data);
        if (_xv_image->data != NULL) {
            if (_xv_image_is_shared) {
	            shmdt(_xv_image->data);
	        } else {
                XFree(_xv_image->data);
	        }
            XFree(_xv_image);
        }
        _xv_image = NULL;
        _xv_image_is_shared = false;
    }

    if (_shm_info) {
        // TODO: call shmdt?
        free(_shm_info);
        _shm_info=NULL;
    }
}


} // namespace gnash

