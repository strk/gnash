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

#include "rect.h"
#include "log.h"
#include "stream.h"
#include "matrix.h"
#include "types.h" // for TWIPS_TO_PIXELS

namespace gnash {

rect::rect()
	:
	m_x_min(1),
	m_x_max(-1)
{
}

void rect::set_null()
{
	m_x_min = 1;
	m_x_max = -1;
}

bool rect::is_null() const
{
	return m_x_max < m_x_min;
}

void	rect::read(stream* in)
{
	in->align();
	int	nbits = in->read_uint(5);
	m_x_min = (float) in->read_sint(nbits);
	m_x_max = (float) in->read_sint(nbits);
	m_y_min = (float) in->read_sint(nbits);
	m_y_max = (float) in->read_sint(nbits);

//		IF_DEBUG(log_msg("rect::read() nbits = %d\n", nbits));
}

// Debug spew.
void	rect::print() const
{
	if ( is_null() )
	{
		log_parse(" null rectangle");
	}
	else
	{
	log_parse("xmin = %g, ymin = %g, xmax = %g, ymax = %g",
		TWIPS_TO_PIXELS(m_x_min),
		TWIPS_TO_PIXELS(m_y_min),
		TWIPS_TO_PIXELS(m_x_max),
		TWIPS_TO_PIXELS(m_y_max));
	}
}


bool	rect::point_test(float x, float y) const
// Return true if the specified point is inside this rect.
{
	if ( is_null() ) return false;

	if (x < m_x_min
	    || x > m_x_max
	    || y < m_y_min
	    || y > m_y_max)
	{
		return false;
	}
	else
	{
		return true;
	}
}


void rect::enclose_point(float x, float y)
{
	m_y_min = m_y_max = y;
	m_x_min = m_x_max = x;
}

void	rect::expand_to_point(float x, float y)
{
	if ( is_null() ) 
	{
		enclose_point(x,y);
	}
	else
	{
		m_x_min = fmin(m_x_min, x);
		m_y_min = fmin(m_y_min, y);
		m_x_max = fmax(m_x_max, x);
		m_y_max = fmax(m_y_max, y);
	}
}


point	rect::get_corner(int i) const
// Get one of the rect verts.
{
	assert ( ! is_null() ); // caller should check this
	assert(i >= 0 && i < 4);
	return point(
		(i == 0 || i == 3) ? m_x_min : m_x_max,
		(i < 2) ? m_y_min : m_y_max);
}


void	rect::enclose_transformed_rect(const matrix& m, const rect& r)
// Set ourself to bound a rectangle that has been transformed
// by m.  This is an axial bound of an oriented (and/or
// sheared, scaled, etc) box.
{
	assert ( ! r.is_null() ); // caller should check this
	// Get the transformed bounding box.
	point	p0, p1, p2, p3;
	m.transform(&p0, r.get_corner(0));
	m.transform(&p1, r.get_corner(1));
	m.transform(&p2, r.get_corner(2));
	m.transform(&p3, r.get_corner(3));

	m_x_min = m_x_max = p0.m_x;
	m_y_min = m_y_max = p0.m_y;
	expand_to_point(p1.m_x, p1.m_y);
	expand_to_point(p2.m_x, p2.m_y);
	expand_to_point(p3.m_x, p3.m_y);
}

void  rect::expand_to_rect(const rect& r) 
{
	if ( r.is_null() ) return; // nothing to do
	point tmp;
	tmp = r.get_corner(0);  expand_to_point(tmp.m_x, tmp.m_y);    
	tmp = r.get_corner(1);  expand_to_point(tmp.m_x, tmp.m_y);    
	tmp = r.get_corner(2);  expand_to_point(tmp.m_x, tmp.m_y);    
	tmp = r.get_corner(3);  expand_to_point(tmp.m_x, tmp.m_y);    
}	

void	rect::expand_to_transformed_rect(const matrix& m, const rect& r)
{
	// a null rectangle will always be null, no matter
	// how you transform it.
	if ( r.is_null() ) return;

	// Get the transformed bounding box.
	point	p0, p1, p2, p3;
	m.transform(&p0, r.get_corner(0));
	m.transform(&p1, r.get_corner(1));
	m.transform(&p2, r.get_corner(2));
	m.transform(&p3, r.get_corner(3));

	expand_to_point(p0.m_x, p0.m_y);
	expand_to_point(p1.m_x, p1.m_y);
	expand_to_point(p2.m_x, p2.m_y);
	expand_to_point(p3.m_x, p3.m_y);
}


void	rect::set_lerp(const rect& a, const rect& b, float t)
// Set this to the lerp of a and b.
{
	assert ( ! a.is_null() ); // caller should check this
	assert ( ! b.is_null() ); // caller should check this
	m_x_min = flerp(a.m_x_min, b.m_x_min, t);
	m_y_min = flerp(a.m_y_min, b.m_y_min, t);
	m_x_max = flerp(a.m_x_max, b.m_x_max, t);
	m_y_max = flerp(a.m_y_max, b.m_y_max, t);
}


}	// end namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
