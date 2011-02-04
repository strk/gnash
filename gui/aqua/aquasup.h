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


#ifndef __AQUASUP_H__
#define __AQUASUP_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <vector>

#include "gui.h"

#if defined(RENDERER_OPENGL)
#include "aqua_ogl_glue.h"
#endif

namespace gnash {

class DSOEXPORT AquaGui : public Gui
{
	private:
	
    std::vector< geometry::Range2d<int> > _drawbounds;

    int m_stage_width;
    int m_stage_height;
  	
  	int valid_x(int x);
  	int valid_y(int y);
  	void key_event(int key, bool down);
  	unsigned int	_timeout;

    EventLoopTimerRef* _advance_timer;

#if defined(RENDERER_OPENGL)
    AquaOglGlue		_glue;
#ifdef FIX_I810_LOD_BIAS
    float			_tex_lod_bias;
#endif
#endif
  	
	public:
	AquaGui(unsigned long xid, float scale, bool loop, RunResources& r);
    virtual ~AquaGui();
    virtual bool init(int argc, char **argv[]);
    virtual void setCursor(gnash_cursor_type newcursor);
    virtual bool createWindow(const char *title, int width, int height,
                              int xPosition = 0, int yPosition = 0);
    virtual bool run();
    virtual bool createMenu();
    virtual bool setupEvents();
    virtual void renderBuffer();
    virtual void setTimeout(unsigned int timeout);
};

}

#endif
