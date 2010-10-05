// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

///
/// Author: Visor <cutevisor@gmail.com>
///

#ifndef GNASH_RENDER_HANDLER_OVG_H
#define GNASH_RENDER_HANDLER_OVG_H

#include <EGL/egl.h>
#include <VG/openvg.h>
#include <VG/vgu.h>
#include <VG/vgext.h>

#include "Geometry.h"
//#include "BitmapInfo.h"
#include "Renderer.h"

namespace gnash {

typedef std::vector<const Path*> PathRefs;

struct eglVertex {
    eglVertex(float x, float y)
        : _x(x), _y(y)
        {
        }
  
    eglVertex(const point& p)
        : _x(p.x), _y(p.y)
        {
        }

    VGfloat _x;
    VGfloat _y;
};

typedef std::map<const Path*, VGPath > PathPointMap;

class Renderer_ovg_base : public Renderer
{
public:

    Renderer_ovg_base() { }

    // virtual classes should have virtual destructors
    virtual ~Renderer_ovg_base() {}

    // these methods need to be accessed from outside:
    virtual void init(float x, float y)=0;

    // These methods are only for debugging and development
    void printVGParams();
    void printVGHardware();
    void printVGPath();
};

DSOEXPORT Renderer_ovg_base* create_Renderer_ovg(const char *pixelformat);

} // namespace gnash

#endif // __RENDER_HANDLER_OVG_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
