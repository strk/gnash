// MovieClip.h:  Stateful live Sprite instance, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

// Stateful live Sprite instance

#ifndef GNASH_MOVIECLIP_H
#define GNASH_MOVIECLIP_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h" 
#endif

#include <vector>
#include <map>
#include <string>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/intrusive_ptr.hpp>

#include "ControlTag.h"
#include "movie_definition.h" // for inlines
#include "DisplayList.h" // DisplayList 
#include "DisplayObjectContainer.h"
#include "as_environment.h" // for composition
#include "DynamicShape.h" // for composition
#include "dsodefs.h" // for DSOEXPORT

// Forward declarations
namespace gnash {
    class Movie;
    class swf_event;
    class drag_state;
    class LoadVariablesThread;
    class GradientRecord;
    class TextField;
    class BitmapData_as;
    class CachedBitmap;
    namespace SWF {
        class PlaceObject2Tag;
    }
}

namespace gnash {

/// A MovieClip is a container for DisplayObjects.
//
/// TODO: This class should inherit from Sprite
//
/// In AS3 is it distinguished from a Sprite by having a timeline, i.e.
/// more than one frame. In AS2, there is no Sprite class.
//
/// There are basically two types of MovieClip: dynamic and non-dynamic.
/// Dynamic clips are created using createEmptyMovieClip() or 
/// duplicateMovieClip(). Non-dynamic MovieClips are parsed from a SWF file.
/// The isDynamic() member function is the only way to tell the difference
/// (see following paragraph).
//
/// The presence of a definition (the _def member) reveals whether the
/// MovieClip was constructed with an immutable definition or not. MovieClips
/// created using createEmptyMovieClip() have no definition. MovieClips
/// constructed using duplicateMovieClip() have the same definition as the
/// duplicated clip. They are "dynamic", but may have a definition!
//
/// A MovieClip always has an _swf member. This is the top-level SWF 
/// (Movie) containing either the definition or the code from
/// which the MovieClip was created. The _url member and SWF version are
/// dependent on the _swf. Exports are also sought in this Movie.
class DSOTEXPORT MovieClip : public DisplayObjectContainer 
{
public:

    typedef std::vector<TextField*> TextFields;

    /// A container for textfields, indexed by their variable name
    typedef std::map<ObjectURI, TextFields, ObjectURI::LessThan>
        TextFieldIndex;

    typedef std::map<std::string, std::string> MovieVariables;

    typedef movie_definition::PlayList PlayList;

    enum PlayState
    {
        PLAYSTATE_PLAY,
        PLAYSTATE_STOP
    };

    /// Construct a MovieClip instance
    //
    /// @param def
    ///     Pointer to the movie_definition this object is an
    ///     instance of (may be a top-level movie or a sprite).
    ///     This may be 0 if there is no immutable definition.
    ///
    /// @param root
    /// The "relative" _swf of this sprite, which is the 
    /// instance of top-level sprite defined by the same
    /// SWF that also contained *this* sprite definition.
    /// Note that this can be *different* from the top-level
    /// movie accessible through the VM, in case this sprite
    /// was defined in an externally loaded movie.
    ///
    /// @param parent
    ///     Parent of the created instance in the display list.
    ///     May be 0 for top-level movies (_level#).
    MovieClip(as_object* object, const movie_definition* def,
            Movie* root, DisplayObject* parent);

    virtual ~MovieClip();

    // Return the originating SWF
    virtual Movie* get_root() const;

    virtual bool trackAsMenu();

    /// Queue event in the global action queue.
    //
    /// notifyEvent(id) will be called by execution of the queued
    /// action
    void queueEvent(const event_id& id, int lvl);

    void queueLoad();

    /// Return the _root ActionScript property of this sprite.
    //
    /// Relative or absolute is determined by the _lockroot property,
    /// see getLockRoot and setLockRoot. May return this.
    virtual MovieClip* getAsRoot();

    /// Get the composite bounds of all component drawing elements
    virtual SWFRect getBounds() const;

    // See dox in DisplayObject.h
    virtual bool pointInShape(std::int32_t x, std::int32_t y) const;

    // See dox in DisplayObject.h
    virtual bool pointInVisibleShape(std::int32_t x, std::int32_t y) const;

    /// return true if the given point is located in a(this) hitable sprite.
    ///
    /// all sprites except mouse-insensitive dynamic masks are hitable.
    /// _visible property is ignored for hitable DisplayObjects.
    virtual bool pointInHitableShape(std::int32_t x, std::int32_t y) const;

    /// Return 0-based index to current frame
    size_t get_current_frame() const
    {
        return _currentFrame;
    }

    size_t get_frame_count() const
    {
        return _def ? _def->get_frame_count() : 1;
    }

    /// Return number of completely loaded frames of this sprite/movie
    //
    /// Note: the number is also the last frame accessible (frames
    /// numberes are 1-based)
    ///
    size_t get_loaded_frames() const
    {
        return _def ? _def->get_loading_frame() : 1;
    }

    /// Return total number of bytes in the movie
    /// (not sprite!)
    size_t get_bytes_total() const
    {
        return isDynamic() ? 0 : _def->get_bytes_total();
    }

    /// Return number of loaded bytes in the movie
    /// (not sprite!)
    size_t get_bytes_loaded() const
    {
        return isDynamic() ? 0 : _def->get_bytes_loaded();
    }

    const SWFRect& get_frame_size() const
    {
        static const SWFRect r;
        return _def ? _def->get_frame_size() : r;
    }

    /// Stop or play the sprite.
    //
    /// If stopped, any stream sound associated with this sprite
    /// will also be stopped.
    ///
    DSOEXPORT void setPlayState(PlayState s);

    PlayState getPlayState() const { return _playState; }

    // delegates to movie_root (possibly wrong)
    void set_background_color(const rgba& color);

    /// Return true if we have any mouse event handlers.
    //
    /// NOTE: this function currently does not consider
    ///       general mouse event handlers MOUSE_MOVE, MOUSE
    virtual bool mouseEnabled() const;

    /// \brief
    /// Return the topmost entity that the given point
    /// covers that can receive mouse events.  NULL if
    /// none.  Coords are in parent's frame.
    virtual InteractiveObject* topmostMouseEntity(std::int32_t x,
            std::int32_t y);

    // see dox in DisplayObject.h
    const DisplayObject* findDropTarget(std::int32_t x, std::int32_t y,
            DisplayObject* dragging) const;

    void setDropTarget(const std::string& tgt) {
        _droptarget = tgt;
    }

    const std::string& getDropTarget() const {
        return _droptarget;
    }
    
    /// Advance to the next frame of the MovieClip.
    //
    /// Actions will be executed or pushed to the queue as necessary.
    virtual void advance();

    /// Set the sprite state at the specified frame number.
    //
    /// 0-based frame numbers!! 
    ///(in contrast to ActionScript and Flash MX)
    ///
    DSOEXPORT void goto_frame(size_t target_frame_number);

    /// Parse frame spec and return a 0-based frame number.
    //
    /// If frame spec cannot be converted to !NAN and !Infinity number
    /// it will be converted to a string and considered a
    /// frame label (returns false if referring to an
    /// unknwown label).
    ///
    /// @param frame_spec
    /// The frame specification.
    ///
    /// @param frameno
    /// The evaluated frame number (0-based)
    ///
    /// @return
    /// True if the frame_spec could be resolved to a frame number.
    /// False if the frame_spec was invalid.
    bool get_frame_number(const as_value& frame_spec, size_t& frameno) const;

    /// Look up the labeled frame, and jump to it.
    bool goto_labeled_frame(const std::string& label);
        
    /// Render this MovieClip.
    virtual void display(Renderer& renderer, const Transform& xform);
    
    /// Draw this MovieClip
    //
    /// This is effectively the same as display(), but uses only the passed
    /// transform.
    void draw(Renderer& renderer, const Transform& xform);

    void omit_display();

    /// Swap depth of the given DisplayObjects in the DisplayList
    //
    /// See DisplayList::swapDepths for more info
    void swapDepths(DisplayObject* ch1, int newdepth)
    {
        _displayList.swapDepths(ch1, newdepth);
    }

    /// Return the DisplayObject at given depth in our DisplayList.
    //
    /// @return NULL if the specified depth is available (no chars there)
    DisplayObject* getDisplayObjectAtDepth(int depth);

    /// Attach a DisplayObject at the specified depth.
    DisplayObject* addDisplayListObject(DisplayObject* obj, int depth);

    /// Place a DisplayObject or mask to the DisplayList.
    //
    /// This method instantiates the given DisplayObject definition
    /// and places it on the stage at the given depth.
    ///
    /// If the specified depth is already occupied, it results a no-ops.
    /// Otherwise, a new DisplayObject will be created and onload handler
    /// will be triggerred.
    ///
    /// @param tag
    ///     A swf defined placement tag (PlaceObject, or PlaceObject2,
    ///     or PlaceObject3).
    ///     No ownership transfer, the tag is still owned by the
    ///     movie_definition class.
    ///
    /// @param dlist
    ///     The display list to add the DisplayObject to.
    ///
    /// @return
    ///     A pointer to the DisplayObject being added or NULL
    DisplayObject* add_display_object(const SWF::PlaceObject2Tag* tag,
            DisplayList& dlist);

    /// Proxy of DisplayList::moveDisplayObject()
    void move_display_object(const SWF::PlaceObject2Tag* tag,
            DisplayList& dlist);

    /// Proxy of DisplayList::replaceDisplayObject()
    void replace_display_object(const SWF::PlaceObject2Tag* tag,
            DisplayList& dlist);

    /// Proxy of DisplayList::removeDisplayObject()
    void remove_display_object(const SWF::PlaceObject2Tag* tag,
            DisplayList& dlist);

    /// \brief
    /// Remove the object at the specified depth.
    //
    /// NOTE: 
    /// (1)the id parameter is currently unused, but 
    /// required to avoid breaking of inheritance from movie.h.
    /// (2)the id might be used for specifying a DisplayObject
    /// in the depth(think about multiple DisplayObjects within the same
    /// depth, not tested and a rare case)
    void remove_display_object(int depth, int /*id*/);

    void unloadMovie();

    /// Attach the given DisplayObject instance to current display list
    //
    /// @param newch    The DisplayObject instance to attach.
    /// @param depth    The depth to assign to the instance.
    void attachCharacter(DisplayObject& newch, int depth, as_object* initObject);

    /// Handle placement event
    //
    /// This callback will (not known to be a problem):
    ///
    /// (1) Register ourselves with the global instance list
    /// (2) Take note of our original target path
    /// (3) Register as listener of core broadcasters
    /// (4) Execute tags of frame 0
    ///
    /// The callback will also (known to be bogus):
    //
    /// (1) Construct this instance as an ActionScript object.
    ///     See constructAsScriptObject() method, including constructing
    ///     registered class and adding properties.
    virtual void construct(as_object* initObj = 0);

    /// Mark this sprite as destroyed
    //
    /// This is an override of DisplayObject::destroy()
    ///
    /// A sprite should be destroyed when is removed from the display
    /// list and is not more needed for names (target) resolutions.
    /// Sprites are needed for names resolution whenever themselves
    /// or a contained object has an onUnload event handler defined, 
    /// in which case we want the event handler to find the 'this'
    /// variable w/out attempting to rebind it.
    ///
    /// When a sprite is destroyed, all its children are also destroyed.
    /// 
    /// Note: this function will release most memory associated with
    /// the sprite as no members or drawable should be needed anymore.
    void destroy();
        
    /// Add the given action buffer to the list of action
    /// buffers to be processed at the end of the next
    /// frame advance.
    void add_action_buffer(const action_buffer* a)
    {
        if (!_callingFrameActions) queueAction(*a);
        else execute_action(*a);
    }

    
    /// \brief
    /// Execute the given init action buffer, if not done yet
    /// for the target DisplayObject id.
    //
    /// The action will normally be pushed on queue, but will
    /// be executed immediately if we are executing actions
    /// resulting from a callFame instead.
    ///
    /// @param a
    /// The action buffer to execute
    ///
    /// @param cid
    /// The referenced DisplayObject id
    void execute_init_action_buffer(const action_buffer& a, int cid);

    /// Execute a single action buffer (DOACTION block)
    void execute_action(const action_buffer& ab);

    MovieClip* to_movie () { return this; }

    /// The various methods for sending data in requests.
    //
    /// Used in loadMovie, getURL, loadVariables etc.
    enum VariablesMethod
    {
        METHOD_NONE = 0,
        METHOD_GET,
        METHOD_POST
    };

    // See dox in DisplayObject.h
    virtual void getLoadedMovie(Movie* newMovie);

    /// \brief
    /// Load url-encoded variables from the given url, optionally
    /// sending variables from this timeline too.
    //
    /// A LoadVariablesThread will be started to load and parse variables
    /// and added to the _loadVariableRequests. Then, at every ::advance_sprite
    /// any completed threads will be processed
    /// (see processCompletedLoadVariableRequests)
    ///
    /// NOTE: the given url will be security-checked
    ///
    /// @param urlstr: The url to load variables from.
    ///
    /// @param sendVarsMethod: The VariablesMethod to use. If METHOD_NONE,
    ///                        no data will be sent.
    void loadVariables(const std::string& urlstr,
            VariablesMethod sendVarsMethod);

    /// Get TextField variables
    //
    /// TODO: this is unlikely to be the best way of doing it, and it would
    /// simplify things if this function could be dropped.
    bool getTextFieldVariables(const ObjectURI& uri, as_value& val);

    // Set TextField variables
    //
    /// TODO: this is also unlikely to be the best way to do it.
    bool setTextFieldVariables(const ObjectURI& uri, const as_value& val);

    /// Search for a named object on the DisplayList
    //
    /// These are properties, but not attached as genuine members to the
    /// MovieClip object. They take priority over DisplayObject magic
    /// properties and inherited properties, but not over own properties.
    //
    /// @param name     Object identifier. This function handles
    ///                 case-sensitivity.
    /// @return         The object if found, otherwise 0.
    DisplayObject* getDisplayListObject(const ObjectURI& uri);

    /// Overridden to look in DisplayList for a match
    as_object* pathElement(const ObjectURI& uri);

    /// Execute the actions for the specified frame. 
    //
    /// The frame_spec could be an integer or a string.
    virtual void call_frame_actions(const as_value& frame_spec);

    /// Duplicate this sprite in its timeline
    //
    /// Add the new DisplayObject at a the given depth to this sprite
    /// parent displaylist.
    ///
    /// NOTE: the call will fail for the root movie (no parent).
    /// NOTE2: any DisplayObject at the given target depth will be
    ///        replaced by the new DisplayObject
    /// NOTE3: event handlers will also be copied
    ///
    /// @param newname
    ///     Name for the copy
    ///
    /// @param newdepth
    ///     Depth for the copy
    ///
    /// @param init_object
    ///     If not null, will be used to copy properties over.
    MovieClip* duplicateMovieClip(const std::string& newname,
        int newdepth, as_object* init_object = 0);

    /// Called when a mouse event affects this MovieClip
    virtual void mouseEvent(const event_id& id) {
        notifyEvent(id);
    }
        
    /// Dispatch event handler(s), if any.
    //
    /// This handles key, mouse, and specific MovieClip events.
    /// TODO: split this sensibly.
    void notifyEvent(const event_id& id);

    // inherited from DisplayObject class, see dox in DisplayObject.h
    virtual as_environment& get_environment() {
        return _environment;
    }

    /// \brief
    /// Set a TextField variable to this timeline
    //
    /// A TextField variable is a variable that acts
    /// as a setter/getter for a TextField 'text' member.
    void set_textfield_variable(const ObjectURI& name, TextField* ch);

    void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);
    
    const DisplayList& getDisplayList() const {
            return _displayList;
    }

    /// Return the next highest available depth
    //
    /// Placing an object at the depth returned by
    /// this function should result in a DisplayObject
    /// that is displayd above all others
    int getNextHighestDepth() const {
        return _displayList.getNextHighestDepth();
    }

    /// Set the currently playing m_sound_stream_id
    // 
    // TODO: rename to setStreamingSoundId
    void setStreamSoundId(int id);

    /// Remove this sprite from the stage.
    //
    /// This function is intended to be called by 
    /// effect of a removeMovieClip() ActionScript call
    /// and implements the checks required for this specific
    /// case.
    ///
    /// Callers are:
    /// - The ActionRemoveClip tag handler.
    /// - The global removeMovieClip(target) function.
    /// - The MovieClip.removeMovieClip() method.
    ///
    /// The removal will not occur if the depth of this
    /// DisplayObjects is not in the "dynamic" range [0..1048575]
    /// as described at the following URL:
    /// 
    /// http://www.senocular.com/flash/tutorials/depths/?page=2
    ///
    /// A testcases for this behaviour can be found in 
    ///
    /// testsuite/misc-ming.all/displaylist_depths_test.swf
    void removeMovieClip();

    /// Direct access to the Graphics object for drawing.
    DynamicShape& graphics() {
        set_invalidated();
        return _drawable;
    }

    /// Set focus to this MovieClip
    //
    /// @return true if this MovieClip can receive focus.
    virtual bool handleFocus();

    /// @} Drawing API

    /// Set all variables in the given map with their corresponding values
    DSOEXPORT void setVariables(const MovieVariables& vars);

    /// Enumerate child DisplayObjects
    //
    /// See DisplayObject::enumerateNonProperties for more info.
    virtual void visitNonProperties(KeyVisitor& v) const;

    /// Delete DisplayObjects removed from the stage
    /// from the display lists
    void cleanupDisplayList();

    /// Queue the given action buffer
    //
    /// The action will be pushed on the current
    /// global list (see movie_root).
    ///
    void queueAction(const action_buffer& buf);

    /// Construct this instance as an ActionScript object
    //
    /// This method invokes the constructor associated with our
    /// definition, either MovieClip or any user-speficied one
    /// (see sprite_definition::registerClass). 
    /// It will also invoke the onClipConstruct and onConstruct handlers.
    void constructAsScriptObject();

    /// Return true if getAsRoot() should return the *relative* root,
    /// false otherwise.
    bool getLockRoot() const { return _lockroot; }

    /// Set whether getAsRoot() should return the *relative* root,
    /// false otherwise. True for relative root.
    void setLockRoot(bool lr) { _lockroot=lr; }

    /// Return the version of the SWF this MovieClip was parsed from.
    virtual int getDefinitionVersion() const;

protected:

    /// Unload all contents in the displaylist and this instance
    //
    /// Return true if there was an unloadHandler.
    virtual bool unloadChildren();

    /// Mark sprite-specific reachable resources.
    //
    /// sprite-specific reachable resources are:
    ///     - DisplayList items (current, backup and frame0 ones)
    /// - Canvas for dynamic drawing (_drawable)
    /// - sprite environment
    /// - definition the sprite has been instantiated from
    /// - Textfields having an associated variable registered in this instance.
    /// - Relative root of this instance (_swf)
    ///
    virtual void markOwnResources() const;
    
    // Used by BitmapMovie.
    void placeDisplayObject(DisplayObject* ch, int depth) {       
        _displayList.placeDisplayObject(ch, depth);  
    }

private:

    /// Process any completed loadVariables request
    void processCompletedLoadVariableRequests();

    /// Process a completed loadVariables request
    void processCompletedLoadVariableRequest(LoadVariablesThread& request);

    
    /// Execute the tags associated with the specified frame.
    //
    /// @param frame
    ///     Frame number. 0-based
    ///
    /// @param dlist
    ///     The display list to have control tags act upon.
    ///
    /// @param typeflags
    ///     Which kind of control tags we want to execute. 
    void executeFrameTags(size_t frame, DisplayList& dlist,
            int typeflags = SWF::ControlTag::TAG_DLIST |
                            SWF::ControlTag::TAG_ACTION);

    void stopStreamSound();

    /// Return value of the 'enabled' property cast to a boolean value.
    //
    /// This is true if not found (undefined to bool evaluates to false).
    //
    /// When a MovieClip is "disabled", its handlers of button-like events 
    /// are disabled, and automatic tab ordering won't include it.
    bool isEnabled() const;

    /// Check whether a point hits our drawable shape.
    //
    /// This is possible because the drawable does not have its own
    /// transform, so we can use our own. The points are expressed in
    /// world space.
    bool hitTestDrawable(std::int32_t x, std::int32_t y) const;

    /// Advance to a previous frame.
    //
    /// This function will basically restore the DisplayList as it supposedly
    /// was *before* executing tags in target frame and then execute target
    /// frame tags (both DLIST and ACTION ones).
    ///
    /// In practice, it will:
    ///
    /// - Remove from current DisplayList:
    /// - Timeline instances constructed after target frame 
    /// - Timeline instances constructed before or at the target frame but no
    ///   more at the original depth
    /// - Dynamic instances found in the static depth zone
    /// - Execute all displaylist tags from first to one-before target frame,
    ///   appropriately setting _currentFrame as it goes, finally execute
    ///   both displaylist and action
    ///   tags for target frame.
    ///
    /// Callers of this methods are:
    /// - goto_frame (for jump-backs)
    /// - advance_sprite (for loop-back)
    ///
    /// See:
    //  http://www.gnashdev.org/wiki/index.php/TimelineControl
    ///              #Timeline_instances
    ///
    /// @param targetFrame
    /// The target frame for which we're willing to restore the static
    /// DisplayList.
    /// 0-based.
    //
    /// POSTCONDITIONS:
    ///
    /// - _currentFrame == targetFrame
    ///
    /// TODO: consider using this same function for jump-forward too,
    ///       with some modifications...
    ///
    void restoreDisplayList(size_t targetFrame);

    /// Increment _currentFrame, and take care of looping.
    void increment_frame_and_check_for_loop();

    /// Unregister textfield variables bound to unloaded TextFields
    void cleanup_textfield_variables();

    /// This is either sprite_definition (for sprites defined by
    /// DefineSprite tag) or movie_def_impl (for the top-level movie).
    const boost::intrusive_ptr<const movie_definition> _def;

    /// List of loadVariables requests
    typedef boost::ptr_list<LoadVariablesThread> LoadVariablesThreads;
    
    /// List of active loadVariable requests 
    //
    /// At ::advance_sprite time, all completed requests will
    /// be processed (variables imported in this timeline scope)
    /// and removed from the list.
    LoadVariablesThreads _loadVariableRequests;

    /// The SWF that this MovieClip belongs to.
    Movie* _swf;

    /// The canvas for dynamic drawing
    DynamicShape _drawable;

    PlayState _playState;

    /// This timeline's variable scope
    as_environment _environment;

    /// We'll only allocate Textfield variables map if
    /// we need them (ie: anyone calls set_textfield_variable)
    ///
    std::unique_ptr<TextFieldIndex> _text_variables;

    std::string _droptarget;

    // 0-based index to current frame
    size_t _currentFrame;
    
    /// soundid for current playing stream. If no stream set to -1
    int m_sound_stream_id;

    // true if this sprite reached the last frame and restarted
    bool _hasLooped;

    // true if orphaned tags (tags found after last advertised showframe)
    // have been executed at least once.
    bool _flushedOrphanedTags;

    // true is we're calling frame actions
    bool _callingFrameActions;

    bool _lockroot;

    bool _onLoadCalled;
};

} // end of namespace gnash

#endif // GNASH_SPRITE_INSTANCE_H
