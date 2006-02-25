// shape.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Quadratic bezier outline shapes, the basis for most SWF rendering.


#ifndef GNASH_SHAPE_H
#define GNASH_SHAPE_H


#include "styles.h"


namespace gnash {
	struct character;
	struct stream;
	struct shape_character_def;
	namespace tesselate {
		struct trapezoid_accepter;
		struct tesselating_shape {
			virtual ~tesselating_shape();
			virtual void tesselate(float error_tolerance, 
					       trapezoid_accepter *accepter) const = 0;
		};
	}


	struct edge
	// Together with the previous anchor, defines a quadratic
	// curve segment.
	{
		edge();
		edge(float cx, float cy, float ax, float ay);
		void	tesselate_curve() const;
		bool	is_straight() const;
		
	//private:
		// *quadratic* bezier: point = p0 * t^2 + p1 * 2t(1-t) + p2 * (1-t)^2
		float	m_cx, m_cy;		// "control" point
		float	m_ax, m_ay;		// "anchor" point
	};


	struct path
	// A subset of a shape -- a series of edges sharing a single set
	// of styles.
	{
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

	struct mesh
	// For holding a pre-tesselated shape.
	{
		mesh();

		void	set_tri_strip(const point pts[], int count);

		void	display(const base_fill_style& style, float ratio) const;

		void	output_cached_data(tu_file* out);
		void	input_cached_data(tu_file* in);
	private:
		std::vector<Sint16>	m_triangle_strip;
	};


	struct line_strip
	// For holding a line-strip (i.e. polyline).
	{
		line_strip();
		line_strip(int style, const point coords[], int coord_count);

		void	display(const base_line_style& style, float ratio) const;

		int	get_style() const { return m_style; }
		void	output_cached_data(tu_file* out);
		void	input_cached_data(tu_file* in);
	private:
		int	m_style;
		std::vector<Sint16>	m_coords;
	};


	/// A whole shape, tesselated to a certain error tolerance.
	struct mesh_set
	{
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


	/// \brief
	/// Represents the outline of one or more shapes, along with
	/// information on fill and line styles.
	struct shape_character_def : public character_def, public tesselate::tesselating_shape
	{
		shape_character_def();
		virtual ~shape_character_def();

		virtual void	display(character* inst);
		bool	point_test_local(float x, float y);

		float	get_height_local();
		float	get_width_local();

		void	read(stream* in, int tag_type, bool with_style, movie_definition_sub* m);
		void	display(
			const matrix& mat,
			const cxform& cx,
			float pixel_scale,
			const std::vector<fill_style>& fill_styles,
			const std::vector<line_style>& line_styles) const;
		virtual void	tesselate(float error_tolerance, tesselate::trapezoid_accepter* accepter) const;
		const rect&	get_bound() const { return m_bound; }
		void	compute_bound(rect* r) const;	// @@ what's the difference between this and get_bound?

		void	output_cached_data(tu_file* out, const cache_options& options);
		void	input_cached_data(tu_file* in);

		const std::vector<fill_style>&	get_fill_styles() const { return m_fill_styles; }
		const std::vector<line_style>&	get_line_styles() const { return m_line_styles; }
		const std::vector<path>&	get_paths() const { return m_paths; }

		// morph uses this
		void	set_bound(const rect& r) { m_bound = r; /* should do some verifying */ }

	protected:
		friend struct morph2_character_def;

		// derived morph classes changes these
		std::vector<fill_style>	m_fill_styles;
		std::vector<line_style>	m_line_styles;
		std::vector<path>	m_paths;

	private:
		void	sort_and_clean_meshes() const;
		
		rect	m_bound;

		// Cached pre-tesselated meshes.
		mutable std::vector<mesh_set*>	m_cached_meshes;
	};

}	// end namespace gnash


#endif // GNASH_SHAPE_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
