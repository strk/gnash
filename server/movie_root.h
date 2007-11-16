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

/* $Id: movie_root.h,v 1.89 2007/11/16 13:24:30 strk Exp $ */

/// \page events_handling Handling of user events
///
/// There are two kinds of events:
/// - system generated
/// - user generated
///
/// System generated events are those like load, data recive, unload,
/// enter frame, etc.
/// User generated events are mouse movements and clicks, keyboard activity.
///
/// Events can trigger actions execution, if "handlers" are specified for
/// a specific event with ActionScript code.
/// The actions triggered by user events are executed *immediately*, not
/// at the next frame iteration. Nonetheless, since rendering of the stage
/// usually happens at fixed rate (frame rate) you won't see the effects
/// of actions execution until next iteration... unless...
///
/// Well, *some* events actions always trigger immediate redisplay, while
/// some others require a call to a special function to do so.
///
/// The events actions that trigger immediate redisplay are Button actions.
/// Colin Mook, in his "ActionScript - The Definitive Guide" sais:
/// << Buttons naturally update between frames >>
///
/// Other events, in particular MovieClip events such as mouseDown, mouseUp,
/// mouseMove, keyDown and keyUp don't by default trigger redisplay, unless
/// the attached action code makes a call to the special function named
/// 'updateAfterEvent()'.
///
/// For this purpose, user events notification functions in gnash core 
/// library return a boolean value, which tells wheter any action triggered
/// by the event requires immediate redisplay.
///
/// At the time of writing (2006-10-19) this is not implemented yet and
/// the return code is always TRUE. We shall work on it :)
///
/// The events notification functions that currently support this interface
/// are:
///
/// - bool movie_root::notify_mouse_moved(int x, int y);
/// - bool movie_root::notify_mouse_clicked(bool mouse_pressed, int mask);
/// - bool movie_root::notify_key_event(key::code k, bool down);
/// 
/// 


#ifndef GNASH_MOVIE_ROOT_H
#define GNASH_MOVIE_ROOT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"

#include "mouse_button_state.h" // for composition
#include "drag_state.h" // for composition
#include "movie_instance.h" // for inlines
#include "timers.h" // for composition
#include "asobj/Key.h"
#include "smart_ptr.h" // for memory management

#include <vector>
#include <list>
#include <set>

// Forward declarations
namespace gnash {
    class ExecutableCode; // for ActionQueue
    class Stage;
    class URL;
}

namespace gnash
{

struct DepthComparator
{
    typedef boost::intrusive_ptr<sprite_instance> LevelMovie;

    bool operator() (const LevelMovie& d1, const LevelMovie& d2)
    {
        return d1->get_depth() < d2->get_depth();
    }
};

/// The movie stage (absolute top level node in the characters hierarchy)
//
/// This is a wrapper around the set of loaded levels being played.
///
/// There is a *single* instance of this class for each run;
/// loading external movies will *not* create a new instance of it.
///
class DSOEXPORT movie_root 
{

public:

    /// Default constructor
    //
    /// Make sure to call setRootMovie() 
    /// before using any of this class methods !
    ///
    movie_root();

    ~movie_root();

    /// Set the root movie, replacing the current one if any.
    //
    /// This is needed for the cases in which the top-level movie
    /// is replaced by another movie by effect of a loadMovie call
    /// or similar.
    ///
    /// TODO: inspect what happens about VM version
    ///   (should the *new* movie drive VM operations?
    ///    -- hope not ! )
    ///
    /// Make sure to call this method before using the movie_root,
    /// as most operations are delegated to the associated/wrapped
    /// movie_instance.
    ///
    /// Note that the display viewport will be updated to match
    /// the size of given movie.
    ///
    /// A call to this method is equivalent to a call to setLevel(0, movie).
    ///
    /// @param movie
    /// The movie_instance to wrap.
    /// Will be stored in an intrusive_ptr.
    /// Must have a depth of 0.
    ///
    void setRootMovie(movie_instance* movie);

    /// Return the movie at the given level (0 if unloaded level).
    //
    /// POST CONDITIONS:
    /// - The returned character has a depth equal to 'num'
    ///
    boost::intrusive_ptr<movie_instance> getLevel(unsigned int num) const;

    /// Load movie at the specified URL in the given level 
    //
    /// Note that the display viewport will be updated to reflect
    /// the new layout.
    ///
    /// @param num
    /// The level number to load into.
    ///
    /// @param url
    /// The url to load the movie from.
    /// Can contain a query string, which would be parsed.
    ///
    /// @return false on error, true on success
    ///
    bool loadLevel(unsigned int num, const URL& url);

    /// @@ should this delegate to _level0?  probably !
    void set_member(
        const std::string& /*name*/,
        const as_value& /*val*/)
    {
    }

    /// @@ should this delegate to _level0?  probably !
    bool get_member(const std::string& /*name*/,
            as_value* /*val*/)
    {
        return false;
    }

    /// Change display viewport coordinates
    //
    /// This currently also change the display scale
    /// but we should instead only do it if rescaling
    /// is allowed.
    ///
    void set_display_viewport(int x0, int y0, int w, int h);

    /// Get current viewport width, in pixels
    unsigned getWidth() const
    {
        return m_viewport_width;
    }

    /// Get current viewport height, in pixels
    unsigned getHeight() const
    {
        return m_viewport_height;
    }

    /// Set whether rescaling is allowed or not.
    //
    /// When rescaling is not allowed the Stage listeners
    /// will get notified on any resize attempt.
    ///
    void allowRescaling(bool v)
    {
        _allowRescale=v;
    }

    bool isRescalingAllowed()
    {
        return _allowRescale;
    }

    /// \brief
        /// The host app can use this to tell the movie when
        /// user's mouse pointer has moved.
    //
    /// Coordinates are in pixels.
    ///
    /// This function should return TRUE iff any action triggered
    /// by the event requires redraw, see \ref events_handling for
    /// more info.
    ///
        bool notify_mouse_moved(int x, int y);

    /// \brief
    /// The host app can use this to tell the movie when the
    /// user clicked or released the mouse button.
    //
    /// @param mouse_pressed
    /// true if the mouse has been pressed, false if released
    ///
    /// @param mask
    /// ???
    ///
    /// This function should return TRUE iff any action triggered
    /// by the event requires redraw, see \ref events_handling for
    /// more info.
    ///
    bool notify_mouse_clicked(bool mouse_pressed, int mask);

    /// The host app can use this to tell the movie where the
    /// user's mouse pointer is.
    void notify_mouse_state(int x, int y, int buttons);

    /// \brief
    /// The host app can use this to tell the movie when
    /// user pressed or released a key.
    //
    /// This function should return TRUE iff any action triggered
    /// by the event requires redraw, see \ref events_handling for
    /// more info.
    ///
    bool notify_key_event(key::code k, bool down);

	/// \brief
    /// Use this to retrieve the last state of the mouse, as set via
    /// notify_mouse_state(). 
    //
    /// Coordinates are in PIXELS, NOT TWIPS.
    ///
    void get_mouse_state(int& x, int& y, int& buttons);

    void get_drag_state(drag_state& st);

    void set_drag_state(const drag_state& st);

    /// @return current top-level root sprite (_level0)
    sprite_instance* get_root_movie()
    {
        if ( _movies.empty() ) return NULL;
        return getLevel(0).get();
    }

    void stop_drag()
    {
        log_msg("stop_drag called");
        m_drag_state.reset();
    }

    /// Return definition of _level0 
    //
    /// TODO: drop this function ?
    ///
    movie_definition* get_movie_definition() const
    {
        assert(!_movies.empty());
        return getLevel(0)->get_movie_definition(); 
    }

    /// Add an interval timer
    //
    /// @param timer
    /// A Timer, ownership will be transferred. Must not be NULL.
    ///
    /// @param internal
    /// If true, this is an internal timer, so will get a negative id.
    ///
    /// @return An integer indentifying the timer
    ///         for subsequent call to clear_interval_timer.
    ///         It will NEVER be zero.
    ///
    unsigned int add_interval_timer(std::auto_ptr<Timer> timer, bool internal=false);

    /// Remove timer identified by given integer
    //
    /// @return true on success, false on error (no such timer)
    ///
    bool clear_interval_timer(unsigned int x);

    /// Return 0-based frame index of _level0
    //
    /// TODO: drop this function
    ///
    size_t get_current_frame() const
    {
        assert(!_movies.empty());
        return getLevel(0)->get_current_frame();
    }

    // @@ should this be in movie_instance ?
    float get_frame_rate() const {
        return get_movie_definition()->get_frame_rate();
    }

    /// \brief
    /// Return the size of a logical movie pixel as
    /// displayed on-screen, with the current device
    /// coordinates.
    float   get_pixel_scale() const
    {
        return m_pixel_scale;
    }

    // @@ Is this one necessary?
    //
    // TODO: drop this
    character* get_character(int character_id)
    {
        assert(!_movies.empty());
        return getLevel(0)->get_character(character_id);
    }

    void set_background_color(const rgba& color);

    void set_background_alpha(float alpha);

    float get_background_alpha() const
    {
        return m_background_color.m_a / 255.0f;
    }

    /// Entry point for movie advancement
    //
    /// This function does:
    ///     - Execute all timers
    ///     - Reset the next Random number
    ///     - Advance all advanceable characters in reverse-placement order
    ///     - Cleanup key listeners
    ///     - Process all queued actions
    ///     - Remove unloaded characters from the advanceable characters list.
    ///     - Run the GC collector
    ///
    void advance(float delta_time);

    /// 0-based!! delegates to _level0
    //
    /// TODO: drop this method ?
    ///
    void goto_frame(size_t target_frame_number)
    {
        getLevel(0)->goto_frame(target_frame_number);
    }

    void display();

    /// Delegate to _level0
    void set_play_state(sprite_instance::play_state s)
    {
        getLevel(0)->set_play_state(s);
    }

    /// For ActionScript interfacing convenience.
    //
    /// TODO: check if we really  need this. I guess we might
    ///       need for fscommand:, but we lack documentation
    ///       about where to find the method (which level?)
    ///
    const char* call_method(const char* method_name,
            const char* method_arg_fmt, ...);

    /// For ActionScript interfacing convenience.
    //
    /// TODO: check if we really  need this. I guess we might
    ///       need for fscommand:, but we lack documentation
    ///       about where to find the method (which level?)
    ///
    const char* call_method_args(const char* method_name,
            const char* method_arg_fmt, va_list args);

    void * get_userdata() { return m_userdata; }
    void set_userdata(void * ud ) { m_userdata = ud;  }

	/// Notify both the character listeners and general object listeners
	DSOEXPORT void notify_key_listeners(key::code k, bool down);

	/// Push a new character listener to the container after constructing the character
    void add_key_listener(as_object* listener);

	/// Remove a character from the container only when the character is unloaded
    void remove_key_listener(as_object* listener);

    DSOEXPORT void notify_mouse_listeners(const event_id& event);

    void add_mouse_listener(as_object* listener);

    void remove_mouse_listener(as_object* listener);

    /// Get the character having focus
    //
    /// The character having focus will receive mouse button
    /// and key presses/releases.
    ///
    /// @return the character having focus or NULL of none.
    ///
    character* getFocus();

    /// Set the character having focus
    //
    /// @param ch
    /// The character having focus. NULL to kill focus.
    ///
    void setFocus(character* ch);
    
    DSOEXPORT void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);

    /// Return the topmost active entity under the pointer
    //
    /// This method returns cached info, with cache updated
    /// by notify_mouse_moved (and should be updated also
    /// by movie advancement or actions execution maybe, not
    /// currently implmented).
    ///
    /// @return the topmost active entity under pointer or NULL if none.
    ///
    character* getActiveEntityUnderPointer() const;

    /// Return true if the mouse pointer is over an active entity
    bool isMouseOverActiveEntity() const;

    bool testInvariant() const;

    /// Action priority levels
    enum ActionPriorityLevel {

        /// Init actions, Init event handlers
        apINIT=0,

        /// Construct event handlers
        apCONSTRUCT=1,

        /// EnterFrame event handlers
        apENTERFRAME=2,

        /// Frame actions, load handlers, unload handlers
        apDOACTION=3,

        /// Last element used to easy computation of size...
        apSIZE
        
    };

    /// Push an executable code to the ActionQueue
    void pushAction(std::auto_ptr<ExecutableCode> code, int lvl=apDOACTION);

    /// Push an executable code to the ActionQueue
    void pushAction(const action_buffer& buf, boost::intrusive_ptr<character> target, int lvl=apDOACTION);

    /// Push a function code to the ActionQueue
    void pushAction(boost::intrusive_ptr<as_function> func, boost::intrusive_ptr<character> target, int lvl=apDOACTION);

#ifdef GNASH_USE_GC
    /// Mark all reachable resources (for GC)
    //
    /// Resources reachable from movie_root are:
    ///
    /// - All _level# movies (_movies)
    /// - Mouse entities (m_mouse_button_state)
    /// - Timer targets (_intervalTimers)
    /// - Resources reachable by ActionQueue code (_actionQueue)
    /// - Key listeners (_keyListeners || m_key_listeners)
    /// - global Key object (_keyobject)
    ///
    void markReachableResources() const;
#endif // GNASH_USE_GC

    /// \brief
    /// Register a newly born advanceable character to the
    /// list of characters to be advanced on next ::advance call.
    //
    /// The character will only be advanced if not unloaded when
    /// its turn comes. Characters are advanced in reverse-placement
    /// order (first registered is advanced last)
    ///
    void addLiveChar(boost::intrusive_ptr<character> ch)
    {
	// Don't register the object in the list twice 
	assert(std::find(_liveChars.begin(), _liveChars.end(), ch) == _liveChars.end());
        _liveChars.push_front(ch);
    }

    /// Cleanup all resources and run the GC collector
    //
    /// This method should be invoked before calling setRootMovie again
    /// for a clean restart.
    ///
    void clear();

    /// Reset stage to it's initial state
    //
    void reset();

    /// Call this method for disabling run of actions
    //
    /// NOTE: this will only work for queued actions, not
    ///       for *every* action. Supposedly all actions should
    ///       be queued, but this is not really always the case.
    ///       Notable exceptions are:
    ///         - Actions in callFrame target frame
    ///           but only executed by execution of the callFrame opcode
    ///         - on{,Clip}{Initialize,Construct} event handlers
    ///         - User event handlers (mouse,keyboard)
    ///
    void disableScripts();

    /// Return true if scripts execution is disabled
    bool scriptsDisabled() const { return _disableScripts; };

    /// Process action queues with higher priority then the priority
    /// of the action queue currently being processed.
    //
    /// This is intended to be called at the end of any function call
    /// and at the end of an action block.
    ///
    /// TODO: be aware of infinite loops !
    ///
    void flushHigherPriorityActionQueues();

private:

    /// Take care of dragging, if needed
    void doMouseDrag();

    /// Delete all elements on the action queue and empty it.
    void clearActionQueue();

    /// An element of the advanceable characters
    typedef boost::intrusive_ptr<character> AdvanceableCharacter;

    /// A list of AdvanceableCharacters
    //
    /// This is a list (not a vector) as we want to allow
    /// ::advance of each element to insert new characters before
    /// the start w/out invalidating iterators scanning the
    /// list forward for proper movie advancement
    ///
    typedef std::list<AdvanceableCharacter> LiveChars;

    /// The list of advanceable character, in placement order
    LiveChars _liveChars;

    /// Forbid copy 
    movie_root(const movie_root& ) { abort(); }

    /// Forbid assignment
    movie_root& operator=(const movie_root& ) { abort(); return *this; }

    /// Execute expired timers
    void executeTimers();

    /// Notify the global Key ActionScript object about a key status change
    key_as_object * notify_global_key(key::code k, bool down);

    /// Remove all listeners with a ref-count of 1
    /// (only referenced as key listeners)
    // in new design:
    // remove unloaded characters and unregistered as_objects 
    // from the key listeners container.
    void cleanup_key_listeners();

    /// Return the current Stage object
    //
    /// Can return NULL if it's been deleted or not
    /// yet initialized.
    ///
    boost::intrusive_ptr<Stage> getStageObject();

    typedef std::list<ExecutableCode*> ActionQueue;

    ActionQueue _actionQueue[apSIZE];

    /// Process all actions in the queue
    void processActionQueue();

    // TODO: use Range2d<int> ?
    int         m_viewport_x0, m_viewport_y0;

    /// Width and height of viewport, in pixels
    int         m_viewport_width, m_viewport_height;

    float       m_pixel_scale;

    rgba        m_background_color;
    float       m_timer;
    int         m_mouse_x, m_mouse_y, m_mouse_buttons;
    void *      m_userdata;

    mouse_button_state  m_mouse_button_state;

    // Flags for event handlers
    bool            m_on_event_xmlsocket_ondata_called;
    bool            m_on_event_xmlsocket_onxml_called;
    bool            m_on_event_load_progress_called;

    typedef std::map<int, Timer*> TimerMap;

    TimerMap _intervalTimers;
    unsigned int _lastTimerId;

	/// A set of as_objects kept by intrusive_ptr
	/// TODO: dont' use std::use, we need to well control the calling order
    typedef std::set< boost::intrusive_ptr<as_object> > ListenerSet;

    /// key listeners container
    typedef std::list< boost::intrusive_ptr<as_object> > KeyListeners;

    /// Characters for listening key events
    KeyListeners m_key_listeners;

    boost::intrusive_ptr<key_as_object> _keyobject;

    /// Objects listening for mouse events (down,up,move)
    ListenerSet m_mouse_listeners;

    character*              m_active_input_text;
    float                   m_time_remainder;

    /// @@ fold this into m_mouse_button_state?
    drag_state m_drag_state;

    /// The movie instance wrapped by this movie_root
    //
    /// We keep a pointer to the base sprite_instance class
    /// to avoid having to replicate all of the base class
    /// interface to the movie_instance class definition
    ///
    /// TODO: use a different container, to allow for _level0 and _level100
    ///       to exist while just taking 2 elements in the container.
    ///       Appropriate container could be list, set or map (order is important)
    ///
    typedef boost::intrusive_ptr<sprite_instance> LevelMovie;
    typedef std::map<unsigned int, LevelMovie> Levels;
    Levels _movies;

    /// This function should return TRUE iff any action triggered
    /// by the event requires redraw, see \ref events_handling for
    /// more info.
    ///
    bool fire_mouse_event();

    /// If set to false, no rescale should be performed
    /// when changing viewport size
    bool _allowRescale;

    /// \brief
    /// Return the topmost entity covering the given point
    /// and enabled to receive mouse events.
    //
    /// Return NULL if no "active" entity is found under the pointer.
    ///
    /// Coordinates of the point are given in world coordinate space.
    /// (twips)
    ///
    /// @param x
    ///     X ordinate of the pointer, in world coordinate space (twips)
    ///
    /// @param y
    ///     Y ordinate of the pointer, in world coordiante space (twips).
    ///
    character* getTopmostMouseEntity(float x, float y);

    /// Delete characters removed from the stage
    /// from the display lists
    void cleanupDisplayList();

    /// Advance a live character
    //
    /// @param ch
    ///     The character to advance, will NOT be advanced if unloaded
    ///
    /// @param delta_time
    ///     A left-over parameter from the ancient times... will be forwarded
    ///     to the calls to ::advance
    ///
    static void advanceLiveChar(boost::intrusive_ptr<character> ch, float delta_time);

    /// Advance all non-unloaded live chars
    void advanceLiveChars(float delta_time);

    /// Put the given movie at the given level 
    //
    /// @param movie
    /// The movie_instance to store at the given level.
    /// Will be stored in an intrusive_ptr.
    /// It's depth will be set to <num> and it's name to
    /// _level<num>
    ///
    void setLevel(unsigned int num, boost::intrusive_ptr<movie_instance> movie);

    /// Return the global Key object 
    //
    /// @@ might be worth making public
    ///
    boost::intrusive_ptr<key_as_object> getKeyObject();

    /// Boundaries of the Stage are always world boundaries
    /// and are only invalidated by changes in the background
    /// color.
    void setInvalidated() { _invalidated=true; }

    /// Every ::display call clears the invalidated flag
    //
    /// See setInvalidated();
    ///
    void clearInvalidated() { _invalidated=false; }

    /// An invalidated stage will trigger complete redraw
    //
    /// So, this method should return true everytime a complete
    /// redraw is needed. This is tipically only needed when
    /// the background changes.
    ///
    /// See setInvalidated() and clearInvalidated().
    ///
    bool isInvalidated() { return _invalidated; }

    /// See setInvalidated
    bool _invalidated;

    /// This is set to true if execution of scripts
    /// aborted due to action limit set or whatever else
    bool _disableScripts;

    /// Return the priority level of first action queue containing actions.
    //
    /// Scanned in proprity order (lower first)
    ///
    int minPopulatedPriorityQueue() const;

    /// Process all actions in the the given queue, till more actions
    /// are found in lower levels, in which case we have an earlier
    /// return.
    int processActionQueue(int lvl);

    int _processingActionLevel;

    bool processingActions() const
    {
        return (_processingActionLevel < apSIZE);
    }

};


} // namespace gnash

#endif // GNASH_MOVIE_ROOT_H

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
