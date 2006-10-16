// types.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Some basic types for gnash.

#include "types.h"

#include "log.h"
#include "stream.h"
#include "render.h"
#include "gnash.h"
#include <cstring>


#ifndef HAVE_ISFINITE
# ifndef isfinite 
#  define isfinite finite
# endif 
#endif 

namespace gnash {

	//
	// point
	//


	bool	point::bitwise_equal(const point& p) const
	// Bitwise comparison; return true if *this is bitwise
	// identical to p.
	{
		return memcmp(this, &p, sizeof(p)) == 0;
	}


	//
	// cxform
	//


	cxform	cxform::identity;


	cxform::cxform()
	// Initialize to identity transform.
	{
		m_[0][0] = 1;
		m_[1][0] = 1;
		m_[2][0] = 1;
		m_[3][0] = 1;
		m_[0][1] = 0;
		m_[1][1] = 0;
		m_[2][1] = 0;
		m_[3][1] = 0;
	}

	void	cxform::concatenate(const cxform& c)
	// Concatenate c's transform onto ours.  When
	// transforming colors, c's transform is applied
	// first, then ours.
	{
		m_[0][1] += m_[0][0] * c.m_[0][1];
		m_[1][1] += m_[1][0] * c.m_[1][1];
		m_[2][1] += m_[2][0] * c.m_[2][1];
		m_[3][1] += m_[3][0] * c.m_[3][1];

		m_[0][0] *= c.m_[0][0];
		m_[1][0] *= c.m_[1][0];
		m_[2][0] *= c.m_[2][0];
		m_[3][0] *= c.m_[3][0];
	}

	
	rgba	cxform::transform(const rgba in) const
	// Apply our transform to the given color; return the result.
	{
		rgba	result;

		result.m_r = (uint8_t) fclamp(in.m_r * m_[0][0] + m_[0][1], 0, 255);
		result.m_g = (uint8_t) fclamp(in.m_g * m_[1][0] + m_[1][1], 0, 255);
		result.m_b = (uint8_t) fclamp(in.m_b * m_[2][0] + m_[2][1], 0, 255);
		result.m_a = (uint8_t) fclamp(in.m_a * m_[3][0] + m_[3][1], 0, 255);

		return result;
	}


	void	cxform::read_rgb(stream* in)
	{
		in->align();

		int	has_add = in->read_uint(1);
		int	has_mult = in->read_uint(1);
		int	nbits = in->read_uint(4);

		if (has_mult) {
			m_[0][0] = in->read_sint(nbits) / 255.0f;
			m_[1][0] = in->read_sint(nbits) / 255.0f;
			m_[2][0] = in->read_sint(nbits) / 255.0f;
			m_[3][0] = 1;
		}
		else {
			for (int i = 0; i < 4; i++) { m_[i][0] = 1; }
		}
		if (has_add) {
			m_[0][1] = (float) in->read_sint(nbits);
			m_[1][1] = (float) in->read_sint(nbits);
			m_[2][1] = (float) in->read_sint(nbits);
			m_[3][1] = 1;
		}
		else {
			for (int i = 0; i < 4; i++) { m_[i][1] = 0; }
		}
	}

	void	cxform::read_rgba(stream* in)
	{
		in->align();

		int	has_add = in->read_uint(1);
		int	has_mult = in->read_uint(1);
		int	nbits = in->read_uint(4);

		if (has_mult) {
			m_[0][0] = in->read_sint(nbits) / 256.0f;
			m_[1][0] = in->read_sint(nbits) / 256.0f;
			m_[2][0] = in->read_sint(nbits) / 256.0f;
			m_[3][0] = in->read_sint(nbits) / 256.0f;
		}
		else {
			for (int i = 0; i < 4; i++) { m_[i][0] = 1; }
		}
		if (has_add) {
			m_[0][1] = (float) in->read_sint(nbits);
			m_[1][1] = (float) in->read_sint(nbits);
			m_[2][1] = (float) in->read_sint(nbits);
			m_[3][1] = (float) in->read_sint(nbits);
		}
		else {
			for (int i = 0; i < 4; i++) { m_[i][1] = 0; }
		}
	}

        /// Force component values to be in legal range.
        void cxform::clamp()
	{
		m_[0][0] = fclamp(m_[0][0], 0, 1);
		m_[1][0] = fclamp(m_[1][0], 0, 1);
		m_[2][0] = fclamp(m_[2][0], 0, 1);
		m_[3][0] = fclamp(m_[3][0], 0, 1);
		
		m_[0][1] = fclamp(m_[0][1], -255.0f, 255.0f);
		m_[1][1] = fclamp(m_[1][1], -255.0f, 255.0f);
		m_[2][1] = fclamp(m_[2][1], -255.0f, 255.0f);
		m_[3][1] = fclamp(m_[3][1], -255.0f, 255.0f);
	}

	void	cxform::print() const
	// Debug log.
	{
		log_parse("    *         +");
		log_parse("| %4.4f %4.4f|", m_[0][0], m_[0][1]);
		log_parse("| %4.4f %4.4f|", m_[1][0], m_[1][1]);
		log_parse("| %4.4f %4.4f|", m_[2][0], m_[2][1]);
		log_parse("| %4.4f %4.4f|", m_[3][0], m_[3][1]);
	}


	//
	// rgba
	//

	void	rgba::read(stream* in, int tag_type)
	{
		if (tag_type <= 22)
		{
			read_rgb(in);
		}
		else
		{
			read_rgba(in);
		}
	}

	void	rgba::read_rgba(stream* in)
	{
		read_rgb(in);
		m_a = in->read_u8();
	}

	void	rgba::read_rgb(stream* in)
	{
		m_r = in->read_u8();
		m_g = in->read_u8();
		m_b = in->read_u8();
		m_a = 0x0FF;
	}

	void	rgba::print()
	// For debugging.
	{
		log_parse("rgba: %d %d %d %d", m_r, m_g, m_b, m_a);
	}

	
	void	rgba::set_lerp(const rgba& a, const rgba& b, float f)
	{
		m_r = (uint8_t) frnd(flerp(a.m_r, b.m_r, f));
		m_g = (uint8_t) frnd(flerp(a.m_g, b.m_g, f));
		m_b = (uint8_t) frnd(flerp(a.m_b, b.m_b, f));
		m_a = (uint8_t) frnd(flerp(a.m_a, b.m_a, f));
	}


	//
	// rect
	//


	void	rect::read(stream* in)
	{
		in->align();
		int	nbits = in->read_uint(5);
		m_x_min = (float) in->read_sint(nbits);
		m_x_max = (float) in->read_sint(nbits);
		m_y_min = (float) in->read_sint(nbits);
		m_y_max = (float) in->read_sint(nbits);

//		IF_DEBUG(log_msg("rect::read() nbits = %d\n", nbits));
	}

	// Debug spew.
	void	rect::print() const
	{
		log_parse("xmin = %g, ymin = %g, xmax = %g, ymax = %g",
			TWIPS_TO_PIXELS(m_x_min),
			TWIPS_TO_PIXELS(m_y_min),
			TWIPS_TO_PIXELS(m_x_max),
			TWIPS_TO_PIXELS(m_y_max));
	}

	
	bool	rect::point_test(float x, float y) const
	// Return true if the specified point is inside this rect.
	{
		if (x < m_x_min
		    || x > m_x_max
		    || y < m_y_min
		    || y > m_y_max)
		{
			return false;
		}
		else
		{
			return true;
		}
	}


	void	rect::expand_to_point(float x, float y)
	// Expand this rectangle to enclose the given point.
	{
		m_x_min = fmin(m_x_min, x);
		m_y_min = fmin(m_y_min, y);
		m_x_max = fmax(m_x_max, x);
		m_y_max = fmax(m_y_max, y);
	}


	point	rect::get_corner(int i) const
	// Get one of the rect verts.
	{
		assert(i >= 0 && i < 4);
		return point(
			(i == 0 || i == 3) ? m_x_min : m_x_max,
			(i < 2) ? m_y_min : m_y_max);
	}


	void	rect::enclose_transformed_rect(const matrix& m, const rect& r)
	// Set ourself to bound a rectangle that has been transformed
	// by m.  This is an axial bound of an oriented (and/or
	// sheared, scaled, etc) box.
	{
		// Get the transformed bounding box.
		point	p0, p1, p2, p3;
		m.transform(&p0, r.get_corner(0));
		m.transform(&p1, r.get_corner(1));
		m.transform(&p2, r.get_corner(2));
		m.transform(&p3, r.get_corner(3));

		m_x_min = m_x_max = p0.m_x;
		m_y_min = m_y_max = p0.m_y;
		expand_to_point(p1.m_x, p1.m_y);
		expand_to_point(p2.m_x, p2.m_y);
		expand_to_point(p3.m_x, p3.m_y);
	}
	
	void  rect::expand_to_rect(const rect& r) 
  {
    point tmp;
    tmp = r.get_corner(0);  expand_to_point(tmp.m_x, tmp.m_y);    
    tmp = r.get_corner(1);  expand_to_point(tmp.m_x, tmp.m_y);    
    tmp = r.get_corner(2);  expand_to_point(tmp.m_x, tmp.m_y);    
    tmp = r.get_corner(3);  expand_to_point(tmp.m_x, tmp.m_y);    
  }	

	void	rect::expand_to_transformed_rect(const matrix& m, const rect& r)
	{
		// Get the transformed bounding box.
		point	p0, p1, p2, p3;
		m.transform(&p0, r.get_corner(0));
		m.transform(&p1, r.get_corner(1));
		m.transform(&p2, r.get_corner(2));
		m.transform(&p3, r.get_corner(3));

		expand_to_point(p0.m_x, p0.m_y);
		expand_to_point(p1.m_x, p1.m_y);
		expand_to_point(p2.m_x, p2.m_y);
		expand_to_point(p3.m_x, p3.m_y);
	}


	void	rect::set_lerp(const rect& a, const rect& b, float t)
	// Set this to the lerp of a and b.
	{
		m_x_min = flerp(a.m_x_min, b.m_x_min, t);
		m_y_min = flerp(a.m_y_min, b.m_y_min, t);
		m_x_max = flerp(a.m_x_max, b.m_x_max, t);
		m_y_max = flerp(a.m_y_max, b.m_y_max, t);
	}


}	// end namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
