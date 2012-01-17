// RGBA.h: RGBA color handling.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#ifndef GNASH_RGBA_H
#define GNASH_RGBA_H

#include <string>
#include <boost/cstdint.hpp> 

#include "dsodefs.h"
#include "SWF.h"

namespace gnash {

/// A basic RGBA type
//
/// This both represents a SWF RGBA record and is a basic Gnash type for
/// color values.
class DSOEXPORT rgba
{
public:

    /// Construct default RGBA value.
    //
    /// Default value is 0xffffffff (solid white).
    rgba()
        :
        m_r(255),
        m_g(255),
        m_b(255),
        m_a(255)
    {}

    /// Construct an RGBA with the provided values
    //
    /// @param r Red
    /// @param g Green
    /// @param b Blue
    /// @param a Alpha (transparency)
    rgba(boost::uint8_t r, boost::uint8_t g, boost::uint8_t b, 
            boost::uint8_t a)
        :
        m_r(r),
        m_g(g),
        m_b(b),
        m_a(a)
    {
    }

    /// Parse a 32-bit unsigned integer as three packed R,G,B bytes.
    //
    /// Alpha will be untouched.
    /// Blue is the least significant byte.
    ///
    /// This function is meant to be used to
    /// parse ActionScript colors in numeric format.
    void parseRGB(boost::uint32_t rgbCol) {
        m_r = static_cast<boost::uint8_t>(rgbCol >> 16);
        m_g = static_cast<boost::uint8_t>(rgbCol >> 8);
        m_b = static_cast<boost::uint8_t>(rgbCol);
    }

    /// Return a 32-bit unsigned integer as four packed R,G,B bytes.
    //
    /// Blue is the least significant byte. The most significant (alpha)
    /// byte is unused.
    ///
    /// This function is meant to be used to output ActionScript colors
    /// in numeric format.
    boost::uint32_t toRGB() const {
        return (m_r << 16) + (m_g << 8) + m_b;
    }

    /// Return a 32-bit unsigned integer as four packed A,R,G,B bytes.
    //
    /// Blue is the least significant byte.
    ///
    /// This function is meant to be used to output ActionScript colors
    /// in numeric format.
    boost::uint32_t toRGBA() const {
        return toRGB() + (m_a << 24);
    }

    friend std::ostream& operator<<(std::ostream& os, const rgba& r);

    bool operator==(const rgba& o) const {
        return m_r == o.m_r && 
               m_g == o.m_g && 
               m_b == o.m_b && 
               m_a == o.m_a;
    }

    bool operator!=(const rgba& o) const {
        return !(*this == o);
    }

    boost::uint8_t m_r, m_g, m_b, m_a;

};

std::ostream& operator<<(std::ostream& os, const rgba& r);

/// Create an RGBA value from a hex string (e.g. FF0000)
//
/// @param color    A hex string in 'rrbbgg' format. This must contain only
///                 a valid hexadecimal number. It is the caller's
///                 responsibility to check it.
rgba colorFromHexString(const std::string& color);

/// Used for morphing.
rgba lerp(const rgba& a, const rgba& b, float f);

} // namespace gnash

#endif 


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
