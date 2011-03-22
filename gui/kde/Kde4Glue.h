//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#ifndef GNASH_KDE4_GLUE_H
#define GNASH_KDE4_GLUE_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "snappingrange.h"

#include <QWidget>

class QRect;
class QGLWidget;

namespace gnash {
    class Renderer;
    class DrawingWidget;
    class Kde4Gui;
}

namespace gnash {

class DrawingWidget : public QWidget
{
    Q_OBJECT

public:
    DrawingWidget(Kde4Gui& gui);
    ~DrawingWidget() {}

#ifdef RENDERER_OPENGL
    QGLWidget* _glWidget;
#endif 
public slots:
    
    void properties();
    void preferences();
    void play(); 
    void pause();
    void stop();
    void restart();
    void refresh();
    void fullscreen(bool isFull); 
    void quit();

protected:
    void paintEvent(QPaintEvent* event);
    void timerEvent(QTimerEvent* event);
    void resizeEvent(QResizeEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event); 
    void mouseMoveEvent(QMouseEvent* event); 
    void wheelEvent(QWheelEvent* event); 

    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void contextMenuEvent(QContextMenuEvent* event);
    
private:
    Kde4Gui& _gui;
};  

class Kde4Glue
{
  public:
    Kde4Glue() : _drawing_area(NULL) {}
    virtual ~Kde4Glue() { }
    virtual bool init(int argc, char **argv[]) = 0;

    virtual void prepDrawingArea(DrawingWidget *drawing_area) = 0;
    virtual Renderer* createRenderHandler() = 0;
    virtual void render() = 0;
    virtual void render(const QRect& updateRect) = 0;
    virtual void setInvalidatedRegions(const InvalidatedRanges& /* ranges */) {}
    virtual void resize(int, int) {}
    virtual void initBuffer(int, int) {}
  protected:
    DrawingWidget     *_drawing_area;
};

} // namespace gnash

#endif
