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

#ifndef GNASH_GUI_H
#define GNASH_GUI_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/intrusive_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <string>
#include <map>
#include <utility>

#include "SWFRect.h"  // for composition
#include "snappingrange.h"  // for InvalidatedRanges
#include "ScreenShotter.h"
#include "GnashKey.h"
#include "Renderer.h" 
#include "VirtualClock.h"
#include "SystemClock.h"
#include "GnashEnums.h" 
#include "movie_root.h"

#ifdef USE_SWFTREE
#include "tree.hh" // for tree
#endif

// Define this to enable fps debugging without touching
// gnashconfig.h
//#define GNASH_FPS_DEBUG

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
namespace gnash {
    class SWFRect;
    class ScreenShotter;
    class RunResources;
    class movie_root;
    class movie_definition;
}

namespace gnash {

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

    /// \brief/
    ///  Initialise the gui and the associated renderer.
    /// 
    /// @param argc The commandline argument count.
    /// @param argv The commandline arguments.
    /// @return True on success; false on failure.
    virtual bool init(int argc, char **argv[]) = 0;

    /// Set main loop delay in milliseconds. 
    virtual void setInterval(unsigned int interval) {
      _interval = interval;
    }

    /// Return the clock provided by this Gui.
    //
    /// The Gui clock will be paused when the gui is put
    /// in pause mode and resumed when gui playback is resumed.
    ///
    virtual VirtualClock& getClock() { return _virtualClock; }

    /// Set the time in milliseconds after which the programme should exit.
    virtual void setTimeout(unsigned int timeout) = 0;

    void setScreenShotter(std::auto_ptr<ScreenShotter> ss);

    /// \brief
    /// Create and display our window.
    ///
    /// @param title The window title.
    /// @param width The desired window width in pixels.
    /// @param height The desired window height in pixels.
    /// @param xPosition The desired window X position from the top left corner.
    /// @param yPosition The desired window Y position from the top left corner.
    virtual bool createWindow(const char* title, int width, int height,
                       int xPosition = 0, int yPosition = 0) = 0;

    virtual void resizeWindow(int width, int height);

    /// Start main rendering loop.
    virtual bool run() = 0;

    /// Always called on exit.
    //
    /// Handles any common functions, then calls virtual quitUI().
    void quit();

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
    virtual void setInvalidatedRegion(const SWFRect& bounds);
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

    virtual void setClipboard(const std::string& copy);

    // Information for System.capabilities to be reimplemented in
    // each gui.
    virtual double getPixelAspectRatio() const { return 0; }

    virtual std::pair<int, int> screenResolution() const {
        return std::make_pair(0, 0);
    }

    virtual double getScreenDPI() const { return 0; }

    /// Get the screen color type.
    //
    /// The choice is between "color" and something designating
    /// monochrome (not sure what). If this isn't implemented in the
    /// gui we return "color".
    virtual std::string getScreenColor() const {
        return "color";
    }

    /// @return Whether or not the movie should be looped indefinitely.
    bool loops() const { return _loop; }

    /// @return Whether the movie is running fullscreen or not.    
    bool isFullscreen() const { return _fullscreen; }

    /// Mouse notification callback to be called when the mouse is moved.
    //
    /// @param x    The mouse coordinate X component in user/window
    ///             coordinate space (pixels).
    /// @param y    The mouse coordinate Y component in user/window
    ///             coordinate space (pixels).
    void notifyMouseMove(int x, int y);

    /// Mouse notification callback to be called when the mouse is clicked.
    //
    /// @param mouse_pressed Determines whether the mouse button is being
    ///                      pressed (true) or being released (false)
    void notifyMouseClick(bool mouse_pressed);

    /// Send a mouse wheel event to the stage.
    //
    /// @param delta    A number expressing the extent of the wheel scroll.
    void notifyMouseWheel(int delta);

    /// Key event notification to be called when a key is pressed or depressed
    //
    /// @param k The key code.
    /// @param modifier
    ///   Modifier key identifiers from gnash::key::modifier ORed together
    /// @param pressed
    ///   Determines whether the key is being pressed (true)
    ///   or being released (false)
    ///
    void notify_key_event(gnash::key::code k, int modifier, bool pressed);

    /// Resize the client area view and the window accordingly.
    //
    /// @param width  The desired width in pixels.
    /// @param height The desired height in pixels.
    void resize_view(int width, int height);

    /// Update stage SWFMatrix accordingly to window size and flash Stage
    /// configuration (scaleMode, alignment)
    //
    /// This method should be called from the core lib when Stage configuration
    /// change or is called by resize_view.
    ///
    void updateStageMatrix();

    /// \brief
    /// Give movie an heart-beat.
    //
    /// This is to take place after the
    /// interval specified in the call to setInterval().
    ///
    /// Wheter or not this beat advanced the movie to the next frame
    /// depends on elapsed time since last advancement.
    ///
    /// @return true if this beat resulted in actual frame advancement.
    ///
    bool advanceMovie(bool doDisplay = true);

    /// Convenience static wrapper around advanceMovie for callbacks happiness.
    //
    /// NOTE: this function always return TRUE, for historical reasons.
    /// TODO: bring code up-to-date to drop this legacy return code..
    ///       
    static bool advance_movie(Gui* gui) {
        gui->advanceMovie();
        return true;
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

    /// Hide the menu bar when using standalone player
    ///
    virtual void hideMenu();
    
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
    bool isPlugin() const { return ((_xid)); }

    /// Take a screenshot now!
    void takeScreenShot();

    /// Set the maximum number of frame advances before Gnash exits.
    void setMaxAdvances(unsigned long ul) { if (ul) _maxAdvances = ul; }
    
    void showUpdatedRegions(bool x) { _showUpdatedRegions = x; }
    bool showUpdatedRegions() const { return _showUpdatedRegions; }

    /// Instruct the core to restart the movie and
    /// set state to play(). This does not change pause
    /// state.
    void restart();

    /// Set rendering quality, if not locked by RC file..
    void setQuality(Quality q);

    /// Get current rendering quality
    Quality getQuality() const;

    /// Toggle sound state between muted and unmuted. If
    /// there is no active sound handler this does nothing.
    void toggleSound();

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
    /// Return a tree containing information about the movie playing.
    std::auto_ptr<movie_root::InfoTree> getMovieInfo() const;
#endif

    typedef std::map<std::string, std::string> VariableMap;

    /// Add variables to set into instances of the top-level movie definition
    void addFlashVars(VariableMap& vars);

    /// Set the definition of top-level movie
    void setMovieDefinition(movie_definition* md);

    /// Set the stage to advance/display
    void setStage(movie_root* stage);

    /// Set the name of a file to dump audio to
    void setAudioDump(const std::string& fname) {
        _audioDump = fname;
    }

    /// The root movie, or "Stage"
    movie_root* getStage() { return _stage; };
    
    /// Handle error message from the core
    //
    /// @param msg        The error message recieved
    ///
    virtual void error(const std::string& /*msg*/) {}

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

    /// Width of a window pixel, in stage pseudopixel units.
    float getXScale() const { return _xscale; };

    /// Height of a window pixel, in stage pseudopixel units.
    float getYScale() const { return _yscale; };

    /// Height of a window pixel, in stage pseudopixel units.
    float getFPS() const { return (_movieDef) ? _movieDef->get_frame_rate() : 0;
    };

protected:

    /// Default constructor. Initialises members to safe defaults.
    Gui(RunResources& r);

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
    Gui(unsigned long xid, float scale, bool loop, RunResources& r);
    
    /// End main rendering loop calling GUI-specific exit functions.
    //
    /// Do not call this directly. Call quit() instead.
    //
    /// The default implementation calls exit(EXIT_SUCCESS), which isn't nice.
    /// Please implement the proper main loop quitter in the subclasses.
    virtual void quitUI() {
        std::exit(EXIT_SUCCESS);
    }

    /// Watch a file descriptor.
    //
    /// An implementing Gui should monitor the file descriptor in its main
    /// loop. When the file descriptor is triggered, the implementation should
    /// call callCallback().
    ///
    /// @param fd The file descriptor to be watched
    virtual bool watchFD(int /* fd */)
    {
        log_unimpl("This GUI does not implement FD watching.");
        return false;
    }


    /// Determines if playback should restart after the movie ends.
    bool _loop;

    /// The X Window ID to attach to. If zero, we create a new window.
    unsigned long _xid;

    // This would be 0,0,_width,_height, so maybe
    // we should not duplicate the info with those
    // explicit values too..
    geometry::Range2d<int> _validbounds;

    /// Desired window width.
    int _width;

    /// Desired window height.
    int _height;

    /// Per-run resources
    RunResources& _runResources;

    /// Main loop interval: the time between successive advance_movie calls.
    unsigned int _interval;

    /// The handler which is called to update the client area of our window.
    boost::shared_ptr<Renderer> _renderer;

    /// Signals that the next frame must be re-rendered completely because the
    /// window size did change.
    bool _redraw_flag;

    // True if Gnash is running in fullscreen
    bool _fullscreen;

    // True if mouse pointer is showing
    bool _mouseShown;

    // Maximum number of advances before exit; 0 for no limit.
    unsigned long _maxAdvances;
    
    /// Counter to keep track of frame advances
    unsigned long _advances;

    /// Name of a file to dump audio to
    std::string _audioDump;

    /// Called by Gui::stop().  This can be used by GUIs to implement pause
    /// widgets (so that resuming a stopped animation is more user-friendly)
    virtual void stopHook() {}

    /// Called by Gui::play().
    virtual void playHook() {}

    /// Determines whether the Gui is visible (not obscured).
    virtual bool visible() { return true; }
private:

    struct Display;

    std::map<int /* fd */, boost::function<void ()> > _fd_callbacks;

    /// Width of a window pixel, in stage pseudopixel units.
    float _xscale;

    /// Height of a window pixel, in stage pseudopixel units.
    float _yscale;

    /// Window pixel X offset of stage origin
    boost::int32_t _xoffset;

    /// Window pixel Y offset of stage origin
    boost::int32_t _yoffset;

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
    
    /// Number of frames rendering of which was dropped
    unsigned int frames_dropped;

    /// \brief
    /// Should be called on every frame advance (including inter-frames caused
    /// by mouse events).
    //
    /// Based on fps-timer_interval. See setFpsTimerInterval.
    ///
    void fpsCounterTick();

#endif // def GNASH_FPS_DEBUG

    VariableMap _flashVars;

    boost::intrusive_ptr<movie_definition> _movieDef;
    
    /// The root movie, or "Stage"
    movie_root* _stage;

    /// True if the application has been put into "stop" mode
    bool _stopped;

    /// True if the application didn't start yet
    bool _started;

    /// If true, updated regions (invalidated ranges) are visibly outlined.
    bool _showUpdatedRegions;

    SystemClock _systemClock;
    InterruptableVirtualClock _virtualClock;
    
    /// Checked on each advance for screenshot activity if it exists.
    boost::scoped_ptr<ScreenShotter> _screenShotter;

#ifdef ENABLE_KEYBOARD_MOUSE_MOVEMENTS 
    int _xpointer;
    int _ypointer;
    bool _keyboardMouseMovements;
    int _keyboardMouseMovementsStep;
#endif // ENABLE_KEYBOARD_MOUSE_MOVEMENTS
};

/// Named constructors
namespace gui {
  std::auto_ptr<Gui> createFBGui(unsigned long xid, float scale, bool loop, RunResources& r);
}
std::auto_ptr<Gui> createGTKGui(unsigned long xid, float scale, bool loop, RunResources& r);
std::auto_ptr<Gui> createKDEGui(unsigned long xid, float scale, bool loop, RunResources& r);
std::auto_ptr<Gui> createQt4Gui(unsigned long xid, float scale, bool loop, RunResources& r);
std::auto_ptr<Gui> createSDLGui(unsigned long xid, float scale, bool loop, RunResources& r);
std::auto_ptr<Gui> createFLTKGui(unsigned long xid, float scale, bool loop, RunResources& r);
std::auto_ptr<Gui> createAQUAGui(unsigned long xid, float scale, bool loop, RunResources& r);
std::auto_ptr<Gui> createRISCOSGui(unsigned long xid, float scale, bool loop, RunResources& r);
std::auto_ptr<Gui> createAOS4Gui(unsigned long xid, float scale, bool loop, RunResources& r);
std::auto_ptr<Gui> createHaikuGui(unsigned long xid, float scale, bool loop, RunResources& r);
std::auto_ptr<Gui> createDumpGui(unsigned long xid, float scale, bool loop, RunResources& r);

 
} // end of gnash namespace

// end of _GUI_H_
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
