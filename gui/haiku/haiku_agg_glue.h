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

#ifndef HAIKU_AGG_GLUE_H
#define HAIKU_AGG_GLUE_H

#include <vector>
#include <boost/cstdint.hpp> // for boost::?int??_t 
#include <Renderer.h>

#include <SupportDefs.h>

class BWindow;

namespace gnash
{

class Gui;
class BeV;

/* copy in plugin.cpp */
const int GNASH_MOUSE_CLICKED = 'GMCL';
const int GNASH_MOUSE_MOVED = 'GMMV';
const int GNASH_KEY_EVENT = 'GKKE';
const int GNASH_RESIZE = 'GRSZ';
const int GNASH_HIDDEN = 'GHID';
const int GNASH_SHOWN = 'GSHN';


const int GNASH_SET_FULLSCREEN = 'GSFS';
const int GNASH_UNSET_FULLSCREEN = 'GUFS';

class HaikuAggGlue
{
  public:
    HaikuAggGlue(Gui *gui, unsigned long xid);
    virtual ~HaikuAggGlue();

    bool init(int argc, char **argv[], BWindow **win, std::string sharefilename);
    Renderer* createRenderHandler(int depth);
    void setInvalidatedRegions(const InvalidatedRanges& ranges);
    void ViewNoMore();
    void ViewNeeded();
    bool prepDrawingArea(int width, int height, boost::uint32_t sdl_flags);
    boost::uint32_t maskFlags(boost::uint32_t sdl_flags);
    void render();
    void render(int minx, int miny, int maxx, int maxy);
    void Shown();
    void Hidden();
    //void BlankScreen();
  private:
    unsigned char   *_offscreenbuf;
    unsigned char   *_sharebuf;
    int _width, _height;
    unsigned int _bufsize;
    Renderer  *_agg_renderer;
    
    geometry::Range2d<int> _validbounds;
    std::vector< geometry::Range2d<int> > _drawbounds;
    int _bpp;
    BWindow **_win;
    BeV *_view;
    Gui *_gui;
    unsigned long _xid;
    std::string _sharefilename;
    int _sharefd;
    bool _viewhidden;
};

}

#endif

