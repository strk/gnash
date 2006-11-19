//
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

//
//

#include "gnash.h"

namespace gnash
{

class RiscosGlue
{
  public:
    virtual ~RiscosGlue() { };
    virtual bool init(int argc, char **argv[]) = 0;

    virtual void prepFramebuffer(void *framebuffer, int width, int height) = 0;
    virtual render_handler* createRenderHandler() = 0;
    virtual void setRenderHandlerSize(int /*width*/, int /*height*/) { };
    virtual void render(int /* x */, int /* y */) = 0;
    virtual void render(int x, int y, int /*minx*/, int /*miny*/, int /*maxx*/, int /*maxy*/)
			{ render(x, y);	};
//    virtual void configure(GtkWidget *const widget,
//                           GdkEventConfigure *const event) = 0;
  protected:
    void *_framebuffer;
    int _fbwidth;
    int _fbheight;
};

} // namespace gnash
