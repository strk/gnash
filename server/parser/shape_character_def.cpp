// shape_character_def.cpp:  Quadratic bezier outline shapes, for Gnash.
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

/* $Id: shape_character_def.cpp,v 1.35 2007/08/27 16:52:28 strk Exp $ */

// Based on the public domain shape.cpp of Thatcher Ulrich <tu@tulrich.com> 2003

// Quadratic bezier outline shapes are the basis for most SWF rendering.

#include "shape_character_def.h"

#include "impl.h"
#include "log.h"
#include "render.h"
#include "stream.h"
#include "sprite_instance.h"

#include "tu_file.h"

#include <cfloat>
#include <algorithm>

// Define the macro below to always compute bounds for shape characters
// and compare them with the bounds encoded in the SWF
//#define GNASH_DEBUG_SHAPE_BOUNDS 1

//#define DEBUG_DISPLAY_SHAPE_PATHS    // won't probably work anymore (Udo)
#ifdef DEBUG_DISPLAY_SHAPE_PATHS
// For debugging only!
bool	gnash_debug_show_paths = false;
#endif // DEBUG_DISPLAY_SHAPE_PATHS


namespace gnash {

static float	s_curve_max_pixel_error = 1.0f;


//
// helper functions.
//

void	set_curve_max_pixel_error(float pixel_error)
{
    s_curve_max_pixel_error = fclamp(pixel_error, 1e-6f, 1e6f);
}

float	get_curve_max_pixel_error()
{
    return s_curve_max_pixel_error;
}


// Read fill styles, and push them onto the given style array.
static void
read_fill_styles(std::vector<fill_style>& styles, stream* in,
		int tag_type, movie_definition* m)
{

	// Get the count.
	in->ensureBytes(1);
	uint16_t fill_style_count = in->read_u8();
	if (tag_type > 2)
	{
		if (fill_style_count == 0xFF)
		{
			in->ensureBytes(2);
			fill_style_count = in->read_u16();
		}
	}

	IF_VERBOSE_PARSE (
		log_parse(_("  read_fill_styles: count = %u"), fill_style_count);
	);

	// Read the styles.
	styles.reserve(styles.size()+fill_style_count);
	for (uint16_t i = 0; i < fill_style_count; ++i)
	{
		// TODO: add a fill_style constructor directly
		//       reading from stream
		fill_style fs;
		fs.read(in, tag_type, m);
		// Push a style anyway, so any path referring to
		// it still finds it..
		styles.push_back(fs);
	}

}


static void
read_line_styles(std::vector<line_style>& styles, stream* in, int tag_type)
    // Read line styles and push them onto the back of the given array.
{
    // Get the count.
    in->ensureBytes(1);
    int	line_style_count = in->read_u8();

		IF_VERBOSE_PARSE
		(
    log_parse(_("  read_line_styles: count = %d"), line_style_count);
    		);

    // @@ does the 0xFF flag apply to all tag types?
    // if (tag_type > 2)
    // {
    if (line_style_count == 0xFF) {
        in->ensureBytes(2);
	line_style_count = in->read_u16();
		IF_VERBOSE_PARSE
		(
	log_parse(_("  read_line_styles: count2 = %d"), line_style_count);
		);
    }
    // }

    // Read the styles.
    for (int i = 0; i < line_style_count; i++) {
	styles.resize(styles.size() + 1);
	//styles[styles.size() - 1].read(in, tag_type);
	styles.back().read(in, tag_type);
    }
}


shape_character_def::shape_character_def()
	:
	character_def(),
	m_fill_styles(),
	m_line_styles(),
	m_paths(),
	m_bound()
	//,m_cached_meshes()
{
}

shape_character_def::shape_character_def(const shape_character_def& o)
	:
	character_def(o),
	tesselate::tesselating_shape(o),
	m_fill_styles(o.m_fill_styles),
	m_line_styles(o.m_line_styles),
	m_paths(o.m_paths),
	m_bound(o.m_bound)
	//,m_cached_meshes()
{
}


shape_character_def::~shape_character_def()
{
	//clear_meshes();
}


void
shape_character_def::read(stream* in, int tag_type, bool with_style,
	movie_definition* m)
{
    if (with_style)
    {
	m_bound.read(in);

		IF_VERBOSE_PARSE
		(
		 std::string b = m_bound.toString();
    log_parse(_("  bound rect: %s"), b.c_str());
    		);

	read_fill_styles(m_fill_styles, in, tag_type, m);
	read_line_styles(m_line_styles, in, tag_type);
    }

    /// Adding a dummy fill style is just needed to make the
    /// parser somewhat more robust. This fill style is not
    /// really used, as text rendering will use style information
    /// from TEXTRECORD tag instead.
    ///
    if ( tag_type == SWF::DEFINEFONT || tag_type == SWF::DEFINEFONT2 )
    {
	    assert(!with_style);
	    //m_fill_styles.push_back(fill_style());
    }


    //log_msg("Read %u fill styles, %u line styles", m_fill_styles.size(), m_line_styles.size());

    int	num_fill_bits = in->read_uint(4);
    int	num_line_bits = in->read_uint(4);

		IF_VERBOSE_PARSE
		(
    log_parse(_("  shape_character_def read: nfillbits = %d, nlinebits = %d"), num_fill_bits, num_line_bits);
    		);

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
	bool isEdgeRecord = in->read_bit();
	if (!isEdgeRecord) {
	    // Parse the record.
	    int	flags = in->read_uint(5);
	    if (flags == flagEnd) {
		// End of shape records.

		// Store the current path if any.
		if (! current_path.is_empty())
		    {
			m_paths.push_back(current_path);
			current_path.m_edges.resize(0);
		    }

		break;
	    }
	    if (flags & flagMove) {
		// move_to = 1;

		// Store the current path if any, and prepare a fresh one.
		if (! current_path.is_empty()) {
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

		IF_VERBOSE_PARSE
		(
		if (SHAPE_LOG)
		    log_parse(_("  shape_character read: moveto %4g %4g"), x, y);
		);
	    }
	    if ((flags & flagFillStyle0Change) && num_fill_bits > 0)
	    {
		// fill_style_0_change = 1;
		if (! current_path.is_empty()) {
		    m_paths.push_back(current_path);
		    current_path.m_edges.resize(0);
		    current_path.m_ax = x;
		    current_path.m_ay = y;
		}
		unsigned style = in->read_uint(num_fill_bits);
		if (style > 0) {
		    style += fill_base;
		}

    		if ( tag_type == SWF::DEFINEFONT || tag_type == SWF::DEFINEFONT2 )
		{
			if ( style > 1 ) // 0:hide 1:renderer
			{
				IF_VERBOSE_MALFORMED_SWF(
				log_swferror(_("Invalid fill style %d in fillStyle0Change record for font tag (0 or 1 valid). Set to 0."), style);
				);
				style = 0;
			}
		}
		else
		{
			if ( style > m_fill_styles.size() ) // 1-based index 
			{
				IF_VERBOSE_MALFORMED_SWF(
				log_swferror(_("Invalid fill style %d in fillStyle0Change record - " SIZET_FMT " defined. Set to 0."), style, m_fill_styles.size());
				);
				style = 0;
			}
		}

		current_path.setLeftFill(style);
		IF_VERBOSE_PARSE(
		if (SHAPE_LOG) {
		    log_parse(_("  shape_character read: fill0 (left) = %d"), current_path.getLeftFill());
		}
		);

	    }
	    if ((flags & flagFillStyle1Change) && num_fill_bits > 0)
	    {
		// fill_style_1_change = 1;
		if (! current_path.is_empty()) {
		    m_paths.push_back(current_path);
		    current_path.m_edges.resize(0);
		    current_path.m_ax = x;
		    current_path.m_ay = y;
		}
		unsigned style = in->read_uint(num_fill_bits);
		if (style > 0) {
		    style += fill_base;
		}

    		if ( tag_type == SWF::DEFINEFONT || tag_type == SWF::DEFINEFONT2 )
		{
			if ( style > 1 ) // 0:hide 1:renderer
			{
				IF_VERBOSE_MALFORMED_SWF(
				log_swferror(_("Invalid fill style %d in fillStyle1Change record for font tag (0 or 1 valid). Set to 0."), style);
				);
				style = 0;
			}
		}
		else
		{
			if ( style > m_fill_styles.size() ) // 1-based index 
			{
				IF_VERBOSE_MALFORMED_SWF(
				log_swferror(_("Invalid fill style %d in fillStyle1Change record - " SIZET_FMT " defined. Set to 0."), style, m_fill_styles.size());
				);
				style = 0;
			}
		}
		current_path.setRightFill(style); // getRightFill() = style;
		IF_VERBOSE_PARSE (
		if (SHAPE_LOG) {
		    log_parse(_("  shape_character read: fill1 (right) = %d"), current_path.getRightFill());
		}
		)
	    }
	    if ((flags & flagLineStyleChange) && num_line_bits > 0)
	    {
		// line_style_change = 1;
		if (! current_path.is_empty()) {
		    m_paths.push_back(current_path);
		    current_path.m_edges.resize(0);
		    current_path.m_ax = x;
		    current_path.m_ay = y;
		}
		unsigned style = in->read_uint(num_line_bits);
		if (style > 0) {
		    style += line_base;
		}
    		if ( tag_type == SWF::DEFINEFONT || tag_type == SWF::DEFINEFONT2 )
		{
			if ( style > 1 ) // 0:hide 1:renderer
			{
				IF_VERBOSE_MALFORMED_SWF(
				log_swferror(_("Invalid line style %d in lineStyleChange record for font tag (0 or 1 valid). Set to 0."), style);
				);
				style = 0;
			}
		}
		else
		{
			if ( style > m_line_styles.size() ) // 1-based index 
			{
				IF_VERBOSE_MALFORMED_SWF(
				log_swferror(_("Invalid fill style %d in lineStyleChange record - " SIZET_FMT " defined. Set to 0."), style, m_line_styles.size());
				);
				style = 0;
			}
		}
		current_path.setLineStyle(style);
		IF_VERBOSE_PARSE (
		if (SHAPE_LOG)
		{
		    log_parse(_("  shape_character_read: line = %d"), current_path.getLineStyle());
		}
		)
	    }
	    if (flags & flagHasNewStyles)
	    {
		if (!with_style)
		{
			IF_VERBOSE_MALFORMED_SWF(
			log_swferror("Unexpected HasNewStyle flag in tag %d shape record", tag_type);
	    		)
			// Used to be tag_type += SWF::DEFINESHAPE2, but
			// I can't belive any such thing could be correct...
			continue;
		}

		IF_VERBOSE_PARSE (
		log_parse(_("  shape_character read: more fill styles"));
		);

		// Store the current path if any.
		if (! current_path.is_empty()) {
		    m_paths.push_back(current_path);
		    current_path.clear();
		}

		// Tack on an empty path signalling a new shape.
		// @@ need better understanding of whether this is correct??!?!!
		// @@ i.e., we should just start a whole new shape here, right?
		m_paths.push_back(path());
		m_paths.back().m_new_shape = true;

		fill_base = m_fill_styles.size();
		line_base = m_line_styles.size();
		read_fill_styles(m_fill_styles, in, tag_type, m);
		read_line_styles(m_line_styles, in, tag_type);
		num_fill_bits = in->read_uint(4);
		num_line_bits = in->read_uint(4);
	    }
	} else {
	    // EDGERECORD
	    bool edge_flag = in->read_bit();
	    if (edge_flag == 0) {
		// curved edge
		int num_bits = 2 + in->read_uint(4);
		float	cx = x + in->read_sint(num_bits);
		float	cy = y + in->read_sint(num_bits);
		float	ax = cx + in->read_sint(num_bits);
		float	ay = cy + in->read_sint(num_bits);

		IF_VERBOSE_PARSE (
		if (SHAPE_LOG)
		{
		    log_parse(_("  shape_character read: curved edge   = %4g %4g - %4g %4g - %4g %4g"), x, y, cx, cy, ax, ay);
		}
		);

		current_path.m_edges.push_back(edge(cx, cy, ax, ay));

		x = ax;
		y = ay;
	    } else {
		// straight edge
		int	num_bits = 2 + in->read_uint(4);
		int	line_flag = in->read_uint(1);
		float	dx = 0, dy = 0;
		if (line_flag) {
		    // General line.
		    dx = (float) in->read_sint(num_bits);
		    dy = (float) in->read_sint(num_bits);
		} else {
		    int	vert_flag = in->read_uint(1);
		    if (vert_flag == 0) {
			// Horizontal line.
			dx = (float) in->read_sint(num_bits);
		    } else {
			// Vertical line.
			dy = (float) in->read_sint(num_bits);
		    }
		}

		IF_VERBOSE_PARSE (
		if (SHAPE_LOG)
		{
		    log_parse(_("  shape_character_read: straight edge = %4g %4g - %4g %4g"), x, y, x + dx, y + dy);
		}
		);

		current_path.m_edges.push_back(edge(x + dx, y + dy, x + dx, y + dy));

		x += dx;
		y += dy;
	    }
	}
    }

    if ( ! with_style )
    {
        // TODO: performance would be improved by computing 
        //       the bounds as edges are parsed.
        compute_bound(&m_bound);
    }
#ifdef GNASH_DEBUG_SHAPE_BOUNDS
    else
    {
        rect computedBounds;
        compute_bound(&computedBounds);
        if ( computedBounds != m_bounds )
        {
            log_debug("Shape character read for tag %d contained embedded bounds %s, while we computed bounds %s",
                tag_type, m_bound.toString().c_str(), computedBounds.toString().c_str());
        }
    }
#endif // GNASH_DEBUG_SHAPE_BOUNDS
}


void	shape_character_def::display(character* inst)
    // Draw the shape using our own inherent styles.
{
//    GNASH_REPORT_FUNCTION;


  gnash::render::draw_shape_character(this, inst);


/*
    matrix	mat = inst->get_world_matrix();
    cxform	cx = inst->get_world_cxform();

    float	pixel_scale = inst->get_parent()->get_pixel_scale();
    display(mat, cx, pixel_scale, m_fill_styles, m_line_styles);
    */
}


#ifdef DEBUG_DISPLAY_SHAPE_PATHS

#include "ogl.h"


static void	point_normalize(point* p)
{
    float	mag2 = p->m_x * p->m_x + p->m_y * p->m_y;
    if (mag2 < 1e-9f) {
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
    while (mask) {
	if (mask & fill_number)	{
	    // Vert line --> 1.
	    glVertex2f(x, y - 40.0f);
	    glVertex2f(x, y + 40.0f);
	} else {
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
    float /* object_space_max_error */,
    const std::vector<path>& paths,
    const std::vector<fill_style>& /* fill_styles */,
    const std::vector<line_style>& /* line_styles */)
{
    for (unsigned int i = 0; i < paths.size(); i++) {
//			if (i > 0) break;//xxxxxxxx
	const path&	p = paths[i];

	if (p.getLeftFill() == 0 && p.getRightFill() == 0) {
	    continue;
	}

	gnash::render::set_matrix(mat);

	// Color the line according to which side has
	// fills.
	if (p.getLeftFill() == 0) glColor4f(1, 0, 0, 0.5);
	else if (p.getRightFill() == 0) glColor4f(0, 1, 0, 0.5);
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

	for (unsigned int j = 0; j < p.m_edges.size(); j++)	{
	    mat.transform(&pt, point(p.m_edges[j].m_cx, p.m_edges[j].m_cy));
	    glVertex2f(pt.m_x, pt.m_y);
	    mat.transform(&pt, point(p.m_edges[j].m_ax, p.m_edges[j].m_ay));
	    glVertex2f(pt.m_x, pt.m_y);
	}

	glEnd();

	// Draw arrowheads.
	point	dir, right, p0, p1;
	glBegin(GL_LINES);
	{for (unsigned int j = 0; j < p.m_edges.size(); j++)
	    {
		mat.transform(&p0, point(p.m_edges[j].m_cx, p.m_edges[j].m_cy));
		mat.transform(&p1, point(p.m_edges[j].m_ax, p.m_edges[j].m_ay));
		dir = point(p1.m_x - p0.m_x, p1.m_y - p0.m_y);
		point_normalize(&dir);
		right = point(-dir.m_y, dir.m_x);	// perpendicular

		const float	ARROW_MAG = 60.f;	// TWIPS?
		if (p.getLeftFill() != 0)
		    {
			glColor4f(0, 1, 0, 0.5);
			glVertex2f(p0.m_x,
				   p0.m_y);
			glVertex2f(p0.m_x - dir.m_x * ARROW_MAG - right.m_x * ARROW_MAG,
				   p0.m_y - dir.m_y * ARROW_MAG - right.m_y * ARROW_MAG);

			show_fill_number(point(p0.m_x - right.m_x * ARROW_MAG * 4,
					       p0.m_y - right.m_y * ARROW_MAG * 4),
					 p.getLeftFill());
		    }
		if (p.getRightFill() != 0)
		    {
			glColor4f(1, 0, 0, 0.5);
			glVertex2f(p0.m_x,
				   p0.m_y);
			glVertex2f(p0.m_x - dir.m_x * ARROW_MAG + right.m_x * ARROW_MAG,
				   p0.m_y - dir.m_y * ARROW_MAG + right.m_y * ARROW_MAG);

			show_fill_number(point(p0.m_x + right.m_x * ARROW_MAG * 4,
					       p0.m_y + right.m_y * ARROW_MAG * 4),
					 p.getRightFill());
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
    const std::vector<fill_style>& fill_styles,
    const std::vector<line_style>& line_styles) const
{
  shape_character_def* this_non_const = const_cast<shape_character_def*>(this);

  render_handler* renderer = get_render_handler();
  renderer->draw_shape_character(this_non_const, mat, cx, pixel_scale,
    fill_styles, line_styles);
}




void	shape_character_def::tesselate(float error_tolerance, tesselate::trapezoid_accepter* accepter) const
    // Push our shape data through the tesselator.
{
    tesselate::begin_shape(accepter, error_tolerance);
    for (unsigned int i = 0; i < m_paths.size(); i++) {
	if (m_paths[i].m_new_shape == true)	{
	    // Hm; should handle separate sub-shapes in a less lame way.
	    tesselate::end_shape();
	    tesselate::begin_shape(accepter, error_tolerance);
	} else {
	    m_paths[i].tesselate();
	}
    }
    tesselate::end_shape();
}


bool	shape_character_def::point_test_local(float x, float y)
    // Return true if the specified point is on the interior of our shape.
    // Incoming coords are local coords.
{
    if (m_bound.point_test(x, y) == false) {
	// Early out.
	return false;
    }

    size_t npaths = m_paths.size();
    
    if ( ! npaths ) return false;
    
    point pt(x, y);

    // Try each of the paths.
    for (size_t i = 0; i < npaths; ++i)
    {
	const path& pth = m_paths[i];

	if ( pth.empty() ) continue;

	// If it has a line style, check for strokes there
	if ( pth.m_line != 0 )
	{
		assert(m_line_styles.size() >= pth.m_line);
		line_style& ls = m_line_styles[pth.m_line-1];
		int thickness = ls.get_width();
		float sqdist;
		if ( thickness == 0 )
		{
			// hairline has always a tolerance of a single twip
			sqdist = 1;
		}
		else
		{
			float dist = thickness/2;
			sqdist = dist*dist;
		}
		//cout << "Thickness of line is " << thickness << " squared is " << sqdist << endl;
		if ( pth.withinSquareDistance(pt, sqdist) ) return true;
	}

	// Check for point in polygon (if filled - but that test is in point_test itself)
	if (pth.point_test(x, y)) return true;
    }

    return false;
}


float shape_character_def::get_height_local() const
{
    return m_bound.height();
}

float shape_character_def::get_width_local() const
{
    return m_bound.width();
}


void	shape_character_def::compute_bound(rect* r) const
    // Find the bounds of this shape, and store them in
    // the given rectangle.
{
    r->set_null();
    
    for (unsigned int i = 0; i < m_paths.size(); i++)
    {
    
        const path&	p = m_paths[i];

        size_t nedges = p.m_edges.size();
        if ( ! nedges ) continue;

        if (p.m_line)
	{
		unsigned thickness=m_line_styles[p.m_line-1].get_width();
		if ( thickness ) // hairlines don't add any bound
		{
			// NOTE: Half of thickness would be enough (and correct) for
			// radius, but that would not match how Flash calculates the
			// bounds using the drawing API.                        
			float radius = thickness;

			r->expand_to_circle(p.m_ax, p.m_ay, radius);
			for (unsigned int j = 0; j<nedges; j++)
			{
				r->expand_to_circle(p.m_edges[j].m_ax, p.m_edges[j].m_ay, radius);
				r->expand_to_circle(p.m_edges[j].m_cx, p.m_edges[j].m_cy, radius);
			}

			continue;
		}
	}

	assert(p.m_line==0 || m_line_styles[p.m_line-1].get_width()==0);

	r->expand_to_point(p.m_ax, p.m_ay);
	for (unsigned int j = 0; j<nedges; j++)
	{
		r->expand_to_point(p.m_edges[j].m_ax, p.m_edges[j].m_ay);
                r->expand_to_point(p.m_edges[j].m_cx, p.m_edges[j].m_cy);
	}
    }
}


#if 0 // deprecated

void
shape_character_def::clear_meshes()
{
    // Free our mesh_sets.
    for (unsigned int i = 0; i < m_cached_meshes.size(); i++) {
	delete m_cached_meshes[i];
    }
}

void	shape_character_def::output_cached_data(tu_file* out, const cache_options& /* options */)
    // Dump our precomputed mesh data to the given stream.
{
    int	n = m_cached_meshes.size();
    out->write_le32(n);

    for (int i = 0; i < n; i++) {
	m_cached_meshes[i]->output_cached_data(out);
    }
}


void	shape_character_def::input_cached_data(tu_file* in)
    // Initialize our mesh data from the given stream.
{
    int	n = in->read_le32();

    m_cached_meshes.resize(n);

    for (int i = 0; i < n; i++)	{
	mesh_set*	ms = new mesh_set();
	ms->input_cached_data(in);
	m_cached_meshes[i] = ms;
    }
}

#endif // deprecated cached data

#ifdef GNASH_USE_GC
void
shape_character_def::markReachableResources() const
{
	assert(isReachable());
	for (FillStyleVect::const_iterator i=m_fill_styles.begin(), e=m_fill_styles.end();
			i != e; ++i)
	{
		i->markReachableResources();
	}
}
#endif // GNASH_USE_GC

}	// end namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
