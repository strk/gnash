// Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.


#ifndef GNASH_MOUSE_BUTTON_STATE_H
#define GNASH_MOUSE_BUTTON_STATE_H

#include "smart_ptr.h" // GNASH_USE_GC
#include "character_def.h"
#include "smart_ptr.h" // for composition and inlines
#include "InteractiveDisplayObject.h" // for use in intrusive_ptr

// Forward declarations
namespace gnash {
	class MovieClip;
}

namespace gnash {

/// Helper to generate mouse events, given mouse state & history.
struct MouseButtonState
{

public:

	/// Possible button states
	enum State {
        UP,
        DOWN
    };  

	/// entity that currently owns the mouse pointer
	boost::intrusive_ptr<InteractiveDisplayObject> activeEntity;

	/// what's underneath the mouse right now
	boost::intrusive_ptr<InteractiveDisplayObject> topmostEntity;

	/// previous state of mouse button
	bool previousButtonState;	

	/// current state of mouse button
	bool currentButtonState;	

	/// whether mouse was inside the active_entity last frame
	bool wasInsideActiveEntity;

	MouseButtonState()
		:
		previousButtonState(UP),
		currentButtonState(UP),
		wasInsideActiveEntity(false)
	{
	}

#ifdef GNASH_USE_GC
	/// Mark reachable objects (active and topmost entities)
	void markReachableResources() const
	{
		if ( activeEntity.get() ) activeEntity->setReachable();
		if ( topmostEntity.get() ) topmostEntity->setReachable();
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
