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


#ifndef GNASH_DISPLAY_OBJECT_H
#define GNASH_DISPLAY_OBJECT_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h" // USE_SWFTREE
#endif

#include <vector>
#include <map>
#include <string>
#include <cassert>
#include <boost/cstdint.hpp> // For C99 int types
#include <boost/noncopyable.hpp>
#include <boost/logic/tribool.hpp>

#include "ObjectURI.h" 
#include "GC.h"
#include "Transform.h"
#include "event_id.h" 
#include "SWFRect.h"
#include "SWFMatrix.h"
#include "SWFCxForm.h"
#include "dsodefs.h" 
#include "snappingrange.h"
#ifdef USE_SWFTREE
# include "tree.hh"
#endif


//#define DEBUG_SET_INVALIDATED 1

// Forward declarations
namespace gnash {
    class MovieClip;
    class movie_root;
    class fn_call;
    class Movie;
    class ExecutableCode;
    class action_buffer;
    class movie_definition;
    class StaticText;
    class InteractiveObject;
    class Renderer;
    class as_object;
    class as_value;
    class as_environment;
    class DisplayObject;
    class KeyVisitor;
    namespace SWF {
        class TextRecord;
    }
}

namespace gnash {

/// Returns true if the DisplayObject is referenceable in ActionScript
//
/// A DisplayObject is referenceable if it has an associated object.
bool isReferenceable(const DisplayObject& d);

/// Set special properties
//
/// This sets the magic properties of DisplayObjects.
//
/// @param key      The string table key of the property to set.
/// @param obj      The DisplayObject whose property should be set
/// @param val      An as_value representing the new value of the property.
///                 Some values may be rejected.
bool setDisplayObjectProperty(DisplayObject& obj, const ObjectURI& uri,
        const as_value& val);

/// Get special properties
//
/// This gets the magic properties of DisplayObjects and handles special
/// MovieClip properties such as DisplayList members.
//
/// @param key      The uri of the property to get.
/// @param obj      The DisplayObject whose property should be got
/// @param val      An as_value to be set to the value of the property.
bool getDisplayObjectProperty(DisplayObject& obj, const ObjectURI& uri,
        as_value& val);

/// Get a property by its numeric index.
//
/// Used by ASHandlers to get the DisplayObject properties indexed by number
//
/// @param index    The index of the property to get.
/// @param o        The DisplayObject whose property should be got
/// @param val      An as_value to be set to the value of the property.
void getIndexedProperty(size_t index, DisplayObject& o, as_value& val);

/// Set a property by its numeric index.
//
/// Used by ASHandlers to set the DisplayObject properties indexed by number
//
/// @param index    The index of the property to set.
/// @param o        The DisplayObject whose property should be set
/// @param val      An as_value representing the new value of the property.
///                 Some values may be rejected.
void setIndexedProperty(size_t index, DisplayObject& o, const as_value& val);

/// Copy SWFMatrix and caches from given DisplayObjecta
//
/// @param from     The DisplayObject to copy from
/// @param to       The DisplayObject to copy to.
void copyMatrix(const DisplayObject& from, DisplayObject& to);

/// Get concatenated SWFMatrix (all ancestor transforms and our SWFMatrix)
//
/// Maps from our local space into "world" space
/// (i.e. root movie space).
//
/// @param includeRoot      Whether the transform of the Stage (_root)
///                         should be concatenated. This is required to be
///                         false for pointInBounds.
SWFMatrix getWorldMatrix(const DisplayObject& d, bool includeRoot = true);

/// Get concatenated color transform of a DisplayObject
//
/// Maps from our local space into normal color space.
SWFCxForm getWorldCxForm(const DisplayObject& d);

/// DisplayObject is the base class for all DisplayList objects.
//
/// It represents a single active element in a movie. This class does not
/// supply any interactivity. The hierarchy of DisplayObjects in a movie
/// provides all visual elements in a SWF. The DisplayObject hierarchy
/// is independent of ActionScript resources, but can be controlled via AS.
//
/// DisplayObjects that can be controlled through ActionScript have an
/// associated as_object. DisplayObjects such as Shape, do not have an
/// associated object and cannot be referenced in AS.
//
/// Derived classes include InteractiveObject, StaticText, Bitmap,
/// Video, and Shape.
//
/// All DisplayObjects may be constructed during SWF parsing. In this case
/// they are constructed using an immutable, non-copyable SWF::DefinitionTag. 
/// This tag should never be changed!
//
/// Most DisplayObjects may also be constructed dynamically. In AS3, Bitmaps
/// and Shapes can be dynamically created. Dynamically-created DisplayObjects
/// must not have a SWF::DefinitionTag!
//
/// The presence of a definition tag may be used to distinguish static from
/// dynamic DisplayObjects, but tags are not always stored. They are not
/// stored in most InteractiveObjects because most properties can be
/// overridden during SWF execution.
class DSOTEXPORT DisplayObject : public GcResource, boost::noncopyable
{
public:

    /// Construct a DisplayObject
    //
    /// @param mr       The movie_root containing the DisplayObject hierarchy.
    ///                 All DisplayObjects may need movie_root resources.
    /// @param object   An object to be associated with this DisplayObject.
    ///                 If this is non-null, the DisplayObject will be
    ///                 referenceable in ActionScript. Referenceable
    ///                 DisplayObjects may access AS resources through their
    ///                 associated object.
    /// @param parent   The parent of the new DisplayObject. This may be null.
    DisplayObject(movie_root& mr, as_object* object, DisplayObject* parent);

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
    ///   3:  Max depth for a PlaceObject call is 16384 (which becomes 
    ///       0 in the statics)
    /// (all of the above correct?)
    static const int removedDepthOffset = -32769; 

    /// This value is used for m_clip_depth when 
    /// the DisplayObject is not a layer mask.
    //
    /// Depths below -16384 are illegal, so this
    /// value should not collide with real depths.  
    ///
    static const int noClipDepthValue = -1000000;

    /// Return a reference to the variable scope of this DisplayObject.
    virtual as_environment& get_environment() {
        // MovieClip must override this
        // and any other DisplayObject will have
        // a parent!
        assert(_parent != NULL);
        return _parent->get_environment();
    }

    /// Enumerate any non-proper properties
    //
    /// This function allows enumeration of properties that are
    /// derived from the DisplayObject type, e.g. DisplayList members.
    ///
    /// The default implementation adds nothing
    virtual void visitNonProperties(KeyVisitor&) const {}

    /// \brief
    /// Return the parent of this DisplayObject, or NULL if
    /// the DisplayObject has no parent.
    DisplayObject* parent() const
    {
        return _parent;
    }

    /// Set the parent of this DisplayObject
    //
    /// In AS3, DisplayObjects may be created before being attached to 
    /// a parent. In AS2, this is only used for external movies
    void set_parent(DisplayObject* parent)
    {
        _parent = parent;
    }

    virtual MovieClip* to_movie() { return 0; }

    int get_depth() const { return _depth; }

    void set_depth(int d) { _depth = d; }

    /// Get sound volume for this DisplayObject
    int getVolume() const { return _volume; }

    /// Set sound volume for this DisplayObject
    void setVolume(int vol) { _volume = vol; }

    /// Get concatenated sound volume for this DisplayObject
    //
    /// NOTE: the concatenated volume does NOT include
    ///       global volume settings, which is the one
    ///       controlled by Sound instances created passing
    ///       null, undefined or no argument to constructor.
    ///
    int getWorldVolume() const;

    /// DisplayObjects can return the version of the SWF they were parsed from.
    virtual int getDefinitionVersion() const {
        return -1;
    }
    
    const Transform& transform() const {
        return _transform;
    }


    /// Set local transform SWFMatrix for this DisplayObject
    //
    /// @param m the new SWFMatrix to assign to this DisplayObject
    ///
    /// @param updateCache if true, updates the cache values
    ///        from the SWFMatrix (only if SWFMatrix != current SWFMatrix)
    ///
    void setMatrix(const SWFMatrix& m, bool updateCache = false);

    /// Set the xscale value of current SWFMatrix
    //
    /// This is used when setting _xscale.
    /// See xscale_getset.
    ///
    /// @param factor scale factor, in percent
    ///
    void set_x_scale(double factor);

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
    ///
    /// @param w new width, in TWIPS. 
    //
    /// TextField does this differently (caches not updated).
    virtual void setWidth(double width);

    /// Set the height of this DisplayObject, modifying its SWFMatrix
    //
    /// This is used when setting _height
    ///
    /// @param h new height, in TWIPS. 
    ///
    virtual void setHeight(double height);

    void setCxForm(const SWFCxForm& cx) 
    {       
        if (_transform.colorTransform != cx) {
            set_invalidated();
            _transform.colorTransform = cx;
        }
    }

    boost::uint16_t get_ratio() const { return _ratio; }

    void set_ratio(boost::uint16_t r) {
        if (r != _ratio) set_invalidated(); 
        _ratio = r;       
    }

    /// Returns the clipping depth (if any) of this DisplayObject.
    /// The parameter tells us to use the DisplayObject as a mask for
    /// all the objects contained in the display list from _depth
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
        
    /// Returns true when the DisplayObject (and its childs) is used as a mask
    /// for other DisplayObjects at higher depth (up to get_clip_depth).
    /// isMaskLayer() does not return true when one of its
    /// parents is a mask and the DisplayObject itself is not.
    ///
    /// See also isDynamicMask() and isMask()
    ///     
    bool isMaskLayer() const
    {
        return (m_clip_depth != noClipDepthValue && !_maskee);
    }

    /// Returns true when the DisplayObject (and its childs) is used as a mask
    /// for another DisplayObject.
    /// isDynamicMask() does not return true when one of its
    /// parents is a mask and the DisplayObject itself is not.
    ///
    /// NOTE: there's no way to obtain the maskee from a dynamic mask
    ///
    /// See also isMaskLayer() and isMask()
    ///     
    bool isDynamicMask() const
    {
        return (_maskee);
    }

    /// Return the DisplayObject masking this instance (if any)
    DisplayObject* getMask() const
    {
#if GNASH_PARANOIA_LEVEL > 1
        if (_mask) assert(_mask->_maskee == this);
#endif
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

    /// Set DisplayObject name, initializing the original target member
    void set_name(const ObjectURI& uri) {
        _name = uri;
    }

    const ObjectURI& get_name() const { return _name; }

    /// Get the built-in function handlers code for the given event
    //
    /// NOTE: this function is only for getting statically-defined
    ///       event handlers, which are the ones attached to a DisplayObject
    ///       with a PlaceObject2. It's the DisplayObject's responsibility
    ///       to properly fetch any user-defined event handler, which 
    ///       are the ones attached to a DisplayObject with ActionScript code.
    ///
    std::unique_ptr<ExecutableCode> get_event_handler(const event_id& id) const;

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
	virtual void display(Renderer& renderer, const Transform& xform) = 0;

    /// Search for StaticText objects
    //
    /// If this is a StaticText object and contains SWF::TextRecords, these
    /// are written to the passed parameter.
    /// @ return    0 if this object is not a StaticText or contains no text.
    virtual StaticText* getStaticText(std::vector<const SWF::TextRecord*>&,
            size_t&) {
        return 0;
    }

	virtual SWFRect getBounds() const = 0;

    /// Return true if the given point falls in this DisplayObject's bounds
    //
    /// @param x        Point x coordinate in world space
    /// @param y        Point y coordinate in world space
    /// @return         Whether (x, y) is within the DisplayObject's bounds.
    ///                 This ignores _root's transform. 
    bool pointInBounds(boost::int32_t x, boost::int32_t y) const
    {
        SWFRect bounds = getBounds();
        const SWFMatrix wm = getWorldMatrix(*this, false);
        wm.transform(bounds);
        return bounds.point_test(x, y);
    }

    /// Return true if the given point falls in this DisplayObject's shape
    //
    /// @param x        Point x coordinate in world space
    /// @param y        Point y coordinate in world space
    /// @return         Whether (x, y) is within the DisplayObject's bounds.
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
        if (!visible()) return false;
        if (isDynamicMask() || isMaskLayer()) return false;
        return pointInShape(x, y);
    }

    /// Return the relative root of this DisplayObject
    //
    /// The "relative" is the Movie created by
    /// the same SWF definition that contained the
    /// definition of this DisplayObject.
    ///
    /// The default implementation is to invoke get_root
    /// against this DisplayObject's parent.
    ///
    virtual Movie* get_root() const {
        return parent()->get_root();
    }

    /// Return the _root ActionScript property of this DisplayObject.
    //
    /// By default calls get_root(). The resulting MovieClip may be passed
    /// to actionscript methods, so it is not const. As the override in
    /// MovieClip may return this, the method cannot be const either.
    virtual MovieClip* getAsRoot();

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
    virtual as_object* pathElement(const ObjectURI& uri);

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
    bool visible() const { return _visible; }

    /// Return true if an handler for the given event is defined
    //
    /// NOTE that we look for both clip-defined and user-defined
    /// handlers, which is likely error prone since we're doing
    /// this in a non-virtual function. Main use for this method
    /// is for being called by ::unload() to verify an Unload handler
    /// is available.
    bool hasEventHandler(const event_id& id) const;

	/// DisplayObjects are not a mouse entity by default.
    //
    /// Override this function for InteractiveObjects.
	virtual InteractiveObject* topmostMouseEntity(boost::int32_t, 
            boost::int32_t) {
        return 0;
    }
	
    /// Find highest depth DisplayObject whose shape contains the given
    /// point and is not the DisplayObject being dragged or any of its childs.
    //
    /// Point coordinates in global twips.
    virtual const DisplayObject* findDropTarget(boost::int32_t x, 
            boost::int32_t y, DisplayObject* dragging) const
    {
        if (this != dragging && visible() && pointInVisibleShape(x, y)) {
            return this;
        }
        
        return 0;
    }

    /// Return whether this DisplayObject has been invalidated or not
    bool invalidated() const {
        return _invalidated;
    }

    /// Return whether this DisplayObject has and invalidated child or not
    bool childInvalidated() const {
        return _child_invalidated;
    }

    /// Notify a change in the DisplayObject's appearance.
    virtual void update() {
        set_invalidated();
    }

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
    ///             its parent as being invalidated.
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
    /// recursively inform all its parents of the change.
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
    /// prevent the parent to be informed when this DisplayObject (or a
    /// child) is invalidated again (see set_invalidated() recursion).
    void clear_invalidated() {
        _invalidated = false;
        _child_invalidated = false;        
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
    /// Only instances with _invalidated flag set are checked unless
    /// force is set.
    ///
    virtual void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);

    /// Called instead of display() when the DisplayObject is not visible
    /// on stage.
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
    /// This handles all ActionScript construction and initialization events.
    virtual void construct(as_object* /*init*/ = 0)
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
    bool unload();

    /// Accept a loaded Movie
    virtual void getLoadedMovie(Movie* newMovie);

    /// Return true if this DisplayObject was unloaded from the stage
    bool unloaded() const {
        return _unloaded;
    }

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
    bool boundsInClippingArea(Renderer& renderer) const; 

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
    bool DSOEXPORT allowHandCursor() const;

#ifdef USE_SWFTREE
    typedef tree<std::pair<std::string, std::string> > InfoTree; 
    /// Append DisplayObject info in the tree
    //
    /// @param tr
    /// The tree to append movie to
    ///
    /// @param it
    /// The iterator to append info to.
    ///
    /// @return iterator the appended subtree
    virtual InfoTree::iterator getMovieInfo(InfoTree& tr,
            InfoTree::iterator it);
#endif
    
    /// Used to assign a name to unnamed instances
    ObjectURI getNextUnnamedInstanceName();

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
  
    double rotation() const {
        return _rotation;
    }

    double scaleX() const {
        return _xscale;
    }

    double scaleY() const {
        return _yscale;
    }

    as_object* object() const {
        return _object;
    }

    /// Getter-setter for blendMode.
    static as_value blendMode(const fn_call& fn);
  
    /// Mark all reachable resources.
    //
    /// Try not to override this function in derived classes. This always
    /// marks the base class's resources and calls markOwnResources() to
    /// take care of any further GC resources.
    virtual void markReachableResources() const;

    /// Called by markReachableResources()
    //
    /// DisplayObjects should mark their own resources in this function.
    virtual void markOwnResources() const {}

    boost::tribool focusRect() const {
        return _focusRect;
    }

    void focusRect(boost::tribool focus) {
        _focusRect = focus;
    }

protected:
    
    /// Render a dynamic mask for a specified DisplayObject
    //
    /// Dynamic masks are rendered out-of-turn when the object they are masking
    /// is drawn. 
    //
    /// A MaskRenderer object should be constructed at the beginning of
    /// relevant display() functions; it then takes care of rendering the
    /// mask with the appropriate transform and cleaning up afterwards.
    class MaskRenderer
    {
    public:
        MaskRenderer(Renderer& r, const DisplayObject& o);
        ~MaskRenderer();
    private:
        Renderer& _renderer;
        DisplayObject* _mask;
    };

    virtual bool unloadChildren() { return false; }

    /// Get the movie_root to which this DisplayObject belongs.
    movie_root& stage() const {
        return _stage;
    }

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

    const Events& get_event_handlers() const
    {
        return _event_handlers;
    }

    void set_event_handlers(const Events& copyfrom);

    /// Name of this DisplayObject (if any)
    ObjectURI _name; 

    DisplayObject* _parent;

    /// look for '.', 'this',    '..', '_parent', '_level0' and '_root'
    //
    /// NOTE: case insensitive up to SWF6, sensitive from SWF7 up
    ///
    as_object* getPathElementSeparator(string_table::key key);

    /// \brief
    /// Bounds of this DisplayObject instance before first invalidation
    /// since last call to clear_invalidated().
    ///
    /// This stores the bounds of the DisplayObject before it has been
    /// changed, ie. the position when set_invalidated() is being called.
    /// While drawing, both the old and the new bounds are updated (rendered).
    /// When moving a DisplayObject A to B then both the position A needs
    /// to be re-rendered (to reveal the backgrond) and the position B
    /// needs to be re-rendered (to show the DisplayObject in its new
    /// position). The bounds may be identical or overlap, but
    /// SnappingRanges takes care of that.
    /// 
    /// Will be set by set_invalidated() and used by
    /// get_invalidated_bounds().
    InvalidatedRanges m_old_invalidated_ranges;

private:

    /// Register a DisplayObject masked by this instance
    void setMaskee(DisplayObject* maskee);

    /// The as_object to which this DisplayObject is attached.
    as_object* _object;

    /// The movie_root to which this DisplayObject belongs.
    movie_root& _stage;

    Transform _transform;
    
    Events _event_handlers;

    /// Cache values for ActionScript access.
    /// NOTE: not all DisplayObjects need this, just the
    ///       ones which are ActionScript-referenceable
    double _xscale, _yscale, _rotation;

    /// The depth of this DisplayObject.
    boost::int32_t _depth;

    boost::tribool _focusRect;

    /// Volume control associated to this DisplayObject
    //
    /// This is used by Sound objects
    ///
    /// NOTE: probably only ActionScript-referenceable DisplayObjects
    ///       need this (assuming soft ref don't rebind to other
    ///       kind of DisplayObjects).
    ///
    int _volume;

    boost::uint16_t _ratio;
    int m_clip_depth;

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

    /// Set to yes when this instance has been unloaded
    bool _unloaded;

    /// This flag should be set to true by a call to destroy()
    bool _destroyed;

    /// \brief
    /// Set when the visual aspect of this particular DisplayObject or movie
    /// has been changed and redrawing is necessary.    
    //
    /// This is initialized to true as the initial state for
    /// any DisplayObject is the "invisible" state (it wasn't there)
    /// so it starts in invalidated mode.
    ///
    bool _invalidated;

    /// Just like _invalidated but set when a child is invalidated instead
    /// of this DisplayObject instance. _invalidated and _child_invalidated
    /// can be set at the same time. 
    bool _child_invalidated;


};

/// Get local transform SWFMatrix for this DisplayObject
inline const SWFMatrix&
getMatrix(const DisplayObject& o)
{ 
    return o.transform().matrix;
}

inline const SWFCxForm&
getCxForm(const DisplayObject& o) 
{
    return o.transform().colorTransform;
}

inline SWFMatrix
getWorldMatrix(const DisplayObject& d, bool includeRoot)
{
    SWFMatrix m = d.parent() ?
        getWorldMatrix(*d.parent(), includeRoot) : SWFMatrix();

    if (d.parent() || includeRoot) m.concatenate(getMatrix(d));
    return m;
}

inline SWFCxForm
getWorldCxForm(const DisplayObject& d)
{
    SWFCxForm cx = d.parent() ? getWorldCxForm(*d.parent()) : SWFCxForm();
    cx.concatenate(getCxForm(d));
    return cx;
}

inline bool
isReferenceable(const DisplayObject& d)
{
    return d.object();
}

/// Return the as_object associated with a DisplayObject if it exists
//
/// @param d    The DisplayObject to check. May be null.
/// @return     null if either the DisplayObject or the associated object is
///             null. Otherwise the associated object.
inline as_object*
getObject(const DisplayObject* d)
{
    return d ? d->object() : 0;
}

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
