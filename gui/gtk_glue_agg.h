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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "gtk_glue.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#undef ENABLE_MIT_SHM

// Experimental support for MIT-SHM
// see http://www.xfree86.org/current/mit-shm.html
// currently has some problems, see https://savannah.gnu.org/bugs/?20301
#ifdef ENABLE_MIT_SHM
#include <X11/extensions/XShm.h>
#endif


namespace gnash
{

class GtkAggGlue : public GtkGlue
{
  public:
    GtkAggGlue();
    ~GtkAggGlue();

    bool init(int argc, char **argv[]);
    void prepDrawingArea(GtkWidget *drawing_area);
    render_handler* createRenderHandler();
    void setRenderHandlerSize(int width, int height);
    void beforeRendering();
    void render();
    void render(int minx, int miny, int maxx, int maxy);
    void configure(GtkWidget *const widget, GdkEventConfigure *const event);
    
  private:
    unsigned char *_offscreenbuf;
    int _offscreenbuf_size;
    render_handler *_agg_renderer;
    int _width, _height, _bpp;
    char _pixelformat[16];
    bool _have_shm;
#ifdef ENABLE_MIT_SHM
    XImage *_shm_image;
    XShmSegmentInfo *_shm_info;
    
    /// Checks if the MIT-SHM extension is available (supported by the server)
    bool check_mit_shm(Display *display);
#else
    bool check_mit_shm(void *display);
#endif    
    
    /// Tries to create a SHM image. 
    ///
    /// This can still fail even if check_mit_shm() returned true in case
    /// we have not the appropriate pixel format compiled in or any other
    /// error happened. create_shm_image() replaces (destroys) an already
    /// allocated SHM image.
    void create_shm_image(unsigned int width, unsigned int height);
    
    /// Destroys a previously created SHM image (deals with NULL pointer)
    void destroy_shm_image();
    
    /// Tries to create a AGG render handler based on the X server pixel
    /// format. Returns NULL on failure.
    render_handler *create_shm_handler();    
    
    /// converts a bitmask to a shift/size information (used for pixel format
    /// detection)
    void decode_mask(unsigned long mask, unsigned int *shift, unsigned int *size);
    
    /// Tries to detect the pixel format used by the X server (usually RGB24).
    /// It does not have to match the hardware pixel format, just the one
    /// expected for pixmaps. This function is /not/ used for MIT-SHM!
    bool detect_pixelformat();
    
};

} // namespace gnash
