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

#ifndef GNASH_GTKSUP_H
#define GNASH_GTKSUP_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <string>
#include <utility>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "gui.h"
#include "gtk_glue.h"

namespace gnash {

class GtkGui : public Gui
{
public:

    GtkGui(unsigned long xid, float scale, bool loop, RunResources& r);
    
    virtual ~GtkGui();
    
    /// GUI interface implementation

    virtual bool init(int argc, char **argv[]);
    virtual bool createWindow(int width, int height);
    virtual bool createWindow(const char *title, int width, int height,
                              int xPosition = 0, int yPosition = 0);
    virtual void resizeWindow(int width, int height);

    virtual bool run();

    virtual void quitUI();

    virtual bool createMenu();

    virtual bool createMenuAlt(); //an alternative popup menu
    
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
    virtual double getPixelAspectRatio() const;
    virtual std::pair<int, int> screenResolution() const;
    virtual double getScreenDPI() const;

    virtual void setClipboard(const std::string& copy);

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
    void createQualityMenu(GtkWidget *obj);
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

    bool checkX11Extension(const std::string& ext);

    virtual bool visible() { return _visible; }

    void setVisible(bool visible) { _visible = visible; }

private:

    GtkWidget* _window;
    GtkWidget* _resumeButton;
    
    // A window only for rendering the plugin as fullscreen.
    GtkWidget* _overlay;
    
    // The area rendered into by Gnash
    GtkWidget* _canvas;
    bool _visible;

    GtkMenu* _popup_menu;
    GtkMenu* _popup_menu_alt;
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
    GtkTreeModel* makeTreeModel(std::auto_ptr<movie_root::InfoTree> tree);
#endif

    void stopHook();
    void playHook();

    guint _advanceSourceTimer;

    void startAdvanceTimer();

    void stopAdvanceTimer();
};

} // namespace gnash

#endif
