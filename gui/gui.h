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

#ifndef _GUI_H_
#define _GUI_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"
#include "gnash.h"  // for rect

#include <string>

//#include "gnash.h"

// Forward declarations
namespace gnash
{
	class render_handler;
}

namespace gnash
{

/// Parent class from which all GUI implementations will depend.
class DSOEXPORT Gui {
public:
    /// Default constructor. Initialises members to safe defaults.
    Gui();

    /** \brief
     * Expanded constructor for more control over member values.
     *
     * @param xid The X11 Window ID to attach to. If this is argument is zero,
     * a new window is created.
     *
     * @param scale The scale used to resize the window size, which has been
     * established by extracting information from the SWF file.
     * 
     * @param loop Defines whether or not the movie should be played once or
     * looped indefinitely.
     *
     * @param depth Colour depth to be used in the client area of our window.
     */
    Gui(unsigned long xid, float scale, bool loop, unsigned int depth);

    virtual ~Gui();
    
    /** \brief
     * Initialise the gui and the associated renderer.
     * 
     * @param argc The commandline argument count.
     * @param argv The commandline arguments.
     */
    virtual bool init(int argc, char **argv[]) = 0;

    /// Set main loop delay in milliseconds. 
    virtual void setInterval(unsigned int interval) = 0;

    /// Set the time in milliseconds after which the programme should exit.
    virtual void setTimeout(unsigned int timeout) = 0;

    /** \brief
     * Create and display our window.
     *
     * @param title The window title.
     * @param width The desired window width in pixels.
     * @param height The desired window height in pixels.
     */   
    virtual bool createWindow(const char* title, int width, int height) = 0;

    /// Start main rendering loop.
    virtual bool run() = 0;

    /// Create a menu and attach it to our window.
    virtual bool createMenu() = 0;

    /// Register event handlers.
    virtual bool setupEvents() = 0;
    
  	/// Defines the region on the stage that needs to be redrawn/updated.
    //
    /// Changes outside that region are unnecessary but not disallowed.
    /// Coordinates are in TWIPS!
    ///
    /// Note this information is given to the GUI and not directly to the 
    /// renderer because both of them need to support this feature for 
    /// correct results. It is up to the GUI to forward this information to
    /// the renderer.
    ///
    virtual void set_invalidated_region(const rect& /*bounds*/) {      
      // does not need to be implemented (optional feature),
      // but still needs to be available.
      // Why "rect" (floats)? Because the gui does not really
      // know about the
      // scale the renderer currently uses... 
    } 
    
    
    /// Asks the GUI handler if the next frame should be redrawn completely. 
    /// For example, when the contents of the player window have been destroyed,
    /// then want_draw() should return true so that set_invalidated_region() is
    /// called with the coordinates of the complete screen. 
    virtual bool want_redraw() {
      return false;
    }

    /// \brief
    /// Render the current buffer. For OpenGL, this means that the buffer is
    /// swapped.
    virtual void renderBuffer() = 0;

    /// @return The value to which the movie width should be scaled.
    float getXScale()                { return _xscale; }

    /// @return The value to which the movie height shold be scaled.
    float getYScale()                { return _yscale; }

    /// @return Whether or not the movie should be looped indefinitely.
    bool loops()                     { return _loop; }

    /** @name Menu callbacks
     *  These callbacks will be called when a menu item is clicked.
     *  @{
     */
    static void menu_restart();
    static void menu_quit();
    static void menu_play();
    static void menu_pause();
    static void menu_stop();
    static void menu_step_forward();
    static void menu_step_backward();
    static void menu_jump_forward();
    static void menu_jump_backward();
    /// @}
 
    /// Mouse notification callback to be called when the mouse is moved.
    //
    /// @param x The mouse coordinate X component in pixels.
    /// @param y The mouse coordinate Y component in pixels.
    static void notify_mouse_moved(int x, int y);

    /// Mouse notification callback to be called when the mouse is clicked.
    //
    /// @param mouse_pressed Determines whether the mouse button is being
    ///                      pressed (true) or being released (false)
    /// @param mask A binary representation of the buttons currently pressed.
    static void notify_mouse_clicked(bool mouse_pressed, int mask);

    /// \brief
    /// Advances the movie to the next frame. This is to take place after the
    /// interval specified in the call to setInterval().
    static bool advance_movie(Gui* gui);

    /// Resize the client area view and the window accordingly.
    /// @param width  The desired width in pixels.
    /// @param height The desired height in pixels.
    void resize_view(int width, int height);

protected:
    /// Determines if playback should restart after the movie ends.
    bool            _loop;
    /// The X Window ID to attach to. If zero, we create a new window.
    unsigned long   _xid;
    /// Desired window width.
    int             _width;
    /// Desired window height.
    int             _height;
    /// The window width scale.
    float           _xscale;
    /// The window height scale.
    float           _yscale;
    /// Desired colour depth in bits.
    int             _depth;
    /// Main loop interval: the time between successive advance_movie calls.
    unsigned int    _interval;
    /// The handler which is called to update the client area of our window.
    render_handler* _renderer;
    /// Invalidated bounds of previous frame
    rect            _last_invalidated_bounds;
};

 
} // end of gnash namespace

// end of _GUI_H_
#endif
