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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <fltk/Item.h>
#include <fltk/Window.h>

#ifdef HAVE_X11_X_H
#include <fltk/x11.h>
#endif

#include <fltk/events.h>
#include <fltk/run.h>
#include <fltk/Cursor.h>
#include <fltk/layout.h>
#include <fltk/MenuBar.h>
#include <fltk/ItemGroup.h>
#include <fltk/file_chooser.h>

#include "fltksup.h"
#include "gui.h"
#include "VM.h"
#include "RunResources.h"

#include "Renderer.h"

using namespace std;
using namespace fltk;



namespace gnash 
{


FltkGui::FltkGui(unsigned long xid, float scale, bool loop, RunResources& r)
  : Window(0, 0),
    Gui(xid, scale, loop, r),
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
      using namespace geometry;
      Range2d<int> bounds(0, 0, _width, _height);
      _glue->render(bounds);

      return;
    }

    if (! _drawbounds_vec.size() ) { 
      return; // XXX what about Cairo?
    }
  
    for (unsigned bno=0; bno < _drawbounds_vec.size(); bno++) {
       geometry::Range2d<int>& bounds = _drawbounds_vec[bno];

       assert ( bounds.isFinite() );

       _glue->render(bounds);
    }
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
        notifyMouseClick(true);
        return true;
      case RELEASE:
        Window::handle(event);
        notifyMouseClick(false);
        return true;
      case MOVE:
      {
        if (!_xid && event_y() < static_cast<int>(_menu_height)) {
          return Window::handle(event);
        }
        notifyMouseMove(event_x(), event_y()-_menu_height);
        return Window::handle(event);
      }
      case SHORTCUT:
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

    int modifier = gnash::key::GNASH_MOD_NONE;

    unsigned long state = event_state();

    if (state & SHIFT) { 
        modifier = modifier | gnash::key::GNASH_MOD_SHIFT;
    }
    if (state & CTRL) {
        modifier = modifier | gnash::key::GNASH_MOD_CONTROL;
    }
    if (state & ALT) {
        modifier = modifier | gnash::key::GNASH_MOD_ALT;
    }

    for (int i = 0; table[i].fltkKey; i++) {
        if (key == table[i].fltkKey) {
            notify_key_event((gnash::key::code)table[i].gnashKey, modifier, 
                             true);
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
FltkGui::init(int /* argc */, char *** /*argv */)
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
#ifdef HAVE_X11_X_H
    if (_xid) {
      // Make FLTK render into an X window indicated by the XID.
      CreatedWindow::set_xid(this, _xid);
      return;
    }
#endif
    Window::create();
}

bool
FltkGui::createWindow(const char* title, int width, int height,
                      int xPosition, int yPosition)
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
#elif defined(RENDERER_OPENGL)
#error FLTK/OpenGL is currently broken. Please try again soon... 
    FltkCairoGlue _glue;
#endif
    createMenu();
    end();

    _renderer.reset(_glue->createRenderHandler());
    if (!_renderer.get()) return false;
    _runResources.setRenderer(_renderer);

    _glue->initBuffer(width, height);

    // The minimum size of the window is 1x1 pixels.
    size_range (1, 1);

    show();

    return true;
}


static void fltk_menu_open_file(Widget*, void*)
{
    const char *newfile = fltk::file_chooser("Open File", "*.swf", NULL);
    if (!newfile) {
      return;
    }

    // menu_open_file()..
}

static void fltk_menu_save_file_as(Widget*, void*)
{
    const char* savefile = file_chooser("Save File as", NULL, NULL);
    if (!savefile) {
      return;
    }

    // menu_save_file();
}

static void fltk_menu_fullscreen(Widget*, void* ptr)
{
//    GNASH_REPORT_FUNCTION;

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


static void
fltk_menu_quit(Widget*, void* ptr)
{
    FltkGui* gui = static_cast<FltkGui*>(ptr);
    gui->quit();
}

static void
fltk_menu_play(Widget*, void* ptr)
{
    FltkGui* gui = static_cast<FltkGui*>(ptr);
    gui->play();
}

static void
fltk_menu_pause(Widget*, void* ptr)
{
    FltkGui* gui = static_cast<FltkGui*>(ptr);
    gui->pause();
}

static void
fltk_menu_stop(Widget*, void* ptr)
{
    FltkGui* gui = static_cast<FltkGui*>(ptr);
    gui->stop();
}

static void
fltk_menu_restart(Widget*, void* ptr)
{
    FltkGui* gui = static_cast<FltkGui*>(ptr);
    gui->restart();
}

static void
fltk_menu_toggle_sound(Widget*, void* ptr)
{
    FltkGui* gui = static_cast<FltkGui*>(ptr);
    gui->toggleSound();
}

void
FltkGui::addMenuItems()
{
    ItemGroup* file = new ItemGroup("File");


    file->begin();
    new Item("Open",                    0, fltk_menu_open_file);
    new Item("Save as",                 0, fltk_menu_save_file_as);
    new Item("Quit",                    0, fltk_menu_quit, this);
    file->end();

    ItemGroup* edit = new ItemGroup("Edit");
    edit->begin();
    new Item("Preferences");
    edit->end();

    ItemGroup* view = new ItemGroup("View");
    view->begin();
    new Item("Double size");
    new Item("Fullscreen",              0, fltk_menu_fullscreen, this);
    view->end();

    ItemGroup* movie_ctrl = new ItemGroup("Movie control");
    movie_ctrl->begin();
    new Item("Play Movie",              0, fltk_menu_play, this);
    new Item("Pause Movie",             0, fltk_menu_pause, this);
    new Item("Stop Movie",              0, fltk_menu_stop, this);
    new Item("Restart Movie",           0, fltk_menu_restart, this);
    new Item("Toggle Sound",            0, fltk_menu_toggle_sound, this);
    movie_ctrl->end();

    ItemGroup* help = new ItemGroup("Help");
    help->begin();
    new Item("About");
    help->end();
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
FltkGui::setInvalidatedRegions(const InvalidatedRanges& ranges)
{
    // forward to renderer
    //
    // Why? Why have the region been invalidated ??
    // Was the renderer offscreen buffer also invalidated
    // (need to rerender)?
    // Was only the 'onscreen' buffer be invalidated (no need to rerender,
    // just to blit) ??
    //
    // To be safe just assume this 'invalidated' region is actually
    // the offscreen buffer, for safety, but we need to clarify this.
    //
    _renderer->set_invalidated_regions(ranges);

    _drawbounds_vec.clear();

    for (size_t rno=0; rno<ranges.size(); rno++) {

      geometry::Range2d<int> bounds = Intersection(
      _renderer->world_to_pixel(ranges.getRange(rno)),
      _validbounds);

      // it may happen that a particular range is out of the screen, which 
      // will lead to bounds==null. 
      if (bounds.isNull()) continue;

      assert(bounds.isFinite());

      _drawbounds_vec.push_back(bounds);

    }
}

// end of namespace
}

