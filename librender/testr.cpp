// 
//   Copyright (C) 2010 Free Software Foundation, Inc
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

#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <map>
#include <cassert>
#include <regex.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "log.h"
#include "dejagnu.h"
#include "Renderer.h"
#include "Renderer_agg.h"

TestState runtest;

using namespace gnash; 

// The debug log used by all the gnash libraries.
static LogFile& dbglogfile = LogFile::getDefaultInstance();

// FIXME: this should be a command line option
#undef GTK_TEST_RENDER
 
int
main(int argc, char *argv[])
{
    // FIXME: for now, always run verbose till this supports command line args
    dbglogfile.setVerbosity();

    const char *pixelformat = "RGB24";
    
#ifdef GTK_TEST_RENDER
    // FIXME: GTK specific!
    GtkWidget *drawing_area = 0;
    if (!drawing_area) {
        log_error("No GDK drawing area!");
        exit(-1);
    }
    
    GdkVisual *wvisual = gdk_drawable_get_visual(drawing_area->window);

    GdkImage* tmpimage = gdk_image_new (GDK_IMAGE_FASTEST, wvisual, 1, 1);
    const GdkVisual* visual = tmpimage->visual;

    // FIXME: we use bpp instead of depth, because depth doesn't appear to
    // include the padding byte(s) the GdkImage actually has.
    pixelformat = agg_detect_pixel_format(
        visual->red_shift, visual->red_prec,
        visual->green_shift, visual->green_prec,
        visual->blue_shift, visual->blue_prec,
        tmpimage->bpp * 8);
#endif  // end of GTK_TEST_RENDER
    
    Renderer *agg_renderer = create_Renderer_agg(pixelformat);
    if (agg_renderer > 0) {
        runtest.pass("Got AGG Renderer");
    } else {
        runtest.fail("Didn't get AGG Renderer");
    }
}

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
