// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc.
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


#ifndef HAIKUGUI_H
#define HAIKUGUI_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "gui.h"

#ifndef RENDERER_AGG
#error "Haiku GUI needs AGG renderer"
#endif

#include "haiku_agg_glue.h"

#include <string>

#include "OS.h"


namespace gnash
{
class BeApp;



class HaikuGui : public Gui
{
    BeApp *_app;
#ifdef RENDERER_AGG
    HaikuAggGlue _glue;
#endif
    unsigned int _timeout;
    std::string _sharefilename;
public:
    HaikuGui(unsigned long xid, float scale, bool loop, RunResources& r);
    virtual ~HaikuGui();

    virtual bool init(int argc, char **argv[]);
    void init_dumpfile();
    virtual bool createWindow(const char *title, int width, int height,
                              int xPosition = 0, int yPosition = 0);
    virtual bool run();
    virtual bool createMenu();
    virtual bool createMenuBar();
    virtual bool setupEvents();
    virtual void setFullscreen();
    virtual void unsetFullscreen();
    virtual void setFullscreenEtc();
    virtual void unsetFullscreenEtc();
    virtual void renderBuffer();
    virtual void setInterval(unsigned int interval);
    virtual void setTimeout(unsigned int timeout);
    virtual void error(const std::string &msg);
    virtual bool yesno(const std::string &question);

    void GnashPulse();
    unsigned int getInterval();
    virtual void resize_view(int width, int height);
    void Shown();
    void Hidden();

    void setInvalidatedRegions(const InvalidatedRanges& ranges);
};


};

#endif

