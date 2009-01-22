// gtk.cpp: Gnome ToolKit graphical user interface, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "log.h"

#include "gui.h"
#include "rc.h"
#include "gtksup.h"
#include "sound_handler.h"
#include "render_handler.h"
#include "VM.h"
#include "lirc.h"

#include <iostream>
#ifdef HAVE_X11
#include <X11/keysym.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <string>

#ifdef HAVE_FFMPEG_AVCODEC_H
extern "C" {
# include "ffmpeg/avcodec.h" // Only for the version number
}
#endif

#ifdef HAVE_LIBAVCODEC_AVCODEC_H
extern "C" {
# include "libavcodec/avcodec.h" // Only for the version number
}
#endif

#ifdef HAVE_GST_GST_H
# include "gst/gstversion.h" // Only for the version number
#endif

#ifdef RENDERER_OPENGL
#include "gtk_glue_gtkglext.h"
#endif

#ifdef RENDERER_CAIRO
#include "gtk_glue_cairo.h"
#endif

#ifdef RENDERER_AGG
#include "gtk_glue_agg.h"
#endif

#ifdef GUI_HILDON
# include <hildon/hildon.h>
#endif

namespace gnash 
{

bool createFileMenu(GtkWidget *obj);
bool createEditMenu(GtkWidget *obj);
bool createHelpMenu(GtkWidget *obj);
bool createControlMenu(GtkWidget *obj);

// This is global so it can be accessed by the evnt handler, which
// isn't part of this class. 
Lirc *lirc;
bool lirc_handler(void*, int, void* data);

GtkGui::~GtkGui()
{
}

GtkGui::GtkGui(unsigned long xid, float scale, bool loop, unsigned int depth)
	:
	Gui(xid, scale, loop, depth)
#ifdef GUI_HILDON
	,_hildon_program(0)
#endif
	,_window(0)
	,_resumeButton(0)
	,_overlay(0)
	,_drawingArea(0)
	,_popup_menu(0)
	,_menubar(0)
	,_vbox(0)
	,_glue()
	,_advanceSourceTimer(0)
{
}

bool
GtkGui::init(int argc, char **argv[])
{
    //GNASH_REPORT_FUNCTION;

    gtk_init (&argc, argv);

#ifdef GUI_HILDON
    _hildon_program = hildon_program_get_instance();
#endif
    
    // TODO: don't rely on a macro to select renderer
#ifdef RENDERER_CAIRO
    _glue.reset(new GtkCairoGlue);
#elif defined(RENDERER_OPENGL)
    _glue.reset(new GtkGlExtGlue);
#elif defined(RENDERER_AGG)
    _glue.reset(new GtkAggGlue);
#endif
    _glue->init (argc, argv);

    add_pixmap_directory (PKGDATADIR);

    if (_xid) {
        _window = gtk_plug_new(_xid);
        log_debug (_("Created XEmbedded window"));
    } else {
#ifdef GUI_HILDON
        _window = hildon_window_new();
        hildon_program_add_window(_hildon_program, HILDON_WINDOW(_window));
#else
        _window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
#endif
        log_debug (_("Created top level window"));
    }
    
    addGnashIcon(GTK_WINDOW(_window));

    _drawingArea = gtk_drawing_area_new();

    // Increase reference count to prevent its destruction (which could happen
    // later if we remove it from its container).
    g_object_ref(G_OBJECT(_drawingArea));

    _resumeButton = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(_resumeButton),
            gtk_label_new(_("Click to play")));
    gtk_widget_show_all(_resumeButton);

    // Same here.
    g_object_ref(G_OBJECT(_resumeButton));

    // This callback indirectly results in playHook() being called.
    g_signal_connect(G_OBJECT(_resumeButton), "clicked",
            G_CALLBACK(menuitem_play_callback), this);

    // If we don't set this flag we won't be able to grab focus
    // ( grabFocus() would be a no-op )
    GTK_WIDGET_SET_FLAGS (GTK_WIDGET(_drawingArea), GTK_CAN_FOCUS);

    createMenu();

#ifdef RENDERER_OPENGL
    // OpenGL _glue needs to prepare the drawing area for OpenGL rendering before
    // widgets are realized and before the configure event is fired.
    // TODO: find a way to make '_glue' use independent from actual renderer in use
    _glue->prepDrawingArea(_drawingArea);
#endif

    // A vertical box is used to allow display of the menu bar and paused widget
    _vbox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(_vbox);
    gtk_container_add(GTK_CONTAINER(_window), _vbox);

#if defined(USE_MENUS) && !defined(GUI_HILDON)
    if ( ! _xid ) {
        createMenuBar();
    }
#endif

    gtk_box_pack_start(GTK_BOX(_vbox), _drawingArea, TRUE, TRUE, 0);

    setupEvents();

    gtk_widget_realize(_window);
    gtk_widget_show(_drawingArea);
    gtk_widget_show(_window);

#if defined(RENDERER_CAIRO) || defined(RENDERER_AGG)
    // cairo needs the _drawingArea.window to prepare it ..
    // TODO: find a way to make '_glue' use independent from actual renderer in use
    _glue->prepDrawingArea(_drawingArea);
#endif
    
#ifdef USE_LIRC
    lirc = new Lirc();
    if (lirc->init("/dev/lircd")) {
        int fd = lirc->getFileFd();
        addFDListener(fd, lirc_handler, &fd);
    } else {
        log_debug(_("LIRC daemon not running"));
    }
#endif
    
    _renderer = _glue->createRenderHandler();
    if ( ! _renderer ) return false;
    set_render_handler(_renderer);

    // The first time stop() was called, stopHook() might not have had a chance
    // to do anything, because GTK+ wasn't garanteed to be initialised.
    //if (isStopped()) stopHook();

    return true;
}

bool
GtkGui::run()
{
    //GNASH_REPORT_FUNCTION;

    // Kick-start before setting the interval timeout
    advance_movie(this);
    
#if 0
    // From http://www.idt.mdh.se/kurser/cd5040/ht02/gtk/glib/glib-the-main-event-loop.html#G-TIMEOUT-ADD-FULL
    //
    // Note that timeout functions may be delayed, due to the
    // processing of other event sources. Thus they should not be
    // relied on for precise timing. After each call to the timeout
    // function, the time of the next timeout is recalculated based
    // on the current time and the given interval (it does not try to
    // 'catch up' time lost in delays).
    //
    // NOTE: this is OK (research on 'elastic frame rate').
    //
    _advanceSourceTimer = g_timeout_add_full (G_PRIORITY_LOW, _interval, (GSourceFunc)advance_movie,
                        this, NULL);
#endif

    gtk_main();
    return true;
}

void
GtkGui::setTimeout(unsigned int timeout)
{
    g_timeout_add(timeout, (GSourceFunc)gtk_main_quit, NULL);
}

bool
GtkGui::addFDListener(int fd, callback_t callback, void* data)
{
    // NOTE: "The default encoding for GIOChannel is UTF-8. If your application
    // is reading output from a command using via pipe, you may need to set the
    // encoding to the encoding of the current locale (see g_get_charset())
    // with the g_io_channel_set_encoding() function."

    GIOChannel* gio_read = g_io_channel_unix_new(fd);
    
    if (!gio_read) {
        return false;
    }
    
    if (!g_io_add_watch (gio_read, GIOCondition(G_IO_IN | G_IO_HUP),
                         GIOFunc (callback), data)) {
        g_io_channel_unref(gio_read);
        return false;    
    }
    
    return true;    
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
        //log_debug (_("Created fullscreen window"));
        
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
        // This way allows the drawing area to be shrunk, which is what we really want,
        // but not only after we've gone fullscreen.
        // It could be a good hack if it were done earlier.
        // There really doesn't seem to be a proper way of setting the starting size
        // of a widget but allowing it to be shrunk.
        gtk_widget_set_size_request(_drawingArea, -1, -1);
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
        gtk_widget_reparent (_vbox, _window);
        
        // Apply key event callbacks to the plugin instance.
        setupWindowEvents();
        if (_overlay) {
            gtk_widget_destroy(_overlay);
            //log_debug (_("Destroyed fullscreen window"));
        }        
    }
    
    // Stand-alone
    else {
	    gtk_window_unfullscreen(GTK_WINDOW(_window));
	    showMenu(true);
    }
    
    _fullscreen = false;
}

void 
GtkGui::setCursor(gnash_cursor_type newcursor)
{
  //GNASH_REPORT_FUNCTION;

	if (! _mouseShown) return;

    GdkCursorType cursortype;

    switch(newcursor) {
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
    gdk_window_set_cursor (_drawingArea->window, gdkcursor);
  
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

    if (!show)
    {
        GdkPixmap *pixmap;
        GdkColor *color;

        color = g_new0(GdkColor, 1);
        pixmap = gdk_pixmap_new(NULL, 1, 1, 1);
        GdkCursor* cursor = gdk_cursor_new_from_pixmap(pixmap, pixmap,
                                                    color, color, 0, 0);

        gdk_window_set_cursor (_drawingArea->window, cursor);

        g_free(color);
        g_object_unref(pixmap);	
        gdk_cursor_unref(cursor);

        _mouseShown = false;

    }
	else if (show)
    {
        _mouseShown = true;	
    }
    
    return state;
}

void
GtkGui::showMenu(bool show)
{
#ifdef USE_MENUS
if (_menubar)
{
    if (show) {
        gtk_widget_show(_menubar);
	    return;
	}
    gtk_widget_hide(_menubar);
}
#endif
}

double
GtkGui::getPixelAspectRatio()
{
    GdkScreen* screen = gdk_screen_get_default();

    // Screen size / number of pixels = pixel size.
    // The physical size of the screen may be reported wrongly by gdk (from X),
    // but it's the best we have. This method agrees with the pp in my case.
    double pixelAspectRatio =
            (gdk_screen_get_height_mm(screen) / static_cast<double>(getScreenResY())) / 
            (gdk_screen_get_width_mm(screen) / static_cast<double>(getScreenResX()));
    return pixelAspectRatio;
}

int
GtkGui::getScreenResX()
{
    return gdk_screen_width();
}

int
GtkGui::getScreenResY()
{
    return gdk_screen_height(); 
}

double
GtkGui::getScreenDPI()
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
    g_signal_connect(G_OBJECT(gtk_widget_get_toplevel(_drawingArea)), "delete_event",
                   G_CALLBACK(delete_event), this);
    g_signal_connect(G_OBJECT(gtk_widget_get_toplevel(_drawingArea)), "key_press_event",
                   G_CALLBACK(key_press_event), this);
    g_signal_connect(G_OBJECT(gtk_widget_get_toplevel(_drawingArea)), "key_release_event",
                   G_CALLBACK(key_release_event), this);
}

// public virtual
bool
GtkGui::setupEvents()
{
  //GNASH_REPORT_FUNCTION;

    setupWindowEvents();

    gtk_widget_add_events(_drawingArea, GDK_EXPOSURE_MASK
                        | GDK_BUTTON_PRESS_MASK
                        | GDK_BUTTON_RELEASE_MASK
                        | GDK_KEY_RELEASE_MASK
                        | GDK_KEY_PRESS_MASK        
                        | GDK_POINTER_MOTION_MASK);
  
    g_signal_connect_swapped(G_OBJECT(_drawingArea),
                            "button_press_event",
                            G_CALLBACK(popup_handler),
                            GTK_OBJECT(_popup_menu));
  
    g_signal_connect(G_OBJECT(_drawingArea), "button_press_event",
                   G_CALLBACK(button_press_event), this);
    g_signal_connect(G_OBJECT(_drawingArea), "button_release_event",
                   G_CALLBACK(button_release_event), this);
    g_signal_connect(G_OBJECT(_drawingArea), "motion_notify_event",
                   G_CALLBACK(motion_notify_event), this);
  
    g_signal_connect_after(G_OBJECT (_drawingArea), "realize",
                         G_CALLBACK (realize_event), NULL);
    g_signal_connect(G_OBJECT (_drawingArea), "configure_event",
                   G_CALLBACK (configure_event), this);
    g_signal_connect(G_OBJECT (_drawingArea), "expose_event",
                    G_CALLBACK (expose_event), this);

    return true;
}

void
GtkGui::grabFocus()
{
    gtk_widget_grab_focus(GTK_WIDGET(_drawingArea));
}

void
GtkGui::quit()
{
    gtk_main_quit();
}

void
GtkGui::setInterval(unsigned int interval)
{
    _interval = interval;
    if (_advanceSourceTimer)
    {
        g_source_remove(_advanceSourceTimer);
    }
#if 1
    _advanceSourceTimer = g_timeout_add_full (G_PRIORITY_LOW, _interval, (GSourceFunc)advance_movie,
                        this, NULL);
    log_debug("Advance interval timer set to %d ms (~ %d FPS)", _interval, 1000/_interval);
#endif
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
#ifdef GUI_HILDON
//     _hildon_toolbar = create_hildon_toolbar(_hildon_program);
//     hildon_window_add_toolbar(HILDON_WINDOW(_window),
//                               GTK_TOOLBAR(_hildon_toolbar));
#else
    gtk_box_pack_start(GTK_BOX (_vbox), _menubar, FALSE, FALSE, 0);
#endif

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

    GtkWidget *separator1 = gtk_separator_menu_item_new ();
    gtk_widget_show (separator1);
    gtk_container_add (GTK_CONTAINER (_popup_menu), separator1);

    /// The sound handler is initialized after the Gui is created, and
    /// may be disabled or enabled dynamically.
    GtkCheckMenuItem *menuitem_sound =
        GTK_CHECK_MENU_ITEM(gtk_check_menu_item_new_with_label(_("Sound")));
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(menuitem_sound), TRUE);
    gtk_menu_append(_popup_menu, GTK_WIDGET(menuitem_sound));
    gtk_widget_show(GTK_WIDGET(menuitem_sound));
    g_signal_connect(GTK_OBJECT(menuitem_sound), "activate",
                     G_CALLBACK(&menuitem_sound_callback), this);

    GtkMenuItem *menuitem_quit =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label(_("Quit Gnash")));
    gtk_menu_append(_popup_menu, GTK_WIDGET(menuitem_quit));
    gtk_widget_show(GTK_WIDGET(menuitem_quit));
    g_signal_connect(GTK_OBJECT(menuitem_quit), "activate",
                     G_CALLBACK(&menuitem_quit_callback), this);

#ifdef GUI_HILDON
     hildon_window_set_menu(HILDON_WINDOW(_window),
                               GTK_MENU(_popup_menu));
     gtk_widget_show_all(GTK_WIDGET(_popup_menu));   
#endif

    return true;
}

bool
GtkGui::createWindow(const char *title, int width, int height)
{
// First call the old createWindow function and then set the title.
// In case there's some need to not setting the title.
    bool ret = createWindow(width, height);
    gtk_window_set_title(GTK_WINDOW(_window), title);

    if (!_xid) {
    
        // This sets the *minimum* size for the drawing area and thus will
        // also resize the window. 
        // Advantage: The window is sized correctly, no matter what other
        // widgets are visible
        // Disadvantage: The window cannot be shrinked, which is bad.   
        gtk_widget_set_size_request(_drawingArea, width, height);

    }
    return ret;
}

static GList *pixmaps_directories = NULL;

/* Use this function to set the directory containing installed pixmaps. */
void
GtkGui::add_pixmap_directory                   (const gchar     *directory)
{
    pixmaps_directories = g_list_prepend (pixmaps_directories, g_strdup (directory));
}


/* This is an internally used function to find pixmap files. */
gchar*
GtkGui::find_pixmap_file                       (const gchar     *filename)
{
    GList *elem;

    /* We step through each of the pixmaps directory to find it. */
    elem = pixmaps_directories;
    while (elem) {
        gchar *pathname = g_strdup_printf ("%s%s%s", (gchar*)elem->data,
                G_DIR_SEPARATOR_S, filename);
        if (g_file_test (pathname, G_FILE_TEST_EXISTS))
            return pathname;
        g_free (pathname);
        elem = elem->next;
    }
    return NULL;
}


/* This is an internally used function to create pixmaps. */
GdkPixbuf*
GtkGui::createPixbuf (const gchar *filename)
{
    gchar *pathname = NULL;
    GdkPixbuf *pixbuf;
    GError *error = NULL;

    if (!filename || !filename[0])
       return NULL;

    pathname = find_pixmap_file (filename);

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

#ifdef USE_SWFTREE

// This creates a GtkTree model for displaying movie info.
GtkTreeModel*
GtkGui::makeTreeModel (std::auto_ptr<InfoTree> treepointer)
{

    InfoTree& info = *treepointer;

    enum
    {
        STRING1_COLUMN,
        STRING2_COLUMN,
        NUM_COLUMNS
    };
    
    GtkTreeStore *model = gtk_tree_store_new (NUM_COLUMNS,
                         G_TYPE_STRING,
                         G_TYPE_STRING);
    
    GtkTreeIter iter;
    GtkTreeIter child_iter;
    GtkTreeIter parent_iter;

    // Depth within the *GTK* tree.
    int depth = 0;    

    assert(info.depth(info.begin()) == 0); // seems assumed in the code below
    for (InfoTree::iterator i=info.begin(), e=info.end(); i!=e; ++i)
    {
        StringPair& p = *i;

        std::ostringstream os;
        os << info.depth(i);  

        int newdepth = info.depth(i);

        if (newdepth > depth) {
            assert(newdepth == depth+1);
            depth++;                   
            iter=child_iter;                  
        }

        if (newdepth < depth) {
            int gap = depth-newdepth;
            depth = newdepth;
            while(gap--)
            {
                gtk_tree_model_iter_parent (GTK_TREE_MODEL(model), &parent_iter, &iter);  
                iter = parent_iter;
	    }
        }

        //Read in data from present node
        if (depth == 0) gtk_tree_store_append (model, &child_iter, NULL);
        else gtk_tree_store_append (model, &child_iter, &iter);

        gtk_tree_store_set (model, &child_iter,
                           STRING1_COLUMN, p.first.c_str(),   // "Variable"
    		               STRING2_COLUMN, p.second.c_str(),  // "Value"
			               -1);

    }

    return GTK_TREE_MODEL(model);

}

#endif

// Adds the Gnash icon to a window.
void
GtkGui::addGnashIcon(GtkWindow* window)
{
    GdkPixbuf *window_icon_pixbuf = createPixbuf ("GnashG.png");
    if (window_icon_pixbuf) {
        gtk_window_set_icon (GTK_WINDOW (window), window_icon_pixbuf);
		gdk_pixbuf_unref (window_icon_pixbuf);
    }
}

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
    //GNASH_REPORT_FUNCTION;
    
    assert(_width>0);
    assert(_height>0);
    
    _width = width;
    _height = height;
    
    _validbounds.setTo(0, 0, _width, _height);
    _glue->setRenderHandlerSize(_width, _height);
    
    return true;
}

void
GtkGui::beforeRendering()
{
    _glue->beforeRendering();
}

void
GtkGui::renderBuffer()
{
    gdk_window_process_updates(_drawingArea->window, false);
}

void
GtkGui::expose(const GdkRegion *region) 
{
    gint num_rects;
    GdkRectangle* rects;

    // In some versions of GTK this can't be const...
    GdkRegion* nonconst_region = const_cast<GdkRegion*>(region);

    gdk_region_get_rectangles (nonconst_region, &rects, &num_rects);
    assert(num_rects);

    for (int i=0; i<num_rects; ++i) {
      const GdkRectangle& cur_rect = rects[i];
      _glue->render(cur_rect.x, cur_rect.y, cur_rect.x + cur_rect.width,
                    cur_rect.y + cur_rect.height);
    }

    g_free(rects);
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
        gdk_window_invalidate_rect(_drawingArea->window, &rect, false);
    }

}

/// This method is called when the "OK" button is clicked in the open file
/// dialog. For GTK <= 2.4.0, this is a callback called by GTK itself.
void GtkGui::openFile (GtkWidget *widget, gpointer /* user_data */)
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


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///                             Dialogues                                   ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace { // anonimous

class PreferencesDialog {

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
        GtkWidget *lcTraceToggle;
        GtkWidget *solSandbox;
        GtkWidget *osText;
        GtkWidget *versionText;
        GtkWidget *urlOpenerText;
        GtkWidget *librarySize;
        GtkWidget *startStoppedToggle;
#ifdef USE_DEBUGGER
        GtkWidget *DebuggerToggle;
#endif

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
        	lcTraceToggle(0),
        	solSandbox(0),
        	osText(0),
        	versionText(0),
        	urlOpenerText(0),
        	librarySize(0),
        	startStoppedToggle(0)
#ifdef USE_DEBUGGER
        	,DebuggerToggle(0)
#endif
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
PreferencesDialog::handlePrefs (GtkWidget* dialog, gint response, gpointer data)
{

    PrefWidgets *prefs = static_cast<PrefWidgets*>(data);

    if (response == GTK_RESPONSE_OK) {

        // If 'Save' was clicked, set all the values in _rcfile
        RcInitFile& _rcfile = RcInitFile::getDefaultInstance();
        // For getting from const gchar* to std::string&
        std::string tmp;
    
        if ( prefs->soundToggle )
            _rcfile.useSound(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(prefs->soundToggle)));
    
        if ( prefs->actionDumpToggle )
            _rcfile.useActionDump(
    		    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(prefs->actionDumpToggle)));

        if ( prefs->parserDumpToggle )
            _rcfile.useParserDump(
    		    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(prefs->parserDumpToggle)));
    	
        if ( prefs->logfileName ) {
    	    tmp = gtk_entry_get_text(GTK_ENTRY(prefs->logfileName));
            _rcfile.setDebugLog(tmp);
        }
        
        if ( prefs->writeLogToggle ) {
            _rcfile.useWriteLog(
        	    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(prefs->writeLogToggle)));
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
                gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(prefs->ASCodingErrorToggle)));
        }

        if ( prefs->malformedSWFToggle ) {
            _rcfile.showMalformedSWFErrors(
                gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(prefs->malformedSWFToggle)));
        }

        if ( prefs->localHostToggle ) {
            _rcfile.useLocalHost(
                gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(prefs->localHostToggle)));
        }

        if ( prefs->localDomainToggle ) {
            _rcfile.useLocalDomain(
                gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(prefs->localDomainToggle)));
        }

        if ( prefs->solLocalDomainToggle ) {
            _rcfile.setSOLLocalDomain(
                gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(prefs->solLocalDomainToggle)));
        }

        if ( prefs->solReadOnlyToggle ) {
            _rcfile.setSOLReadOnly(
                gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(prefs->solReadOnlyToggle)));
        }

        if ( prefs->localConnectionToggle ) {
            _rcfile.setLocalConnection(
                gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(prefs->localConnectionToggle)));
        }

        if ( prefs->insecureSSLToggle ) {
            _rcfile.insecureSSL(
                gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(prefs->insecureSSLToggle)));
        }

        if ( prefs->lcTraceToggle ) {
            _rcfile.setLCTrace(
                gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(prefs->lcTraceToggle)));
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
    	
    	// Let _rcfile decide which file to update: generally the file being used if
    	// specified in GNASHRC environment variable, or in the user's home directory
    	// if that can be found.
    	// TODO: We can also specify here which file should be written by passing
    	// that instead. How might that best be done?
    	_rcfile.updateFile();

        // Close the window when 'ok' is clicked
        gtk_widget_destroy(dialog);
    }

    else if (response == GTK_RESPONSE_CANCEL) {
        // Close the window when 'cancel' is clicked
        gtk_widget_destroy(dialog);
    }
    
    // response == GTK_RESPONSE_DELETE_EVENT

    if (prefs) delete prefs;
}


void
PreferencesDialog::show()
{
    gtk_widget_show_all (_prefsDialog);    
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
    GtkGui::addGnashIcon(GTK_WINDOW(_prefsDialog));

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
    gtk_box_pack_start(GTK_BOX(timeoutbox), _prefs->streamsTimeoutScale, FALSE, FALSE, 0);
    // Align to _rcfile value:
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(_prefs->streamsTimeoutScale), _rcfile.getStreamsTimeout());

}

void
PreferencesDialog::addLoggingTab()
{
    GtkWidget *loggingvbox = gtk_vbox_new (FALSE, 10);
   
    // Tab label
    GtkWidget *loggingtablabel = gtk_label_new_with_mnemonic (_("_Logging"));
    gtk_notebook_append_page(GTK_NOTEBOOK(_notebook), GTK_WIDGET(loggingvbox), loggingtablabel); 

    // Logging options
    GtkWidget *logginglabel = gtk_label_new (_("<b>Logging options</b>"));
    gtk_label_set_use_markup (GTK_LABEL (logginglabel), TRUE);
    gtk_box_pack_start(GTK_BOX(loggingvbox), logginglabel, FALSE, FALSE, 0);
    
    GtkWidget *verbositylabel = gtk_label_new (_("Verbosity level:"));
    gtk_box_pack_start(GTK_BOX(loggingvbox), verbositylabel, FALSE, FALSE, 0);
    gtk_misc_set_alignment (GTK_MISC (verbositylabel), 0, 0.5);

    _prefs->verbosityScale = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (_rcfile.verbosityLevel(), 0, 10, 1, 0, 0)));
    gtk_scale_set_digits (GTK_SCALE (_prefs->verbosityScale), 0);
    gtk_range_set_update_policy (GTK_RANGE (_prefs->verbosityScale), GTK_UPDATE_DISCONTINUOUS);
    gtk_box_pack_start(GTK_BOX(loggingvbox), _prefs->verbosityScale, FALSE, FALSE, 0);
    
    _prefs->writeLogToggle = gtk_check_button_new_with_mnemonic (_("Log to _file"));
    gtk_box_pack_start(GTK_BOX(loggingvbox), _prefs->writeLogToggle, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (_prefs->writeLogToggle), _rcfile.useWriteLog());
    
    GtkWidget *logfilelabel = gtk_label_new (_("Logfile name:"));
    gtk_box_pack_start(GTK_BOX(loggingvbox), logfilelabel, FALSE, FALSE, 0);
    gtk_misc_set_alignment (GTK_MISC (logfilelabel), 0, 0.5);
    
    _prefs->logfileName = gtk_entry_new ();
    gtk_box_pack_start(GTK_BOX(loggingvbox), _prefs->logfileName, FALSE, FALSE, 0);
    // Put debug filename in the entry box      
    gtk_entry_set_text(GTK_ENTRY(_prefs->logfileName), _rcfile.getDebugLog().c_str());
    
    _prefs->parserDumpToggle = gtk_check_button_new_with_mnemonic (_("Log _parser output"));
    gtk_box_pack_start(GTK_BOX(loggingvbox), _prefs->parserDumpToggle, FALSE, FALSE, 0);
    // Align button state with _rcfile
    gtk_toggle_button_set_active (
    		GTK_TOGGLE_BUTTON (_prefs->parserDumpToggle),
    		_rcfile.useParserDump());

    _prefs->actionDumpToggle = gtk_check_button_new_with_mnemonic (_("Log SWF _actions"));
    gtk_box_pack_start(GTK_BOX(loggingvbox), _prefs->actionDumpToggle, FALSE, FALSE, 0);
    // Align button state with _rcfile
    gtk_toggle_button_set_active (
    		GTK_TOGGLE_BUTTON (_prefs->actionDumpToggle),
    		_rcfile.useActionDump());

    _prefs->malformedSWFToggle = gtk_check_button_new_with_mnemonic (_("Log malformed SWF _errors"));
    gtk_box_pack_start(GTK_BOX(loggingvbox), _prefs->malformedSWFToggle, FALSE, FALSE, 0);
    // Align button state with _rcfile
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (_prefs->malformedSWFToggle),
    			_rcfile.showMalformedSWFErrors());

    _prefs->ASCodingErrorToggle = gtk_check_button_new_with_mnemonic (_("Log ActionScript _coding errors"));
    gtk_box_pack_start(GTK_BOX(loggingvbox), _prefs->ASCodingErrorToggle, FALSE, FALSE, 0);
    // Align button state with _rcfile
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (_prefs->ASCodingErrorToggle),
    			_rcfile.showASCodingErrors());

    _prefs->lcTraceToggle = gtk_check_button_new_with_mnemonic (
    				_("Log _Local Connection activity"));
    gtk_box_pack_start (GTK_BOX(loggingvbox), _prefs->lcTraceToggle, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (_prefs->lcTraceToggle),
    			_rcfile.getLCTrace()); 

#ifdef USE_DEBUGGER

    _prefs->DebuggerToggle = gtk_check_button_new_with_mnemonic (_("Enable _debugger"));
    gtk_box_pack_start(GTK_BOX(loggingvbox), _prefs->DebuggerToggle, FALSE, FALSE, 0);
    // Align button state with _rcfile
    gtk_toggle_button_set_active (
    			GTK_TOGGLE_BUTTON (_prefs->DebuggerToggle),
    			_rcfile.useDebugger());

#endif

}

void
PreferencesDialog::addSecurityTab()
{
    // Security Tab
    GtkWidget *securityvbox = gtk_vbox_new (FALSE, 14);

    // Security tab title
    GtkWidget *securitytablabel = gtk_label_new_with_mnemonic (_("_Security"));
    
    gtk_notebook_append_page(GTK_NOTEBOOK(_notebook), GTK_WIDGET(securityvbox), securitytablabel); 
    
    // Network connection
    GtkWidget *netconnectionslabel = gtk_label_new (_("<b>Network connections</b>"));
    gtk_label_set_use_markup (GTK_LABEL (netconnectionslabel), TRUE);
    gtk_box_pack_start(GTK_BOX(securityvbox), netconnectionslabel, FALSE, FALSE, 0);
 
    _prefs->localHostToggle = gtk_check_button_new_with_mnemonic (_("Connect only to local _host"));
    gtk_box_pack_start (GTK_BOX(securityvbox), _prefs->localHostToggle, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (_prefs->localHostToggle), _rcfile.useLocalHost());
    
    _prefs->localDomainToggle = gtk_check_button_new_with_mnemonic (_("Connect only to local _domain"));
    gtk_box_pack_start (GTK_BOX(securityvbox), _prefs->localDomainToggle, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (_prefs->localDomainToggle), _rcfile.useLocalDomain());

    _prefs->insecureSSLToggle = gtk_check_button_new_with_mnemonic (_("Disable SSL _verification"));
    gtk_box_pack_start (GTK_BOX(securityvbox), _prefs->insecureSSLToggle, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (_prefs->insecureSSLToggle), _rcfile.insecureSSL());
    
    GtkWidget *whitelistexpander = gtk_expander_new_with_mnemonic (_("_Whitelist"));
    gtk_box_pack_start (GTK_BOX (securityvbox), whitelistexpander, FALSE, FALSE, 0);
    
    GtkWidget *whitelistcomboboxentry1 = gtk_combo_box_entry_new_text ();
    gtk_container_add (GTK_CONTAINER(whitelistexpander), whitelistcomboboxentry1);

    GtkWidget *blacklistexpander = gtk_expander_new_with_mnemonic (_("_Blacklist"));
    gtk_box_pack_start (GTK_BOX (securityvbox), blacklistexpander, FALSE, FALSE, 0);
    
    GtkWidget *blacklistcomboboxentry2 = gtk_combo_box_entry_new_text ();
    gtk_container_add (GTK_CONTAINER(blacklistexpander), blacklistcomboboxentry2);

    // Privacy
    GtkWidget *privacylabel = gtk_label_new (_("<b>Privacy</b>"));
    gtk_label_set_use_markup (GTK_LABEL (privacylabel), TRUE);
    gtk_box_pack_start (GTK_BOX(securityvbox), privacylabel, FALSE, FALSE, 0);

    GtkWidget *solsandboxlabel = gtk_label_new (_("Shared objects directory:"));
    gtk_box_pack_start (GTK_BOX(securityvbox), solsandboxlabel, FALSE, FALSE, 0);
    gtk_misc_set_alignment (GTK_MISC (solsandboxlabel), 0, 0.5);

    _prefs->solSandbox = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(_prefs->solSandbox), _rcfile.getSOLSafeDir().c_str());
    gtk_box_pack_start (GTK_BOX(securityvbox), _prefs->solSandbox, FALSE, FALSE, 0);

    _prefs->solReadOnlyToggle = gtk_check_button_new_with_mnemonic ( 
    				_("Do _not write Shared Object files"));
    gtk_box_pack_start (GTK_BOX(securityvbox), _prefs->solReadOnlyToggle, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (_prefs->solReadOnlyToggle),
    			_rcfile.getSOLReadOnly());

    _prefs->solLocalDomainToggle = gtk_check_button_new_with_mnemonic (
    				_("Only _access local Shared Object files"));
    gtk_box_pack_start (GTK_BOX(securityvbox), _prefs->solLocalDomainToggle, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (_prefs->solLocalDomainToggle),
    			_rcfile.getSOLLocalDomain());

    _prefs->localConnectionToggle = gtk_check_button_new_with_mnemonic (
    				_("Disable Local _Connection object"));
    gtk_box_pack_start (GTK_BOX(securityvbox), _prefs->localConnectionToggle, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (_prefs->localConnectionToggle),
    			_rcfile.getLocalConnection()); 
}

void
PreferencesDialog::addMediaTab()
{
    // Media Tab
    GtkWidget *mediavbox = gtk_vbox_new (FALSE, 2);

    // Media tab title
    GtkWidget *mediatablabel = gtk_label_new_with_mnemonic (_("_Media"));
    
    gtk_notebook_append_page(GTK_NOTEBOOK(_notebook), GTK_WIDGET(mediavbox), mediatablabel); 
    
    // Sound
    GtkWidget *soundlabel = gtk_label_new (_("<b>Sound</b>"));
    gtk_label_set_use_markup (GTK_LABEL (soundlabel), TRUE);
    gtk_box_pack_start(GTK_BOX(mediavbox), soundlabel, FALSE, FALSE, 0);
   
    _prefs->soundToggle = gtk_check_button_new_with_mnemonic (_("Use sound _handler"));
    gtk_box_pack_start (GTK_BOX(mediavbox), _prefs->soundToggle, FALSE, FALSE, 0);
    // Align button state with rcfile
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (_prefs->soundToggle), _rcfile.useSound());
}

void
PreferencesDialog::addPlayerTab()
{
    // Player Tab
    GtkWidget *playervbox = gtk_vbox_new (FALSE, 14);

    // Player tab title
    GtkWidget *playertablabel = gtk_label_new_with_mnemonic (_("_Player"));
    
    gtk_notebook_append_page(GTK_NOTEBOOK(_notebook), GTK_WIDGET(playervbox), playertablabel); 

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
    gtk_box_pack_start(GTK_BOX(versionhbox), _prefs->versionText, FALSE, FALSE, 0);
    // Put text in the entry box      
    gtk_entry_set_text(GTK_ENTRY(_prefs->versionText), _rcfile.getFlashVersionString().c_str());

    // OS label
    GtkWidget *oshbox = gtk_hbox_new (FALSE, 2);
    gtk_box_pack_start(GTK_BOX(playervbox), oshbox, FALSE, FALSE, 0);
    
    GtkWidget *OSlabel = gtk_label_new (_("Operating system:"));
    gtk_misc_set_alignment (GTK_MISC (OSlabel), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(oshbox), OSlabel, FALSE, FALSE, 0);
    
    _prefs->osText = gtk_entry_new ();
    gtk_box_pack_start(GTK_BOX(oshbox), _prefs->osText, FALSE, FALSE, 0);
    // Put text in the entry box      
    gtk_entry_set_text(GTK_ENTRY(_prefs->osText), _rcfile.getFlashSystemOS().c_str());
    
    GtkWidget *OSadvicelabel = gtk_label_new (_("<i>If blank, Gnash will "
    					   "detect your OS</i>"));
    gtk_label_set_use_markup (GTK_LABEL (OSadvicelabel), TRUE);
    gtk_misc_set_alignment (GTK_MISC (OSadvicelabel), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(playervbox), OSadvicelabel, FALSE, FALSE, 0);     

    // URL opener
    GtkWidget *urlopenerbox = gtk_hbox_new (FALSE, 2);
    gtk_box_pack_start(GTK_BOX(playervbox), urlopenerbox, FALSE, FALSE, 0);
    
    GtkWidget *urlopenerlabel = gtk_label_new (_("URL opener:"));
    gtk_misc_set_alignment (GTK_MISC (urlopenerlabel), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(urlopenerbox), urlopenerlabel, FALSE, FALSE, 0);
    
    _prefs->urlOpenerText = gtk_entry_new ();
    gtk_box_pack_start(GTK_BOX(urlopenerbox), _prefs->urlOpenerText, FALSE, FALSE, 0);
    // Put text in the entry box      
    gtk_entry_set_text(GTK_ENTRY(_prefs->urlOpenerText), _rcfile.getURLOpenerFormat().c_str());

    // Performance
    GtkWidget *performancelabel = gtk_label_new (_("<b>Performance</b>"));
    gtk_label_set_use_markup (GTK_LABEL (performancelabel), TRUE);
    gtk_box_pack_start(GTK_BOX(playervbox), performancelabel, FALSE, FALSE, 0);

    // Library size
    GtkWidget *libraryhbox = gtk_hbox_new (FALSE, 2);
    gtk_box_pack_start(GTK_BOX(playervbox), libraryhbox, FALSE, FALSE, 0);

    GtkWidget *librarylabel = gtk_label_new (_("Max size of movie library:"));
    gtk_misc_set_alignment (GTK_MISC (librarylabel), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(libraryhbox), librarylabel, FALSE, FALSE, 0);

    _prefs->librarySize = gtk_spin_button_new_with_range(0, 100, 1);
    gtk_box_pack_start(GTK_BOX(libraryhbox), _prefs->librarySize, FALSE, FALSE, 0);
    // Align to _rcfile value:
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(_prefs->librarySize), _rcfile.getMovieLibraryLimit());

    _prefs->startStoppedToggle = gtk_check_button_new_with_mnemonic (
    				_("Start _Gnash in pause mode"));
    gtk_box_pack_start (GTK_BOX(playervbox), _prefs->startStoppedToggle, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (_prefs->startStoppedToggle),
    			_rcfile.startStopped());
} // end of ::addPlayerTab



} // end of anonimous namespace


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
    gtk_window_set_default_size (GTK_WINDOW(propsDialog),
                                 500, 300);
    
    // Suggest to the window manager to allow "maximize"
    // As there can be (will be) a lot of data.
    gtk_window_set_type_hint(GTK_WINDOW(propsDialog),
                            GDK_WINDOW_TYPE_HINT_NORMAL);

    addGnashIcon(GTK_WINDOW(propsDialog));

    // Destroy the window when a button is clicked.
    g_signal_connect (propsDialog, "response",
               G_CALLBACK(gtk_widget_destroy), NULL);

    GtkWidget *propsvbox = gtk_vbox_new (FALSE, 1);
    gtk_container_add (GTK_CONTAINER (
                        GTK_DIALOG(propsDialog)->vbox), propsvbox);

#ifdef USE_SWFTREE

    std::auto_ptr<InfoTree> infoptr = getMovieInfo();

    GtkWidget *scrollwindow1 = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollwindow1),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_AUTOMATIC);

    gtk_box_pack_start (
            GTK_BOX (propsvbox), scrollwindow1, TRUE, TRUE, 0);

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

    gint coloffset;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    // Add columns:
    
    // 'Variable' column:
    renderer = gtk_cell_renderer_text_new ();
    coloffset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(treeview),
					       -1, _("Variable"),
					       renderer, "text",
					       STRING1_COLUMN,
					       NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), coloffset - 1);

    // 'Value' column:
    // Set to be 'editable' so that the data can be selected and
    // copied; it can't actually be edited, though.
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer, "xalign", 0.0, "editable", TRUE, NULL);
    coloffset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(treeview),
					       -1, _("Value"),
					       renderer, "text",
					       STRING2_COLUMN,
					       NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), coloffset - 1);

    //Add tree to scrollwindow.
    gtk_container_add (GTK_CONTAINER (scrollwindow1), treeview);

#endif

    gtk_widget_show_all (propsDialog);

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
	NULL
    };

    std::string comments = _("Gnash is the GNU SWF Player based on GameSWF.");

    comments.append(_("\nRenderer: "));
    comments.append(RENDERER_CONFIG);
    comments.append(_("\nGUI: "));
    comments.append("GTK2"); // gtk of course!
    comments.append(_("\nMedia: "));
    comments.append(MEDIA_CONFIG" ");
#ifdef HAVE_GST_GST_H
    comments.append(_("\nBuilt against gstreamer version: "));
    std::ostringstream ss;
    ss << GST_VERSION_MAJOR << "." << GST_VERSION_MINOR << "." << GST_VERSION_MICRO;
    comments.append(ss.str());
#endif
#ifdef HAVE_FFMPEG_AVCODEC_H
    comments.append(_("\nBuilt against ffmpeg version: "));
    comments.append(LIBAVCODEC_IDENT);
#endif

    gtk_about_dialog_set_url_hook(NULL, NULL, NULL);
    GdkPixbuf *logo_pixbuf = createPixbuf("GnashG.png");

#if 1
    // gtk-2.8.20 (Debian 4.0) doesn't work fine with 
    // the gtk_show_about_dialog() call [ omits info ].
    // See bug #24426.

    GtkWidget* aboutWidget = gtk_about_dialog_new();
    GtkAboutDialog* about = GTK_ABOUT_DIALOG(aboutWidget);

    gtk_about_dialog_set_name (about, "Gnash");
    gtk_about_dialog_set_version(about, VERSION);
    gtk_about_dialog_set_copyright(about, "Copyright (C) 2005, 2006, 2007, "
            "2008 The Free Software Foundation");
    gtk_about_dialog_set_comments (about, comments.c_str());
    gtk_about_dialog_set_authors(about, authors);
    gtk_about_dialog_set_documenters(about, documentors);
    gtk_about_dialog_set_artists(about, artists);
    gtk_about_dialog_set_translator_credits(about, _("translator-credits"));
    gtk_about_dialog_set_logo(about, logo_pixbuf);
    gtk_about_dialog_set_license(about, 
        "This program is free software; you can redistribute it and/or modify\n"
        "it under the terms of the GNU General Public License as published by\n"
        "the Free Software Foundation; either version 3 of the License, or\n"
        "(at your option) any later version.\n\n"
        "This program is distributed in the hope that it will be useful,\n"
        "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
        "GNU General Public License for more details.\n"
        "You should have received a copy of the GNU General Public License\n"
        "along with this program; if not, write to the Free Software\n"
        "Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 "
        "USA"
	);
    gtk_about_dialog_set_website(about, "http://www.gnu.org/software/gnash/");

    // Destroy the dialogue box when 'close' is clicked.
    g_signal_connect(G_OBJECT(aboutWidget),
            "response",  G_CALLBACK(gtk_widget_destroy), aboutWidget);

    gtk_widget_show (aboutWidget);

#else


    gtk_show_about_dialog (
        NULL,
        "program-name", _("Gnash"), 
        "version", VERSION,
        "copyright", "Copyright (C) 2005, 2006, 2007, 2008 "
                     "The Free Software Foundation",
        "comments", comments.c_str(),
        "authors", authors,
        "documenters", documentors,
        "artists", artists,
        "translator-credits", _("translator-credits"),
        "logo", logo_pixbuf,
        "license", 
        "This program is free software; you can redistribute it and/or modify\n"
        "it under the terms of the GNU General Public License as published by\n"
        "the Free Software Foundation; either version 3 of the License, or\n"
        "(at your option) any later version.\n\n"
        "This program is distributed in the hope that it will be useful,\n"
        "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
        "GNU General Public License for more details.\n"
        "You should have received a copy of the GNU General Public License\n"
        "along with this program; if not, write to the Free Software\n"
        "Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA",
        "website", "http://www.gnu.org/software/gnash/",
        NULL);
#endif
	if (logo_pixbuf)
		gdk_pixbuf_unref(logo_pixbuf);
}

void
GtkGui::menuitem_openfile_callback(GtkMenuItem* /*menuitem*/, gpointer data)
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

    g_signal_connect_swapped (selector->ok_button, "clicked", 
                              G_CALLBACK (gtk_widget_destroy), dialog);

    g_signal_connect_swapped (selector->cancel_button, "clicked",
                              G_CALLBACK (gtk_widget_destroy), dialog); 
   
    gtk_widget_show (dialog);
#endif // GTK_CHECK_VERSION(2,4,0)
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///                             Callbacks                                   ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/// 'About' callback
void
GtkGui::menuitem_about_callback(GtkMenuItem* /*menuitem*/, gpointer data)
{
    GtkGui* gui = static_cast<GtkGui*>(data);
    gui->showAboutDialog();
}

/// Preferences callback
void
GtkGui::menuitem_preferences_callback(GtkMenuItem* /*menuitem*/, gpointer data)
{
    
    GtkGui* gui = static_cast<GtkGui*>(data);
    gui->showPreferencesDialog();
}

// Properties Callback
void
GtkGui::menuitem_movieinfo_callback(GtkMenuItem* /*menuitem*/, gpointer data)
{
    GtkGui* gui = static_cast<GtkGui*>(data);
    gui->showPropertiesDialog();
}

// This pops up the menu when the right mouse button is clicked
gint
GtkGui::popup_handler(GtkWidget *widget, GdkEvent *event)
{
//    GNASH_REPORT_FUNCTION;

    GtkMenu *menu = GTK_MENU(widget);
//    printf("event type # %i\n", event->type);
    if (event->type == GDK_BUTTON_PRESS) {
        GdkEventButton *event_button = (GdkEventButton *) event;
        if (event_button->button == 3) {
            gtk_menu_popup(menu, NULL, NULL, NULL, NULL,
                           event_button->button, event_button->time);
            return TRUE;
        }
    }
    return FALSE;
}

/// \brief Toggle the sound on or off
void
GtkGui::menuitem_sound_callback(GtkMenuItem* /*menuitem*/, gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    Gui* gui = static_cast<Gui*>(data);
    gui->toggleSound();
}

void
GtkGui::menuitem_fullscreen_callback(GtkMenuItem* /*menuitem*/, gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    Gui* gui = static_cast<Gui*>(data);
    gui->toggleFullscreen();
}


/// \brief restart the movie from the beginning
void
GtkGui::menuitem_restart_callback(GtkMenuItem* /*menuitem*/, gpointer data)
{
    //GNASH_REPORT_FUNCTION;
    Gui* gui = static_cast<Gui*>(data);
    gui->restart();
}

/// \brief quit complete, and close the application
void
GtkGui::menuitem_quit_callback(GtkMenuItem* /*menuitem*/, gpointer /*data*/)
{
//    GNASH_REPORT_FUNCTION;

    gtk_main_quit();
}

/// \brief Start the movie playing from the current frame.
void
GtkGui::menuitem_play_callback(GtkMenuItem* /*menuitem*/, gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    Gui* gui = static_cast<Gui*>(data);
    gui->play();
}

/// \brief toggle between playing or paused.
void
GtkGui::menuitem_pause_callback(GtkMenuItem* /*menuitem*/, gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    Gui* gui = static_cast<Gui*>(data);
    gui->pause();
}

/// \brief stop the movie that's playing.
void
GtkGui::menuitem_stop_callback(GtkMenuItem* /*menuitem*/, gpointer data)
{
    GNASH_REPORT_FUNCTION;
    Gui* gui = static_cast<Gui*>(data);
    gui->stop();
}

/// \brief step forward 1 frame
void
GtkGui::menuitem_step_forward_callback(GtkMenuItem* /*menuitem*/,
		gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    Gui* gui = static_cast<Gui*>(data);
    gui->menu_step_forward();
}

/// \brief step backward 1 frame
void
GtkGui::menuitem_step_backward_callback(GtkMenuItem* /*menuitem*/,
		gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    Gui* gui = static_cast<Gui*>(data);
    gui->menu_step_backward();
}

/// \brief jump forward 10 frames
void
GtkGui::menuitem_jump_forward_callback(GtkMenuItem* /*menuitem*/,
                               gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    Gui* gui = static_cast<Gui*>(data);
    gui->menu_jump_forward();
}

/// \brief jump backward 10 frames
void
GtkGui::menuitem_jump_backward_callback(GtkMenuItem* /*menuitem*/,
                                gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    Gui* gui = static_cast<Gui*>(data);
    gui->menu_jump_backward();
}

/// \brief Force redraw
void
GtkGui::menuitem_refresh_view_callback(GtkMenuItem* /*menuitem*/,
                                gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    Gui* gui = static_cast<Gui*>(data);
    gui->refreshView();
}

/// \brief Force redraw
void
GtkGui::menuitem_show_updated_regions_callback(GtkMenuItem* /*menuitem*/,
                                gpointer data)
{
    Gui* gui = static_cast<Gui*>(data);
    gui->showUpdatedRegions(! (gui->showUpdatedRegions()) );
    
    // refresh to clear the remaining red lines...
    if ( ! (gui->showUpdatedRegions()))
      gui->refreshView();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///                             Event Handlers                              ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

gboolean
GtkGui::expose_event(GtkWidget *const /*widget*/,
             GdkEventExpose *const event,
             const gpointer data)
{
//	GNASH_REPORT_FUNCTION;

    GtkGui* gui = static_cast<GtkGui*>(data);

    gui->expose(event->region);

    return TRUE;
}

gboolean
GtkGui::configure_event(GtkWidget *const widget,
                GdkEventConfigure *const event,
                const gpointer data)
{
//    GNASH_REPORT_FUNCTION;

    GtkGui* obj = static_cast<GtkGui*>(data);

    GtkGlue& glue = *(obj->_glue);

    glue.configure(widget, event);
    obj->resize_view(event->width, event->height);

    return TRUE;
}

gboolean
GtkGui::realize_event(GtkWidget* /*widget*/, GdkEvent* /*event*/,
		gpointer /*data*/)
{
    //GNASH_REPORT_FUNCTION;

    return TRUE;
}

// Shut everything down and exit when we're destroyed as a window
gboolean
GtkGui::delete_event(GtkWidget* /*widget*/, GdkEvent* /*event*/,
			gpointer /*data*/)
{
    GNASH_REPORT_FUNCTION;

    gtk_main_quit();
    return TRUE;
}


gboolean
GtkGui::key_press_event(GtkWidget *const /*widget*/,
                GdkEventKey *const event,
                const gpointer data)
{
    //GNASH_REPORT_FUNCTION;

    Gui* gui = static_cast<Gui*>(data);

    /* Forward key event to gnash */
    gnash::key::code	c = gdk_to_gnash_key(event->keyval);
    int mod = gdk_to_gnash_modifier(event->state);
    
    if (c != gnash::key::INVALID) {
        gui->notify_key_event(c, mod, true);
    }
        
    return true;
}

gboolean
GtkGui::key_release_event(GtkWidget *const /*widget*/,
                GdkEventKey *const event,
                const gpointer data)
{
    //GNASH_REPORT_FUNCTION;

    Gui* gui = static_cast<Gui*>(data);

    /* Forward key event to gnash */
    gnash::key::code	c = gdk_to_gnash_key(event->keyval);
    int mod = gdk_to_gnash_modifier(event->state);
    
    if (c != gnash::key::INVALID) {
        gui->notify_key_event(c, mod, false);
    }
    
    return true;
}

gboolean
GtkGui::button_press_event(GtkWidget *const /*widget*/,
                           GdkEventButton *const event,
                           const gpointer data)
{

    /// Double- and triple-clicks should not send an extra event!
    /// Flash has no built-in double click.
    if (event->type != GDK_BUTTON_PRESS) return false;

    GtkGui *obj = static_cast<GtkGui *>(data);

    obj->grabFocus();

    int	mask = 1 << (event->button - 1);
    obj->notify_mouse_clicked(true, mask);
    return true;
}

gboolean
GtkGui::button_release_event(GtkWidget * const /*widget*/,
                             GdkEventButton * const event,
                             const gpointer data)
{
    //GNASH_REPORT_FUNCTION;
    Gui *obj = static_cast<Gui *>(data);

    int	mask = 1 << (event->button - 1);
    obj->notify_mouse_clicked(false, mask);
    return true;
}

gboolean
GtkGui::motion_notify_event(GtkWidget *const /*widget*/,
                            GdkEventMotion *const event,
                            const gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    Gui *obj = static_cast<Gui *>(data);

    obj->notify_mouse_moved(event->x, event->y);
    return true;
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
//    GNASH_REPORT_FUNCTION;
    GtkWidget *menuitem = gtk_menu_item_new_with_mnemonic (_("_File"));
    gtk_widget_show (menuitem);
    gtk_container_add (GTK_CONTAINER (obj), menuitem);
    
    GtkWidget *menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menu);

// Open    
    GtkWidget *open =
 	gtk_image_menu_item_new_from_stock ("gtk-open", NULL);
    gtk_widget_show (open);
    gtk_container_add (GTK_CONTAINER (menu), open);
    g_signal_connect ((gpointer) open, "activate",
                      G_CALLBACK (&menuitem_openfile_callback),
                      this);

// Save    
    GtkWidget *save =
 	gtk_image_menu_item_new_from_stock ("gtk-save", NULL);
    gtk_widget_show (save);
    gtk_container_add (GTK_CONTAINER (menu), save);
    // Disabled until save functionality is implemented:
    gtk_widget_set_sensitive(save,FALSE); 

// Save as
    GtkWidget *save_as =
 	gtk_image_menu_item_new_from_stock ("gtk-save-as", NULL);
    gtk_widget_show (save_as);
    gtk_container_add (GTK_CONTAINER (menu), save_as);
    // Disabled until save-as functionality is implemented:
    gtk_widget_set_sensitive(save_as,FALSE);
    
    GtkWidget *separatormenuitem1 = gtk_separator_menu_item_new ();
    gtk_widget_show (separatormenuitem1);
    gtk_container_add (GTK_CONTAINER (menu), separatormenuitem1);

// Properties
    GtkWidget *properties =
 	gtk_image_menu_item_new_from_stock ("gtk-properties", NULL);
    gtk_widget_show (properties);
    gtk_container_add (GTK_CONTAINER (menu), properties);
    g_signal_connect ((gpointer) properties, "activate",
                      G_CALLBACK (&menuitem_movieinfo_callback),
                      this);

    GtkWidget *separator2 = gtk_separator_menu_item_new ();
    gtk_widget_show (separator2);
    gtk_container_add (GTK_CONTAINER (menu), separator2);

    GtkWidget *quit = gtk_image_menu_item_new_from_stock ("gtk-quit", NULL);
    gtk_widget_show (quit);
    gtk_container_add (GTK_CONTAINER (menu), quit);

    g_signal_connect ((gpointer) quit, "activate",
                      G_CALLBACK (&menuitem_quit_callback),
                      this);
}

// Create an Edit menu that can be used from the menu bar or the popup.
void
GtkGui::createEditMenu(GtkWidget *obj)
{
//    GNASH_REPORT_FUNCTION;
    
    GtkWidget *menuitem = gtk_menu_item_new_with_mnemonic (_("_Edit"));
    gtk_widget_show (menuitem);
    gtk_container_add (GTK_CONTAINER (obj), menuitem);
    
    GtkWidget *menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menu);

    GtkWidget *preferences1 =
 	gtk_image_menu_item_new_from_stock ("gtk-preferences", NULL);
    gtk_widget_show (preferences1);
    gtk_container_add (GTK_CONTAINER (menu), preferences1);

    g_signal_connect ((gpointer) preferences1, "activate",
                      G_CALLBACK (&menuitem_preferences_callback),
                      this);
}

// Create a Help menu that can be used from the menu bar or the popup.
void
GtkGui::createHelpMenu(GtkWidget *obj)
{
//    GNASH_REPORT_FUNCTION;
    GtkWidget *menuitem = gtk_menu_item_new_with_mnemonic (_("_Help"));
    gtk_widget_show (menuitem);
    gtk_container_add (GTK_CONTAINER (obj), menuitem);
    
    GtkWidget *menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menu);

    GtkWidget *about = gtk_image_menu_item_new_from_stock ("gtk-about", NULL);
    gtk_widget_show (about);
    gtk_container_add (GTK_CONTAINER (menu), about);
    
    g_signal_connect ((gpointer) about, "activate",
                      G_CALLBACK (&menuitem_about_callback),
                      this);
}

// Create a View menu that can be used from the menu bar or the popup.
void
GtkGui::createViewMenu(GtkWidget *obj)
{

//    GNASH_REPORT_FUNCTION;
    GtkWidget *menuitem = gtk_menu_item_new_with_mnemonic (_("_View"));
    gtk_widget_show (menuitem);
    gtk_container_add (GTK_CONTAINER (obj), menuitem);
    
    GtkWidget *menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menu);

    // Refresh
    GtkImageMenuItem *menuitem_refresh =
 	GTK_IMAGE_MENU_ITEM(
	    gtk_image_menu_item_new_with_label(_("Redraw")));
    gtk_image_menu_item_set_image (menuitem_refresh,
				   gtk_image_new_from_stock("gtk-refresh",
						 	     GTK_ICON_SIZE_MENU));
    gtk_menu_append(menu, GTK_WIDGET(menuitem_refresh));
    gtk_widget_show(GTK_WIDGET(menuitem_refresh));
    g_signal_connect ((gpointer) menuitem_refresh, "activate",
        G_CALLBACK (&menuitem_refresh_view_callback), this);

    // Fullscreen
#if GTK_CHECK_VERSION(2,8,0)
    GtkImageMenuItem *menuitem_fullscreen = GTK_IMAGE_MENU_ITEM(
	    gtk_image_menu_item_new_with_label(_("Toggle fullscreen")));
    gtk_image_menu_item_set_image (menuitem_fullscreen,
				   gtk_image_new_from_stock("gtk-fullscreen",
						 	     GTK_ICON_SIZE_MENU));
#else
    GtkMenuItem *menuitem_fullscreen =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label(_("Toggle fullscreen")));
#endif
    gtk_menu_append(menu, GTK_WIDGET(menuitem_fullscreen));
    gtk_widget_show(GTK_WIDGET(menuitem_fullscreen));
    g_signal_connect(GTK_OBJECT(menuitem_fullscreen), "activate",
                         G_CALLBACK(&menuitem_fullscreen_callback), this);

// Can be disabled at compile time.
#ifndef DISABLE_REGION_UPDATES_DEBUGGING
    GtkCheckMenuItem *menuitem_show_updated_regions =
        GTK_CHECK_MENU_ITEM(gtk_check_menu_item_new_with_label(_("Show updated ranges")));
   
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(menuitem_show_updated_regions),
                                    showUpdatedRegions() );

    gtk_menu_append(menu, GTK_WIDGET(menuitem_show_updated_regions));
    gtk_widget_show(GTK_WIDGET(menuitem_show_updated_regions));
    g_signal_connect(GTK_OBJECT(menuitem_show_updated_regions), "activate",
                     G_CALLBACK(&menuitem_show_updated_regions_callback), this);
#endif

}

// Create a Control menu that can be used from the menu bar or the popup.
void
GtkGui::createControlMenu(GtkWidget *obj)
{
//    GNASH_REPORT_FUNCTION;

// Movie Control Menu
    GtkWidget *menuitem_control =
	gtk_menu_item_new_with_mnemonic (_("Movie _Control"));
    gtk_widget_show (menuitem_control);
    gtk_container_add (GTK_CONTAINER (obj), menuitem_control);
    
    GtkWidget *menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_control), menu);

// Play
#if GTK_CHECK_VERSION(2,6,0)
    GtkMenuItem *menuitem_play = GTK_MENU_ITEM(
 	gtk_image_menu_item_new_from_stock ("gtk-media-play", NULL));
#else
    GtkMenuItem *menuitem_play =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label(_("Play")));
#endif
    gtk_menu_append(menu, GTK_WIDGET(menuitem_play));
    gtk_widget_show(GTK_WIDGET(menuitem_play));    
    g_signal_connect ((gpointer) menuitem_play, "activate",
        G_CALLBACK (&menuitem_play_callback), this);

// Pause
#if GTK_CHECK_VERSION(2,6,0)
    GtkMenuItem *menuitem_pause = GTK_MENU_ITEM(
 	gtk_image_menu_item_new_from_stock ("gtk-media-pause", NULL));
#else
    GtkMenuItem *menuitem_pause =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label(_("Pause")));
#endif
    gtk_menu_append(menu, GTK_WIDGET(menuitem_pause));
    gtk_widget_show(GTK_WIDGET(menuitem_pause));
    g_signal_connect ((gpointer) menuitem_pause, "activate",
        G_CALLBACK (&menuitem_pause_callback), this);

// Stop
#if GTK_CHECK_VERSION(2,6,0)
    GtkMenuItem *menuitem_stop = GTK_MENU_ITEM(
 	gtk_image_menu_item_new_from_stock ("gtk-media-stop", NULL));
#else
    GtkMenuItem *menuitem_stop =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label(_("Stop")));
#endif
    gtk_menu_append(menu, GTK_WIDGET(menuitem_stop));
    gtk_widget_show(GTK_WIDGET(menuitem_stop));
    g_signal_connect ((gpointer) menuitem_stop, "activate",
        G_CALLBACK (&menuitem_stop_callback), this);

    GtkWidget *separator1 = gtk_separator_menu_item_new ();
    gtk_widget_show (separator1);
    gtk_container_add (GTK_CONTAINER (menu), separator1);

// Restart
// 
    GtkImageMenuItem *menuitem_restart =
 	GTK_IMAGE_MENU_ITEM(
	     gtk_image_menu_item_new_with_label(_("Restart Movie")));
// Suitable image?
    gtk_menu_append(menu, GTK_WIDGET(menuitem_restart));
    gtk_widget_show(GTK_WIDGET(menuitem_restart));
    g_signal_connect ((gpointer) menuitem_restart, "activate",
        G_CALLBACK (&menuitem_restart_callback), this);

#if 0 // Presently disabled

    GtkWidget *separator2 = gtk_separator_menu_item_new ();
    gtk_widget_show (separator1);
    gtk_container_add (GTK_CONTAINER (menu), separator2);

// Step Forward
    GtkImageMenuItem *menuitem_step_forward =
 	GTK_IMAGE_MENU_ITEM(
	    gtk_image_menu_item_new_with_label(_("Step Forward Frame")));
    gtk_image_menu_item_set_image (menuitem_step_forward,
				   gtk_image_new_from_stock("gtk-go-forward",
						 	     GTK_ICON_SIZE_MENU));
    gtk_menu_append(menu, GTK_WIDGET(menuitem_step_forward));
    gtk_widget_show(GTK_WIDGET(menuitem_step_forward));
//     gtk_widget_add_accelerator (GTK_WIDGET(menuitem_step_forward), "activate", accel_group,
//                                 GDK_bracketleft, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

// Step Backward
    GtkImageMenuItem *menuitem_step_backward =
 	GTK_IMAGE_MENU_ITEM(
	    gtk_image_menu_item_new_with_label(_("Step Backward Frame")));
    gtk_image_menu_item_set_image (menuitem_step_backward,
				   gtk_image_new_from_stock("gtk-go-back",
						 	     GTK_ICON_SIZE_MENU));
    gtk_menu_append(menu, GTK_WIDGET(menuitem_step_backward));
    gtk_widget_show(GTK_WIDGET(menuitem_step_backward));
//     gtk_widget_add_accelerator (GTK_WIDGET(menuitem_step_forward), "activate", accel_group,
//                                 GDK_bracketright, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

// Jump Forward
// Stock image from Gtk-2.6 should be ignored in earlier versions.
    GtkImageMenuItem *menuitem_jump_forward =
        GTK_IMAGE_MENU_ITEM(
	    gtk_image_menu_item_new_with_label(_("Jump Forward 10 Frames")));
    gtk_image_menu_item_set_image (menuitem_jump_forward,
				   gtk_image_new_from_stock("gtk-media-forward",
						 	     GTK_ICON_SIZE_MENU));
    gtk_menu_append(menu, GTK_WIDGET(menuitem_jump_forward));
    gtk_widget_show(GTK_WIDGET(menuitem_jump_forward));

// Jump Backward
// Stock image from Gtk-2.6 should be ignored in earlier versions.
    GtkImageMenuItem *menuitem_jump_backward =
 	GTK_IMAGE_MENU_ITEM(gtk_image_menu_item_new_with_label(_("Jump Backward 10 Frames")));
    gtk_image_menu_item_set_image (menuitem_jump_backward,
				   gtk_image_new_from_stock("gtk-media-rewind",
						 	     GTK_ICON_SIZE_MENU));
    gtk_menu_append(menu, GTK_WIDGET(menuitem_jump_backward));
    gtk_widget_show(GTK_WIDGET(menuitem_jump_backward));

#endif

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///                             Other stuff                                 ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

gnash::key::code
GtkGui::gdk_to_gnash_key(guint key)
{
    gnash::key::code  c(gnash::key::INVALID);

    // ascii 32-126 in one range:    
    if (key >= GDK_space && key <= GDK_asciitilde) {
        c = (gnash::key::code) ((key - GDK_space) + gnash::key::SPACE);
    }

    // Function keys:
    else if (key >= GDK_F1 && key <= GDK_F15)	{
        c = (gnash::key::code) ((key - GDK_F1) + gnash::key::F1);
    }

    // Keypad:
    else if (key >= GDK_KP_0 && key <= GDK_KP_9) {
        c = (gnash::key::code) ((key - GDK_KP_0) + gnash::key::KP_0);
    }

    // Extended ascii:
    else if (key >= GDK_nobreakspace && key <= GDK_ydiaeresis) {
        c = (gnash::key::code) ((key - GDK_nobreakspace) + gnash::key::NOBREAKSPACE);
    }

    // non-character keys don't correlate, so use a look-up table.
    else {
        struct {
            guint             gdk;
            gnash::key::code  gs;
        } table[] = {
            { GDK_BackSpace, gnash::key::BACKSPACE },
            { GDK_Tab, gnash::key::TAB },
            { GDK_Clear, gnash::key::CLEAR },
            { GDK_Return, gnash::key::ENTER },
            
            { GDK_Shift_L, gnash::key::SHIFT },
            { GDK_Shift_R, gnash::key::SHIFT },
            { GDK_Control_L, gnash::key::CONTROL },
            { GDK_Control_R, gnash::key::CONTROL },
            { GDK_Alt_L, gnash::key::ALT },
            { GDK_Alt_R, gnash::key::ALT },
            { GDK_Caps_Lock, gnash::key::CAPSLOCK },
            
            { GDK_Escape, gnash::key::ESCAPE },
            
            { GDK_Page_Down, gnash::key::PGDN },
            { GDK_Page_Up, gnash::key::PGUP },
            { GDK_Home, gnash::key::HOME },
            { GDK_End, gnash::key::END },
            { GDK_Left, gnash::key::LEFT },
            { GDK_Up, gnash::key::UP },
            { GDK_Right, gnash::key::RIGHT },
            { GDK_Down, gnash::key::DOWN },
            { GDK_Insert, gnash::key::INSERT },
            { GDK_Delete, gnash::key::DELETEKEY },
            
            { GDK_Help, gnash::key::HELP },
            { GDK_Num_Lock, gnash::key::NUM_LOCK },

            { GDK_VoidSymbol, gnash::key::INVALID }
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
GtkGui::gdk_to_gnash_modifier(int state)
{
    int modifier = gnash::key::GNASH_MOD_NONE;

    if (state & GDK_SHIFT_MASK) {
      modifier = modifier | gnash::key::GNASH_MOD_SHIFT;
    }
    if (state & GDK_CONTROL_MASK) {
      modifier = modifier | gnash::key::GNASH_MOD_CONTROL;
    }
    if (state & GDK_MOD1_MASK) {
      modifier = modifier | gnash::key::GNASH_MOD_ALT;
    }

    return modifier;
}

bool
lirc_handler(void*, int, void* /*data*/)
{ 
    GNASH_REPORT_FUNCTION;
//    int* fd = static_cast<int*>(data);
    
    // want to remove this handler. You may want to close fd.
    log_debug("%s\n", lirc->getButton());
  
    // Want to keep this handler
    return true;
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
}

void
GtkGui::playHook()
{
    assert ((_resumeButton && _vbox) || !(_resumeButton || _vbox));
    if (_resumeButton) {
        gtk_container_remove(GTK_CONTAINER(_vbox), _resumeButton);
    }
}

} // end of namespace gnash

