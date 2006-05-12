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

#ifndef _GUI_H_
#define _GUI_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>

#include "gnash.h"

namespace gnash
{

typedef bool (*callback_t)(void*);
typedef enum {IDLE_MOVIE = 0, PLAY_MOVIE = 1, RESTART_MOVIE, PAUSE_MOVIE, STOP_MOVIE, STEP_FORWARD, STEP_BACKWARD, JUMP_FORWARD, JUMP_BACKWARD, QUIT_MOVIE} movie_state_e;

extern const char *GNASH;
extern movie_state_e menu_state;


class Gui {
public:
    Gui();
    Gui(unsigned long xid, float scale, bool loop, unsigned int depth);
    virtual ~Gui();
    
    bool init(int xid, int argc, char **argv[]);
    bool createWindow(int xid, int width, int height);    
    virtual bool createWindow(int width, int height) = 0;
    virtual bool run(void *) = 0;
    virtual void resizeWindow() = 0;
    virtual bool createMenu() = 0;
    virtual bool setupEvents() = 0;
    virtual void renderBuffer() = 0;

    void setMouseX(int x)           { _mouse_x = x; }
    void setMouseY(int y)           { _mouse_y= y; }
    void setMouseButtons(int mask)  { _mouse_buttons = mask; }
    int getMouseX()                 { return _mouse_x; }
    int getMouseY()                 { return _mouse_y; }
    int getMouseButtons()           { return _mouse_buttons; }
    float getScale()                { return _scale; }
    bool loops()                    { return _loop; }

    void addMouseHandler(callback_t ptr);
    void addKeyboardHandler(callback_t ptr);
    void setXembed(int xid);

    static void menu_restart();
    static void menu_quit();
    static void menu_play();
    static void menu_pause();
    static void menu_stop();
    static void menu_step_forward();
    static void menu_step_backward();
    static void menu_jump_forward();
    static void menu_jump_backward();
    static bool advance_movie(void *data);

protected:
    bool            _loop;
    unsigned long   _xid;
    int             _width;
    int             _height;
    int             _mouse_x;
    int             _mouse_y;
    float           _scale;
    int             _mouse_buttons;
    bool            _xembed;
    int             _depth;
    std::string     _name;
    callback_t      _mouse_handler;
    callback_t      _heyboard_handler;
    unsigned int    _interval;
    render_handler* _renderer;
};
 
  
} // end of gnash namespace

// end of _GUI_H_
#endif
