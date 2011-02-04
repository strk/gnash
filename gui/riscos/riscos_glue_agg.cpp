//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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


#include <cstdio>
#include <cerrno>
#include <cstring>

#include "gnash.h"
#include "log.h"
#include "Renderer.h"
#include "Renderer_agg.h"
#include "riscos_glue_agg.h"

namespace gnash
{

RiscosAggGlue::RiscosAggGlue() :
	_offscreenbuf(NULL),
	_offscreenbuf_size(0),
	_agg_renderer(NULL),
	_width(0),
	_height(0),
	_bpp(0)
{
}

RiscosAggGlue::~RiscosAggGlue()
{
  free(_offscreenbuf);
}

bool
RiscosAggGlue::init(int /*argc*/, char **/*argv*/[])
{
  // We only support 32bpp modes
  _bpp = 24;

  return true;
}

void
RiscosAggGlue::prepFramebuffer(void *framebuffer, int width, int height)
{
    _framebuffer = framebuffer;
    _fbwidth = width;
    _fbheight = height;
}

Renderer*
RiscosAggGlue::createRenderHandler()
{
  _agg_renderer = create_Renderer_agg("RGB24");
  if (! _agg_renderer) {
      throw GnashException(_("Could not create AGG renderer with pixelformat RGB24"));
  }
  return _agg_renderer;
}

void
RiscosAggGlue::setRenderHandlerSize(int width, int height)
{
  assert(width>0);
  assert(height>0);
  assert(_agg_renderer!=NULL);

#define CHUNK_SIZE (100*100*(_bpp/8))

  if (width == _width && height == _height)
    return;

  int new_bufsize = width*height*((_bpp+7)/8);

  // TODO: At the moment we only increase the buffer and never decrease it.
  // Should be changed sometime.
  if (new_bufsize > _offscreenbuf_size) {
    new_bufsize = static_cast<int>(new_bufsize / CHUNK_SIZE + 1) * CHUNK_SIZE;
    // TODO: C++ conform alternative to realloc?
    _offscreenbuf = static_cast<unsigned char *>( realloc(_offscreenbuf, new_bufsize) );

    if (!_offscreenbuf) {
      log_debug("Could not allocate %i bytes for offscreen buffer: %s\n",
               new_bufsize, strerror(errno) );
      return;
    }

    log_debug("RISC OS-AGG: %i bytes offscreen buffer allocated\n", new_bufsize);

    _offscreenbuf_size = new_bufsize;
    memset(_offscreenbuf, 0, new_bufsize);
  }

  _width = width;
  _height = height;

  // Only the AGG renderer has the function init_buffer, which is *not* part
  // of the renderer api. It allows us to change the renderers movie size
  // (and buffer address) during run-time.
  static_cast<Renderer_agg_base *>(_agg_renderer)->init_buffer(
         _offscreenbuf,
         _offscreenbuf_size,
         _width,
         _height,
         _width*((_bpp+7)/8));
}

void
RiscosAggGlue::render(int x, int y)
{
  // Update the entire screen
  render(x, y, 0, 0, _width, _height);
}

void
RiscosAggGlue::render(int x, int y, int minx, int miny, int maxx, int maxy)
{
  // Update only the invalidated rectangle
  unsigned char *fb = (unsigned char *)_framebuffer;
  int fbw = _fbwidth * 4; // (((_bpp / 8) + 3) & ~3);
  int osw = _width * (_bpp / 8);

  for (int row = miny; row < maxy; row++) {
//    int fbr = row * (((_bpp / 8) + 3) & ~3);
//    int osr = row * (_bpp / 8);

    for (int col = minx; col < maxx; col++) {
      int fbc = (col + x) * 4; // (((_bpp / 8) + 3) & ~3);
      int osc = col * (_bpp / 8);

      fb[(row + y) * fbw + fbc + 0] = _offscreenbuf[row * osw + osc + 0];
      fb[(row + y) * fbw + fbc + 1] = _offscreenbuf[row * osw + osc + 1];
      fb[(row + y) * fbw + fbc + 2] = _offscreenbuf[row * osw + osc + 2];
      fb[(row + y) * fbw + fbc + 3] = 0;
    }
  }

}

#if 0
void
RiscosAggGlue::configure(GtkWidget *const /*widget*/, GdkEventConfigure *const event)
{
	if (_agg_renderer)
		setRenderHandlerSize(event->width, event->height);
}
#endif

} // namespace gnash

