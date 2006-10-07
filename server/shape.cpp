// shape.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Quadratic bezier outline shapes, the basis for most SWF rendering.


#include "shape.h"

#include "impl.h"
//#include "log.h"
#include "render.h"
#include "stream.h"
#include "tesselate.h"
#include "movie_definition.h" // TODO: check if really needed
#include "bitmap_character_def.h"

#include "tu_file.h"

#include <cfloat>


namespace gnash {

namespace tesselate {
tesselating_shape::~tesselating_shape()
{
}
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
	if ( is_straight() )
	{
		//log_msg("is straight!!");
		tesselate::add_line_segment(m_ax, m_ay);
	}
	else
	{
		tesselate::add_curve_segment(m_cx, m_cy, m_ax, m_ay);
	}
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
    if (m_edges.size() <= 0) {
	return false;
    }

    if (m_fill0 < 0) {
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
    for (int i = 0, n = m_edges.size(); i < n; i++) {
	const edge& e = m_edges[i];
	
	float x1 = e.m_ax;
	float y1 = e.m_ay;
	
	if (e.is_straight()) {
	    // Straight-line case.
	    
	    // See if (x0,y0)-(x1,y1) crosses (x,y)-(infinity,y)
	    
	    // Does the segment straddle the horizontal ray?
	    bool cross_up = (y0 < y && y1 >= y);
	    bool cross_down = (!cross_up) && (y0 > y && y1 <= y);
	    if (cross_up || cross_down)	{
		// Straddles.
		
		// Is the crossing point to the right of x?
		float dy = y1 - y0;

		// x_intercept = x0 + (x1 - x0) * (y - y0) / dy;
		float x_intercept_times_dy = x0 * dy + (x1 - x0) * (y - y0);

		// text x_intercept > x
				
		// factor out the division; two cases depending on sign of dy
		if (cross_up) {
		    assert(dy > 0);
		    if (x_intercept_times_dy > x * dy) {
			ray_crossings++;
		    }
		} else {
		    // dy is negative; reverse the inequality test
		    assert(dy < 0);
		    if (x_intercept_times_dy < x * dy) {
			ray_crossings++;
		    }
		}
	    }
	} else {
	    // Curve case.
	    float cx = e.m_cx;
	    float cy = e.m_cy;

	    // Find whether & where the curve crosses y
	    if ((y0 < y && y1 < y && cy < y)
		|| (y0 > y && y1 > y && cy > y)) {
		// All above or all below -- no possibility of crossing.
	    } else if (x0 < x && x1 < x && cx < x) {
		// All to the left -- no possibility of crossing to the right.
	    } else {
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
		if (rad < 0) {
		    // No real solutions.
		} else {
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

		    if (A != 0)	{
			float t0 = q / A;
			if (t0 >= 0 && t0 < 1) {
			    float x_at_t0 =
				x0 + 2 * (cx - x0) * t0 + (x1 + x0 - 2 * cx) * t0 * t0;
			    if (x_at_t0 > x) {
				ray_crossings++;
			    }
			}
		    }

		    if (q != 0)	{
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

    if (ray_crossings & 1) {
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
    for (unsigned int i = 0; i < m_edges.size(); i++) {
	m_edges[i].tesselate_curve();
    }
    tesselate::end_path();
}


// Utility.


void	write_coord_array(tu_file* out, const std::vector<int16_t>& pt_array)
    // Dump the given coordinate array into the given stream.
{
    int	n = pt_array.size();

    out->write_le32(n);
    for (int i = 0; i < n; i++)	{
	out->write_le16((uint16_t) pt_array[i]);
    }
}


void	read_coord_array(tu_file* in, std::vector<int16_t>* pt_array)
    // Read the coordinate array data from the stream into *pt_array.
{
    int	n = in->read_le32();

    pt_array->resize(n);
    for (int i = 0; i < n; i ++) {
	(*pt_array)[i] = (int16_t) in->read_le16();
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
    for (int i = 0; i < count; i++) {
	m_triangle_strip[i * 2] = int16_t(pts[i].m_x);
	m_triangle_strip[i * 2 + 1] = int16_t(pts[i].m_y);
    }

//		m_triangle_strip.resize(count);
//		memcpy(&m_triangle_strip[0], &pts[0], count * sizeof(point));
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
    // Default constructor, for std::vector<>.
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
    for (int i = 0; i < coord_count; i++) {
	m_coords[i * 2] = int16_t(coords[i].m_x);
	m_coords[i * 2 + 1] = int16_t(coords[i].m_y);
    }
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
class tri_stripper
{
public:
    // A set of strips; we'll join them together into one
    // strip during the flush.
    std::vector< std::vector<point> >	m_strips;
    int	m_last_strip_used;

    tri_stripper()
	: m_last_strip_used(-1)
	{
	}

    void	add_trapezoid(const point& l0, const point& r0, const point& l1, const point& r1)
	// Add two triangles to our strip.
	{
	    // See if we can attach this mini-strip to an existing strip.

	    if (l0.bitwise_equal(r0) == false) {
		// Check the next strip first; trapezoids will
		// tend to arrive in rotating order through
		// the active strips.
		assert(m_last_strip_used >= -1 && m_last_strip_used < (int) m_strips.size());
		int i = m_last_strip_used + 1, n = m_strips.size();
		for ( ; i < n; i++)	{
		    std::vector<point>&	str = m_strips[i];
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
		for (i = 0; i <= m_last_strip_used; i++) {
		    std::vector<point>&	str = m_strips[i];
		    assert(str.size() >= 3);	// should have at least one tri already.
				
		    int	last = str.size() - 1;
		    if (str[last - 1].bitwise_equal(l0) && str[last].bitwise_equal(r0))	{
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
	    if (m_strips.size()) {
		std::vector<point>	big_strip;

		big_strip = m_strips[0];
		assert(big_strip.size() >= 3);

		for (unsigned int i = 1, n = m_strips.size(); i < n; i++) {
		    // Append to the big strip.
		    const std::vector<point>&	str = m_strips[i];
		    assert(str.size() >= 3);	// should have at least one tri already.
				
		    int	last = big_strip.size() - 1;
		    if (big_strip[last] == str[1]
			&& big_strip[last - 1] == str[0]) {
			// Strips fit right together.  Append.
			big_strip.insert(big_strip.end(), str.begin() + 2, str.end());
		    } else if (big_strip[last] == str[0]
			       && big_strip[last - 1] == str[1]) {
			// Strips fit together with a half-twist.
			point	to_dup = big_strip[last - 1];
			big_strip.push_back(to_dup);
			big_strip.insert(big_strip.end(), str.begin() + 2, str.end());
		    } else {
			// Strips need a degenerate to link them together.
			point	to_dup = big_strip[last];
			big_strip.push_back(to_dup);
			big_strip.push_back(str[0]);
			big_strip.insert(big_strip.end(), str.begin(), str.end());
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
    class collect_traps : public tesselate::trapezoid_accepter
    {
    public:
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
		if (s == NULL) {
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
		     ++it) {
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



void	mesh_set::set_tri_strip(int style, const point pts[], int count)
    // Set mesh associated with the given fill style to the
    // specified triangle strip.
{
    assert(style >= 0);
    assert(style < 10000);	// sanity check

    // Expand our mesh list if necessary.
    if (style >= (int) m_meshes.size()) {
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
    for (int i = 0; i < mesh_n; i++) {
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
    for (int i = 0; i < mesh_n; i++) {
	m_meshes[i].input_cached_data(in);
    }

    int	lines_n = in->read_le32();
    m_line_strips.resize(lines_n);
    {for (int i = 0; i < lines_n; i++)
	{
	    m_line_strips[i].input_cached_data(in);
	}}
}

}	// end namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
