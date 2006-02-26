// collision.h	-- by Thatcher Ulrich <tu@tulrich.com>

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Basic types needed for doing collision queries.


#ifndef COLLISION_H
#define COLLISION_H


#include "geometry.h"


// Actually a line-segment query.
struct ray_query
{
	ray_query(const vec3& start_pos, const vec3& unit_direction, float distance);

	enum start_end_enum { start_end };
	ray_query(start_end_enum e, const vec3& start_pos, const vec3& end_pos);

	// Internal helper to compute m_inv_*
	void	compute_inverses();

	vec3	m_start;
	vec3	m_end;
	vec3	m_dir;
	vec3	m_inv_dir;	// 1/x for each component of m_dir
	vec3	m_inv_displacement;	// 1/x for each component of (m_end - m_start)
	float	m_length;
};


#endif // COLLISION_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:

