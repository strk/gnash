// Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.


#ifndef GNASH_MOUSE_BUTTON_STATE_H
#define GNASH_MOUSE_BUTTON_STATE_H


//#include "impl.h" // should get rid of this
#include "character_def.h"
#include "sound.h"
#include "smart_ptr.h" // for composition and inlines

// Forward declarations
namespace gnash {
	class sprite_instance;
}

namespace gnash {

/// Helper to generate mouse events, given mouse state & history.
class mouse_button_state
{

public:

	/// entity that currently owns the mouse pointer
	weak_ptr<movie>	m_active_entity;

	/// what's underneath the mouse right now
	weak_ptr<movie>	m_topmost_entity;

	/// previous state of mouse button
	bool	m_mouse_button_state_last;	

	/// current state of mouse button
	bool	m_mouse_button_state_current;	

	/// whether mouse was inside the active_entity last frame
	bool	m_mouse_inside_entity_last;

	mouse_button_state()
		:
		m_mouse_button_state_last(0),
		m_mouse_button_state_current(0),
		m_mouse_inside_entity_last(false)
	{
	}

};

}	// end namespace gnash


#endif // GNASH_MOUSE_BUTTON_STATE_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
