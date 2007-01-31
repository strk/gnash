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
#include <fltk/x11.h>
#include <fltk/events.h>
#include <fltk/run.h>
#include <fltk/Cursor.h>
#include <fltk/layout.h>
#include <fltk/MenuBar.h>
#include <fltk/ItemGroup.h>
#include <fltk/file_chooser.h>

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
    Gui(xid, scale, loop, depth),
    _menu_height(_xid ? 0 : 20)
{
}

FltkGui::~FltkGui()
{
    delete _popup_menu;
}


void
FltkGui::renderBuffer()
{
    // FLTK has a nice mechanism where you can set damage() to whatever you want
    // so in draw() you can check what exactly you want to redraw. But
    // unfortunately it doesn't seem to remember what bits you turn on. So I'll
    // just do it the old-fashioned way.
    static bool firstRun = true;

    if (firstRun) {
      // Redraw the whole rendering area.
      rect draw_bounds(0, 0, _width, _height);
      setInvalidatedRegion(draw_bounds);
      firstRun = false;
    }

    _glue->redraw();
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
        if (!_xid && event_y() < _menu_height) {
          return Window::handle(event);
        }
        int x = event_x() / _xscale;
        int y = (event_y() - _menu_height) / _yscale;
        notify_mouse_moved(x, y);
        return Window::handle(event);;
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
    if (_xid) {
      CreatedWindow::set_xid(this, _xid);
    } else {
      Window::create();
    }
}

bool
FltkGui::createWindow(const char* title, int width, int height)
{
    resize(width, _menu_height + height);


    label(title);
    begin();

    if (!_xid) {
      MenuBar* menubar = new MenuBar(0, 0, width, _menu_height);
      menubar->begin();
      addMenuItems();
      menubar->end();
    }
#ifdef RENDERER_AGG
    _glue = new FltkAggGlue(0, _menu_height, width, height);
#elif defined(RENDERER_CAIRO)
#error FLTK/Cairo is currently broken. Please try again soon... 
    FltkCairoGlue _glue;
#endif
    createMenu();
    end();

    _renderer = _glue->createRenderHandler();
    set_render_handler(_renderer);

    _glue->initBuffer(width, height);

    // The minimum size of the window is 1x1 pixels.
    size_range (1, 1);

    show();

    return true;
}


static void menu_fltk_open_file()
{
    const char *newfile = fltk::file_chooser("Open File", "*.swf", NULL);
    if (!newfile) {
      return;
    }

    // menu_open_file()..
}

static void menu_fltk_save_file_as()
{
    const char* savefile = file_chooser("Save File as", NULL, NULL);
    if (!savefile) {
      return;
    }

    // menu_save_file();
}

static void full(Widget*, void* ptr)
{
    GNASH_REPORT_FUNCTION;

    static bool fullscreen = false;
    static Rectangle oldBounds;

    fullscreen = !fullscreen;

    FltkGui* gui = static_cast<FltkGui*>(ptr);
    if (fullscreen) {
      oldBounds.set(gui->x(), gui->y(), gui->w(), gui->h());
      gui->fullscreen();
    } else {
      gui->fullscreen_off(oldBounds.x(), oldBounds.y(), oldBounds.w(), oldBounds.h());
    }
}


void
FltkGui::addMenuItems()
{

#define callback_cast(ptr) reinterpret_cast<Callback*>(ptr)
    ItemGroup* file = new ItemGroup("File");
    file->begin();
    new Item("Open",                    0, callback_cast(menu_fltk_open_file));
    new Item("Save as",                 0, callback_cast(menu_fltk_save_file_as));
    //new Item("Save as..."               0, callback_cast(menu_fltk_save_as));
    new Item("Quit",                    0, callback_cast(menu_quit));
    file->end();

    ItemGroup* edit = new ItemGroup("Edit");
    edit->begin();
    new Item("Preferences");
    edit->end();

    ItemGroup* view = new ItemGroup("View");
    view->begin();
    new Item("Double size");
    new Item("Fullscreen",              0, callback_cast(full), this);
    view->end();

    ItemGroup* movie_ctrl = new ItemGroup("Movie control");
    movie_ctrl->begin();
    new Item("Play Movie",              0, callback_cast(menu_play));
    new Item("Pause Movie",             0, callback_cast(menu_pause));
    new Item("Stop Movie",              0, callback_cast(menu_stop));
    new Item("Restart Movie",           0, callback_cast(menu_restart));
    new Item("Step Forward Frame",      0, callback_cast(menu_step_forward));
    new Item("Step Backward Frame",     0, callback_cast(menu_step_backward));
    new Item("Jump Forward 10 Frames",  0, callback_cast(menu_jump_forward));
    new Item("Jump Backward 10 Frames", 0, callback_cast(menu_jump_backward));
    new Item("Toggle Sound",            0, callback_cast(menu_toggle_sound));
    movie_ctrl->end();

    ItemGroup* help = new ItemGroup("Help");
    help->begin();
    new Item("About");
    help->end();

#undef callback_cast
}



bool
FltkGui::createMenu()
{
    _popup_menu = new PopupMenu(0, 0, w(), h());
    _popup_menu->type(PopupMenu::POPUP3);

    _popup_menu->begin();

     addMenuItems();

    _popup_menu->end();

    return true;
}

void
FltkGui::layout()
{
    if (!VM::isInitialized()) {
      // No movie yet; don't bother resizing anything.
      return;
    }
    
    // Let FLTK update the window borders, etc.
    Window::layout();

    if ((layout_damage() & LAYOUT_WH)) {
      _glue->resize(w(), h() - _menu_height);
      resize_view(w(), h() - _menu_height);
    }

    // Invalidate the whole drawing area.
    rect draw_bounds(0, 0, _width, _height);
    setInvalidatedRegion(draw_bounds);

    _glue->redraw();
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
FltkGui::setInvalidatedRegion(const rect& bounds)
{
#if 0
    // temporarily disabled
    _glue->invalidateRegion(bounds);
#endif
}


// end of namespace
}

