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

/* $Id: character.h,v 1.32 2006/12/06 10:21:32 strk Exp $ */

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

	/// Name of this character (if any)
	std::string	_name;

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

	bool m_visible;

	boost::intrusive_ptr<character> m_parent;

	/// Implement mouse-dragging for this movie.
	void do_mouse_drag();

	/// look for '.', '..', '_level0' and '_root'
	character* get_relative_target_common(const std::string& name);

	/// \brief
	/// Set when the visual aspect of this particular character or movie
	/// has been changed and redrawing is necessary.  
	bool m_invalidated;


	/// Bounds of character instance before invalidating it
	rect m_old_invalidated_bounds;
  	

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
	m_old_invalidated_bounds() 
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
	    set_invalidated();
	    m_matrix = m;
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

    void set_name(const char* name) { _name = name; }

    const std::string& get_name() const { return _name; }

    virtual bool can_handle_mouse_event() = 0;

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
	bool get_event_handler(const event_id& id, as_value* result)
	{
		std::map<event_id, as_value>::iterator it = \
			_event_handlers.find(id);
		if ( it == _event_handlers.end() ) return false;
		*result = it->second;
		return true;
	}

	void set_event_handler(const event_id& id, const as_value& method)
	{
		_event_handlers[id] = method;
		if (id.m_id == event_id::KEY_PRESS)
		{
			has_keypress_event();
		}
	}

		virtual void has_keypress_event() {}

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

    virtual size_t	get_current_frame() const { assert(0); return 0; }
    virtual bool	has_looped() const { assert(0); return false; }
    virtual void	restart() { /*assert(0);*/ }

    virtual void	advance(float /*delta_time*/)
    {
//	printf("%s:\n", __PRETTY_FUNCTION__); // FIXME:
    }	// for buttons and sprites

    virtual void	goto_frame(size_t /*target_frame*/) {}

    virtual bool	get_accept_anim_moves() const { return true; }

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

	void set_invalidated()
	{
	
		// flag already set, don't do anything
		if (m_invalidated) return;
	
		m_invalidated = true;
	  
		// Ok, at this point the instance will change it's
		// visual aspect after the
		// call to set_invalidated(). We save the *current*
		// position of the instance because this region must
		// be updated even (or first of all) if the character
		// moves away from here.
		m_old_invalidated_bounds.set_null();
		get_invalidated_bounds(&m_old_invalidated_bounds, true);
     
	}
  
	// Should be called by display()
	void clear_invalidated() {
		m_invalidated = false;    
		m_old_invalidated_bounds.set_null();
	}
  
  
	/// Checks if the character instance is still enclosed in
	/// the given bounds.  Otherwise it will expand them to surround
	/// the character. It is used to determine what area needs
	/// to be re-rendered. The coordinates are world coordinates.
	/// Only instances with m_invalidated flag set are checked unless
	/// force is set.
	virtual void get_invalidated_bounds(rect* bounds, bool force) = 0;


	
};



}	// end namespace gnash


#endif // GNASH_CHARACTER_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
