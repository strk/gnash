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
//

/* $Id: aquasup.h,v 1.8 2007/07/22 23:40:09 nihilus Exp $ */

#ifndef __AQUASUP_H__
#define __AQUASUP_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vector>

#include "gui.h"

#if defined(RENDERER_OPENGL)
#include "aqua_ogl_glue.h"
#endif

namespace gnash {

class AquaGui : public Gui
{
	private:
	
    std::vector< geometry::Range2d<int> > _drawbounds;

    int m_stage_width;
    int m_stage_height;
  	
  	int valid_x(int x);
  	int valid_y(int y);
  	void key_event(int key, bool down);
  	unsigned int	_timeout;

#if defined(RENDERER_OPENGL)
    AquaOglGlue		_glue;
#endif
	  	
	public:
		AquaGui();
		AquaGui(unsigned long xid, float scale, bool loop, unsigned int depth);
    virtual ~AquaGui();
    virtual bool init(int argc, char ***argv);
    virtual bool createWindow(const char* title, int width, int height);
    virtual bool run();
    virtual bool createMenu();
    virtual bool setupEvents();
    virtual void renderBuffer();
    virtual void setInvalidatedRegions(const InvalidatedRanges& ranges);
    virtual void setInterval(unsigned int interval);
    virtual void setTimeout(unsigned int timeout);
};

}

#endif