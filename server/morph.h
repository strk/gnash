// morph.h -- Mike Shaver <shaver@off.net> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.
//
// Morph directives for shape tweening.

#ifndef GNASH_MORPH_H
#define GNASH_MORPH_H

#include "shape.h"
#include "styles.h"
#include "tesselate.h"

namespace gnash {
	class morph_path
	{
	public:
		morph_path();
		morph_path(float ax, float ay, int fill0, int fill1, int line);
		bool is_empty() const { return m_edges[0].size() == 0; }
		void tesselate(float ratio) const;

		int m_fill0, m_fill1, m_line;
		float m_ax[2], m_ay[2];
		std::vector<edge> m_edges[2];
		bool m_new_shape;
	};

        class shape_morph_def : public character_def
        {
	public:
                shape_morph_def();
                virtual ~shape_morph_def();
                virtual void display(character *instance_info);
                void read(stream* in, int tag_type, bool with_style,
			  movie_definition* m);
		virtual void tesselate(float error_tolerance, tesselate::trapezoid_accepter *accepter, float ratio) const;

        private:
		void read_edge(stream* in, edge& e, float& x, float& y);
		int read_shape_record(stream* in, movie_definition* m,
				      bool start);

		rect	m_bound_orig, m_bound_target;
		std::vector<morph_fill_style> m_fill_styles;
		std::vector<morph_line_style> m_line_styles;
		std::vector<morph_path> m_paths;

		float m_last_ratio;
		mesh_set *m_last_mesh;
        };

	class morph_tesselating_shape : public tesselate::tesselating_shape
	{
	public:
		morph_tesselating_shape(shape_morph_def *sh, float ratio) :
			m_sh(sh), m_ratio(ratio) { }
		virtual void tesselate(float error_tolerance, tesselate::trapezoid_accepter *accepter) const
		{
			m_sh->tesselate(error_tolerance, accepter, m_ratio);
		}

		
	private:
		shape_morph_def *m_sh;
		float            m_ratio;
	};
		

}

#endif /* GNASH_MORPH_H */
// Local Variables:
// mode: C++
// c-basic-offset: 8
// tab-width: 8
// indent-tabs-mode: t
// End:
