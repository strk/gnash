//
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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
//

/* $Id: gtk_glue_agg.cpp,v 1.18 2007/06/26 17:40:09 udog Exp $ */


/*

TODO: Support MIT-SHM to push image data more quickly on screen, avoiding
overhead by X Server IPC.

References:
  http://en.wikipedia.org/wiki/MIT-SHM
  http://www.xfree86.org/current/mit-shm.html

Also worth checking:
  http://en.wikipedia.org/wiki/X_video_extension
*/


#include <cstdio>
#include <cerrno>
#include <cstring>
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
	_bpp(0)
{
}

GtkAggGlue::~GtkAggGlue()
{
  free(_offscreenbuf);
  destroy_shm_image();
}

bool
GtkAggGlue::init(int /*argc*/, char **/*argv*/[])
{
    gdk_rgb_init();
    
    _have_shm = check_mit_shm(gdk_display);
    
#ifdef PIXELFORMAT_RGB565
    _bpp = 16;
#else
    // GDK's gdk_draw_rgb_image() needs 24-bit RGB data, so we initialize the
    // AGG renderer with RGB24 and let GTK take care of the proper pixel format.
    _bpp = 24;
#endif
    return true;
}

bool 
GtkAggGlue::check_mit_shm(Display *display) 
{
#ifdef ENABLE_MIT_SHM
  int major, minor, dummy;
  Bool pixmaps;
  
  log_msg("Checking support for MIT-SHM...");
  
  if (!XQueryExtension(display, "MIT-SHM", &dummy, &dummy, &dummy)) 
  {
    log_msg("WARNING: No MIT-SHM extension available, using standard XLib "
      "calls (slower)");
    return false;
  }
  
  if (XShmQueryVersion(display, &major, &minor, &pixmaps )!=True)
	{
    log_msg("WARNING: MIT-SHM not ready (network link?), using standard XLib "
      "calls (slower)");
    return false;
	}
	
	log_msg("NOTICE: MIT-SHM available (version %d.%d)!", major, minor);
	
	
#else
	return false; // !ifdef ENABLE_MIT_SHM
#endif
  
}

void 
GtkAggGlue::create_shm_image(unsigned int width, unsigned int height)
{

  //Visual xvisual = ((GdkVisualPrivate*) visual)->xvisual; 

  destroy_shm_image();
  
  /*if (!_shm_info) {
    _shm_info = malloc(sizeof XShmSegmentInfo);
  }*/ 
  
  //_shm_image = XShmCreateImage();
}

void 
GtkAggGlue::destroy_shm_image()
{
#ifdef ENABLE_MIT_SHM
  if (!_shm_image) return; // not allocated
  
  XDestroyImage(_shm_image);
  _shm_image=NULL;
  
#endif
}

void
GtkAggGlue::prepDrawingArea(GtkWidget *drawing_area)
{
    _drawing_area = drawing_area;
}

render_handler*
GtkAggGlue::create_shm_handler()
{
#ifdef ENABLE_MIT_SHM
  GdkVisual *visual = gdk_drawable_get_visual(_drawing_area->window);
  
  char *pixelformat = agg_detect_pixel_format(
    visual->red_shift, visual->red_prec,
    visual->green_shift, visual->green_prec,
    visual->blue_shift, visual->blue_prec,
    visual->depth); 

  if (!pixelformat) {
    log_msg("Pixel format of X server not recognized (%d:%d, %d:%d, %d:%d, %d bpp)",
      visual->red_shift, visual->red_prec,
      visual->green_shift, visual->green_prec,
      visual->blue_shift, visual->blue_prec,
      visual->depth);
    return NULL; 
  }
  
  log_msg("X server is using %s pixel format", pixelformat);
  
  render_handler* res = create_render_handler_agg(pixelformat);
  
  if (!res) 
    log_msg("Failed creating a renderer instance for this pixel format. "
      "Most probably Gnash has not compiled in (configured) support "
      "for this pixel format - using standard pixmaps instead");      
  
  
  return res;
    
#else
  return NULL;
#endif
}

render_handler*
GtkAggGlue::createRenderHandler()
{

  // try with MIT-SHM
  _agg_renderer = create_shm_handler();
  if (_agg_renderer) return _agg_renderer;

#ifdef PIXELFORMAT_RGB565
#warning A pixel format of RGB565; you must have a (hacked) GTK which supports \
         this format (e.g., GTK on the OLPC).
    _agg_renderer = create_render_handler_agg("RGB565");
#else
    _agg_renderer = create_render_handler_agg("RGB24");
#endif
    return _agg_renderer;
}

void
GtkAggGlue::setRenderHandlerSize(int width, int height)
{
	assert(width>0);
	assert(height>0);
	assert(_agg_renderer!=NULL);

	#define CHUNK_SIZE (100*100*(_bpp/8))
	
	create_shm_image(width, height);

	if (width == _width && height == _height)
	   return;
	   
	int new_bufsize = width*height*(_bpp/8);
	
	// TODO: At the moment we only increase the buffer and never decrease it. Should be
	// changed sometime.
	if (new_bufsize > _offscreenbuf_size) {
		new_bufsize = static_cast<int>(new_bufsize / CHUNK_SIZE + 1) * CHUNK_SIZE;
		// TODO: C++ conform alternative to realloc?
		_offscreenbuf	= static_cast<unsigned char *>( realloc(_offscreenbuf, new_bufsize) );

		if (!_offscreenbuf) {
		  log_msg("Could not allocate %i bytes for offscreen buffer: %s\n",
				new_bufsize, strerror(errno)
			);
			return;
		}

		log_msg("GTK-AGG: %i bytes offscreen buffer allocated\n", new_bufsize);

		_offscreenbuf_size = new_bufsize;
		memset(_offscreenbuf, 0, new_bufsize);
	}

  _width = width;
	_height = height;

	// Only the AGG renderer has the function init_buffer, which is *not* part of
	// the renderer api. It allows us to change the renderers movie size (and buffer
	// address) during run-time.
	static_cast<render_handler_agg_base *>(_agg_renderer)->init_buffer(
	  _offscreenbuf,
		_offscreenbuf_size,
		_width,
		_height
	);
	
}

void
GtkAggGlue::render()
{
	// Update the entire screen
	gdk_draw_rgb_image (
		_drawing_area->window,
		_drawing_area->style->fg_gc[GTK_STATE_NORMAL],
		0,
		0,
		_width,
		_height,
		GDK_RGB_DITHER_NONE,
		_offscreenbuf,
		(int)(_width*_bpp/8)
	);
	
}

void
GtkAggGlue::render(int minx, int miny, int maxx, int maxy)
{

	// Update only the invalidated rectangle
	gdk_draw_rgb_image (
		_drawing_area->window,
		_drawing_area->style->fg_gc[GTK_STATE_NORMAL],
		minx,
  	miny,
		maxx-minx+1,
		maxy-miny+1,
		GDK_RGB_DITHER_NORMAL,
		_offscreenbuf + miny*(_width*(_bpp/8)) + minx*(_bpp/8),
		(int)((_width)*_bpp/8)
	);
}

void
GtkAggGlue::configure(GtkWidget *const /*widget*/, GdkEventConfigure *const event)
{
	if (_agg_renderer)
		setRenderHandlerSize(event->width, event->height);
}



} // namespace gnash

