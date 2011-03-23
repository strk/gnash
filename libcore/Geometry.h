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

#ifndef GNASH_GEOMETRY_H
#define GNASH_GEOMETRY_H

#include "dsodefs.h"
#include "SWFMatrix.h"
#include "SWFRect.h"
#include "Point2d.h"

#include <vector> // for path composition
#include <cmath> // sqrt


// Forward declarations
namespace gnash {
    class LineStyle;
}

namespace gnash { 

/// \brief
/// Defines an edge with a control point and an anchor point.
/// 
/// Could be a quadratic bezier curve, or a straight line(degenerated curve).
///
class Edge
{
public:
    
    // Quadratic bezier: point = p0 * t^2 + p1 * 2t(1-t) + p2 * (1-t)^2
    point cp; // control point, TWIPS
    point ap; // anchor    point, TWIPS

    Edge() 
        :
        cp(0, 0),
        ap(0, 0)
    {}
    
    Edge(boost::int32_t cx, boost::int32_t cy, boost::int32_t ax,
            boost::int32_t ay)
        :
        cp(cx, cy),
        ap(ax, ay)
    {}

    Edge(const Edge& from)
        : 
        cp(from.cp),
        ap(from.ap)
    {}

    Edge(const point& ncp, const point& nap)
        :
        cp(ncp),
        ap(nap)
    {}

    bool straight() const
    {
        return cp == ap;
    }
    
    /// Transform the edge according to the given SWFMatrix.
    void transform(const SWFMatrix& mat)
    {
        mat.transform(ap);
        mat.transform(cp);
    }

    /// Return squared distance between point pt and segment A-B
    static double
    squareDistancePtSeg(const point& p, const point& A, const point& B)
    {
        boost::int32_t dx = B.x - A.x;
        boost::int32_t dy = B.y - A.y;

        if ( dx == 0 && dy == 0 ) 
        {
            return p.squareDistance(A);
        }

        boost::int32_t pdx = p.x - A.x;
        boost::int32_t pdy = p.y - A.y;

        double u = (static_cast<double>(pdx) * dx + static_cast<double>(pdy) * dy ) /
            (static_cast<double>(dx)*dx + static_cast<double>(dy)*dy );

        if (u <= 0)
        {
            return p.squareDistance(A); 
        }

        if (u >= 1)
        {
            return p.squareDistance(B);
        }

        point px(A, B, u); // FIXME: this interpolation introduce a precision loss (point is int-based)
        return p.squareDistance(px);
    }

    /// Return distance between point pt and segment A-B
    static double
    distancePtSeg(const point& pt, const point& A, const point& B)
    {
        double square = squareDistancePtSeg(pt, A, B);
        return std::sqrt(square);
    }

    /// Find point of the quadratic curve defined by points A,C,B
    //
    /// @param A The first point
    /// @param C The second point (control point)
    /// @param B The third point (anchor point)
    /// @param ret The point to write result into
    /// @param t the step factor between 0 and 1
    ///

    static point
    pointOnCurve(const point& A, const point& C, const point& B, float t)
    {
        point Q1(A, C, t);
        point Q2(C, B, t);
        point R(Q1, Q2, t);

        return R;
    }

    /// Return square distance between point pt and the point on curve found by
    /// applying the T parameter to the quadratic bezier curve function
    //
    /// @param A The first point of the bezier curve
    /// @param C The second point of the bezier curve (control point)
    /// @param B The third point of the bezier curve (anchor point)
    /// @param p The point we want to compute distance from 
    /// @param t the step factor between 0 and 1
    ///
    static boost::int64_t squareDistancePtCurve(const point& A,
                         const point& C,
                         const point& B,
                         const point& p, float t)
    {
        return p.squareDistance( pointOnCurve(A, C, B, t) );
    }
};


/// A subset of a shape, a series of edges sharing a single set of styles. 
class DSOEXPORT Path
{
public:
    /// Left fill style index (1-based)
    unsigned m_fill0;

    /// Right fill style index (1-based)
    unsigned m_fill1;

    /// Line style index (1-based)
    unsigned m_line;

    /// Start point of the path
    point ap; 

    /// Edges forming the path
    std::vector<Edge> m_edges;

    /// This flag is set when the path is the first one of a new "sub-shape".
    /// All paths with a higher index in the list belong to the same 
    /// shape unless they have m_new_shape==true on their own.
    /// Sub-shapes affect the order in which outlines and shapes are rendered.
    bool m_new_shape;
    
    /// Default constructor
    //
    /// @param newShape
    ///    True if this path starts a new subshape
    ///
    Path(bool newShape = false)
        : 
        m_new_shape(newShape)
    {
        reset(0, 0, 0, 0, 0);
    }

    Path(const Path& from)
        : 
        m_fill0(from.m_fill0),
        m_fill1(from.m_fill1),
        m_line(from.m_line),
        ap(from.ap),
        m_edges(from.m_edges),
        m_new_shape(from.m_new_shape)                
    {
    }
    
    /// Initialize a path 
    //
    /// @param ax
    ///    X coordinate of path origin in TWIPS
    ///
    /// @param ay
    ///    Y coordinate in path origin in TWIPS
    ///
    /// @param fill0
    ///    Fill style index for left fill (1-based).
    ///    Zero means NO style.
    ///
    /// @param fill1
    ///    Fill style index for right fill (1-based)
    ///    Zero means NO style.
    ///
    /// @param line
    ///    Line style index for right fill (1-based).
    ///    Zero means NO style.
    ///
    /// @param newShape
    ///    True if this path starts a new subshape
    Path(boost::int32_t ax, boost::int32_t ay, 
            unsigned fill0, unsigned fill1, unsigned line, 
            bool newShape)
        :
        m_new_shape(newShape)
    {
        reset(ax, ay, fill0, fill1, line);
    }

    /// Re-initialize a path, maintaining the "new shape" flag untouched
    //
    /// @param ax
    ///    X coordinate of path origin in TWIPS
    ///
    /// @param ay
    ///    Y coordinate in path origin in TWIPS
    ///
    /// @param fill0
    ///    Fill style index for left fill
    ///
    /// @param fill1
    ///    Fill style index for right fill
    //
    /// @param line
    ///    Line style index for right fill
    ///
    void reset(boost::int32_t ax, boost::int32_t ay, 
            unsigned fill0, unsigned fill1, unsigned line)
    // Reset all our members to the given values, and clear our edge list.
    {
        ap.x = ax;
        ap.y = ay;
        m_fill0 = fill0;
        m_fill1 = fill1;
        m_line = line;

        m_edges.resize(0);
        assert(empty());
    }

    /// Expand given SWFRect to include bounds of this path
    //
    /// @param r
    ///    The rectangle to expand with our own bounds
    ///
    /// @param thickness
    ///    The thickess of our lines, half the thickness will
    ///    be added in all directions in swf8+, all of it will
    ///    in swf7-
    ///
    /// @param swfVersion
    ///    SWF version to use.
    ///
    void
    expandBounds(SWFRect& r, unsigned int thickness, int swfVersion) const
    {
        const Path&    p = *this;
        size_t nedges = m_edges.size();
        
        if ( ! nedges ) return; // this path adds nothing

        if (thickness)
        {
            // NOTE: Half of thickness would be enough (and correct) for
            // radius, but that would not match how Flash calculates the
            // bounds using the drawing API.                                                
            unsigned int radius = swfVersion < 8 ? thickness : thickness/2;

            r.expand_to_circle(ap.x, ap.y, radius);
            for (unsigned int j = 0; j<nedges; j++)
            {
                r.expand_to_circle(m_edges[j].ap.x, m_edges[j].ap.y, radius);
                r.expand_to_circle(m_edges[j].cp.x, m_edges[j].cp.y, radius);
            }
        }
        else
        {
            r.expand_to_point(ap.x, ap.y);
            for (unsigned int j = 0; j<nedges; j++)
            {
                r.expand_to_point(m_edges[j].ap.x, p.m_edges[j].ap.y);
                r.expand_to_point(m_edges[j].cp.x, p.m_edges[j].cp.y);
            }
        }
    }

    /// @{ Primitives for the Drawing API
    ///
    /// Name of these functions track Ming interface
    ///

    /// Draw a straight line.
    //
    /// Point coordinates are relative to path origin
    /// and expressed in TWIPS.
    ///
    /// @param x
    ///    X coordinate in TWIPS
    ///
    /// @param y
    ///    Y coordinate in TWIPS
    ///
    void 
    drawLineTo(boost::int32_t dx, boost::int32_t dy)
    {
        m_edges.push_back(Edge(dx, dy, dx, dy)); 
    }

    /// Draw a curve.
    //
    /// Offset values are relative to path origin and
    /// expressed in TWIPS.
    ///
    /// @param cx
    ///    Control point's X coordinate.
    ///
    /// @param cy
    ///    Control point's Y coordinate.
    ///
    /// @param ax
    ///    Anchor point's X ordinate.
    ///
    /// @param ay
    ///    Anchor point's Y ordinate.
    ///
    void 
    drawCurveTo(boost::int32_t cdx, boost::int32_t cdy, boost::int32_t adx, boost::int32_t ady)
    {
        m_edges.push_back(Edge(cdx, cdy, adx, ady)); 
    }

    /// Remove all edges and reset style infomation 
    void clear()
    {
        m_edges.resize(0);
        m_fill0 = m_fill1 = m_line = 0;
    }

    /// @} Primitives for the Drawing API


    /// Returns true if the last and the first point of the path match
    bool isClosed() const 
    {
        if (m_edges.empty()) return true;
        return m_edges.back().ap == ap; 
    }

    /// Close this path with a straight line, if not already closed
    void close()
    {
        if ( m_edges.empty() ) return;

        // Close it with a straight edge if needed
        const Edge& lastedge = m_edges.back();
        if ( lastedge.ap != ap )
        {
            Edge newedge(ap, ap);
            m_edges.push_back(newedge);
        }
    }

    /// \brief
    /// Return true if the given point is within the given squared distance
    /// from this path edges.
    //
    /// NOTE: if the path is empty, false is returned.
    ///
    bool
    withinSquareDistance(const point& p, double dist) const
    {
        size_t nedges = m_edges.size();

        if ( ! nedges ) return false;

        point px(ap);
        for (size_t i=0; i<nedges; ++i)
        {
            const Edge& e = m_edges[i];
            point np(e.ap);

            if (e.straight())
            {
                double d = Edge::squareDistancePtSeg(p, px, np);
                if ( d <= dist ) return true;
            }
            else
            {

                const point& A = px;
                const point& C = e.cp;
                const point& B = e.ap;

                // Approximate the curve to segCount segments
                // and compute distance of query point from each
                // segment.
                //
                // TODO: find an apprpriate value for segCount based
                //             on rendering scale ?
                //
                int segCount = 10; 
                point p0(A.x, A.y);
                for (int i=1; i<=segCount; ++i)
                {
                    float t1 = static_cast<float>(i) / segCount;
                    point p1 = Edge::pointOnCurve(A, C, B, t1);

                    // distance from point and segment being an approximation 
                    // of the curve 
                    double d = Edge::squareDistancePtSeg(p, p0, p1);
                    if ( d <= dist ) return true;

                    p0.setTo(p1.x, p1.y);
                }
            }
            px = np;
        }

        return false;
    }

    /// Transform all path coordinates according to the given SWFMatrix.
    void transform(const SWFMatrix& mat)
    {
        mat.transform(ap);
        std::vector<Edge>::iterator it = m_edges.begin(), ie = m_edges.end();
        for(; it != ie; it++)
        {
            (*it).transform(mat);
        }
    }        

    /// Set this path as the start of a new (sub)shape
    void setNewShape() 
    { 
            m_new_shape=true; 
    }

    /// Return true if this path starts a new (sub)shape
    bool getNewShape() const 
    { 
        return m_new_shape; 
    }

    /// Return true if this path contains no edges
    bool empty() const
    {
        return m_edges.empty();
    }

    /// Set the fill to use on the left side
    //
    /// @param f
    ///    The fill index (1-based).
    ///    When this path is added to a DefineShapeTag,
    ///    the index (decremented by 1) will reference an element
    ///    in the FillStyle vector defined for that shape.
    ///    If zero, no fill will be active.
    ///
    void setLeftFill(unsigned f)
    {
        m_fill0 = f;
    }

    unsigned getLeftFill() const
    {
        return m_fill0;
    }

    /// Set the fill to use on the left side
    //
    /// @param f
    ///    The fill index (1-based).
    ///    When this path is added to a DefineShapeTag,
    ///    the index (decremented by 1) will reference an element
    ///    in the FillStyle vector defined for that shape.
    ///    If zero, no fill will be active.
    ///
    void setRightFill(unsigned f)
    {
        m_fill1 = f;
    }

    unsigned getRightFill() const
    {
        return m_fill1;
    }

    /// Set the line style to use for this path
    //
    /// @param f
    ///    The LineStyle index (1-based).
    ///    When this path is added to a DefineShapeTag,
    ///    the index (decremented by 1) will reference an element
    ///    in the LineStyle vector defined for that shape.
    ///    If zero, no fill will be active.
    ///
    void setLineStyle(unsigned i)
    {
        m_line = i;
    }

    unsigned getLineStyle() const
    {
        return m_line;
    }

    /// Return the number of edges in this path
    size_t size() const
    {
        return m_edges.size();
    }

    /// Return a reference to the Nth edge 
    Edge& operator[] (size_t n)
    {
        return m_edges[n];
    }

    /// Return a const reference to the Nth edge 
    const Edge& operator[] (size_t n) const
    {
        return m_edges[n];
    }

    /// Returns true if this path begins a new subshape. <-- VERIFYME
    bool isNewShape() const
    {
        return m_new_shape;
    }

}; // end of class Path

namespace geometry
{

bool pointTest(const std::vector<Path>& paths,
    const std::vector<LineStyle>& lineStyles, boost::int32_t x,
    boost::int32_t y, const SWFMatrix& wm);

} // namespace geometry


} // namespace gnash

#endif // GNASH_GEOMETRY_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
