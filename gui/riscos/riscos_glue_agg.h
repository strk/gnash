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


#include "riscos_glue.h"

namespace gnash
{

class RiscosAggGlue : public RiscosGlue
{
  public:
    RiscosAggGlue();
    ~RiscosAggGlue();

    bool init(int argc, char **argv[]);
    void prepFramebuffer(void *framebuffer, int width, int height);
    Renderer* createRenderHandler();
    void setRenderHandlerSize(int width, int height);
    void render(int x, int y);
    void render(int x, int y, int minx, int miny, int maxx, int maxy);
//    void configure(GtkWidget *const widget, GdkEventConfigure *const event);

  private:
    unsigned char *_offscreenbuf;
    int _offscreenbuf_size;
    Renderer *_agg_renderer;
    int _width, _height, _bpp;
};

} // namespace gnash
