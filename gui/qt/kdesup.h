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

#ifndef GNASH_KDESUP_H
#define GNASH_KDESUP_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "gui.h"

#include <qapplication.h>
#include <qpopupmenu.h>

#ifdef RENDERER_OPENGL
# include <qgl.h>
# include "kde_glue_opengl.h"
# define WIDGETCLASS QGLWidget
# define GLUE KdeOpenGLGlue
#elif defined(RENDERER_CAIRO)
// #include <cairo.h>
// #include "kde_glue_cairo.h"
# error "Cairo not supported yet for KDE!"
#elif defined(RENDERER_AGG)
# include "kde_glue_agg.h"
# define WIDGETCLASS QWidget
# define GLUE KdeAggGlue
#endif


namespace gnash
{


class KdeGui;

class qwidget : public WIDGETCLASS
{
    Q_OBJECT
public:
    qwidget(KdeGui* godfather);
    void setInterval(unsigned int interval);
protected:
#if QT_VERSION > 2312
    void contextMenuEvent(QContextMenuEvent *event);
#endif
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent( QMouseEvent * );
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void timerEvent(QTimerEvent *);
    void resizeEvent(QResizeEvent *event);
    void paintEvent (QPaintEvent *event);
public slots:
    void menuitem_restart_callback();
    void menuitem_refresh_callback();
    void menuitem_quit_callback();
    void menuitem_play_callback();
    void menuitem_pause_callback();
    void menuitem_stop_callback();
private:
    QPopupMenu    _qmenu;
    KdeGui*       _godfather;
};


class DSOEXPORT KdeGui :  public Gui
{

public:
    KdeGui(unsigned long xid, float scale, bool loop, RunResources& r);
    virtual ~KdeGui();
    virtual bool init(int argc, char **argv[]);
    virtual bool createWindow(const char* windowtitle, int width, int height,
                              int xPosition = 0, int yPosition = 0);
    virtual bool run();
    virtual bool createMenu();
    virtual bool setupEvents();
    virtual void renderBuffer();
    virtual void setInterval(unsigned int interval);
    virtual void setTimeout(unsigned int timeout);
    virtual void handleKeyEvent(QKeyEvent *event, bool down);
    virtual void setCursor(gnash_cursor_type newcursor);
    void setInvalidatedRegions(const InvalidatedRanges& ranges);
    void resize(int width, int height);
    void quitUI();
 private:
    std::unique_ptr<QApplication>  _qapp;
    std::unique_ptr<qwidget>       _qwidget;
    GLUE                         _glue;    

    gnash::key::code qtToGnashKey(QKeyEvent *event);
    int qtToGnashModifier(Qt::ButtonState state);
};

// end of namespace gnash 
}

// end of __KDESUP_H__
#endif
