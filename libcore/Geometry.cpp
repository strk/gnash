//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "Geometry.h"

#include <cmath>

#include "log.h"
#include "LineStyle.h"

namespace gnash {
namespace geometry {

namespace {

// TODO: this should be moved to libgeometry or something
// Finds the quadratic bezier curve crossings with the line Y.
// The function can have zero, one or two solutions (cross1, cross2). The
// return value of the function is the number of solutions.
// x0, y0 = start point of the curve
// x1, y1 = end point of the curve (anchor, aka ax|ay)
// cx, cy = control point of the curve
// If there are two crossings, cross1 is the nearest to x0|y0 on the curve.
int curve_x_crossings(float x0, float y0, float x1, float y1,
    float cx, float cy, float y, float &cross1, float &cross2)
{
    int count=0;

    // check if any crossings possible
    if ( ((y0 < y) && (y1 < y) && (cy < y))
        || ((y0 > y) && (y1 > y) && (cy > y)) )
    {
        // all above or below -- no possibility of crossing
        return 0;
    }

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

    if (rad < 0)
    {
        return 0;
    }
    else
    {
        float q;
        float sqrt_rad = std::sqrt(rad);
        if (B < 0)
        {
            q = -0.5f * (B - sqrt_rad);
        }
        else
        {
            q = -0.5f * (B + sqrt_rad);
        }

        // The old-school way.
        // float t0 = (-B + sqrt_rad) / (2 * A);
        // float t1 = (-B - sqrt_rad) / (2 * A);

        if (q != 0)
        {
            float t1 = C / q;
            if (t1 >= 0 && t1 < 1)
            {
                float x_at_t1 =
                    x0 + 2 * (cx - x0) * t1 + (x1 + x0 - 2 * cx) * t1 * t1;

                count++;
                assert(count==1);
                cross1 = x_at_t1;             // order is important!
            }
        }

        if (A != 0)
        {
            float t0 = q / A;
            if (t0 >= 0 && t0 < 1)
            {
                float x_at_t0 =
                    x0 + 2 * (cx - x0) * t0 + (x1 + x0 - 2 * cx) * t0 * t0;

                count++;
                // order is important!
                if (count == 2) cross2 = x_at_t0;
                else cross1 = x_at_t0;
            }
        }

    }

    return count;
}

} // anonymous namespace

bool
pointTest(const std::vector<Path>& paths,
        const std::vector<LineStyle>& lineStyles, boost::int32_t x,
        boost::int32_t y, const SWFMatrix& wm)
{

    /*
    Principle:
    For the fill of the shape, we project a ray from the test point to the left
    side of the shape counting all crossings. When a line or curve segment is
    crossed we add 1 if the left fill style is set. Regardless of the left fill
    style we subtract 1 from the counter then the right fill style is set.
    This is true when the line goes in downward direction. If it goes upward,
    the fill styles are reversed.

    The final counter value reveals if the point is inside the shape (and
    depends on filling rule, see below).
    This method should not depend on subshapes and work for some malformed
    shapes situations:
    - wrong fill side (eg. left side set for a clockwise drawen rectangle)
    - intersecting paths
    */
    point pt(x, y);

    // later we will need non-zero for glyphs... (TODO)
    bool even_odd = true;  

    unsigned npaths = paths.size();
    int counter = 0;

    // browse all paths
    for (unsigned pno=0; pno<npaths; pno++)
    {
        const Path& pth = paths[pno];
        unsigned nedges = pth.m_edges.size();

        float next_pen_x = pth.ap.x;
        float next_pen_y = pth.ap.y;
        float pen_x, pen_y;

        if (pth.m_new_shape)
        {
            if (( even_odd && (counter % 2) != 0) ||
                 (!even_odd && (counter != 0)) )
            {
                // the point is inside the previous subshape, so exit now
                return true;
            }

            counter=0;
        }
        if (pth.empty()) continue;

        // If the path has a line style, check for strokes there
        if (pth.m_line != 0 )
        {
            assert(lineStyles.size() >= pth.m_line);
            const LineStyle& ls = lineStyles[pth.m_line-1];
            double thickness = ls.getThickness();
            if (! thickness )
            {
                thickness = 20; // at least ONE PIXEL thick.
            }
            else if ((!ls.scaleThicknessVertically()) &&
                    (!ls.scaleThicknessHorizontally()) )
            {
                // TODO: pass the SWFMatrix to withinSquareDistance instead ?
                double xScale = wm.get_x_scale();
                double yScale = wm.get_y_scale();
                thickness *= std::max(xScale, yScale);
            }
            else if (ls.scaleThicknessVertically() != 
                    ls.scaleThicknessHorizontally())
            {
                LOG_ONCE(log_unimpl("Collision detection for "
                            "unidirectionally scaled strokes"));
            }

            double dist = thickness / 2.0;
            double sqdist = dist * dist;
            if (pth.withinSquareDistance(pt, sqdist))
                return true;
        }

        // browse all edges of the path
        for (unsigned eno=0; eno<nedges; eno++)
        {
            const Edge& edg = pth.m_edges[eno];
            pen_x = next_pen_x;
            pen_y = next_pen_y;
            next_pen_x = edg.ap.x;
            next_pen_y = edg.ap.y;

            float cross1 = 0.0, cross2 = 0.0;
            int dir1 = 0, dir2 = 0; // +1 = downward, -1 = upward
            int crosscount = 0;

            if (edg.straight())
            {
                // ignore horizontal lines
                // TODO: better check for small difference?
                if (edg.ap.y == pen_y)  
                {
                    continue;
                }
                // does this line cross the Y coordinate?
                if ( ((pen_y <= y) && (edg.ap.y >= y))
                    || ((pen_y >= y) && (edg.ap.y <= y)) )
                {

                    // calculate X crossing
                    cross1 = pen_x + (edg.ap.x - pen_x) *
                        (y - pen_y) / (edg.ap.y - pen_y);

                    if (pen_y > edg.ap.y)
                        dir1 = -1;  // upward
                    else
                        dir1 = +1;  // downward

                    crosscount = 1;
                }
                else
                {
                    // no crossing found
                    crosscount = 0;
                }
            }
            else
            {
                // ==> curve case
                crosscount = curve_x_crossings(pen_x, pen_y, edg.ap.x, edg.ap.y,
                    edg.cp.x, edg.cp.y, y, cross1, cross2);
                dir1 = pen_y > y ? -1 : +1;
                dir2 = dir1 * (-1); // second crossing always in opposite dir.
            } // curve

            // ==> we have now:
            //  - one (cross1) or two (cross1, cross2) ray crossings (X
            //    coordinate)
            //  - dir1/dir2 tells the direction of the crossing
            //    (+1 = downward, -1 = upward)
            //  - crosscount tells the number of crossings

            // need at least one crossing
            if (crosscount == 0)
            {
                continue;
            }

            // check first crossing
            if (cross1 <= x)
            {
                if (pth.m_fill0 > 0) counter += dir1;
                if (pth.m_fill1 > 0) counter -= dir1;
            }

            // check optional second crossing (only possible with curves)
            if ( (crosscount > 1) && (cross2 <= x) )
            {
                if (pth.m_fill0 > 0) counter += dir2;
                if (pth.m_fill1 > 0) counter -= dir2;
            }

        }// for edge
    } // for path

    return ( (even_odd && (counter % 2) != 0) ||
             (!even_odd && (counter != 0)) );
}

} // namespace geometry
} // namespace gnash


