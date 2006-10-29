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

#ifndef __KDESUP_H__
#define __KDESUP_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"
#include "gui.h"

#include <qobject.h>
#include <qgl.h>
#include <qwidget.h>
#include <qpopupmenu.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qeventloop.h>
#include <qlabel.h>
#include <qevent.h>
#include <qkeycode.h>
#include <qmessagebox.h>

#ifdef RENDERER_OPENGL
# include <qgl.h>
# include "kde_glue_opengl.h"
#elif defined(RENDERER_CAIRO)
// #include <cairo.h>
// #include "kde_glue_cairo.h"
# error "Cairo not supported yet for KDE!"
#endif

namespace gnash
{

class DSOEXPORT KdeGui : public QGLWidget, public Gui
{
    Q_OBJECT
public:
//    KdeGui();
    KdeGui(WId embed);
    KdeGui(unsigned long xid, float scale, bool loop, unsigned int depth);
    virtual ~KdeGui();
    virtual bool init(int argc, char **argv[]);
    virtual bool createWindow(const char* windowtitle, int width, int height);
    virtual bool run();
    virtual bool createMenu();
    virtual bool setupEvents();
    virtual void renderBuffer();
    virtual void setInterval(unsigned int interval);
    virtual void setTimeout(unsigned int timeout);
public slots:
    void menuitem_restart_callback();
    void menuitem_quit_callback();
    void menuitem_play_callback();
    void menuitem_pause_callback();
    void menuitem_stop_callback();
    void menuitem_step_forward_callback();
    void menuitem_step_backward_callback();
    void menuitem_jump_forward_callback();
    void menuitem_jump_backward_callback();
    void timer_advance_movie();

    void about();    
protected:
    void resizeEvent(QResizeEvent *event);
    void timerEvent(QTimerEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseHandle(const QPoint &pos);
    
signals:
    void explain(const QString&);
private:
    KdeOpenGLGlue _glue;    
    QPopupMenu    *_qmenu;
    QApplication  *_qapp;
    QGLWidget     *_qwidget;
    QTimer        *_timer;
};

// end of namespace gnash 
}

// end of __KDESUP_H__
#endif
