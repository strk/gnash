// 
//   Copyright (C) 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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

#ifndef GNASHCXFORM_H
#define GNASHCXFORM_H

#include "dsodefs.h" // for DSOEXPORT

#include <string>
#include <iosfwd>
#include <boost/cstdint.hpp> // for boost::?int??_t 

namespace gnash {
	class rgba;
	class SWFStream;
}

namespace gnash {

/// Color Transformation Record
class DSOEXPORT cxform
{
public:

    boost::int16_t ra; // RedMultTerm,   8.8 fixed point
    boost::int16_t rb; // RedAddTerm,    16 bit integer(no fraction)
    boost::int16_t ga; // GreenMultTerm  8.8 fixed point
    boost::int16_t gb; // GreenAddTerm   16 bit integer(no fraction)
    boost::int16_t ba; // BlueMultTerm   8.8 fixed point
    boost::int16_t bb; // BlueAddTerm    16 bit integer(no fraction)
    boost::int16_t aa; // AlphaMultTerm  8.8 fixed point
    boost::int16_t ab; // AlphaAddTerm   16 bit integer(no fraction)
    
    /// Initialize to the identity color transform (no transform)
    cxform();
    
    /// Concatenate cxform c onto ours. 
    //
    /// When transforming colors, c's transform is applied
    /// first, then ours.
    ///
    void concatenate(const cxform& c);
    
    /// Transform the given color, return the result.
    rgba transform(const rgba& in) const;
    
    /// Transform the given color.
    void transform(boost::uint8_t& r, boost::uint8_t& g, boost::uint8_t& b, boost::uint8_t& a) const;    

	/// Store the cxform record to an external array.
    void  store_to(boost::int16_t * dst) const
    {
        *dst++ = ra; *dst++ = rb; 
        *dst++ = ga; *dst++ = gb; 
        *dst++ = ba; *dst++ = bb; 
        *dst++ = aa; *dst++ = ab; 
    }

	/// Load an cxform record from an external array.
    cxform & load_from(float * src)
    {
    	// enbrace the overflows intentionally.
        ra = static_cast<boost::int16_t>((*src++) * 2.56f);
        rb = static_cast<boost::int16_t>(*src++);
        ga = static_cast<boost::int16_t>((*src++) * 2.56f);
        gb = static_cast<boost::int16_t>(*src++);
        ba = static_cast<boost::int16_t>((*src++) * 2.56f);
        bb = static_cast<boost::int16_t>(*src++);
        aa = static_cast<boost::int16_t>((*src++) * 2.56f);
        ab = static_cast<boost::int16_t>(*src++);
        return *this;
    }
    
    /// Returns true when the cxform equals identity (no transform).
    bool is_identity() const;
    
    /// Returns true when the cxform leads to alpha == 0
    //
    /// Not the _alpha property, but the visible alpha related to dislpay. 
    /// The two might be completely diffrent. eg. mc._alpha ranges in [-32768, 32767]
    /// But the alpha on screen ranges in [0, 255]
    bool is_invisible() const;
    
    /// Read RGB from the SWF input stream.
    void read_rgb(SWFStream& in);

    /// Read RGBA from the SWF input stream.
    void read_rgba(SWFStream& in);

    friend bool operator== (const cxform&, const cxform&);
    friend bool operator!= (const cxform&, const cxform&);
	
    friend std::ostream& operator<< (std::ostream& os, const cxform& cx);

	std::string toString() const;

};


inline bool
operator== (const cxform& a, const cxform& b)
{
	return	a.ra == b.ra &&
            a.rb == b.rb &&
            a.ga == b.ga &&
            a.gb == b.gb &&
            a.ba == b.ba &&
            a.bb == b.bb &&
            a.aa == b.aa &&
            a.ab == b.ab;
}

inline bool
operator!=(const cxform& a, const cxform& b)
{
    return !(a == b);
}

}	// namespace gnash

#endif // GNASHCXFORM_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
