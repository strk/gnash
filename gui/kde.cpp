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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qapplication.h>
#include <qgl.h>
#include <qtimer.h>
#include <qwidget.h>
#include <qpopupmenu.h>

#include "gnash.h"
#include "movie_definition.h" 
#include "log.h"

#include "gui.h"
#include "kdesup.h"
#include "klash.moc"

#include <iostream>
#include <X11/keysym.h>

using namespace std;

namespace gnash 
{

KdeGui::KdeGui(WId embed)
{
//    GNASH_REPORT_FUNCTION;
    create (embed);
    _qwidget = this;
    _qmenu = new QPopupMenu(this);
    _qmenu->insertItem("Play Movie", this, SLOT(menuitem_play_callback()));
    _qmenu->insertItem("Pause Movie", this, SLOT(menuitem_pause_callback()));
    _qmenu->insertItem("Stop Movie", this, SLOT(menuitem_stop_callback()));
    _qmenu->insertItem("Restart Movie", this, SLOT(menuitem_restart_callback()));
    _qmenu->insertItem("Step Forward", this, SLOT(menuitem_step_forward_callback()));
    _qmenu->insertItem("Step Backward", this, SLOT( menuitem_step_backward_callback()));
    _qmenu->insertItem("Jump Forward", this, SLOT(menuitem_jump_forward_callback()));
    _qmenu->insertItem("Jump Backward", this, SLOT(menuitem_jump_backward_callback()));
    _qmenu->insertItem("Quit Gnash", this, SLOT(menuitem_quit_callback()));
    

}

void
KdeGui::contextMenuEvent(QContextMenuEvent*)
{
//    GNASH_REPORT_FUNCTION;
//    printf("Got Right Click!\n");
    _qmenu->exec();
}

void
KdeGui::about()
{
//    GNASH_REPORT_FUNCTION;
    QMessageBox::about(this, "Klash", "The Gnash Flash player for KDE.\n");
}

KdeGui::~KdeGui()
{
//    GNASH_REPORT_FUNCTION;
}

KdeGui::KdeGui(unsigned long xid, float scale, bool loop, unsigned int depth)
 : Gui(xid, scale, loop, depth)
{
//    GNASH_REPORT_FUNCTION;
}


bool
KdeGui::init(int argc, char **argv[])
{
//    GNASH_REPORT_FUNCTION;

    _glue.init (argc, argv);
    
    return true;
}

bool
KdeGui::createWindow(int width, int height)
{
//    GNASH_REPORT_FUNCTION;

    _qwidget = new KdeGui(_xid);
    _qwidget->makeCurrent();
    _qwidget->setGeometry(0, 0, width, height);
    _qwidget->show();

    _glue.prepDrawingArea(_qwidget);
    
    _width = width;
    _height = height;
    _renderer = _glue.createRenderHandler();
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
KdeGui::setTimeout(unsigned int timeout)
{
//    GNASH_REPORT_FUNCTION;
//    _timeout = timeout;
}

void
KdeGui::setCallback(unsigned int interval)
{
//    GNASH_REPORT_FUNCTION;
    _interval = interval;
    
    _timer = new QTimer(this);
    connect(_timer, SIGNAL(timeout()), this, SLOT(timer_advance_movie()));
    _timer->start(200);
}

void
KdeGui::timer_advance_movie()
{
//    GNASH_REPORT_FUNCTION;
    this->advance_movie(static_cast<Gui *>(this));
}

void
KdeGui::timerEvent(QTimerEvent *event)
{
//    GNASH_REPORT_FUNCTION;
    if (event->timerId() == _timer->timerId()) {
        // advance the movie
    } else {
        QWidget::timerEvent(event);
    }
}

bool
KdeGui::run(void *arg)
{
//    GNASH_REPORT_FUNCTION;

    QApplication *_qapp = (QApplication *)arg;
    connect(_qapp, SIGNAL(lastWindowClosed()), _qapp, SLOT(quit()));
    
    _qapp->exec();

    return true;
}

bool
KdeGui::createMenu()
{
    GNASH_REPORT_FUNCTION;

    return true;
}

bool
KdeGui::setupEvents()
{
  GNASH_REPORT_FUNCTION;

  return true;
}


/// \brief restart the movie from the beginning
void
KdeGui::menuitem_restart_callback()
{
//    GNASH_REPORT_FUNCTION;
    menu_restart();
}

/// \brief quit complete, and close the application
void
KdeGui::menuitem_quit_callback()
{
//    GNASH_REPORT_FUNCTION;
    _qapp->closeAllWindows();
    _qapp->quit();
}

/// \brief Start the movie playing from the current frame.
void
KdeGui::menuitem_play_callback()
{
//    GNASH_REPORT_FUNCTION;
    menu_play();
}

/// \brief toggle that's playing or paused.
void
KdeGui::menuitem_pause_callback()
{
//    GNASH_REPORT_FUNCTION;
    menu_pause();
}

/// \brief stop the movie that's playing.
void
KdeGui::menuitem_stop_callback()
{
//    GNASH_REPORT_FUNCTION;
    menu_stop();
}

/// \brief step forward 1 frame
void
KdeGui::menuitem_step_forward_callback()
{
//    GNASH_REPORT_FUNCTION;
    menu_step_forward();
}

/// \brief step backward 1 frame
void
KdeGui::menuitem_step_backward_callback()
{
//    GNASH_REPORT_FUNCTION;
    menu_step_backward();
}

/// \brief jump forward 10 frames
void
KdeGui::menuitem_jump_forward_callback()
{
//    GNASH_REPORT_FUNCTION;
    menu_jump_forward();
}

/// \brief jump backward 10 frames
void
KdeGui::menuitem_jump_backward_callback()
{
//    GNASH_REPORT_FUNCTION;
    menu_jump_backward();
}

//
// Event handlers
//

void
KdeGui::resizeEvent(QResizeEvent *event)
{
//    GNASH_REPORT_FUNCTION;
    resize_view(int(event->size().width()), int(event->size().height()));
    
}

void
KdeGui::mouseMoveEvent(QMouseEvent *event)
{
    GNASH_REPORT_FUNCTION;
//    mouseHandle(event->pos());
}


void
KdeGui::mousePressEvent(QMouseEvent *event)
{
    GNASH_REPORT_FUNCTION;
//     if (event->button() == QMouseEvent::LeftButton) {
//         mouseHandle( event->pos() );
//     }
}

void
KdeGui::mouseHandle(const QPoint &pos)
{
    GNASH_REPORT_FUNCTION;
    
//     int i = pos2index(pos.x() );
//     int j = pos2index(pos.y() );
//    setPoint( i, j );
}




// end of namespace gnash
}
