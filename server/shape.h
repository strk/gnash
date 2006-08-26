// shape.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Quadratic bezier outline shapes, the basis for most SWF rendering.


#ifndef GNASH_SHAPE_H
#define GNASH_SHAPE_H


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
		path(float ax, float ay, int fill0, int fill1, int line);

		void	reset(float ax, float ay, int fill0, int fill1, int line);
		bool	is_empty() const;

		bool	point_test(float x, float y);

		// Push the path into the tesselator.
		void	tesselate() const;

	//private:
		int	m_fill0, m_fill1, m_line;
		float	m_ax, m_ay;	// starting point
		std::vector<edge>	m_edges;
		bool	m_new_shape;
	};

	/// For holding a pre-tesselated shape.
	class mesh
	{
	public:
		mesh();

		void	set_tri_strip(const point pts[], int count);

		void	display(const base_fill_style& style, float ratio) const;

		void	output_cached_data(tu_file* out);
		void	input_cached_data(tu_file* in);
	private:
		std::vector<int16_t>	m_triangle_strip;
	};


	/// For holding a line-strip (i.e. polyline).
	class line_strip
	{
	public:
		line_strip();
		line_strip(int style, const point coords[], int coord_count);

		void	display(const base_line_style& style, float ratio) const;

		int	get_style() const { return m_style; }
		void	output_cached_data(tu_file* out);
		void	input_cached_data(tu_file* in);
	private:
		int	m_style;
		std::vector<int16_t>	m_coords;
	};


	/// A whole shape, tesselated to a certain error tolerance.
	class mesh_set
	{
	public:
		mesh_set();
		mesh_set(const tesselate::tesselating_shape* sh,
			 float error_tolerance);

//		int	get_last_frame_rendered() const;
//		void	set_last_frame_rendered(int frame_counter);
		float	get_error_tolerance() const { return m_error_tolerance; }

		void display(
			const matrix& m,
			const cxform& cx,
			const std::vector<fill_style>& fills,
			const std::vector<line_style>& line_styles) const;

		void display(
			const matrix& m,
			const cxform& cx,
			const std::vector<morph_fill_style>& fills,
			const std::vector<morph_line_style>& line_styles,
			float ratio) const;

		void	set_tri_strip(int style, const point pts[], int count);
		void	add_line_strip(int style, const point coords[], int coord_count);

		void	output_cached_data(tu_file* out);
		void	input_cached_data(tu_file* in);

	private:
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
