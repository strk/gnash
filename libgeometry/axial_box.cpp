// axial_box.cpp	-- by Thatcher Ulrich <tu@tulrich.com>

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Simple AABB structure


#include "axial_box.h"
#include "tu_random.h"
#include "utility.h"


vec3	axial_box::get_random_point() const
// Return a random point inside this box.
{
	return vec3(
		flerp(m_min[0], m_max[0], tu_random::get_unit_float()),
		flerp(m_min[1], m_max[1], tu_random::get_unit_float()),
		flerp(m_min[2], m_max[2], tu_random::get_unit_float()));
}


void	axial_box::set_enclosing(const axial_box& a)
// Ensure that the box encloses the box.
{
	m_min.x = fmin(m_min.x, a.get_min().x);
	m_min.y = fmin(m_min.y, a.get_min().y);
	m_min.z = fmin(m_min.z, a.get_min().z);
	m_max.x = fmax(m_max.x, a.get_max().x);
	m_max.y = fmax(m_max.y, a.get_max().y);
	m_max.z = fmax(m_max.z, a.get_max().z);

	assert(is_valid());
}


int	axial_box::get_longest_axis() const
// Return axis with the largest size.
{
	vec3	size = get_size();
	if (size.x > size.y)
	{
		if (size.x > size.z)
		{
			return 0;	// x is longest
		}
		return 2;	// z is longest
	}
	else
	{
		if (size.y > size.z)
		{
			return 1;	// y is longest
		}
		else return 2; 	// z is longest
	}
}





// local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
