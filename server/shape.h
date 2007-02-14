// shape.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Quadratic bezier outline shapes, the basis for most SWF rendering.

/* $Id: shape.h,v 1.17 2007/02/14 13:10:11 strk Exp $ */

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

		bool	is_empty() const;

		bool	point_test(float x, float y);

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

		/// @} Primitives for the Drawing API

		/// Set the fill to use on the left side
		//
		/// @param f
		///	The fill index. When this path is
		///	added to a shape_character_def, the
		///	fill will reference the vector of
		///	fill_style defined for that shape
		///
		void setLeftFill(int f)
		{
			m_fill0 = f;
		}

		/// Set the fill to use on the left side
		//
		/// @param f
		///	The fill index. When this path is
		///	added to a shape_character_def, the
		///	fill will reference the vector of
		///	fill_style defined for that shape
		///
		void setRightFill(int f)
		{
			m_fill1 = f;
		}


	//private:

		/// Left fill style index (1-based)
		int	m_fill0;

		/// Right fill style index (1-based)
	        int	m_fill1;

		/// Line style index (1-based)
	        int	m_line;

		/// Path/shape origin
		float	m_ax, m_ay;

		/// Edges forming the path
		std::vector<edge> m_edges;

		/// ?
		bool	m_new_shape;
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
