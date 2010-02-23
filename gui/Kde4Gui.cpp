// kde.cpp:  K Development Environment top level window, for Gnash.
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


#include <map>
#include <boost/assign/list_inserter.hpp>

#include <QMainWindow>
#include <QX11Info>
#include <QMenu>
#include <QMenuBar>
#include <QWidget>
#include <QCursor>
#include <QApplication>
#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QTimer>
#include <QEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLayout>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStack>
#include <QTabWidget>
#include <QFrame>
#include <QLabel>
#include <QSlider>
#include <QLineEdit>
#include <QCheckBox>
#include <QLCDNumber>
#include <QSpinBox>

#include "Range2d.h"

#include "gnash.h"
#include "movie_definition.h" 
#include "log.h"

#include "gui.h"
#include "Kde4Gui.h"
#include "klash4.moc"
#include "Renderer.h"
#include "RunResources.h" 

// Macro for using gettext strings where Qt expects QStrings
#define _q(Str) QString::fromUtf8(_(Str))

extern "C" {
#include <X11/Xlib.h>
}

namespace gnash 
{

Kde4Gui::Kde4Gui(unsigned long xid, float scale, bool loop, RunResources& r)
 : Gui(xid, scale, loop, r)
{
}


Kde4Gui::~Kde4Gui()
{
}


bool
Kde4Gui::init(int argc, char **argv[])
{

    char** r = NULL;
    int* i = new int(0);

    _application.reset(new QApplication(*i, r));
    _window.reset(new QMainWindow());
    _embedWidget = new EmbedWidget(*this);
    _drawingWidget = _embedWidget->drawingWidget();

    _glue.init (argc, argv);

    setupActions();
    setupMenus();

    if (!_xid) {
        createMainMenu();
    }

    // Make sure key events are ready to be passed
    // before the widget can receive them.
    setupKeyMap();
    
    return true;
}


bool
Kde4Gui::run()
{
    return _application->exec();
}


bool
Kde4Gui::createWindow(const char* windowtitle, int width, int height,
                     int xPosition, int yPosition)
{
    _width = width;
    _height = height;

    _drawingWidget->setMinimumSize(_width, _height);

    // Enable receiving of mouse events.
    _drawingWidget->setMouseTracking(true);
    _drawingWidget->setFocusPolicy(Qt::StrongFocus);
    _window->setWindowTitle(windowtitle);
    _window->setWindowIcon(QIcon(PKGDATADIR"/GnashG.png"));
    
    if(_xid) {
        _embedWidget->embedInto(_xid);
        _embedWidget->show();
        // Adjust width and height to the window we're being embedded into...
        XWindowAttributes winAttributes;
        XGetWindowAttributes(QX11Info::display(), _xid, &winAttributes);
        _width=winAttributes.width;
        _height=winAttributes.height;
    } else {
        // The QMainWindow takes ownership of the widgets.
        _window->setCentralWidget(_embedWidget);
        if (xPosition > -1 || yPosition > -1)
            _window->move(xPosition, yPosition);
        _window->show();
    }

    _glue.prepDrawingArea(_drawingWidget);

    _renderer.reset(_glue.createRenderHandler());

    if (!_renderer.get()) {
        return false;
    }

    _validbounds.setTo(0, 0, _width, _height);
    _glue.initBuffer(_width, _height);
    
    log_debug(_("Setting renderer"));

    _runResources.setRenderer(_renderer);
    
    log_debug(_("Set renderer"));
   
    return true;
}


void
Kde4Gui::resizeWindow(int width, int height)
{
    _width = width;
    _height = height;

    _drawingWidget->setMinimumSize(_width, _height);
}

void
Kde4Gui::popupMenu(const QPoint& point)
{
    QMenu popupMenu(_drawingWidget);
    popupMenu.addMenu(fileMenu);
    popupMenu.addMenu(editMenu);
    popupMenu.addMenu(movieControlMenu);
    popupMenu.addMenu(viewMenu);
    popupMenu.exec(point);
}


void
Kde4Gui::renderBuffer()
{
    
    for (DrawBounds::const_iterator i = _drawbounds.begin(),
                        e = _drawbounds.end(); i != e; ++i) {
        
        // it may happen that a particular range is out of the screen, which 
        // will lead to bounds==null. 
        if (i->isNull()) continue;
        
        assert(i->isFinite()); 

        _drawingWidget->update(i->getMinX(), i->getMinY(),
                               i->width(), i->height());

    }
}


void
Kde4Gui::renderWidget(const QRect& updateRect)
{
    // This call renders onto the widget using a QPainter,
    // which *must only happen inside a paint event*.
    _glue.render(updateRect);
}


void
Kde4Gui::setInvalidatedRegions(const InvalidatedRanges& ranges)
{
    _renderer->set_invalidated_regions(ranges);

    _drawbounds.clear();

    for (size_t i = 0, e = ranges.size(); i != e; ++i) {

        geometry::Range2d<int> bounds = Intersection(
        _renderer->world_to_pixel(ranges.getRange(i)),
        _validbounds);

        // It may happen that a particular range is out of the screen, which 
        // will lead to bounds==null. 
        if (bounds.isNull()) continue;

        assert(bounds.isFinite());

        _drawbounds.push_back(bounds);

    }
}


void
Kde4Gui::setTimeout(unsigned int timeout)
{
    // This must go through Gui::quit() to make sure screenshots are
    // handled if necessary.
    QTimer::singleShot(timeout, _drawingWidget, SLOT(quit()));
}


void
Kde4Gui::setInterval(unsigned int interval)
{
    _drawingWidget->startTimer(interval);
}


void
Kde4Gui::setCursor(gnash_cursor_type newcursor)
{
    if (! _mouseShown) return;

    switch (newcursor) {
        case CURSOR_HAND:
            _drawingWidget->setCursor(Qt::PointingHandCursor);
            break;
        case CURSOR_INPUT:
            _drawingWidget->setCursor(Qt::IBeamCursor); 
            break;
        default:
            _drawingWidget->unsetCursor(); 
    }
}

bool
Kde4Gui::showMouse(bool show)
{
    bool prevState = _mouseShown;
    _mouseShown = show;

    if (show) {
        _drawingWidget->unsetCursor();
    }
    else {
        _drawingWidget->setCursor(Qt::BlankCursor);
    }

    return prevState;
}

void
Kde4Gui::setFullscreen()
{
    _fullscreen = true;
    fullscreenAction->setChecked(_fullscreen);

    _embedWidget->setWindowFlags(Qt::Window);
    _embedWidget->showFullScreen();
}

void
Kde4Gui::unsetFullscreen()
{
    _fullscreen = false;
    fullscreenAction->setChecked(_fullscreen);

    if (_embedWidget->isFullScreen()) {
        _embedWidget->setWindowFlags(Qt::Widget);
        _embedWidget->showNormal();
        if (_xid) {
            _embedWidget->embedInto(_xid);
        }
    }
}

gnash::key::code
Kde4Gui::qtToGnashKey(QKeyEvent *event)
{

    // This should be initialized by now.
    assert (!_keyMap.empty());

    // Gnash uses its own keycodes to map key events
    // to the three sometimes weird and confusing values that flash movies
    // can refer to. See gnash.h for the keycodes and map.
    //
    // Gnash's keycodes are gnash::key::code. They are mainly in ascii order.
    // Standard ascii characters (32-127) have the same value. Extended ascii
    // characters (160-254) are in ascii order but correspond to
    // gnash::key::code
    // 169-263. Non-character values must normally be mapped separately.

    const int key = event->key();

    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
          return static_cast<gnash::key::code>(
                key - Qt::Key_0 + gnash::key::_0);
    }

    // All other characters between ascii 32 and 126 are simple.
    // From space (32) to slash (47):
    else if (key >= Qt::Key_Space && key <= Qt::Key_AsciiTilde) {
        return static_cast<gnash::key::code>(
                key - Qt::Key_Space + gnash::key::SPACE);
    }

    // Function keys:
    else if (key >= Qt::Key_F1 && key <= Qt::Key_F15) {
        return static_cast<gnash::key::code>(
                key - Qt::Key_F1 + gnash::key::F1);
    }

    // Extended ascii from non-breaking (160) space to 