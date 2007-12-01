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
//

/* $Id: character.h,v 1.112 2007/12/01 00:14:59 strk Exp $ */

#ifndef GNASH_CHARACTER_H
#define GNASH_CHARACTER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gnash.h"
#include "types.h"
#include "utility.h"
#include "event_id.h" // for inlines
#include "as_object.h" // for inheritance
#include "rect.h" // for composition (invalidated bounds)
#include "matrix.h" // for composition
#include "cxform.h" // for composition
#include "log.h"
#include "snappingrange.h"
#include "Range2d.h"

#include <map>
#include <cstdarg>
#include <string>
#include <cassert>
#include <typeinfo>

//#define DEBUG_SET_INVALIDATED 1

// Forward declarations
namespace gnash {
	class sprite_instance;
	class movie_instance;
	class ExecutableCode;
	class action_buffer;
}

namespace gnash {

/// Informations about timeline instances creation
//
/// See: http://www.gnashdev.org/wiki/index.php/TimelineControl#Timeline_instances
///
class TimelineInfo
{

public:

	/// Construct a TimelineInfo
	//
	/// @param depth
	///	Depth at which the instance was placed 
	///
	/// @param frame
	///	Frame number in which the instance was placed (0-based)
	///
	/// @param replace
	///	True if this object was placed by a REPLACE tag
	///	(see PlaceObject2).
	///
	TimelineInfo(int depth, int frame, bool replace)
		:
		_depth(depth),
		_frame(frame),
		_replace(replace)
	{
	}


	/// Return depth of initial placement 
	int placedAtDepth() const { return _depth; }

	/// Return frame number of initial placement (0-based)
	size_t placedInFrame() const { return _frame; }

	/// Return true if this instance replaced an other one at same depth
	bool placedByReplaceTag() const { return _replace; }

private:

	/// Original depth
	int _depth;

	/// Frame of placement, 0-based
	size_t _frame;

	/// Placed by Replace tag ?
	bool _replace;
};

/// Character is a live, stateful instance of a character_def.
//
/// It represents a single active element in a movie.
/// Inheritance from movie is an horrible truth!
///
class character : public as_object
{

public:

	// action_buffer is externally owned
	typedef std::vector<const action_buffer*> BufferList;
	typedef std::map<event_id, BufferList> Events;

private:

	int	m_id;

	int	m_depth;
	cxform	m_color_transform;
	matrix	m_matrix;
	int 	m_ratio;
	int	m_clip_depth;
	Events  _event_handlers;
	void	(*m_display_callback)(void*);
	void*	m_display_callback_user_ptr;

	/// Used to assign a name to unnamed instances
	static unsigned int _lastUnnamedInstanceNum;

	/// Set to yes when this instance has been unloaded
	bool _unloaded;

	/// This flag should be set to true by a call to destroy()
	bool _destroyed;

	/// Build the _target member recursive on parent
	std::string computeTargetPath() const;

	/// Timeline info, for timeline instances
	//
	/// For dynamically-created instances this is always NULL
	///
	std::auto_ptr<TimelineInfo> _timelineInfo;

	/// The character masking this instance (if any)
	character* _mask;

protected:

#ifdef GNASH_USE_GC
	/// Mark all reachable resources, override from as_object.
	//
	/// The default implementation calls markCharacterReachable().
	///
	/// If a derived class provides access to more GC-managed
	/// resources, it should override this method and call 
	/// markCharacterReachableResources() as the last step.
	///
	virtual void markReachableResources() const
	{
		markCharacterReachable();
	}

	/// Mark character-specific reachable resources
	//
	/// These are: the character's parent, mask and the default
	///             as_object reachable stuff.
	///
	void markCharacterReachable() const
	{
		if ( m_parent ) m_parent->setReachable();
		if ( _mask )
		{
			// TODO: check if we should drop when isUnloaded() or isDestroyed()
			_mask->setReachable();
		}
		markAsObjectReachable();
	}
#endif // GNASH_USE_GC

	const Events& get_event_handlers() const
	{
	    return _event_handlers;
	}

	/// Return a user defined event handler, if any
	//
	/// @param name
	/// 	Function name to fetch. It will be converted to 
	///	lowercase if current VM has been initialized against
	///	an SWF version inferior to 7.
	///
	/// @return
	///	A function if a member with the given name exists and
	///	casts to an as_function. A NULL pointer otherwise.
	///
	boost::intrusive_ptr<as_function> getUserDefinedEventHandler(const std::string& name) const;

	void set_event_handlers(const Events& copyfrom);

	/// Used to assign a name to unnamed instances
	static std::string getNextUnnamedInstanceName();

	/// Name of this character (if any)
	std::string	_name;

	bool m_visible;

	boost::intrusive_ptr<character> m_parent;

	/// look for '.', 'this',  '..', '_parent', '_level0' and '_root'
	//
	/// NOTE: case insensitive up to SWF6, sensitive from SWF7 up
	///
	as_object* get_path_element_character(string_table::key key);

	/// \brief
	/// Set when the visual aspect of this particular character or movie
	/// has been changed and redrawing is necessary.  
	//
	/// This is initialized to true as the initial state for
	/// any character is the "invisible" state (it wasn't there)
	/// so it starts in invalidated mode.
	///
	bool m_invalidated;

	/// Just like m_invalidated but set when a child is invalidated instead
	/// of this character instance. m_invalidated and m_child_invalidated
	/// can be set at the same time. 
	bool m_child_invalidated;

	/// \brief
	/// Bounds of this character instance before first invalidation
	/// since last call to clear_invalidated().
	///
	/// This stores the bounds of the character before it has been changed, ie. 
	/// the position when set_invalidated() is being called. While drawing, both 
	/// the old and the new bounds are updated (rendered). When moving a 
	/// character A to B then both the position A needs to be re-rendered (to 
	/// reveal the backgrond) and the position B needs to be re-rendered (to show 
	/// the character in it's new position). The bounds may be identical or 
	/// overlap, but SnappingRanges takes care of that.
	/// 
	/// Will be set by set_invalidated() and used by
	/// get_invalidated_bounds().
	///
	InvalidatedRanges m_old_invalidated_ranges;

	/// Wheter this character has been transformed by ActionScript code
	//
	/// Once we've been moved by ActionScript,
	/// Don't accept moves from anim tags (PlaceObject)
	///
	/// See get_accept_anim_moves() function
	///
	bool _scriptTransformed;

	bool _dynamicallyCreated;

	

	/// @{ Common ActionScript getter-setters for characters

public:  // TODO: make protected

#if 0
	static as_value onrollover_getset(const fn_call& fn);

	static as_value onrollout_getset(const fn_call& fn);

	static as_value onload_getset(const fn_call& fn);

	static as_value onpress_getset(const fn_call& fn);

	static as_value onrelease_getset(const fn_call& fn);

	static as_value onreleaseoutside_getset(const fn_call& fn);

	static as_value onmouseup_getset(const fn_call& fn);

	static as_value onmousedown_getset(const fn_call& fn);

	static as_value onmousemove_getset(const fn_call& fn);
#endif

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

	/// @} Common ActionScript getter-setters for characters

public:

    /// This is the amount added to displaylist tag defined depths.
    /// Character placed by tags (vs. characters instantiated by ActionScript)
    /// always have negative depths by effect of this offset.
    //
    /// Macromedia Flash help says: depth starts at -16383 (0x3FFF)
    ///
    /// See: http://www.senocular.com/flash/tutorials/depths/?page=2
    ///
    static const int staticDepthOffset = -16384;

    /// This is the offset at which character's depth is
    /// shifted when a character is removed from stage but
    /// an onUnload event handler is defined.
    ///
    /// Example: a character at depth 60 gets moved to
    ///          depth -32829 (-32769-60) when unloaded and
    ///          an onUnload event handler is defined for it
    ///	         or any of its childs.
    ///
    /// So, to recap:
    ///		1:  -32769 to -16385 are removed
    ///		2:  -16384 to      0 are statics
    ///		3:  Max depth for a PlaceoObject call is 16384 (which becomes 0 in the statics)
    ///	(all of the above correct?)
    ///
    ///
    static const int removedDepthOffset = -32769; 

    /// Return true if the given depth is in the removed zone
    static bool depthInRemovedZone(int depth)
    {
        return depth < staticDepthOffset;
    }

    /// Return true if this character's depth is in the removed zone
    bool depthInRemovedZone()
    {
        return depthInRemovedZone(get_depth());
    }
    
    /// This value is used for m_clip_depth when 
    /// the character is not a layer mask.
    //
    /// Depths below -16384 are illegal, so this
    /// value should not collide with real depths.  
    ///
    static const int noClipDepthValue = -1000000;

    /// This value is used for m_clip_depth when 
    /// the character is a dynamic mask.
    //
    /// Depths below -16384 are illegal, so this
    /// value should not collide with real depths.  
    ///
    static const int dynClipDepthValue = -2000000;

	// Maybe it's better to move all these constants to DisplayListTag
	static const int noRatioValue = -1;

    ~character();

    character(character* parent, int id)
	:
	m_id(id),
	m_depth(0),
	m_ratio(0),
	m_clip_depth(noClipDepthValue),
	m_display_callback(NULL),
	m_display_callback_user_ptr(NULL),
	_unloaded(false),
	_destroyed(false),
	_mask(0),
	m_visible(true),
	m_parent(parent),
	m_invalidated(true),
	m_child_invalidated(true),
	m_old_invalidated_ranges(),
	_scriptTransformed(false),
	_dynamicallyCreated(false)
    {
	    assert((parent == NULL && m_id == -1)
		   || (parent != NULL && m_id >= 0));
	    assert(m_old_invalidated_ranges.isNull());
    }

	/// Return a reference to the variable scope of this character.
	//
	/// TODO: make const/return const& ?
	///
	virtual as_environment& get_environment() {
		// sprite_instance must override this
		// and any other character will have
		// a parent!
		assert(m_parent != NULL);
		return m_parent->get_environment();
	}

    // Accessors for basic display info.
    int	get_id() const { return m_id; }

	/// \brief
	/// Return the parent of this character, or NULL if
	/// the character has no parent.
	character* get_parent() const
	{
			return m_parent.get();
	}

    // for extern movie
    void set_parent(character* parent) { m_parent = parent; }

    int	get_depth() const { return m_depth; }

    void	set_depth(int d) { m_depth = d; }

    const matrix&	get_matrix() const { return m_matrix; }

    void	set_matrix(const matrix& m)
	{
	    assert(m.is_valid());
	    if (!(m == m_matrix)) {
	      set_invalidated(__FILE__, __LINE__);
	      m_matrix = m;
	    }
	}

    /// Set the xscale value of current matrix
    //
    /// This is used when setting either _xscale or _width.
    /// See xscale_getset and width_getset
    ///
    void set_x_scale(float factor);

    /// Set the yscale value of current matrix
    //
    ///
    /// This is used when setting either _yscale or _height
    /// See xscale_getset and width_getset
    ///
    void set_y_scale(float factor);

    const cxform&	get_cxform() const { return m_color_transform; }

    void	set_cxform(const cxform& cx) 
    {       
      if (!(cx == m_color_transform)) {
      	set_invalidated(__FILE__, __LINE__);
      	m_color_transform = cx;
      }
    }

    void	concatenate_cxform(const cxform& cx) { m_color_transform.concatenate(cx); }

    void	concatenate_matrix(const matrix& m) { m_matrix.concatenate(m); }

    int		get_ratio() const { return m_ratio; }

    void	set_ratio(int r)
    {
      if (r!=m_ratio) set_invalidated(__FILE__, __LINE__); 
      m_ratio = r;       
    }

    /// Returns the clipping depth (if any) of this character. The parameter is 
    /// tells us to use the character as a mask for all the objects contained 
    /// in the display list from m_depth to m_clipping_depth inclusive.
    /// 
    /// The value returned by get_clip_depth() is only valid when isMaskLayer()
    /// returns true!
    ///  
    int get_clip_depth() const { return m_clip_depth; }

	/// See get_clip_depth()
	void set_clip_depth(int d)
	{
		m_clip_depth = d;
		_mask = 0; // in case we're masked by some other char
	}
    
	/// Returns true when the character (and it's childs) is used as a mask
	/// for other characters at higher depth (up to get_clip_depth).
	/// isMaskLayer() does *not* return true when one of it's
	/// parents is a mask and the character itself is not.
	///
	/// See also isDynamicMask() and isMask()
	///   
	bool isMaskLayer() const
	{
		return (m_clip_depth!=noClipDepthValue);
	}

	/// Returns true when the character (and it's childs) is used as a mask
	/// for another character.
	/// isDynamicMask() does *not* return true when one of it's
	/// parents is a mask and the character itself is not.
	///
	/// NOTE: there's no way to obtain the maskee from a dynamic mask
	///
	/// See also isMaskLeyer() and isMask()
	///   
	bool isDynamicMask() const
	{
		return (m_clip_depth==dynClipDepthValue);
	}

	character* to_character() { return this; }

	/// Return the character masked by this instance (if any)
	character* getMask() const
	{
		return _mask;
	}

	/// Register a character as a mask for this instance.
	///
	/// @param mask The character to use as a mask, possibly NULL.
	///
	void setMask(character* mask)
	{
		if ( _mask == mask ) return;

		set_invalidated();

		if ( _mask )
		{
			// TODO: should we reset any original clip depth
			//       specified by PlaceObject tag ?
			_mask->set_clip_depth(noClipDepthValue);
		}
		_mask = mask;
		if ( mask )
		{
			/// Mark the mask as a dynamic one
			mask->set_clip_depth(dynClipDepthValue); 
		}
	}


	/// Returns true if this character is a mask (either layer or dynamic mask)
	bool isMask() const
	{
		return isDynamicMask() || isMaskLayer();
	}

    virtual void set_name(const char* name) { _name = name; }

    const std::string& get_name() const { return _name; }

    /// Return true if this character can handle mouse events.
    //
    /// The default implementation returns false.
    ///
    virtual bool can_handle_mouse_event() const {
        return false;
    }

    // For edit_text support (Flash 5).  More correct way
    // is to do "text_character.text = whatever", via
    // set_member().
    virtual const char*	get_text_name() const { return ""; }

		// The Flash user can write moviclip="text", but it should not lead to crash
    virtual void set_text_value(const char* /*new_text*/) { }

	/// \brief
	/// Get our concatenated matrix (all our ancestor transforms,
	/// times our matrix). 
	///
	/// Maps from our local space into "world" space
	/// (i.e. root movie space).
	virtual matrix	get_world_matrix() const;

	/// \brief
	/// Get our concatenated color transform (all our ancestor transforms,
	/// times our cxform). 
	///
	/// Maps from our local space into normal color space.
	virtual cxform	get_world_cxform() const;

	/// Get the built-in function handlers code for the given event
	//
	/// NOTE: this function is only for getting statically-defined
	///       event handlers, which are the ones attached to a character
	///       with a PlaceObject2. It's the character's responsibility
	///       to properly fetch any user-defined event handler, which 
	///       are the ones attached to a character with ActionScript code.
	///
	std::auto_ptr<ExecutableCode> get_event_handler(const event_id& id) const;

	/// Set a built-in function handler for the given event
	//
	/// Mark the character as having mouse or Key event
	/// handlers if this is the case.
	///
	/// NOTE: this function is only for registering statically-defined
	///       event handlers, which are the ones attached to a character
	///       with a PlaceObject2. It's the character's responsibility
	///       to properly invoke any user-defined event handler, which 
	///       are the ones attached to a character with ActionScript code.
	///
	/// @param id
	///	The event triggering the handler.
	///
	/// @param code
	///	An action buffer to execute when given event is triggered.
	///	The buffer is externally owned (not copied), make sure it
	///	is kept alive for the whole lifetime of this character.
	///
	void add_event_handler(const event_id& id, const action_buffer& code);

	/// \brief
	/// Call this when a character get equipped
	/// with a Key event handler
	//
	/// TODO: provide a function to *unset*
	///       the flag. This should happen
	///       when Key event handler is
	///       set to undefined or equivalent..
	///
	virtual void has_key_event() {}

	/// \brief
	/// Call this when a character get equipped
	/// with a mouse event handler (move,down,up)
	//
	/// TODO: provide a function to *unset*
	///       the flag. This should happen
	///       when all mouse event handlers are
	///       set to undefined or equivalent..
	///
	virtual void has_mouse_event() {}

	/// Render this character
	virtual void	display() {}

    	/// Returns local, untransformed height of this character in TWIPS
	//
	/// Use getBounds() if you need more then simply the height.
	///
	float get_height() const
	{
		geometry::Range2d<float> bounds = getBounds();
		if ( bounds.isFinite() ) return bounds.height();
    		return 0;
	}

    	/// Returns local, untransformed width of this character in TWIPS
	//
	/// Use getBounds() if you need more then simply the width.
	///
	float get_width() const
	{
		geometry::Range2d<float> bounds = getBounds();
		if ( bounds.isFinite() ) return bounds.width();
    		return 0;
	}

    	/// Returns local, untransformed bounds of this character in TWIPS
	//
	/// The default implementation prints an error and returns a NULL rect.
	///
	/// Container characters (sprite and buttons) return the composite
	/// bounds of all their childrens, appropriaterly transformed with
	/// their local matrix.
	///
	virtual geometry::Range2d<float> getBounds() const
	{
		log_error("FIXME: character %s did not override the getBounds() method",
				typeid(*this).name());
		return geometry::Range2d<float>(geometry::nullRange);
	}

	/// Return true if the given point falls in this character's bounds
	//
	/// Point coordinates are in world TWIPS
	///
	bool pointInBounds(float x, float y) const
	{
		geometry::Range2d<float> bounds = getBounds();
		matrix wm = get_world_matrix();
		wm.transform(bounds);
		return bounds.contains(x, y);
	}

	/// Return true if the given point falls in this character's shape
	//
	/// Point coordinates are in world TWIPS
	///
	/// The default implementation warns about a missing
	/// override and invokes pointInBounds().
	///
	///
	virtual bool pointInShape(float x, float y) const
	{
		log_error("Character %s did not override pointInShape() - using pointInBounds() instead", typeid(*this).name());
		return pointInBounds(x, y);
	}

	/// Return true if the given point falls in this character's visible shape
	//
	/// Point coordinates are in world TWIPS
	///
	/// The default implementation returns false if the character is
	/// not visible, calling pointInBounds() otherwise.
	///
	/// Note that this is good for simple characters but needs
	/// to be overridden for characters with childs. When a
	/// character has childs it must take into account the case
	/// in which some childs are visible and some are not.
	///
	virtual bool pointInVisibleShape(float x, float y) const
	{
		if ( get_visible() ) return pointInShape(x, y);
		else return false;
	}

	/// Return the "relative" root of this character
	//
	/// The "relative" is the movie_instance created by
	/// the same SWF definition that contained the
	/// definition of this character.
	///
	/// TODO: what about programmatically created characters ?
	///	  which would their "relative" root be ?
	///
	/// The default implementation is to invoke get_root_movie
	/// against this character's parent.
	///
	virtual sprite_instance* get_root_movie();

	/// By default call get_root on the parent
	virtual movie_instance* get_root() const {
		return get_parent()->get_root();
	}

	/// Find the object which is one degree removed from us,
	/// given the relative pathname.
	///
	/// If the pathname is "..", then return our parent.
	/// If the pathname is ".", then return ourself.  If
	/// the pathname is "_level0" or "_root", then return
	/// the root movie.
	///
	/// Otherwise, the name should refer to one our our
	/// named characters, so we return it.
	///
	/// NOTE: In ActionScript 2.0, top level names (like
	/// "_root" and "_level0") are CASE SENSITIVE.
	/// Character names in a display list are CASE
	/// SENSITIVE. Member names are CASE INSENSITIVE.  Gah.
	///
	/// In ActionScript 1.0, everything seems to be CASE
	/// INSENSITIVE.
	///
	virtual as_object* get_path_element(string_table::key key)
	{
		return get_path_element_character(key);
	}

	/// Restart the character
	//
	/// This is only meaningful for sprite instances, but default
	/// it's a no-op.
	///
	/// It is needed by button_character_instance
	/// TODO: have button_character_instance cast to_movie()
	///       and drop this one
	virtual void	restart() { }

	/// Advance this character to next frame.
	//
	/// Character advancement is only meaningful for sprites
	/// and sprite containers (button characters) because
	/// sprites are the only characters that have frames.
	/// 
	/// Frame advancement include execution of all control tags.
	/// 
	virtual void advance(float /*delta_time*/)
	{
		// GNASH_REPORT_FUNCTION 
	}

	// TODO: verify if this is really needed (I guess not)
	virtual void	goto_frame(size_t /*target_frame*/) {}

	/// \brief
	/// Return true if PlaceObjects tag are allowed to move
	/// this character.
	//
	/// Once a character has been transformed by ActionScript,
	/// further transformation trought non-action SWF constrol tags
	/// is not allowed.
	///
	/// See scriptTransformed()
	///
	bool get_accept_anim_moves() const
	{
		return ! _scriptTransformed && ! _dynamicallyCreated;
	}

	/// Was this character dynamically created ?
	//
	/// "Dynamically created" means created trough ActionScript.
	///
	/// NOTE, With current code:
	///	- Characters created by means of a loadMovie are 
	///	  NOT set as dynamic (should check if they should)
	///	- Characters created by attachMovie ARE dynamic
	///	- Characters created by duplicateMovieClip ARE dynamic
	/// - Characters created by createEmptyMovieClip ARE dynamic
	///	- Characters created by new Video ARE dynamic
	///	- Characters created by createTextField ARE dynamic
	///
	///
	bool isDynamic() const {

		// WARNING: cannot use _timelinInfo for this, unless 
		// we'll provide a TimelineInfo object for top level movies
		// (_level#) and dynamically loaded movies too...
		// which would have no use except implementing
		// isDynamic(). Note that we have NO automated test for this, but
		// the "Magical Trevor 2" movie aborts due to a call to getBytesTotal
		// against the root movie, and bug #19844 show the effect with dynamically
		// loaded movies.

		// Anyway, any dynamically created character must NOT have a
		// _timelineInfo object (see setDynamic)
		assert(_dynamicallyCreated ? (_timelineInfo.get() == 0) : 1 );

		return _dynamicallyCreated;
	}

	/// Mark this character as dynamically created
	//
	/// "Dynamically created" means created trough ActionScript
	///
	/// TODO: deprecate this function, all characters should be
	///	  dynamic by default Unless setTimelineInfo is called.
	///
	void setDynamic() {
		assert(_timelineInfo.get() == NULL);
		_dynamicallyCreated = true;
		//assert(get_depth() > 0);
	}

	/// \brief
	/// Call this function when the sprite has been
	/// transformed due to ActionScript code.
	//
	/// This information will be used while executing
	/// PlaceObject tags in that ActionScript-transformed
	/// characters won't be allowed to be moved.
	///
	/// TODO: make protected
	///
	void transformedByScript() 
	{
		_scriptTransformed = true;
	}

	// Set whether this character should be rendered
	void set_visible(bool visible)
	{
		if (m_visible!=visible) set_invalidated(__FILE__, __LINE__);  
		m_visible = visible;      
	}

	// Return true if this character should be rendered
	bool get_visible() const { return m_visible; }

	virtual void	set_display_callback(void (*callback)(void*), void* user_ptr)
	{
	    m_display_callback = callback;
	    m_display_callback_user_ptr = user_ptr;
	}

	virtual void	do_display_callback()
	{
//		GNASH_REPORT_FUNCTION;
			
		if (m_display_callback)
		{
		    (*m_display_callback)(m_display_callback_user_ptr);
		}
	}

	/// Return mouse state in given variables
	//
	/// Use this to retrieve the last state of the mouse, as set via
	/// notify_mouse_state().  Coordinates are in PIXELS, NOT TWIPS.
	///
    	/// The default implementation calls get_mouse_state against
	/// the character's parent. The final parent (a sprite_instance)
	/// will delegate the call to it's associated movie_root, which
	/// does all the work.
	///
	virtual void get_mouse_state(int& x, int& y, int& buttons);

	/// These have been moved down from movie.h to remove that file
	/// from the inheritance chain. It is probably still a misdesign
	/// to require these functions for all characters.
	/// @{

	virtual float get_pixel_scale() const
	{
		return 1.0f;
	}

	virtual movie_definition *get_movie_definition()
	{
		return NULL;
	}

	/// ActionScript event handler.  Returns true if a handler was called.
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

	virtual void on_button_event(const event_id& id)
	{
		on_event(id);
	}

	/// \brief
	/// Return the topmost entity covering the given point
	/// and enabled to receive mouse events.
	//
	/// Return NULL if no "active" entity is found under the pointer.
	///
	/// Coordinates of the point are given in parent's coordinate space.
	/// This means that in order to convert the point to the local coordinate
	/// space you need to apply an inverse transformation using this
	/// character matrix. Example:
	///
	///	point p(x,y);
	///	get_matrix().transform_by_inverse(p);
	///	-- p is now in local coordinates
	///
	/// Don't blame me for this mess, I'm just trying to document the existing
	/// functions ... --strk
	///
	/// @param x
	/// 	X ordinate of the pointer, in parent's coordinate space.
	///
	/// @param y
	/// 	Y ordinate of the pointer, in parent's coordiante space.
	///
	virtual character* get_topmost_mouse_entity(float /* x */, float /* y */)
	{
		return NULL;
	}

	/// Returns true when the object (type) should get a instance name even 
	/// if none is provided manually.
	virtual bool wantsInstanceName()
	{
		return false; 
	}

	/// Returns true when the object (type) can be referenced by ActionScipt
	bool isActionScriptReferenceable()
	{
		// The way around
		// [ wantsInstanceName() returning isActionScriptReferenceable() ]
		// would be cleaner, but I wouldn't want to touch all files now.
		return wantsInstanceName();
	}

	/// @}

	/// \brief
	/// This function marks the character as being modified in aspect
	/// and keeps track of current invalidated bounds the first time
	/// it's called after each call to clear_invalidated().
	//
	/// Call this function *before* any change in this character
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
	/// NOTE: Marking a character as invalidated automatically marks
	///       it's parent as being invalidated.
	///
	/// @see \ref region_update
	///
	void set_invalidated();
	void set_invalidated(const char* debug_file, int debug_line);
	
	
	/// Calls set_invalidated() and extends old_invalidated_ranges to the
	/// given value so that also this area gets re-rendered (used when
	/// replacing characters).	
	void extend_invalidated_bounds(const InvalidatedRanges& ranges);
	
	
	/// Called by a child to signalize it has changed visibily. The
	/// difference to set_invalidated() is that *this* character does
	/// not need to redraw itself completely. This function will 
  /// recursively inform all it's parents of the change.
  void set_child_invalidated();
  
	/// Clear invalidated flag and reset m_old_invalidated_bounds to null.
	void clear_invalidated() {
		m_invalidated = false;
    m_child_invalidated = false;    
		m_old_invalidated_ranges.setNull();
	}
  
  
	/// \brief
	/// Add the character's invalidated bounds *to* the given ranges list.
	//
	/// NOTE that this method should include the bounds that it
	/// covered the last time clear_invalidated() was called,
	/// as those need to be rerendered as well (to clear the region
	/// previously occupied by this character).
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
	virtual void add_invalidated_bounds(InvalidatedRanges& ranges, bool force) = 0;

	/// Callback invoked whenever a character is placed on stage
	//
	/// This function must be called when the character is placed on
	/// stage for the first time.
	///
	virtual void stagePlacementCallback()
	{
	}

	/// Unload this instance from the stage.
	//
	/// This function must be called when the character is removed
	/// from the stage.
	/// It will take care of properly calling
	/// unload against any child characters and queuing the
	/// 'UNLOAD' event handler.
	///
	/// @return true if any onUnload event handler was defined
	///         by either this or any child characters, false
	///         otherwise.
	///
	virtual bool unload();

	/// Return true if this character was unloaded from the stage
	bool isUnloaded() { return _unloaded; }

	/// Mark this character as destroyed
	//
	/// A character should be destroyed when is removed from the display
	/// list and is not more needed for names (target) resolutions.
	/// Sprites are needed for names resolution whenever themselves
	/// or a contained object has an onUnload event handler defined, 
	/// in which case we want the event handler to find the 'this'
	/// variable w/out attempting to rebind it.
	///
	/// Note: this function can safely release most memory associated
	///       with the character as it will not be needed anymore.
	///
	virtual void destroy();

	/// Return true if this character was destroyed.
	//
	/// See destroy() for more info.
	///
	bool isDestroyed() const { return _destroyed; }

public: // istn't this 'public' reduntant ?

	/// Return full path to this object, in slash notation
	//
	/// e.g. "/sprite1/sprite2/ourSprite"
	///
	std::string getTargetPath() const;

	/// Return full path to this object, in dot notation
	//
	/// e.g. "_level0.sprite1.sprite2.ourSprite"
	///
	std::string getTarget() const;

	/// Set timeline information for this instance
	//
	/// Timeline info should be set only once, right after creation
	/// of a timeline instance. If this function is called twice
	/// Gnash will abort.
	/// Once timeline informations are added to a character, the
	/// character become a "timeline instance". If not timeline info
	/// is available, this is a dynamically-created character.
	/// See isDynamic();
	///
	/// @param depth
	///	Depth of first placement.
	///
	/// @param frame
	///	Frame of first placement. 0-based.
	///
	/// @param replacing
	///	True if this object was placed by a REPLACE tag
	///	(see PlaceObject2).
	///
	/// NOTE: if we want to compute TimelineInfo records once at
	/// 	  parse time and reuse pointers we'll just need to take
	///	  a TimelineInfo pointer as parameter, externally owned.
	///	  For now, we'll use a new object for each instance.
	///
	void setTimelineInfo(int depth, int frame, bool replacing)
	{
		assert(_timelineInfo.get()==NULL); // don't call twice !
		_timelineInfo.reset(new TimelineInfo(depth, frame, replacing));
	}

	/// Return timeline information, if this is a timeline instance
	//
	/// For dynamic instances, NULL is returned.
	/// Ownership of the returned object belong to this character.
	///
	TimelineInfo* getTimelineInfo() { return _timelineInfo.get(); }

#ifdef NEW_KEY_LISTENER_LIST_DESIGN
	boost::intrusive_ptr<as_function> getUserDefinedEventHandler(const std::string& name) const;
#endif

	/// Return true if this character is a selectable TextField
	//
	/// This method is used by Gui to set up an appropriate cursor
	/// for input textfields.
	///
	bool virtual isSelectableTextField() const { return false; }
};



}	// end namespace gnash

#ifdef DEBUG_SET_INVALIDATED
#define set_invalidated() set_invalidated(__FILE__, __LINE__)
#endif

#endif // GNASH_CHARACTER_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
