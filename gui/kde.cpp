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


#include <qwidget.h>
#include <qmessagebox.h>
#include <qcursor.h>
#ifdef HAVE_KDE3
#include <qxembed.h>
#endif
#include <qnamespace.h>
#include <qtimer.h>
#include <qcursor.h>
#if HAVE_QT3
#include <qeventloop.h>
#endif
#include "Range2d.h"

#include "gnash.h"
#include "movie_definition.h" 
#include "log.h"

#include "gui.h"
#include "kdesup.h"
#include "klash3.moc"
#include "GnashNumeric.h" // for pixelsToTwips 
#include "RunResources.h" // for pixelsToTwips 

using namespace std;

namespace gnash 
{

KdeGui::~KdeGui()
{
//    GNASH_REPORT_FUNCTION;
}


KdeGui::KdeGui(unsigned long xid, float scale, bool loop, RunResources& r)
 : Gui(xid, scale, loop, r)
{
}

bool
KdeGui::init(int argc, char **argv[])
{
//    GNASH_REPORT_FUNCTION;
    _qapp.reset(new QApplication(argc, *argv));
    _qwidget.reset(new qwidget(this)); 
#ifdef HAVE_KDE3
    if (_xid) {
        QXEmbed::initialize();
        QXEmbed::embedClientIntoWindow(_qwidget.get(), _xid);
    }
#endif
    _glue.init (argc, argv);
    
    return true;
}

bool
KdeGui::createWindow(const char* windowtitle, int width, int height,
                     int xPosition, int yPosition)
{
//    GNASH_REPORT_FUNCTION;

    // Move the window to correct position if requested by user.
    if (xPosition > 1) {
        if (yPosition > 1) {
            _qwidget->setGeometry(xPosition, yPosition, width, height);
            }
        else {  // If only given X position by user
            _qwidget->setGeometry(xPosition, 0, width, height);
            }
        }
    else if (yPosition > 1) { // If only given Y position by user
        _qwidget->setGeometry(0, yPosition, width, height);
    }
    _qwidget->setCaption(windowtitle);

    _qapp->setMainWidget(_qwidget.get());
    _qwidget->show();

    _glue.prepDrawingArea(_qwidget.get());
    _renderer.reset(_glue.createRenderHandler());
    if (!_renderer.get())
    {
        // something went wrong
        return false;
    }
    _glue.initBuffer(width, height);
    
    _width = width;
    _height = height;
    _runResources.setRenderer(_renderer);
    
    return true;
}

void
KdeGui::renderBuffer()
{
//    GNASH_REPORT_FUNCTION;
    _glue.render();
}


void
KdeGui::setInvalidatedRegions(const InvalidatedRanges& ranges)
{
    _glue.setInvalidatedRegions(ranges);
}

void
KdeGui::setTimeout(unsigned int timeout)
{
//    GNASH_REPORT_FUNCTION;
    QTimer::singleShot(timeout, _qapp.get(), SLOT(quit()));
}

void
KdeGui::setInterval(unsigned int interval)
{
//    GNASH_REPORT_FUNCTION;
    _qwidget->setInterval(interval);
    
}
void
KdeGui::setCursor(gnash_cursor_type newcursor)
{
    switch(newcursor) {
        case CURSOR_HAND:
#if QT_VERSION > 2312
            _qwidget->setCursor(Qt::PointingHandCursor);
#else
            _qwidget->setCursor(PointingHandCursor);
#endif
            break;
        case CURSOR_INPUT:
#if QT_VERSION > 2312
            _qwidget->setCursor(Qt::IbeamCursor); 
#else
            _qwidget->setCursor(IbeamCursor); 
#endif
            break;
        default:
            _qwidget->unsetCursor(); 
    }
}

bool
KdeGui::run()
{
//    GNASH_REPORT_FUNCTION;
    _qapp->exec();

    return true;
}

bool
KdeGui::createMenu()
{
//    GNASH_REPORT_FUNCTION;

    return true;
}

bool
KdeGui::setupEvents()
{
//  GNASH_REPORT_FUNCTION;

  return true;
}

gnash::key::code
KdeGui::qtToGnashKey(QKeyEvent *event)
{

    // Gnash uses its own keycodes to map key events
    // to the three sometimes weird and confusing values that flash movies
    // can refer to. See gnash.h for the keycodes and map.
    //
    // Gnash's keycodes are gnash::key::code. They are mainly in ascii order.
    // Standard ascii characters (32-127) have the same value. Extended ascii
    // characters (160-254) are in ascii order but correspond to gnash::key::code
    // 169-263. Non-character values must normally be mapped separately.

    gnash::key::code c = gnash::key::INVALID;
    int key = event->key();

    // Qt seems to treat numbers on the keypad and main keyboard
    // as the same key event, so needs this check:
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
      if (event->state() & Qt::Keypad)
          c = (gnash::key::code) ((key - Qt::Key_0) + gnash::key::KP_0);
      else
          c = (gnash::key::code) ((key - Qt::Key_0) + gnash::key::_0);
    }

    // All other characters between ascii 32 and 126 are simple.
    // From space (32) to slash (47):
    else if (key >= Qt::Key_Space && key <= Qt::Key_Slash) {
        c = (gnash::key::code) ((key - Qt::Key_Space) + gnash::key::SPACE);
    }

    // From colon (58) to tilde (126):
    else if (key >= Qt::Key_Colon && key <= Qt::Key_AsciiTilde) {
        c = (gnash::key::code) ((key - Qt::Key_Colon) + gnash::key::COLON);
    }

    // Function keys:
    else if (key >= Qt::Key_F1 && key <= Qt::Key_F15) {
        c = (gnash::key::code) ((key - Qt::Key_F1) + gnash::key::F1);
    }

    // Extended ascii from non-breaking (160) space to 