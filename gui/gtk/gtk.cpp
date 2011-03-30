// gtk.cpp: Gnome ToolKit graphical user interface, for Gnash.
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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "gtksup.h"
#include "revno.h"

#include <iostream>
#include <string>
#include <utility>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#ifdef HAVE_VA_VA_H
# include <va/va.h>
# include "vaapi_utils.h"
#endif
#ifdef HAVE_VA_VA_X11_H
# include <va/va_x11.h>
#endif
#ifdef HAVE_X11
# include <X11/keysym.h>
# include <gdk/gdkx.h>
# include <X11/Xlib.h>
# ifdef HAVE_XV
#  include <X11/extensions/Xv.h>
#  include <X11/extensions/Xvlib.h>
# endif
#endif

#include "log.h"
#include "gui.h"
#include "rc.h"
#include "sound_handler.h"
#include "Renderer.h"
#include "RunResources.h"
#include "VM.h"
#include "GnashEnums.h"
#include "gtk_canvas.h"

#ifdef HAVE_VA_VA_H
extern VAStatus va_getDriverName(VADisplay dpy, char **driver_name);
#endif

namespace gnash 
{

// Forward declarations
namespace {

    // Menu Item callbacks
    void menuSound(GtkMenuItem *menuitem, gpointer instance);
    void menuFullscreen(GtkMenuItem *menuitem, gpointer instance);
    void menuRestart(GtkMenuItem *menuitem, gpointer instance);
    void menuQuit(GtkMenuItem *menuitem, gpointer instance);
    void menuPlay(GtkMenuItem *menuitem, gpointer instance);
    void menuPause(GtkMenuItem *menuitem, gpointer instance);
    void menuStop(GtkMenuItem *menuitem, gpointer instance);
    void menuAbout(GtkMenuItem *menuitem, gpointer instance);
    void menuOpenFile(GtkMenuItem *menuitem, gpointer instance);
    void menuPreferences(GtkMenuItem *menuitem, gpointer instance);
    void menuMovieInfo(GtkMenuItem *menuitem, gpointer instance);
    void menuRefreshView(GtkMenuItem *menuitem, gpointer instance);
    void menuShowUpdatedRegions(GtkMenuItem *menuitem, gpointer instance); 
    void menuQualityLow(GtkMenuItem *menuitem, gpointer instance); 
    void menuQualityMedium(GtkMenuItem *menuitem, gpointer instance); 
    void menuQualityHigh(GtkMenuItem *menuitem, gpointer instance); 
    void menuQualityBest(GtkMenuItem *menuitem, gpointer instance);

    void timeoutQuit(gpointer data);

    // Event handlers
    gboolean realizeEvent(GtkWidget *widget, GdkEvent *event, gpointer data);
    gboolean deleteEvent(GtkWidget *widget, GdkEvent *event, gpointer data);
    gboolean configureEvent(GtkWidget *widget, GdkEventConfigure *event,
                                    gpointer data);
    gboolean keyPressEvent(GtkWidget *widget, GdkEventKey *event,
                                    gpointer data);
    gboolean keyReleaseEvent(GtkWidget *widget, GdkEventKey *event,
                                    gpointer data);
    gboolean buttonPressEvent(GtkWidget *widget, GdkEventButton *event,
                                       gpointer data);
    gboolean buttonReleaseEvent(GtkWidget *widget, GdkEventButton *event,
                                         gpointer data);
    gboolean mouseWheelEvent(GtkWidget *widget, GdkEventScroll *event,
                                       gpointer data);
    gboolean motionNotifyEvent(GtkWidget *widget, GdkEventMotion *event,
                                        gpointer data);
    gboolean visibilityNotifyEvent(GtkWidget *widget, GdkEventVisibility *event,
                                   gpointer data);
    gint popupHandler(GtkWidget *widget, GdkEvent *event);    

    gint popupHandlerAlt(GtkWidget *widget, GdkEvent *event);    

    void openFile(GtkWidget *widget, gpointer data);

    void addPixmapDirectory(const gchar *directory);
    
    void addGnashIcon(GtkWindow* window);

    gchar* findPixmapFile(const gchar *filename);
    
    GdkPixbuf* createPixbuf(const gchar *filename);

    key::code gdk_to_gnash_key(guint key);

    int gdk_to_gnash_modifier(int key);

    //for use in popupHandler
    bool _showMenuState;

}

GtkGui::~GtkGui()
{
}

GtkGui::GtkGui(unsigned long xid, float scale, bool loop, RunResources& r)
    :
    Gui(xid, scale, loop, r)
    ,_window(0)
    ,_resumeButton(0)
    ,_overlay(0)
    ,_canvas(0)
    ,_visible(true)
    ,_popup_menu(0)
    ,_popup_menu_alt(0)
    ,_menubar(0)
    ,_vbox(0)
    ,_advanceSourceTimer(0)
{
}

bool
GtkGui::init(int argc, char **argv[])
{
#ifdef HAVE_X11
    if (!XInitThreads()) {
        log_debug("Failed to initialize X threading support\n");
        return false;
    }
#endif
    gtk_init(&argc, argv);

    addPixmapDirectory (PKGDATADIR);

    if (_xid) {
#ifdef _WIN32
        _window = gtk_plug_new((void *)_xid);
#else
        _window = gtk_plug_new(_xid);
#endif
        log_debug (_("Created XEmbedded window"));
    } else {
        _window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        log_debug (_("Created top level window"));
    }
    
    addGnashIcon(GTK_WINDOW(_window));

    std::string hwaccel = _runResources.getHWAccelBackend();
    std::string renderer = _runResources.getRenderBackend();

    if (renderer == "opengl") {
        // See if our X11 server supports the DRI extension, otherwise
        // there is no point in trying to use OpenGL.
        bool dri = false;
        if (checkX11Extension("DRI")) {
            log_debug("DRI extension found");
            dri = true;
        }
        bool glx = false;
        // See if our X11 server supports the GLX extension, otherwise
        // there is no point in trying to use OpenGL.
        if (checkX11Extension("GLX")) {
            log_debug("GLX extension found");
            glx = true;
        }
        // If we don't have these extensions, don't bother with OpenGl,
        // drop back to AGG.
        if (!glx || !dri) {
            g_warning("This system lacks a hardware OpenGL driver!");
        }
    }

    // Gnash can only use the XVideo extension if our X server supports it.
    if (hwaccel == "xv") {
        // See if our X11 server supports the Xvideo extension, otherwise
        // there is no point in trying to use Xvideo for scaling.
        if (checkX11Extension("XVideo")) {
            log_debug("Xvideo extension found");
        }
    }

    _canvas = gnash_canvas_new();
    gnash_canvas_setup(GNASH_CANVAS(_canvas), hwaccel, renderer, argc, argv);
    // Increase reference count to prevent its destruction (which could happen
    // later if we remove it from its container).
    g_object_ref(G_OBJECT(_canvas));

    _resumeButton = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(_resumeButton),
            gtk_label_new(_("Click to play")));
    gtk_widget_show_all(_resumeButton);

    // Same here.
    g_object_ref(G_OBJECT(_resumeButton));

    // This callback indirectly results in playHook() being called.
    g_signal_connect(_resumeButton, "clicked", G_CALLBACK(menuPlay), this);

    createMenu();
    createMenuAlt();

    // A vertical box is used to allow display of the menu bar and paused widget
    _vbox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(_vbox);
    gtk_container_add(GTK_CONTAINER(_window), _vbox);

#if defined(USE_MENUS) 
    if ( ! _xid ) {
        createMenuBar();
    }
#endif

    gtk_box_pack_start(GTK_BOX(_vbox), _canvas, TRUE, TRUE, 0);

    setupEvents();

    gtk_widget_realize(_window);
    gtk_widget_show(_canvas);
    gtk_widget_show(_window);
    
    _renderer = gnash_canvas_get_renderer(GNASH_CANVAS(_canvas));
    _runResources.setRenderer(_renderer);

    // The first time stop() was called, stopHook() might not have had a chance
    // to do anything, because GTK+ wasn't garanteed to be initialised.
    //if (isStopped()) stopHook();

    return true;
}

bool
GtkGui::run()
{
    // Kick-start before setting the interval timeout
    advance_movie(this);
    
    gtk_main();
    return true;
}

void
GtkGui::setTimeout(unsigned int timeout)
{
    g_timeout_add(timeout, (GSourceFunc)timeoutQuit, this);
}


void
GtkGui::error(const std::string& msg)
{
    
    RcInitFile& rcfile = RcInitFile::getDefaultInstance();
    
    if (!rcfile.popupMessages()) return;

    GtkWidget* popup = gtk_dialog_new_with_buttons("Gnash Error",
            GTK_WINDOW(_window),
            static_cast<GtkDialogFlags>(GTK_DIALOG_DESTROY_WITH_PARENT),
            GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);

    g_signal_connect_swapped(popup, "response", G_CALLBACK(gtk_widget_destroy),
            popup);

#if GTK_CHECK_VERSION(2,14,0)
    GtkWidget* content = gtk_dialog_get_content_area(GTK_DIALOG(popup));
#else
    GtkWidget* content = GTK_DIALOG(popup)->vbox;
#endif

    GtkWidget* label = gtk_label_new(msg.c_str());
    gtk_widget_set_size_request(label, 400, 200);
    gtk_label_set_line_wrap(GTK_LABEL(label), true);
    gtk_box_pack_start(GTK_BOX(content), label, false, false, 0);
    gtk_widget_show_all(popup);
} 
    
void
GtkGui::setClipboard(const std::string& copy)
{
    GtkClipboard* cb = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_clear(cb);
    gtk_clipboard_set_text(cb, copy.c_str(), copy.size());
}

void
GtkGui::setFullscreen()
{

    if (_fullscreen) return;

    // Plugin
    if (_xid) {
    
        // Create new window and make fullscreen
        _overlay = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        addGnashIcon(GTK_WINDOW(_overlay));
        gtk_window_fullscreen(GTK_WINDOW(_overlay));
        
        // Reparent drawing area from GtkPlug to fullscreen window
        gtk_widget_realize(_overlay);      
        gtk_widget_reparent(_vbox, _overlay);
        
        // Apply key event callbacks to the new window.
        setupWindowEvents();

        gtk_widget_show(_overlay);
    }
    
    // Stand-alone
    else {
    
        // This is a hack to fix another hack (see createWindow). If the minimum
        // size (size request) is larger than the screen, fullscreen messes up.
        // This way allows the drawing area to be shrunk, which is what we
        // really want, but not only after we've gone fullscreen.
        // It could be a good hack if it were done earlier.
        // There really doesn't seem to be a proper way of setting the
        // starting size of a widget but allowing it to be shrunk.
        gtk_widget_set_size_request(_canvas, -1, -1);
        gtk_window_fullscreen(GTK_WINDOW(_window));

        showMenu(false);
    }
    
    _fullscreen = true;
}

void
GtkGui::unsetFullscreen()
{
    if (!_fullscreen) return;
    
    // Plugin
    if (_xid) {
        gtk_widget_reparent(_vbox, _window);
        
        // Apply key event callbacks to the plugin instance.
        setupWindowEvents();
        if (_overlay) {
            gtk_widget_destroy(_overlay);
        }        
    }
    else {
        // Stand-alone
        gtk_window_unfullscreen(GTK_WINDOW(_window));
        showMenu(true);
    }
    
    _fullscreen = false;
}

void
GtkGui::hideMenu()
{
    // Not showing menu anyway if it's a plugin
    if (_fullscreen || _xid) return;

    // Stand-alone
    showMenu(false);
}


void 
GtkGui::setCursor(gnash_cursor_type newcursor)
{

    if (!_mouseShown) return;

    GdkCursorType cursortype;

    switch (newcursor)
    {
        case CURSOR_HAND:
            cursortype = GDK_HAND2;
            break;
        case CURSOR_INPUT:
            cursortype = GDK_XTERM;
            break;
        default:
            cursortype = GDK_LAST_CURSOR;
    }
  
    GdkCursor* gdkcursor = NULL;
  
    if (cursortype != GDK_LAST_CURSOR) {
         gdkcursor = gdk_cursor_new(cursortype);
    }

    // The parent of _drawingArea is different for the plugin in fullscreen
    gdk_window_set_cursor(_canvas->window, gdkcursor);
  
    if (gdkcursor) {
        gdk_cursor_unref(gdkcursor);
    }
}

// Returns whether the mouse was visible before call.
bool
GtkGui::showMouse(bool show)
{

    bool state = _mouseShown;

    if (show == _mouseShown) return state;

    if (!show) {
        GdkPixmap *pixmap;
        GdkColor *color;

        color = g_new0(GdkColor, 1);
        pixmap = gdk_pixmap_new(NULL, 1, 1, 1);
        GdkCursor* cursor = gdk_cursor_new_from_pixmap(pixmap, pixmap,
                                                    color, color, 0, 0);

        gdk_window_set_cursor (_canvas->window, cursor);

        g_free(color);
        g_object_unref(pixmap);    
        gdk_cursor_unref(cursor);

        _mouseShown = false;

    }
    else if (show) _mouseShown = true;
    
    return state;
}

void
GtkGui::showMenu(bool show)
{
    RcInitFile& rcfile = RcInitFile::getDefaultInstance();

    // If we allow the swf author to set Stage.showMenu
    if( !rcfile.ignoreShowMenu() ) {
        _showMenuState = show;
    }

#ifdef USE_MENUS
    if (!_menubar) {
        return;
    }

    if (show) {
        gtk_widget_show(_menubar);
    } else {
        gtk_widget_hide(_menubar);
    }
#endif
    
}

double
GtkGui::getPixelAspectRatio() const
{
    GdkScreen* screen = gdk_screen_get_default();

    const std::pair<int, int> res = screenResolution();

    // Screen size / number of pixels = pixel size.
    // The physical size of the screen may be reported wrongly by gdk (from X),
    // but it's the best we have. This method agrees with the pp in my case.
    double pixelAspectRatio =
        (gdk_screen_get_height_mm(screen) / static_cast<double>(res.first)) / 
        (gdk_screen_get_width_mm(screen) / static_cast<double>(res.second));
    return pixelAspectRatio;
}

std::pair<int, int>
GtkGui::screenResolution() const
{
    return std::make_pair(gdk_screen_width(), gdk_screen_height());
}

double
GtkGui::getScreenDPI() const
{
#if GTK_CHECK_VERSION(2,10,0)
    GdkScreen* screen = gdk_screen_get_default();
    return gdk_screen_get_resolution(screen);
#else
    return 0;
#endif
}

// private
void
GtkGui::setupWindowEvents()
{
    g_signal_connect(gtk_widget_get_toplevel(_canvas),
            "delete_event", G_CALLBACK(deleteEvent), this);
    g_signal_connect(gtk_widget_get_toplevel(_canvas),
            "key_press_event", G_CALLBACK(keyPressEvent), this);
    g_signal_connect(gtk_widget_get_toplevel(_canvas),
            "key_release_event", G_CALLBACK(keyReleaseEvent), this);
}

// public virtual
bool
GtkGui::setupEvents()
{

    setupWindowEvents();

    gtk_widget_add_events(_canvas, GDK_EXPOSURE_MASK
                        | GDK_VISIBILITY_NOTIFY_MASK
                        | GDK_BUTTON_PRESS_MASK
                        | GDK_BUTTON_RELEASE_MASK
                        | GDK_KEY_RELEASE_MASK
                        | GDK_KEY_PRESS_MASK        
                        | GDK_POINTER_MOTION_MASK);
  
    _showMenuState = true; //Default for showMenu

    g_signal_connect_swapped(_canvas, "button_press_event",
                            G_CALLBACK(popupHandler), _popup_menu);
  
    g_signal_connect_swapped(_canvas, "button_press_event",
                            G_CALLBACK(popupHandlerAlt), _popup_menu_alt);

    g_signal_connect(_canvas, "button_press_event",
                   G_CALLBACK(buttonPressEvent), this);
    g_signal_connect(_canvas, "button_release_event",
                   G_CALLBACK(buttonReleaseEvent), this);
    g_signal_connect(_canvas, "motion_notify_event",
                   G_CALLBACK(motionNotifyEvent), this);
    g_signal_connect(_canvas, "scroll_event",
                   G_CALLBACK(mouseWheelEvent), this);
    g_signal_connect(_canvas, "visibility-notify-event",
                   G_CALLBACK(visibilityNotifyEvent), this);
  
    g_signal_connect_after(_canvas, "realize",
                         G_CALLBACK (realizeEvent), NULL);

    // connect_after because we are going to cause a rendering and the canvas
    // widget should have had a chance to update the size of the render area
    g_signal_connect_after(_canvas, "configure_event",
                   G_CALLBACK (configureEvent), this);

    return true;
}

void
GtkGui::grabFocus()
{
    gtk_widget_grab_focus(GTK_WIDGET(_canvas));
}

void
GtkGui::quitUI() 
{
    gtk_main_quit();
}

/*private*/
void
GtkGui::startAdvanceTimer()
{
    stopAdvanceTimer();
    
    _advanceSourceTimer = g_timeout_add_full(G_PRIORITY_LOW, _interval,
            (GSourceFunc)advance_movie, this, NULL);

    log_debug(_("Advance interval timer set to %d ms (~ %d FPS)"),
            _interval, 1000/_interval);
}

/*private*/
void
GtkGui::stopAdvanceTimer()
{
    if (_advanceSourceTimer)
    {
        g_source_remove(_advanceSourceTimer);
        _advanceSourceTimer = 0;
    }
}

void
GtkGui::setInterval(unsigned int interval)
{
    _interval = interval;

    if ( ! isStopped() ) startAdvanceTimer();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///                             Widget functions                            ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Setup the menu bar for the top of the window frame.
bool
GtkGui::createMenuBar()
{
    _menubar = gtk_menu_bar_new();
    gtk_widget_show(_menubar);
    gtk_box_pack_start(GTK_BOX(_vbox), _menubar, FALSE, FALSE, 0);

    createFileMenu(_menubar);
    createEditMenu(_menubar);
    createViewMenu(_menubar);
    createControlMenu(_menubar);
    createHelpMenu(_menubar);
    
    return true;   
}

bool
GtkGui::createMenu()
{
    // A menu that pops up (normally) on a right-mouse click.

    _popup_menu = GTK_MENU(gtk_menu_new());
    
#ifdef USE_MENUS
    // If menus are disabled, these are not added to the popup menu
    // either.
    createFileMenu(GTK_WIDGET(_popup_menu));
    createEditMenu(GTK_WIDGET(_popup_menu));
    createViewMenu(GTK_WIDGET(_popup_menu));
    createControlMenu(GTK_WIDGET(_popup_menu));
#endif
    createHelpMenu(GTK_WIDGET(_popup_menu));

    GtkWidget *separator1 = gtk_separator_menu_item_new();
    gtk_widget_show(separator1);
    gtk_container_add (GTK_CONTAINER(_popup_menu), separator1);

    /// The sound handler is initialized after the Gui is created, and
    /// may be disabled or enabled dynamically.
    GtkCheckMenuItem *menusound =
        GTK_CHECK_MENU_ITEM(gtk_check_menu_item_new_with_label(_("Sound")));
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(menusound), TRUE);
    gtk_menu_append(_popup_menu, GTK_WIDGET(menusound));
    gtk_widget_show(GTK_WIDGET(menusound));
    g_signal_connect(menusound, "activate", G_CALLBACK(menuSound), this);

    GtkWidget *separator2 = gtk_separator_menu_item_new();
    gtk_widget_show(separator2);
    gtk_container_add (GTK_CONTAINER(_popup_menu), separator2);

    GtkWidget *quit = gtk_image_menu_item_new_from_stock("gtk-quit", 0);
    gtk_widget_show(quit);
    gtk_container_add(GTK_CONTAINER(_popup_menu), quit);
    g_signal_connect(quit, "activate", G_CALLBACK(menuQuit), this);

    return true;
}

bool
GtkGui::createMenuAlt()
{
    //An alternative short version of the popup menu

    _popup_menu_alt = GTK_MENU(gtk_menu_new());

#ifdef USE_MENUS
    // If menus are disabled, these are not added to the popup menu
    // either.
    createEditMenu(GTK_WIDGET(_popup_menu_alt));
#endif
    createHelpMenu(GTK_WIDGET(_popup_menu_alt));

    GtkWidget *separator1 = gtk_separator_menu_item_new();
    gtk_widget_show(separator1);
    gtk_container_add (GTK_CONTAINER(_popup_menu_alt), separator1);

    GtkWidget *quit = gtk_image_menu_item_new_from_stock("gtk-quit", 0);
    gtk_widget_show(quit);
    gtk_container_add(GTK_CONTAINER(_popup_menu_alt), quit);
    g_signal_connect(quit, "activate", G_CALLBACK(menuQuit), this);

    return true;
}

void
GtkGui::resizeWindow(int width, int height)
{
    log_debug(_("GtkGui: Window resize request received"));

    if (!_xid) {
    
        // This sets the *minimum* size for the drawing area and thus will
        // also resize the window if needed. 
        // Advantage: The window is sized correctly, no matter what other
        // widgets are visible
        // Disadvantage: The window will never be shrinked, which is bad.   
        gtk_widget_set_size_request(_canvas, width, height);
    }
}

bool
GtkGui::createWindow(const char *title, int width, int height,
                     int xPosition, int yPosition)
{
// First call the old createWindow function and then set the title.
// In case there's some need to not setting the title.
    bool ret = createWindow(width, height);
    gtk_window_set_title(GTK_WINDOW(_window), title);
    
    // Move the window to correct position if requested by user.
    int x, y;
    gtk_window_get_position(GTK_WINDOW(_window), &x, &y);
    if (xPosition > -1) x = xPosition; // -1 so we can tell if user requested window move
    if (yPosition > -1) y = yPosition; // as 0 is also a valid coordinate.
    gtk_window_move(GTK_WINDOW(_window), x, y);

    if (!_xid) {
    
        // This sets the *minimum* size for the drawing area and thus will
        // also resize the window. 
        // Advantage: The window is sized correctly, no matter what other
        // widgets are visible
        // Disadvantage: The window cannot be shrinked, which is bad.   
        gtk_widget_set_size_request(_canvas, width, height);

    }
    return ret;
}

#ifdef USE_SWFTREE

// This creates a GtkTree model for displaying movie info.
GtkTreeModel*
GtkGui::makeTreeModel(std::auto_ptr<movie_root::InfoTree> treepointer)
{

    const movie_root::InfoTree& info = *treepointer;

    enum
    {
        STRING1_COLUMN,
        STRING2_COLUMN,
        NUM_COLUMNS
    };
    
    GtkTreeStore *model = gtk_tree_store_new (NUM_COLUMNS,
                         G_TYPE_STRING, G_TYPE_STRING);
    
    GtkTreeIter iter;
    GtkTreeIter child_iter;
    GtkTreeIter parent_iter;

    // Depth within the *GTK* tree.
    int depth = 0;    

    assert(info.depth(info.begin()) == 0); // seems assumed in the code below
    for (movie_root::InfoTree::iterator i = info.begin(), e = info.end();
            i != e; ++i) {

        const movie_root::InfoTree::value_type& p = *i;

        std::ostringstream os;
        os << info.depth(i);  

        int newdepth = info.depth(i);

        if (newdepth > depth) {
            assert(newdepth == depth+1);
            depth++;                   
            iter=child_iter;                  
        }

        if (newdepth < depth) {
            int gap = depth - newdepth;
            depth = newdepth;
            while (gap--) {
                gtk_tree_model_iter_parent(GTK_TREE_MODEL(model),
                        &parent_iter, &iter);  
                iter = parent_iter;
            }
        }

        //Read in data from present node
        if (depth == 0) gtk_tree_store_append(model, &child_iter, NULL);
        else gtk_tree_store_append(model, &child_iter, &iter);

        gtk_tree_store_set(model, &child_iter,
                          STRING1_COLUMN, p.first.c_str(),   // "Variable"
                          STRING2_COLUMN, p.second.c_str(),  // "Value"
                          -1);

    }

    return GTK_TREE_MODEL(model);

}

#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///                             Rendering stuff                             ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool
GtkGui::createWindow(int width, int height)
{
    assert(_width>0);
    assert(_height>0);
    
    _width = width;
    _height = height;
    
    _validbounds.setTo(0, 0, _width, _height);
    
    return true;
}

void
GtkGui::beforeRendering()
{
    gnash_canvas_before_rendering(GNASH_CANVAS(_canvas), getStage());
}

void
GtkGui::renderBuffer()
{
    gdk_window_process_updates(_canvas->window, false);
}

void
GtkGui::setInvalidatedRegions(const InvalidatedRanges& ranges)
{
    // forward to renderer
    //
    // Why? Why have the region been invalidated ??
    //   A: I don't understand this question.
    // Was the renderer offscreen buffer also invalidated
    // (need to rerender)?
    //   A: Yes.
    // Was only the 'onscreen' buffer be invalidated (no need to rerender,
    // just to blit) ??
    //   A: I don't understand this question.
    //
    // Clarification: the render (optionally) only draws to the invalidated
    // (i.e., changed) part of the buffer. So we need to tell the renderer
    // where that is. The renderer draws to the offscreen buffer. (Although
    // that should be obvious!)
    _renderer->set_invalidated_regions(ranges);
    
    for (unsigned rno=0; rno<ranges.size(); rno++) {
        geometry::Range2d<int> bounds = Intersection(
            _renderer->world_to_pixel(ranges.getRange(rno)),
            _validbounds);
        
        // it may happen that a particular range is out of the screen, which 
        // will lead to bounds==null. 
        if (bounds.isNull()) continue;
        
        assert(bounds.isFinite()); 
        
        GdkRectangle rect;
        rect.x = bounds.getMinX();
        rect.y = bounds.getMinY();
        rect.width = bounds.width();
        rect.height = bounds.height();

        // We add the rectangle to the part of the window to be redrawn
        // (also known as the "clipping" or "damaged" area). in renderBuffer(),
        // we force a redraw.
        gdk_window_invalidate_rect(_canvas->window, &rect, false);
    }

}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///                             Dialogues                                   ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace { 

class PreferencesDialog
{

public:

    PreferencesDialog(GtkWidget* window);

    void show();

private:

    // A struct containing pointers to widgets for passing preference 
    // data from the dialogue   
    struct PrefWidgets {
        GtkWidget *soundToggle;
        GtkWidget *actionDumpToggle;
        GtkWidget *parserDumpToggle;
        GtkWidget *malformedSWFToggle;
        GtkWidget *ASCodingErrorToggle;
        GtkWidget *logfileName;
        GtkWidget *writeLogToggle;
        GtkWidget *verbosityScale;
        GtkWidget *streamsTimeoutScale;
        GtkWidget *localDomainToggle;
        GtkWidget *localHostToggle;
        GtkWidget *solReadOnlyToggle;
        GtkWidget *solLocalDomainToggle;
        GtkWidget *localConnectionToggle;
        GtkWidget *insecureSSLToggle; 
        GtkWidget *solSandbox;
        GtkWidget *osText;
        GtkWidget *versionText;
        GtkWidget *urlOpenerText;
        GtkWidget *librarySize;
        GtkWidget *startStoppedToggle;
        GtkWidget *mediaDir;
        GtkWidget *saveStreamingMediaToggle;
        GtkWidget *saveLoadedMediaToggle;

        PrefWidgets()
            :
            soundToggle(0),
            actionDumpToggle(0),
            parserDumpToggle(0),
            malformedSWFToggle(0),
            ASCodingErrorToggle(0),
            logfileName(0),
            writeLogToggle(0),
            verbosityScale(0),
            streamsTimeoutScale(0),
            localDomainToggle(0),
            localHostToggle(0),
            solReadOnlyToggle(0),
            solLocalDomainToggle(0),
            localConnectionToggle(0),
            insecureSSLToggle(0), 
            solSandbox(0),
            osText(0),
            versionText(0),
            urlOpenerText(0),
            librarySize(0),
            startStoppedToggle(0),
            mediaDir(0),
            saveStreamingMediaToggle(0),
            saveLoadedMediaToggle(0)
        {}

    };

    static void handlePrefs(GtkWidget* widget, gint response, gpointer data);

    /// Network Tab
    void addNetworkTab();

    /// Logging Tab
    void addLoggingTab();

    void addSecurityTab();

    void addMediaTab();

    void addPlayerTab();

    GtkWidget* _window;

    PrefWidgets* _prefs;

    RcInitFile& _rcfile;

    GtkWidget* _prefsDialog;

    GtkWidget* _notebook;

};


// Callback to read values from the preferences dialogue and set rcfile
// values accordingly.
void
PreferencesDialog::handlePrefs(GtkWidget* dialog, gint response, gpointer data)
{

    PrefWidgets *prefs = static_cast<PrefWidgets*>(data);

    RcInitFile& _rcfile = RcInitFile::getDefaultInstance();

    if (response == GTK_RESPONSE_OK) {

        // For getting from const gchar* to std::string&
        std::string tmp;
    
        if (prefs->soundToggle) {
            _rcfile.useSound(gtk_toggle_button_get_active(
                        GTK_TOGGLE_BUTTON(prefs->soundToggle)));
        }

        if (prefs->saveLoadedMediaToggle) {
            _rcfile.saveLoadedMedia(
                gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
                        prefs->saveLoadedMediaToggle)));
        }

        if (prefs->saveStreamingMediaToggle) {
            _rcfile.saveStreamingMedia(
                gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
                        prefs->saveStreamingMediaToggle)));
        }

        if (prefs->mediaDir) {
            tmp = gtk_entry_get_text(GTK_ENTRY(prefs->mediaDir));
            _rcfile.setMediaDir(tmp);
        }

        if (prefs->actionDumpToggle) {
            _rcfile.useActionDump(
                gtk_toggle_button_get_active(
                    GTK_TOGGLE_BUTTON(prefs->actionDumpToggle)));
        }
        
        if (prefs->parserDumpToggle) {
            _rcfile.useParserDump(
                gtk_toggle_button_get_active(
                    GTK_TOGGLE_BUTTON(prefs->parserDumpToggle)));
        }

        if ( prefs->logfileName ) {
            tmp = gtk_entry_get_text(GTK_ENTRY(prefs->logfileName));
            _rcfile.setDebugLog(tmp);
        }
        
        if ( prefs->writeLogToggle ) {
            _rcfile.useWriteLog(
                gtk_toggle_button_get_active(
                    GTK_TOGGLE_BUTTON(prefs->writeLogToggle)));
        }
            
        if ( prefs->verbosityScale ) {
            _rcfile.verbosityLevel(static_cast<int>(
                gtk_range_get_value(GTK_RANGE(prefs->verbosityScale))));
        }

        if ( prefs->streamsTimeoutScale ) {
            _rcfile.setStreamsTimeout(
                gtk_spin_button_get_value_as_int(
                    GTK_SPIN_BUTTON(prefs->streamsTimeoutScale)));
        }

        if ( prefs->ASCodingErrorToggle ) {
            _rcfile.showASCodingErrors(
                gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
                        prefs->ASCodingErrorToggle)));
        }

        if ( prefs->malformedSWFToggle ) {
            _rcfile.showMalformedSWFErrors(
                gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
                        prefs->malformedSWFToggle)));
        }

        if ( prefs->localHostToggle ) {
            _rcfile.useLocalHost(
                gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
                        prefs->localHostToggle)));
        }

        if ( prefs->localDomainToggle ) {
            _rcfile.useLocalDomain(
                gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
                        prefs->localDomainToggle)));
        }

        if ( prefs->solLocalDomainToggle ) {
            _rcfile.setSOLLocalDomain(
                gtk_toggle_button_get_active(
                    GTK_TOGGLE_BUTTON(prefs->solLocalDomainToggle)));
        }

        if ( prefs->solReadOnlyToggle ) {
            _rcfile.setSOLReadOnly(
                gtk_toggle_button_get_active(
                    GTK_TOGGLE_BUTTON(prefs->solReadOnlyToggle)));
        }

        if ( prefs->localConnectionToggle ) {
            _rcfile.setLocalConnection(
                gtk_toggle_button_get_active(
                    GTK_TOGGLE_BUTTON(prefs->localConnectionToggle)));
        }

        if ( prefs->insecureSSLToggle ) {
            _rcfile.insecureSSL(
                gtk_toggle_button_get_active(
                    GTK_TOGGLE_BUTTON(prefs->insecureSSLToggle)));
        }

        if ( prefs->solSandbox ) {
            tmp = gtk_entry_get_text(GTK_ENTRY(prefs->solSandbox));
            _rcfile.setSOLSafeDir(tmp);
        }

        if ( prefs->osText ) {
            tmp = gtk_entry_get_text(GTK_ENTRY(prefs->osText));
            _rcfile.setFlashSystemOS(tmp);
        }
        
        if ( prefs->versionText ) {
            tmp = gtk_entry_get_text(GTK_ENTRY(prefs->versionText));
            _rcfile.setFlashVersionString(tmp);        
        }

        if ( prefs->librarySize ) {
            _rcfile.setMovieLibraryLimit(
                gtk_spin_button_get_value_as_int(
                    GTK_SPIN_BUTTON(prefs->librarySize)));
        }

        if ( prefs->startStoppedToggle ) {
            _rcfile.startStopped(
                gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(prefs->startStoppedToggle)));
        }

        if ( prefs->urlOpenerText ) {
            tmp = gtk_entry_get_text(GTK_ENTRY(prefs->urlOpenerText));            
            _rcfile.setURLOpenerFormat(tmp);
        }
        
        // Let _rcfile decide which file to update: generally the file
        // being used if specified in GNASHRC environment variable, or in
        // the user's home directory if that can be found.
        // TODO: We can also specify here which file should be written
        // by passing that instead. How might that best be done?
        _rcfile.updateFile();

        // Close the window when 'ok' is clicked
        gtk_widget_destroy(dialog);
    }

    else if (response == GTK_RESPONSE_CANCEL) {
        // Close the window when 'cancel' is clicked
        gtk_widget_destroy(dialog);
    }

    delete prefs;
}

void
PreferencesDialog::show()
{
    gtk_widget_show_all(_prefsDialog);
}

PreferencesDialog::PreferencesDialog(GtkWidget* window)
    :
    _window(window),
    _prefs(new PrefWidgets),
    _rcfile(RcInitFile::getDefaultInstance())
{
    // Create top-level window
    _prefsDialog = gtk_dialog_new_with_buttons(
                    _("Gnash preferences"),
                    GTK_WINDOW(_window),
                    // Needs an explicit cast in C++
                    GtkDialogFlags(
                    GTK_DIALOG_DESTROY_WITH_PARENT |
                    GTK_DIALOG_NO_SEPARATOR),
                    // The buttons and their response codes:
                    GTK_STOCK_OK, GTK_RESPONSE_OK,
                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                    NULL);
    // Add Gnash icon
    addGnashIcon(GTK_WINDOW(_prefsDialog));

    // Add notebook (tabs) to dialogue's vbox
    _notebook = gtk_notebook_new ();
    gtk_container_add (
            GTK_CONTAINER(GTK_DIALOG(_prefsDialog)->vbox), _notebook);

    // Pass the widgets containing settings to the callback function
    // when any button is clicked or when the dialogue is destroyed.
    g_signal_connect (_prefsDialog, "response", G_CALLBACK(&handlePrefs), _prefs);

    addLoggingTab();
    addSecurityTab();
    addNetworkTab();
    addMediaTab();
    addPlayerTab();
}

void
PreferencesDialog::addNetworkTab()
{
    GtkWidget *vbox = gtk_vbox_new (FALSE, 10);

    // Tab label
    GtkWidget *label = gtk_label_new_with_mnemonic (_("_Network"));
    gtk_notebook_append_page(GTK_NOTEBOOK(_notebook), GTK_WIDGET(vbox), label); 

    // Network preferences 
    label = gtk_label_new (_("<b>Network preferences</b>"));
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

    // Streams timeout
    GtkWidget *timeoutbox = gtk_hbox_new (FALSE, 2);
    gtk_box_pack_start(GTK_BOX(vbox), timeoutbox, FALSE, FALSE, 0);
    
    label = gtk_label_new (_("Network timeout in seconds (0 for no timeout):"));
    gtk_box_pack_start(GTK_BOX(timeoutbox), label, FALSE, FALSE, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    _prefs->streamsTimeoutScale = gtk_spin_button_new_with_range(0, 300, 1);
    gtk_box_pack_start(GTK_BOX(timeoutbox), _prefs->streamsTimeoutScale, FALSE,
            FALSE, 0);
    // Align to _rcfile value:
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(_prefs->streamsTimeoutScale),
            _rcfile.getStreamsTimeout());

}

void
PreferencesDialog::addLoggingTab()
{
    GtkWidget *loggingvbox = gtk_vbox_new (FALSE, 10);
   
    // Tab label
    GtkWidget *loggingtablabel = gtk_label_new_with_mnemonic (_("_Logging"));
    gtk_notebook_append_page(GTK_NOTEBOOK(_notebook), GTK_WIDGET(loggingvbox),
            loggingtablabel); 

    // Logging options
    GtkWidget *logginglabel = gtk_label_new (_("<b>Logging options</b>"));
    gtk_label_set_use_markup (GTK_LABEL (logginglabel), TRUE);
    gtk_box_pack_start(GTK_BOX(loggingvbox), logginglabel, FALSE, FALSE, 0);
    
    GtkWidget *verbositylabel = gtk_label_new (_("Verbosity level:"));
    gtk_box_pack_start(GTK_BOX(loggingvbox), verbositylabel, FALSE, FALSE, 0);
    gtk_misc_set_alignment (GTK_MISC (verbositylabel), 0, 0.5);

    _prefs->verbosityScale = gtk_hscale_new(GTK_ADJUSTMENT(
                gtk_adjustment_new(_rcfile.verbosityLevel(), 0, 10, 1, 0, 0)));
    gtk_scale_set_digits(GTK_SCALE(_prefs->verbosityScale), 0);
    gtk_range_set_update_policy(GTK_RANGE(
                _prefs->verbosityScale), GTK_UPDATE_DISCONTINUOUS);
    gtk_box_pack_start(GTK_BOX(loggingvbox), _prefs->verbosityScale, FALSE,
            FALSE, 0);
    
    _prefs->writeLogToggle = 
        gtk_check_button_new_with_mnemonic(_("Log to _file"));
    gtk_box_pack_start(GTK_BOX(loggingvbox), _prefs->writeLogToggle, FALSE,
            FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_prefs->writeLogToggle),
            _rcfile.useWriteLog());
    
    GtkWidget *logfilelabel = gtk_label_new(_("Logfile name:"));
    gtk_box_pack_start(GTK_BOX(loggingvbox), logfilelabel, FALSE, FALSE, 0);
    gtk_misc_set_alignment(GTK_MISC(logfilelabel), 0, 0.5);
    
    _prefs->logfileName = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(loggingvbox), _prefs->logfileName, FALSE,
            FALSE, 0);

    // Put debug filename in the entry box      
    gtk_entry_set_text(GTK_ENTRY(_prefs->logfileName),
            _rcfile.getDebugLog().c_str());
    
    _prefs->parserDumpToggle = 
        gtk_check_button_new_with_mnemonic(_("Log _parser output"));
    gtk_box_pack_start(GTK_BOX(loggingvbox), _prefs->parserDumpToggle, FALSE,
            FALSE, 0);
    // Align button state with _rcfile
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(_prefs->parserDumpToggle),
            _rcfile.useParserDump());

    _prefs->actionDumpToggle =
        gtk_check_button_new_with_mnemonic(_("Log SWF _actions"));
    gtk_box_pack_start(GTK_BOX(loggingvbox), _prefs->actionDumpToggle, FALSE,
            FALSE, 0);
    // Align button state with _rcfile
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_prefs->actionDumpToggle),
            _rcfile.useActionDump());

    _prefs->malformedSWFToggle = 
        gtk_check_button_new_with_mnemonic(_("Log malformed SWF _errors"));
    gtk_box_pack_start(GTK_BOX(loggingvbox), _prefs->malformedSWFToggle,
            FALSE, FALSE, 0);
    // Align button state with _rcfile
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_prefs->malformedSWFToggle),
            _rcfile.showMalformedSWFErrors());

    _prefs->ASCodingErrorToggle = gtk_check_button_new_with_mnemonic(
            _("Log ActionScript _coding errors"));
    gtk_box_pack_start(GTK_BOX(loggingvbox), _prefs->ASCodingErrorToggle,
            FALSE, FALSE, 0);
    // Align button state with _rcfile
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_prefs->ASCodingErrorToggle),
                _rcfile.showASCodingErrors());

}

void
PreferencesDialog::addSecurityTab()
{
    // Security Tab
    GtkWidget *securityvbox = gtk_vbox_new (FALSE, 14);

    // Security tab title
    GtkWidget *securitytablabel = gtk_label_new_with_mnemonic (_("_Security"));
    
    gtk_notebook_append_page(GTK_NOTEBOOK(_notebook),
            GTK_WIDGET(securityvbox), securitytablabel); 
    
    // Network connection
    GtkWidget *netconnectionslabel = gtk_label_new(
            _("<b>Network connections</b>"));
    gtk_label_set_use_markup(GTK_LABEL(netconnectionslabel), TRUE);
    gtk_box_pack_start(GTK_BOX(securityvbox), netconnectionslabel, FALSE,
            FALSE, 0);
 
    _prefs->localHostToggle = gtk_check_button_new_with_mnemonic(
            _("Connect only to local _host"));
    gtk_box_pack_start(GTK_BOX(securityvbox), _prefs->localHostToggle, FALSE,
            FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_prefs->localHostToggle),
            _rcfile.useLocalHost());
    
    _prefs->localDomainToggle = gtk_check_button_new_with_mnemonic(
            _("Connect only to local _domain"));
    gtk_box_pack_start(GTK_BOX(securityvbox), _prefs->localDomainToggle,
            FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_prefs->localDomainToggle),
            _rcfile.useLocalDomain());

    _prefs->insecureSSLToggle = gtk_check_button_new_with_mnemonic(
            _("Disable SSL _verification"));
    gtk_box_pack_start(GTK_BOX(securityvbox), _prefs->insecureSSLToggle,
            FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_prefs->insecureSSLToggle),
            _rcfile.insecureSSL());
    
    GtkWidget *whitelistexpander =
        gtk_expander_new_with_mnemonic(_("_Whitelist"));
    gtk_box_pack_start(GTK_BOX(securityvbox), whitelistexpander, FALSE,
            FALSE, 0);
    
    GtkWidget *whitelistcomboboxentry1 = gtk_combo_box_entry_new_text();
    gtk_container_add(GTK_CONTAINER(whitelistexpander),
            whitelistcomboboxentry1);

    GtkWidget *blacklistexpander = 
        gtk_expander_new_with_mnemonic(_("_Blacklist"));
    gtk_box_pack_start(GTK_BOX (securityvbox), blacklistexpander, FALSE,
            FALSE, 0);
    
    GtkWidget *blacklistcomboboxentry2 = gtk_combo_box_entry_new_text();
    gtk_container_add (GTK_CONTAINER(blacklistexpander),
            blacklistcomboboxentry2);

    // Privacy
    GtkWidget *privacylabel = gtk_label_new(_("<b>Privacy</b>"));
    gtk_label_set_use_markup(GTK_LABEL(privacylabel), TRUE);
    gtk_box_pack_start(GTK_BOX(securityvbox), privacylabel, FALSE, FALSE, 0);

    GtkWidget *solsandboxlabel = gtk_label_new(_("Shared objects directory:"));
    gtk_box_pack_start(GTK_BOX(securityvbox), solsandboxlabel, FALSE,
            FALSE, 0);
    gtk_misc_set_alignment(GTK_MISC (solsandboxlabel), 0, 0.5);

    _prefs->solSandbox = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(_prefs->solSandbox), 
            _rcfile.getSOLSafeDir().c_str());
    gtk_box_pack_start(GTK_BOX(securityvbox), _prefs->solSandbox, FALSE,
            FALSE, 0);

    _prefs->solReadOnlyToggle = gtk_check_button_new_with_mnemonic( 
                    _("Do _not write Shared Object files"));
    gtk_box_pack_start(GTK_BOX(securityvbox), _prefs->solReadOnlyToggle,
            FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_prefs->solReadOnlyToggle),
                _rcfile.getSOLReadOnly());

    _prefs->solLocalDomainToggle = gtk_check_button_new_with_mnemonic(
                    _("Only _access local Shared Object files"));
    gtk_box_pack_start(GTK_BOX(securityvbox), _prefs->solLocalDomainToggle,
            FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
                _prefs->solLocalDomainToggle), _rcfile.getSOLLocalDomain());

    _prefs->localConnectionToggle = gtk_check_button_new_with_mnemonic(
                    _("Disable Local _Connection object"));
    gtk_box_pack_start(GTK_BOX(securityvbox), _prefs->localConnectionToggle,
            FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
                _prefs->localConnectionToggle), _rcfile.getLocalConnection()); 
}

void
PreferencesDialog::addMediaTab()
{
    // Media Tab
    GtkWidget *mediavbox = gtk_vbox_new (FALSE, 2);

    // Media tab title
    GtkWidget *mediatablabel = gtk_label_new_with_mnemonic (_("_Media"));
    
    gtk_notebook_append_page(GTK_NOTEBOOK(_notebook),
            GTK_WIDGET(mediavbox), mediatablabel); 
    
    // Sound
    GtkWidget *soundlabel = gtk_label_new(_("<b>Sound</b>"));
    gtk_label_set_use_markup(GTK_LABEL(soundlabel), TRUE);
    gtk_box_pack_start(GTK_BOX(mediavbox), soundlabel, FALSE, FALSE, 0);
   
    _prefs->soundToggle = gtk_check_button_new_with_mnemonic(
            _("Use sound _handler"));
    gtk_box_pack_start(GTK_BOX(mediavbox), _prefs->soundToggle, FALSE,
            FALSE, 0);
    // Align button state with rcfile
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_prefs->soundToggle),
            _rcfile.useSound());

    // Save Media
    GtkWidget *savemedia = gtk_label_new(_("<b>Media Streams</b>"));
    gtk_label_set_use_markup(GTK_LABEL(savemedia), TRUE);
    gtk_box_pack_start(GTK_BOX(mediavbox), savemedia, FALSE, FALSE, 0);
   
    // Save streamed media Toggle
    _prefs->saveStreamingMediaToggle = gtk_check_button_new_with_mnemonic(
            _("Save media streams to disk"));
    gtk_box_pack_start (GTK_BOX(mediavbox), _prefs->saveStreamingMediaToggle,
            FALSE, FALSE, 0);
    // Align button state with rcfile
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
            _prefs->saveStreamingMediaToggle), _rcfile.saveStreamingMedia());

    // Save loaded media Toggle
    _prefs->saveLoadedMediaToggle = gtk_check_button_new_with_mnemonic(
            _("Save dynamically loaded media to disk"));
    gtk_box_pack_start (GTK_BOX(mediavbox), _prefs->saveLoadedMediaToggle,
            FALSE, FALSE, 0);
    // Align button state with rcfile
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
                _prefs->saveLoadedMediaToggle), _rcfile.saveLoadedMedia());

    // Directory for saving media
    GtkWidget *mediastreamslabel = gtk_label_new(_("Saved media directory:"));
    gtk_box_pack_start(GTK_BOX(mediavbox), mediastreamslabel, FALSE,
            FALSE, 0);
    gtk_misc_set_alignment (GTK_MISC (mediastreamslabel), 0, 0.5);

    _prefs->mediaDir = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(_prefs->mediaDir), 
            _rcfile.getMediaDir().c_str());
    gtk_box_pack_start(GTK_BOX(mediavbox), _prefs->mediaDir, FALSE,
            FALSE, 0);

}

void
PreferencesDialog::addPlayerTab()
{
    // Player Tab
    GtkWidget *playervbox = gtk_vbox_new (FALSE, 14);

    // Player tab title
    GtkWidget *playertablabel = gtk_label_new_with_mnemonic (_("_Player"));
    
    gtk_notebook_append_page(GTK_NOTEBOOK(_notebook),
            GTK_WIDGET(playervbox), playertablabel); 

    // Player description
    GtkWidget *descriptionlabel = gtk_label_new (_("<b>Player description</b>"));
    gtk_label_set_use_markup (GTK_LABEL (descriptionlabel), TRUE);
    gtk_box_pack_start(GTK_BOX(playervbox), descriptionlabel, FALSE, FALSE, 0);

    // Version string
    GtkWidget *versionhbox = gtk_hbox_new (FALSE, 2);
    gtk_box_pack_start(GTK_BOX(playervbox), versionhbox, FALSE, FALSE, 0);

    GtkWidget *versionlabel = gtk_label_new (_("Player version:"));
    gtk_misc_set_alignment (GTK_MISC (versionlabel), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(versionhbox), versionlabel, FALSE, FALSE, 0);

    _prefs->versionText = gtk_entry_new ();
    gtk_box_pack_start(GTK_BOX(versionhbox), _prefs->versionText,
            FALSE, FALSE, 0);

    // Put text in the entry box      
    gtk_entry_set_text(GTK_ENTRY(_prefs->versionText),
            _rcfile.getFlashVersionString().c_str());

    // OS label
    GtkWidget *oshbox = gtk_hbox_new (FALSE, 2);
    gtk_box_pack_start(GTK_BOX(playervbox), oshbox, FALSE, FALSE, 0);
    
    GtkWidget *OSlabel = gtk_label_new (_("Operating system:"));
    gtk_misc_set_alignment (GTK_MISC (OSlabel), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(oshbox), OSlabel, FALSE, FALSE, 0);
    
    _prefs->osText = gtk_entry_new ();
    gtk_box_pack_start(GTK_BOX(oshbox), _prefs->osText, FALSE, FALSE, 0);
    // Put text in the entry box      
    gtk_entry_set_text(GTK_ENTRY(_prefs->osText),
            _rcfile.getFlashSystemOS().c_str());
    
    GtkWidget *OSadvicelabel = gtk_label_new (_("<i>If blank, Gnash will "
                           "detect your OS</i>"));
    gtk_label_set_use_markup (GTK_LABEL (OSadvicelabel), TRUE);
    gtk_misc_set_alignment (GTK_MISC (OSadvicelabel), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(playervbox), OSadvicelabel, FALSE, FALSE, 0);

    // URL opener
    GtkWidget *urlopenerbox = gtk_hbox_new(FALSE, 2);
    gtk_box_pack_start(GTK_BOX(playervbox), urlopenerbox, FALSE, FALSE, 0);
    
    GtkWidget *urlopenerlabel = gtk_label_new (_("URL opener:"));
    gtk_misc_set_alignment (GTK_MISC (urlopenerlabel), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(urlopenerbox), urlopenerlabel, FALSE, FALSE, 0);
    
    _prefs->urlOpenerText = gtk_entry_new ();
    gtk_box_pack_start(GTK_BOX(urlopenerbox), _prefs->urlOpenerText, FALSE,
            FALSE, 0);
    // Put text in the entry box      
    gtk_entry_set_text(GTK_ENTRY(_prefs->urlOpenerText),
            _rcfile.getURLOpenerFormat().c_str());

    // Performance
    GtkWidget *performancelabel = gtk_label_new(_("<b>Performance</b>"));
    gtk_label_set_use_markup (GTK_LABEL (performancelabel), TRUE);
    gtk_box_pack_start(GTK_BOX(playervbox), performancelabel, FALSE, FALSE, 0);

    GtkWidget* qualitybox = gtk_hbox_new(FALSE, 2);
    gtk_box_pack_start(GTK_BOX(playervbox), qualitybox, FALSE, FALSE, 0);

    GtkWidget* qualityoptions = gtk_vbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(qualitybox), qualityoptions, FALSE, FALSE, 0);

    // Library size
    GtkWidget *libraryhbox = gtk_hbox_new (FALSE, 2);
    gtk_box_pack_start(GTK_BOX(playervbox), libraryhbox, FALSE, FALSE, 0);

    GtkWidget *librarylabel = gtk_label_new (_("Max size of movie library:"));
    gtk_misc_set_alignment (GTK_MISC (librarylabel), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(libraryhbox), librarylabel, FALSE, FALSE, 0);

    _prefs->librarySize = gtk_spin_button_new_with_range(0, 100, 1);
    gtk_box_pack_start(GTK_BOX(libraryhbox), _prefs->librarySize, FALSE,
            FALSE, 0);
    // Align to _rcfile value:
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(_prefs->librarySize),
            _rcfile.getMovieLibraryLimit());

    _prefs->startStoppedToggle = gtk_check_button_new_with_mnemonic (
                    _("Start _Gnash in pause mode"));
    gtk_box_pack_start(GTK_BOX(playervbox), _prefs->startStoppedToggle,
            FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_prefs->startStoppedToggle),
                _rcfile.startStopped());
} 


} // anonymous namespace


void
GtkGui::showPreferencesDialog()
{
    
    PreferencesDialog preferencesDialog(_window);
    preferencesDialog.show();
}

void
GtkGui::showPropertiesDialog()
{

    GtkWidget *propsDialog = gtk_dialog_new_with_buttons(
                        _("Movie properties"),
                        GTK_WINDOW(_window),
                        // The cast is necessary if there is more
                        // than one option.
                        GtkDialogFlags(
                        GTK_DIALOG_DESTROY_WITH_PARENT),
                        // Just a 'close' button
                        GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                        NULL);

    // Not too small... But I'd rather not have to specify
    // a size in pixels.
    gtk_window_set_default_size(GTK_WINDOW(propsDialog), 500, 300);
    
    // Suggest to the window manager to allow "maximize"
    // As there can be (will be) a lot of data.
    gtk_window_set_type_hint(GTK_WINDOW(propsDialog),
                            GDK_WINDOW_TYPE_HINT_NORMAL);

    addGnashIcon(GTK_WINDOW(propsDialog));

    // Destroy the window when a button is clicked.
    g_signal_connect (propsDialog, "response",
               G_CALLBACK(gtk_widget_destroy), NULL);

    GtkWidget *propsvbox = gtk_vbox_new (FALSE, 1);
    gtk_container_add(GTK_CONTAINER(
                        GTK_DIALOG(propsDialog)->vbox), propsvbox);

#ifdef USE_SWFTREE

    std::auto_ptr<movie_root::InfoTree> infoptr = getMovieInfo();

    GtkWidget *scrollwindow1 = gtk_scrolled_window_new(0, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwindow1),
                      GTK_POLICY_AUTOMATIC,
                      GTK_POLICY_AUTOMATIC);

    gtk_box_pack_start(GTK_BOX (propsvbox), scrollwindow1, TRUE, TRUE, 0);

    enum
    {
        STRING1_COLUMN,
        STRING2_COLUMN
    };

    GtkTreeModel *model = makeTreeModel(infoptr);

    GtkWidget *treeview = gtk_tree_view_new_with_model (model);

    g_object_unref (model);

    ///
    /// Tree view behaviour.
    
    /// Search on "variable" column
    gtk_tree_view_set_enable_search(GTK_TREE_VIEW(treeview), TRUE);
    gtk_tree_view_set_search_column (GTK_TREE_VIEW(treeview), 0);
    
    /// Nice shading
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(treeview), TRUE);
    
    gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(treeview), TRUE);

    // Add columns:
    
    // 'Variable' column:
    GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(
            GTK_TREE_VIEW(treeview),
            -1, _("Variable"),
            renderer, "text",
            STRING1_COLUMN,
            NULL);

    // 'Value' column:
    // Set to be 'editable' so that the data can be selected and
    // copied; it can't actually be edited, though.
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer, "xalign", 0.0, "editable", TRUE, NULL);
    gtk_tree_view_insert_column_with_attributes(
            GTK_TREE_VIEW(treeview),
            -1, _("Value"),
            renderer, "text",
            STRING2_COLUMN,
            NULL);

    //Add tree to scrollwindow.
    gtk_container_add(GTK_CONTAINER(scrollwindow1), treeview);

#endif

    gtk_widget_show_all(propsDialog);

}

// \brief Show info about gnash
void
GtkGui::showAboutDialog()
{
    const gchar *documentors[] = { 
        "Rob Savoye", 
        "Sandro Santilli",
        "Ann Barcomb",
        NULL 
    };

    const gchar *artists[] = { 
        "Jason Savoye",
        NULL
    };

    const gchar *authors[] = { 
        "Rob Savoye", 
        "Sandro Santilli", 
        "Bastiaan Jacques", 
        "Tomas Groth", 
        "Udo Giacomozzi", 
        "Hannes Mayr", 
        "Markus Gothe", 
        "Vitaly Alexeev",
        "John Gilmore",
        "Zou Lunkai",
        "Benjamin Wolsey",
        "Russ Nelson",
        "Dossy Shiobara",
        "Jonathan Crider",
        "Ben Limmer",
        "Bob Naugle",
        "Si Liu",
        "Sharad Desai",
        NULL
    };

	const std::string license = 
        _("This program is free software; you can redistribute it and/or modify\n"
         "it under the terms of the GNU General Public License as published by\n"
         "the Free Software Foundation; either version 3 of the License, or\n"
         "(at your option) any later version.\n\n"
         "This program is distributed in the hope that it will be useful,\n"
         "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
         "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
         "GNU General Public License for more details.\n"
         "You should have received a copy of the GNU General Public License\n"
         "along with this program; if not, write to the Free Software\n"
         "Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301\n"
         "USA or visit http://www.gnu.org/licenses/.");

    media::MediaHandler* m = _runResources.mediaHandler();

    std::string comments =
        _("Gnash is the GNU SWF Player based on GameSWF.");
    comments.append(_("\nRenderer: "));
    comments.append(_renderer->description());
    comments.append(_("\nHardware Acceleration: "));
    comments.append(HWACCEL_CONFIG);
    comments.append(_("\nGUI: "));
    comments.append("GTK2"); // gtk of course!
    comments.append(_("\nMedia: "));
    comments.append(m ? m->description() : "no media handler");

    GdkPixbuf* logo_pixbuf = createPixbuf("GnashG.png");

    // gtk-2.8.20 (Debian 4.0) doesn't work fine with 
    // the gtk_show_about_dialog() call [ omits info ].
    // See bug #24426.
    GtkWidget* aboutWidget = gtk_about_dialog_new();
    addGnashIcon(GTK_WINDOW(aboutWidget)); 
    GtkAboutDialog* about = GTK_ABOUT_DIALOG(aboutWidget);

    gtk_about_dialog_set_name (about, "Gnash");
    std::string version = VERSION;
    version.append("\n");
    version.append("(");
    version.append(BRANCH_NICK);
    version.append("-");
    version.append(BRANCH_REVNO);
    version.append("-");
    version.append(COMMIT_ID);
    version.append(")");

    gtk_about_dialog_set_version(about, version.c_str());
    gtk_about_dialog_set_copyright(about, "Copyright (C) 2005, 2006, 2007, "
            "2008, 2009, 2010, 2011 The Free Software Foundation");
    gtk_about_dialog_set_comments (about, comments.c_str());
    gtk_about_dialog_set_authors(about, authors);
    gtk_about_dialog_set_documenters(about, documentors);
    gtk_about_dialog_set_artists(about, artists);
    gtk_about_dialog_set_translator_credits(about, _("translator-credits"));
    gtk_about_dialog_set_logo(about, logo_pixbuf);
    gtk_about_dialog_set_license(about, license.c_str());
    gtk_about_dialog_set_website(about, "http://www.gnu.org/software/gnash/");

    // Destroy the dialogue box when 'close' is clicked.
    g_signal_connect(aboutWidget, "response", 
            G_CALLBACK(gtk_widget_destroy), aboutWidget);

    gtk_widget_show (aboutWidget);

    if (logo_pixbuf) gdk_pixbuf_unref(logo_pixbuf);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///                                Menus                                    ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


// Create a File menu that can be used from the menu bar or the popup.
void
GtkGui::createFileMenu(GtkWidget *obj)
{
    GtkWidget *menuitem = gtk_menu_item_new_with_mnemonic(_("_File"));
    gtk_widget_show(menuitem);
    gtk_container_add(GTK_CONTAINER(obj), menuitem);
    
    GtkWidget *menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), menu);

    // Open    
    GtkWidget *open = gtk_image_menu_item_new_from_stock("gtk-open", 0);
    gtk_widget_show(open);
    gtk_container_add(GTK_CONTAINER(menu), open);
    g_signal_connect(open, "activate", G_CALLBACK(menuOpenFile), this);

    // Save    
    GtkWidget *save = gtk_image_menu_item_new_from_stock("gtk-save", 0);
    gtk_widget_show(save);
    gtk_container_add(GTK_CONTAINER(menu), save);
    // Disabled until save functionality is implemented:
    gtk_widget_set_sensitive(save, FALSE); 

    // Save as
    GtkWidget *saveAs = gtk_image_menu_item_new_from_stock("gtk-save-as", 0);
    gtk_widget_show(saveAs);
    gtk_container_add(GTK_CONTAINER(menu), saveAs);
    // Disabled until save-as functionality is implemented:
    gtk_widget_set_sensitive(saveAs, FALSE);
    
    GtkWidget *separatormenuitem1 = gtk_separator_menu_item_new();
    gtk_widget_show(separatormenuitem1);
    gtk_container_add(GTK_CONTAINER(menu), separatormenuitem1);

    // Properties
    GtkWidget *properties =
        gtk_image_menu_item_new_from_stock("gtk-properties", 0);
    gtk_widget_show(properties);
    gtk_container_add(GTK_CONTAINER(menu), properties);
    g_signal_connect(properties, "activate", G_CALLBACK(menuMovieInfo), this);

    GtkWidget *separator2 = gtk_separator_menu_item_new();
    gtk_widget_show(separator2);
    gtk_container_add(GTK_CONTAINER(menu), separator2);

    GtkWidget *quit = gtk_image_menu_item_new_from_stock("gtk-quit", 0);
    gtk_widget_show(quit);
    gtk_container_add(GTK_CONTAINER(menu), quit);
    g_signal_connect(quit, "activate", G_CALLBACK(menuQuit), this);
}

// Create an Edit menu that can be used from the menu bar or the popup.
void
GtkGui::createEditMenu(GtkWidget *obj)
{
    
    GtkWidget *menuitem = gtk_menu_item_new_with_mnemonic(_("_Edit"));
    gtk_widget_show(menuitem);
    gtk_container_add(GTK_CONTAINER(obj), menuitem);
    
    GtkWidget *menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), menu);

    GtkWidget *preferences =
        gtk_image_menu_item_new_from_stock("gtk-preferences", 0);
    gtk_widget_show(preferences);
    gtk_container_add(GTK_CONTAINER(menu), preferences);

    g_signal_connect(preferences, "activate", G_CALLBACK(menuPreferences),
                      this);
}

// Create a Help menu that can be used from the menu bar or the popup.
void
GtkGui::createHelpMenu(GtkWidget *obj)
{
    GtkWidget *menuitem = gtk_menu_item_new_with_mnemonic(_("_Help"));
    gtk_widget_show(menuitem);
    gtk_container_add(GTK_CONTAINER (obj), menuitem);
    
    GtkWidget *menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), menu);

    GtkWidget *about = gtk_image_menu_item_new_from_stock("gtk-about", 0);
    gtk_widget_show(about);
    gtk_container_add(GTK_CONTAINER(menu), about);
    
    g_signal_connect(about, "activate", G_CALLBACK(menuAbout), this);
}


// Create a View menu that can be used from the menu bar or the popup.
void
GtkGui::createViewMenu(GtkWidget *obj)
{

    GtkWidget *menuitem = gtk_menu_item_new_with_mnemonic (_("_View"));
    gtk_widget_show (menuitem);
    gtk_container_add (GTK_CONTAINER (obj), menuitem);
    
    GtkWidget *menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menu);

    // Refresh
    GtkWidget *refresh = gtk_image_menu_item_new_with_label(_("Redraw"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(refresh),
        gtk_image_new_from_stock("gtk-refresh", GTK_ICON_SIZE_MENU));
    gtk_menu_append(menu, refresh);
    gtk_widget_show(refresh);
    g_signal_connect(refresh, "activate", G_CALLBACK(menuRefreshView), this);

    // Fullscreen
#if GTK_CHECK_VERSION(2,8,0)
    GtkWidget *fullscreen = 
        gtk_image_menu_item_new_with_label(_("Toggle fullscreen"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(fullscreen),
               gtk_image_new_from_stock("gtk-fullscreen", GTK_ICON_SIZE_MENU));
#else
    GtkWidget *fullscreen = 
        gtk_menu_item_new_with_label(_("Toggle fullscreen"));
#endif
    gtk_menu_append(menu, fullscreen);
    gtk_widget_show(GTK_WIDGET(fullscreen));
    g_signal_connect(fullscreen, "activate", G_CALLBACK(menuFullscreen), this);

// Can be disabled at compile time.
#ifndef DISABLE_REGION_UPDATES_DEBUGGING
    GtkWidget *updated_regions =
        gtk_check_menu_item_new_with_label(_("Show updated ranges"));
   
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(updated_regions),
            showUpdatedRegions());

    gtk_menu_append(menu, updated_regions);
    gtk_widget_show(updated_regions);
    g_signal_connect(updated_regions, "activate",
                     G_CALLBACK(menuShowUpdatedRegions), this);
#endif

    createQualityMenu(menu);

}

// Create a Quality menu that can be used from the View menu
void
GtkGui::createQualityMenu(GtkWidget *obj)
{
    GtkWidget *menuitem = gtk_menu_item_new_with_mnemonic (_("_Quality"));
    gtk_widget_show (menuitem);
    gtk_container_add (GTK_CONTAINER (obj), menuitem);
    
    GtkWidget *menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menu);

    // TODO: use to also show current quality state

    // Low
    GtkWidget* item = gtk_menu_item_new_with_label(_("Low"));
    gtk_menu_append(menu, item);
    gtk_widget_show(item);
    g_signal_connect(item, "activate", G_CALLBACK(menuQualityLow), this);

    // Medium
    item = gtk_menu_item_new_with_label(_("Medium"));
    gtk_menu_append(menu, item);
    gtk_widget_show(item);
    g_signal_connect(item, "activate", G_CALLBACK(menuQualityMedium), this);

    // High
    item = gtk_menu_item_new_with_label(_("High"));
    gtk_menu_append(menu, item);
    gtk_widget_show(item);
    g_signal_connect(item, "activate", G_CALLBACK(menuQualityHigh), this);

    // Best
    item = gtk_menu_item_new_with_label(_("Best"));
    gtk_menu_append(menu, item);
    gtk_widget_show(item);
    g_signal_connect(item, "activate", G_CALLBACK(menuQualityBest), this);

}

// Create a Control menu that can be used from the menu bar or the popup.
void
GtkGui::createControlMenu(GtkWidget *obj)
{

    // Movie Control Menu
    GtkWidget *control = gtk_menu_item_new_with_mnemonic(_("Movie _Control"));
    gtk_widget_show(control);
    gtk_container_add(GTK_CONTAINER(obj), control);
    
    GtkWidget *menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(control), menu);

    // Play
#if GTK_CHECK_VERSION(2,6,0)
    GtkWidget *play = gtk_image_menu_item_new_from_stock("gtk-media-play", 0);
#else
    GtkWidget *play = gtk_menu_item_new_with_label(_("Play"));
#endif
    gtk_menu_append(menu, play);
    gtk_widget_show(play);    
    g_signal_connect (play, "activate", G_CALLBACK(menuPlay), this);

    // Pause
#if GTK_CHECK_VERSION(2,6,0)
    GtkWidget *pause = 
        gtk_image_menu_item_new_from_stock ("gtk-media-pause", 0);
#else
    GtkWidget *pause = gtk_menu_item_new_with_label(_("Pause"));
#endif
    gtk_menu_append(menu, pause);
    gtk_widget_show(pause);
    g_signal_connect(pause, "activate", G_CALLBACK(menuPause), this);

    // Stop
#if GTK_CHECK_VERSION(2,6,0)
    GtkWidget *stop = gtk_image_menu_item_new_from_stock("gtk-media-stop", 0);
#else
    GtkWidget *stop = gtk_menu_item_new_with_label(_("Stop"));
#endif
    gtk_menu_append(menu, stop);
    gtk_widget_show(stop);
    g_signal_connect(stop, "activate", G_CALLBACK(menuStop), this);

    GtkWidget *separator1 = gtk_separator_menu_item_new();
    gtk_widget_show(separator1);
    gtk_container_add(GTK_CONTAINER(menu), separator1);

    // Restart
    // 
    GtkWidget *restart = gtk_image_menu_item_new_with_label(_("Restart Movie"));

    // Suitable image?
    gtk_menu_append(menu, restart);
    gtk_widget_show(restart);
    g_signal_connect(restart, "activate", G_CALLBACK(menuRestart), this);

}

// This assumes that the parent of _drawingArea is _window, which
// isn't the case in the plugin fullscreen (it's _overlay). Currently
// we return from fullscreen when Gui::stop() is called, which
// seems like a good idea, and also avoids this problem.
void
GtkGui::stopHook()
{

    // Assert they're either both initialised or both uninitialised
    assert ((_resumeButton && _vbox) || !(_resumeButton || _vbox));
    if (_resumeButton) {
        gtk_box_pack_start(GTK_BOX(_vbox), _resumeButton, FALSE, FALSE, 0);
    }

    stopAdvanceTimer();
}

void
GtkGui::playHook()
{
    assert ((_resumeButton && _vbox) || !(_resumeButton || _vbox));
    if (_resumeButton) {
        gtk_container_remove(GTK_CONTAINER(_vbox), _resumeButton);
    }

    startAdvanceTimer();
}

// See if the X11 server we're using supports an extension.
bool 
GtkGui::checkX11Extension(const std::string& ext)
{
#ifdef HAVE_X11
#if GTK_CHECK_VERSION(2,22,0)
	#define GDK_DISPLAY() (GDK_DISPLAY_XDISPLAY(gdk_display_get_default()))
#endif 
    int n = 0;
    char **extlist = XListExtensions(GDK_DISPLAY(), &n);

    if (extlist) {
        for (int i = 0; i < n; i++) {
            if (std::strncmp(ext.c_str(), extlist[i], ext.size()) == 0) {
                return true;
            }
        }
    }
#endif /* HAVE_X11 */ 
    // do not free, Xlib can depend on contents being unaltered
    return false;
}

bool
GtkGui::yesno(const std::string& question)
{
    bool ret = true;

    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(_window),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_QUESTION,
        GTK_BUTTONS_YES_NO,
        "%s", question.c_str());

    switch (gtk_dialog_run(GTK_DIALOG(dialog)))
    {
        case GTK_RESPONSE_YES:
            ret = true;
            break;
        case GTK_RESPONSE_NO:
            ret = false;
            break;
        default:
            break;
    }

    gtk_widget_destroy(dialog);

    return ret;
}


/// Anonymous namespace for callbacks, local functions, event handlers etc.
namespace {

static GList *pixmaps_directories = NULL;

// Adds the Gnash icon to a window.
void
addGnashIcon(GtkWindow* window)
{
    GdkPixbuf *window_icon_pixbuf = createPixbuf ("GnashG.png");
    if (window_icon_pixbuf) {
        gtk_window_set_icon (GTK_WINDOW (window), window_icon_pixbuf);
        gdk_pixbuf_unref (window_icon_pixbuf);
    }
}

/* This is an internally used function to create pixmaps. */
GdkPixbuf*
createPixbuf (const gchar *filename)
{
    gchar *pathname = NULL;
    GdkPixbuf *pixbuf;
    GError *error = NULL;

    if (!filename || !filename[0])
       return NULL;

    pathname = findPixmapFile (filename);

    if (!pathname) {
        log_error (_("Couldn't find pixmap file: %s"), filename);
        g_warning (_("Couldn't find pixmap file: %s"), filename);
        return NULL;
    }

    pixbuf = gdk_pixbuf_new_from_file (pathname, &error);
    if (!pixbuf) {
        log_error (_("Failed to load pixbuf file: %s: %s"), pathname, error->message);
        g_error_free (error);
    }
    g_free (pathname);
    return pixbuf;
}

key::code
gdk_to_gnash_key(guint key)
{
    key::code c(key::INVALID);

    // ascii 32-126 in one range:    
    if (key >= GDK_space && key <= GDK_asciitilde) {
        c = (key::code) ((key - GDK_space) + key::SPACE);
    }

    // Function keys:
    else if (key >= GDK_F1 && key <= GDK_F15)    {
        c = (key::code) ((key - GDK_F1) + key::F1);
    }

    // Keypad:
    else if (key >= GDK_KP_0 && key <= GDK_KP_9) {
        c = (key::code) ((key - GDK_KP_0) + key::KP_0);
    }

    // Extended ascii:
    else if (key >= GDK_nobreakspace && key <= GDK_ydiaeresis) {
        c = (key::code) ((key - GDK_nobreakspace) + 
                key::NOBREAKSPACE);
    }

    // non-character keys don't correlate, so use a look-up table.
    else {
        struct {
            guint             gdk;
            key::code  gs;
        } table[] = {
            { GDK_BackSpace, key::BACKSPACE },
            { GDK_Tab, key::TAB },
            { GDK_Clear, key::CLEAR },
            { GDK_Return, key::ENTER },
            
            { GDK_Shift_L, key::SHIFT },
            { GDK_Shift_R, key::SHIFT },
            { GDK_Control_L, key::CONTROL },
            { GDK_Control_R, key::CONTROL },
            { GDK_Alt_L, key::ALT },
            { GDK_Alt_R, key::ALT },
            { GDK_Caps_Lock, key::CAPSLOCK },
            
            { GDK_Escape, key::ESCAPE },
            
            { GDK_Page_Down, key::PGDN },
            { GDK_Page_Up, key::PGUP },
            { GDK_Home, key::HOME },
            { GDK_End, key::END },
            { GDK_Left, key::LEFT },
            { GDK_Up, key::UP },
            { GDK_Right, key::RIGHT },
            { GDK_Down, key::DOWN },
            { GDK_Insert, key::INSERT },
            { GDK_Delete, key::DELETEKEY },
            
            { GDK_Help, key::HELP },
            { GDK_Num_Lock, key::NUM_LOCK },

            { GDK_VoidSymbol, key::INVALID }
        };
        
        for (int i = 0; table[i].gdk != GDK_VoidSymbol; i++) {
            if (key == table[i].gdk) {
                c = table[i].gs;
                break;
            }
        }
    }
    
    return c;
}

int
gdk_to_gnash_modifier(int state)
{
    int modifier = key::GNASH_MOD_NONE;

    if (state & GDK_SHIFT_MASK) {
      modifier = modifier | key::GNASH_MOD_SHIFT;
    }
    if (state & GDK_CONTROL_MASK) {
      modifier = modifier | key::GNASH_MOD_CONTROL;
    }
    if (state & GDK_MOD1_MASK) {
      modifier = modifier | key::GNASH_MOD_ALT;
    }

    return modifier;
}

/* Use this function to set the directory containing installed pixmaps. */
void
addPixmapDirectory(const gchar* directory)
{
    pixmaps_directories = 
        g_list_prepend(pixmaps_directories, g_strdup(directory));
}


/* This is an internally used function to find pixmap files. */
gchar*
findPixmapFile(const gchar* filename)
{
    GList *elem;

    /* We step through each of the pixmaps directory to find it. */
    elem = pixmaps_directories;
    while (elem) {
        gchar *pathname = g_strdup_printf("%s%s%s", (gchar*)elem->data,
                G_DIR_SEPARATOR_S, filename);
        if (g_file_test (pathname, G_FILE_TEST_EXISTS))
            return pathname;
        g_free (pathname);
        elem = elem->next;
    }
    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///                             Event Handlers                              ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

gboolean
configureEvent(GtkWidget *const /*widget*/, GdkEventConfigure *const event,
        const gpointer data)
{
    GtkGui* obj = static_cast<GtkGui*>(data);
    obj->resize_view(event->width, event->height);

    return false;
}

gboolean
realizeEvent(GtkWidget* /*widget*/, GdkEvent* /*event*/, gpointer /*data*/)
{
    return true;
}

// Shut everything down and exit when we're destroyed as a window
gboolean
deleteEvent(GtkWidget* /*widget*/, GdkEvent* /*event*/, gpointer data)
{
    Gui* gui = static_cast<Gui*>(data);
    gui->quit();
    return true;
}


gboolean
keyPressEvent(GtkWidget *const /*widget*/, GdkEventKey *const event,
        const gpointer data)
{

    Gui* gui = static_cast<Gui*>(data);

    /* Forward key event to gnash */
    key::code c = gdk_to_gnash_key(event->keyval);
    int mod = gdk_to_gnash_modifier(event->state);
    
    if (c != key::INVALID) {
        gui->notify_key_event(c, mod, true);
    }
        
    return true;
}

gboolean
keyReleaseEvent(GtkWidget *const /*widget*/, GdkEventKey *const event,
        const gpointer data)
{

    Gui* gui = static_cast<Gui*>(data);

    /* Forward key event to gnash */
    key::code    c = gdk_to_gnash_key(event->keyval);
    int mod = gdk_to_gnash_modifier(event->state);
    
    if (c != key::INVALID) {
        gui->notify_key_event(c, mod, false);
    }
    
    return true;
}

gboolean
mouseWheelEvent(GtkWidget *const /*widget*/, GdkEventScroll* const event,
        const gpointer data)
{
    assert(event->type == GDK_SCROLL);
    GtkGui *obj = static_cast<GtkGui*>(data);

    obj->grabFocus();

    switch (event->direction) {
        case GDK_SCROLL_UP:
            obj->notifyMouseWheel(1);
            break;
        case GDK_SCROLL_DOWN:
            obj->notifyMouseWheel(-1);
            break;
        default:
            break;
    }


    return true;
}

gboolean
buttonPressEvent(GtkWidget *const /*widget*/, GdkEventButton *const event,
        const gpointer data)
{

    /// Double- and triple-clicks should not send an extra event!
    /// Flash has no built-in double click.
    if (event->type != GDK_BUTTON_PRESS) return false;

    GtkGui *obj = static_cast<GtkGui*>(data);

    obj->grabFocus();
    obj->notifyMouseClick(true);
    return true;
}

gboolean
buttonReleaseEvent(GtkWidget * const /*widget*/,
     GdkEventButton* const /*event*/, const gpointer data)
{
    Gui *obj = static_cast<Gui*>(data);
    obj->notifyMouseClick(false);
    return true;
}

gboolean
motionNotifyEvent(GtkWidget *const /*widget*/, GdkEventMotion *const event,
        const gpointer data)
{
    Gui *obj = static_cast<Gui *>(data);

    obj->notifyMouseMove(event->x, event->y);
    return true;
}

gboolean
visibilityNotifyEvent(GtkWidget *const /*widget*/, GdkEventVisibility  *const event,
                  const gpointer data)
{
    GtkGui *obj = static_cast<GtkGui *>(data);

    switch (event->state) {
        case GDK_VISIBILITY_FULLY_OBSCURED:
            obj->setVisible(false);
            break;
        case GDK_VISIBILITY_PARTIAL:
        case GDK_VISIBILITY_UNOBSCURED:
            obj->setVisible(true);
            break;
    }
        
    return false; // propagate the event to other listeners, if any.
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///                             Callbacks                                   ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/// This method is called when the "OK" button is clicked in the open file
/// dialog. For GTK <= 2.4.0, this is a callback called by GTK itself.
void
openFile(GtkWidget *widget, gpointer /* user_data */)
{
#if 0
    // We'll need this when implementing file opening.
    GtkGui* gui = static_cast<GtkGui*>(user_data);
#endif
   
#if GTK_CHECK_VERSION(2,4,0)
    char* filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget));
#else   
    GtkWidget* file_selector = gtk_widget_get_ancestor(widget,
                                 g_type_from_name("GtkFileSelection"));

    GtkFileSelection* filesel = GTK_FILE_SELECTION (file_selector);
    const char* filename = gtk_file_selection_get_filename (filesel);
#endif

    // FIXME: we want to do something like calling gtk_main_quit here, so
    // run() will return. If run() is then changed to return a pointer to the
    // next file to be played, then the Player class can play the next file,
    // unless run() returns NULL.
    log_error (_("Attempting to open file %s.\n"
               "NOTE: the file open functionality is not yet implemented!"),
               filename);

#if GTK_CHECK_VERSION(2,4,0)
    g_free(filename);
#endif
}


void
menuOpenFile(GtkMenuItem* /*menuitem*/, gpointer data)
{
    GtkWidget* dialog;
    GtkGui* gui = static_cast<GtkGui*>(data);

#if GTK_CHECK_VERSION(2,4,0)
    dialog = gtk_file_chooser_dialog_new (_("Open file"),
                                          NULL,
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                          NULL);
    
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
        openFile(dialog, gui);
    }
    
    gtk_widget_destroy (dialog);
#else
    dialog = gtk_file_selection_new (_("Open file"));

    GtkFileSelection* selector = GTK_FILE_SELECTION(dialog);

    g_signal_connect (selector->ok_button, "clicked", G_CALLBACK (openFile),
                      gui);

    g_signal_connect_swapped(selector->ok_button, "clicked", 
                              G_CALLBACK(gtk_widget_destroy), dialog);

    g_signal_connect_swapped(selector->cancel_button, "clicked",
                              G_CALLBACK(gtk_widget_destroy), dialog); 
   
    gtk_widget_show (dialog);
#endif // GTK_CHECK_VERSION(2,4,0)
}


/// 'About' callback
void
menuAbout(GtkMenuItem* /*menuitem*/, gpointer data)
{
    GtkGui* gui = static_cast<GtkGui*>(data);
    gui->showAboutDialog();
}

/// Preferences callback
void
menuPreferences(GtkMenuItem* /*menuitem*/, gpointer data)
{
    GtkGui* gui = static_cast<GtkGui*>(data);
    gui->showPreferencesDialog();
}

// Properties Callback
void
menuMovieInfo(GtkMenuItem* /*menuitem*/, gpointer data)
{
    GtkGui* gui = static_cast<GtkGui*>(data);
    gui->showPropertiesDialog();
}

/// \brief This pops up the menu when the right mouse button is clicked
gint
popupHandler(GtkWidget *widget, GdkEvent *event)
{
    GtkMenu *menu = GTK_MENU(widget);
    
    if( _showMenuState ) {
        if (event->type == GDK_BUTTON_PRESS) {
            GdkEventButton* event_button =
                            reinterpret_cast<GdkEventButton*>(event);
            if (event_button->button == 3) {
                gtk_menu_popup(menu, NULL, NULL, NULL, NULL,
                               event_button->button, event_button->time);
                return TRUE;
            } 
        }
    }

    return FALSE;
}

/// \brief This handles the alternative popup for showMenu 
gint
popupHandlerAlt(GtkWidget *widget, GdkEvent *event)
{
    GtkMenu *menu = GTK_MENU(widget);
    
    
    if( !_showMenuState ) {
        if (event->type == GDK_BUTTON_PRESS) {
            GdkEventButton* event_button =
                            reinterpret_cast<GdkEventButton*>(event);
            if (event_button->button == 3) {
                gtk_menu_popup(menu, NULL, NULL, NULL, NULL,
                               event_button->button, event_button->time);
                return TRUE;
            } 
        }
    }

    return FALSE;
}

/// \brief Toggle the sound on or off
void
menuSound(GtkMenuItem* /*menuitem*/, gpointer data)
{
    Gui* gui = static_cast<Gui*>(data);
    gui->toggleSound();
}

void
menuFullscreen(GtkMenuItem* /*menuitem*/, gpointer data)
{
    Gui* gui = static_cast<Gui*>(data);
    gui->toggleFullscreen();
}

void
timeoutQuit(gpointer data)
{
    Gui* gui = static_cast<Gui*>(data);
    gui->quit();
}


/// \brief restart the movie from the beginning
void
menuRestart(GtkMenuItem* /*menuitem*/, gpointer data)
{
    Gui* gui = static_cast<Gui*>(data);
    gui->restart();
}

void
menuQuit(GtkMenuItem* /*menuitem*/, gpointer data)
{
    Gui* gui = static_cast<Gui*>(data);
    gui->quit();
}

/// \brief Start the movie playing from the current frame.
void
menuPlay(GtkMenuItem* /*menuitem*/, gpointer data)
{
    Gui* gui = static_cast<Gui*>(data);
    gui->play();
}

/// \brief toggle between playing or paused.
void
menuPause(GtkMenuItem* /*menuitem*/, gpointer data)
{
    Gui* gui = static_cast<Gui*>(data);
    gui->pause();
}

/// \brief stop the movie that's playing.
void
menuStop(GtkMenuItem* /*menuitem*/, gpointer data)
{
    Gui* gui = static_cast<Gui*>(data);
    gui->stop();
}


/// \brief Force redraw
void
menuRefreshView(GtkMenuItem* /*menuitem*/, gpointer data)
{
    Gui* gui = static_cast<Gui*>(data);
    gui->refreshView();
}

/// \brief Force redraw
void
menuShowUpdatedRegions(GtkMenuItem* /*menuitem*/, gpointer data)
{
    Gui* gui = static_cast<Gui*>(data);
    gui->showUpdatedRegions(!gui->showUpdatedRegions());
    
    // refresh to clear the remaining red lines...
    if (!gui->showUpdatedRegions()) {
        gui->refreshView();
    }
}

/// \brief Set quality to LOW level
void
menuQualityLow(GtkMenuItem* /*menuitem*/, gpointer data)
{
    Gui* gui = static_cast<Gui*>(data);
    gui->setQuality(QUALITY_LOW);
}

/// \brief Set quality to MEDIUM level
void
menuQualityMedium(GtkMenuItem* /*menuitem*/, gpointer data)
{
    Gui* gui = static_cast<Gui*>(data);
    gui->setQuality(QUALITY_MEDIUM);
}

/// \brief Set quality to HIGH level
void
menuQualityHigh(GtkMenuItem* /*menuitem*/, gpointer data)
{
    Gui* gui = static_cast<Gui*>(data);
    gui->setQuality(QUALITY_HIGH);
}

/// \brief Set quality to BEST level
void
menuQualityBest(GtkMenuItem* /*menuitem*/, gpointer data)
{
    Gui* gui = static_cast<Gui*>(data);
    gui->setQuality(QUALITY_BEST);
}

} // anonymous namespace

} // end of namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
