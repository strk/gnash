// Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.


#ifndef GNASH_MOUSE_BUTTON_STATE_H
#define GNASH_MOUSE_BUTTON_STATE_H

#include "smart_ptr.h" // GNASH_USE_GC
#include "character_def.h"
#include "smart_ptr.h" // for composition and inlines
#include "character.h" // for use in intrusive_ptr

// Forward declarations
namespace gnash {
	class sprite_instance;
}

namespace gnash {

/// Helper to generate mouse events, given mouse state & history.
class mouse_button_state
{

public:

	/// Possible button states
	enum state {

		/// Button is depressed
		UP=0,

		/// Button is pressed
		DOWN=1
	};

	/// entity that currently owns the mouse pointer
	boost::intrusive_ptr<character>	m_active_entity;

	/// what's underneath the mouse right now
	boost::intrusive_ptr<character>	m_topmost_entity;

	/// previous state of mouse button
	bool	m_mouse_button_state_last;	

	/// current state of mouse button
	bool	m_mouse_button_state_current;	

	/// whether mouse was inside the active_entity last frame
	bool	m_mouse_inside_entity_last;

	mouse_button_state()
		:
		m_mouse_button_state_last(UP),
		m_mouse_button_state_current(UP),
		m_mouse_inside_entity_last(false)
	{
	}

#ifdef GNASH_USE_GC
	/// Mark reachable objects (active and topmost entities)
	void markReachableResources() const
	{
		if ( m_active_entity.get() ) m_active_entity->setReachable();
		if ( m_topmost_entity.get() ) m_topmost_entity->setReachable();
	}
#endif // GNASH_USE_GC
};

}	// end namespace gnash


#endif // GNASH_MOUSE_BUTTON_STATE_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
