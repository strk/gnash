// Kde4Glue.cpp: KDE4/Qt4 shared code to connect the various renderers to
// the Kde4 gui.
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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "Kde4Glue.h"
#include "Kde4Glue.moc"
#include "Kde4Gui.h"
#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>

namespace gnash {

/// DrawingWidget implementation
DrawingWidget::DrawingWidget(Kde4Gui& gui)
    :
    _gui(gui)
{
}

void
DrawingWidget::paintEvent(QPaintEvent *event)
{
    _gui.renderWidget(event->rect());
}

void
DrawingWidget::timerEvent(QTimerEvent*)
{
    Gui::advance_movie(&_gui);
}

void
DrawingWidget::wheelEvent(QWheelEvent* event)
{
    _gui.notifyMouseWheel(event->delta() > 0 ? 1 : -1);
}

void
DrawingWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPoint position = event->pos();
    _gui.notifyMouseMove(position.x(), position.y());
}

void
DrawingWidget::contextMenuEvent(QContextMenuEvent* event)
{
    _gui.popupMenu(event->globalPos());
}

void
DrawingWidget::mousePressEvent(QMouseEvent* /* event */)
{
    _gui.notifyMouseClick(true);
}

void
DrawingWidget::mouseReleaseEvent(QMouseEvent* /* event */)
{
    _gui.notifyMouseClick(false);
}

void
DrawingWidget::keyPressEvent(QKeyEvent *event)
{
    _gui.handleKeyEvent(event, true);
}

void
DrawingWidget::keyReleaseEvent(QKeyEvent *event)
{
    _gui.handleKeyEvent(event, false);
}

void
DrawingWidget::resizeEvent(QResizeEvent *event)
{
    _gui.resize(event->size().width(), event->size().height());
    update();
}

void
DrawingWidget::properties()
{
    _gui.showProperties();
}

void
DrawingWidget::preferences()
{
    _gui.showPreferences();
}

void
DrawingWidget::play()
{
    _gui.play();
}

void
DrawingWidget::pause()
{
    _gui.pause();
}

void
DrawingWidget::restart()
{
    _gui.restart();
}

void
DrawingWidget::stop()
{
    _gui.stop();
}                                                             

void
DrawingWidget::refresh()
{
    _gui.refreshView();                                       
}

void
DrawingWidget::quit()
{
    _gui.quit();
}

void
DrawingWidget::fullscreen(bool isFull)
{
    if (isFull) {
        _gui.setFullscreen();
    }
    else {
        _gui.unsetFullscreen();
    }
}

}
