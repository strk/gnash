// shape.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Quadratic bezier outline shapes, the basis for most SWF rendering.

/* $Id: shape.h,v 1.26 2007/11/05 08:06:03 strk Exp $ */

#ifndef GNASH_SHAPE_H
#define GNASH_SHAPE_H

#include "tu_config.h"
#include "styles.h"


// Forward declarations
namespace gnash {
	namespace tesselate {
		class tesselating_shape;
	}
}

namespace gnash {

	/// \brief
	/// Together with the previous anchor,
	/// defines a quadratic curve segment.
	class edge
	{
	public:
		edge();
		edge(float cx, float cy, float ax, float ay);
		void	tesselate_curve() const;
		bool	is_straight() const;

		/// Return squared distance between point pt and segment A-B
		static float squareDistancePtSeg(const point& pt, const point& A, const point& B);

		/// Return distance between point pt and segment A-B
		static float distancePtSeg(const point& pt, const point& A, const point& B);
		
	//private:
		// *quadratic* bezier: point = p0 * t^2 + p1 * 2t(1-t) + p2 * (1-t)^2
		float	m_cx, m_cy;		// "control" point
		float	m_ax, m_ay;		// "anchor" point
	};


	/// \brief
	/// A subset of a shape -- a series of edges sharing a single set
	/// of styles.
	class path
	{
	public:
		path();

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
		///
		path(float ax, float ay, int fill0, int fill1, int line);

		/// Re-initialize a path 
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

		/// Return true if this path contains no edges
		bool	is_empty() const;

		/// Return true if this path contains no edges
		bool	empty() const
		{
			return is_empty();
		}

		/// Ray crossing count.
		//
		/// Update ray crossing for the given query point using
		/// edges in this path. Used to detect point in shape.
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

		/// Push the path into the tesselator.
		void	tesselate() const;

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

	//private:

		/// Left fill style index (1-based)
		unsigned m_fill0;

		/// Right fill style index (1-based)
	        unsigned m_fill1;

		/// Line style index (1-based)
	        unsigned m_line;

		/// Path/shape origin
		float	m_ax, m_ay;

		/// Edges forming the path
		std::vector<edge> m_edges;

		/// ?
		bool m_new_shape;

	};

	/// For holding a pre-tesselated shape.
	class mesh
	{
	public:
		mesh();

		void	set_tri_strip(const point pts[], int count);


		void	output_cached_data(tu_file* out);
		void	input_cached_data(tu_file* in);
	private:
	  friend class triangulating_render_handler; 
		std::vector<int16_t>	m_triangle_strip;
	};


	/// For holding a line-strip (i.e. polyline).
	class line_strip
	{
	public:
		line_strip();
		line_strip(int style, const point coords[], int coord_count);

		int	get_style() const { return m_style; }
		void	output_cached_data(tu_file* out);
		void	input_cached_data(tu_file* in);
	private:
    friend class triangulating_render_handler; 
		int	m_style;
		std::vector<int16_t>	m_coords;
	};


	/// A whole shape, tesselated to a certain error tolerance.
	class DSOEXPORT mesh_set
	{
	public:
		mesh_set();
		mesh_set(const tesselate::tesselating_shape* sh,
			 float error_tolerance);

//		int	get_last_frame_rendered() const;
//		void	set_last_frame_rendered(int frame_counter);
		float	get_error_tolerance() const { return m_error_tolerance; }

		void	set_tri_strip(int style, const point pts[], int count);
		void	add_line_strip(int style, const point coords[], int coord_count);

		void	output_cached_data(tu_file* out);
		void	input_cached_data(tu_file* in);

	private:
    friend class triangulating_render_handler; 
//		int	m_last_frame_rendered;	// @@ Hm, we shouldn't spontaneously drop cached data I don't think...
		float	m_error_tolerance;
		std::vector<mesh>	m_meshes;	// One mesh per style.
		std::vector<line_strip>	m_line_strips;
	};


}	// end namespace gnash


#endif // GNASH_SHAPE_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
