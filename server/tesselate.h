// tesselate.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// 2D shape tesselator.  Actually a "trapezoidizer".  Takes line/curve
// segments with a style index, and outputs trapezoids.


#ifndef GNASH_TESSELATE_H
#define GNASH_TESSELATE_H


#include "gnash.h"
#include "types.h"
#include "Point2d.h" // for point


namespace gnash {
namespace tesselate {

struct trapezoid
{
	float	m_y0, m_y1;
	float	m_lx0, m_lx1, m_rx0, m_rx1;
};

// Interface for receiving the results of
// trapezoidation.
class trapezoid_accepter
{
public:
	virtual ~trapezoid_accepter();
	virtual void	accept_trapezoid(int style, const trapezoid& tr) = 0;
	virtual void	accept_line_strip(int style, const point coords[], int coord_count) = 0;
};

// A shape has one or more paths.  The paths in a
// shape are tesselated together using a typical
// polygon odd-even rule.
//
// The error tolerance tells the tesselator how much
// geometric error is allowed along curve edges.
void	begin_shape(trapezoid_accepter* accepter, float curve_error_tolerance);
void	end_shape();

// A path is enclosed within a shape.  If fill styles
// are active, a path should be a closed shape
// (i.e. the last point should match the first point).
// Set your styles before rendering the path; all
// segments in a path must have the same styles.
void	begin_path(int style_left, int style_right, int line_style, float ax, float ay);
void	add_line_segment(float ax, float ay);
void	add_curve_segment(float cx, float cy, float ax, float ay);
void	end_path();

class tesselating_shape {
public:
	virtual ~tesselating_shape();
	virtual void tesselate(float error_tolerance, 
			       trapezoid_accepter *accepter) const = 0;
};

} // end namespace tesselate
}// end namespace gnash


#endif // GNASH_TESSELATE_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
