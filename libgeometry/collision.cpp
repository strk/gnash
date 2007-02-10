// collision.cpp	-- by Thatcher Ulrich <tu@tulrich.com>

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Misc helper code for doing collision tests.

/* $Id: collision.cpp,v 1.11 2007/02/10 18:33:03 nihilus Exp $ */

#include <cfloat>

#include "collision.h"

using namespace std;

ray_query::ray_query(const vec3& start_pos, const vec3& unit_direction, float distance)
	:
	m_start(start_pos),
	m_end(start_pos + unit_direction * distance),
	m_dir(unit_direction),
	m_length(distance)
{
	assert(m_length > 0);

	compute_inverses();
}


ray_query::ray_query(start_end_enum /* e */, const vec3& start_pos, const vec3& end_pos)
	:
	m_start(start_pos),
	m_end(end_pos)
{
	vec3	disp = m_end - m_start;
	m_length = disp.magnitude();
	assert(m_length > 0);

	if (m_length > 0)
	{
		m_dir = disp;
		m_dir /= m_length;
	}

	compute_inverses();
}


void	ray_query::compute_inverses()
// Compute m_inv_dir and m_inv_displacement
{
	vec3	disp(m_end);
	disp -= m_start;

	// Threshold, below which we don't want to compute 1/x.
	static const float	DANGER_LIMIT_MIN = 1e-25f;

	for (int i = 0; i < 3; i++)
	{
		// m_inv_dir
		float	comp = m_dir[i];
		if (fabsf(comp) <= DANGER_LIMIT_MIN)
		{
			m_inv_dir[i] = -FLT_MAX;	// arbitrary crap
			m_dir[i] = 0;	// don't tolerate tiny tiny component.  Client code will know not to use this axis.
		}
		else
		{
			m_inv_dir[i] = 1.0f / comp;
		}

		// m_inv_displacement
		comp = disp[i];
		if (fabsf(comp) <= DANGER_LIMIT_MIN)
		{
			m_inv_displacement[i] = -FLT_MAX;	// arbitrary crap
			m_dir[i] = 0;	// don't tolerate tiny tiny component.  Client code will know not to use this axis.
		}
		else
		{
			m_inv_displacement[i] = 1.0f / comp;
		}
	}
}




// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
