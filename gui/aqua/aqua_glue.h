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


#ifndef AQUA_GLUE_H
#define AQUA_GLUE_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif


#include "gnash.h"
#include "snappingrange.h"
#include <AGL/agl.h>

namespace gnash
{

class AquaGlue
{
  public:
    virtual ~AquaGlue() { }
    virtual bool init(int argc, char **argv[]) = 0;
    virtual bool prepDrawingArea(int width, int height, AGLDrawable drawable) = 0;
    virtual Renderer* createRenderHandler() = 0;
    virtual void render() = 0;
  protected:
    int _bpp;
};

}

#endif /* AQUA_GLUE_H */
