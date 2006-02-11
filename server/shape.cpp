// shape.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Quadratic bezier outline shapes, the basis for most SWF rendering.


#include "shape.h"

#include "impl.h"
#include "log.h"
#include "render.h"
#include "stream.h"
#include "tesselate.h"

#include "tu_file.h"

#include <float.h>


#define DEBUG_DISPLAY_SHAPE_PATHS
#ifdef DEBUG_DISPLAY_SHAPE_PATHS
	// For debugging only!
	bool	gnash_debug_show_paths = false;
#endif // DEBUG_DISPLAY_SHAPE_PATHS


namespace gnash {

	namespace tesselate {
		tesselating_shape::~tesselating_shape()
		{
		}
	}

 	static float	s_curve_max_pixel_error = 1.0f;

	void	set_curve_max_pixel_error(float pixel_error)
	{
		s_curve_max_pixel_error = fclamp(pixel_error, 1e-6f, 1e6f);
	}

	float	get_curve_max_pixel_error()
	{
		return s_curve_max_pixel_error;
	}


	//
	// edge
	//

	edge::edge()
		:
		m_cx(0), m_cy(0),
		m_ax(0), m_ay(0)
	{}

	edge::edge(float cx, float cy, float ax, float ay)
		:
		m_cx(cx), m_cy(cy),
		m_ax(ax), m_ay(ay)
	{
	}

	void	edge::tesselate_curve() const
	// Send this segment to the tesselator.
	{
		tesselate::add_curve_segment(m_cx, m_cy, m_ax, m_ay);
	}


	bool	edge::is_straight() const
	{
		return m_cx == m_ax && m_cy == m_ay;
	}


	//
	// path
	//


	path::path()
		:
		m_new_shape(false)
	{
		reset(0, 0, 0, 0, 0);
	}

	path::path(float ax, float ay, int fill0, int fill1, int line)
	{
		reset(ax, ay, fill0, fill1, line);
	}


	void	path::reset(float ax, float ay, int fill0, int fill1, int line)
	// Reset all our members to the given values, and clear our edge list.
	{
		m_ax = ax;
		m_ay = ay;
		m_fill0 = fill0;
		m_fill1 = fill1;
		m_line = line;

		m_edges.resize(0);

		assert(is_empty());
	}


	bool	path::is_empty() const
	// Return true if we have no edges.
	{
		return m_edges.size() == 0;
	}


	bool	path::point_test(float x, float y)
	// Point-in-shape test.  Return true if the query point is on the filled
	// interior of this shape.
	{
		if (m_edges.size() <= 0)
		{
			return false;
		}

		if (m_fill0 < 0)
		{
			// No interior fill.
			
			// @@ This isn't quite right due to some paths
			// doing double-duty with both fill0 and fill1
			// styles.

			// TODO: get rid of this stupid fill0/fill1
			// business -- a path should always be
			// counterclockwise and have one fill.  For
			// input paths with fill1, generate a separate
			// reversed path with fill set to fill1.
			// Group all paths with the same fill into a
			// path group; do the point_test on the whole
			// group.
			return false;
		}

		// Shoot a horizontal ray from (x,y) to the right, and
		// count the number of edge crossings.  An even number
		// of crossings means the point is outside; an odd
		// number means it's inside.

		float x0 = m_ax;
		float y0 = m_ay;

		int ray_crossings = 0;
		for (int i = 0, n = m_edges.size(); i < n; i++)
		{
			const edge& e = m_edges[i];

			float x1 = e.m_ax;
			float y1 = e.m_ay;

			if (e.is_straight()) {
				// Straight-line case.
				
				// See if (x0,y0)-(x1,y1) crosses (x,y)-(infinity,y)
			
				// Does the segment straddle the horizontal ray?
				bool cross_up = (y0 < y && y1 >= y);
				bool cross_down = (!cross_up) && (y0 > y && y1 <= y);
				if (cross_up || cross_down)
				{
					// Straddles.
				
					// Is the crossing point to the right of x?
					float dy = y1 - y0;

					// x_intercept = x0 + (x1 - x0) * (y - y0) / dy;
					float x_intercept_times_dy = x0 * dy + (x1 - x0) * (y - y0);
					float x_times_dy = x * dy;

					// text x_intercept > x
				
					// factor out the division; two cases depending on sign of dy
					if (cross_up)
					{
						assert(dy > 0);
						if (x_intercept_times_dy > x * dy)
						{
							ray_crossings++;
						}
					}
					else
					{
						// dy is negative; reverse the inequality test
						assert(dy < 0);
						if (x_intercept_times_dy < x * dy)
						{
							ray_crossings++;
						}
					}
				}
			}
			else
			{
				// Curve case.
				float cx = e.m_cx;
				float cy = e.m_cy;

				// Find whether & where the curve crosses y
				if ((y0 < y && y1 < y && cy < y)
				    || (y0 > y && y1 > y && cy > y))
				{
					// All above or all below -- no possibility of crossing.
				}
				else if (x0 < x && x1 < x && cx < x)
				{
					// All to the left -- no possibility of crossing to the right.
				}
				else
				{
					// Find points where the curve crosses y.

					// Quadratic bezier is:
					//
					// p = (1-t)^2 * a0 + 2t(1-t) * c + t^2 * a1
					//
					// We need to solve for x at y.
					
					// Use the quadratic formula.

					// Numerical Recipes suggests this variation:
					// q = -0.5 [b +sgn(b) sqrt(b^2 - 4ac)]
					// x1 = q/a;  x2 = c/q;

					float A = y1 + y0 - 2 * cy;
					float B = 2 * (cy - y0);
					float C = y0 - y;

					float rad = B * B - 4 * A * C;
					if (rad < 0)
					{
						// No real solutions.
					}
					else
					{
						float q;
						float sqrt_rad = sqrtf(rad);
						if (B < 0) {
							q = -0.5f * (B - sqrt_rad);
						} else {
							q = -0.5f * (B + sqrt_rad);
						}

						// The old-school way.
						// float t0 = (-B + sqrt_rad) / (2 * A);
						// float t1 = (-B - sqrt_rad) / (2 * A);

						if (A != 0)
						{
							float t0 = q / A;
							if (t0 >= 0 && t0 < 1) {
								float x_at_t0 =
									x0 + 2 * (cx - x0) * t0 + (x1 + x0 - 2 * cx) * t0 * t0;
								if (x_at_t0 > x) {
									ray_crossings++;
								}
							}
						}

						if (q != 0)
						{
							float t1 = C / q;
							if (t1 >= 0 && t1 < 1) {
								float x_at_t1 =
									x0 + 2 * (cx - x0) * t1 + (x1 + x0 - 2 * cx) * t1 * t1;
								if (x_at_t1 > x) {
									ray_crossings++;
								}
							}
						}
					}
				}
			}

			x0 = x1;
			y0 = y1;
		}

		if (ray_crossings & 1)
		{
			// Odd number of ray crossings means the point
			// is inside the poly.
			return true;
		}
		return false;
	}


	void	path::tesselate() const
	// Push this path into the tesselator.
	{
		tesselate::begin_path(
			m_fill0 - 1,
			m_fill1 - 1,
			m_line - 1,
			m_ax, m_ay);
		for (int i = 0; i < m_edges.size(); i++)
		{
			m_edges[i].tesselate_curve();
		}
		tesselate::end_path();
	}


	// Utility.


	void	write_coord_array(tu_file* out, const array<Sint16>& pt_array)
	// Dump the given coordinate array into the given stream.
	{
		int	n = pt_array.size();

		out->write_le32(n);
		for (int i = 0; i < n; i++)
		{
			out->write_le16((Uint16) pt_array[i]);
		}
	}


	void	read_coord_array(tu_file* in, array<Sint16>* pt_array)
	// Read the coordinate array data from the stream into *pt_array.
	{
		int	n = in->read_le32();

		pt_array->resize(n);
		for (int i = 0; i < n; i ++)
		{
			(*pt_array)[i] = (Sint16) in->read_le16();
		}
	}


	//
	// mesh
	//

	
	mesh::mesh()
	{
	}


	void	mesh::set_tri_strip(const point pts[], int count)
	{
		m_triangle_strip.resize(count * 2);	// 2 coords per point
		
		// convert to ints.
		for (int i = 0; i < count; i++)
		{
			m_triangle_strip[i * 2] = Sint16(pts[i].m_x);
			m_triangle_strip[i * 2 + 1] = Sint16(pts[i].m_y);
		}

//		m_triangle_strip.resize(count);
//		memcpy(&m_triangle_strip[0], &pts[0], count * sizeof(point));
	}


	void	mesh::display(const base_fill_style& style, float ratio) const
	{
		// pass mesh to renderer.
		if (m_triangle_strip.size() > 0)
		{
			style.apply(0, ratio);
			render::draw_mesh_strip(&m_triangle_strip[0], m_triangle_strip.size() >> 1);
		}
	}


	void	mesh::output_cached_data(tu_file* out)
	// Dump our data to *out.
	{
		write_coord_array(out, m_triangle_strip);
	}

	
	void	mesh::input_cached_data(tu_file* in)
	// Slurp our data from *out.
	{
		read_coord_array(in, &m_triangle_strip);
	}


	//
	// line_strip
	//


	line_strip::line_strip()
	// Default constructor, for array<>.
		:
		m_style(-1)
	{}


	line_strip::line_strip(int style, const point coords[], int coord_count)
	// Construct the line strip (polyline) made up of the given sequence of points.
		:
		m_style(style)
	{
		assert(style >= 0);
		assert(coords != NULL);
		assert(coord_count > 1);

//		m_coords.resize(coord_count);
//		memcpy(&m_coords[0], coords, coord_count * sizeof(coords[0]));
		m_coords.resize(coord_count * 2);	// 2 coords per vert
		
		// convert to ints.
		for (int i = 0; i < coord_count; i++)
		{
			m_coords[i * 2] = Sint16(coords[i].m_x);
			m_coords[i * 2 + 1] = Sint16(coords[i].m_y);
		}
	}


	void	line_strip::display(const base_line_style& style, float ratio) const
	// Render this line strip in the given style.
	{
		assert(m_coords.size() > 1);
		assert((m_coords.size() & 1) == 0);

		style.apply(ratio);
		render::draw_line_strip(&m_coords[0], m_coords.size() >> 1);
	}


	void	line_strip::output_cached_data(tu_file* out)
	// Dump our data to *out.
	{
		out->write_le32(m_style);
		write_coord_array(out, m_coords);
	}

	
	void	line_strip::input_cached_data(tu_file* in)
	// Slurp our data from *out.
	{
		m_style = in->read_le32();
		read_coord_array(in, &m_coords);
	}


	// Utility: very simple greedy tri-stripper.  Useful for
	// stripping the stacks of trapezoids that come out of our
	// tesselator.
	struct tri_stripper
	{
		// A set of strips; we'll join them together into one
		// strip during the flush.
		array< array<point> >	m_strips;
		int	m_last_strip_used;

		tri_stripper()
			: m_last_strip_used(-1)
		{
		}

		void	add_trapezoid(const point& l0, const point& r0, const point& l1, const point& r1)
		// Add two triangles to our strip.
		{
			// See if we can attach this mini-strip to an existing strip.

			if (l0.bitwise_equal(r0) == false)
			{
				// Check the next strip first; trapezoids will
				// tend to arrive in rotating order through
				// the active strips.
				assert(m_last_strip_used >= -1 && m_last_strip_used < m_strips.size());
				int i = m_last_strip_used + 1, n = m_strips.size();
				for ( ; i < n; i++)
				{
					array<point>&	str = m_strips[i];
					assert(str.size() >= 3);	// should have at least one tri already.
				
					int	last = str.size() - 1;
					if (str[last - 1].bitwise_equal(l0) && str[last].bitwise_equal(r0))
					{
						// Can join these tris to this strip.
						str.push_back(l1);
						str.push_back(r1);
						m_last_strip_used = i;
						return;
					}
				}
				for (i = 0; i <= m_last_strip_used; i++)
				{
					array<point>&	str = m_strips[i];
					assert(str.size() >= 3);	// should have at least one tri already.
				
					int	last = str.size() - 1;
					if (str[last - 1].bitwise_equal(l0) && str[last].bitwise_equal(r0))
					{
						// Can join these tris to this strip.
						str.push_back(l1);
						str.push_back(r1);
						m_last_strip_used = i;
						return;
					}
				}
			}
			// else this trapezoid is pointy on top, so
			// it's almost certainly the start of a new
			// strip.  Don't bother searching current
			// strips.

			// Can't join with existing strip, so start a new strip.
			m_strips.resize(m_strips.size() + 1);
			m_strips.back().resize(4);
			m_strips.back()[0] = l0;
			m_strips.back()[1] = r0;
			m_strips.back()[2] = l1;
			m_strips.back()[3] = r1;
		}


		void	flush(mesh_set* m, int style) const
		// Join sub-strips together, and push the whole thing into the given mesh_set,
		// under the given style.
		{
			if (m_strips.size())
			{
				array<point>	big_strip;

				big_strip = m_strips[0];
				assert(big_strip.size() >= 3);

				for (int i = 1, n = m_strips.size(); i < n; i++)
				{
					// Append to the big strip.
					const array<point>&	str = m_strips[i];
					assert(str.size() >= 3);	// should have at least one tri already.
				
					int	last = big_strip.size() - 1;
					if (big_strip[last] == str[1]
					    && big_strip[last - 1] == str[0])
					{
 						// Strips fit right together.  Append.
						big_strip.append(&str[2], str.size() - 2);
					}
					else if (big_strip[last] == str[0]
						 && big_strip[last - 1] == str[1])
					{
						// Strips fit together with a half-twist.
						point	to_dup = big_strip[last - 1];
						big_strip.push_back(to_dup);
						big_strip.append(&str[2], str.size() - 2);
					}
					else
					{
						// Strips need a degenerate to link them together.
						point	to_dup = big_strip[last];
						big_strip.push_back(to_dup);
						big_strip.push_back(str[0]);
						big_strip.append(str);
					}
				}

				m->set_tri_strip(style, &big_strip[0], big_strip.size());
			}
		}
	};


	//
	// mesh_set
	//


	mesh_set::mesh_set()
		:
//		m_last_frame_rendered(-1),
		m_error_tolerance(0)	// invalid -- don't use this constructor; it's only here for array (@@ fix array)
	{
	}

	mesh_set::mesh_set(const tesselate::tesselating_shape* sh, float error_tolerance)
	// Tesselate the shape's paths into a different mesh for each fill style.
		:
//		m_last_frame_rendered(0),
		m_error_tolerance(error_tolerance)
	{
		struct collect_traps : public tesselate::trapezoid_accepter
		{
			mesh_set*	m;	// the mesh_set that receives trapezoids.

			// strips-in-progress.
			hash<int, tri_stripper*>	m_strips;

			collect_traps(mesh_set* set) : m(set) {}
			virtual ~collect_traps() {}

			// Overrides from trapezoid_accepter
			virtual void	accept_trapezoid(int style, const tesselate::trapezoid& tr)
			{
				// Add trapezoid to appropriate stripper.

				tri_stripper*	s = NULL;
				m_strips.get(style, &s);
				if (s == NULL)
				{
					s = new tri_stripper;
					m_strips.add(style, s);
				}

				s->add_trapezoid(
					point(tr.m_lx0, tr.m_y0),
					point(tr.m_rx0, tr.m_y0),
					point(tr.m_lx1, tr.m_y1),
					point(tr.m_rx1, tr.m_y1));
			}

			virtual void	accept_line_strip(int style, const point coords[], int coord_count)
			// Remember this line strip in our mesh set.
			{
				m->add_line_strip(style, coords, coord_count);
			}

			void	flush() const
			// Push our strips into the mesh set.
			{
				for (hash<int, tri_stripper*>::const_iterator it = m_strips.begin();
				     it != m_strips.end();
				     ++it)
				{
					// Push strip into m.
					tri_stripper*	s = it->second;
					s->flush(m, it->first);
					
					delete s;
				}
			}
		};
		collect_traps	accepter(this);

		sh->tesselate(error_tolerance, &accepter);
		accepter.flush();

		// triangles should be collected now into the meshes for each fill style.
	}


//	int	mesh_set::get_last_frame_rendered() const { return m_last_frame_rendered; }
//	void	mesh_set::set_last_frame_rendered(int frame_counter) { m_last_frame_rendered = frame_counter; }


	void	mesh_set::display(
		const matrix& mat,
		const cxform& cx,
		const array<fill_style>& fills,
		const array<line_style>& line_styles) const
	// Throw our meshes at the renderer.
	{
		assert(m_error_tolerance > 0);

		// Setup transforms.
		render::set_matrix(mat);
		render::set_cxform(cx);

		// Dump meshes into renderer, one mesh per style.
		for (int i = 0; i < m_meshes.size(); i++)
		{
			m_meshes[i].display(fills[i], 1.0);
		}

		// Dump line-strips into renderer.
		{for (int i = 0; i < m_line_strips.size(); i++)
		{
			int	style = m_line_strips[i].get_style();
			m_line_strips[i].display(line_styles[style], 1.0);
		}}
	}

	void	mesh_set::display(
		const matrix& mat,
		const cxform& cx,
		const array<morph_fill_style>& fills,
		const array<morph_line_style>& line_styles,
		float ratio) const
	// Throw our meshes at the renderer.
	{
		assert(m_error_tolerance > 0);

		// Setup transforms.
		render::set_matrix(mat);
		render::set_cxform(cx);

		// Dump meshes into renderer, one mesh per style.
		for (int i = 0; i < m_meshes.size(); i++)
		{
			m_meshes[i].display(fills[i], ratio);
		}

		// Dump line-strips into renderer.
		{for (int i = 0; i < m_line_strips.size(); i++)
		{
			int	style = m_line_strips[i].get_style();
			m_line_strips[i].display(line_styles[style], ratio);
		}}
	}

	void	mesh_set::set_tri_strip(int style, const point pts[], int count)
	// Set mesh associated with the given fill style to the
	// specified triangle strip.
	{
		assert(style >= 0);
		assert(style < 10000);	// sanity check

		// Expand our mesh list if necessary.
		if (style >= m_meshes.size())
		{
			m_meshes.resize(style + 1);
		}

		m_meshes[style].set_tri_strip(pts, count);
	}

	
	void	mesh_set::add_line_strip(int style, const point coords[], int coord_count)
	// Add the specified line strip to our list of things to render.
	{
		assert(style >= 0);
		assert(style < 1000);	// sanity check
		assert(coords != NULL);
		assert(coord_count > 1);

		m_line_strips.push_back(line_strip(style, coords, coord_count));
	}


	void	mesh_set::output_cached_data(tu_file* out)
	// Dump our data to the output stream.
	{
		out->write_float32(m_error_tolerance);

		int	mesh_n = m_meshes.size();
		out->write_le32(mesh_n);
		for (int i = 0; i < mesh_n; i++)
		{
			m_meshes[i].output_cached_data(out);
		}

		int	lines_n = m_line_strips.size();
		out->write_le32(lines_n);
		{for (int i = 0; i < lines_n; i++)
		{
			m_line_strips[i].output_cached_data(out);
		}}
	}


	void	mesh_set::input_cached_data(tu_file* in)
	// Grab our data from the input stream.
	{
		m_error_tolerance = in->read_float32();

		int	mesh_n = in->read_le32();
		m_meshes.resize(mesh_n);
		for (int i = 0; i < mesh_n; i++)
		{
			m_meshes[i].input_cached_data(in);
		}

		int	lines_n = in->read_le32();
		m_line_strips.resize(lines_n);
		{for (int i = 0; i < lines_n; i++)
		{
			m_line_strips[i].input_cached_data(in);
		}}
	}


	//
	// helper functions.
	//


	static void	read_fill_styles(array<fill_style>* styles, stream* in, int tag_type, movie_definition_sub* m)
	// Read fill styles, and push them onto the given style array.
	{
		assert(styles);

		// Get the count.
		int	fill_style_count = in->read_u8();
		if (tag_type > 2)
		{
			if (fill_style_count == 0xFF)
			{
				fill_style_count = in->read_u16();
			}
		}

		IF_VERBOSE_PARSE(log_msg("  read_fill_styles: count = %d\n", fill_style_count));

		// Read the styles.
		for (int i = 0; i < fill_style_count; i++)
		{
			(*styles).resize((*styles).size() + 1);
			(*styles)[(*styles).size() - 1].read(in, tag_type, m);
		}
	}


	static void	read_line_styles(array<line_style>* styles, stream* in, int tag_type)
	// Read line styles and push them onto the back of the given array.
	{
		// Get the count.
		int	line_style_count = in->read_u8();

		IF_VERBOSE_PARSE(log_msg("  read_line_styles: count = %d\n", line_style_count));

		// @@ does the 0xFF flag apply to all tag types?
		// if (tag_type > 2)
		// {
			if (line_style_count == 0xFF)
			{
				line_style_count = in->read_u16();
				IF_VERBOSE_PARSE(log_msg("  read_line_styles: count2 = %d\n", line_style_count));
			}
		// }

		// Read the styles.
		for (int i = 0; i < line_style_count; i++)
		{
			(*styles).resize((*styles).size() + 1);
			(*styles)[(*styles).size() - 1].read(in, tag_type);
		}
	}


	//
	// shape_character_def
	//


	shape_character_def::shape_character_def()
	{
	}


	shape_character_def::~shape_character_def()
	{
		// Free our mesh_sets.
		for (int i = 0; i < m_cached_meshes.size(); i++)
		{
			delete m_cached_meshes[i];
		}
	}


	void	shape_character_def::read(stream* in, int tag_type, bool with_style, movie_definition_sub* m)
	{
		if (with_style)
		{
			m_bound.read(in);
			read_fill_styles(&m_fill_styles, in, tag_type, m);
			read_line_styles(&m_line_styles, in, tag_type);
		}

		//
		// SHAPE
		//
		int	num_fill_bits = in->read_uint(4);
		int	num_line_bits = in->read_uint(4);

		IF_VERBOSE_PARSE(log_msg("  shape_character_def read: nfillbits = %d, nlinebits = %d\n", num_fill_bits, num_line_bits));

		// These are state variables that keep the
		// current position & style of the shape
		// outline, and vary as we read the edge data.
		//
		// At the moment we just store each edge with
		// the full necessary info to render it, which
		// is simple but not optimally efficient.
		int	fill_base = 0;
		int	line_base = 0;
		float	x = 0, y = 0;
		path	current_path;

#define SHAPE_LOG 0
		// SHAPERECORDS
		for (;;) {
			int	type_flag = in->read_uint(1);
			if (type_flag == 0)
			{
				// Parse the record.
				int	flags = in->read_uint(5);
				if (flags == 0) {
					// End of shape records.

					// Store the current path if any.
					if (! current_path.is_empty())
					{
						m_paths.push_back(current_path);
						current_path.m_edges.resize(0);
					}

					break;
				}
				if (flags & 0x01)
				{
					// move_to = 1;

					// Store the current path if any, and prepare a fresh one.
					if (! current_path.is_empty())
					{
						m_paths.push_back(current_path);
						current_path.m_edges.resize(0);
					}

					int	num_move_bits = in->read_uint(5);
					int	move_x = in->read_sint(num_move_bits);
					int	move_y = in->read_sint(num_move_bits);

					x = (float) move_x;
					y = (float) move_y;

					// Set the beginning of the path.
					current_path.m_ax = x;
					current_path.m_ay = y;

					if (SHAPE_LOG) IF_VERBOSE_PARSE(log_msg("  shape_character read: moveto %4g %4g\n", x, y));
				}
				if ((flags & 0x02)
					&& num_fill_bits > 0)
				{
					// fill_style_0_change = 1;
					if (! current_path.is_empty())
					{
						m_paths.push_back(current_path);
						current_path.m_edges.resize(0);
						current_path.m_ax = x;
						current_path.m_ay = y;
					}
					int	style = in->read_uint(num_fill_bits);
					if (style > 0)
					{
						style += fill_base;
					}
					current_path.m_fill0 = style;
					if (SHAPE_LOG) IF_VERBOSE_PARSE(log_msg("  shape_character read: fill0 = %d\n", current_path.m_fill0));
				}
				if ((flags & 0x04)
					&& num_fill_bits > 0)
				{
					// fill_style_1_change = 1;
					if (! current_path.is_empty())
					{
						m_paths.push_back(current_path);
						current_path.m_edges.resize(0);
						current_path.m_ax = x;
						current_path.m_ay = y;
					}
					int	style = in->read_uint(num_fill_bits);
					if (style > 0)
					{
						style += fill_base;
					}
					current_path.m_fill1 = style;
					if (SHAPE_LOG) IF_VERBOSE_PARSE(log_msg("  shape_character read: fill1 = %d\n", current_path.m_fill1));
				}
				if ((flags & 0x08)
					&& num_line_bits > 0)
				{
					// line_style_change = 1;
					if (! current_path.is_empty())
					{
						m_paths.push_back(current_path);
						current_path.m_edges.resize(0);
						current_path.m_ax = x;
						current_path.m_ay = y;
					}
					int	style = in->read_uint(num_line_bits);
					if (style > 0)
					{
						style += line_base;
					}
					current_path.m_line = style;
					if (SHAPE_LOG) IF_VERBOSE_PARSE(log_msg("  shape_character_read: line = %d\n", current_path.m_line));
				}
				if (flags & 0x10) {
					assert(tag_type >= 22);

					IF_VERBOSE_PARSE(log_msg("  shape_character read: more fill styles\n"));

					// Store the current path if any.
					if (! current_path.is_empty())
					{
						m_paths.push_back(current_path);
						current_path.m_edges.resize(0);

						// Clear styles.
						current_path.m_fill0 = -1;
						current_path.m_fill1 = -1;
						current_path.m_line = -1;
					}
					// Tack on an empty path signalling a new shape.
					// @@ need better understanding of whether this is correct??!?!!
					// @@ i.e., we should just start a whole new shape here, right?
					m_paths.push_back(path());
					m_paths.back().m_new_shape = true;

					fill_base = m_fill_styles.size();
					line_base = m_line_styles.size();
					read_fill_styles(&m_fill_styles, in, tag_type, m);
					read_line_styles(&m_line_styles, in, tag_type);
					num_fill_bits = in->read_uint(4);
					num_line_bits = in->read_uint(4);
				}
			}
			else
			{
				// EDGERECORD
				int	edge_flag = in->read_uint(1);
				if (edge_flag == 0)
				{
					// curved edge
					int num_bits = 2 + in->read_uint(4);
					float	cx = x + in->read_sint(num_bits);
					float	cy = y + in->read_sint(num_bits);
					float	ax = cx + in->read_sint(num_bits);
					float	ay = cy + in->read_sint(num_bits);

					if (SHAPE_LOG) IF_VERBOSE_PARSE(log_msg("  shape_character read: curved edge   = %4g %4g - %4g %4g - %4g %4g\n", x, y, cx, cy, ax, ay));

					current_path.m_edges.push_back(edge(cx, cy, ax, ay));

					x = ax;
					y = ay;
				}
				else
				{
					// straight edge
					int	num_bits = 2 + in->read_uint(4);
					int	line_flag = in->read_uint(1);
					float	dx = 0, dy = 0;
					if (line_flag)
					{
						// General line.
						dx = (float) in->read_sint(num_bits);
						dy = (float) in->read_sint(num_bits);
					}
					else
					{
						int	vert_flag = in->read_uint(1);
						if (vert_flag == 0) {
							// Horizontal line.
							dx = (float) in->read_sint(num_bits);
						} else {
							// Vertical line.
							dy = (float) in->read_sint(num_bits);
						}
					}

					if (SHAPE_LOG) IF_VERBOSE_PARSE(log_msg("  shape_character_read: straight edge = %4g %4g - %4g %4g\n", x, y, x + dx, y + dy));

					current_path.m_edges.push_back(edge(x + dx, y + dy, x + dx, y + dy));

					x += dx;
					y += dy;
				}
			}
		}
	}


	void	shape_character_def::display(character* inst)
	// Draw the shape using our own inherent styles.
	{
//		printf("%s(%d): ", __PRETTY_FUNCTION__, __LINE__);

		matrix	mat = inst->get_world_matrix();
		cxform	cx = inst->get_world_cxform();

		float	pixel_scale = inst->get_parent()->get_pixel_scale();
		display(mat, cx, pixel_scale, m_fill_styles, m_line_styles);
	}


#ifdef DEBUG_DISPLAY_SHAPE_PATHS

#include "ogl.h"


	static void	point_normalize(point* p)
	{
		float	mag2 = p->m_x * p->m_x + p->m_y * p->m_y;
		if (mag2 < 1e-9f)
		{
			// Very short vector.
			// @@ log error

			// Arbitrary unit vector.
			p->m_x = 1;
			p->m_y = 0;
		}

		float	inv_mag = 1.0f / sqrtf(mag2);
		p->m_x *= inv_mag;
		p->m_y *= inv_mag;
	}


	static void	show_fill_number(const point& p, int fill_number)
	{
		// We're inside a glBegin(GL_LINES)

		// Eh, let's do it in binary, least sig four bits...
		float	x = p.m_x;
		float	y = p.m_y;

		int	mask = 8;
		while (mask)
		{
			if (mask & fill_number)
			{
				// Vert line --> 1.
				glVertex2f(x, y - 40.0f);
				glVertex2f(x, y + 40.0f);
			}
			else
			{
				// Rectangle --> 0.
				glVertex2f(x - 10.0f, y - 40.0f);
				glVertex2f(x + 10.0f, y - 40.0f);

				glVertex2f(x + 10.0f, y - 40.0f);
				glVertex2f(x + 10.0f, y + 40.0f);

				glVertex2f(x - 10.0f, y + 40.0f);
				glVertex2f(x + 10.0f, y + 40.0f);

				glVertex2f(x - 10.0f, y - 40.0f);
				glVertex2f(x - 10.0f, y + 40.0f);
			}
			x += 40.0f;
			mask >>= 1;
		}
	}


	static void	debug_display_shape_paths(
		const matrix& mat,
		float object_space_max_error,
		const array<path>& paths,
		const array<fill_style>& fill_styles,
		const array<line_style>& line_styles)
	{
		for (int i = 0; i < paths.size(); i++)
		{
//			if (i > 0) break;//xxxxxxxx
			const path&	p = paths[i];

			if (p.m_fill0 == 0 && p.m_fill1 == 0)
			{
				continue;
			}

			gnash::render::set_matrix(mat);

			// Color the line according to which side has
			// fills.
			if (p.m_fill0 == 0) glColor4f(1, 0, 0, 0.5);
			else if (p.m_fill1 == 0) glColor4f(0, 1, 0, 0.5);
			else glColor4f(0, 0, 1, 0.5);

			// Offset according to which loop we are.
			float	offset_x = (i & 1) * 80.0f;
			float	offset_y = ((i & 2) >> 1) * 80.0f;
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glTranslatef(offset_x, offset_y, 0.f);

			point	pt;

			glBegin(GL_LINE_STRIP);

			mat.transform(&pt, point(p.m_ax, p.m_ay));
			glVertex2f(pt.m_x, pt.m_y);

			for (int j = 0; j < p.m_edges.size(); j++)
			{
				mat.transform(&pt, point(p.m_edges[j].m_cx, p.m_edges[j].m_cy));
				glVertex2f(pt.m_x, pt.m_y);
				mat.transform(&pt, point(p.m_edges[j].m_ax, p.m_edges[j].m_ay));
				glVertex2f(pt.m_x, pt.m_y);
			}

			glEnd();

			// Draw arrowheads.
			point	dir, right, p0, p1;
			glBegin(GL_LINES);
			{for (int j = 0; j < p.m_edges.size(); j++)
			{
				mat.transform(&p0, point(p.m_edges[j].m_cx, p.m_edges[j].m_cy));
				mat.transform(&p1, point(p.m_edges[j].m_ax, p.m_edges[j].m_ay));
				dir = point(p1.m_x - p0.m_x, p1.m_y - p0.m_y);
				point_normalize(&dir);
				right = point(-dir.m_y, dir.m_x);	// perpendicular

				const float	ARROW_MAG = 60.f;	// TWIPS?
				if (p.m_fill0 != 0)
				{
					glColor4f(0, 1, 0, 0.5);
					glVertex2f(p0.m_x,
						   p0.m_y);
					glVertex2f(p0.m_x - dir.m_x * ARROW_MAG - right.m_x * ARROW_MAG,
						   p0.m_y - dir.m_y * ARROW_MAG - right.m_y * ARROW_MAG);

					show_fill_number(point(p0.m_x - right.m_x * ARROW_MAG * 4,
							       p0.m_y - right.m_y * ARROW_MAG * 4),
							 p.m_fill0);
				}
				if (p.m_fill1 != 0)
				{
					glColor4f(1, 0, 0, 0.5);
					glVertex2f(p0.m_x,
						   p0.m_y);
					glVertex2f(p0.m_x - dir.m_x * ARROW_MAG + right.m_x * ARROW_MAG,
						   p0.m_y - dir.m_y * ARROW_MAG + right.m_y * ARROW_MAG);

					show_fill_number(point(p0.m_x + right.m_x * ARROW_MAG * 4,
							       p0.m_y + right.m_y * ARROW_MAG * 4),
							 p.m_fill1);
				}
			}}
			glEnd();

			glPopMatrix();
		}
	}
#endif // DEBUG_DISPLAY_SHAPE_PATHS

		
	void	shape_character_def::display(
		const matrix& mat,
		const cxform& cx,
		float pixel_scale,
		const array<fill_style>& fill_styles,
		const array<line_style>& line_styles) const
	// Display our shape.  Use the fill_styles arg to
	// override our default set of fill styles (e.g. when
	// rendering text).
	{
//		printf("%s(%d): ", __PRETTY_FUNCTION__, __LINE__);

		// Compute the error tolerance in object-space.
		float	max_scale = mat.get_max_scale();
		if (fabsf(max_scale) < 1e-6f)
		{
			// Scale is essentially zero.
			return;
		}

		float	object_space_max_error = 20.0f / max_scale / pixel_scale * s_curve_max_pixel_error;

#ifdef DEBUG_DISPLAY_SHAPE_PATHS
		// Render a debug view of shape path outlines, instead
		// of the tesselated shapes themselves.
		if (gnash_debug_show_paths)
		{
			debug_display_shape_paths(mat, object_space_max_error, m_paths, fill_styles, line_styles);

			return;
		}
#endif // DEBUG_DISPLAY_SHAPE_PATHS

		// See if we have an acceptable mesh available; if so then render with it.
		for (int i = 0, n = m_cached_meshes.size(); i < n; i++)
		{
			const mesh_set*	candidate = m_cached_meshes[i];

			if (object_space_max_error > candidate->get_error_tolerance() * 3.0f)
			{
				// Mesh is too high-res; the remaining meshes are higher res,
				// so stop searching and build an appropriately scaled mesh.
				break;
			}

			if (object_space_max_error > candidate->get_error_tolerance())
			{
				// Do it.
				candidate->display(mat, cx, fill_styles, line_styles);
				return;
			}
		}

		// Construct a new mesh to handle this error tolerance.
		mesh_set*	m = new mesh_set(this, object_space_max_error * 0.75f);
		m_cached_meshes.push_back(m);
		m->display(mat, cx, fill_styles, line_styles);
		
		sort_and_clean_meshes();
	}


	static int	sort_by_decreasing_error(const void* A, const void* B)
	{
		const mesh_set*	a = *(const mesh_set**) A;
		const mesh_set*	b = *(const mesh_set**) B;

		if (a->get_error_tolerance() < b->get_error_tolerance())
		{
			return 1;
		}
		else if (a->get_error_tolerance() > b->get_error_tolerance())
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}


	void	shape_character_def::sort_and_clean_meshes() const
	// Maintain cached meshes.  Clean out mesh_sets that haven't
	// been used recently, and make sure they're sorted from high
	// error to low error.
	{
		// Re-sort.
		if (m_cached_meshes.size() > 0)
		{
			qsort(
				&m_cached_meshes[0],
				m_cached_meshes.size(),
				sizeof(m_cached_meshes[0]),
				sort_by_decreasing_error);

			// Check to make sure the sort worked as intended.
			#ifndef NDEBUG
			for (int i = 0, n = m_cached_meshes.size() - 1; i < n; i++)
			{
				const mesh_set*	a = m_cached_meshes[i];
				const mesh_set*	b = m_cached_meshes[i + 1];

				assert(a->get_error_tolerance() > b->get_error_tolerance());
			}
			#endif // not NDEBUG
		}
	}

		
	void	shape_character_def::tesselate(float error_tolerance, tesselate::trapezoid_accepter* accepter) const
	// Push our shape data through the tesselator.
	{
		tesselate::begin_shape(accepter, error_tolerance);
		for (int i = 0; i < m_paths.size(); i++)
		{
			if (m_paths[i].m_new_shape == true)
			{
				// Hm; should handle separate sub-shapes in a less lame way.
				tesselate::end_shape();
				tesselate::begin_shape(accepter, error_tolerance);
			}
			else
			{
				m_paths[i].tesselate();
			}
		}
		tesselate::end_shape();
	}


	bool	shape_character_def::point_test_local(float x, float y)
	// Return true if the specified point is on the interior of our shape.
	// Incoming coords are local coords.
	{
		if (m_bound.point_test(x, y) == false)
		{
			// Early out.
			return false;
		}

		// Try each of the paths.
		for (int i = 0; i < m_paths.size(); i++)
		{
			if (m_paths[i].point_test(x, y))
			{
				return true;
			}
		}

		return false;
	}


	float shape_character_def::get_height_local()
	{
		return m_bound.height();
	}

	float shape_character_def::get_width_local()
	{
		return m_bound.width();
	}


	void	shape_character_def::compute_bound(rect* r) const
	// Find the bounds of this shape, and store them in
	// the given rectangle.
	{
		r->m_x_min = 1e10f;
		r->m_y_min = 1e10f;
		r->m_x_max = -1e10f;
		r->m_y_max = -1e10f;

		for (int i = 0; i < m_paths.size(); i++)
		{
			const path&	p = m_paths[i];
			r->expand_to_point(p.m_ax, p.m_ay);
			for (int j = 0; j < p.m_edges.size(); j++)
			{
				r->expand_to_point(p.m_edges[j].m_ax, p.m_edges[j].m_ay);
//					r->expand_to_point(p.m_edges[j].m_cx, p.m_edges[j].m_cy);
			}
		}
	}


	void	shape_character_def::output_cached_data(tu_file* out, const cache_options& options)
	// Dump our precomputed mesh data to the given stream.
	{
		int	n = m_cached_meshes.size();
		out->write_le32(n);

		for (int i = 0; i < n; i++)
		{
			m_cached_meshes[i]->output_cached_data(out);
		}
	}


	void	shape_character_def::input_cached_data(tu_file* in)
	// Initialize our mesh data from the given stream.
	{
		int	n = in->read_le32();

		m_cached_meshes.resize(n);

		for (int i = 0; i < n; i++)
		{
			mesh_set*	ms = new mesh_set();
			ms->input_cached_data(in);
			m_cached_meshes[i] = ms;
		}
	}

	
}	// end namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
