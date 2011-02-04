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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "gtk_glue.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <memory>
#include "VaapiImageFormat.h"

namespace gnash
{

// Forward declarations
class VaapiImage;
class VaapiSurface;
class VaapiSubpicture;
class VaapiRectangle;
class VaapiVideoWindow;
class Renderer_agg_base;
class movie_root;

class GtkAggVaapiGlue : public GtkGlue
{
public:
    GtkAggVaapiGlue();
    ~GtkAggVaapiGlue();

    bool init(int argc, char **argv[]);
    void prepDrawingArea(GtkWidget *drawing_area);
    Renderer* createRenderHandler();
    void setRenderHandlerSize(int width, int height);
    virtual void beforeRendering(movie_root* stage);
    void render();
    void render(GdkRegion * const);
    void configure(GtkWidget *const widget, GdkEventConfigure *const event);

private:
    VaapiVideoWindow *getVideoWindow(boost::shared_ptr<VaapiSurface> surface,
                                     GdkWindow *parent_window,
                                     VaapiRectangle const & rect);

    void resetRenderSurface(unsigned int width, unsigned int height);

private:
    Renderer_agg_base                  *_agg_renderer;
    VaapiImageFormat                    _vaapi_image_format;
    boost::shared_ptr<VaapiImage>       _vaapi_image;
    unsigned int                        _vaapi_image_width;
    unsigned int                        _vaapi_image_height;
    boost::shared_ptr<VaapiSubpicture>  _vaapi_subpicture;
    std::auto_ptr<VaapiSurface>         _vaapi_surface;
    unsigned int                        _window_width;
    unsigned int                        _window_height;
    bool                                _window_is_setup;
};

} // namespace gnash
