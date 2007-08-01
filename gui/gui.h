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
//

#ifndef _GUI_H_
#define _GUI_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"
#include "rect.h"  // for composition
#include "snappingrange.h"  // for InvalidatedRanges
#include "gnash.h" // for gnash::key::code type

#include <string>

// Forward declarations
namespace gnash
{
    class render_handler;
    class movie_root;
}

namespace gnash
{


/// Enumerates mouse cursor types.
enum gnash_cursor_type {
  CURSOR_HAND,
  CURSOR_NORMAL,
  CURSOR_INPUT
};

/// Parent class from which all GUI implementations will depend.
class DSOEXPORT Gui {

public:

    virtual ~Gui();

    /** \brief
     * Initialise the gui and the associated renderer.
     * 
     * @param argc The commandline argument count.
     * @param argv The commandline arguments.
     * @return True on success; false on failure.
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

    /// End main rendering loop, making the call to run() return.
    //
    /// The default implementation calls exit(0), which isn't nice.
    /// Please implement the proper main loop quitter in the subclasses.
    ///
    virtual void quit()  { exit(0); }

    /// Create a menu and attach it to our window.
    virtual bool createMenu() = 0;

    /// Register event handlers.
    virtual bool setupEvents() = 0;

    /// Render the current buffer.

    /// For OpenGL, this means that the front and back buffers are swapped.
    virtual void renderBuffer() = 0;

    /// Gives the GUI a *hint* which region of the stage should be redrawn.
    //
    /// There is *no* restriction what the GUI might do with these coordinates. 
    /// Normally the GUI forwards the information to the renderer so that
    /// it avoids rendering regions that did not change anyway. The GUI can
    /// also alter the bounds before passing them to the renderer and it's
    /// absolutely legal for the GUI to simply ignore the call.
    ///
    /// Coordinates are in TWIPS!
    ///
    /// Note this information is given to the GUI and not directly to the 
    /// renderer because both of them need to support this feature for 
    /// correct results. It is up to the GUI to forward this information to
    /// the renderer.
    ///
    // does not need to be implemented (optional feature),
    // but still needs to be available.
    //
    // Why "rect" (floats)? Because the gui does not really
    // know about the scale the renderer currently uses... 
    //
    // <strk> but it does not about the "semantic" of the TWIPS
    //        coordinate space, which is integer values...
    //        The question really is: why floats for TWIPS ?
    //        (guess this goes deep in the core/server libs)
    //
    virtual void setInvalidatedRegion(const rect& bounds);
    virtual void setInvalidatedRegions(const InvalidatedRanges& ranges);
    
    // Called right before rendering anything (after setInvalidatedRegion).
    // Used by GTK-AGG.
    virtual void beforeRendering() { /* nop */ };
    
    // Should return TRUE when the GUI/Renderer combination supports multiple
    // invalidated bounds regions. 
    virtual bool want_multiple_regions() { return false; }

    /// Asks the GUI handler if the next frame should be redrawn completely. 
    //
    /// For example, when the contents of the player window have been destroyed,
    /// then want_redraw() should return true so that setInvalidatedRegion() is
    /// called with the coordinates of the complete screen. 
    virtual bool want_redraw();


    /// Sets the current mouse cursor for the Gui window.
    virtual void setCursor(gnash_cursor_type newcursor);

    /// @return The value to which the movie width should be scaled.
    float getXScale();

    /// @return The value to which the movie height shold be scaled.
    float getYScale();

    /// @return Whether or not the movie should be looped indefinitely.
    bool loops();

    /// Mouse notification callback to be called when the mouse is moved.
    //
    /// @param x The mouse coordinate X component in pixels.
    /// @param y The mouse coordinate Y component in pixels.
    ///
    void notify_mouse_moved(int x, int y);

    /// Mouse notification callback to be called when the mouse is clicked.
    //
    /// @param mouse_pressed Determines whether the mouse button is being
    ///                      pressed (true) or being released (false)
    /// @param mask A binary representation of the buttons currently pressed.
    ///
    void notify_mouse_clicked(bool mouse_pressed, int mask);

    /// Key event notification to be called when a key is pressed or depressed
    //
    /// @param k The key code.
    /// @param modifier Modifier key identifiers from gnash::key::modifier ORed together
    /// @param pressed Determines whether the key is being
    ///           pressed (true) or being released (false)
    ///
    void notify_key_event(gnash::key::code k, int modifier, bool pressed);

    /// Resize the client area view and the window accordingly.
    //
    /// @param width  The desired width in pixels.
    /// @param height The desired height in pixels.
    void resize_view(int width, int height);


    /// \brief
    /// Advances the movie to the next frame. This is to take place after the
    /// interval specified in the call to setInterval().
    static bool advance_movie(Gui* gui);

    /// Put the application in "stop" mode
    //
    /// When in stop mode the application won't be advanced.
    ///
    void stop() { _stopped=true; }

    /// Put the application in "play" mode
    //
    /// When in stop mode the application will be advanced as usual.
    ///
    void play() { _stopped=false; }

    /// Toggle between "stop" and "play" mode
    //
    /// See stop() and play()
    ///
    void pause() { _stopped = !_stopped; }

    /// See stop(), play() and pause()
    bool isStopped() const { return _stopped; }

    /** @name Menu callbacks
     *  These callbacks will be called when a menu item is clicked.
     *  @{
     */
    void menu_restart();
    void menu_quit();
    void menu_about();
    void menu_play();
    void menu_pause();
    void menu_stop();
    void menu_step_forward();
    void menu_step_backward();
    void menu_jump_forward();
    void menu_jump_backward();
    void menu_toggle_sound();
    /// @}

#ifdef GNASH_FPS_DEBUG
    /// Set the interval between FPS debugging prints
    //
    /// See fpsCounterTick()
    ///
    void setFpsTimerInterval(float interval)
    {
	    assert(interval >= 0.0);
	    fps_timer_interval = interval;
    }
#endif // def GNASH_FPS_DEBUG

protected:

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
    /// Determines if playback should restart after the movie ends.
    bool            _loop;
    /// The X Window ID to attach to. If zero, we create a new window.
    unsigned long   _xid;

    // should it be unsigned ints ? (probably!)
    // This would be 0,0,_width,_height, so maybe
    // we should not duplicate the info with those
    // explicit values too..
    geometry::Range2d<int> _validbounds;

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
    /// Signals that the next frame must be re-rendered completely because the
    /// window size did change.
    bool            _redraw_flag;

    bool            _stopped;

private:

    bool display(movie_root* m);
    
#ifdef GNASH_FPS_DEBUG
    unsigned int fps_counter;
    float fps_rate_min, fps_rate_max;   

    // Number of calls to fpsCounterTick, which is also
    // the number of calls to movie_advance()
    unsigned int fps_counter_total;

    uint64_t fps_timer, fps_start_timer;     

    ///	The time, in seconds, between prints (which also resets the fps counter).
    //
    ///	interval must be >= 0
    ///
    float fps_timer_interval;
    
    /// \brief
    /// Should be called on every frame advance (including inter-frames caused
    /// by mouse events).
    //
    /// Based on fps-timer_interval. See setFpsTimerInterval.
    ///
    void fpsCounterTick();

#endif // def GNASH_FPS_DEBUG

};

/// Named constructors
std::auto_ptr<Gui> createGTKGui(unsigned long xid, float scale, bool loop, unsigned int depth);
std::auto_ptr<Gui> createKDEGui(unsigned long xid, float scale, bool loop, unsigned int depth);
std::auto_ptr<Gui> createSDLGui(unsigned long xid, float scale, bool loop, unsigned int depth);
std::auto_ptr<Gui> createFLTKGui(unsigned long xid, float scale, bool loop, unsigned int depth);
std::auto_ptr<Gui> createFBGui(unsigned long xid, float scale, bool loop, unsigned int depth);
std::auto_ptr<Gui> createAQUAGui(unsigned long xid, float scale, bool loop, unsigned int depth);
std::auto_ptr<Gui> createRISCOSGui(unsigned long xid, float scale, bool loop, unsigned int depth);

 
} // end of gnash namespace

// end of _GUI_H_
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
