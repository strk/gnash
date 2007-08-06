// shape.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Quadratic bezier outline shapes, the basis for most SWF rendering.

/* $Id: shape_character_def.h,v 1.14 2007/08/06 03:30:19 strk Exp $ */

#ifndef GNASH_SHAPE_CHARACTER_DEF_H
#define GNASH_SHAPE_CHARACTER_DEF_H


#include "character_def.h" // for inheritance of shape_character_def
#include "tesselate.h" 
#include "shape.h" // for path
#include "rect.h" // for composition


namespace gnash {
	class cxform;
	class matrix;
}

namespace gnash {

	/// \brief
	/// Represents the outline of one or more shapes, along with
	/// information on fill and line styles.
	//
	/// Inheritance from tesselating_shape is only needed to expose a known interface
	/// for mesh_set class use.
	///
	class shape_character_def : public character_def, public tesselate::tesselating_shape
	{
	public:

		typedef std::vector<fill_style> FillStyleVect;
		typedef std::vector<line_style> LineStyleVect;
		typedef std::vector<path> PathVect;

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

		/// Get cached bounds of this shape.
		const rect&	get_bound() const { return m_bound; }

		/// Compute bounds by looking at the component paths
		void	compute_bound(rect* r) const;

		// deprecated
		//void	output_cached_data(tu_file* out, const cache_options& options);
		//void	input_cached_data(tu_file* in);

		const FillStyleVect& get_fill_styles() const { return m_fill_styles; }
		const LineStyleVect& get_line_styles() const { return m_line_styles; }

		const std::vector<path>& get_paths() const { return m_paths; }

		// morph uses this
		void	set_bound(const rect& r) { m_bound = r; /* should do some verifying */ }

	protected:
		friend class morph2_character_def;

#ifdef GNASH_USE_GC
		/// Mark reachable resources (for the GC)
		//
		/// Reachable resources are:
		///	- Associated fill styles (m_fill_styles).
		///	  These are not actual resources, but may contain some.
		///
		virtual void markReachableResources() const;
#endif // GNASH_USE_GC

		// derived morph classes changes these
		FillStyleVect m_fill_styles;
		LineStyleVect m_line_styles;
		PathVect m_paths;
		rect	m_bound;

		/// Free all meshes (deprecated)
		//void clear_meshes();

		/// Copy a shape character definition
		shape_character_def(const shape_character_def& o);

	private:

		/// Shape record flags
		enum ShapeRecordFlags {
			flagEnd = 0x00,
			flagMove = 0x01,
			flagFillStyle0Change = 0x02,
			flagFillStyle1Change = 0x04,
			flagLineStyleChange = 0x08,
			flagHasNewStyles = 0x10
		};

		void	sort_and_clean_meshes() const;
		
		// Don't assign to a shape character definition
		shape_character_def& operator= (const shape_character_def&)
		{
			abort();
			return *this;
		}

		// Cached pre-tesselated meshes. (deprecated)
		//mutable std::vector<mesh_set*>	m_cached_meshes;
	};

}	// end namespace gnash


#endif // GNASH_SHAPE_CHARACTER_DEF_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
