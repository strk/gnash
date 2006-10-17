// shape.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Quadratic bezier outline shapes, the basis for most SWF rendering.

/* $Id: shape_character_def.h,v 1.5 2006/10/17 15:32:48 strk Exp $ */

#ifndef GNASH_SHAPE_CHARACTER_DEF_H
#define GNASH_SHAPE_CHARACTER_DEF_H


#include "character_def.h" // for inheritance of shape_character_def
#include "tesselate.h" 
#include "shape.h" // for path
#include "rect.h" // for composition


namespace gnash {

	/// \brief
	/// Represents the outline of one or more shapes, along with
	/// information on fill and line styles.
	class shape_character_def : public character_def, public tesselate::tesselating_shape
	{
	public:
		shape_character_def();
		virtual ~shape_character_def();

		virtual void	display(character* inst);
		bool	point_test_local(float x, float y);

		float	get_height_local() const;
		float	get_width_local() const;

		void	read(stream* in, int tag_type, bool with_style, movie_definition* m);
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
		friend class morph2_character_def;

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


#endif // GNASH_SHAPE_CHARACTER_DEF_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
