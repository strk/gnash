// gtk.cpp: Gnome ToolKit graphical user interface, for Gnash.
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

/* $Id: gtk.cpp,v 1.122 2007/11/22 16:19:57 rsavoye Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"

#include "gui.h"
#include "rc.h"
#include "gtksup.h"
#include "sound_handler.h"
#include "gnash.h" // for get_sound_handler
#include "render_handler.h"
#include "VM.h"
#include "lirc.h"

#include <iostream>
#include <X11/keysym.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <string>

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

using namespace std;

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
      log_msg (_("Created XEmbedded window"));
    } else {
#ifdef GUI_HILDON
      _window = hildon_window_new();
      hildon_program_add_window(_hildon_program, HILDON_WINDOW(_window));
#else
      _window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
#endif
      log_msg (_("Created top level window"));
    }

    // XXXbjacques: why do we need this?
    gtk_container_set_reallocate_redraws(GTK_CONTAINER (_window), TRUE);

    _window_icon_pixbuf = create_pixbuf ("GnashG.png");
    if (_window_icon_pixbuf) {
        gtk_window_set_icon (GTK_WINDOW (_window), _window_icon_pixbuf);
	gdk_pixbuf_unref (_window_icon_pixbuf);
    }
    _drawing_area = gtk_drawing_area_new();

    createMenu();
#ifdef RENDERER_OPENGL
    // OpenGL _glue needs to prepare the drawing area for OpenGL rendering before
    // widgets are realized and before the configure event is fired.
    // TODO: find a way to make '_glue' use independent from actual renderer in use
    _glue->prepDrawingArea(_drawing_area);
#endif
    setupEvents();

    if (_xid) {
     	gtk_container_add(GTK_CONTAINER(_window), _drawing_area);
    } else {

        // A vertical box is used to allow display of the menu bar
    
        _vbox = gtk_vbox_new(FALSE, 0);
        gtk_widget_show(_vbox);
        gtk_container_add(GTK_CONTAINER(_window), _vbox);
#if defined(USE_MENUS) && !defined(GUI_HILDON)
        createMenuBar();
#endif
        gtk_box_pack_start(GTK_BOX(_vbox), _drawing_area, TRUE, TRUE, 0);
    }

    gtk_widget_realize(_window);
    gtk_widget_show(_drawing_area);
    gtk_widget_show(_window);

#if defined(RENDERER_CAIRO) || defined(RENDERER_AGG)
    // cairo needs the _drawing_area.window to prepare it ..
    // TODO: find a way to make '_glue' use independent from actual renderer in use
    _glue->prepDrawingArea(_drawing_area);
#endif

    lirc = new Lirc();
    if (lirc->init("/dev/lircd")) {
        int fd = lirc->getFileFd();
        addFDListener(fd, lirc_handler, &fd);
    } else {
        log_msg("LIRC daemon not running");
    }
    
    _renderer = _glue->createRenderHandler();
    if ( ! _renderer ) return false;
    set_render_handler(_renderer);

    return true;
}

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
    //GNASH_REPORT_FUNCTION;

    _popup_menu = GTK_MENU(gtk_menu_new());
//    GtkAccelGroup *accel_group = gtk_accel_group_new();;
//    gtk_window_add_accel_group (GTK_WINDOW (_popup_menu), accel_group);
    
#ifdef USE_MENUS
    createFileMenu(GTK_WIDGET(_popup_menu));
    createEditMenu(GTK_WIDGET(_popup_menu));
    createViewMenu(GTK_WIDGET(_popup_menu));
    createControlMenu(GTK_WIDGET(_popup_menu));
#endif
    createHelpMenu(GTK_WIDGET(_popup_menu));
    
//     GtkMenuItem *menuitem_prefs =
//  	GTK_MENU_ITEM(gtk_menu_item_new_with_label("Preferences..."));
//     gtk_menu_append(_popup_menu, GTK_WIDGET(menuitem_prefs));
//     gtk_widget_show(GTK_WIDGET(menuitem_prefs));

    if (get_sound_handler()) {
        GtkMenuItem *menuitem_sound =
            GTK_MENU_ITEM(gtk_menu_item_new_with_label("Toggle Sound"));
//         gtk_widget_add_accelerator (GTK_WIDGET(menuitem_sound), "activate", accel_group,
//                                 GDK_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
        gtk_menu_append(_popup_menu, GTK_WIDGET(menuitem_sound));
        gtk_widget_show(GTK_WIDGET(menuitem_sound));
        g_signal_connect(GTK_OBJECT(menuitem_sound), "activate",
                         G_CALLBACK(&menuitem_sound_callback), this);
    }

    GtkMenuItem *menuitem_quit =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label("Quit Gnash"));
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
//First call the old createWindow function and then set the title.
//In case there's some need to not setting the title.
    bool ret = createWindow(width, height);
    gtk_window_set_title(GTK_WINDOW(_window), title);

    if (!_xid) {
    
      // This sets the *minimum* size for the drawing area and thus will
      // also resize the window. 
      // Advantage: The window is sized correctly, no matter what other
      // widgets are visible
      // Disadvantage: The window cannot be shrinked, which is bad.   
    
      gtk_widget_set_size_request(_drawing_area, width, height);
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
GtkGui::create_pixbuf                          (const gchar     *filename)
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


bool
GtkGui::createWindow(int width, int height)
{
    //GNASH_REPORT_FUNCTION;
    
    assert(_width>0);
    assert(_height>0);
    
    _width = width;
    _height = height;
    
    _validbounds.setTo(0, 0, _width-1, _height-1);
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
    if ( _drawbounds.size() == 0 ) return; // nothing to do..
    
    for (unsigned bno=0; bno < _drawbounds.size(); bno++) {
	
        geometry::Range2d<int>& bounds = _drawbounds[bno];
        
        assert ( bounds.isFinite() );
        
        _glue->render(bounds.getMinX(), bounds.getMinY(),
                      bounds.getMaxX(), bounds.getMaxY());
	
    }
}

void
GtkGui::rerenderPixels(int xmin, int ymin, int xmax, int ymax) 
{

    // This function is called in expose events to force partly re-rendering
    // of the window. The coordinates are PIXELS.
    
    // The macro PIXELS_TO_TWIPS can't be used since the renderer might do 
    // scaling.
    
    InvalidatedRanges ranges;
    
    geometry::Range2d<int> exposed_pixels(xmin, ymin, xmax, ymax);
    
    geometry::Range2d<float> exposed_twips = 
        _renderer->pixel_to_world(exposed_pixels);	
    
    ranges.add(exposed_twips);
    setInvalidatedRegions(ranges);
    
    renderBuffer();   
    
}

void
GtkGui::setInvalidatedRegions(const InvalidatedRanges& ranges)
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
    
    _drawbounds.clear();
    
    for (unsigned rno=0; rno<ranges.size(); rno++) {
        geometry::Range2d<int> bounds = Intersection(
            _renderer->world_to_pixel(ranges.getRange(rno)),
            _validbounds);
        
        // it may happen that a particular range is out of the screen, which 
        // will lead to bounds==null. 
        if (bounds.isNull()) continue;
        
        assert(bounds.isFinite()); 
        
        _drawbounds.push_back(bounds);
        
    }
    
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
GtkGui::quit()
{
    gtk_main_quit();
}

void
GtkGui::setInterval(unsigned int interval)
{
    _interval = interval;
    
    // From http://www.idt.mdh.se/kurser/cd5040/ht02/gtk/glib/glib-the-main-event-loop.html#G-TIMEOUT-ADD-FULL
    //
    // Note that timeout functions may be delayed, due to the
    // processing of other event sources. Thus they should not be
    // relied on for precise timing. After each call to the timeout
    // function, the time of the next timeout is recalculated based
    // on the current time and the given interval (it does not try to
    // 'catch up' time lost in delays).
    //
    // TODO: this is not what we need here, we want instead to 'catch up' !!
    //
    g_timeout_add_full (G_PRIORITY_LOW, interval, (GSourceFunc)advance_movie,
                        this, NULL);
}

bool
GtkGui::run()
{
    //GNASH_REPORT_FUNCTION;
    gtk_main();
    return true;
}

/// This method is called when the "OK" button is clicked in the open file
/// dialog. For GTK <= 2.4.0, this is a callback called by GTK itself.
void GtkGui::open_file (GtkWidget *widget, gpointer /* user_data */)
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
GtkGui::menuitem_openfile_callback(GtkMenuItem* /*menuitem*/, gpointer data)
{
    GtkWidget* dialog;
    GtkGui* gui = static_cast<GtkGui*>(data);

#if GTK_CHECK_VERSION(2,4,0)
    dialog = gtk_file_chooser_dialog_new ("Open file",
                                          NULL,
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                          NULL);
    
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
        open_file(dialog, gui);
    }
    
    gtk_widget_destroy (dialog);
#else
    dialog = gtk_file_selection_new ("Open file");

    GtkFileSelection* selector = GTK_FILE_SELECTION(dialog);

    g_signal_connect (selector->ok_button, "clicked", G_CALLBACK (open_file),
                      gui);

    g_signal_connect_swapped (selector->ok_button, "clicked", 
                              G_CALLBACK (gtk_widget_destroy), dialog);

    g_signal_connect_swapped (selector->cancel_button, "clicked",
                              G_CALLBACK (gtk_widget_destroy), dialog); 
   
    gtk_widget_show (dialog);
#endif // GTK_CHECK_VERSION(2,4,0)
}

/// \brief Show gnash preferences window
void
GtkGui::menuitem_preferences_callback(GtkMenuItem* /*menuitem*/, gpointer /*data*/)
{
//    GNASH_REPORT_FUNCTION;
    
    RcInitFile& rcfile = RcInitFile::getDefaultInstance();

    GtkWidget *window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window1), _("Gnash preferences"));

    GtkWidget *notebook1 = gtk_notebook_new ();
    gtk_widget_show (notebook1);
    gtk_container_add (GTK_CONTAINER (window1), notebook1);

    GtkWidget *frame1 = gtk_frame_new (NULL);
    gtk_widget_show (frame1);
    gtk_container_add (GTK_CONTAINER (notebook1), frame1);
    gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_NONE);
    
    GtkWidget *alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment1);
    gtk_container_add (GTK_CONTAINER (frame1), alignment1);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment1), 0, 0, 12, 0);
    
    GtkWidget *table1 = gtk_table_new (6, 2, FALSE);
    gtk_widget_show (table1);
    gtk_container_add (GTK_CONTAINER (alignment1), table1);
    
    GtkWidget *label5 = gtk_label_new (_("Verbosity"));
    gtk_widget_show (label5);
    gtk_table_attach (GTK_TABLE (table1), label5, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label5), 0, 0.5);

    GtkWidget *hscale1 = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (rcfile.verbosityLevel(), 0, 10, 1, 0, 0)));
    gtk_widget_show (hscale1);
    gtk_table_attach (GTK_TABLE (table1), hscale1, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_scale_set_digits (GTK_SCALE (hscale1), 0);
    gtk_range_set_update_policy (GTK_RANGE (hscale1), GTK_UPDATE_DISCONTINUOUS);
    
    GtkWidget *label6 = gtk_label_new (_("Log to file"));
    gtk_widget_show (label6);
    gtk_table_attach (GTK_TABLE (table1), label6, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label6), 0, 0.5);
    
    GtkWidget *checkbutton1 = gtk_check_button_new_with_mnemonic ("");
    gtk_widget_show (checkbutton1);
    gtk_table_attach (GTK_TABLE (table1), checkbutton1, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    
    if ( rcfile.useWriteLog() == true  )
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton1), TRUE);
    else 
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton1), FALSE);
    
    GtkWidget *label7 = gtk_label_new (_("Log File name"));
    gtk_widget_show (label7);
    gtk_table_attach (GTK_TABLE (table1), label7, 0, 1, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label7), 0, 0.5);
    
    GtkWidget *logfilenameentry = gtk_entry_new ();
    gtk_widget_show (logfilenameentry);
    gtk_table_attach (GTK_TABLE (table1), logfilenameentry, 1, 2, 2, 3,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    
    if (rcfile.useWriteLog() == true ) {
        log_msg (_("Debug log filename: %s"), rcfile.getDebugLog().c_str());
        gtk_entry_set_text( (GtkEntry*) logfilenameentry, rcfile.getDebugLog().c_str()); 
        gtk_widget_set_sensitive(logfilenameentry,TRUE);
    } else {
        gtk_widget_set_sensitive(logfilenameentry,FALSE);
    }
    
    GtkWidget *label8 = gtk_label_new (_("Parser output"));
    gtk_widget_show (label8);
    gtk_table_attach (GTK_TABLE (table1), label8, 0, 1, 3, 4,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label8), 0, 0.5);
    
    GtkWidget *parseroutputcheckbutton2 = gtk_check_button_new_with_mnemonic ("");
    gtk_widget_show (parseroutputcheckbutton2);
    gtk_table_attach (GTK_TABLE (table1), parseroutputcheckbutton2, 1, 2, 3, 4,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    
    if ( rcfile.useParserDump() == true  )
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (parseroutputcheckbutton2), TRUE);
    else 
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (parseroutputcheckbutton2), FALSE);
    
    GtkWidget *label9 = gtk_label_new (_("Debug ActionScript"));
    gtk_widget_show (label9);
    gtk_table_attach (GTK_TABLE (table1), label9, 0, 1, 4, 5,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label9), 0, 0.5);
    
    GtkWidget *debugActionScriptcheckbutton3 = gtk_check_button_new_with_mnemonic ("");
    gtk_widget_show (debugActionScriptcheckbutton3);
    gtk_table_attach (GTK_TABLE (table1), debugActionScriptcheckbutton3, 1, 2, 4, 5,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    
    if (rcfile.useActionDump() == true) {
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (debugActionScriptcheckbutton3), TRUE);
    } else {
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (debugActionScriptcheckbutton3), FALSE);
    }

    GtkWidget *label10 = gtk_label_new (_("Debugger"));
    gtk_widget_show (label10);
    gtk_table_attach (GTK_TABLE (table1), label10, 0, 1, 5, 6,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label10), 0, 0.5);

    GtkWidget *debuggercheckbutton4 = gtk_check_button_new_with_mnemonic ("");
    gtk_widget_show (debuggercheckbutton4);
    gtk_table_attach (GTK_TABLE (table1), debuggercheckbutton4, 1, 2, 5, 6,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    
    if (rcfile.useDebugger() == true) {
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (debuggercheckbutton4), TRUE);
    } else {
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (debuggercheckbutton4), FALSE);
    }
    
    GtkWidget *label4 = gtk_label_new (_("<b>Logging preferences</b>"));
    gtk_widget_show (label4);
    gtk_frame_set_label_widget (GTK_FRAME (frame1), label4);
    gtk_label_set_use_markup (GTK_LABEL (label4), TRUE);
    
    GtkWidget *label1 = gtk_label_new (_("Logging"));
    gtk_widget_show (label1);
    gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), label1);
    
    GtkWidget *frame2 = gtk_frame_new (NULL);
    gtk_widget_show (frame2);
    gtk_container_add (GTK_CONTAINER (notebook1), frame2);
    gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_NONE);
    
    GtkWidget *alignment2 = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment2);
    gtk_container_add (GTK_CONTAINER (frame2), alignment2);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment2), 0, 0, 12, 0);
    
    GtkWidget *table4 = gtk_table_new (4, 2, FALSE);
    gtk_widget_show (table4);
    gtk_container_add (GTK_CONTAINER (alignment2), table4);
    
    GtkWidget *label13 = gtk_label_new (_("Allow remote access from: "));
    gtk_widget_show (label13);
    gtk_table_attach (GTK_TABLE (table4), label13, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label13), 0, 0.5);
    
    GtkWidget *label15 = gtk_label_new (_("Whitelist"));
    gtk_widget_show (label15);
    gtk_table_attach (GTK_TABLE (table4), label15, 0, 1, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label15), 0, 0.5);
    
    GtkWidget *label16 = gtk_label_new (_("Blacklist"));
    gtk_widget_show (label16);
    gtk_table_attach (GTK_TABLE (table4), label16, 0, 1, 3, 4,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label16), 0, 0.5);
    
    GtkWidget *whitelistcomboboxentry1 = gtk_combo_box_entry_new_text ();
    gtk_widget_show (whitelistcomboboxentry1);
    gtk_table_attach (GTK_TABLE (table4), whitelistcomboboxentry1, 1, 2, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);
    
    GtkWidget *blacklistcomboboxentry2 = gtk_combo_box_entry_new_text ();
    gtk_widget_show (blacklistcomboboxentry2);
    gtk_table_attach (GTK_TABLE (table4), blacklistcomboboxentry2, 1, 2, 3, 4,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);
    
    GtkWidget *localhostcheckbutton8 = gtk_check_button_new_with_mnemonic (_("local host only"));
    gtk_widget_show (localhostcheckbutton8);
    gtk_table_attach (GTK_TABLE (table4), localhostcheckbutton8, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    
    if ( rcfile.useLocalHost() == true ) {
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (localhostcheckbutton8), TRUE);
    } else {
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (localhostcheckbutton8), FALSE);
    }
    
    GtkWidget *localdomaincheckbutton9 = gtk_check_button_new_with_mnemonic (_("local domain only"));
    gtk_widget_show (localdomaincheckbutton9);
    gtk_table_attach (GTK_TABLE (table4), localdomaincheckbutton9, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    
    if ( rcfile.useLocalDomain() == true ) {
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (localdomaincheckbutton9), TRUE);
    } else {
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (localdomaincheckbutton9), FALSE);
    }
    
    GtkWidget *label11 = gtk_label_new (_("<b>Security preferences</b>"));
    gtk_widget_show (label11);
    gtk_frame_set_label_widget (GTK_FRAME (frame2), label11);
    gtk_label_set_use_markup (GTK_LABEL (label11), TRUE);
    
    GtkWidget *label2 = gtk_label_new (_("Security"));
    gtk_widget_show (label2);
    gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), label2);
    
    GtkWidget *frame3 = gtk_frame_new (NULL);
    gtk_widget_show (frame3);
    gtk_container_add (GTK_CONTAINER (notebook1), frame3);
    gtk_frame_set_shadow_type (GTK_FRAME (frame3), GTK_SHADOW_NONE);
    
    GtkWidget *alignment3 = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment3);
    gtk_container_add (GTK_CONTAINER (frame3), alignment3);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment3), 0, 0, 12, 0);
    
    GtkWidget *table5 = gtk_table_new (3, 2, FALSE);
    gtk_widget_show (table5);
    gtk_container_add (GTK_CONTAINER (alignment3), table5);

    GtkWidget *label17 = gtk_label_new (_("Enable sound"));
    gtk_widget_show (label17);
    gtk_table_attach (GTK_TABLE (table5), label17, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label17), 0, 0.5);
    
    GtkWidget *checkbutton7 = gtk_check_button_new_with_mnemonic ("");
    gtk_widget_show (checkbutton7);
    gtk_table_attach (GTK_TABLE (table5), checkbutton7, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    if (rcfile.useSound() == true) {
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton7), TRUE);
    } else {
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton7), FALSE);
    }
    
    GtkWidget *label12 = gtk_label_new (_("<b>Sound preferences</b>"));
    gtk_widget_show (label12);
    gtk_frame_set_label_widget (GTK_FRAME (frame3), label12);
    gtk_label_set_use_markup (GTK_LABEL (label12), TRUE);
    
    GtkWidget *label3 = gtk_label_new (_("Sound"));
    gtk_widget_show (label3);
    gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 2), label3);

    gtk_widget_show (window1);    
}

void 
GtkGui::setCursor(gnash_cursor_type newcursor)
{
  //GNASH_REPORT_FUNCTION;

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

  gdk_window_set_cursor (_window->window, gdkcursor);
  
  if (gdkcursor) {
    gdk_cursor_unref(gdkcursor);
  }
}


bool
GtkGui::setupEvents()
{
  //GNASH_REPORT_FUNCTION;

  g_signal_connect(G_OBJECT(_window), "delete_event",
                   G_CALLBACK(delete_event), this);
  g_signal_connect(G_OBJECT(_window), "key_press_event",
                   G_CALLBACK(key_press_event), this);
  g_signal_connect(G_OBJECT(_window), "key_release_event",
                   G_CALLBACK(key_release_event), this);

   gtk_widget_add_events(_drawing_area, GDK_EXPOSURE_MASK
                        | GDK_BUTTON_PRESS_MASK
                        | GDK_BUTTON_RELEASE_MASK
                        | GDK_KEY_RELEASE_MASK
                        | GDK_KEY_PRESS_MASK        
                        | GDK_POINTER_MOTION_MASK);
  
   g_signal_connect_swapped(G_OBJECT(_drawing_area),
                            "button_press_event",
                            G_CALLBACK(popup_handler),
                            GTK_OBJECT(_popup_menu));
  
  g_signal_connect(G_OBJECT(_drawing_area), "button_press_event",
                   G_CALLBACK(button_press_event), this);
  g_signal_connect(G_OBJECT(_drawing_area), "button_release_event",
                   G_CALLBACK(button_release_event), this);
  g_signal_connect(G_OBJECT(_drawing_area), "motion_notify_event",
                   G_CALLBACK(motion_notify_event), this);
  
  g_signal_connect_after(G_OBJECT (_drawing_area), "realize",
                         G_CALLBACK (realize_event), NULL);
  g_signal_connect(G_OBJECT (_drawing_area), "configure_event",
                   G_CALLBACK (configure_event), this);
   g_signal_connect(G_OBJECT (_drawing_area), "expose_event",
                    G_CALLBACK (expose_event), this);
//   g_signal_connect(G_OBJECT (_drawing_area), "unrealize",
//                           G_CALLBACK (unrealize_event), NULL);

  return true;
}

/// \brief show info about gnash
void
GtkGui::menuitem_about_callback(GtkMenuItem* /*menuitem*/, gpointer /*data*/)
{
//    GNASH_REPORT_FUNCTION;
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
        NULL 
    };

    string comments = "Gnash is the GNU Flash movie player based on GameSWF.";

    comments += "\nRenderer: ";
    comments += RENDERER_CONFIG;
    comments += "   GUI: ";
    comments += "GTK2"; // gtk of course!
    comments += "   Media: ";
    comments += MEDIA_CONFIG;
    comments += ".";

    gtk_about_dialog_set_url_hook(NULL, NULL, NULL);
    GdkPixbuf *logo_pixbuf = gdk_pixbuf_new_from_file("GnashG.png", NULL);
    //GtkWidget *about = (GtkWidget*) g_object_new (GTK_TYPE_ABOUT_DIALOG,
    gtk_show_about_dialog (
    		   NULL,
                   "name", "GNASH flash movie player", 
                   "version", VERSION,
                   "copyright", "Copyright (C) 2005, 2006, 2007 The Free Software Foundation",
	           "comments", comments.c_str(),
                   "authors", authors,
                   "documenters", documentors,
		   "artists", artists,
//                   "translator-credits", "translator-credits",
                   "logo", logo_pixbuf,
		   "license", 
		   "This program is free software; you can redistribute it and/or modify\nit under the terms of the GNU General Public License as published by\nthe Free Software Foundation; either version 3 of the License, or\n(at your option) any later version.\n\nThis program is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\nGNU General Public License for more details.\nYou should have received a copy of the GNU General Public License\nalong with this program; if not, write to the Free Software\nFoundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA",
		   "website", "http://www.gnu.org/software/gnash/",
                   NULL);
}

//Movie Properties dialogue
void
GtkGui::menuitem_movieinfo_callback(GtkMenuItem* /*menuitem*/, gpointer data)
{
//    GNASH_REPORT_FUNCTION;

    Gui* gui = static_cast<Gui*>(data);
    assert(gui);

    GtkWidget* label;

    GtkWidget* window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window1), _("Movie Properties"));

    GtkWidget *main_vbox = gtk_vbox_new(FALSE, 2);
    gtk_container_add (GTK_CONTAINER (window1), main_vbox);

    GtkWidget *frame1 = gtk_frame_new(_("Movie Properties"));
    gtk_box_pack_start (GTK_BOX (main_vbox), frame1, TRUE, TRUE, 0);

    GtkWidget *vbox1 = gtk_vbox_new (FALSE, 3);
    gtk_container_add (GTK_CONTAINER (frame1), vbox1);

    GtkWidget *vbox2 = gtk_vbox_new (FALSE, 2);
    gtk_box_pack_start (GTK_BOX (vbox1), vbox2, FALSE, FALSE, 0);

    GtkWidget *label_vbox2 = gtk_label_new(_("VM Properties"));
    gtk_box_pack_start (GTK_BOX (vbox2), label_vbox2, FALSE, FALSE, 0);

    GtkWidget *table1 = gtk_table_new(4, 2, FALSE);
    gtk_box_pack_start (GTK_BOX (vbox2), table1, FALSE, FALSE, 0);


    gtk_box_pack_start (
	GTK_BOX (vbox1), gtk_hseparator_new (), FALSE, FALSE, 0);

    GtkWidget *vbox3 = gtk_vbox_new (FALSE, 3);
    gtk_box_pack_start (
	GTK_BOX (vbox1), vbox3, TRUE, TRUE, 0);

    GtkWidget *scrollwindow1 = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollwindow1),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (
	 GTK_BOX (vbox3), scrollwindow1, TRUE, TRUE, 0);

    std::auto_ptr<InfoTree> infoptr = gui->getMovieInfo();

    if ( ! infoptr.get() )
    {
            label = gtk_label_new (_("VM not initialized yet"));
            gtk_widget_show (label);
            gtk_table_attach_defaults (GTK_TABLE (table1), label, 0, 1, 0, 1);
            return;
    }

    else {

            // Table display
            // This left in while tree information isn't selectable

            InfoTree& info = *infoptr;

            size_t size = info.size();

            for (InfoTree::leaf_iterator i=info.begin_leaf(), e=info.end_leaf();
                 i!=e; ++i)
            {
                StringPair& p = *i;
                guint up = size;
                guint bot = size-1;

                GtkWidget *label_table11 = gtk_label_new(p.first.c_str());
                gtk_table_attach (GTK_TABLE (table1), label_table11, 0, 1, bot, up,
                             (GtkAttachOptions) (GTK_FILL),
                             (GtkAttachOptions) (0), 0, 0);
                gtk_misc_set_alignment (GTK_MISC (label_table11), 0.0, 1.0);
                gtk_widget_show (label_table11);

                GtkWidget *label_table12 = gtk_label_new(p.second.c_str());
                gtk_table_attach (GTK_TABLE (table1), label_table12, 1, 2, bot, up,
                                 (GtkAttachOptions) (GTK_FILL),
                                 (GtkAttachOptions) (0), 0, 0);
                gtk_label_set_selectable (GTK_LABEL (label_table12), TRUE);
                gtk_widget_show (label_table12);

                --size;
            }

            // Tree display
            // Should replace table display when proper
            // InfoTrees are available 

            enum
            {
                NODENAME_COLUMN = 0,
                STRING1_COLUMN,
                STRING2_COLUMN,
                COMMENT_COLUMN,
                NUM_COLUMNS
            };

            GtkTreeModel *model = makeTreeModel(infoptr);

            GtkWidget *treeview = gtk_tree_view_new_with_model (model);

            g_object_unref (model);

            gint col_offset;
            GtkCellRenderer *renderer;
            GtkTreeViewColumn *column;

            //Add columns:
            //First column:
            renderer = gtk_cell_renderer_text_new ();
            g_object_set (renderer, "xalign", 0.0, NULL);
            col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(treeview),
	       						       -1, _("Depth"),
							       renderer, "text",
							       NODENAME_COLUMN,
							       NULL);
            column = gtk_tree_view_get_column (GTK_TREE_VIEW(treeview), col_offset - 1);

            //Second column:

            renderer = gtk_cell_renderer_text_new ();
            g_object_set (renderer, "xalign", 0.0, NULL);
            col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(treeview),
							       -1, _("Variable"),
							       renderer, "text",
							       STRING1_COLUMN,
							       NULL);
            column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);

            //Third column:

            renderer = gtk_cell_renderer_text_new ();
            g_object_set (renderer, "xalign", 0.0, NULL);
            col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(treeview),
							       -1, _("Value"),
							       renderer, "text",
							       STRING2_COLUMN,
							       NULL);
            column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);

            //Add tree to scrollwindow.
            gtk_container_add (GTK_CONTAINER (scrollwindow1), treeview);

            }

    GtkWidget *bbox1 = gtk_hbutton_box_new ();
    gtk_box_pack_start (
	GTK_BOX (main_vbox), bbox1, FALSE, FALSE, 0);


     GtkWidget *button_ok = gtk_button_new_from_stock (GTK_STOCK_OK);
     gtk_box_pack_end (GTK_BOX (bbox1), button_ok, FALSE, FALSE, 0);
     g_signal_connect_swapped (button_ok, "clicked",
             G_CALLBACK(gtk_widget_destroy), window1);

     gtk_widget_show_all (window1);

}


GtkTreeModel*
GtkGui::makeTreeModel (std::auto_ptr<InfoTree> treepointer)

{

     InfoTree& info = *treepointer;

     enum
     {
       NODENAME_COLUMN = 0,
       STRING1_COLUMN,
       STRING2_COLUMN,
       NUM_COLUMNS
     };
    
     GtkTreeStore *model = gtk_tree_store_new (NUM_COLUMNS,
                          G_TYPE_STRING,
                          G_TYPE_STRING,
                          G_TYPE_STRING);
    
     GtkTreeIter iter;
     GtkTreeIter child_iter;
     GtkTreeIter parent_iter;

     int depth = 0;                    // Depth within the gtk tree.    

     for (InfoTree::iterator i=info.begin_leaf(), e=info.end_leaf(); i!=e; ++i)
     {
          StringPair& p = *i;

          int infotreedepth = info.depth(i);  
          char buf[8];
          sprintf(buf, "%d", infotreedepth);
          buf[7] = '\0';                     

          if (info.depth(i) > depth) {          // Align Gtk tree depth.
               depth++;                   
               iter=child_iter;                  
          }

          if (info.depth(i) < depth) {        // Align Gtk tree depth.
               depth = info.depth(i);
               gtk_tree_model_iter_parent (GTK_TREE_MODEL(model), &parent_iter, &iter);  // Get parent iter.
               iter = parent_iter;
          }

          //Read in data from present node
          if (depth == 0) gtk_tree_store_append (model, &child_iter, NULL);
          else gtk_tree_store_append (model, &child_iter, &iter);

          gtk_tree_store_set (model, &child_iter,
                            NODENAME_COLUMN, buf,   //infotree
                            STRING1_COLUMN, p.first.c_str(),     //infotree
    		            STRING2_COLUMN, p.second.c_str(),     //infotree
			    -1);

     }

     return GTK_TREE_MODEL(model);

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
    gui->menu_toggle_sound();
}


/// \brief restart the movie from the beginning
void
GtkGui::menuitem_restart_callback(GtkMenuItem* /*menuitem*/, gpointer data)
{
    //GNASH_REPORT_FUNCTION;
    Gui* gui = static_cast<Gui*>(data);
    gui->menu_restart();
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
    gui->menu_play();
}

/// \brief toggle that's playing or paused.
void
GtkGui::menuitem_pause_callback(GtkMenuItem* /*menuitem*/, gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    Gui* gui = static_cast<Gui*>(data);
    gui->menu_pause();
}

/// \brief stop the movie that's playing.
void
GtkGui::menuitem_stop_callback(GtkMenuItem* /*menuitem*/, gpointer data)
{
    GNASH_REPORT_FUNCTION;
    Gui* gui = static_cast<Gui*>(data);
    gui->menu_stop();
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
    gui->menu_refresh_view();
}

//
// Event handlers
//

gboolean
GtkGui::expose_event(GtkWidget *const /*widget*/,
             GdkEventExpose *const event,
             const gpointer data)
{
//	GNASH_REPORT_FUNCTION;

	GtkGui* gui = static_cast<GtkGui*>(data);


	int xmin = event->area.x, xmax = event->area.x + event->area.width,
	    ymin = event->area.y, ymax = event->area.y + event->area.height;
          
  gui->rerenderPixels(xmin, ymin, xmax, ymax);

	return TRUE;
}

// These event handlers are never used.
#if 0
gboolean
GtkGui::unrealize_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    GNASH_REPORT_FUNCTION;
    
    return TRUE;
}

#endif

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
    int modifier = gnash::key::MOD_NONE;

    if (state & GDK_SHIFT_MASK) {
      modifier = modifier | gnash::key::MOD_SHIFT;
    }
    if (state & GDK_CONTROL_MASK) {
      modifier = modifier | gnash::key::MOD_CONTROL;
    }
    if (state & GDK_MOD1_MASK) {
      modifier = modifier | gnash::key::MOD_ALT;
    }

    return modifier;
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
    //GNASH_REPORT_FUNCTION;
    Gui *obj = static_cast<Gui *>(data);

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

    float xscale = obj->getXScale();
    float yscale = obj->getYScale();
    obj->notify_mouse_moved(int(event->x / xscale), int(event->y / yscale));
    return true;
}

// Create a File menu that can be used from the menu bar or the popup.
void
GtkGui::createFileMenu(GtkWidget *obj)
{
//    GNASH_REPORT_FUNCTION;
    GtkWidget *menuitem = gtk_menu_item_new_with_label (_("File"));
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
    
    GtkWidget *menuitem = gtk_menu_item_new_with_label (_("Edit"));
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
    GtkWidget *menuitem = gtk_menu_item_new_with_label (_("Help"));
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
    GtkWidget *menuitem = gtk_menu_item_new_with_label (_("View"));
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
}

// Create a Control menu that can be used from the menu bar or the popup.
void
GtkGui::createControlMenu(GtkWidget *obj)
{
//    GNASH_REPORT_FUNCTION;

// Movie Control Menu
    GtkWidget *menuitem_control =
	gtk_menu_item_new_with_label (_("Movie Control"));
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

    GtkWidget *separator1 = gtk_separator_menu_item_new ();
    gtk_widget_show (separator1);
    gtk_container_add (GTK_CONTAINER (menu), separator1);

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

bool
lirc_handler(void*, int, void* data)
{ 
    GNASH_REPORT_FUNCTION;
//    int* fd = static_cast<int*>(data);
    
    // want to remove this handler. You may want to close fd.
    log_msg("%s\n", lirc->getButton());
  
    // Want to keep this handler
    return true;
}

} // end of namespace gnash

