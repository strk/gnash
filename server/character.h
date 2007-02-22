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

// 
//
//

/* $Id: character.h,v 1.51 2007/02/22 17:28:04 udog Exp $ */

#ifndef GNASH_CHARACTER_H
#define GNASH_CHARACTER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gnash.h"
#include "types.h"
#include "container.h" // still needed ?
#include "utility.h"
#include "event_id.h" // for inlines
#include "as_object.h" // for inheritance
#include "rect.h" // for composition (invalidated bounds)
#include "matrix.h" // for composition
#include "log.h"

#include <map>
#include <cstdarg>
#include <string>
#include <cassert>
#include <typeinfo>

// Forward declarations
namespace gnash {
	class sprite_instance;
	class movie_instance;
}

namespace gnash {

/// Character is a live, stateful instance of a character_def.
//
/// It represents a single active element in a movie.
/// Inheritance from movie is an horrible truth!
///
class character : public as_object
{

public:

	typedef std::map<event_id, as_value> Events;

private:

	int		m_id;

	int		m_depth;
	cxform	m_color_transform;
	matrix	m_matrix;
	float	m_ratio;
	uint16_t	m_clip_depth;
	bool	m_enabled;
	Events _event_handlers;
	void	(*m_display_callback)(void*);
	void*	m_display_callback_user_ptr;


protected:

	/// Name of this character (if any)
	std::string	_name;

	bool m_visible;

	boost::intrusive_ptr<character> m_parent;

	/// Implement mouse-dragging for this movie.
	void do_mouse_drag();

	/// look for '.', 'this',  '..', '_parent', '_level0' and '_root'
	character* get_relative_target_common(const std::string& name);

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
	//
	/// Will be set by set_invalidated() and used by
	/// get_invalidated_bounds().
	///
	/// NOTE: this is currently initialized as the NULL rectangle.
	///
	rect m_old_invalidated_bounds;
  	
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

	static void onrollover_getset(const fn_call& fn);

	static void onrollout_getset(const fn_call& fn);

	static void onload_getset(const fn_call& fn);

	static void onpress_getset(const fn_call& fn);

	static void onrelease_getset(const fn_call& fn);

	static void onreleaseoutside_getset(const fn_call& fn);

	static void onmouseup_getset(const fn_call& fn);

	static void onmousedown_getset(const fn_call& fn);

	static void onmousemove_getset(const fn_call& fn);

	static void x_getset(const fn_call& fn);

	static void y_getset(const fn_call& fn);

	static void xscale_getset(const fn_call& fn);

	static void yscale_getset(const fn_call& fn);

	static void xmouse_getset(const fn_call& fn);

	static void ymouse_getset(const fn_call& fn);

	static void alpha_getset(const fn_call& fn);

	static void visible_getset(const fn_call& fn);

	static void width_getset(const fn_call& fn);

	static void height_getset(const fn_call& fn);

	static void rotation_getset(const fn_call& fn);

	static void parent_getset(const fn_call& fn);

	/// @} Common ActionScript getter-setters for characters

public:

    character(character* parent, int id)
	:
	m_id(id),
	m_depth(-1),
	m_ratio(0.0f),
	m_clip_depth(0),
	m_enabled(true),
	m_display_callback(NULL),
	m_display_callback_user_ptr(NULL),
	m_visible(true),
	m_parent(parent),
	m_invalidated(true),
	m_child_invalidated(true),
	m_old_invalidated_bounds(),
	_scriptTransformed(false),
	_dynamicallyCreated(false)
	{
	    assert((parent == NULL && m_id == -1)
		   || (parent != NULL && m_id >= 0));
	    assert(m_old_invalidated_bounds.is_null());
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
	      set_invalidated();
	      m_matrix = m;
	    }
	}
    const cxform&	get_cxform() const { return m_color_transform; }
    void	set_cxform(const cxform& cx) 
    { 
      set_invalidated(); 
      m_color_transform = cx;
    }
    void	concatenate_cxform(const cxform& cx) { m_color_transform.concatenate(cx); }
    void	concatenate_matrix(const matrix& m) { m_matrix.concatenate(m); }
    float	get_ratio() const { return m_ratio; }
    void	set_ratio(float f) {
      if (f!=m_ratio) set_invalidated(); 
      m_ratio = f;       
    }
    uint16_t	get_clip_depth() const { return m_clip_depth; }
    void	set_clip_depth(uint16_t d) { m_clip_depth = d; }

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

    // Event handler accessors.
	bool get_event_handler(const event_id& id, as_value* result) const
	{
		std::map<event_id, as_value>::const_iterator it = \
			_event_handlers.find(id);
		if ( it == _event_handlers.end() ) return false;
		*result = it->second;
		return true;
	}

	/// Set a function handler for the given event
	//
	/// Mark the character as having mouse or keypress event
	/// handlers if this is the case.
	///
	void set_event_handler(const event_id& id, const as_value& method);

	/// \brief
	/// Call this when a character get equipped
	/// with a keypress event handler
	//
	/// TODO: provide a function to *unset*
	///       the flag. This should happen
	///       when keypress event handler is
	///       set to undefined or equivalent..
	///
	virtual void has_keypress_event() {}

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

    // Movie interfaces.  By default do nothing.  sprite_instance and some others override these.
    virtual void	display() {}

    	/// Returns local, untransformed height of this character in TWIPS
	virtual float	get_height() const
	{
		log_error("a character class didn't override get_height: %s", typeid(*this).name());
    		return 0;
	}

    	/// Returns local, untransformed width of this character in TWIPS
	virtual float	get_width() const
	{
		log_error("a character class didn't override get_width: %s", typeid(*this).name());
		return 0;
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
	virtual movie_instance* get_root() {
		return get_parent()->get_root();
	}

	/// Find the character which is one degree removed from us,
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
	virtual character* get_relative_target(const std::string& name)
	{
		return get_relative_target_common(name);
	}

    //virtual size_t	get_current_frame() const { assert(0); return 0; }
    //virtual bool	has_looped() const { assert(0); return false; }

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
	/// "Dynamically created" means created trough ActionScript
	///
	bool isDynamic() const {
		return _dynamicallyCreated;
	}

	/// Mark this character as dynamically created
	//
	/// "Dynamically created" means created trough ActionScript
	///
	void setDynamic() {
		_dynamicallyCreated = true;
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


    virtual void	set_visible(bool visible) {
      if (m_visible!=visible) set_invalidated();  
      m_visible = visible;      
    }
    virtual bool	get_visible() const { return m_visible; }

    virtual void	set_display_callback(void (*callback)(void*), void* user_ptr)
	{
	    m_display_callback = callback;
	    m_display_callback_user_ptr = user_ptr;
	}

    virtual void	do_display_callback()
	{
//			GNASH_REPORT_FUNCTION;
			
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
	
	// TODO : make protected
	const std::map<event_id, as_value>& get_event_handlers() const
	{
	    return _event_handlers;
	}

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

	virtual void on_button_event(const event_id& id)
	{
		on_event(id);
	}

	virtual character* get_topmost_mouse_entity(float /* x */, float /* y */)
	{
		return NULL;
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
	
	/// Called by a child to signalize it has changed visibily. The
	/// difference to set_invalidated() is that *this* character does
	/// not need to redraw itself completely. This function will 
  /// recursively inform all it's parents of the change.
  void set_child_invalidated();
  
	/// Clear invalidated flag and reset m_old_invalidated_bounds to null.
	void clear_invalidated() {
		m_invalidated = false;
    m_child_invalidated = false;    
		m_old_invalidated_bounds.set_null();
	}
  
  
	/// \brief
	/// Expand the given rectangle to enclose this character's
	/// invalidated bounds.
	//
	/// NOTE that this method should include the bounds that it
	/// covered the last time clear_invalidated() was called,
	/// as those need to be rerendered as well (to clear the region
	/// previously occupied by this character).
	///
	/// It is used to determine what area needs to be re-rendered.
	/// The coordinates are world coordinates (in TWIPS).
	/// Only instances with m_invalidated flag set are checked unless
	/// force is set.
	///
	virtual void get_invalidated_bounds(rect* bounds, bool force) = 0;

	/// Construct this instance as an ActionScript object.
	//
	/// This function must be called when the character is placed on
	/// stage for the first time. It will take care of invoking
	/// the constructor of its associated class and calling the
	/// 'onConstruct' event handler.
	///
	/// Make sure this instance got an instance name before calling
	/// this method (it's needed for properly setting the "this" pointer
	/// when calling user-defined constructors).
	///
	virtual void construct()
	{
		on_event(event_id::CONSTRUCT);
	}

	
};



}	// end namespace gnash


#endif // GNASH_CHARACTER_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
