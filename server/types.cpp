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
		rgba	result(in.m_r, in.m_g, in.m_b, in.m_a);
		
		transform(result.m_r, result.m_g, result.m_b, result.m_a);

		return result;
	}

  void	cxform::transform(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) const
  // Faster transform() method for loops (avoids creation of rgba object)
  {
		r = (uint8_t) fclamp(r * m_[0][0] + m_[0][1], 0, 255);
		g = (uint8_t) fclamp(g * m_[1][0] + m_[1][1], 0, 255);
		b = (uint8_t) fclamp(b * m_[2][0] + m_[2][1], 0, 255);
		a = (uint8_t) fclamp(a * m_[3][0] + m_[3][1], 0, 255);
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
	
	bool	cxform::is_identity() const
	// Returns true when the cxform equals identity (no transform)
	{	   
	  for (int a=0; a<4; a++)
	   for (int b=0; b<2; b++)
	    if (m_[a][b] != identity.m_[a][b])
	     return false;
	  
	  return true;
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


}	// end namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
