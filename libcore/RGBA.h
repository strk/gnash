// types.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Some basic types.

#ifndef GNASH_TYPES_H
#define GNASH_TYPES_H

#include "SWF.h"

#include <string>
#include <boost/cstdint.hpp> // for boost::?int??_t 

namespace gnash {
	class SWFStream;	// forward declaration

	/// RGBA record
	class rgba
	{
	public:

		friend std::ostream& operator<< (std::ostream& os, const rgba& r);

		boost::uint8_t	m_r, m_g, m_b, m_a;

		/// Default RGBA value is FF.FF.FF.FF
		rgba() : m_r(255), m_g(255), m_b(255), m_a(255) {}

		/// Construct an RGBA with the provided values
		//
		/// @param r Red
		/// @param g Green
		/// @param b Blue
		/// @param a Alpha (transparency)
		///
		rgba(boost::uint8_t r, boost::uint8_t g, 
                boost::uint8_t b, boost::uint8_t a)
			:
			m_r(r), m_g(g), m_b(b), m_a(a)
		{
		}

		/// \brief
		/// Parse a 32-bit unsigned integer
		/// as three packed R,G,B bytes.
		//
		/// Alpha will be untouched.
		/// Blue is the least significant byte.
		///
		/// This function is meant to be used to
		/// parse ActionScript colors in numeric format.
		///
		void parseRGB(boost::uint32_t rgbCol)
		{
			m_r = static_cast<boost::uint8_t>(rgbCol>>16);
			m_g = static_cast<boost::uint8_t>(rgbCol>>8);
			m_b = static_cast<boost::uint8_t>(rgbCol);
		}

		/// \brief
		/// Return a 32-bit unsigned integer
		/// as four packed R,G,B bytes.
		//
		/// Blue is the least significant byte.
		///
		/// This function is meant to be used to
		/// output ActionScript colors in numeric format.
		///
		boost::uint32_t toRGB() const
		{
			return (m_r<<16) + (m_g<<8) + m_b;
		}

        boost::uint32_t toRGBA() const
        {
            return toRGB() + (m_a << 24);
        }

		/// Initialize from input stream.
		//
		///
		/// @param in
		///	The input (SWF) stream
		///
		/// @param t 
		///	I don't know by which logic but
		///	a value <= 22 makes it read RGB
		///	and value > 22 makes it read RGBA
		///
		/// Throw a ParserException if there's no enough bytes in the
		/// currently opened tag for reading. See stream::ensureBytes()
		///
		void read(SWFStream& in, SWF::TagType t);

		/// Initialize from input stream (reads RGBA)
		//
		/// Throw a ParserException if there's no enough bytes in the
		/// currently opened tag for reading. See stream::ensureBytes()
		///
		void read_rgba(SWFStream& in);

		/// Initialize from intput stream (reads RGB)
		void read_rgb(SWFStream& in);

		/// Set r,g,b.a values
		void set(boost::uint8_t r, boost::uint8_t g,
                boost::uint8_t b, boost::uint8_t a)
		{
			m_r = r;
			m_g = g;
			m_b = b;
			m_a = a;
		}

		void set_lerp(const rgba& a, const rgba& b, float f);

		/// Debug log.
		void print() const;

		/// Debug print.
		std::string toString() const;

		// neater string output (example: "0,0,0,255")
		std::string toShortString() const;

		bool operator== (const rgba& o) const
		{
			return m_r == o.m_r && 
				m_g == o.m_g && 
				m_b == o.m_b && 
				m_a == o.m_a;
		}

		bool operator!= (const rgba& o) const
		{
			return ! ( *this == o );
		}
	};

	std::ostream& operator<< (std::ostream& os, const rgba& r);


}	// end namespace gnash


#endif // GNASH_TYPES_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
