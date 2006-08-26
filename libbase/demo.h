// demo.h  -- Thatcher Ulrich <http://tulrich.com> 2005

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Some helper code for making graphical demos.  Covers OpenGL/SDL
// initialization, and some basic viewport navigation.


#ifndef DEMO_H
#define DEMO_H


#include "tu_config.h"


namespace demo
{
	// Open an OpenGL window with the given dimensions.
	void init_video(int width, int height, int depth);

	// A state object you can use to manage 2D scrolling & zooming.
	class nav2d_state
	{
	public:
		// 2d viewport state.
		float m_center_x;
		float m_center_y;
		float m_scale;

		// Current mouse state.
		int m_mouse_x;
		int m_mouse_y;
		int m_mouse_buttons;
		
		// Change in mouse position in pixels, this frame.
		int m_mouse_dx;
		int m_mouse_dy;

		nav2d_state()
			:
			m_center_x(0),
			m_center_y(0),
			m_scale(1),
			m_mouse_x(0),
			m_mouse_y(0),
			m_mouse_buttons(0),
			m_mouse_dx(0),
			m_mouse_dy(0)
		{
		}
	};

	// Checks and processes the SDL message queue.  Returns true
	// if the user wants to exit.
	//
	// TODO: do some kind of callback registry for handling misc
	// app-specific inputs
	bool update_nav2d(nav2d_state* state);

	// Sets the OpenGL projection & modelview according to the 2d
	// view state.
	void set_nav2d_viewport(const nav2d_state& state);
}


#endif // DEMO_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:

