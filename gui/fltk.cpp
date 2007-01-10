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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fltk/Item.h>
#include <fltk/ItemGroup.h>
#include <fltk/PopupMenu.h>
#include <fltk/Widget.h>
#include <fltk/ask.h>
#include <fltk/events.h>
#include <fltk/run.h>
#include <fltk/visual.h>
#include <fltk/Window.h>
#include <fltk/draw.h>
#include <fltk/x.h>
#include <fltk/damage.h>
#include <fltk/layout.h>
#include <fltk/Cursor.h>





#include "fltksup.h"
#include "gnash.h"
#include "log.h"
#include "gui.h"
#include "VM.h"

#include "render_handler.h"

using namespace std;
using namespace fltk;

namespace gnash 
{


FltkGui::FltkGui(unsigned long xid, float scale, bool loop, unsigned int depth)
  : Window(100,100,"Gnash"),
    Gui(xid, scale, loop, depth)
{
}

FltkGui::~FltkGui()
{
    GNASH_REPORT_FUNCTION;

    delete _popup_menu;
}


void
FltkGui::renderBuffer()
{
    GNASH_REPORT_FUNCTION;
    _glue.draw();
    redraw();
}

void
FltkGui::draw()
{
   GNASH_REPORT_FUNCTION;
#if 0
    if (! (damage() & DAMAGE_EXPOSE) || damage) {
      return;
    }
#endif
    rect draw_bounds(-1e10f, -1e10f, +1e10f, +1e10f);

    set_invalidated_region(draw_bounds);

    renderBuffer();
}

int
FltkGui::handle(int event)
{
    GNASH_REPORT_FUNCTION;

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
       // cout << "Captured unknown event: " << event << std::endl;
        return true; //Window::handle(event);
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
      { 0,                  gnash::key::INVALID } // Terminator
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
    GNASH_REPORT_FUNCTION;

    fltk::run();

    return false;
}

bool
FltkGui::init(int argc, char **argv[])
{
    GNASH_REPORT_FUNCTION;

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
    // XXX ensure _xid is set before this function is called
    if (_xid) {
      CreatedWindow::set_xid(this, _xid);
    } else {
      Window::create();
    }
}

bool
FltkGui::createWindow(const char* title, int width, int height)
{
    GNASH_REPORT_FUNCTION;

    resize(width, height);

    _glue.initBuffer(width, height);

    label(title);
    begin();
    createMenu();
    end();

    size_range (1, 1); // XXX

    show();

    return true;
}

bool
FltkGui::createMenu()
{
    GNASH_REPORT_FUNCTION;

    _popup_menu = new PopupMenu(0, 0, w(), h());
    _popup_menu->type(PopupMenu::POPUP3);

    _popup_menu->begin();
    new Item("Play Movie", 0, reinterpret_cast<Callback*>(menu_play));
    new Item("Pause Movie", 0, reinterpret_cast<Callback*>(menu_pause));
    new Item("Stop Movie", 0, reinterpret_cast<Callback*>(menu_stop));
    new Item("Restart Movie", 0,
                   reinterpret_cast<Callback*>(menu_restart));
    new Item("Step Forward Frame", 0,
                   reinterpret_cast<Callback*>(menu_step_forward));
    new Item("Step Backward Frame", 0,
                   reinterpret_cast<Callback*>(menu_step_backward));
    new Item("Jump Forward 10 Frames", 0,
                   reinterpret_cast<Callback*>(menu_jump_forward));
    new Item("Jump Backward 10 Frames", 0,
                   reinterpret_cast<Callback*>(menu_jump_backward));
    new Item("Toggle Sound", 0,
                   reinterpret_cast<Callback*>(menu_toggle_sound));
    new Item("Quit", 0, reinterpret_cast<Callback*>(menu_quit));
    _popup_menu->end();

    return true;
}

void
FltkGui::layout()
{
    GNASH_REPORT_FUNCTION;
    if (!(layout_damage() & ~LAYOUT_WH)) {
      // We're only interested in size changes.
      //  return;
    }
#if 0
    if (!get_current_root()) {
      
      return;
    }
#endif
    if (!VM::isInitialized()) {
      // No movie yet; don't bother resizing anything.
      return;
    }

    Window::layout();

    _glue.resize(w(), h());
    resize_view(w(), h());

    redraw();

}

void 
FltkGui::setCursor(gnash_cursor_type newcursor)
{
    //GNASH_REPORT_FUNCTION;

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

