//
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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
/// The GTK-AGG combination supports the use of the X11 MIT-SHM extension.
/// This extension allows passing image data to the X server in it's native
/// format (defined by the graphics mode). This prevents CPU intensive pixel
/// format conversions for the X server.
///
/// Not all X servers support this extension and it's available for local
/// (not networked) X connections anyway. So the GTK GUI will first *try*
/// to use the extension and on failure provide automatic fallback to standard
/// pixmaps.
///
/// You won't notice this fallback unless you check the log messages (aside
/// from potential performance difference.)
///
/// The macro ENABLE_MIT_SHM must be defined in gtk_glue_agg.h to enable
/// support for the MIT-SHM extension.
///
/// For more information about the extension, have a look at these URLs:
/// http://en.wikipedia.org/wiki/MIT-SHM
/// http://www.xfree86.org/current/mit-shm.html  


// Also worth checking: http://en.wikipedia.org/wiki/X_video_extension

#include <cerrno>
#include <exception>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "gnash.h"
#include "log.h"
#include "render_handler.h"
#include "render_handler_agg.h"
#include "gtk_glue_agg.h"

#ifdef ENABLE_MIT_SHM
#include <X11/Xlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#include <gdk/gdkx.h>
#include <gdk/gdkprivate.h>
#endif


namespace gnash
{

GtkAggGlue::GtkAggGlue() :
    _offscreenbuf(NULL),
    _offscreenbuf_size(0),
    _agg_renderer(NULL),
    _width(0),
    _height(0),
    _bpp(0),
    _have_shm(false)
#ifdef ENABLE_MIT_SHM
  ,_shm_image(NULL)
  ,_shm_info(NULL)
#endif    
{
}

GtkAggGlue::~GtkAggGlue()
{
    destroy_shm_image();
}

bool
GtkAggGlue::init(int /*argc*/, char **/*argv*/[])
{
    gdk_rgb_init();
    
#ifdef ENABLE_MIT_SHM
    _have_shm = check_mit_shm(gdk_display);
#else
    _have_shm = false;
#endif
    
    if (!detect_pixelformat()) {
        // Perhaps this should be logged independently of verbosity. In this
        // case use std::cerr.
        log_error("FATAL: Could not detect the pixel format used by your X server.");
        log_error("Please report this problem to the Gnash developer team.");
        return false;
    }
    
    log_debug("Your X server expects %s pixmap data for standard mode.", _pixelformat);
    
    return true;
}

bool 
#ifdef ENABLE_MIT_SHM
GtkAggGlue::check_mit_shm(Display *display) 
#else
GtkAggGlue::check_mit_shm(void* /*display*/) 
#endif
{
#ifdef ENABLE_MIT_SHM
    int major, minor, dummy;
    Bool pixmaps;
  
    log_debug("Checking support for MIT-SHM...");
  
    if (!XQueryExtension(display, "MIT-SHM", &dummy, &dummy, &dummy)) {
        log_debug("WARNING: No MIT-SHM extension available, using standard XLib "
        "calls (slower)");
        return false;
    }
  
    if (XShmQueryVersion(display, &major, &minor, &pixmaps )!=True) {
        log_debug("WARNING: MIT-SHM not ready (network link?), using standard XLib "
          "calls (slower)");
        return false;
    }
    
    log_debug("NOTICE: MIT-SHM available (version %d.%d)!", major, minor);
    
    return true;
    
#else
    return false; // !ifdef ENABLE_MIT_SHM
#endif
  
}

void 
GtkAggGlue::create_shm_image(
#ifdef ENABLE_MIT_SHM
    unsigned int width, unsigned int height
#else
    unsigned int, unsigned int
#endif
    )
{

    // destroy any already existing structures
    destroy_shm_image();

#ifdef ENABLE_MIT_SHM
    GdkVisual* visual = gdk_drawable_get_visual(_drawing_area->window);
    Visual* xvisual = GDK_VISUAL_XVISUAL(visual); 
  
    // prepare segment info (populated by XShmCreateImage)
    _shm_info = (XShmSegmentInfo*) malloc(sizeof(XShmSegmentInfo));  
    assert(_shm_info != NULL);
  
    // create shared memory XImage
    _shm_image = XShmCreateImage(gdk_display, xvisual, visual->depth, 
                            ZPixmap, NULL, _shm_info, width, height);
    
    if (!_shm_image) {
        log_debug("Failed creating the shared memory XImage!");
        destroy_shm_image();
        return;
    }
  
    // create shared memory segment
    _shm_info->shmid = shmget(IPC_PRIVATE, 
    _shm_image->bytes_per_line * _shm_image->height, IPC_CREAT|0777);
    
    if (_shm_info->shmid == -1) {
        log_debug("Failed requesting shared memory segment (%s). Perhaps the "
          "required memory size is bigger than the limit set by the kernel.",
           std::strerror(errno));
        destroy_shm_image();
        return;
    }
  
      // attach the shared memory segment to our process
    _shm_info->shmaddr = _shm_image->data = (char*) shmat(_shm_info->shmid, 0, 0);
      
    if (_shm_info->shmaddr == (char*) -1) {
        log_debug("Failed attaching to shared memory segment: %s", strerror(errno));
        destroy_shm_image();
        return;
    }
  
    // Give the server full access to our memory segment. We just follow
    // the documentation which recommends this, but we could also give him
    // just read-only access since we don't need XShmGetImage...
    _shm_info->readOnly = False;
  
    // Finally, tell the server to attach to our shared memory segment  
    if (!XShmAttach(gdk_display, _shm_info)) {
        log_debug("Server failed attaching to the shared memory segment");
        destroy_shm_image();
        return;
    }
    // allows below to work. bug fix by cliff (bug #24692)
    XSync(gdk_display, False);

    // mark segment for automatic destruction after last process detaches
    shmctl(_shm_info->shmid, IPC_RMID, 0);
  
    //log_debug("create_shm_image() OK"); // <-- remove this
#endif // ENABLE_MIT_SHM
   
}

void 
GtkAggGlue::destroy_shm_image()
{
#ifdef ENABLE_MIT_SHM
    if (_shm_image) {  
        XDestroyImage(_shm_image);
        _shm_image=NULL;
    }
  
    if (_shm_info) {
        free(_shm_info);
        _shm_info=NULL;
    }

#endif
}

void
GtkAggGlue::prepDrawingArea(GtkWidget *drawing_area)
{
    _drawing_area = drawing_area;

    // Disable double buffering, otherwise gtk tries to update widget
    // contents from its internal offscreen buffer at the end of expose event
    gtk_widget_set_double_buffered(_drawing_area, FALSE);
}

bool 
GtkAggGlue::detect_pixelformat() 
{

  // By definition, GTK/GDK *always* expects RGB24 data for 
  // gdk_draw_rgb_image(). However, we want to support the OLPC which uses a 
  // hacked GTK that works with RGB565 data.
  
#ifdef PIXELFORMAT_RGB24   // normal case 
  
    _bpp = 24;
    _pixelformat = "RGB24";
    return true;

#else

#ifdef PIXELFORMAT_RGB565  // OLPC

#warning A pixel format of RGB565; you must have a (hacked) GTK which supports \
         this format (e.g., GTK on the OLPC).
                  
    _bpp = 16;
    _pixelformat = "RGB565";
    return true;
  
#else

#warning GTK GUI requires --with-pixelformat=RGB24 for AGG renderer
 
    log_error("Missing a supported pixel format for GTK GUI. You probably want to "
              "configure --with-pixelformat=RGB24");
    return false;   

#endif  //ifdef PIXELFORMAT_RGB565   
 
#endif  //ifdef PIXELFORMAT_RGB24
  
  
}

render_handler*
GtkAggGlue::create_shm_handler()
{
#ifdef ENABLE_MIT_SHM
    GdkVisual *visual = gdk_drawable_get_visual(_drawing_area->window);

    // Create a dummy SHM image to detect it's pixel format (we can't use the 
    // info from "visual"). 
    // Is there a better way??

    create_shm_image(256,256);

    if (!_shm_image) return NULL;

    unsigned int red_shift, red_prec;
    unsigned int green_shift, green_prec;
    unsigned int blue_shift, blue_prec;

    decodeMask(_shm_image->red_mask, red_shift, red_prec);
    decodeMask(_shm_image->green_mask, green_shift, green_prec);
    decodeMask(_shm_image->blue_mask, blue_shift, blue_prec);

  
    log_debug("X server pixel format is (R%d:%d, G%d:%d, B%d:%d, %d bpp)",
            red_shift, red_prec,
            green_shift, green_prec,
            blue_shift, blue_prec,
            _shm_image->bits_per_pixel);
  
  
    const char *pixelformat = agg_detect_pixel_format(
            red_shift, red_prec,
            green_shift, green_prec,
            blue_shift, blue_prec,
            _shm_image->bits_per_pixel);

    destroy_shm_image();

    if (!pixelformat) {
        log_debug("Pixel format of X server not recognized!");

        // disable use of shared memory pixmaps
        _have_shm = false;

        return NULL; 
    }

    log_debug("X server is using %s pixel format", pixelformat);

    render_handler* res = create_render_handler_agg(pixelformat);
  
    if (!res) {
        log_debug("Failed creating a renderer instance for this pixel format. "
          "Most probably Gnash has not compiled in (configured) support "
          "for this pixel format - using standard pixmaps instead");
          
        // disable use of shared memory pixmaps
        _have_shm = false;
    }      
  
  
    return res;
    
#else
    return NULL;
#endif
}

render_handler*
GtkAggGlue::createRenderHandler()
{
    // try with MIT-SHM
    if (_have_shm) {
        _agg_renderer = create_shm_handler();
        if (_agg_renderer) return _agg_renderer;
    }

    _agg_renderer = create_render_handler_agg(_pixelformat.c_str());
    return _agg_renderer;
}

void
GtkAggGlue::setRenderHandlerSize(int width, int height)
{
    assert(width > 0);
    assert(height > 0);
    assert(_agg_renderer != NULL);

    static const size_t chunkSize = 100 * 100 * (_bpp / 8);
    
    if (width == _width && height == _height) return;
       
    _width = width;
    _height = height;
       
    // try shared image first
    if (_have_shm) {
        create_shm_image(width, height);
    }
      
#ifdef ENABLE_MIT_SHM
    if (_shm_image) {
    
        // ==> use shared memory image (faster)
        log_debug("GTK-AGG: Using shared memory image");
          
        static_cast<render_handler_agg_base *>(_agg_renderer)->init_buffer(
                (unsigned char*) _shm_info->shmaddr,
                _shm_image->bytes_per_line * _shm_image->height,
                _width,
                _height,
                _shm_image->bytes_per_line
        );
    
    }
    else {
#endif      
  
        // ==> use standard pixmaps (slower, but should work in any case)
        size_t newBufferSize = width * height * ((_bpp + 7) / 8);
          
        // Reallocate the buffer when it shrinks or grows.
        if (newBufferSize != _offscreenbuf_size) {

            newBufferSize = (newBufferSize / chunkSize + 1) * chunkSize;

            try {
                  _offscreenbuf.reset(new unsigned char[newBufferSize]);
                  log_debug("GTK-AGG %i bytes offscreen buffer allocated", newBufferSize);
            }
            catch (std::bad_alloc &e)
            {
                log_error("Could not allocate %i bytes for offscreen buffer: %s",
                      newBufferSize, e.what());
                      
                  // TODO: what to do here? An assertion in render_handler_agg.cpp
                  // fails if we just return.
                  return;
            }
      
            _offscreenbuf_size = newBufferSize;

        }
      
        // Only the AGG renderer has the function init_buffer, which is *not* part of
        // the renderer api. It allows us to change the renderers movie size (and buffer
        // address) during run-time.
        static_cast<render_handler_agg_base *>(_agg_renderer)->init_buffer(
            _offscreenbuf.get(),
            _offscreenbuf_size,
            _width,
            _height,
            _width*((_bpp+7)/8)
        );
      
      
#ifdef ENABLE_MIT_SHM
    }
#endif  
    
}

void 
GtkAggGlue::beforeRendering()
{
#ifdef ENABLE_MIT_SHM
    if (_shm_image) {
        // The shared memory buffer is copied in background(!) since the X 
        // calls are executed asynchroneously. This is dangerous because it
        // may happen that the renderer updates the buffer while the X server
        // still copies the data to the VRAM (flicker can occurr).
        // Instead of using the XShmCompletionEvent for this we just call XSync
        // right before writing to the shared memory again. This will make sure
        // that the X server finishes to copy the data to VRAM before we
        // change it again.
        XSync(gdk_display, False);
    }
#endif  
}

void
GtkAggGlue::render()
{
    if (!_drawing_area) {
        return;
    }


#ifdef ENABLE_MIT_SHM
    if (_shm_image) {
  
        XShmPutImage(
            gdk_display, 
            GDK_WINDOW_XWINDOW(_drawing_area->window), 
            GDK_GC_XGC(_drawing_area->style->fg_gc[GTK_STATE_NORMAL]),  // ???
            _shm_image,
            0, 0,
            0, 0,
            _width, _height,
            False); 
      
      // NOTE: Data will be copied in background, see beforeRendering()
      
    }
    else {
#endif  

        // Update the entire screen
        gdk_draw_rgb_image (
            _drawing_area->window,
            _drawing_area->style->fg_gc[GTK_STATE_NORMAL],
            0,
            0,
            _width,
            _height,
            GDK_RGB_DITHER_NONE,
            _offscreenbuf.get(),
            _width*((_bpp+7)/8)
        );      

#ifdef ENABLE_MIT_SHM
    }
#endif  
    
}

void
GtkAggGlue::render(int minx, int miny, int maxx, int maxy)
{
    if (!_drawing_area) {
        return;
    }

    size_t copy_width = std::min(_width * (_bpp/8), maxx - minx);
    size_t copy_height = std::min(_height, maxy - miny);
    size_t stride = _width*((_bpp+7)/8);

#ifdef ENABLE_MIT_SHM
    if (_shm_image) {
  
    XShmPutImage(
        gdk_display, 
        GDK_WINDOW_XWINDOW(_drawing_area->window), 
        GDK_GC_XGC(_drawing_area->style->fg_gc[GTK_STATE_NORMAL]),  // ???
        _shm_image,
        minx, miny,
        minx, miny,
        copy_width, copy_height,
        False);
      
    // NOTE: Data will be copied in background, see beforeRendering()
 
    }
    else {
#endif
//    log_debug("minx: %d, miny: %d, copy width: %d, copy height: %d, stride: %d",
//                            minx, miny, copy_width, copy_height, stride);
//    log_debug("offscreenbuf size: %d", _offscreenbuf_size);
//    log_debug("From: %d", miny * stride + minx * (_bpp/8));

        // Update only the invalidated rectangle
        gdk_draw_rgb_image (
            _drawing_area->window,
            _drawing_area->style->fg_gc[GTK_STATE_NORMAL],
            minx,
            miny,
            copy_width,
            copy_height,
            GDK_RGB_DITHER_NORMAL,
            _offscreenbuf.get() + miny * stride + minx * (_bpp/8),
            stride
        );

#ifdef ENABLE_MIT_SHM
    }
#endif  
}

void
GtkAggGlue::configure(GtkWidget *const /*widget*/, GdkEventConfigure *const event)
{
    if (_agg_renderer) {
        setRenderHandlerSize(event->width, event->height);
    }
}

#ifdef ENABLE_MIT_SHM
void 
GtkAggGlue::decodeMask(unsigned long mask, unsigned int& shift, unsigned int& size)
{
    shift = 0;
    size = 0;

    if (mask == 0) return; // invalid mask

    while (!(mask & 1)) {
        ++shift;
        mask = mask >> 1;
    }

    while (mask & 1) {
        ++size;
        mask = mask >> 1;
    }
}
#endif

} // namespace gnash

