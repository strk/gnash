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
#include <sstream> // for ::print and ::toString


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

	float point::squareDistance(const point& other) const
	{
		float hside = other.m_x - m_x;
		float vside = other.m_y - m_y;

		return hside*hside + vside*vside;
	}

	float point::distance(const point& other) const
	{
		float sd = squareDistance(other);
		if ( ! sd ) return sd;
		else return sqrt(squareDistance(other));
	}


	//
	// rgba
	//

	void	rgba::read(stream* in, int tag_type)
	{
		switch (tag_type)
		{
			case SWF::DEFINESHAPE:
			case SWF::DEFINESHAPE2:
				read_rgb(in);
				break;
			default:
			case SWF::DEFINESHAPE3:
				read_rgba(in);
				break;
		}
	}

	void	rgba::read_rgba(stream* in)
	{
		read_rgb(in);
    		in->ensureBytes(1);
		m_a = in->read_u8();
	}

	void	rgba::read_rgb(stream* in)
	{
    		in->ensureBytes(3);
		m_r = in->read_u8();
		m_g = in->read_u8();
		m_b = in->read_u8();
		m_a = 0x0FF;
	}

	void	rgba::print() const
	// For debugging.
	{
		log_parse("rgba: %d %d %d %d", m_r, m_g, m_b, m_a);
	}

	std::string rgba::toString() const
	// For debugging.
	{
		std::stringstream ss;
		ss << *this;
		return ss.str();
	}

	std::string rgba::toShortString() const
	// For debugging.
	{
		std::stringstream ss;
		ss << (unsigned)m_r << ","
			<< (unsigned)m_g << ","
			<< (unsigned)m_b << ","
			<< (unsigned)m_a;
		return ss.str();
	}
	
	void	rgba::set_lerp(const rgba& a, const rgba& b, float f)
	{
		m_r = (uint8_t) frnd(flerp(a.m_r, b.m_r, f));
		m_g = (uint8_t) frnd(flerp(a.m_g, b.m_g, f));
		m_b = (uint8_t) frnd(flerp(a.m_b, b.m_b, f));
		m_a = (uint8_t) frnd(flerp(a.m_a, b.m_a, f));
	}

std::ostream&
operator<< (std::ostream& os, const rgba& r)
{
	return os << "rgba: "
		<< (unsigned)r.m_r << ", "
		<< (unsigned)r.m_g << ", "
		<< (unsigned)r.m_b << ", "
		<< (unsigned)r.m_a;
}

}	// end namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
