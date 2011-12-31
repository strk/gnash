//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc.
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
 
#include <cerrno>
#include <exception>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "log.h"
#include "Renderer.h"
#include "Renderer_agg.h"
#include "gtk_glue_agg_vaapi.h"

#include "Movie.h"
#include "movie_root.h"
#include "VM.h"

#include "VaapiDisplayX11.h"
#include "VaapiGlobalContext.h"
#include "VaapiContext.h"
#include "VaapiException.h"
#include "VaapiSurface.h"
#include "VaapiImage.h"
#include "VaapiSubpicture.h"
#include "GnashVaapiImage.h"
#include "GnashVaapiImageProxy.h"
#include "vaapi_utils.h"

#define dprintf(format,...)

// Defined to 1 if the stage will be scaled by the HW and not AGG
// XXX: disabled for now because of generic Gnash rendering problems
#define USE_HW_SCALING 0

namespace gnash
{

static const char *
find_pixel_format(VaapiImageFormat format)
{
    switch (format) {
    case VAAPI_IMAGE_RGBA:
        return "RGBA32";
    case VAAPI_IMAGE_ARGB: return
            "ARGB32";
    case VAAPI_IMAGE_ABGR:
        return "ABGR32";
    case VAAPI_IMAGE_BGRA:
        return "BGRA32";
    default:;
    }
    return NULL;
}

class VaapiVideoWindow : public VaapiContextData {
    GdkWindow          *_window;
    VaapiRectangle      _rect;

public:
    VaapiVideoWindow(GdkWindow *parent_window, VaapiRectangle const & rect);
    ~VaapiVideoWindow();

    /// Move and resize window
    void moveResize(VaapiRectangle const & rect);

    /// Return GDK window XID
    XID xid() const
        { return GDK_DRAWABLE_XID(_window); }

    /// Return GDK window X coordinate
    int x() const
        { return _rect.x; }

    /// Return GDK window Y coordinate
    int y() const
        { return _rect.y; }

    /// Return GDK window width
    unsigned int width() const
        { return _rect.width; }

    /// Return GDK window height
    unsigned int height() const
        { return _rect.height; }
};

VaapiVideoWindow::VaapiVideoWindow(GdkWindow *parent_window, VaapiRectangle const & rect)
{
    GdkWindowAttr wattr;
    wattr.event_mask  = 0;
    wattr.x           = rect.x;
    wattr.y           = rect.y;
    wattr.width       = rect.width;
    wattr.height      = rect.height;
    wattr.wclass      = GDK_INPUT_OUTPUT;
    wattr.window_type = GDK_WINDOW_CHILD;
    _window = gdk_window_new(parent_window, &wattr, GDK_WA_X|GDK_WA_Y);
    if (!_window) {
        throw VaapiException("Could not create video child window");
    }

    gdk_window_show(_window);
    gdk_window_raise(_window);
    gdk_flush();
    _rect = rect;
}

VaapiVideoWindow::~VaapiVideoWindow()
{
    if (_window) {
        gdk_window_destroy(_window);
        _window = NULL;
    }
}

void
VaapiVideoWindow::moveResize(VaapiRectangle const & rect)
{
    if (!_window) {
        return;
    }

    gdk_window_move_resize(_window, rect.x, rect.y, rect.width, rect.height);
    gdk_flush();
    _rect = rect;
}

GtkAggVaapiGlue::GtkAggVaapiGlue()
    : _agg_renderer(NULL)
    , _vaapi_image_format(VAAPI_IMAGE_NONE)
    , _vaapi_image_width(0)
    , _vaapi_image_height(0)
    , _window_width(0)
    , _window_height(0)
    , _window_is_setup(false)
{
}

GtkAggVaapiGlue::~GtkAggVaapiGlue()
{
}

bool
GtkAggVaapiGlue::init(int /*argc*/, char ** /*argv*/[])
{
    VaapiGlobalContext *const gvactx = VaapiGlobalContext::get();
    if (!gvactx) {
        log_error(_("WARNING: failed to create VA-API display."));
        return false;
    }
    return true;
}

void
GtkAggVaapiGlue::prepDrawingArea(GtkWidget *drawing_area)
{
    dprintf("GtkAggVaapiGlue::prepDrawingArea()\n");

    _drawing_area = drawing_area;

    // Disable double buffering. Otherwise, Gtk tries to update widget
    // contents from its internal offscreen buffer at the end of
    // expose events
    gtk_widget_set_double_buffered(_drawing_area, FALSE);
}

Renderer*
GtkAggVaapiGlue::createRenderHandler()
{
    dprintf("GtkAggVaapiGlue::createRenderHandler()\n");

    VaapiGlobalContext * const gvactx = VaapiGlobalContext::get();
    if (!gvactx)
        return NULL;

    std::vector<VaapiImageFormat> formats = gvactx->getSubpictureFormats();
    for (unsigned int i = 0; i < formats.size(); i++) {
        if (vaapi_image_format_is_rgb(formats[i])) {
            _vaapi_image_format = formats[i];
            break;
        }
    }
    if (_vaapi_image_format == VAAPI_IMAGE_NONE)
        return NULL;

    const char *agg_pixel_format;
    agg_pixel_format = find_pixel_format(_vaapi_image_format);
    if (!agg_pixel_format) {
        log_error(_("GTK-AGG: Unknown RGB format %s reported by VA-API."
                    "Please report this to the gnash-dev mailing list."),
                  string_of_FOURCC(_vaapi_image_format));
        return NULL;
    }

    Renderer * const renderer = create_Renderer_agg(agg_pixel_format);
    _agg_renderer = static_cast<Renderer_agg_base *>(renderer);
    return renderer;
}

void
GtkAggVaapiGlue::resetRenderSurface(unsigned int width, unsigned int height)
{
    /* XXX: round up to 128-byte boundaries to workaround GMA500 bugs */
    const unsigned int aligned_width = (width + 31) & -32U;

    // dprintf("GtkAggVaapiGlue::resetRenderSurface(): size %ux%u\n",
    //         width, height);

    _vaapi_surface.reset(new VaapiSurface(width, height));
    _vaapi_image.reset(new VaapiImage(aligned_width, height, _vaapi_image_format));
    _vaapi_image_width = width;
    _vaapi_image_height = height;
    _vaapi_subpicture.reset(new VaapiSubpicture(_vaapi_image));

    if (!_vaapi_image->map()) {
        log_error(_("failed to map VA-API image."));
        return;
    }

    VaapiRectangle r(width, height);
    if (!_vaapi_surface->associateSubpicture(_vaapi_subpicture, r, r)) {
        log_error(_("failed to associate VA-API subpicture."));
        return;
    }
    _vaapi_surface->clear();

    _agg_renderer->init_buffer(
        static_cast<unsigned char*>(_vaapi_image->getPlane(0)),
        _vaapi_image->getPitch(0) * height,
        width,
        height,
        _vaapi_image->getPitch(0));
}

void
GtkAggVaapiGlue::setRenderHandlerSize(int width, int height)
{
    dprintf("GtkAggVaapiGlue::setRenderHandlerSize(): %dx%d\n", width, height);

    _window_width  = width;
    _window_height = height;
}

void 
GtkAggVaapiGlue::beforeRendering(movie_root* stage)
{
    // Process all GDK pending operations
    gdk_flush();

    if (USE_HW_SCALING) {

        static bool first = true;
        if (first && stage) {
            first = false;

            Movie const & mi = stage->getRootMovie();
            const unsigned int width  = mi.widthPixels();
            const unsigned int height = mi.heightPixels();
            resetRenderSurface(width, height);
            dprintf("GtkAggVaapiGlue::beforeRendering(): movie size %dx%d\n",
                    width, height);
        }

        // We force the scale to its original state in case the GUI
        // changed it (in the event of a resize), because we want
        // VA-API to do the scaling for us.
        _agg_renderer->set_scale(1.0, 1.0);
    }
    else if (_vaapi_image_width != _window_width ||
             _vaapi_image_height != _window_height)
        resetRenderSurface(_window_width, _window_height);

    if (!_vaapi_image->map()) {
        log_error(_("failed to map VA-API image."));
        return;
    }
}

VaapiVideoWindow *
GtkAggVaapiGlue::getVideoWindow(boost::shared_ptr<VaapiSurface> surface,
                                GdkWindow *parent_window,
                                VaapiRectangle const & rect)
{
    VaapiContext * const context = surface->getContext();
    if (!context)
        return NULL;

    if (!context->getData()) {
        std::auto_ptr<VaapiContextData> contextData;
        contextData.reset(new VaapiVideoWindow(parent_window, rect));
        if (!contextData.get())
            return NULL;
        context->setData(contextData);
    }
    return dynamic_cast<VaapiVideoWindow *>(context->getData());
}

void
GtkAggVaapiGlue::render()
{
     VaapiGlobalContext * const gvactx = VaapiGlobalContext::get();
     if (!gvactx)
         return;

     if (!_window_is_setup)
         return;

     if (!_vaapi_image.get() || !_vaapi_surface.get())
         return;

     if (!_vaapi_image->unmap()) {
         printf("ERROR: failed to unmap VA-API image\n");
         return;
     }

     VAStatus status;
     status = vaPutSurface(gvactx->display(),
                           _vaapi_surface->get(),
                           GDK_DRAWABLE_XID(_drawing_area->window),
                           0, 0,
                           _vaapi_surface->width(),
                           _vaapi_surface->height(),
                           0, 0,
                           _window_width,
                           _window_height,
                           NULL, 0,
                           VA_FRAME_PICTURE);
     if (!vaapi_check_status(status, "vaPutSurface() canvas"))
         return;

     Renderer_agg_base::RenderImages::const_iterator img, first_img, last_img;
     first_img = _agg_renderer->getFirstRenderImage();
     last_img  = _agg_renderer->getLastRenderImage();

     if (first_img != last_img) {
         for (img = first_img; img != last_img; ++img) {
             boost::shared_ptr<VaapiSurface> surface = (*img)->surface();

             VaapiRectangle src_rect;
             src_rect.x      = (*img)->x();
             src_rect.y      = (*img)->y();
             src_rect.width  = (*img)->width();
             src_rect.height = (*img)->height();

             VaapiRectangle dst_rect;
             const float xscale = _window_width / (float)_vaapi_image_width;
             const float yscale = _window_height / (float)_vaapi_image_height;
             dst_rect.x      = src_rect.x * xscale;
             dst_rect.y      = src_rect.y * yscale;
             dst_rect.width  = src_rect.width * xscale;
             dst_rect.height = src_rect.height * yscale;

             VaapiVideoWindow *videoWindow;
             videoWindow = getVideoWindow(surface, _drawing_area->window, dst_rect);
             if (!videoWindow) {
                 log_error(_("failed to setup video window for surface 0x%08x."), surface->get());
                 continue;
             }
             videoWindow->moveResize(dst_rect);

             VaapiRectangle pic_rect(surface->width(), surface->height());
             if (!surface->associateSubpicture(_vaapi_subpicture, src_rect, pic_rect)) {
                 log_error(_("failed to associate subpicture to surface 0x%08x."), surface->get());
                 continue;
             }

             status = vaPutSurface(gvactx->display(),
                                   surface->get(),
                                   videoWindow->xid(),
                                   0, 0, surface->width(), surface->height(),
                                   0, 0, dst_rect.width, dst_rect.height,
                                   NULL, 0,
                                   VA_FRAME_PICTURE);
             if (!vaapi_check_status(status, "vaPutSurface() video"))
                 continue;

             surface->deassociateSubpicture(_vaapi_subpicture);
         }

         for (img = first_img; img != last_img; ++img) {
             boost::shared_ptr<VaapiSurface> surface = (*img)->surface();

             status = vaSyncSurface(gvactx->display(), surface->get());
             if (!vaapi_check_status(status, "vaSyncSurface() video"))
                 continue;
         }
     }
}

void
GtkAggVaapiGlue::render(GdkRegion * const /*region*/)
{
    render();
}

void
GtkAggVaapiGlue::configure(GtkWidget *const /*widget*/, GdkEventConfigure *const event)
{
    dprintf("GtkAggVaapiGlue::configure()\n");

    if (_agg_renderer)
        setRenderHandlerSize(event->width, event->height);

    _window_is_setup = true;
}

} // namespace gnash

