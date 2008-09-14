// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#ifndef GNASH_GUI_H
#define GNASH_GUI_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "rect.h"  // for composition
#include "snappingrange.h"  // for InvalidatedRanges
#include "GnashKey.h" // for gnash::key::code type
#include "smart_ptr.h"

#ifdef USE_SWFTREE
#include "tree.hh" // for tree
#endif

#include <string>
#include <map>

// Define this to enable fps debugging without touching
// gnashconfig.h
//#define GNASH_FPS_DEBUG

// Define the following macro if you want to skip rendering
// when late on FPS time.
// This is an experimental feature, so it's off by default
//#define SKIP_RENDERING_IF_LATE


/// Define this to disable region updates debugging altogether. If undefined,
/// debugging will be a runtime option. The flag and flag-setting functions
/// will not be disabled (too ugly).
///
/// This should go in gnashconfig.h
///
/// This has the side effect that all frames will be re-rendered completely
/// but in contrast to FORCE_REDRAW it won't re-render when no motion
/// has been detected in the movie (for example when the movie is stopped).
///
//#define DISABLE_REGION_UPDATES_DEBUGGING 1


/// Define this to support keyboard-based pointer movements
#define ENABLE_KEYBOARD_MOUSE_MOVEMENTS 1

// Forward declarations
namespace gnash
{
    class render_handler;
    class movie_root;
    class movie_definition;
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
class Gui {

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
    virtual void setInterval(unsigned int interval) {
      _interval = interval;
    }

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

    // Information for System.capabilities to be reimplemented in
    // each gui.
    virtual double getPixelAspectRatio() { return 0; }
    virtual int getScreenResX() { return 0; }
    virtual int getScreenResY() { return 0; }
    virtual double getScreenDPI() { return 0; }
    virtual std::string getScreenColor() { return ""; }

    /// @return Whether or not the movie should be looped indefinitely.
    bool loops() const { return _loop; }

    /// @return Whether the movie is running fullscreen or not.    
    bool isFullscreen() const { return _fullscreen; }

    /// Mouse notification callback to be called when the mouse is moved.
    //
    /// @param x The mouse coordinate X component in user/window coordinate space (pixels).
    /// @param y The mouse coordinate Y component in user/window coordinate space (pixels).
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

    /// Update stage matrix accordingly to window size and flash Stage
    /// configuration (scaleMode, alignment)
    //
    /// This method should be called from the core lib when Stage configuration
    /// change or is called by resize_view.
    ///
    void updateStageMatrix();

    /// \brief
    /// Advances the movie to the next frame. This is to take place after the
    /// interval specified in the call to setInterval().
    bool advanceMovie();

    /// Convenience static wrapper around advanceMovie for callbacks happiness.
    static bool advance_movie(Gui* gui)
    {
        return gui->advanceMovie();
    }

    /// Force immediate redraw
    ///
    void refreshView();

    /// Attempt to run in a fullscreen window both for plugin and
    /// standalone player.
    //
    /// Use isFullscreen() to see if gnash thinks
    /// it's running in fullscreen or not. The switch to fullscreen may
    /// fail if, for instance, the window manager refuses to allow it, but
    /// the flag will be set anyway.
    virtual void setFullscreen();

    /// Return from fullscreen to normal mode.
    ///
    virtual void unsetFullscreen();
    
    /// Sets whether the gui should show the system mouse pointer
    //
    /// @param show true if the mouse should be shown.
    /// @return true if the state changed.
    virtual bool showMouse(bool show);

    /// Sets whether the menus should be shown (for fscommand)
    //
    /// @param show true if the menu bar should be shown.
    virtual void showMenu(bool show);

    /// Sets whether scaling should be allowed (for fscommand)
    //
    /// @param allow true if stage scaling should be allowed
    virtual void allowScale(bool allow);
    
    // Toggle between fullscreen and normal mode
    void toggleFullscreen();

    /// Put the application in "stop" mode
    //
    /// When in stop mode the application won't be advanced.
    ///
    void stop();

    /// Put the application in "play" mode
    //
    /// When in play mode the application will be advanced as usual.
    ///
    void play();

    /// Toggle between "stop" and "play" mode
    //
    /// See stop() and play()
    ///
    void pause();

    /// Start the movie
    //
    /// This function will create an instance of the registered top-level 
    /// movie definition, set variables into it and place it to the stage.
    ///
    void start();

    /// See stop(), play() and pause()
    bool isStopped() const { return _stopped; }
    
    /// Whether gnash is is running as a plugin
    bool isPlugin() const { return (( _xid )); }
    
    void setMaxAdvances(unsigned long ul) { if (ul > 0) _maxAdvances = ul; }
    
    void showUpdatedRegions(bool x) { _showUpdatedRegions = x; }
    bool showUpdatedRegions() { return _showUpdatedRegions; }

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


#ifdef USE_SWFTREE
    // TODO: use a tree-like structure (tree.hh?)
    typedef std::pair<std::string, std::string> StringPair;
    typedef tree<StringPair> InfoTree;

    /// \brief
    /// Return a tree containing informations about the movie
    /// currently being played (or NULL, if the VM isn't initialized yet)
    ///
    std::auto_ptr<InfoTree> getMovieInfo() const;
#endif

    typedef std::map<std::string, std::string> VariableMap;

    /// Add variables to set into instances of the top-level movie definition
    void addFlashVars(VariableMap& vars);

    /// Set the definition of top-level movie
    void setMovieDefinition(movie_definition* md);

    /// Set the stage to advance/display
    void setStage(movie_root* stage);

    /// Prompt user with a question she can answer with yes/no
    //
    /// @param question
    ///        The question to ask user
    ///
    /// @return
    ///        true for YES, false for NO
    ///
    /// The default implementation always returns true.
    ///
    virtual bool yesno(const std::string& question);

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

    /// Desired colour depth in bits.
    int             _depth;

    /// Main loop interval: the time between successive advance_movie calls.
    unsigned int    _interval;

    /// The handler which is called to update the client area of our window.
    render_handler* _renderer;

    /// Signals that the next frame must be re-rendered completely because the
    /// window size did change.
    bool            _redraw_flag;

    // True if Gnash is running in fullscreen
    bool _fullscreen;

    // True if mouse pointer is showing
    bool _mouseShown;

    // Maximum number of advances before exit; 0 for no limit.
    unsigned long _maxAdvances;
    
    /// Counter to keep track of frame advances
    unsigned long _advances;

    /// Called by Gui::stop().  This can be used by GUIs to implement pause
    /// widgets (so that resuming a stopped animation is more user-friendly)
    virtual void stopHook() {}

    /// Called by Gui::play().
    virtual void playHook() {}

private:

    /// Width of a window pixel, in stage pseudopixel units.
    float           _xscale;

    /// Height of a window pixel, in stage pseudopixel units.
    float           _yscale;

    /// Window pixel X offset of stage origin
    boost::int32_t   _xoffset;

    /// Window pixel Y offset of stage origin
    boost::int32_t   _yoffset;

    bool display(movie_root* m);
    
#ifdef GNASH_FPS_DEBUG
    unsigned int fps_counter;
    float fps_rate_min, fps_rate_max;   

    // Number of calls to fpsCounterTick, which is also
    // the number of calls to movie_advance()
    unsigned int fps_counter_total;

    boost::uint64_t fps_timer, fps_start_timer;     

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

#ifdef SKIP_RENDERING_IF_LATE
    /// Estimated max number of milliseconds required for a call to ::display
    /// This should be incremented everytime we take more
    boost::uint32_t estimatedDisplayTime;
#endif // SKIP_RENDERING_IF_LATE

    VariableMap _flashVars;

    boost::intrusive_ptr<movie_definition> _movieDef;

    movie_root* _stage;

    /// True if the application has been put into "stop" mode
    bool            _stopped;

    /// True if the application didn't start yet
    bool            _started;
    
    /// If true, updated regions (invalidated ranges) are visibly outlined.
    bool _showUpdatedRegions;
    
#ifdef ENABLE_KEYBOARD_MOUSE_MOVEMENTS 
	int _xpointer;
	int _ypointer;
	bool _keyboardMouseMovements;
	int _keyboardMouseMovementsStep;
#endif // ENABLE_KEYBOARD_MOUSE_MOVEMENTS

};

/// Named constructors
std::auto_ptr<Gui> createGTKGui(unsigned long xid, float scale, bool loop, unsigned int depth);
std::auto_ptr<Gui> createKDEGui(unsigned long xid, float scale, bool loop, unsigned int depth);
std::auto_ptr<Gui> createSDLGui(unsigned long xid, float scale, bool loop, unsigned int depth);
std::auto_ptr<Gui> createFLTKGui(unsigned long xid, float scale, bool loop, unsigned int depth);
std::auto_ptr<Gui> createFBGui(unsigned long xid, float scale, bool loop, unsigned int depth);
std::auto_ptr<Gui> createAQUAGui(unsigned long xid, float scale, bool loop, unsigned int depth);
std::auto_ptr<Gui> createRISCOSGui(unsigned long xid, float scale, bool loop, unsigned int depth);
std::auto_ptr<Gui> createDumpGui(unsigned long xid, float scale, bool loop, unsigned int depth);

 
} // end of gnash namespace

// end of _GUI_H_
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
