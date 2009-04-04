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


#ifndef GNASH_CHARACTER_H
#define GNASH_CHARACTER_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h" // USE_SWFTREE
#endif

#include "character_def.h"
#include "smart_ptr.h" // GNASH_USE_GC
#include "event_id.h" // for inlines
#include "as_object.h" // for inheritance
#include "rect.h" // for composition (invalidated bounds)
#include "SWFMatrix.h" // for composition
#include "cxform.h" // for composition
#include "log.h"
#include "snappingrange.h"
#include "Range2d.h"
#ifdef USE_SWFTREE
# include "tree.hh"
#endif

#include <map>
#include <string>
#include <cassert>
#include <boost/cstdint.hpp> // For C99 int types

//#define DEBUG_SET_INVALIDATED 1

// Forward declarations
namespace gnash {
    class MovieClip;
    class movie_instance;
    class ExecutableCode;
    class action_buffer;
    class movie_definition;
    class StaticText;
    class InteractiveDisplayObject;
    namespace SWF {
        class TextRecord;
    }
}

namespace gnash {

/// DisplayObject is the base class for all DisplayList objects.
//
/// It represents a single active element in a movie. This class does not
/// provide any interactivity.
//
/// Derived classes include InteractiveDisplayObject, StaticText, Bitmap,
/// Video, and Shape.
class DisplayObject : public as_object
{

public:

    DisplayObject(DisplayObject* parent, int id);

    virtual ~DisplayObject() {}

    /// The lowest placeable and accessible depth for a DisplayObject.
    /// Macromedia Flash help says: depth starts at -16383 (0x3FFF)
    ///
    /// See: http://www.senocular.com/flash/tutorials/depths/?page=2
    //
    /// See also http://www.kirupa.com/developer/actionscript/depths2.htm
    //
    /// The only way to exceed these bounds is with createEmptyMoveClip(),
    /// which can be placed at any depth within +/- 2**31.
    static const int lowerAccessibleBound = -16384;
    
    /// This is the maximum depth a MovieClip DisplayObject can be placed
    /// at (attachMovie). Kirupa (see above) says 2130690045, but this
    /// seems not to be included in the range.
    static const int upperAccessibleBound = 2130690044;

    /// This is the amount added to displaylist tag defined depths.
    /// DisplayObjects placed by tags (vs. DisplayObjects instantiated by
    /// ActionScript) always have negative depths by effect of this offset.
    static const int staticDepthOffset = lowerAccessibleBound;

    /// This is the offset at which DisplayObject's depth is
    /// shifted when a DisplayObject is removed from stage but
    /// an onUnload event handler is defined.
    ///
    /// Example: a DisplayObject at depth 60 gets moved to
    ///          depth -32829 (-32769-60) when unloaded and
    ///          an onUnload event handler is defined for it
    ///          or any of its childs.
    ///
    /// So, to recap:
    ///   1:  -32769 to -16385 are removed
    ///   2:  -16384 to      0 are statics
    ///   3:  Max depth for a PlaceoObject call is 16384 (which becomes 
    ///       0 in the statics)
    /// (all of the above correct?)
    static const int removedDepthOffset = -32769; 

    /// Return true if the given depth is in the removed zone
    static bool depthInRemovedZone(int depth)
    {
        return depth < staticDepthOffset;
    }

    /// Return true if this DisplayObject's depth is in the removed zone
    bool depthInRemovedZone()
    {
        return depthInRemovedZone(get_depth());
    }
    
    /// This value is used for m_clip_depth when 
    /// the DisplayObject is not a layer mask.
    //
    /// Depths below -16384 are illegal, so this
    /// value should not collide with real depths.  
    ///
    static const int noClipDepthValue = -1000000;

    /// Return a reference to the variable scope of this DisplayObject.
    //
    /// TODO: make const/return const& ?
    ///
    virtual as_environment& get_environment() {
        // MovieClip must override this
        // and any other DisplayObject will have
        // a parent!
        assert(m_parent != NULL);
        return m_parent->get_environment();
    }

    // Accessors for basic display info.
    int get_id() const { return m_id; }

    /// \brief
    /// Return the parent of this DisplayObject, or NULL if
    /// the DisplayObject has no parent.
    DisplayObject* get_parent() const
    {
        return m_parent.get();
    }

    /// for extern movie
    void set_parent(DisplayObject* parent)
    {
        assert(_origTarget.empty());
        m_parent = parent;
    }

    int get_depth() const { return m_depth; }

    void  set_depth(int d) { m_depth = d; }

    /// Get sound volume for this DisplayObject
    int getVolume() const { return _volume; }

    /// Set sound volume for this DisplayObject
    void setVolume(int vol) { _volume=vol; }

    /// Get concatenated sound volume for this DisplayObject
    //
    /// NOTE: the concatenated volume does NOT include
    ///       global volume settings, which is the one
    ///       controlled by Sound instances created passing
    ///       null, undefined or no argument to constructor.
    ///
    int getWorldVolume() const;

    /// Get local transform SWFMatrix for this DisplayObject
    const SWFMatrix& getMatrix() const { return m_matrix; }

    /// Set local transform SWFMatrix for this DisplayObject
    //
    /// @param m the new SWFMatrix to assign to this DisplayObject
    ///
    /// @param updateCache if true, updates the cache values
    ///        from the SWFMatrix (only if SWFMatrix != current SWFMatrix)
    ///
    void setMatrix(const SWFMatrix& m, bool updateCache=false);

    /// Set the xscale value of current SWFMatrix
    //
    /// This is used when setting _xscale.
    /// See xscale_getset.
    ///
    /// @param factor scale factor, in percent
    ///
    void set_x_scale(double factor);

    /// Copy SWFMatrix and caches from given DisplayObject
    void copyMatrix(const DisplayObject& ch);

    /// Set the yscale value of current SWFMatrix
    //
    /// This is used when setting _yscale 
    /// See yscale_getset. 
    ///
    /// @param factor scale factor, in percent
    ///
    void set_y_scale(double factor);

    /// Set the rotation value of current SWFMatrix
    //
    ///
    /// This is used when setting _rotation
    /// See rotation_getset 
    ///
    /// @param rot rotation in degrees. will be trimmed to
    ///        the -180 .. 180 range, can be passed outside it.
    ///
    void set_rotation(double rot);

    /// Set the width of this DisplayObject, modifying its SWFMatrix
    //
    /// This is used when setting _width
    /// See width_getset
    ///
    /// @param w new width, in TWIPS. TODO: should this be an integer ?
    ///
    void set_width(double width);

    /// Set the height of this DisplayObject, modifying its SWFMatrix
    //
    /// This is used when setting _height
    /// See height_getset
    ///
    /// @param h new height, in TWIPS. TODO: should this be an integer ?
    ///
    void set_height(double height);

    const cxform& get_cxform() const { return m_color_transform; }

    void  set_cxform(const cxform& cx) 
    {       
        if (cx != m_color_transform) {
            set_invalidated(__FILE__, __LINE__);
            m_color_transform = cx;
        }
    }

    void concatenate_cxform(const cxform& cx) {
        m_color_transform.concatenate(cx);
    }

    void concatenateMatrix(const SWFMatrix& m) { m_matrix.concatenate(m); }

    int get_ratio() const { return m_ratio; }

    void set_ratio(int r)
    {
        if (r != m_ratio) set_invalidated(__FILE__, __LINE__); 
        m_ratio = r;       
    }

    /// Returns the clipping depth (if any) of this DisplayObject.
    /// The parameter tells us to use the DisplayObject as a mask for
    /// all the objects contained in the display list from m_depth
    /// to m_clipping_depth inclusive.
    /// 
    /// The value returned by get_clip_depth() is only valid when isMaskLayer()
    /// returns true!
    ///  
    int get_clip_depth() const { return m_clip_depth; }

    /// See get_clip_depth()
    void set_clip_depth(int d)
    {
        m_clip_depth = d;
    }
        
    /// Returns true when the DisplayObject (and it's childs) is used as a mask
    /// for other DisplayObjects at higher depth (up to get_clip_depth).
    /// isMaskLayer() does *not* return true when one of it's
    /// parents is a mask and the DisplayObject itself is not.
    ///
    /// See also isDynamicMask() and isMask()
    ///     
    bool isMaskLayer() const
    {
        return (m_clip_depth != noClipDepthValue && !_maskee);
    }

    /// Returns true when the DisplayObject (and it's childs) is used as a mask
    /// for another DisplayObject.
    /// isDynamicMask() does *not* return true when one of it's
    /// parents is a mask and the DisplayObject itself is not.
    ///
    /// NOTE: there's no way to obtain the maskee from a dynamic mask
    ///
    /// See also isMaskLayer() and isMask()
    ///     
    bool isDynamicMask() const
    {
        return _maskee;
    }

    DisplayObject* toDisplayObject() { return this; }

    /// Return the DisplayObject masking this instance (if any)
    DisplayObject* getMask() const
    {
        if ( ! _mask ) return NULL;
        if ( _mask->_maskee != this )
        {
            // TODO: fix this !
            log_error("Our mask maskee is not us");
            return NULL; // for correctness;
        }
        return _mask;
    }

    /// Register a DisplayObject as a mask for this instance.
    ///
    /// @param mask The DisplayObject to use as a mask, possibly NULL.
    /// A reference to us will be registered with the mask, if
    /// not null, so it'll know it's a mask for us, and would stop
    /// being a mask for anything else.
    ///
    void setMask(DisplayObject* mask);

    /// Returns true if this DisplayObject is a mask (either layer or
    /// dynamic mask)
    bool isMask() const
    {
        return isDynamicMask() || isMaskLayer();
    }

    /// Set DisplayObject name, initializing the original target member
    void set_name(const std::string& name)
    {
        _name = name;
    }

    const std::string& get_name() const { return _name; }

    /// \brief
    /// Get our concatenated SWFMatrix (all our ancestor transforms,
    /// times our SWFMatrix). 
    ///
    /// Maps from our local space into "world" space
    /// (i.e. root movie space).
    //
    /// @param includeRoot      Whether the transform of the Stage (_root)
    ///                         should be concatenated. This is required to be
    ///                         false for pointInBounds.
    DSOEXPORT SWFMatrix getWorldMatrix(bool includeRoot = true) const;

    /// \brief
    /// Get our concatenated color transform (all our ancestor transforms,
    /// times our cxform). 
    ///
    /// Maps from our local space into normal color space.
    virtual cxform get_world_cxform() const;

    /// Get the built-in function handlers code for the given event
    //
    /// NOTE: this function is only for getting statically-defined
    ///       event handlers, which are the ones attached to a DisplayObject
    ///       with a PlaceObject2. It's the DisplayObject's responsibility
    ///       to properly fetch any user-defined event handler, which 
    ///       are the ones attached to a DisplayObject with ActionScript code.
    ///
    std::auto_ptr<ExecutableCode> get_event_handler(const event_id& id) const;

    /// Set a built-in function handler for the given event
    //
    /// Mark the DisplayObject as having mouse or Key event
    /// handlers if this is the case.
    ///
    /// NOTE: this function is only for registering statically-defined
    ///       event handlers, which are the ones attached to a DisplayObject
    ///       with a PlaceObject2. It's the DisplayObject's responsibility
    ///       to properly invoke any user-defined event handler, which 
    ///       are the ones attached to a DisplayObject with ActionScript code.
    ///
    /// @param id
    /// The event triggering the handler.
    ///
    /// @param code
    /// An action buffer to execute when given event is triggered.
    /// The buffer is externally owned (not copied), make sure it
    /// is kept alive for the whole lifetime of this DisplayObject.
    ///
    void add_event_handler(const event_id& id, const action_buffer& code);

    /// Render the DisplayObject.
    //
    /// All DisplayObjects must have a display() function.
	virtual void display() = 0;

    /// Search for StaticText objects
    //
    /// If this is a StaticText object and contains SWF::TextRecords, these
    /// are written to the passed parameter.
    /// @ return    0 if this object is not a StaticText or contains no text.
    virtual StaticText* getStaticText(std::vector<const SWF::TextRecord*>&,
            size_t&) {
        return 0;
    }

    /// Returns local, untransformed height of this DisplayObject in TWIPS
    //
    /// Use getBounds() if you need more then simply the height.
    ///
    boost::int32_t get_height() const
    {
        return getBounds().height();
    }

    /// Returns local, untransformed width of this DisplayObject in TWIPS
    //
    /// Use getBounds() if you need more then simply the width.
    ///
    boost::int32_t get_width() const
    {
        return getBounds().width();
    }

	virtual rect getBounds() const = 0;

    /// Return true if the given point falls in this DisplayObject's bounds
    //
    /// @param x        Point x coordinate in world space
    /// @param y        Point y coordinate in world space
    /// @return         Whether (x, y) is within the DisplayObject's bounds.
    ///                 This ignores _root's transform. 
    bool pointInBounds(boost::int32_t x, boost::int32_t y) const
    {
        rect bounds = getBounds();
        SWFMatrix wm = getWorldMatrix(false);
        wm.transform(bounds);
        return bounds.point_test(x, y);
    }

	virtual bool pointInShape(boost::int32_t  x, boost::int32_t  y) const = 0;

    /// true if the given point falls in this DisplayObject's visible shape
    //
    /// Point coordinates are in world TWIPS
    ///
    /// The default implementation returns false if the DisplayObject is
    /// not visible, calling pointInShape() otherwise.
    ///
    /// Note that this is good for simple DisplayObjects but needs
    /// to be overridden for DisplayObjects with childs. When a
    /// DisplayObject has childs it must take into account the case
    /// in which some childs are visible and some are not.
    ///
    virtual bool pointInVisibleShape(boost::int32_t x, boost::int32_t y) const
    {
        if ( ! isVisible() ) return false;
        if ( isMask() ) return false;
        return pointInShape(x, y);
    }

    /// Return the relative root of this DisplayObject
    //
    /// The "relative" is the movie_instance created by
    /// the same SWF definition that contained the
    /// definition of this DisplayObject.
    ///
    /// The default implementation is to invoke get_root
    /// against this DisplayObject's parent.
    ///
    virtual movie_instance* get_root() const {
        return get_parent()->get_root();
    }

    /// Return the _root ActionScript property of this DisplayObject.
    //
    /// By default calls get_root().
    ///
    virtual const MovieClip* getAsRoot() const;

    /// Find the object which is one degree removed from us,
    /// given the relative pathname.
    ///
    /// If the pathname is "..", then return our parent.
    /// If the pathname is ".", then return ourself.    If
    /// the pathname is "_level0" or "_root", then return
    /// the root movie.
    ///
    /// Otherwise, the name should refer to one our our
    /// named DisplayObjects, so we return it.
    ///
    /// NOTE: In ActionScript 2.0, top level names (like
    /// "_root" and "_level0") are CASE SENSITIVE.
    /// Character names in a display list are CASE
    /// SENSITIVE. Member names are CASE INSENSITIVE.    Gah.
    ///
    /// In ActionScript 1.0, everything seems to be CASE
    /// INSENSITIVE.
    ///
    virtual as_object* get_path_element(string_table::key key)
    {
        return getPathElementSeparator(key);
    }

    /// Restart the DisplayObject
    //
    /// This is only meaningful for sprite instances, but default
    /// it's a no-op.
    ///
    /// It is needed by Button
    /// TODO: have Button cast to_movie()
    ///             and drop this one
    virtual void restart() { }

    /// Advance this DisplayObject to next frame.
    //
    /// Character advancement is only meaningful for sprites
    /// and sprite containers (button DisplayObjects) because
    /// sprites are the only DisplayObjects that have frames.
    /// 
    /// Frame advancement include execution of all control tags.
    /// 
    virtual void advance()
    {
        // GNASH_REPORT_FUNCTION 
    }

    /// \brief
    /// Return true if PlaceObjects tag are allowed to move
    /// this DisplayObject.
    //
    /// Once a DisplayObject has been transformed by ActionScript,
    /// further transformation trought non-action SWF constrol tags
    /// is not allowed.
    ///
    /// See scriptTransformed()
    ///
    bool get_accept_anim_moves() const
    {
        return ! _scriptTransformed && ! _dynamicallyCreated;
    }

    /// Was this DisplayObject dynamically created ?
    //
    /// "Dynamically created" means created trough ActionScript.
    ///
    /// NOTE, With current code:
    /// - Characters created by means of a loadMovie are 
    ///     NOT set as dynamic (should check if they should)
    /// - Characters created by attachMovie ARE dynamic
    /// - Characters created by duplicateMovieClip ARE dynamic
    /// - Characters created by createEmptyMovieClip ARE dynamic
    /// - Characters created by new Video ARE dynamic
    /// - Characters created by createTextField ARE dynamic
    ///
    ///
    bool isDynamic() const {
        return _dynamicallyCreated;
    }

    /// Mark this DisplayObject as dynamically created
    void setDynamic() {
        _dynamicallyCreated = true;
    }

    /// \brief
    /// Call this function when the sprite has been
    /// transformed due to ActionScript code.
    //
    /// This information will be used while executing
    /// PlaceObject tags in that ActionScript-transformed
    /// DisplayObjects won't be allowed to be moved.
    ///
    /// TODO: make protected
    ///
    void transformedByScript() 
    {
        _scriptTransformed = true;
    }

    /// Set whether this DisplayObject should be rendered
    //
    /// TODO: handle all visible getter/setters in DisplayObject, not in 
    /// subclasses, and drop this / make it private.
    void set_visible(bool visible);

    // Return true if this DisplayObject should be rendered
    bool isVisible() const { return _visible; }

    /// ActionScript event handler.    Returns true if a handler was called.
    //
    /// Must be overridden or will always return false.
    ///
    virtual bool on_event(const event_id& /* id */)
    {
        return false;
    }

    /// Queue event in the global action queue.
    //
    /// on_event(id) will be called by execution of the queued
    /// action
    ///
    void queueEvent(const event_id& id, int lvl);

    /// Return true if an handler for the given event is defined
    //
    /// NOTE that we look for both clip-defined and user-defined
    /// handlers, which is likely error prone since we're doing
    /// this in a non-virtual function. Main use for this method
    /// is for being called by ::unload() to verify an Unload handler
    /// is available.
    ///
    bool hasEventHandler(const event_id& id) const;

	/// DisplayObjects are not a mouse entity by default.
    //
    /// Override this function for InteractiveDisplayObjects.
	virtual InteractiveDisplayObject* topmostMouseEntity(boost::int32_t, 
            boost::int32_t) {
        return 0;
    }
	
    /// Find highest depth DisplayObject whose shape contains the given
    /// point and is not the DisplayObject being dragged or any of its childs.
    //
    /// Point coordinates in global twips.
    ///
    virtual const DisplayObject* findDropTarget(boost::int32_t x, 
            boost::int32_t y, DisplayObject* dragging) const
    {
        if (this != dragging && isVisible() && pointInVisibleShape(x, y)) {
            return this;
        }
        
        return 0;
    }

    /// Returns true when the object (type) should get a instance name even 
    /// if none is provided manually.
    virtual bool wantsInstanceName() const
    {
        return false; 
    }

    /// Returns true when the object (type) can be referenced by ActionScipt
    bool isActionScriptReferenceable() const
    {
        // The way around
        // [ wantsInstanceName() returning isActionScriptReferenceable() ]
        // would be cleaner, but I wouldn't want to touch all files now.
        return wantsInstanceName();
    }

    /// Returns the closest as-referenceable ancestor
    DisplayObject* getClosestASReferenceableAncestor() 
    {
        if ( isActionScriptReferenceable() ) return this;
        assert(m_parent);
        return m_parent->getClosestASReferenceableAncestor();
    }

    const DisplayObject* getClosestASReferenceableAncestor() const
    {
        DisplayObject* nonconst_this = const_cast<DisplayObject*>(this);
        return nonconst_this->getClosestASReferenceableAncestor();
    }

    /// @}

    /// \brief
    /// This function marks the DisplayObject as being modified in aspect
    /// and keeps track of current invalidated bounds the first time
    /// it's called after each call to clear_invalidated().
    //
    /// Call this function *before* any change in this DisplayObject
    /// that modifies its rendering. This information will be used
    /// to detect visual changes that need to be redrawn.
    ///
    /// It is *important* to call this function *before* the change
    /// rather then after as it will also take care of updating the
    /// previously invalidated bounds (m_old_invalidated_bounds)
    ///
    /// Calling this function multiple time is a no-op, unless
    /// clear_invalidated() is called in between.
    ///
    /// NOTE: Marking a DisplayObject as invalidated automatically marks
    ///             it's parent as being invalidated.
    ///
    /// @see \ref region_update
    ///
    void set_invalidated();
    void set_invalidated(const char* debug_file, int debug_line);
    
    
    /// Calls set_invalidated() and extends old_invalidated_ranges to the
    /// given value so that also this area gets re-rendered (used when
    /// replacing DisplayObjects).    
    void extend_invalidated_bounds(const InvalidatedRanges& ranges);
    
    
    /// Called by a child to signalize it has changed visibily. The
    /// difference to set_invalidated() is that *this* DisplayObject does
    /// not need to redraw itself completely. This function will 
    /// recursively inform all it's parents of the change.
    void set_child_invalidated();
    

    /// Clear invalidated flag and reset m_old_invalidated_bounds to null.
    ///
    /// It is very important that each DisplayObject with any m_XXXX_invalidated
    /// flag set calls clear_invalidated() during the rendering of one frame. 
    /// Basically this means each call to display() must match a call to 
    /// clear_invalidated. This includes no-op display() calls, i.e. when the
    /// DisplayObject is outside of the screen. The DisplayList must still call
    /// clear_invalidated() even if display() is not necessary.
    ///
    /// Not doing so will result in a stale invalidated flag which in turn will
    /// prevent the parent to be informed when this DisplayObject (or a child) is
    /// invalidated again (see set_invalidated() recursion).
    ///    
    void clear_invalidated() {
        m_invalidated = false;
        m_child_invalidated = false;        
        m_old_invalidated_ranges.setNull();
    }
    
    /// \brief
    /// Add the DisplayObject's invalidated bounds *to* the given ranges list.
    //
    /// NOTE that this method should include the bounds that it
    /// covered the last time clear_invalidated() was called,
    /// as those need to be rerendered as well (to clear the region
    /// previously occupied by this DisplayObject).
    ///
    /// That's why it returns the *union* of old_invalidated_ranges and
    /// the current bounds. The function is also used internally by 
    /// set_invalidated() to update m_old_invalidated_ranges itself (you may 
    /// notice some kind of circular reference), but that's no problem since 
    /// old_invalidated_ranges is NULL during that call. 
    ///
    /// It is used to determine what area needs to be re-rendered.
    /// The coordinates are world coordinates (in TWIPS).
    /// Only instances with m_invalidated flag set are checked unless
    /// force is set.
    ///
    virtual void add_invalidated_bounds(InvalidatedRanges& ranges, 
            bool force);

    /// Called instead of display() when the DisplayObject is not visible on stage.
    /// Used to clear the invalidated flags.
    virtual void omit_display() { clear_invalidated(); }; 
    
    /// Callback invoked whenever a DisplayObject is placed on stage
    //
    /// This function must be called when the DisplayObject is placed on
    /// stage for the first time.
    ///
    /// The DisplayObject version of this call sets the original target
    /// of the DisplayObject, for soft references to work.
    /// If you override the method remember to call saveOriginalTarget()
    /// as the first thing.
    ///
    virtual void stagePlacementCallback(as_object* = 0)
    {
        saveOriginalTarget();
    }

    /// Unload this instance from the stage.
    //
    /// This function must be called when the DisplayObject is removed
    /// from the stage.
    /// It will take care of properly calling
    /// unload against any child DisplayObjects and queuing the
    /// 'UNLOAD' event handler.
    ///
    /// @return true if any onUnload event handler was defined
    ///                 by either this or any child DisplayObjects, false
    ///                 otherwise.
    ///
    virtual bool unload();

    /// Return true if this DisplayObject was unloaded from the stage
    bool isUnloaded() { return _unloaded; }

    /// Mark this DisplayObject as destroyed
    //
    /// A DisplayObject should be destroyed when is removed from the display
    /// list and is not more needed for names (target) resolutions.
    /// Sprites are needed for names resolution whenever themselves
    /// or a contained object has an onUnload event handler defined, 
    /// in which case we want the event handler to find the 'this'
    /// variable w/out attempting to rebind it.
    ///
    /// Note: this function can safely release most memory associated
    ///             with the DisplayObject as it will not be needed anymore.
    ///
    virtual void destroy();

    /// Return true if this DisplayObject was destroyed.
    //
    /// See destroy() for more info.
    ///
    bool isDestroyed() const { return _destroyed; }
    
    /// Returns true when the DisplayObject bounds intersect with the current    
    /// rendering clipping area.
    ///
    /// There is no need to do any rendering for this DisplayObject when this 
    /// function returns false because the renderer will not change any pixels
    /// in the area where this DisplayObject is placed.    
    bool boundsInClippingArea() const; 

    /// Return full path to this object, in slash notation
    //
    /// e.g. "/sprite1/sprite2/ourSprite"
    ///
    std::string getTargetPath() const;

    /// Return original target path to this object, in dot notation
    /// as of at construction time.
    //
    /// This is needed to properly dereference dangling soft-references
    /// See testcase misc-swfc.all/soft_reference_test1.sc
    ///
    const std::string& getOrigTarget() const
    {
        return _origTarget;
    }

    /// Return full path to this object, in dot notation
    //
    /// e.g. "_level0.sprite1.sprite2.ourSprite"
    ///
    std::string DSOEXPORT getTarget() const;

#ifdef NEW_KEY_LISTENER_LIST_DESIGN
    boost::intrusive_ptr<as_function> getUserDefinedEventHandler(
            const std::string& name) const;
#endif

    /// Return true if this DisplayObject is a selectable TextField
    //
    /// This method is used by Gui to set up an appropriate cursor
    /// for input textfields.
    ///
    virtual bool isSelectableTextField() const { return false; }

    /// \brief
    /// Return true if this DisplayObject allows turning the cursor
    /// into an hand shape when it happens to be the one receiving
    /// mouse events.
    virtual bool allowHandCursor() const { return true; }

#ifdef USE_SWFTREE
    typedef std::pair<std::string, std::string> StringPair; 
    typedef tree<StringPair> InfoTree; 
    /// Append DisplayObject info in the tree
    //
    /// @param tr
    /// The tree to append movie to
    ///
    /// @param it
    /// The iterator to append info to.
    ///
    /// @return iterator the appended subtree
    ///
    // TODO: use a typedef for tree<StringPair> ?
    virtual InfoTree::iterator getMovieInfo(InfoTree& tr,
            InfoTree::iterator it);
#endif

    enum BlendMode
    {
        BLENDMODE_UNDEFINED = 0,
        BLENDMODE_NORMAL = 1,
        BLENDMODE_LAYER,
        BLENDMODE_MULTIPLY,
        BLENDMODE_SCREEN,
        BLENDMODE_LIGHTEN,
        BLENDMODE_DARKEN,
        BLENDMODE_DIFFERENCE,
        BLENDMODE_ADD,
        BLENDMODE_SUBTRACT,
        BLENDMODE_INVERT,
        BLENDMODE_ALPHA,
        BLENDMODE_ERASE,
        BLENDMODE_OVERLAY,
        BLENDMODE_HARDLIGHT = 14
    };

    BlendMode getBlendMode() const {
        return _blendMode;
    }
 
    void setBlendMode(BlendMode bm) {
        _blendMode = bm;
    }

    // action_buffer is externally owned
    typedef std::vector<const action_buffer*> BufferList;
    typedef std::map<event_id, BufferList> Events;

    /// Set the current focus to this DisplayObject.
    //
    /// @return false if the DisplayObject cannot receive focus, true if it can
    ///         (and does).
    //
    /// Button, Textfield and MovieClip can receive focus. In SWF6 and above,
    /// MovieClip can only receive focus if the focusEnabled property
    /// evaluates to true.
    virtual bool handleFocus() { 
        return false;
    }

    /// Some DisplayObjects require actions on losing focus.
    //
    /// Default is a no-op. TextField implements this function.
    virtual void killFocus() {}
  
    /// Getter-setter for _highquality.
    static as_value highquality(const fn_call& fn);
  
    /// Getter-setter for _quality.
    static as_value quality(const fn_call& fn);
  
    /// Getter-setter for blendMode.
    static as_value blendMode(const fn_call& fn);
  
    /// Getter-setter for _x
    static as_value x_getset(const fn_call& fn);
  
    /// Getter-setter for _y
    static as_value y_getset(const fn_call& fn);
  
    /// Getter-setter for _xscale
    static as_value xscale_getset(const fn_call& fn);
  
    /// Getter-setter for _yscale
    static as_value yscale_getset(const fn_call& fn);
  
    /// Getter-setter for _xmouse
    static as_value xmouse_get(const fn_call& fn);
  
    /// Getter-setter for _ymouse
    static as_value ymouse_get(const fn_call& fn);
  
    /// Getter-setter for _alpha
    static as_value alpha_getset(const fn_call& fn);
  
    /// Getter-setter for _visible
    static as_value visible_getset(const fn_call& fn);
  
    /// Getter-setter for _width
    static as_value width_getset(const fn_call& fn);
  
    /// Getter-setter for _height
    static as_value height_getset(const fn_call& fn);
  
    /// Getter-setter for _rotation
    static as_value rotation_getset(const fn_call& fn);
  
    /// Getter-setter for _parent 
    static as_value parent_getset(const fn_call& fn);
  
    /// Getter-setter for _target 
    static as_value target_getset(const fn_call& fn);
  
    /// Getter-setter for _name
    static as_value name_getset(const fn_call& fn);
  
    /// @} Common ActionScript getter-setters for DisplayObjects
  
protected:

    /// Register currently computable target as
    /// the "original" one. This will be used by
    /// soft references (as_value) and should be
    /// called as soon as the stagePlacementCallback
    /// is invoked.
    ///
    void saveOriginalTarget()
    {
        _origTarget=getTarget();
    }

#ifdef GNASH_USE_GC
    /// Mark all reachable resources, override from as_object.
    //
    /// The default implementation calls markDisplayObjectReachable().
    ///
    /// If a derived class provides access to more GC-managed
    /// resources, it should override this method and call 
    /// markDisplayObjectReachableResources() as the last step.
    ///
    virtual void markReachableResources() const
    {
        markDisplayObjectReachable();
    }

    /// Mark DisplayObject-specific reachable resources
    //
    /// These are: the DisplayObject's parent, mask, maskee and the default
    ///                         as_object reachable stuff.
    ///
    void markDisplayObjectReachable() const;
#endif // GNASH_USE_GC

    const Events& get_event_handlers() const
    {
            return _event_handlers;
    }

    /// Return a user defined event handler, if any
    //
    /// @param name
    ///     Function name to fetch. It will be converted to 
    /// lowercase if current VM has been initialized against
    /// an SWF version inferior to 7.
    ///
    /// @return
    /// A function if a member with the given name exists and
    /// casts to an as_function. A NULL pointer otherwise.
    ///
    boost::intrusive_ptr<as_function> getUserDefinedEventHandler(const std::string& name) const;

    /// Return a user defined event handler, if any
    //
    /// @param key
    ///     Function key to fetch. 
    ///
    /// @return
    /// A function if a member with the given key exists and
    /// casts to an as_function. A NULL pointer otherwise.
    ///
    boost::intrusive_ptr<as_function> getUserDefinedEventHandler(string_table::key key) const;

    void set_event_handlers(const Events& copyfrom);

    /// Used to assign a name to unnamed instances
    static std::string getNextUnnamedInstanceName();

    /// Name of this DisplayObject (if any)
    std::string _name;

    boost::intrusive_ptr<DisplayObject> m_parent;

    /// look for '.', 'this',    '..', '_parent', '_level0' and '_root'
    //
    /// NOTE: case insensitive up to SWF6, sensitive from SWF7 up
    ///
    as_object* getPathElementSeparator(string_table::key key);

    /// \brief
    /// Set when the visual aspect of this particular DisplayObject or movie
    /// has been changed and redrawing is necessary.    
    //
    /// This is initialized to true as the initial state for
    /// any DisplayObject is the "invisible" state (it wasn't there)
    /// so it starts in invalidated mode.
    ///
    bool m_invalidated;

    /// Just like m_invalidated but set when a child is invalidated instead
    /// of this DisplayObject instance. m_invalidated and m_child_invalidated
    /// can be set at the same time. 
    bool m_child_invalidated;

    /// \brief
    /// Bounds of this DisplayObject instance before first invalidation
    /// since last call to clear_invalidated().
    ///
    /// This stores the bounds of the DisplayObject before it has been changed, ie. 
    /// the position when set_invalidated() is being called. While drawing, both 
    /// the old and the new bounds are updated (rendered). When moving a 
    /// DisplayObject A to B then both the position A needs to be re-rendered (to 
    /// reveal the backgrond) and the position B needs to be re-rendered (to
    /// show the DisplayObject in its new position). The bounds may be identical or 
    /// overlap, but SnappingRanges takes care of that.
    /// 
    /// Will be set by set_invalidated() and used by
    /// get_invalidated_bounds().
    ///
    InvalidatedRanges m_old_invalidated_ranges;

private:

    /// Register a DisplayObject masked by this instance
    void setMaskee(DisplayObject* maskee);

    /// Build the _target member recursive on parent
    std::string computeTargetPath() const;

    int m_id;

    int m_depth;
    cxform  m_color_transform;
    SWFMatrix  m_matrix;

    /// Cache values for ActionScript access.
    /// NOTE: not all DisplayObjects need this, just the
    ///       ones which are ActionScript-referenceable
    double _xscale, _yscale, _rotation;

    /// Volume control associated to this DisplayObject
    //
    /// This is used by Sound objects
    ///
    /// NOTE: probably only ActionScript-referenceable DisplayObjects
    ///       need this (assuming soft ref don't rebind to other
    ///       kind of DisplayObjects).
    ///
    int  _volume;

    int   m_ratio;
    int m_clip_depth;
    Events  _event_handlers;

    /// Used to assign a name to unnamed instances
    static unsigned int _lastUnnamedInstanceNum;

    /// Set to yes when this instance has been unloaded
    bool _unloaded;

    /// This flag should be set to true by a call to destroy()
    bool _destroyed;

    /// The DisplayObject masking this instance (if any)
    DisplayObject* _mask;

    /// The DisplayObject masked by this instance (if any)
    DisplayObject* _maskee;

    /// Original target, as at construction time
    std::string _origTarget;

    BlendMode _blendMode;

    bool _visible;

    /// Whether this DisplayObject has been transformed by ActionScript code
    //
    /// Once we've been moved by ActionScript,
    /// Don't accept moves from anim tags (PlaceObject)
    ///
    /// See get_accept_anim_moves() function
    ///
    bool _scriptTransformed;

    bool _dynamicallyCreated;

};

/// Stream operator for DisplayObject blend mode.
std::ostream&
operator<<(std::ostream& o, DisplayObject::BlendMode bm);

} // end namespace gnash


#ifdef DEBUG_SET_INVALIDATED
#define set_invalidated() set_invalidated(__FILE__, __LINE__)
#endif


#endif // GNASH_CHARACTER_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
