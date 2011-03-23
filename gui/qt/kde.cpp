// kde.cpp:  K Development Environment top level window, for Gnash.
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
    // can refer to. See GnashKey.h for the keycodes and map.
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

    // Extended ascii from non-breaking (160) space to ÿ (264) is in the same
    // order.
    else if (key >= Qt::Key_nobreakspace && key <= Qt::Key_ydiaeresis) {
        c = (gnash::key::code) ((key - Qt::Key_nobreakspace) + gnash::key::NOBREAKSPACE);
    }

    else {
        // many keys don't correlate, so just use a look-up table.
        struct {
            int               qt;
            gnash::key::code  gs;
        } table[] = {
            { Qt::Key_Backspace, gnash::key::BACKSPACE },
            { Qt::Key_Tab, gnash::key::TAB },
#if QT_VERSION > 2312
            { Qt::Key_Clear, gnash::key::CLEAR },
#endif
            { Qt::Key_Return, gnash::key::ENTER },
            { Qt::Key_Enter, gnash::key::ENTER },

            { Qt::Key_Shift, gnash::key::SHIFT },
            { Qt::Key_Control, gnash::key::CONTROL },
            { Qt::Key_Alt, gnash::key::ALT },
            { Qt::Key_CapsLock, gnash::key::CAPSLOCK },

            { Qt::Key_Escape, gnash::key::ESCAPE },
            //{ Qt::Key_Space, gnash::key::SPACE },

            { Qt::Key_Next, gnash::key::PGDN },
            { Qt::Key_Prior, gnash::key::PGUP },
            { Qt::Key_Home, gnash::key::HOME },
            { Qt::Key_End, gnash::key::END },
            { Qt::Key_Left, gnash::key::LEFT },
            { Qt::Key_Up, gnash::key::UP },
            { Qt::Key_Right, gnash::key::RIGHT },
            { Qt::Key_Down, gnash::key::DOWN },
            { Qt::Key_Insert, gnash::key::INSERT },
            { Qt::Key_Delete, gnash::key::DELETEKEY },

            { Qt::Key_Help, gnash::key::HELP },
            { Qt::Key_NumLock, gnash::key::NUM_LOCK },
            //{ Qt::Key_Semicolon, gnash::key::SEMICOLON },
            //{ Qt::Key_Equal, gnash::key::EQUALS },
            //{ Qt::Key_Minus, gnash::key::MINUS },
            //{ Qt::Key_Slash, gnash::key::SLASH },
            //{ Qt::Key_BracketLeft, gnash::key::LEFT_BRACKET },
            //{ Qt::Key_Backslash, gnash::key::BACKSLASH },
            //{ Qt::Key_BracketRight, gnash::key::RIGHT_BRACKET },
            //{ Qt::Key_QuoteDbl, gnash::key::DOUBLE_QUOTE },
            { 0, gnash::key::INVALID }
        };
        
        for (int i = 0; table[i].qt != 0; i++) {
            if (key == table[i].qt) {
                c = table[i].gs;
                break;
            }
        }
    }
    
    return c;
}

int
KdeGui::qtToGnashModifier(Qt::ButtonState state)
{
    int modifier = gnash::key::GNASH_MOD_NONE;

    if (state & Qt::ShiftButton) {
        modifier = modifier | gnash::key::GNASH_MOD_SHIFT;
    }
    if (state & Qt::ControlButton) {
        modifier = modifier | gnash::key::GNASH_MOD_CONTROL;
    }
    if (state & Qt::AltButton) {
       modifier = modifier | gnash::key::GNASH_MOD_ALT;
    }

    return modifier;
}

void
KdeGui::handleKeyEvent(QKeyEvent *event, bool down)
{
    gnash::key::code c = qtToGnashKey(event);
    int mod = qtToGnashModifier(event->state());
    notify_key_event(c, mod, down);
}

void
KdeGui::resize(int width, int height)
{
    _glue.resize(width, height);
    resize_view(width, height);
}

void
KdeGui::quitUI()
{
#if 1
    _qapp->exit();
#else // dunno what this wanted to achive
#if QT_VERSION > 2312
    _qapp->eventLoop()->exit();
#endif
#endif
}


/// \brief restart the movie from the beginning
void
qwidget::menuitem_restart_callback()
{
//    GNASH_REPORT_FUNCTION;
    _godfather->restart();
}

/// \brief force redraw of current frame
void
qwidget::menuitem_refresh_callback()
{
//    GNASH_REPORT_FUNCTION;
    _godfather->refreshView();
}

/// \brief quit complete, and close the application
void
qwidget::menuitem_quit_callback()
{
//    GNASH_REPORT_FUNCTION;
    _godfather->quit();
}

/// \brief Start the movie playing from the current frame.
void
qwidget::menuitem_play_callback()
{
//    GNASH_REPORT_FUNCTION;
    _godfather->play();
}

/// \brief toggle that's playing or paused.
void
qwidget::menuitem_pause_callback()
{
//    GNASH_REPORT_FUNCTION;
    _godfather->pause();
}

/// \brief stop the movie that's playing.
void
qwidget::menuitem_stop_callback()
{
//    GNASH_REPORT_FUNCTION;
    _godfather->stop();
}

//
// Event handlers
//



void
qwidget::mouseMoveEvent(QMouseEvent *event)
{
//    GNASH_REPORT_FUNCTION;
    assert(_godfather);
    QPoint position = event->pos();

    _godfather->notifyMouseMove(position.x(), position.y());
}

qwidget::qwidget(KdeGui* godfather)
{
    _qmenu.insertItem(_("Play Movie"), this, SLOT(menuitem_play_callback()));
    _qmenu.insertItem(_("Pause Movie"), this, SLOT(menuitem_pause_callback()));
    _qmenu.insertItem(_("Stop Movie"), this, SLOT(menuitem_stop_callback()));
    _qmenu.insertItem(_("Restart Movie"), this, SLOT(menuitem_restart_callback()));
#if 0 // Presently disabled
    _qmenu.insertItem(_("Step Forward"), this, SLOT(menuitem_step_forward_callback()));
    _qmenu.insertItem(_("Step Backward"), this, SLOT( menuitem_step_backward_callback()));
    _qmenu.insertItem(_("Jump Forward"), this, SLOT(menuitem_jump_forward_callback()));
    _qmenu.insertItem(_("Jump Backward"), this, SLOT(menuitem_jump_backward_callback()));
#endif
    _qmenu.insertItem(_("Refresh"), this, SLOT(menuitem_refresh_callback()));
    _qmenu.insertItem(_("Quit Gnash"), this, SLOT(menuitem_quit_callback()));

    _godfather = godfather;

    setMouseTracking(true);  
    setFocusPolicy(QWidget::StrongFocus);
}

void 
qwidget::setInterval(unsigned int interval)
{
    startTimer(interval);
}


void
qwidget::timerEvent(QTimerEvent *)
{
    Gui::advance_movie(_godfather);
}

#if QT_VERSION > 2312
void
qwidget::contextMenuEvent(QContextMenuEvent*)
{
    _qmenu.exec(QCursor::pos());
}
#endif

void
qwidget::mousePressEvent(QMouseEvent* /* event */)
{
    _godfather->notifyMouseClick(true);
}

void
qwidget::mouseReleaseEvent(QMouseEvent* /* event */)
{
    _godfather->notifyMouseClick(false);
}

void
qwidget::keyPressEvent(QKeyEvent *event)
{
    _godfather->handleKeyEvent(event, true);
}

void
qwidget::keyReleaseEvent(QKeyEvent *event)
{
    _godfather->handleKeyEvent(event, false);
}

void
qwidget::resizeEvent(QResizeEvent *event)
{
    _godfather->resize(event->size().width(), event->size().height());
}

void 
qwidget::paintEvent(QPaintEvent *event)
{
    const QRegion& region = event->region();
    QRect rect = region.boundingRect();

    int xmin = static_cast<int> (pixelsToTwips(rect.x()-1)),
        ymin = static_cast<int> (pixelsToTwips(rect.y()-1)),
        xmax = static_cast<int> (pixelsToTwips(rect.right()+1)),
        ymax = static_cast<int> (pixelsToTwips(rect.bottom()+1));

    geometry::Range2d<int> range(xmin, ymin, xmax, ymax);
    InvalidatedRanges ranges;
    ranges.add(range);

    
    _godfather->setInvalidatedRegions(ranges);
    _godfather->renderBuffer();
}

// end of namespace gnash
}

