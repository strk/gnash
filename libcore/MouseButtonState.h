// Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.


#ifndef GNASH_MOUSE_BUTTON_STATE_H
#define GNASH_MOUSE_BUTTON_STATE_H

#include "InteractiveObject.h" 

// Forward declarations
namespace gnash {
	class MovieClip;
}

namespace gnash {

/// Helper to generate mouse events, given mouse state & history.
struct MouseButtonState
{

public:

	/// entity that currently owns the mouse pointer
	InteractiveObject* activeEntity;

	/// what's underneath the mouse right now
	InteractiveObject* topmostEntity;

	/// previous state of mouse button
	bool wasDown;	

	/// current state of mouse button
	bool isDown;	

	/// whether mouse was inside the active_entity last frame
	bool wasInsideActiveEntity;

	MouseButtonState()
		:
        activeEntity(0),
        topmostEntity(0),
		wasDown(false),
		isDown(false),
		wasInsideActiveEntity(false)
	{
	}

	/// Mark reachable objects (active and topmost entities)
	void markReachableResources() const {
		if (activeEntity) activeEntity->setReachable();
		if (topmostEntity) topmostEntity->setReachable();
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
