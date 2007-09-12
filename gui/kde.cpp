// kde.cpp:  K Development Environment top level window, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
#include "config.h"
#endif


#include <qwidget.h>
#include <qmessagebox.h>
#include <qcursor.h>
#include <qxembed.h>
#include <qnamespace.h>
#include <qtimer.h>
#include <qeventloop.h>

#include "Range2d.h"

#include "gnash.h"
#include "movie_definition.h" 
#include "log.h"

#include "gui.h"
#include "kdesup.h"
#include "klash.moc"

using namespace std;

namespace gnash 
{

KdeGui::~KdeGui()
{
//    GNASH_REPORT_FUNCTION;
}


KdeGui::KdeGui(unsigned long xid, float scale, bool loop, unsigned int depth)
 : Gui(xid, scale, loop, depth)
{
}

bool
KdeGui::init(int argc, char **argv[])
{
//    GNASH_REPORT_FUNCTION;
    _qapp.reset(new QApplication(argc, *argv));
    _qwidget.reset(new qwidget(this)); 
    if (_xid) {
        QXEmbed::initialize();
        QXEmbed::embedClientIntoWindow(_qwidget.get(), _xid);
    }

    _glue.init (argc, argv);
    
    return true;
}

bool
KdeGui::createWindow(const char* windowtitle, int width, int height)
{
//    GNASH_REPORT_FUNCTION;

    _qwidget->setGeometry(0, 0, width, height);
    _qwidget->setCaption(windowtitle);

    _qapp->setMainWidget(_qwidget.get());
    _qwidget->show();

    _glue.prepDrawingArea(_qwidget.get());
    _renderer = _glue.createRenderHandler();
    if ( ! _renderer )
    {
        // something went wrong
        return false;
    }
    _glue.initBuffer(width, height);
    
    _width = width;
    _height = height;
    set_render_handler(_renderer);
    
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
    gnash::key::code c = gnash::key::INVALID;
    int key = event->key();
    
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
      if (event->state() & Qt::Keypad)
          c = (gnash::key::code) ((key - Qt::Key_0) + gnash::key::KP_0);
      else
          c = (gnash::key::code) ((key - Qt::Key_0) + gnash::key::_0);
    } else if (key >= Qt::Key_A && key <= Qt::Key_Z) {
        c = (gnash::key::code) ((key - Qt::Key_A) + gnash::key::A);
    } else if (key >= Qt::Key_F1 && key <= Qt::Key_F15) {
        c = (gnash::key::code) ((key - Qt::Key_F1) + gnash::key::F1);
    } else {
        // many keys don't correlate, so just use a look-up table.
        struct {
            int               qt;
            gnash::key::code  gs;
        } table[] = {
            { Qt::Key_Backspace, gnash::key::BACKSPACE },
            { Qt::Key_Tab, gnash::key::TAB },
            { Qt::Key_Clear, gnash::key::CLEAR },
            { Qt::Key_Return, gnash::key::ENTER },
            { Qt::Key_Enter, gnash::key::ENTER },

            { Qt::Key_Shift, gnash::key::SHIFT },
            { Qt::Key_Control, gnash::key::CONTROL },
            { Qt::Key_Alt, gnash::key::ALT },
            { Qt::Key_CapsLock, gnash::key::CAPSLOCK },

            { Qt::Key_Escape, gnash::key::ESCAPE },
            { Qt::Key_Space, gnash::key::SPACE },

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
            { Qt::Key_Semicolon, gnash::key::SEMICOLON },
            { Qt::Key_Equal, gnash::key::EQUALS },
            { Qt::Key_Minus, gnash::key::MINUS },
            { Qt::Key_Slash, gnash::key::SLASH },
            { Qt::Key_BracketLeft, gnash::key::LEFT_BRACKET },
            { Qt::Key_Backslash, gnash::key::BACKSLASH },
            { Qt::Key_BracketRight, gnash::key::RIGHT_BRACKET },
            { Qt::Key_QuoteDbl, gnash::key::QUOTE },
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
    int modifier = gnash::key::MOD_NONE;

    if (state & Qt::ShiftButton) {
        modifier = modifier | gnash::key::MOD_SHIFT;
    }
    if (state & Qt::ControlButton) {
        modifier = modifier | gnash::key::MOD_CONTROL;
    }
    if (state & Qt::AltButton) {
       modifier = modifier | gnash::key::MOD_ALT;
    }

    return modifier;
}

void
KdeGui::handleKeyEvent(QKeyEvent *event, bool down)
{
    gnash::key::code c = qtToGnashKey(event);
    int mod = qtToGnashModifier(event->state());
    notify_key_event(c, 0, mod, down);
}

void
KdeGui::resize(int width, int height)
{
    _glue.resize(width, height);
    resize_view(width, height);
}

void
KdeGui::quit()
{
    _qapp->eventLoop()->exit();
}


/// \brief restart the movie from the beginning
void
qwidget::menuitem_restart_callback()
{
//    GNASH_REPORT_FUNCTION;
    _godfather->menu_restart();
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
    _godfather->menu_play();
}

/// \brief toggle that's playing or paused.
void
qwidget::menuitem_pause_callback()
{
//    GNASH_REPORT_FUNCTION;
    _godfather->menu_pause();
}

/// \brief stop the movie that's playing.
void
qwidget::menuitem_stop_callback()
{
//    GNASH_REPORT_FUNCTION;
    _godfather->menu_stop();
}

/// \brief step forward 1 frame
void
qwidget::menuitem_step_forward_callback()
{
//    GNASH_REPORT_FUNCTION;
    _godfather->menu_step_forward();
}

/// \brief step backward 1 frame
void
qwidget::menuitem_step_backward_callback()
{
//    GNASH_REPORT_FUNCTION;
    _godfather->menu_step_backward();
}

/// \brief jump forward 10 frames
void
qwidget::menuitem_jump_forward_callback()
{
//    GNASH_REPORT_FUNCTION;
    _godfather->menu_jump_forward();
}

/// \brief jump backward 10 frames
void
qwidget::menuitem_jump_backward_callback()
{
//    GNASH_REPORT_FUNCTION;
    _godfather->menu_jump_backward();
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

    int newX = static_cast<int> (position.x() / _godfather->getXScale());
    int newY = static_cast<int> (position.y() / _godfather->getYScale());

    _godfather->notify_mouse_moved(newX, newY);
}

qwidget::qwidget(KdeGui* godfather)
{
    _qmenu.insertItem(_("Play Movie"), this, SLOT(menuitem_play_callback()));
    _qmenu.insertItem(_("Pause Movie"), this, SLOT(menuitem_pause_callback()));
    _qmenu.insertItem(_("Stop Movie"), this, SLOT(menuitem_stop_callback()));
    _qmenu.insertItem(_("Restart Movie"), this, SLOT(menuitem_restart_callback()));
    _qmenu.insertItem(_("Step Forward"), this, SLOT(menuitem_step_forward_callback()));
    _qmenu.insertItem(_("Step Backward"), this, SLOT( menuitem_step_backward_callback()));
    _qmenu.insertItem(_("Jump Forward"), this, SLOT(menuitem_jump_forward_callback()));
    _qmenu.insertItem(_("Jump Backward"), this, SLOT(menuitem_jump_backward_callback()));
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

void
qwidget::contextMenuEvent(QContextMenuEvent*)
{
    _qmenu.exec(QCursor::pos());
}

void
qwidget::mousePressEvent(QMouseEvent* /* event */)
{
    _godfather->notify_mouse_clicked(true, 1);
}

void
qwidget::mouseReleaseEvent(QMouseEvent* /* event */)
{
    _godfather->notify_mouse_clicked(false, 1);
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

    int xmin = static_cast<int> (PIXELS_TO_TWIPS(rect.x()-1)),
        ymin = static_cast<int> (PIXELS_TO_TWIPS(rect.y()-1)),
        xmax = static_cast<int> (PIXELS_TO_TWIPS(rect.right()+1)),
        ymax = static_cast<int> (PIXELS_TO_TWIPS(rect.bottom()+1));

    geometry::Range2d<int> range(xmin, ymin, xmax, ymax);
    InvalidatedRanges ranges;
    ranges.add(range);

    
    _godfather->setInvalidatedRegions(ranges);
    _godfather->renderBuffer();
}

// end of namespace gnash
}

