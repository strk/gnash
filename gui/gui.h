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
// Linking Gnash statically or dynamically with other modules is making
// a combined work based on Gnash. Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
// 
// In addition, as a special exception, the copyright holders of Gnash give
// you permission to combine Gnash with free software programs or
// libraries that are released under the GNU LGPL and/or with Mozilla, 
// so long as the linking with Mozilla, or any variant of Mozilla, is
// through its standard plug-in interface. You may copy and distribute
// such a system following the terms of the GNU GPL for Gnash and the
// licenses of the other code concerned, provided that you include the
// source code of that other code when and as the GNU GPL requires
// distribution of source code. 
// 
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is
// their choice whether to do so.  The GNU General Public License gives
// permission to release a modified version without this exception; this
// exception also makes it possible to release a modified version which
// carries forward this exception.
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

typedef bool (*callback_t)(int x);
typedef enum {IDLE_MOVIE, PLAY_MOVIE, RESTART_MOVIE, PAUSE_MOVIE, STOP_MOVIE, STEP_FORWARD, STEP_BACKWARD, JUMP_FORWARD, JUMP_BACKWARD, QUIT_MOVIE} movie_state_e;

extern const char *GNASH;
extern movie_state_e menu_state;
 
class Gui {
public:
    Gui();
    Gui(int argc, char **argv[]);
    Gui(int x, int y, int width, int height, const char *label);
    virtual ~Gui();
    
    virtual bool init(int xid, int argc, char **argv[]) = 0;
    virtual bool init(int argc, char **argv[]) = 0;
    virtual bool createWindow(int width, int height, long int xid) = 0;
//    virtual bool checkEvents(movie_interface *movie);
    
    virtual bool poll(gnash::movie_interface* m, gnash::movie_definition* md) = 0;
    virtual bool run() = 0;
    virtual void resizeWindow() = 0;
    virtual bool createMenu() = 0;
    virtual bool setupEvents() = 0;
    virtual void startGL() = 0;
    virtual void endGL() = 0;
    virtual void drawTestGraphic() = 0;
    
    void setMouseX(int x) { _mouse_x = x; };
    void setMouseY(int y) { _mouse_y= y; };
    void setMouseButtons(int mask) { _mouse_buttons = mask; };
    int getMouseX() { return _mouse_x; };
    int getMouseY() { return _mouse_y; };
    int getMouseButtons() { return _mouse_buttons; };
    
    void addMouseHandler(callback_t ptr);
    void addKeyboardHandler(callback_t ptr);
    void setXembed(int xid);

protected:
    int         _xid;
    int         _width;
    int         _height;
    int         _mouse_x;
    int         _mouse_y;
    int         _mouse_buttons;
    bool        _xembed;
    int         _depth;
    std::string _name;
    callback_t _mouse_handler;
    callback_t _heyboard_handler;
};
 
  
} // end of gnash namespace

// end of _GUI_H_
#endif
