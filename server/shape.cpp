// shape.cpp:  shape path, edge and meshes
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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


#include "shape.h"

#include "tu_file.h"
#include "rect.h"
#include "log.h"

#include <cfloat>
#include <map>

#ifdef DEBUG_POINT_ON_CURVE
# include <sstream>
#endif

#include <boost/bind.hpp>


namespace gnash {

//
// edge
//

/* public static */
point
edge::pointOnCurve(const point& A, const point& C, const point& B, float t)
{
	// See http://en.wikipedia.org/wiki/B%C3%A9zier_curve#Quadratic_B.C3.A9zier_curves

	if ( t == 0.0 ) return A;
	else if ( t == 1.0 ) return B;

	point Q1(A, C, t);
	point Q2(C, B, t);
	point R = point(Q1, Q2, t);

#ifdef DEBUG_POINT_ON_CURVE
	std::stringstream ss;
	ss <<  "A:" << A << " C:" << C << " B:" << B
		<< " T:" << t
		<< " Q1:" << Q1 << " Q2:" << Q2
		<< " R:" << R;
	log_debug("%s", ss.str().c_str());
#endif

	return R;
}

float
edge::distancePtSeg(const point& pt, const point& A, const point& B)
{
	float square = squareDistancePtSeg(pt, A, B);
	return sqrt(square);
}

float
edge::squareDistancePtSeg(const point& p, const point& A, const point& B)
{
	float dx = B.x - A.x;
	float dy = B.y - A.y;

        /* if start==end, then use pt distance */
        if ( dx == 0 && dy == 0 ) return p.squareDistance(A); 

	float pdx = p.x - A.x;
	float pdy = p.y - A.y;

        float u = (pdx * dx + pdy * dy) / (dx*dx + dy*dy);

        if (u<0)
	{
		//cout << "R was < 0 " << endl;
		return p.squareDistance(A); 
	}

        if (u>1)
	{
		//cout << "R was > 1 " << endl;
		return p.squareDistance(B);
	}

	point px;
	px.x = A.x + u * (B.x - A.x);
	px.y = A.y + u * (B.y - A.y);

	//cout << "R was between 0 and 1, u is " << u << " px : " << px.x << "," << px.y << endl;

	return p.squareDistance(px);
}

void
edge::transform(const matrix& mat)
{
  mat.transform(ap);
  mat.transform(cp);
}



//
// path
//


path::path(bool newShape)
    :
    m_new_shape(newShape)
{
    reset(0, 0, 0, 0, 0);
}

path::path(float ax, float ay, int fill0, int fill1, int line, bool newShape)
    :
    m_new_shape(newShape)
{
    reset(ax, ay, fill0, fill1, line);
}


void	path::reset(float ax, float ay, int fill0, int fill1, int line)
    // Reset all our members to the given values, and clear our edge list.
{
    ap.x = ax;
    ap.y = ay;
    m_fill0 = fill0;
    m_fill1 = fill1;
    m_line = line;

    m_edges.resize(0);

    assert(is_empty());
}


bool	path::is_empty() const
    // Return true if we have no edges.
{
    return m_edges.size() == 0;
}

void
path::expandBounds(rect& r, unsigned int thickness) const
{
        const path&	p = *this;

        size_t nedges = m_edges.size();
        if ( ! nedges ) return; // this path adds nothing

        if (thickness)
	{
		// NOTE: Half of thickness would be enough (and correct) for
		// radius, but that would not match how Flash calculates the
		// bounds using the drawing API.                        
		float radius = thickness;

		r.expand_to_circle(ap.x, ap.y, radius);
		for (unsigned int j = 0; j<nedges; j++)
		{
			r.expand_to_circle(m_edges[j].ap.x, m_edges[j].ap.y, radius);
			r.expand_to_circle(m_edges[j].cp.x, m_edges[j].cp.y, radius);
		}

		return;
	}

	r.expand_to_point(ap.x, ap.y);
	for (unsigned int j = 0; j<nedges; j++)
	{
		r.expand_to_point(m_edges[j].ap.x, p.m_edges[j].ap.y);
                r.expand_to_point(m_edges[j].cp.x, p.m_edges[j].cp.y);
	}
}

void
path::ray_crossing(int& ray_crossings, float x, float y) const
{
    if ( m_edges.empty() ) return;

    // Shoot a horizontal ray from (x,y) to the right, and
    // count the number of edge crossings.  An even number
    // of crossings means the point is outside; an odd
    // number means it's inside.

    float x0 = ap.x;
    float y0 = ap.y;

    for (int i = 0, n = m_edges.size(); i < n; i++) {
	const edge& e = m_edges[i];
	
	float x1 = e.ap.x;
	float y1 = e.ap.y;
	
	if (e.isStraight()) {
	    // Straight-line case.
	    
	    // See if (x0,y0)-(x1,y1) crosses (x,y)-(infinity,y)
	    
	    // Does the segment straddle the horizontal ray?
	    bool cross_up = (y0 < y && y1 > y);
	    bool cross_down = (!cross_up) && (y0 > y && y1 < y);
	    if (cross_up || cross_down)	{
		// Straddles.
		
		// Is the crossing point to the right of x?
		float dy = y1 - y0;

		// x_intercept = x0 + (x1 - x0) * (y - y0) / dy;
		float x_intercept_times_dy = x0 * dy + (x1 - x0) * (y - y0);

		// text x_intercept > x
				
		// factor out the division; two cases depending on sign of dy
		if (cross_up) {
		    assert(dy > 0);
		    if (x_intercept_times_dy > x * dy) {
			ray_crossings++;
		    }
		} else {
		    // dy is negative; reverse the inequality test
		    assert(dy < 0);
		    if (x_intercept_times_dy < x * dy) {
			ray_crossings++;
		    }
		}
	    }
	} else {
	    // Curve case.
	    float cx = e.cp.x;
	    float cy = e.cp.y;

	    // Find whether & where the curve crosses y
	    if ((y0 < y && y1 < y && cy < y)
		|| (y0 > y && y1 > y && cy > y)) {
		// All above or all below -- no possibility of crossing.
	    } else if (x0 < x && x1 < x && cx < x) {
		// All to the left -- no possibility of crossing to the right.
	    } else {
		// Find points where the curve crosses y.
		
		// Quadratic bezier is:
		//
		// p = (1-t)^2 * a0 + 2t(1-t) * c + t^2 * a1
		//
		// We need to solve for x at y.
		
		// Use the quadratic formula.
		
		// Numerical Recipes suggests this variation:
		// q = -0.5 [b +sgn(b) sqrt(b^2 - 4ac)]
		// x1 = q/a;  x2 = c/q;

		float A = y1 + y0 - 2 * cy;
		float B = 2 * (cy - y0);
		float C = y0 - y;

		float rad = B * B - 4 * A * C;
		if (rad < 0) {
		    // No real solutions.
		} else {
		    float q;
		    float sqrt_rad = sqrtf(rad);
		    if (B < 0) {
			q = -0.5f * (B - sqrt_rad);
		    } else {
			q = -0.5f * (B + sqrt_rad);
		    }

		    // The old-school way.
		    // float t0 = (-B + sqrt_rad) / (2 * A);
		    // float t1 = (-B - sqrt_rad) / (2 * A);

		    if (A != 0)	{
			float t0 = q / A;
			if (t0 >= 0 && t0 < 1) {
			    float x_at_t0 =
				x0 + 2 * (cx - x0) * t0 + (x1 + x0 - 2 * cx) * t0 * t0;
			    if (x_at_t0 > x) {
				ray_crossings++;
			    }
			}
		    }

		    if (q != 0)	{
			float t1 = C / q;
			if (t1 >= 0 && t1 < 1) {
			    float x_at_t1 =
				x0 + 2 * (cx - x0) * t1 + (x1 + x0 - 2 * cx) * t1 * t1;
			    if (x_at_t1 > x) {
				ray_crossings++;
			    }
			}
		    }
		}
	    }
	}

	x0 = x1;
	y0 = y1;
    }

    return;
}

void 
path::drawLineTo(float dx, float dy)
{
	m_edges.push_back(edge(dx, dy, dx, dy)); 
}

void 
path::drawCurveTo(float cdx, float cdy, float adx, float ady)
{
	m_edges.push_back(edge(cdx, cdy, adx, ady)); 
}

void
path::close()
{
	// nothing to do if path there are no edges
	if ( m_edges.empty() ) return;

	// Close it with a straight edge if needed
	const edge& lastedge = m_edges.back();
	if ( lastedge.ap.x != ap.x || lastedge.ap.y != ap.y )
	{
		edge newedge(ap.x, ap.y, ap.x, ap.y);
		m_edges.push_back(newedge);
	}
}

bool
path::withinSquareDistance(const point& p, float dist) const
{
	size_t nedges = m_edges.size();

	if ( ! nedges ) return false;

	point px(ap.x, ap.y);
	for (size_t i=0; i<nedges; ++i)
	{
		const edge& e = m_edges[i];
		point np(e.ap.x, e.ap.y);

		if ( e.isStraight() )
		{
			float d = edge::squareDistancePtSeg(p, px, np);
			if ( d < dist ) return true;
		}
		else
		{
			// It's a curve !

			const point& A = px;
			const point& C = e.cp;
			const point& B = e.ap;

			// TODO: early break if point is NOT in the area
			//       defined by the triangle ACB and it's square 
			//       distance from it is > then the requested one

			// Approximate the curve to segCount segments
			// and compute distance of query point from each
			// segment.
			//
			// TODO: find an apprpriate value for segCount based
			//       on rendering scale ?
			//
			int segCount = 10; 
			point p0 = A;
			for (int i=1; i<=segCount; ++i)
			{
				float t1 = (float)i/segCount;
				point p1 = edge::pointOnCurve(A, C, B, t1);

				// distance from point and segment being an approximation
				// of the curve 
				float d = edge::squareDistancePtSeg(p, p0, p1);

				//float d = edge::squareDistancePtCurve(A, C, B, p, t);
				//log_debug("Factor %26.26g, distance %g (asked %g)", t, sqrt(d), sqrt(dist));
				if ( d <= dist ) return true;

				p0 = p1;
			}
		}

		px = np;
	}

	return false;
}

void
path::transform(const matrix& mat)
{
  using namespace boost;
  
  mat.transform(ap);
  std::for_each(m_edges.begin(), m_edges.end(),
                bind(&edge::transform, _1, ref(mat)));                
}

}	// end namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
