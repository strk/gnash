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
//
//


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <qgl.h>
#include <qtimer.h>
#include <qwidget.h>
#include <qmessagebox.h>
#include <qcursor.h>

#include "gnash.h"
#include "movie_definition.h" 
#include "log.h"

#include "gui.h"
#include "kdesup.h"
#include "klash.moc"

//#include <cstdio>
#include <X11/keysym.h>

using namespace std;

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
}

namespace gnash 
{

KdeGui::~KdeGui()
{
//    GNASH_REPORT_FUNCTION;
    delete _qwidget;
}


KdeGui::KdeGui(unsigned long xid, float scale, bool loop, unsigned int depth)
 : Gui(xid, scale, loop, depth)
{
    GNASH_REPORT_FUNCTION;


}

bool
KdeGui::init(int argc, char **argv[])
{
    _qapp = new QApplication(argc, *argv);
    _qwidget = new qwidget(_xid, this); 

//    GNASH_REPORT_FUNCTION;
    _glue.init (argc, argv);
    
    return true;
}

bool
KdeGui::createWindow(const char* /*windowtitle*/, int width, int height)
{
    GNASH_REPORT_FUNCTION;

    _qwidget->makeCurrent();
    _qwidget->setGeometry(0, 0, width, height);
    _qapp->setMainWidget(_qwidget);
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
KdeGui::setInterval(unsigned int interval)
{
    GNASH_REPORT_FUNCTION;
    _qwidget->setInterval(interval);
    
}

bool
KdeGui::run()
{
    GNASH_REPORT_FUNCTION;
 //   _qwidget->connect(&_qapp, SIGNAL(lastWindowClosed()), &_qapp, SLOT(quit()));
    
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
#if 0
    _qapp->closeAllWindows();
    _qapp->quit();
 #endif

    exit(0);
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
    GNASH_REPORT_FUNCTION;
    assert(_godfather);
    QPoint position = event->pos();

    _godfather->notify_mouse_moved(position.x(), position.y());
}

qwidget::qwidget(WId embed, KdeGui* godfather)
  : QGLWidget(0, "hi")
{
    create (embed);

    _qmenu.insertItem("Play Movie", this, SLOT(menuitem_play_callback()));
    _qmenu.insertItem("Pause Movie", this, SLOT(menuitem_pause_callback()));
    _qmenu.insertItem("Stop Movie", this, SLOT(menuitem_stop_callback()));
    _qmenu.insertItem("Restart Movie", this, SLOT(menuitem_restart_callback()));
    _qmenu.insertItem("Step Forward", this, SLOT(menuitem_step_forward_callback()));
    _qmenu.insertItem("Step Backward", this, SLOT( menuitem_step_backward_callback()));
    _qmenu.insertItem("Jump Forward", this, SLOT(menuitem_jump_forward_callback()));
    _qmenu.insertItem("Jump Backward", this, SLOT(menuitem_jump_backward_callback()));
    _qmenu.insertItem("Quit Gnash", this, SLOT(menuitem_quit_callback()));

    _godfather = godfather;

    setMouseTracking(true);  
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
qwidget::mousePressEvent(QMouseEvent *event)
{
    _godfather->notify_mouse_clicked(true, 1);
}

void
qwidget::mouseReleaseEvent(QMouseEvent *event)
{
    _godfather->notify_mouse_clicked(false, 1);
}


void
qwidget::resizeEvent(QResizeEvent *event)
{
    _godfather->resize_view(int(event->size().width()), int(event->size().height()));
    
}

// end of namespace gnash
}

