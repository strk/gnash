// morph.cpp
// -- Thatcher Ulrich <tu@tulrich.com>, Mike Shaver <shaver@off.net> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Loading and rendering of morphing shapes.

#include "morph.h"

#include "impl.h"
#include "log.h"
#include "stream.h"

#include "tu_file.h"
#include "render.h"

#include <float.h>

#define SHAPE_LOG 0

namespace gnash {

	shape_morph_def::shape_morph_def() :
		m_last_ratio(-1.0f), m_last_mesh(0)
	{
	}

	shape_morph_def::~shape_morph_def()
	{ 
		delete m_last_mesh;
	}

	void shape_morph_def::display(character *inst)
	{
		float ratio = inst->m_ratio;
		IF_VERBOSE_ACTION(log_msg("smd: displaying %d at ratio %g\n",
					  inst->m_id, ratio));
		matrix mat = inst->get_world_matrix();
		cxform cx = inst->get_world_cxform();

		float max_error = 20.0f / mat.get_max_scale() /
			inst->get_parent()->get_pixel_scale();
		if (ratio != m_last_ratio) {
			delete m_last_mesh;
			m_last_ratio = ratio;
			morph_tesselating_shape mts(this, ratio);
			m_last_mesh = new mesh_set(&mts, max_error * 0.75f);
		}
		m_last_mesh->display(mat, cx, m_fill_styles, m_line_styles, ratio);
	}

	/* virtual */ void shape_morph_def::tesselate(float error_tolerance,
						      tesselate::trapezoid_accepter *accepter,
						      float ratio)
		const
	{
		IF_VERBOSE_ACTION(log_msg("smd: tesselating at ratio %g\n",
					  ratio));
		
		// XXX sharing
		tesselate::begin_shape(accepter, error_tolerance);
		for (int i = 0; i < m_paths.size(); i++) {
			if (m_paths[i].m_new_shape) {
				tesselate::end_shape();
				tesselate::begin_shape(accepter, error_tolerance);
			} else {
				m_paths[i].tesselate(ratio);
			}
		}
		tesselate::end_shape();
	}

	void shape_morph_def::read(stream *in, int tag_type, bool with_style,
				   movie_definition_sub *m)
	{
		assert(tag_type == 46);
		int pos = in->get_underlying_stream()->get_position();
		IF_VERBOSE_PARSE(log_msg("smd: initial pos %d\n", pos));
		m_bound_orig.read(in);
		m_bound_target.read(in);

		IF_VERBOSE_PARSE(log_msg("smd: orig bounds ");
				 m_bound_orig.print();
				 log_msg("smd: target bounds ");
				 m_bound_target.print());

		int offset = in->read_u32();
		UNUSED(offset);
		pos = in->get_underlying_stream()->get_position();

		int fill_style_count = in->read_variable_count();
		
		IF_VERBOSE_PARSE(log_msg("smd: fsc = %d\n", fill_style_count));
		for (int i = 0; i < fill_style_count; i++) {
			m_fill_styles.push_back(morph_fill_style(in, m));
		}

		int line_style_count = in->read_variable_count();
		
		IF_VERBOSE_PARSE(log_msg("smd: lsc = %d\n", line_style_count));
		{for (int i = 0; i < line_style_count; i++) {
			m_line_styles.push_back(morph_line_style(in));
		}}

		int edges1 = read_shape_record(in, m, true);
		IF_VERBOSE_PARSE(log_msg("morph: read %d edges for shape 1\n",
					 edges1));

		int pos2 = in->get_underlying_stream()->get_position();
		assert(pos + offset == pos2);

		int edges2 = read_shape_record(in, m, false);
		IF_VERBOSE_PARSE(log_msg("morph: read %d edges for shape 2\n",
					 edges2));

		assert(edges1 == edges2);
		pos2 = in->get_underlying_stream()->get_position();
		IF_VERBOSE_PARSE(log_msg("smd: final pos %d\n", pos2));
				
	}

	void shape_morph_def::read_edge(stream* in, edge& e, float& x, float& y)
	{
		int edge_type = in->read_uint(1);
		int bits = 2 + in->read_uint(4);
		if (edge_type == 0) { // curved
			e.m_cx = x + in->read_sint(bits);
			e.m_cy = y + in->read_sint(bits);
			x = e.m_ax = e.m_cx + in->read_sint(bits);
			y = e.m_ay = e.m_cy + in->read_sint(bits);
			if (SHAPE_LOG) IF_VERBOSE_PARSE(log_msg("smd::re: curved edge   = %4g %4g - %4g %4g - %4g %4g\n", x, y, e.m_cx, e.m_cy, e.m_ax, e.m_ay));
			return;
		}

		// straight edge
		int line_flag = in->read_uint(1);
		float dx = 0, dy = 0;
		if (line_flag) { // general line
			dx = float(in->read_sint(bits));
			dy = float(in->read_sint(bits));
		} else {
			int vert_flag = in->read_uint(1);
			if (vert_flag) { // vertical line
				dy = float(in->read_sint(bits));
			} else {
				dx = float(in->read_sint(bits));
			}
		}

		e.m_cx = x + dx / 2;
		e.m_cy = y + dy / 2;
		x = e.m_ax = x + dx;
		y = e.m_ay = y + dy;
		if (SHAPE_LOG) IF_VERBOSE_PARSE(log_msg("smd::re: straight edge = %4g %4g - %4g %4g\n", x - dx, y - dy, x, y));
	}

	int shape_morph_def::read_shape_record(stream* in,
					       movie_definition_sub* m,
					       bool start)
	{
		morph_path current_path;
		edge e;
		float x = 0, y = 0;
		int fill_base = 0, line_base = 0;
		int pathidx = 0, edgeidx = 0;
		int edge_count = 0;

		in->align();
		int fill_bits = in->read_uint(4);
		int line_bits = in->read_uint(4);
		IF_VERBOSE_PARSE(log_msg("smd: bits %d/%d\n",
					 fill_bits, line_bits));

		if (!start) {
			for (int i = 0; i < m_paths.size(); i++) {
				int len = m_paths[i].m_edges[0].size();
				m_paths[i].m_edges[1].resize(len);
			}
		}

		for (;;) {
			int type_flag = in->read_uint(1);
			if (type_flag == 1) {
				// EDGERECORD
				read_edge(in, e, x, y);
				edge_count++;
				if (start) {
					current_path.m_edges[0].push_back(e);
					continue;
				}
				array<edge> &edges = m_paths[pathidx].m_edges[1];
				edges[edgeidx] = e;
				edgeidx++;
				while (edgeidx == edges.size()) {
					pathidx++;
					if (pathidx < m_paths.size()) {
						m_paths[pathidx].m_ax[1] = x;
						m_paths[pathidx].m_ay[1] = y;
					}
					edgeidx = 0;
				}
									   
				continue;
			}

			if (start) {
				if (!current_path.is_empty()) {
					m_paths.push_back(current_path);
					current_path.m_edges[0].resize(0);
				}
			} else {
				if (edgeidx) {
					pathidx++;
					if (pathidx < m_paths.size()) {
						m_paths[pathidx].m_ax[1] = x;
						m_paths[pathidx].m_ay[1] = y;
					}
					edgeidx = 0;
				}
			}

			int flags = in->read_uint(5);
			if (flags == 0) return edge_count;

			if (flags & 0x01) { // MOVETO
				int num_move_bits = in->read_uint(5);
				x = float(in->read_sint(num_move_bits));
				y = float(in->read_sint(num_move_bits));
				if (SHAPE_LOG) IF_VERBOSE_PARSE(log_msg("morph: MOVETO %g,%g (%d)\n", x, y, num_move_bits));
			}

			// If we're starting a new path, it starts at the
			// end (x,y) of the last edge we pushed (possibly
			// overridden by a MOVETO record).
			if (start) {
				current_path.m_ax[0] = x;
				current_path.m_ay[0] = y;
			} else {
				m_paths[pathidx].m_ax[1] = x;
				m_paths[pathidx].m_ay[1] = y;
			}

			if ((flags & 0x02) && fill_bits) { // FILL0
				assert(start);
				int style = in->read_uint(fill_bits);
				if (style > 0) style += fill_base;
				current_path.m_fill0 = style;
				if (SHAPE_LOG) IF_VERBOSE_PARSE(log_msg("morph: fill0 = %d\n", current_path.m_fill0));
			}

			if ((flags & 0x04) && fill_bits) { // FILL1
				assert(start);
				int style = in->read_uint(fill_bits);
				if (style > 0) style += fill_base;
				current_path.m_fill1 = style;
				if (SHAPE_LOG) IF_VERBOSE_PARSE(log_msg("morph: fill1 = %d\n", current_path.m_fill1));
			}
			
			if ((flags & 0x08) && line_bits) { // LINE
				assert(start);
				int style = in->read_uint(line_bits);
				if (style > 0) style += line_base;
				current_path.m_line = style;
				if (SHAPE_LOG) IF_VERBOSE_PARSE(log_msg("scr: line = %d\n", current_path.m_line));
			}

			if (flags & 0x10) { // NEWSTYLES
				assert(start);
				// Clear styles.
				current_path.m_fill0 = current_path.m_fill1 =
					current_path.m_line = -1;

				// Empty path signalling new shape.
				// XXX should we start a whole new shape here?
				m_paths.push_back(morph_path());
				m_paths.back().m_new_shape = true;

				fill_base = m_fill_styles.size();
				line_base = m_line_styles.size();

				int count = in->read_variable_count();
				for (int i = 0; i < count; i++) {
					m_fill_styles.push_back(morph_fill_style(in, m));
				}

				count = in->read_variable_count();
				{for (int i = 0; i < count; i++) {
					m_line_styles.push_back(morph_line_style(in));
				}}

				fill_bits = in->read_uint(4);
				fill_bits = in->read_uint(4);

				if (SHAPE_LOG) IF_VERBOSE_PARSE(log_msg("morph: read %d/%d new styles (bits now %d/%d)\n", m_fill_styles.size() - fill_base, m_line_styles.size() - line_base, fill_bits, line_bits));
							 
			}
		}
	}

	morph_fill_style::morph_fill_style()
		:
		m_type(0),
		m_bitmap_character(0)
	{
	}

	morph_fill_style::morph_fill_style(stream* in, movie_definition_sub *m)
		:
		m_bitmap_character(0)
	{
		read(in, m);
	}

	morph_fill_style::~morph_fill_style()
	{
	}

	void morph_fill_style::read(stream* in, movie_definition_sub* m)
	{
		m_type = in->read_u8();
		switch(m_type) {
		case 0x00: {
			m_color[0].read_rgba(in);
			m_color[1].read_rgba(in);
			IF_VERBOSE_PARSE(log_msg("fsr: color1 ");
					 m_color[0].print();
					 log_msg("fsr: color2 ");
					 m_color[1].print());
			return;
		}
		case 0x10: { // linear gradient fill
			m_gradient_matrix[0].set_identity();
			m_gradient_matrix[1].set_identity();
			m_gradient_matrix[0].concatenate_translation(128.f,0.f);
			m_gradient_matrix[1].concatenate_translation(128.f,0.f);
			m_gradient_matrix[0].concatenate_scale(1.0f / 128.0f);
			m_gradient_matrix[1].concatenate_scale(1.0f / 128.0f);
			goto read_gradients;
		case 0x12: // radial gradient fill
			m_gradient_matrix[0].set_identity();
			m_gradient_matrix[1].set_identity();
			m_gradient_matrix[0].concatenate_translation(32.f,32.f);
			m_gradient_matrix[1].concatenate_translation(32.f,32.f);
			m_gradient_matrix[0].concatenate_scale(1.0f / 512.0f);
			m_gradient_matrix[1].concatenate_scale(1.0f / 512.0f);
			
		read_gradients:
			matrix input_matrix, m;

			input_matrix.read(in);
			m.set_inverse(input_matrix);
			m_gradient_matrix[0].concatenate(m);

			m.set_inverse(input_matrix);
			input_matrix.read(in);
			m_gradient_matrix[1].concatenate(m);
			
			// GRADIENT
			int num_gradients = in->read_u8();
			IF_VERBOSE_PARSE(log_msg("fsr: num_gradients: %d\n",
						 num_gradients));
			assert(num_gradients > 0 && num_gradients < 9);
			m_gradients[0].resize(num_gradients);
			m_gradients[1].resize(num_gradients);
			while(num_gradients--) {
				m_gradients[0][num_gradients].read(in, 46);
				m_gradients[1][num_gradients].read(in, 46);
			}

			// @@ hack.
			m_color[0] = m_gradients[0][0].m_color;
			m_color[1] = m_gradients[1][0].m_color;
			return;
		}
		case 0x40: { // tiled bitmap fill
		case 0x41: // clipped bitmap fill
			int char_id = in->read_u16();
			IF_VERBOSE_PARSE(log_msg("fsr: bitmap_char = %d\n",
						 char_id));
			m_bitmap_character = m->get_bitmap_character(char_id);

			matrix m;
			
			// For some reason, it looks like they store the inverse
			// of the TWIPS-to-texcoords matrix.
			m.read(in);
			m_bitmap_matrix[0].set_inverse(m);
			m.read(in);
			m_bitmap_matrix[1].set_inverse(m);
			return;
		}
		default:
			assert(0);
			break;
		}
	}

	void morph_fill_style::apply(int fill_side, float ratio) const
	{
		assert(m_type == 0x00);
		rgba color;
		color.set_lerp(m_color[0], m_color[1], ratio);
		gnash::render::fill_style_color(fill_side, color);
	}

	morph_line_style::morph_line_style()
	{
		m_width[0] = m_width[1] = 0;
	}

	morph_line_style::morph_line_style(stream* in)
	{
		read(in);
	}

	void morph_line_style::read(stream* in)
	{
		m_width[0] = in->read_u16();
		m_width[1] = in->read_u16();
		
		m_color[0].read_rgba(in);
		m_color[1].read_rgba(in);

		IF_VERBOSE_PARSE(log_msg("mls 1: width %d color ", m_width[0]);
				 m_color[0].print();
				 log_msg("mls 2: width %d color ", m_width[1]);
				 m_color[1].print());
	}

	void morph_line_style::apply(float ratio) const
	{
		rgba color;
		color.set_lerp(m_color[0], m_color[1], ratio);
		gnash::render::line_style_color(color);

		gnash::render::line_style_width(flerp(m_width[0], m_width[1], ratio));
	}

	morph_path::morph_path() :
		m_fill0(0), m_fill1(0), m_line(0), m_new_shape(false)
	{ }

	void morph_path::tesselate(float ratio) const
	{
		tesselate::begin_path(m_fill0 - 1, m_fill1 - 1, m_line - 1,
				      flerp(m_ax[0], m_ax[1], ratio),
				      flerp(m_ay[0], m_ay[1], ratio));
		assert(m_edges[0].size() == m_edges[1].size());
		for (int i = 0; i < m_edges[0].size(); i++) {
			const edge &e0 = m_edges[0][i], &e1 = m_edges[1][i];
			tesselate::add_curve_segment(flerp(e0.m_cx, e1.m_cx, ratio),
						     flerp(e0.m_cy, e1.m_cy, ratio),
						     flerp(e0.m_ax, e1.m_ax, ratio),
						     flerp(e0.m_ay, e1.m_ay, ratio));
		}
		tesselate::end_path();
	}
	
}


// Local Variables:
// mode: C++
// c-basic-offset: 8
// tab-width: 8
// indent-tabs-mode: t
// End:
