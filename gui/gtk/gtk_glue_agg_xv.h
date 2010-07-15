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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "gtk_glue.h"
#include "MediaHandler.h"

#include <string>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <boost/scoped_array.hpp>

#include <X11/Xlib.h>

#include <X11/extensions/XShm.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>


namespace gnash
{

class GtkAggXvGlue : public GtkGlue
{
  public:
    GtkAggXvGlue();
    ~GtkAggXvGlue();

    bool init(int argc, char **argv[]);
    void prepDrawingArea(GtkWidget *drawing_area);
    Renderer* createRenderHandler();
    void beforeRendering();
    void render();
    void render(int minx, int miny, int maxx, int maxy);
    void configure(GtkWidget *const widget, GdkEventConfigure *const event);

  private:
    bool findXvPort(Display* display);
    bool grabXvPort(Display *display, XvPortID port);
    bool create_xv_image(unsigned int width, unsigned int height);
    bool create_xv_shmimage(unsigned int width, unsigned int height);
    void destroy_x_image();
    void setupRendering();
    void decode_mask(unsigned long mask, unsigned int *shift, unsigned int *size);
    bool isFormatBetter(const XvImageFormatValues& oldformat,
                        const XvImageFormatValues& newformat);
    bool ensurePortGrabbed(Display *display, XvPortID port);
    std::string findPixelFormat(const XvImageFormatValues& format);

    /// This will be used in case of RGB->YUV conversion.
    boost::scoped_array<boost::uint8_t> _offscreenbuf;
    Renderer *_agg_renderer;
    size_t _stride;

    /// If the hardware accepts RGB, then Agg will render directly into this.
    XvImage* _xv_image;           
    bool _xv_image_is_shared;
    XvPortID _xv_port;
    unsigned int _xv_max_width;
    unsigned int _xv_max_height;
    int _window_width;
    int _window_height;
    int _movie_width;
    int _movie_height;
    
    media::MediaHandler* _mediaHandler;
    std::auto_ptr<media::VideoConverter> _video_converter;
    
    XvImageFormatValues _xv_format;

    XShmSegmentInfo *_shm_info;
};

} // namespace gnash










