// morph2_character_def.cpp:   Load and render morphing shapes, for Gnash.
//
//   Copyright (C) 2006, 2007 Free Software Foundation, Inc.
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

/* $Id: morph2_character_def.cpp,v 1.11 2007/07/01 10:54:34 bjacques Exp $ */

// Based on the public domain morph2.cpp of:
// Thatcher Ulrich <tu@tulrich.com>, Mike Shaver <shaver@off.net> 2003,
// Vitalij Alexeev <tishka92@mail.ru> 2004.

#include "morph2_character_def.h"
#include "stream.h"
#include "render.h"
#include "movie_definition.h"
#include "bitmap_character_def.h"
#include "sprite_instance.h"


namespace gnash {

// Facilities for working with list of paths.
class PathList {

public:

	PathList(const std::vector<path>& paths)
		:
		_paths(paths),
		_currpath(0),
		_curredge(0),
		_nedges(computeNumberOfEdges(_paths))
	{}

	/// Return number of edges in the path list
	size_t size()
	{
		return _nedges;
	}

	/// Get next edge in the path list.
	//
	/// After last edge in the list has been fetched,
	/// next call to this function will return first
	/// edge again.
	///
	const edge& getNextEdge()
	{
		const edge& ret = _paths[_currpath][_curredge];
		if ( ++_curredge >= _paths[_currpath].size() )
		{
			if ( ++_currpath >= _paths.size() )
			{
				// this is not really needed,
				// but it's simpler to do so that
				// to make next call fail or abort..
				_currpath = 0;
				_curredge = 0;
			}
		}
		return ret;
	}

	/// Compute total number of edges
	static size_t computeNumberOfEdges(const std::vector<path>& paths)
	{
		size_t count=0;
		for (size_t i = 0, e=paths.size(); i<e; ++i)
		{
			count += paths[i].size();
		}
		return count;
	}

private:

	const std::vector<path>& _paths;

	size_t _currpath;

	size_t _curredge;

	size_t _nedges;

};

	morph2_character_def::morph2_character_def():
		m_last_ratio(-1.0f), m_mesh(0)
	{
		m_shape1 = new shape_character_def();
		m_shape2 = new shape_character_def();
	}


	morph2_character_def::~morph2_character_def()
	{
	}

	void	morph2_character_def::display(character* inst)
	{
//		GNASH_REPORT_FUNCTION;

		unsigned int i;
		float ratio = inst->get_ratio() / 65535.0; 

		// bounds
		rect	new_bound;
		new_bound.set_lerp(m_shape1->get_bound(), m_shape2->get_bound(), ratio);
		set_bound(new_bound);

		// fill styles
		for (i=0; i < m_fill_styles.size(); i++)
		{
			fill_style& fs = m_fill_styles[i];

			const fill_style& fs1 = m_shape1->get_fill_styles()[i];
			const fill_style& fs2 = m_shape2->get_fill_styles()[i];

			fs.set_lerp(fs1, fs2, ratio);
		}

		// line styles
		for (i=0; i < m_line_styles.size(); i++)
		{
			line_style& ls = m_line_styles[i];
			const line_style& ls1 = m_shape1->get_line_styles()[i];
			const line_style& ls2 = m_shape2->get_line_styles()[i];
			ls.m_width = (uint16_t)frnd(flerp(ls1.get_width(), ls2.get_width(), ratio));
			ls.m_color.set_lerp(ls1.get_color(), ls2.get_color(), ratio);
		}

		// This is used for cases in which number
		// of paths in start shape and end shape are not
		// the same.
		path empty_path;
		edge empty_edge;

		// shape
		unsigned int k=0, n=0;
		const std::vector<path>& paths1 = m_shape1->get_paths();
		const std::vector<path>& paths2 = m_shape2->get_paths();
		for (i=0; i < m_paths.size(); i++)
		{
			path& p = m_paths[i];
			const path& p1 = i < paths1.size() ? paths1[i] : empty_path;
			const path& p2 = n < paths2.size() ? paths2[n] : empty_path;

			float new_ax = flerp(p1.m_ax, p2.m_ax, ratio);
			float new_ay = flerp(p1.m_ay, p2.m_ay, ratio);

			p.reset ( new_ax, new_ay, p1.getLeftFill(), p2.getRightFill(), p1.getLineStyle() );

 			// @@ hack.
			if (p.getLeftFill() == 0 && p.getRightFill() == 0)
			{
				if (m_shape1->get_fill_styles().size() > 0) p.setLeftFill(1);
			}


			//  edges;
			size_t len = p1.size();
			p.m_edges.resize(len);

			for (size_t j=0; j < p.size(); j++)
			{
				edge& e = p[j];
				const edge& e1 = j < p1.size() ? p1[j] : empty_edge;
				const edge& e2 = k < p2.size() ? p2[k] : empty_edge;

				e.m_cx = flerp(e1.m_cx, e2.m_cx, ratio);
				e.m_cy = flerp(e1.m_cy, e2.m_cy, ratio);
				e.m_ax = flerp(e1.m_ax, e2.m_ax, ratio);
				e.m_ay = flerp(e1.m_ay, e2.m_ay, ratio);
				k++;
				if (p2.size() <= k)
				{
					k=0; n++;
				}
			}
		}

//  display

    {
    gnash::render::draw_shape_character(this, inst);
    }

/*
		matrix mat = inst->get_world_matrix();
		cxform cx = inst->get_world_cxform();
		float max_error = 20.0f / mat.get_max_scale() /	inst->get_parent()->get_pixel_scale();
		if (ratio != m_last_ratio)
		{
			delete m_mesh;
			m_last_ratio = ratio;
			m_mesh = new mesh_set(this, max_error * 0.75f);
		}
  	m_mesh->display(mat, cx, m_fill_styles, m_line_styles);
*/
	}


	void	morph2_character_def::read(stream* in, int tag_type, bool with_style, movie_definition* md)
	{
		assert(tag_type == SWF::DEFINEMORPHSHAPE);

		UNUSED(tag_type);
		UNUSED(with_style);

		rect	bound1, bound2;
		bound1.read(in);
		bound2.read(in);
		m_shape1->set_bound(bound1);
		m_shape2->set_bound(bound2);

		offset = in->read_u32();

		fill_style_count = in->read_variable_count();
		int i;
		for (i = 0; i < fill_style_count; i++) {
			fill_style fs1, fs2;

			fs1.m_type = in->read_u8();
			fs2.m_type = fs1.m_type;

			IF_VERBOSE_PARSE(
			  log_parse(_("morph fill style type = 0x%X"),
			    fs1.m_type);
			);

			if (fs1.m_type == 0x00)
			{
				fs1.m_color.read_rgba(in);
				fs2.m_color.read_rgba(in);

				IF_VERBOSE_PARSE(
				  log_parse(_("morph fill style begin color: "));
				  fs1.m_color.print();
				  log_parse(_("morph fill style end color: "));
				  fs2.m_color.print();
				);
			}
			else if (fs1.m_type == 0x10 || fs1.m_type == 0x12)
			{
				matrix	input_matrix1, input_matrix2;

				input_matrix1.read(in);
				input_matrix2.read(in);

				fs1.m_gradient_matrix.set_identity();
				fs2.m_gradient_matrix.set_identity();
				if (fs1.m_type == 0x10)
				{
					fs1.m_gradient_matrix.concatenate_translation(128.f, 0.f);
					fs1.m_gradient_matrix.concatenate_scale(1.0f / 128.0f);
					fs2.m_gradient_matrix.concatenate_translation(128.f, 0.f);
					fs2.m_gradient_matrix.concatenate_scale(1.0f / 128.0f);
				}
				else
				{
					fs1.m_gradient_matrix.concatenate_translation(32.f, 32.f);
					fs1.m_gradient_matrix.concatenate_scale(1.0f / 512.0f);
					fs2.m_gradient_matrix.concatenate_translation(32.f, 32.f);
					fs2.m_gradient_matrix.concatenate_scale(1.0f / 512.0f);
				}

				matrix	m1, m2;
				m1.set_inverse(input_matrix1);
				fs1.m_gradient_matrix.concatenate(m1);
				m2.set_inverse(input_matrix2);
				fs2.m_gradient_matrix.concatenate(m2);

				// GRADIENT
				int	num_gradients = in->read_u8();
				assert(num_gradients >= 1 && num_gradients <= 8);

				fs1.m_gradients.resize(num_gradients);
				fs2.m_gradients.resize(num_gradients);

				for (int j = 0; j < num_gradients; j++)
				{
					fs1.m_gradients[j].read(in, tag_type);
					fs2.m_gradients[j].read(in, tag_type);
				}

				IF_VERBOSE_PARSE(
				  log_parse(_("morph fsr: num_gradients = %d"),
				    num_gradients);
				);

				// @@ hack.
				if (num_gradients > 0)
				{
					fs1.m_color = fs1.m_gradients[0].m_color;
					fs2.m_color = fs2.m_gradients[0].m_color;
				}
			}
			else if (fs1.m_type == 0x40 || fs1.m_type == 0x41)
			{

				int	bitmap_char_id = in->read_u16();
				IF_VERBOSE_PARSE(
				  log_parse(_("morph fsr bitmap_char = %d"),
				    bitmap_char_id);
				);

				// Look up the bitmap character.
				fs1.m_bitmap_character = md->get_bitmap_character_def(bitmap_char_id);
				fs2.m_bitmap_character = fs1.m_bitmap_character;

				matrix	m1, m2;
				m1.read(in);
				m2.read(in);

				// For some reason, it looks like they store the inverse of the
				// TWIPS-to-texcoords matrix.
				fs1.m_bitmap_matrix.set_inverse(m1);
				fs2.m_bitmap_matrix.set_inverse(m2);
			}
			m_shape1->m_fill_styles.push_back(fs1);
			m_shape2->m_fill_styles.push_back(fs2);
		}

		line_style_count = in->read_variable_count();
		for (i = 0; i < line_style_count; i++) {
			line_style ls1, ls2;
			ls1.m_width = in->read_u16();
			ls2.m_width = in->read_u16();
			ls1.m_color.read(in, tag_type);
			ls2.m_color.read(in, tag_type);
			m_shape1->m_line_styles.push_back(ls1);
			m_shape2->m_line_styles.push_back(ls2);
		}

		m_shape1->read(in, tag_type, false, md);
		in->align();
		m_shape2->read(in, tag_type, false, md);

		assert(m_shape1->m_fill_styles.size() == m_shape2->m_fill_styles.size());
		assert(m_shape1->m_line_styles.size() == m_shape2->m_line_styles.size());

		// setup array size
		m_fill_styles.resize(m_shape1->m_fill_styles.size());
		unsigned int k;
		for (k = 0; k < m_fill_styles.size(); k++)
		{
			fill_style& fs = m_fill_styles[k];
			fill_style& fs1 = m_shape1->m_fill_styles[k];
			fs.m_gradients.resize(fs1.m_gradients.size());
		}
		m_line_styles.resize(m_shape1->m_line_styles.size());
		m_paths.resize(m_shape1->m_paths.size());

		unsigned edges_count1 = PathList::computeNumberOfEdges(m_shape1->m_paths);
		unsigned edges_count2 = PathList::computeNumberOfEdges(m_shape2->m_paths);

		IF_VERBOSE_PARSE(
		  log_parse("morph: "
			  "startShape(paths:" SIZET_FMT ", edges:%u), "
			  "endShape(paths:" SIZET_FMT ", edges:%u)",
			  m_shape1->m_paths.size(), edges_count1,
			  m_shape2->m_paths.size(), edges_count2);
		);

		IF_VERBOSE_MALFORMED_SWF(


		if ( m_shape1->m_paths.size() != m_shape2->m_paths.size() )
		{
			log_swferror(_("Different number of paths "
				"in start (" SIZET_FMT ") and end (" SIZET_FMT
				") shapes of a morph"),
				m_shape1->m_paths.size(),
				m_shape2->m_paths.size());
		}
		else if ( edges_count1 != edges_count2 )
		{
			log_swferror(_("Different number of edges "
				"in start (%u) and end (%u) shapes "
				"of a morph"),
				edges_count1, edges_count1);
		}

		);

	}
}

// Local Variables:
// mode: C++
// c-basic-offset: 8
// tab-width: 8
// indent-tabs-mode: t
// End:
