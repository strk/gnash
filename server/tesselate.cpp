// tesselate.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// 2D shape tesselator.  Actually a "trapezoidizer".  Takes line/curve
// segments with a style index, and outputs trapezoids.
//
// Comments on tesselation strategies:
//
// Current method basically works but has some problems.  It's slow,
// it generates lots of triangles where the poly edges are curvy, and
// occasionally it makes a leak around very thin shapes.
//
// Shewchuk's constrained delaunay triangulator: not appropriate for
// rendering; it's big thing is making roundish triangles, at the
// expense of triangle count.  That's the right tradeoff for FEA
// (Shewchuk's main target application), but the wrong tradeoff for
// rendering; we don't have any particular problem with skinny
// triangles.
//
// Something like "A Fast Trapezoidation Technique For Planar
// Polygons".  The key improvement there is to only cut from a
// vertex/event to the nearest adjacent edges, instead of slicing
// across the entire set of active edges.  This sounds good.
//
// Or, "FIST: Fast Industrial-Strength Triangulation of Polygons" by
// Martin Held.  This looks good; application is rasterization.  The
// funny thing here is that they explicitly deal with holes by
// inserting zero-area bridge edges to connect the holes with the
// outer boundary, to make one continuous poly.  This seems a little
// hacky and possibly tricky.
//
// Or, just optimize what we've got.  I took a quick look at the cairo
// project's trapezoider -- it's very similar to ours but seems to do
// its work in-place without resizing the edge list constantly, so it
// should be pretty quick.  They do some high-precision integer voodoo
// in their intercept calculation that I don't 100% understand.
//
//
// Findings re SWF polygons:
//
// (Based on experiments with Flash MX 2004; old movies seem to obey
// these rules as well, as far as I can tell):
//
// * no intersecting edges -- if you make intersections in the tool
// (even by stretching curve segments around), the tool inserts verts
// automatically.
//
// * left-fill and right-fill seem to be randomly selected.
//
// * individual paths (i.e. segment paths with a consistent set of
// fill styles) are not closed
//
// * there's no odd-even insanity; the tool cleans up the poly if you
// try to make self-intersections and flips and such.
//
// * if you duplicate the paths with two fill styles (so every path
// has just one fill style), and reverse the right-fill paths so they
// make left-fill paths, and then join open verts of the same fill
// style, you will be left with a set of non-intersecting closed polys
// with a consistent winding.  I.e. fairly well-behaved.  They may
// have holes though.
//
// Current outlook: Any of "A Fast Trapezoidation", "FIST", or
// cairo-style should work.  FIST will make the fewest tris (since it
// doesn't add any verts).  Current method can be optimized as well --
// can remove intersection checks (they're probably just harmful).
//
// FIST will probably be the most impervious to leaks, since it
// rationally deals with self-intersection by just overlapping parts
// of the tesselated poly.

/* $Id: tesselate.cpp,v 1.15 2007/11/10 11:51:43 strk Exp $ */

#include "tesselate.h"
#include "types.h"
#include "utility.h"
#include "container.h"
#include "Point2d.h"

#include <vector>

namespace gnash {
namespace tesselate {

	// Curve subdivision error tolerance.
	static float	s_tolerance = 1.0f;
	static trapezoid_accepter*	s_accepter = NULL;

	trapezoid_accepter::~trapezoid_accepter()
	{
	}

        class fill_segment
	{
	public:
		point	m_begin;
		point	m_end;
		int	m_left_style, m_right_style, m_line_style;

		fill_segment() {}

		fill_segment(
			const point& a,
			const point& b,
			int left_style,
			int right_style,
			int line_style)
			:
			m_begin(a),
			m_end(b),
			m_left_style(left_style),
			m_right_style(right_style),
			m_line_style(line_style)
		{
			// For rasterization, we want to ensure that
			// the segment always points towards positive
			// y...
			if (m_begin.y > m_end.y)
			{
				flip();
			}
		}

		void	flip()
		// Exchange end points, and reverse fill sides.
		{
			swap(&m_begin, &m_end);

			// swap fill styles...
			swap(&m_left_style, &m_right_style);
		}

		float	get_height() const
		// Return segment height.
		{
			assert(m_end.y >= m_begin.y);

			return m_end.y - m_begin.y;
		}
	};


	// More Renderer state.
	static std::vector<fill_segment>	s_current_segments;	// @@ should not dynamically resize this thing!
	static std::vector<point>	s_current_path;			// @@ should not dynamically resize this thing!
	static point	s_last_point;
	static int	s_current_left_style;
	static int	s_current_right_style;
	static int	s_current_line_style;
	static bool	s_shape_has_line;	// flag to let us skip the line rendering if no line styles were set when defining the shape.
	static bool	s_shape_has_fill;	// flag to let us skip the fill rendering if no fill styles were set when defining the shape.


	static void	peel_off_and_emit(int i0, int i1, float y0, float y1);


	void	begin_shape(trapezoid_accepter* accepter, float curve_error_tolerance)
	{
		assert(accepter);
		s_accepter = accepter;

		// ensure we're not already in a shape or path.
		// make sure our shape state is cleared out.
		assert(s_current_segments.size() == 0);
		s_current_segments.resize(0);

		assert(s_current_path.size() == 0);
		s_current_path.resize(0);

		assert(curve_error_tolerance > 0);
		if (curve_error_tolerance > 0)
		{
			s_tolerance = curve_error_tolerance;
		}
		else
		{
			s_tolerance = 1.0f;
		}

		s_current_line_style = -1;
		s_current_left_style = -1;
		s_current_right_style = -1;
		s_shape_has_fill = false;
		s_shape_has_line = false;
	}


	static int	compare_segment_y(const void* a, const void* b)
	// For sorting segments by m_begin.y, and then by height.
	{
		const fill_segment*	A = (const fill_segment*) a;
		const fill_segment*	B = (const fill_segment*) b;

		const float	ay0 = A->m_begin.y;
		const float	by0 = B->m_begin.y;

		if (ay0 < by0)
		{
			return -1;
		}
		else if (ay0 == by0)
		{
			const float	ah = A->get_height();
			const float	bh = B->get_height();

			if (ah < bh)
			{
				return -1;
			}
			else if (ah == bh)
			{
				return 0;
			}
			else
			{
				return 1;
			}
		}
		else
		{
			return 1;
		}
	}


	static int	compare_segment_x(const void* a, const void* b)
	// For sorting segments by m_begin.x, and then by m_end.y.
	{
		const fill_segment*	A = (const fill_segment*) a;
		const fill_segment*	B = (const fill_segment*) b;

		const float	ax0 = A->m_begin.x;
		const float	bx0 = B->m_begin.x;

		if (ax0 < bx0)
		{
			return -1;
		}
		else if (ax0 == bx0)
		{
			const float	ax1 = A->m_end.y;
			const float	bx1 = B->m_end.y;

			if (ax1 < bx1)
			{
				return -1;
			}
			else if (ax1 == bx1)
			{
				return 0;
			}
			else
			{
				return 1;
			}
		}
		else
		{
			return 1;
		}
	}


	void	output_current_segments()
	// Draw our shapes and lines, then clear the segment list.
	{
		if (s_shape_has_fill)
		{
			//
			// Output the trapezoids making up the filled shape.
			//

			// sort by begining y (smaller first), then by height (shorter first)
			qsort(
				&s_current_segments[0],
				s_current_segments.size(),
				sizeof(s_current_segments[0]),
				compare_segment_y);
		
			unsigned int	base = 0;
			while (base < s_current_segments.size())
			{
				float	        ytop = s_current_segments[base].m_begin.y;
				unsigned int	next_base = base + 1;
				for (;;)
				{
					if (next_base == s_current_segments.size()
					    || s_current_segments[next_base].m_begin.y > ytop)
					{
						break;
					}
					next_base++;
				}

				// sort this first part again by y
				qsort(
					&s_current_segments[base],
					next_base - base,
					sizeof(s_current_segments[0]),
					compare_segment_y);

				// s_current_segments[base] through s_current_segments[next_base - 1] is all the segs that start at ytop
				if (next_base >= s_current_segments.size()
				    || s_current_segments[base].m_end.y <= s_current_segments[next_base].m_begin.y)
				{
					// No segments start between ytop and
					// [base].m_end.y, so we can peel
					// off that whole interval and render
					// it right away.
					float	ybottom = s_current_segments[base].m_end.y;
					peel_off_and_emit(base, next_base, ytop, ybottom);

					while (base < s_current_segments.size()
					       && s_current_segments[base].m_end.y <= ybottom)
					{
						base++;
					}
				}
				else
				{
					float	ybottom = s_current_segments[next_base].m_begin.y;
					assert(ybottom > ytop);
					peel_off_and_emit(base, next_base, ytop, ybottom);

					// don't update base; it's still active.
				}
			}
		}
		
		s_current_segments.clear();
	}


	void	peel_off_and_emit(int i0, int i1, float y0, float y1)
	// Clip the interval [y0, y1] off of the segments from
	// s_current_segments[i0 through (i1-1)] and emit the clipped
	// trapezoids.  Modifies the values in s_current_segments.
	{
		assert(i0 < i1);

		if (y0 == y1)
		{
			// Don't bother doing any work...
			return;
		}

		// Peel off first.
		std::vector<fill_segment>	slab;	// @@ make this use static storage
		for (int i = i0; i < i1; i++)
		{
			fill_segment*	f = &s_current_segments[i];
			assert(f->m_begin.y == y0);
			assert(f->m_end.y >= y1);

			float	dy = f->m_end.y - f->m_begin.y;
			float	t = 1.0f;
			if (dy > 0)
			{
				t = (y1 - f->m_begin.y) / dy;
			}
			point	intersection;
			intersection.y = y1;
			intersection.x = f->m_begin.x + (f->m_end.y - f->m_begin.x) * t;

			// Peel off.
			slab.push_back(*f);
			slab.back().m_end = intersection;

			// Modify segment.
			s_current_segments[i].m_begin = intersection;
		}

		// Sort by x.
		qsort(&slab[0], slab.size(), sizeof(slab[0]), compare_segment_x);

		// Emit the trapezoids in this slab.
		if (slab.size() > 0
		    && slab[0].m_left_style == -1
		    && slab[0].m_right_style >= 0)
		{
			// Reverse sense of polygon fill!  Right fill style is in charge.
			for (unsigned int i = 0; i < slab.size() - 1; i++)
			{
				if (slab[i].m_right_style >= 0)
				{
					trapezoid	tr;
					tr.m_y0 = slab[i].m_begin.y;
					tr.m_y1 = slab[i].m_end.y;
					tr.m_lx0 = slab[i].m_begin.x;
					tr.m_lx1 = slab[i].m_end.y;
					tr.m_rx0 = slab[i + 1].m_begin.x;
					tr.m_rx1 = slab[i + 1].m_end.y;
					s_accepter->accept_trapezoid(slab[i].m_right_style, tr);
				}
			}
		}
		else
		{
			for (unsigned int i = 0; i < slab.size() - 1; i++)
			{
				if (slab[i].m_left_style >= 0)
				{
					trapezoid	tr;
					tr.m_y0 = slab[i].m_begin.y;
					tr.m_y1 = slab[i].m_end.y;
					tr.m_lx0 = slab[i].m_begin.x;
					tr.m_lx1 = slab[i].m_end.y;
					tr.m_rx0 = slab[i + 1].m_begin.x;
					tr.m_rx1 = slab[i + 1].m_end.y;
					s_accepter->accept_trapezoid(slab[i].m_left_style, tr);
				}
			}
		}
	}


	void	end_shape()
	{
		output_current_segments();

		s_accepter = NULL;

		s_current_path.clear();
	}


	void	begin_path(int style_left, int style_right, int line_style, float ax, float ay)
	// This call begins recording a sequence of segments, which
	// all share the same fill & line styles.  Add segments to the
	// shape using add_curve_segment() or add_line_segment(), and
	// call end_path() when you're done with this sequence.
	//
	// Pass in -1 for styles that you want to disable.  Otherwise pass in
	// the integral ID of the style for filling, to the left or right.
	{
		s_current_left_style = style_left;
		s_current_right_style = style_right;
		s_current_line_style = line_style;

		s_last_point.x = ax;
		s_last_point.y = ay;

		assert(s_current_path.size() == 0);
		s_current_path.resize(0);

		s_current_path.push_back(s_last_point);

		if (style_left != -1 || style_right != -1)
		{
			s_shape_has_fill = true;
		}

		if (line_style != -1)
		{
			s_shape_has_line = true;
		}
	}


	void	add_line_segment(float ax, float ay)
	// Add a line running from the previous anchor point to the
	// given new anchor point.
	{
		point	p(ax, ay);

		// s_current_segments is used for filling shapes.
		s_current_segments.push_back(
			fill_segment(
				s_last_point,
				p,
				s_current_left_style,
				s_current_right_style,
				s_current_line_style));

		s_last_point = p;

		s_current_path.push_back(p);
	}


	static void	curve(float p0x, float p0y, float p1x, float p1y, float p2x, float p2y)
	// Recursive routine to generate bezier curve within tolerance.
	{
#ifndef NDEBUG
		static int	recursion_count = 0;
		recursion_count++;
		if (recursion_count > 500)
		{
			abort();	// probably a bug!
		}
#endif // not NDEBUG

		// @@ use class point in here?

		// Midpoint on line between two endpoints.
		float	midx = (p0x + p2x) * 0.5f;
		float	midy = (p0y + p2y) * 0.5f;

		// Midpoint on the curve.
		float	qx = (midx + p1x) * 0.5f;
		float	qy = (midy + p1y) * 0.5f;

		float	dist = std::abs(midx - qx) + std::abs(midy - qy);

		if (dist < s_tolerance)
		{
			// Emit edge.
//			add_line_segment(qx, qy);
			add_line_segment(p2x, p2y);
		}
		else
		{
			// Error is too large; subdivide.
			curve(p0x, p0y, (p0x + p1x) * 0.5f, (p0y + p1y) * 0.5f, qx, qy);
			curve(qx, qy, (p1x + p2x) * 0.5f, (p1y + p2y) * 0.5f, p2x, p2y);
		}

#ifndef NDEBUG
		recursion_count--;
#endif // not NDEBUG
	}

	
	void	add_curve_segment(float cx, float cy, float ax, float ay)
	// Add a curve segment to the shape.  The curve segment is a
	// quadratic bezier, running from the previous anchor point to
	// the given new anchor point (ax, ay), with (cx, cy) acting
	// as the control point in between.
	{
		// Subdivide, and add line segments...
		curve(s_last_point.x, s_last_point.y, cx, cy, ax, ay);
	}


	void	end_path()
	// Mark the end of a set of edges that all use the same styles.
	{
		if (s_current_line_style >= 0 && s_current_path.size() > 1)
		{
			//
			// Emit our line.
			//
			s_accepter->accept_line_strip(s_current_line_style, &s_current_path[0], s_current_path.size());
		}

		s_current_path.resize(0);
	}


}	// end namespace tesselate
}	// end namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
