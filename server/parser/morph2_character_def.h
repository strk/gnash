// morph2.h -- Mike Shaver <shaver@off.net> 2003, , Vitalij Alexeev <tishka92@mail.ru> 2004.

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

#ifndef GNASH_MORPH2_H
#define GNASH_MORPH2_H

#include "shape.h"
#include "shape_character_def.h" // for inheritance of morph2_character_def


namespace gnash {

	/// DefineMorphShape tag
	//
	class morph2_character_def : public shape_character_def
	{
	public:
		morph2_character_def();
		virtual ~morph2_character_def();
		void	read(stream* in, int tag_type, bool with_style, movie_definition* m);
		virtual void	display(character* inst);
		void lerp_matrix(matrix& t, const matrix& m1, const matrix& m2, const float ratio);

		// Question: What is the bound of a morph? Is this conceptually correct?
		/// TODO: optimize this by take ratio into consideration, to decrease some
		/// invalidated area when rendering morphs
		virtual const rect&	get_bound() const 
		{ 
			m_bound.expand_to_rect(m_shape1->m_bound);
			m_bound.expand_to_rect(m_shape2->m_bound);
			return m_bound;
		}

	private:
		shape_character_def* m_shape1;
		shape_character_def* m_shape2;
		unsigned int offset;
		int fill_style_count;
		int line_style_count;
		float m_last_ratio;
		mesh_set*	m_mesh;
		mutable rect m_bound;
	};
}


#endif // GNASH_MORPH2_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
