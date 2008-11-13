// types.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Some basic types for gnash.
#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

#include <boost/thread.hpp>

#include "RGBA.h"

#include "utility.h"
#include "log.h"
#include "SWFStream.h"
#include <sstream> // for ::print and ::toString

namespace gnash {

	//
	// rgba
	//

	/// Can throw ParserException on premature end of input stream
	void	rgba::read(SWFStream& in, int tag_type)
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

	/// Can throw ParserException on premature end of input stream
	void	rgba::read_rgba(SWFStream& in)
	{
		read_rgb(in);
    		in.ensureBytes(1);
		m_a = in.read_u8();
	}

	/// Can throw ParserException on premature end of input stream
	void	rgba::read_rgb(SWFStream& in)
	{
    		in.ensureBytes(3);
		m_r = in.read_u8();
		m_g = in.read_u8();
		m_b = in.read_u8();
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
	    using utility::frnd;
	    using utility::flerp;
		m_r = static_cast<boost::uint8_t>(frnd(flerp(a.m_r, b.m_r, f)));
		m_g = static_cast<boost::uint8_t>(frnd(flerp(a.m_g, b.m_g, f)));
		m_b = static_cast<boost::uint8_t>(frnd(flerp(a.m_b, b.m_b, f)));
		m_a = static_cast<boost::uint8_t>(frnd(flerp(a.m_a, b.m_a, f)));
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
