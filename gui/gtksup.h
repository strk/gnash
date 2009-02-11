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

#ifndef GNASH_GTKSUP_H
#define GNASH_GTKSUP_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "gnash.h"
#include "gtk_glue.h"

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#ifdef GUI_HILDON
extern "C" {
# include <hildon/hildon.h>
}
#endif

#ifdef USE_ALP
# include <alp/bundlemgr.h>
#endif

namespace gnash
{


class GtkGui : public Gui
{
public:

    /// For the Gtk GUI FD listener, whatever that might be for.
    typedef bool (*callback_t)(void*, int, void *data);

    GtkGui(unsigned long xid, float scale, bool loop, unsigned int depth);
    
    virtual ~GtkGui();
    
    /// GUI interface implementation

    virtual bool init(int argc, char **argv[]);
    virtual bool createWindow(int width, int height);
    virtual bool createWindow(const char *title, int width, int height);

    virtual bool run();

    virtual void quit();

    virtual bool createMenu();
    
    /// Set up callbacks for key, mouse and other GTK events.
    //
    /// Must be called after the drawing area has been added to
    /// a top level window, as it calls setupWindowEvents() to
    /// add key event callbacks to the top level window.
    virtual bool setupEvents();
    virtual void beforeRendering();
    virtual void renderBuffer();
    virtual void setInterval(unsigned int interval);
    virtual void setTimeout(unsigned int timeout);
    
    virtual void setFullscreen();
    virtual void unsetFullscreen();
    
    virtual void hideMenu();

    /// For System.capabilities information.
    virtual double getPixelAspectRatio();
    virtual int getScreenResX();
    virtual int getScreenResY();
    virtual double getScreenDPI();
    
    /// Add a listener with default priority that listens for IN and HUP
    /// events on a file descriptor.
    //
    /// @param fd The file descriptor to poll.
    /// @param callback A pointer to a callback function with the following
    ///                 signature:
    ///        bool func(void*, int, void* data)
    ///        The first and second arguments should be ignored.
    ///        The last argument is a user-specified pointer. The
    ///        callback should return false if the listener is to be removed.
    /// @param data A pointer to a user-defined data structure.
    /// @return true on success, false on failure.
    bool addFDListener(int fd, callback_t callback, void* data);

    /// Grab focus so to receive all key events
    //
    /// Might become a virtual in the base class
    ///
    void grabFocus();

    /// Create a menu bar for the application, attach to our window. 
    //  This should only appear in the standalone player.
    bool createMenuBar();
    void createFileMenu(GtkWidget *obj);
    void createEditMenu(GtkWidget *obj);
    void createViewMenu(GtkWidget *obj);
    void createHelpMenu(GtkWidget *obj);
    void createControlMenu(GtkWidget *obj);
    
    // Display a properties dialogue
    void showPropertiesDialog();
    
    // Display a preferences dialogue
    void showPreferencesDialog();
    
    // Display an About dialogue
    void showAboutDialog();

    void expose(const GdkRegion* region);

    void setInvalidatedRegions(const InvalidatedRanges& ranges);

    bool want_multiple_regions() { return true; }

    virtual void setCursor(gnash_cursor_type newcursor);
    
    virtual bool showMouse(bool show);

    virtual void showMenu(bool show);

    virtual void error(const std::string& msg);

    GtkGlue& rendererGlue() { return *_glue; }

private:

#ifdef GUI_HILDON
    HildonProgram *_hildon_program;
#endif

    GtkWidget* _window;
    GtkWidget* _resumeButton;
    
    // A window only for rendering the plugin as fullscreen.
    GtkWidget* _overlay;
    
    // The area rendered into by Gnash
    GtkWidget* _drawingArea;    

    GtkMenu* _popup_menu;
    GtkWidget* _menubar;
    GtkWidget* _vbox;

    /// Add key press events to the toplevel window.
    //
    /// The plugin fullscreen creates a new top level
    /// window, so this function must be called every time
    /// the drawing area is reparented.
    void setupWindowEvents();

#ifdef USE_SWFTREE
    // Create a tree model for displaying movie info
    GtkTreeModel* makeTreeModel (std::auto_ptr<InfoTree> treepointer);
#endif

    std::auto_ptr<GtkGlue> _glue;

    void stopHook();
    void playHook();

    guint _advanceSourceTimer;
};

} // namespace gnash

#endif
