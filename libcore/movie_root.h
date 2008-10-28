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
#include "gnashconfig.h" //USE_SWFTREE
#endif

#include "smart_ptr.h" // GNASH_USE_GC
#include "dsodefs.h" // DSOEXPORT
#include "mouse_button_state.h" // for composition
#include "drag_state.h" // for composition
#include "asobj/Key_as.h"
#include "smart_ptr.h" // for memory management
#include "URL.h" // for loadMovie
#include "GnashKey.h" // key::code
#include "movie_instance.h"

#ifdef USE_SWFTREE
# include "tree.hh"
#endif

// GNASH_PARANOIA_LEVEL:
// 0 : (not unimplemented)
// 1 : quick assertions
// 2 : add testInvariant
//
#ifndef GNASH_PARANOIA_LEVEL
# define GNASH_PARANOIA_LEVEL 1
#endif

#include <vector>
#include <list>
#include <set>
#include <bitset>
#include <boost/noncopyable.hpp>

// Forward declarations
namespace gnash {
    class ExecutableCode; // for ActionQueue
    class Stage_as;
    class URL;
    class Timer;
    class MovieClip;
    class VirtualClock;
}

namespace gnash
{

struct DepthComparator
{
    typedef boost::intrusive_ptr<MovieClip> LevelMovie;

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
class DSOEXPORT movie_root : boost::noncopyable
{

public:

    /// Default constructor
    //
    /// Make sure to call setRootMovie() 
    /// before using any of this class methods !
    ///
    movie_root(const movie_definition& def, VirtualClock& clock,
            const std::string& baseURL);

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

    /// Swap depth of a level (or two)
    //
    /// Character's depths are updated.
    ///
    /// @param sp
    ///        The level to change depth/level of. A pointer to it is expected
    ///        to be found in the _level# container, or an error will be printed
    ///        and the call would result in a no-op.
    ///
    /// @param depth
    ///        New depth to assign to the character. If another level exists at 
    ///        the target depth the latter is moved in place of the former, with
    ///        its depth also updated.
    ///
    void swapLevels(boost::intrusive_ptr<MovieClip> sp, int depth);

    /// Drop level at given depth.
    //
    /// @param depth
    ///   Depth of the level to drop. Note that this is 
    ///   -character::staticDepthOffset for the root movie. Must be >=0 and
    ///   <= 1048575 or an assertion will fail. Note that if the depth
    ///   evaluates to the original root movie nothing happens (not allowed
    ///   to remove that). It is not tested if it's allowed to remove _level0
    ///   after loading into it.
    void dropLevel(int depth);

    /// @@ should this delegate to _level0?  probably !
    void set_member(const std::string& /*name*/, const as_value& /*val*/)
    {
    }

    /// @@ should this delegate to _level0?  probably !
    bool get_member(const std::string& /*name*/, as_value* /*val*/)
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

    /// \brief
    /// Return the notional width of the stage, value depending
    /// on scaleMode
    unsigned getStageWidth() const;

    /// \brief
    /// Return the notional height of the stage, actual value depending
    /// on scaleMode
    unsigned getStageHeight() const;

    /// \brief
    /// The host app can use this to tell the movie when
    /// user's mouse pointer has moved.
    //
    /// Coordinates are in Stage Coordinate Space (pseudo-pixels units).
    ///
    /// This function should return TRUE iff any action triggered
    /// by the event requires redraw, see \ref events_handling for
    /// more info.
    ///
    /// TODO: take twips (or float pixels), or we won't be able to
    ///       support sub-pixel accuracy in collision detection.
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

    /// @return the originating root movie (not necessarely _level0)
    movie_instance* getRootMovie() const
    {
        return _rootMovie.get();
    }

    void stop_drag()
    {
        m_drag_state.reset();
    }

    /// Return definition of originating root movie 
    //
    /// TODO: rename to getOriginatingDefinition ?
    ///
    movie_definition* get_movie_definition() const
    {
        return getRootMovie()->get_movie_definition();
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
    unsigned int add_interval_timer(std::auto_ptr<Timer> timer,
            bool internal = false);

    /// Remove timer identified by given integer
    //
    /// @return true on success, false on error (no such timer)
    ///
    bool clear_interval_timer(unsigned int x);

    /// Return 0-based frame index of originating root movie
    //
    /// TODO: drop this function (currently used by gprocessor)
    ///       or change it to to delegate to _level0 ?
    ///
    size_t get_current_frame() const
    {
        return getRootMovie()->get_current_frame();
    }

    void set_background_color(const rgba& color);

    void set_background_alpha(float alpha);

    float get_background_alpha() const
    {
        return m_background_color.m_a / 255.0f;
    }

    /// Main and only callback from hosting application.
    /// Expected to be called at 10ms resolution.
    void advance();

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
    void advanceMovie();

    /// 0-based!! delegates to originating root movie
    //
    /// TODO: drop this method. currently used by gprocessor.
    void goto_frame(size_t target_frame_number)
    {
        getRootMovie()->goto_frame(target_frame_number);
    }

    void display();

    /// Delegate to originating root movie
    //
    /// TODO: drop ?
    void set_play_state(MovieClip::play_state s)
    {
        getRootMovie()->set_play_state(s);
    }

    /// Notify still loaded character listeners for key events
    DSOEXPORT void notify_key_listeners(key::code k, bool down);

    /// Push a new character listener for key events
    void add_key_listener(character* listener)
    {
        add_listener(m_key_listeners, listener);
    }

    /// Remove a character listener for key events
    void remove_key_listener(character* listener)
    {
        remove_listener(m_key_listeners, listener);
    }

    /// Notify still loaded character listeners for mouse events
    DSOEXPORT void notify_mouse_listeners(const event_id& event);

    /// Push a new character listener for mouse events
    void add_mouse_listener(character* listener)
    {
        add_listener(m_mouse_listeners, listener);
    }

    /// Remove a character listener for mouse events
    void remove_mouse_listener(character* listener)
    {
        remove_listener(m_mouse_listeners, listener);
    }

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
    
    DSOEXPORT void add_invalidated_bounds(InvalidatedRanges& ranges,
            bool force);
    
    void dump_character_tree() const;

    /// Return the topmost active entity under the pointer
    //
    /// This method returns cached info, with cache updated
    /// by notify_mouse_moved (and should be updated also
    /// by movie advancement or actions execution maybe, not
    /// currently implmented).
    ///
    /// @return the topmost active entity under pointer or NULL if none.
    character* getActiveEntityUnderPointer() const;

    /// Return the topmost non-dragging entity under the pointer
    //
    /// This method triggers a displaylist scan
    ///
    /// @return the topmost non-dragging entity under pointer or NULL if none
    const character* getEntityUnderPointer() const;

    /// Return the character currently being dragged, if any
    character* getDraggingCharacter() const;

    /// Return true if the mouse pointer is over an active entity
    bool isMouseOverActiveEntity() const;

    bool testInvariant() const;

    /// The possible values of Stage.displayState
    enum DisplayState {
        normal,
        fullScreen
    };

    /// The possibile values of Stage.scaleMode
    enum ScaleMode {
        showAll,
        noScale,
        exactFit,
        noBorder
    };

    /// The possible horizonal positions of the Stage
    enum StageHorizontalAlign {
        STAGE_H_ALIGN_C,
        STAGE_H_ALIGN_L,
        STAGE_H_ALIGN_R,
    };

    /// The possible vertical position of the Stage
    enum StageVerticalAlign {
        STAGE_V_ALIGN_C,
        STAGE_V_ALIGN_T,       
        STAGE_V_ALIGN_B
    };

    /// The possible elements of a Stage.alignMode.
    enum AlignMode {
        STAGE_ALIGN_L,
        STAGE_ALIGN_T,
        STAGE_ALIGN_R,
        STAGE_ALIGN_B
    };

    /// Sets movie_root's horizontal and vertical alignment to one
    /// of the three possible positions for each dimension.
    void setStageAlignment(short s);

    typedef std::pair<StageHorizontalAlign, StageVerticalAlign> StageAlign;

    /// Returns the current alignment of the stage (left/right/centre, top/
    /// bottom/centre) as a std::pair
    StageAlign getStageAlignment() const;

    /// Sets the Stage object's align mode.
    void setStageScaleMode(ScaleMode sm);
    
    /// Returns the Stage object's align mode.
    ScaleMode getStageScaleMode() const { return _scaleMode; }

    // The string representation of the current align mode.
    std::string getStageAlignMode() const;

    /// Returns the Stage object's align mode.
    DisplayState getStageDisplayState() const { return _displayState; }

    // The string representation of the current align mode.
    void setStageDisplayState(const DisplayState ds);

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
    void pushAction(const action_buffer& buf,
            boost::intrusive_ptr<character> target, int lvl=apDOACTION);

    /// Push a function code to the ActionQueue
    void pushAction(boost::intrusive_ptr<as_function> func,
            boost::intrusive_ptr<character> target, int lvl=apDOACTION);

#ifdef GNASH_USE_GC
    /// Mark all reachable resources (for GC)
    //
    /// Resources reachable from movie_root are:
    ///
    /// - All _level# movies (_movies)
    /// - The original root movie (_rootMovie)
    /// - Mouse entities (m_mouse_button_state)
    /// - Timer targets (_intervalTimers)
    /// - Resources reachable by ActionQueue code (_actionQueue)
    /// - Key listeners (m_key_listeners)
    /// - Mouse listeners (m_mouse_listeners)
    /// - global Key object (_keyobject)
    /// - global Mouse object (_mouseobject)
    /// - Any character being dragged 
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
#if GNASH_PARANOIA_LEVEL > 1
        assert(std::find(_liveChars.begin(), _liveChars.end(), ch) ==
            _liveChars.end());
#endif
        _liveChars.push_front(ch);
    }

    /// Cleanup all resources and run the GC collector
    //
    /// This method should be invoked before calling setRootMovie again
    /// for a clean restart.
    ///
    void clear();

    /// Reset stage to its initial state
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

    character* findCharacterByTarget(const std::string& tgtstr) const;

    /// Queue a request for loading a movie
    //
    /// This function constructs the URL and, if required, the postdata
    /// from the arguments. The variables to send should *not* be appended
    /// to @param urlstr before calling this function.
    //
    /// @param urlstr   The url exactly as requested. This may already
    ///                 contain a query string.
    /// @param target   Target for request.
    /// @param data     The variables data to send, URL encoded in
    ///                 key/value pairs
    /// @param method   The VariablesMethod to use for sending the data. If
    ///                 MovieClip::METHOD_NONE, no data will be sent.
    void loadMovie(const std::string& url, const std::string& target,
            const std::string& data, MovieClip::VariablesMethod method);

    /// Send a request to the hosting application (e.g. browser).
    //
    /// This function constructs the URL and, if required, the postdata
    /// from the arguments. The variables to send should *not* be appended
    /// to @param urlstr before calling this function.
    //
    /// @param urlstr   The url exactly as requested. This may already
    ///                 contain a query string.
    /// @param target   Target for request.
    /// @param data     The variables data to send, URL encoded in
    ///                 key/value pairs
    /// @param method   The VariablesMethod to use for sending the data. If
    ///                 MovieClip::METHOD_NONE, no data will be sent.
    void getURL(const std::string& urlstr, const std::string& target,
            const std::string& data, MovieClip::VariablesMethod method);


    /// Return true if the given string can be interpreted as a _level name
    //
    /// @param name
    ///   The target string.
    ///   Will be considered case-insensitive if VM version is < 7.
    ///
    /// @param levelno
    ///   Output parameter, will be set to the level number, if true is
    ///   returned
    bool isLevelTarget(const std::string& name, unsigned int& levelno);


    /// Set a filedescriptor to use for host application requests
    /// (for browser communication mostly)
    void setHostFD(int fd)
    {
        assert(fd > 0);
        _hostfd = fd;
    }

    /// Get the filedescriptor to use for host application requests
    /// (for browser communication mostly)
    ///
    /// @return -1 if no filedescriptor is provided by host app.
    int getHostFD() const
    {
        return _hostfd;
    }

    /// Abstract base class for FS handlers
    class AbstractFsCallback {
    public:
        virtual void notify(const std::string& cmd, const std::string& arg)=0;
        virtual ~AbstractFsCallback() {}
    };

    /// ActionScript embedded in a movie can use the built-in
    /// fscommand() function to send data back to the host
    /// application.  If you are interested in this data, register
    /// a handler, which will be called when the embedded scripts
    /// call fscommand().
    ///
    /// The handler gets the MovieClip* that the script is
    /// embedded in, and the two string arguments passed by the
    /// script to fscommand().
    DSOEXPORT void registerFSCommandCallback(AbstractFsCallback* handler)
    {
        _fsCommandHandler = handler;
    }

    /// Call this to notify FS commands
    DSOEXPORT void handleFsCommand(const std::string& cmd,
            const std::string& arg) const;

    /// Abstract base class for hosting app handler
    class AbstractIfaceCallback {
    public:

        /// Get Gui-related information for the core.
        //
        /// This should be used for occasional AS calls, such as for
        /// Mouse.hide, System.capabilities etc. The return can be
        /// various types, so it is passed as a string.
        virtual std::string call(const std::string& cmd,
                const std::string& arg) = 0;

        /// Ask the hosting application for a yes / no answer to
        /// a question.
        virtual bool yesNo(const std::string& cmd) = 0;
        virtual ~AbstractIfaceCallback() {}
    };

    /// A callback to the GUI (or whatever is listening) for sending
    /// events and receiving replies. Used for ActionScript interface
    /// with the gui (Mouse visibility, Stage alignment etc and System
    /// information, for instance).
    ///
    /// See callInterface method
    DSOEXPORT void registerEventCallback(AbstractIfaceCallback* handler)
    {
        _interfaceHandler = handler;
    }

    /// Call into the hosting application
    ///
    /// Will use callback set with registerEventCallback
    DSOEXPORT std::string callInterface(const std::string& cmd,
            const std::string& arg) const;

    /// Called from the ScriptLimits tag parser to set the
    /// global script limits. It is expected behaviour that
    /// each new loaded movie should override this.
    /// Can be overridden from gnashrc.
    //
    /// @param recursion the maximum number of recursions when
    ///             finding 'super'.
    ///             The default value for this (i.e. when no
    ///             ScriptLimits tag is present) is documented to be
    ///             256, but this may change and appears not to be
    ///             crucial for (backward) compatibility.
    /// @param timeout the timeout in seconds for script execution.
    ///             The default value for this (i.e. when no
    ///             ScriptLimits tag is present) is documented to be
    ///             15 to 20 seconds, depending on platform.
    void setScriptLimits(boost::uint16_t recursion, boost::uint16_t timeout);
    
    /// Get the current global recursion limit for this movie: it can
    /// be changed by loaded movies.
    boost::uint16_t getRecursionLimit() const
    {
        return _recursionLimit;
    }

    /// Get the current global script timeout limit for this movie: it
    /// can be changed by loaded movies.
    boost::uint16_t getTimeoutLimit() const
    {
        return _timeoutLimit;
    }

#ifdef USE_SWFTREE
    typedef std::pair<std::string, std::string> StringPair;
    void getMovieInfo(tree<StringPair>& tr, tree<StringPair>::iterator it);
#endif

    /// Get the immutable base URL for this run.
    //
    /// @return The base URL set when movie_root was constructed.
    const std::string& getBaseURL() const { return _baseURL; }

	/// Get URL of the SWF movie used to initialize this VM
	//
	/// This information will be used for security checks
	///
	const std::string& getOriginalURL() const { return _originalURL; }

private:

    /// The base URL for this movie, which may be specified by the caller.
    //
    /// This is a runtime constant because it must not change during a 
    /// run.
    const std::string _baseURL;

    /// The URL of the original root movie.
    //
    /// This is a runtime constant because it must not change during a 
    /// run.
    const std::string _originalURL;

    /// This initializes a SharedObjectLibrary, which requires 
    /// _originalURL, so that must be initialized first.
    VM& _vm;

    /// Registered Interface command handler, if any
    AbstractIfaceCallback* _interfaceHandler;

    /// Registered FsCommand handler, if any
    AbstractFsCallback* _fsCommandHandler;

    /// A load movie request
    class LoadMovieRequest {
    public:
        /// @param postdata
        ///   If not null POST method will be used for HTTP.
        ///
        LoadMovieRequest(const URL& u, const std::string& t,
                const std::string* postdata)
                :
                _target(t),
                _url(u),
                _usePost(false)
        {
            if ( postdata )
            {
                _postData = *postdata;
                _usePost = true;
            }
        }

        const std::string& getTarget() const { return _target; }
        const URL& getURL() const { return _url; }
        const std::string& getPostData() const { return _postData; }
        bool usePost() const { return _usePost; }

    private:
        std::string _target;
        URL _url;
        bool _usePost;
        std::string _postData;
    };

    /// Load movie requests
    typedef std::list<LoadMovieRequest> LoadMovieRequests;
    LoadMovieRequests _loadMovieRequests;

    /// Process all load movie requests
    void processLoadMovieRequests();

    /// Process a single load movie request
    void processLoadMovieRequest(const LoadMovieRequest& r);

    /// Listeners container
    typedef std::list< boost::intrusive_ptr<character> > CharacterList;

    /// key and mouse listeners container
    typedef CharacterList KeyListeners;
    typedef CharacterList MouseListeners;

    /// Take care of dragging, if needed
    void doMouseDrag();

    /// Delete all elements on the action queue and empty it.
    void clearActionQueue();

    /// Delete all elements on the timers list
    void clearIntervalTimers();

    /// An element of the advanceable characters
    typedef boost::intrusive_ptr<character> AdvanceableCharacter;

    /// A list of AdvanceableCharacters
    //
    /// This is a list (not a vector) as we want to allow
    /// ::advance of each element to insert new characters before
    /// the start w/out invalidating iterators scanning the
    /// list forward for proper movie advancement
    typedef std::list<AdvanceableCharacter> LiveChars;

    /// The list of advanceable character, in placement order
    LiveChars _liveChars;

    /// Execute expired timers
    void executeTimers();

    /// Notify the global Key ActionScript object about a key status change
    Key_as * notify_global_key(key::code k, bool down);

    /// Remove unloaded key and mouselisteners.
    void cleanupUnloadedListeners()
    {
        cleanupUnloadedListeners(m_key_listeners);
        cleanupUnloadedListeners(m_mouse_listeners);
    }

    /// Erase unloaded characters from the given listeners list
    static void cleanupUnloadedListeners(CharacterList& ll);

    /// Cleanup references to unloaded characters and run the garbage collector.
    void cleanupAndCollect();

    /// Push a character listener to the front of given container, if not
    /// already present
    static void add_listener(CharacterList& ll, character* elem);

    /// Remove a listener from the list
    static void remove_listener(CharacterList& ll, character* elem);

    /// Return the current Stage object
    //
    /// Can return NULL if it's been deleted or not
    /// yet initialized.
    boost::intrusive_ptr<Stage_as> getStageObject();

    typedef std::list<ExecutableCode*> ActionQueue;

    ActionQueue _actionQueue[apSIZE];

    /// Process all actions in the queue
    void processActionQueue();

    // TODO: use Range2d<int> ?
    int m_viewport_x0, m_viewport_y0;

    /// Width and height of viewport, in pixels
    int m_viewport_width, m_viewport_height;

    rgba m_background_color;
    bool m_background_color_set;

    float m_timer;
    int m_mouse_x, m_mouse_y, m_mouse_buttons;

    MouseButtonState  m_mouse_button_state;

    typedef std::map<int, Timer*> TimerMap;

    TimerMap _intervalTimers;
    unsigned int _lastTimerId;

    /// Characters for listening key events
    KeyListeners m_key_listeners;

    boost::intrusive_ptr<Key_as> _keyobject;

    boost::intrusive_ptr<as_object> _mouseobject;

    /// Objects listening for mouse events (down,up,move)
    MouseListeners m_mouse_listeners;

    character*  m_active_input_text;
    float m_time_remainder;

    /// @todo fold this into m_mouse_button_state?
    drag_state m_drag_state;

    typedef boost::intrusive_ptr<MovieClip> LevelMovie;
    typedef std::map<int, LevelMovie> Levels;

    /// The movie instance wrapped by this movie_root
    //
    /// We keep a pointer to the base MovieClip class
    /// to avoid having to replicate all of the base class
    /// interface to the movie_instance class definition
    Levels _movies;

    /// The root movie. This is initially the same as getLevel(0) but might
    /// change during the run. It will be used to setup and retrive initial
    /// stage size
    boost::intrusive_ptr<movie_instance> _rootMovie;

    /// This function should return TRUE iff any action triggered
    /// by the event requires redraw, see \ref events_handling for
    /// more info.
    bool fire_mouse_event();

    bool generate_mouse_button_events();

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
    character* getTopmostMouseEntity(boost::int32_t x, boost::int32_t y);

    /// Delete characters removed from the stage
    /// from the display lists
    void cleanupDisplayList();

    /// Advance a live character
    //
    /// @param ch
    ///     The character to advance, will NOT be advanced if unloaded
    ///
    static void advanceLiveChar(boost::intrusive_ptr<character> ch);

    /// Advance all non-unloaded live chars
    void advanceLiveChars();

    /// Put the given movie at the given level 
    //
    /// @param movie
    /// The movie_instance to store at the given level.
    /// Will be stored in an intrusive_ptr.
    /// Its depth will be set to <num>+character::staticDepthOffset and
    /// its name to _level<num>
    void setLevel(unsigned int num, boost::intrusive_ptr<movie_instance> movie);

    /// Return the global Key object 
    //
    /// @@ might be worth making public
    ///
    boost::intrusive_ptr<Key_as> getKeyObject();

    /// Return the global Mouse object 
    //
    /// TODO: expose the Mouse as_object directly for faster calls ?
    ///
    boost::intrusive_ptr<as_object> getMouseObject();

    /// Boundaries of the Stage are always world boundaries
    /// and are only invalidated by changes in the background
    /// color.
    void setInvalidated() { _invalidated = true; }

    /// Every ::display call clears the invalidated flag
    //
    /// See setInvalidated();
    ///
    void clearInvalidated() { _invalidated = false; }

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

    const character* findDropTarget(boost::int32_t x, boost::int32_t y,
            character* dragging) const;

    /// filedescriptor to write to for host application requests
    //
    /// -1 if none
    int _hostfd;
    
    std::bitset<4u> _alignMode;
    
    ScaleMode _scaleMode;
    
    DisplayState _displayState;
    
    // The maximum number of recursions e.g. when finding
    // 'super', set in the ScriptLimits tag.
    boost::uint16_t _recursionLimit;

    // The timeout in seconds for script execution, in the
    // ScriptLimits tag.    
    boost::uint16_t _timeoutLimit;

    void handleActionLimitHit(const std::string& ref);

    // delay between movie advancement, in milliseconds
    unsigned int _movieAdvancementDelay;

    // time of last movie advancement, in milliseconds
    unsigned int _lastMovieAdvancement;
};

} // namespace gnash

#endif // GNASH_MOVIE_ROOT_H

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
