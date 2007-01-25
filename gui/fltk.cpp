//
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fltk/Item.h>
#include <fltk/Window.h>
//#include <fltk/x11.h>
#include <fltk/osx.h>
#include <fltk/events.h>
#include <fltk/run.h>
#include <fltk/Cursor.h>
#include <fltk/layout.h>

#include "fltksup.h"
#include "gnash.h"
#include "gui.h"
#include "VM.h"

#include "render_handler.h"

using namespace std;
using namespace fltk;

namespace gnash 
{


FltkGui::FltkGui(unsigned long xid, float scale, bool loop, unsigned int depth)
  : Window(0, 0),
    Gui(xid, scale, loop, depth)
{
}

FltkGui::~FltkGui()
{
    delete _popup_menu;
}


void
FltkGui::renderBuffer()
{
    redraw();
}

// All drawing operations (i.e., the drawing calls the renderer makes) must take
// place in draw().
void
FltkGui::draw()
{
    // FLTK has a nice mechanism where you can set damage() to whatever you want
    // so in draw() you can check what exactly you want to redraw. But
    // unfortunately it doesn't seem to remember what bits you turn on. So I'll
    // just do it the old-fashioned way.
    static bool firstRun = true;

    if (firstRun) {
      // Redraw the whole rendering area.
      rect draw_bounds(-1e10f, -1e10f, +1e10f, +1e10f);
      set_invalidated_region(draw_bounds);
      firstRun = false;
    }

    _glue.draw();
}

int
FltkGui::handle(int event)
{
    switch (event) {
      case TIMEOUT:
        advance_movie(this);
        repeat_timeout(_interval);
        return true;
      case PUSH:
        Window::handle(event);
        notify_mouse_clicked(true, 1);
        return true;
      case RELEASE:
        Window::handle(event);
        notify_mouse_clicked(false, 1);
        return true;
      case MOVE:
      {
        int x = event_x() / _xscale;
        int y = event_y() / _yscale;
        notify_mouse_moved(x, y);
        return true;
      }
      case KEY:
        handleKey(event_key());
        return true;
      default:
        return Window::handle(event);
    }
}

void
FltkGui::handleKey(unsigned key)
{
    // TODO: there are more keys
    struct {
      unsigned              fltkKey;
      gnash::key::code      gnashKey;
    } table[] = {
      { BackSpaceKey,       gnash::key::BACKSPACE },
      { TabKey,             gnash::key::TAB },
      { ClearKey,           gnash::key::CLEAR },
      { ReturnKey,          gnash::key::ENTER },
      { LeftShiftKey,       gnash::key::SHIFT },
      { RightShiftKey,      gnash::key::SHIFT },
      { LeftCtrlKey,        gnash::key::CONTROL },
      { RightCtrlKey,       gnash::key::CONTROL },
      { LeftAltKey,         gnash::key::ALT },
      { RightAltKey,        gnash::key::ALT },
      { CapsLockKey,        gnash::key::CAPSLOCK },
      { EscapeKey,          gnash::key::ESCAPE },
      { SpaceKey,           gnash::key::SPACE },
      { PageDownKey,        gnash::key::PGDN },
      { PageUpKey,          gnash::key::PGUP },
      { HomeKey,            gnash::key::HOME },
      { EndKey,             gnash::key::END },
      { LeftKey,            gnash::key::LEFT },
      { UpKey,              gnash::key::UP },
      { RightKey,           gnash::key::RIGHT },
      { DownKey,            gnash::key::DOWN },
      { InsertKey,          gnash::key::INSERT },
      { DeleteKey,          gnash::key::DELETEKEY },
      { HelpKey,            gnash::key::HELP },
      { NumLockKey,         gnash::key::NUM_LOCK },
      { SubtractKey,        gnash::key::MINUS },
      { DivideKey,          gnash::key::SLASH },
      { 0,                  gnash::key::INVALID }
#if 0
            // These appear to be unavailable in fltk
            { bracketleft, gnash::key::LEFT_BRACKET },
            { backslash, gnash::key::BACKSLASH },
            { bracketright, gnash::key::RIGHT_BRACKET },
            { quotedbl, gnash::key::QUOTE },
            { VoidSymbol, gnash::key::INVALID }
            { SemicolonKey, gnash::key::SEMICOLON },
            { equalKey, gnash::key::EQUALS },
#endif
    };

    for (int i = 0; table[i].fltkKey; i++) {
        if (key == table[i].fltkKey) {
            gnash::notify_key_event(table[i].gnashKey, true);
            break;
        }
    }
}

bool
FltkGui::run()
{
    fltk::run();

    return true;
}

bool
FltkGui::init(int argc, char **argv[])
{
    _renderer = _glue.createRenderHandler();
    set_render_handler(_renderer);

    return true;
}

void
FltkGui::setInterval(unsigned int time)
{
    _interval = time / 1000.0;
    add_timeout (_interval);
}

void
FltkGui::create()
{
    // TODO: make the set_xid() call conditional on the availability of X11.
    if (/*_xid*/ false) {
      //CreatedWindow::set_xid(this, _xid);
    } else {
      Window::create();
    }
}

bool
FltkGui::createWindow(const char* title, int width, int height)
{
    resize(width, height);

    _glue.initBuffer(width, height);

    label(title);
    begin();
    createMenu();
    end();

    // The minimum size of the window is 1x1 pixels.
    size_range (1, 1);

    show();

    return true;
}

bool
FltkGui::createMenu()
{
    _popup_menu = new PopupMenu(0, 0, w(), h());
    _popup_menu->type(PopupMenu::POPUP3);

    _popup_menu->begin();

#define callback_cast(ptr) reinterpret_cast<Callback*>(ptr)
    new Item("Play Movie",              0, callback_cast(menu_play));
    new Item("Pause Movie",             0, callback_cast(menu_pause));
    new Item("Stop Movie",              0, callback_cast(menu_stop));
    new Item("Restart Movie",           0, callback_cast(menu_restart));
    new Item("Step Forward Frame",      0, callback_cast(menu_step_forward));
    new Item("Step Backward Frame",     0, callback_cast(menu_step_backward));
    new Item("Jump Forward 10 Frames",  0, callback_cast(menu_jump_forward));
    new Item("Jump Backward 10 Frames", 0, callback_cast(menu_jump_backward));
    new Item("Toggle Sound",            0, callback_cast(menu_toggle_sound));
    new Item("Quit",                    0, callback_cast(menu_quit));
#undef callback_cast

    _popup_menu->end();

    return true;
}

void
FltkGui::layout()
{
    if ((layout_damage() & LAYOUT_CHILD )) {
      // We're not interested in children. Sorry.
      return;
    }
    if (!VM::isInitialized()) {
      // No movie yet; don't bother resizing anything.
      return;
    }
    
    // Let FLTK update the window borders, etc.
    Window::layout();

    if ((layout_damage() & LAYOUT_WH)) {
      _glue.resize(w(), h());
      resize_view(w(), h());
    }

    // Invalidate the whole drawing area.
    rect draw_bounds(-1e10f, -1e10f, +1e10f, +1e10f);
    set_invalidated_region(draw_bounds);

    redraw();
}

void 
FltkGui::setCursor(gnash_cursor_type newcursor)
{
    fltk::Cursor* cursortype;

    switch(newcursor) {
      case gnash::CURSOR_HAND:
        cursortype = fltk::CURSOR_HAND;
        break;
      case gnash::CURSOR_INPUT:
        cursortype = fltk::CURSOR_INSERT;
        break;
      default:
        cursortype = fltk::CURSOR_DEFAULT;
    }

    cursor(cursortype);
}

void
FltkGui::set_invalidated_region(const rect& bounds)
{
    _glue.invalidateRegion(bounds);
}


// end of namespace
}

