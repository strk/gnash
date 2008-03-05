// shape.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Quadratic bezier outline shapes, the basis for most SWF rendering.


#ifndef GNASH_SHAPE_H
#define GNASH_SHAPE_H

#include "tu_config.h"
#include "styles.h"

#include <vector> // for path composition


// Forward declarations
namespace gnash {
	class rect; // for path::expandBounds
}

namespace gnash {

	/// \brief
	/// Together with the previous anchor,
	/// defines a quadratic curve segment.
	class edge
	{
	public:
		edge()
			:
			cp(),
			ap()
		{
		}

		edge(float cx, float cy, float ax, float ay)
			:
    			cp(cx, cy),
			ap(ax, ay)
		{
		}

		edge(point ncp, point nap)
			:
			cp(ncp),
			ap(nap)
		{
		}

		bool isStraight() const
		{
			return cp == ap;
		}

		bool is_straight() const { return isStraight(); }
		
		/// Transform the edge according to the given matrix.
		void transform(const matrix& mat);

		/// Return squared distance between point pt and segment A-B
		static float squareDistancePtSeg(const point& pt, const point& A, const point& B);

		/// Return distance between point pt and segment A-B
		static float distancePtSeg(const point& pt, const point& A, const point& B);

		/// Find point of the quadratic curve defined by points A,C,B
		//
		/// @param A The first point
		/// @param C The second point (control point)
		/// @param B The third point (anchor point)
		/// @param ret The point to write result into
		/// @param t the step factor between 0 and 1
		///
		static point pointOnCurve(const point& A, const point& C, const point& B, float t);

		/// Return square distance between point pt and the point on curve found by
		/// applying the T parameter to the quadratic bezier curve function
		//
		/// @param A The first point of the bezier curve
		/// @param C The second point of the bezier curve (control point)
		/// @param B The third point of the bezier curve (anchor point)
		/// @param p The point we want to compute distance from 
		/// @param t the step factor between 0 and 1
		///
		static float squareDistancePtCurve(const point& A, const point& C, const point& B, const point& p, float t)
		{
			return p.squareDistance( pointOnCurve(A, C, B, t) );
		}
		
	//private:
		// *quadratic* bezier: point = p0 * t^2 + p1 * 2t(1-t) + p2 * (1-t)^2
		point cp; // "control" point
		point ap; // "anchor" point
	};


	/// \brief
	/// A subset of a shape -- a series of edges sharing a single set
	/// of styles.
	class path
	{
	public:
		/// Default constructor
		//
		/// @param newShape
		///	True if this path starts a new subshape
		///
		path(bool newShape=false);

		/// Initialize a path 
		//
		/// @param ax
		///	X coordinate of path origin in TWIPS
		///
		/// @param ay
		///	Y coordinate in path origin in TWIPS
		///
		/// @param fill0
		///	Fill style index for left fill (1-based).
		///	Zero means NO style.
		///
		/// @param fill1
		///	Fill style index for right fill (1-based)
		///	Zero means NO style.
		///
		/// @param line
		///	Line style index for right fill (1-based).
		///	Zero means NO style.
		///
		/// @param newShape
		///	True if this path starts a new subshape
		///
		///
		path(float ax, float ay, int fill0, int fill1, int line, bool newShape=false);

		/// Re-initialize a path, maintaining the "new shape" flag untouched
		//
		/// @param ax
		///	X coordinate of path origin in TWIPS
		///
		/// @param ay
		///	Y coordinate in path origin in TWIPS
		///
		/// @param fill0
		///	Fill style index for left fill
		///
		/// @param fill1
		///	Fill style index for right fill
		//
		/// @param line
		///	Line style index for right fill
		///
		void	reset(float ax, float ay, int fill0, int fill1, int line);

		/// Set this path as the start of a new (sub)shape
		void setNewShape() { m_new_shape=true; }

		/// Return true if this path starts a new (sub)shape
		bool getNewShape() const { return m_new_shape; }

		/// Return true if this path contains no edges
		bool	is_empty() const;

		/// Return true if this path contains no edges
		bool	empty() const
		{
			return is_empty();
		}

		/// Ray crossing count. (OBSOLETE)
		//
		/// Update ray crossing for the given query point using
		/// edges in this path. Was used to detect point in shape,
    /// but now we're using a different method.
		///
		/// @param ray_crossings
		///	Number of crossings, updated by this method.
		///
		/// @param x
		///     X ordinate of the query point, in local coordinate space.
		///
		/// @param y
		///     Y ordinate of the query point, in local coordinate space.
		///
		void ray_crossing(int& ray_crossings, float x, float y) const;

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
		///	X coordinate in TWIPS
		///
		/// @param y
		///	Y coordinate in TWIPS
		///
		void drawLineTo(float x, float y);

		/// Draw a curve.
		//
		/// Offset values are relative to path origin and
		/// expressed in TWIPS.
		///
		/// @param cx
		///	Control point's X coordinate.
		///
		/// @param cy
		///	Control point's Y coordinate.
		///
		/// @param ax
		///	Anchor point's X ordinate.
		///
		/// @param ay
		///	Anchor point's Y ordinate.
		///
		void drawCurveTo(float cx, float cy, float ax, float ay);

		/// Remove all edges and reset style infomation 
		void clear()
		{
			m_edges.resize(0);
			m_fill0 = m_fill1 = m_line = 0;
		}

		/// @} Primitives for the Drawing API

		/// Set the fill to use on the left side
		//
		/// @param f
		///	The fill index (1-based).
		///	When this path is added to a shape_character_def,
		///	the index (decremented by 1) will reference an element
		///	in the fill_style vector defined for that shape.
		///	If zero, no fill will be active.
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
		///	The fill index (1-based).
		///	When this path is added to a shape_character_def,
		///	the index (decremented by 1) will reference an element
		///	in the fill_style vector defined for that shape.
		///	If zero, no fill will be active.
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
		///	The line_style index (1-based).
		///	When this path is added to a shape_character_def,
		///	the index (decremented by 1) will reference an element
		///	in the line_style vector defined for that shape.
		///	If zero, no fill will be active.
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
		edge& operator[] (size_t n)
		{
			return m_edges[n];
		}

		/// Return a const reference to the Nth edge 
		const edge& operator[] (size_t n) const
		{
			return m_edges[n];
		}

		/// Close this path with a straight line, if not already closed
		void close();

		/// \brief
		/// Return true if the given point is withing the given squared distance
		/// from this path edges.
		//
		/// NOTE: if the path is empty, false is returned.
		///
		bool withinSquareDistance(const point& p, float dist) const;


		/// Expand given rect to include bounds of this path
		//
		/// @param r
		///	The rectangle to expand with our own bounds
		///
		/// @param thickness
		///	The thickess of our lines, half the thickness will
		///	be added in all directions
		///
		void expandBounds(rect& r, unsigned int thickness) const;

		bool isNewShape() const
		{
			return m_new_shape;
		}
		
		/// Transform all path coordinates according to the given matrix.
		void transform(const matrix& mat);

	//private:

		/// Left fill style index (1-based)
		unsigned m_fill0;

		/// Right fill style index (1-based)
	        unsigned m_fill1;

		/// Line style index (1-based)
	        unsigned m_line;

		/// Path/shape origin 
		point ap; 

		/// Edges forming the path
		std::vector<edge> m_edges;

		/// This flag is set when the path is the first one of a new "sub-shape".
		/// All paths with a higher index in the list belong to the same 
		/// shape unless they have m_new_shape==true on their own.
		/// Sub-shapes affect the order in which outlines and shapes are rendered.
		bool m_new_shape;

	};

}	// end namespace gnash


#endif // GNASH_SHAPE_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
