// view_state.h	-- Thatcher Ulrich <tu@tulrich.com>

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// "view_state", a struct that holds various important rendering
// states during render traversal.


#ifndef VIEW_STATE_H
#define VIEW_STATE_H


#include "geometry.h"
#include "cull.h"


struct view_state
// Description of basic rendering state.  Passed as an arg to
// visual::render().
{
	int	m_frame_number;
	matrix	m_matrix;
	plane_info	m_frustum[6];	// In local coordinates.

	vec3	get_viewpoint() const
	{
		vec3 v;
		m_matrix.apply_inverse_rotation(&v, -m_matrix.get_column(3));
		return v;
	}

	// @@ Should this contain the culling state?  Or keep that separate?

	// @@ Add transformation methods, perhaps lazy updating of frustum planes... ??
};


#endif // VIEW_STATE_H
