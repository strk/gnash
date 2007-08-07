// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
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

#include <sstream> // for ::print and ::toString

namespace gnash {

void	rect::read(stream* in)
{
	// TODO: find how many bytes are required to 
	//       read the whole rect and ensure they
	//       are available in the current tag
	//       using in->ensureBytes(x)
	//
	in->align();
	int	nbits = in->read_uint(5);
	float xmin = (float) in->read_sint(nbits);
	float xmax = (float) in->read_sint(nbits);
	float ymin = (float) in->read_sint(nbits);
	float ymax = (float) in->read_sint(nbits);

	// Check for swapped X or Y values
	if (xmax < xmin || ymax < ymin)
	{
		// We set invalid rectangles to NULL, but we might instead
		// want to actually swap the values IFF the proprietary player
		// does so. TODO: check it out.
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror("Invalid rectangle: xmin=%g xmax=%g "
			"ymin=%g ymax=%g. Read as Null.",
			xmin, xmax, ymin, ymax);
		);
		_range.setNull();
		return;
	}

	_range.setTo(xmin, ymin, xmax, ymax);

}

// Debug spew.
void	rect::print() const
{
	log_parse("%s", toString().c_str());
}


point
rect::get_corner(int i) const
// Get one of the rect verts.
{
	assert(i >= 0 && i < 4);
	return point(
		(i == 0 || i == 3) ? _range.getMinX() : _range.getMaxX(),
		(i < 2) ? _range.getMinY() : _range.getMaxY() );
}


void	rect::enclose_transformed_rect(const matrix& m, const rect& r)
// Set ourself to bound a rectangle that has been transformed
// by m.  This is an axial bound of an oriented (and/or
// sheared, scaled, etc) box.
{
	// Assertion unneeded as get_corner will
	// do that
	//assert ( ! _range.isNull() );

	// Get the transformed bounding box.
	point	p0, p1, p2, p3;
	m.transform(&p0, r.get_corner(0));
	m.transform(&p1, r.get_corner(1));
	m.transform(&p2, r.get_corner(2));
	m.transform(&p3, r.get_corner(3));

	enclose_point(p0.m_x, p0.m_y);
	expand_to_point(p1.m_x, p1.m_y);
	expand_to_point(p2.m_x, p2.m_y);
	expand_to_point(p3.m_x, p3.m_y);
}

void  rect::expand_to_rect(const rect& r) 
{
	_range.expandTo(r._range);
	return;
}	

void	rect::expand_to_transformed_rect(const matrix& m, const rect& r)
{
	// a null rectangle will add nothing, and a world
	// rectangle will always be world, no matter
	// how you transform it.
	if ( is_world() || r.is_null() ) return;
	if ( r.is_world() ) 
	{
		set_world();
		return;
	}

	// Get the transformed bounding box.
	point	p0, p1, p2, p3;
	m.transform(&p0, r.get_corner(0));
	m.transform(&p1, r.get_corner(1));
	m.transform(&p2, r.get_corner(2));
	m.transform(&p3, r.get_corner(3));

	// TODO: check for numeric overflow ?

	_range.expandTo(p0.m_x, p0.m_y);
	_range.expandTo(p1.m_x, p1.m_y);
	_range.expandTo(p2.m_x, p2.m_y);
	_range.expandTo(p3.m_x, p3.m_y);
}


void	rect::set_lerp(const rect& a, const rect& b, float t)
// Set this to the lerp of a and b.
{
	// Don't need to assert here, get_{x,y}_{min,max} will do that
	//assert ( ! a.is_null() ); // caller should check this
	//assert ( ! b.is_null() ); // caller should check this
	
	// TODO: remove double calls to get_{x,y}_{min,max}
	//       to remove double equivalent assertions
	float xmin = flerp(a.get_x_min(), b.get_x_min(), t);
	float ymin = flerp(a.get_y_min(), b.get_y_min(), t);
	float xmax = flerp(a.get_x_max(), b.get_x_max(), t);
	float ymax = flerp(a.get_y_max(), b.get_y_max(), t);

	_range.setTo(xmin, ymin, xmax, ymax);
}

void
rect::clamp(point& p) const
{
	// assertion unneeded, as get{Min,Max}{X,Y}
	// will assert itself is called against
	// a Null Range2d
	//assert( ! _range.isNull() );

	// nothing to do, point is surely inside
	if ( _range.isWorld() ) return;

	p.m_x = fclamp(p.m_x, _range.getMinX(), _range.getMaxX());
	p.m_y = fclamp(p.m_y, _range.getMinY(), _range.getMaxY());
}

std::string
rect::toString() const
{
	std::stringstream ss;
	ss << _range;
	return ss.str();
}


}	// end namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
