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


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <QWidget>
#include <QGLWidget>
#include <QRect>

#include "Qt4GlueOgl.h"
#include "Qt4Gui.h"
#include "Renderer.h"
#include "opengl/Renderer_ogl.h"
#include "GnashException.h"

namespace gnash
{

Qt4OglGlue::Qt4OglGlue()
:
  _width(0),
  _height(0),
  _renderer(nullptr)
{
}

Qt4OglGlue::~Qt4OglGlue()
{
}

bool
Qt4OglGlue::init(int /* argc */, char *** /* argv */)
{
    return true;
}


void
Qt4OglGlue::prepDrawingArea(DrawingWidget *drawing_area)
{
    assert(drawing_area);
    _drawing_area = drawing_area;
    _drawing_area->_glWidget = new QGLWidget(drawing_area);
    _drawing_area->_glWidget->setVisible(drawing_area->isVisible());
    _drawing_area->_glWidget->setMinimumSize(drawing_area->minimumSize());
    _drawing_area->_glWidget->setSizePolicy(QSizePolicy::Expanding,
                                            QSizePolicy::Expanding);
    _drawing_area->_glWidget->makeCurrent();
}


void
Qt4OglGlue::render()
{
    assert(_drawing_area);
    _drawing_area->_glWidget->swapBuffers();
}


void
Qt4OglGlue::render(const QRect& /*updateRect*/)
{
  render();
}


Renderer*
Qt4OglGlue::createRenderHandler()
{
    _renderer = renderer::opengl::create_handler();

    if ( ! _renderer ) {
        throw GnashException("Could not create OpenGL renderer");
    }
    return _renderer;
}

// end of namespace gnash
}
