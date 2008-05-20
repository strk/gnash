// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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
	class stream;
}

namespace gnash {

/// Color Transformation Record
class DSOEXPORT cxform
{
public:

    friend bool operator== (const cxform&, const cxform&);
    friend std::ostream& operator<< (std::ostream& os, const cxform& cx);

    boost::int16_t ra; // RedMultTerm,   8.8 fixed point
    boost::int16_t rb; // RedAddTerm,   16.0 fixed point
    boost::int16_t ga; // GreenMultTerm  8.8 fixed point
    boost::int16_t gb; // GreenAddTerm  16.0 fixed point
    boost::int16_t ba; // BlueMultTerm   8.8 fixed point
    boost::int16_t bb; // BlueAddTerm   16.0 fixed point
    boost::int16_t aa; // AlphaMultTerm  8.8 fixed point
    boost::int16_t ab; // AlphaAddTerm  16.0 fixed point
    
    /// Initialize to the identity color transform (no transform)
    cxform();
    
    /// Concatenate c's transform onto ours. 
    //
    /// When transforming colors, c's transform is applied
    /// first, then ours.
    ///
    void concatenate(const cxform& c);
    
    /// Transform the given color, return the result.
    rgba transform(const rgba& in) const;
    
    /// Transform the given color.
    void transform(boost::uint8_t& r, boost::uint8_t& g, boost::uint8_t& b, boost::uint8_t& a) const;    

    void  store_to(boost::int16_t * dst) const
    {
        *dst++ = ra; *dst++ = rb; 
        *dst++ = ga; *dst++ = gb; 
        *dst++ = ba; *dst++ = bb; 
        *dst++ = aa; *dst++ = ab; 
    }
    
    cxform & load_from(float * src)
    {
    // enbrace the overflows intentionally.
        ra = (boost::int16_t)((*src++) * 2.56f);
        rb = (boost::int16_t)(*src++);
        ga = (boost::int16_t)((*src++) * 2.56f);
        gb = (boost::int16_t)(*src++);
        ba = (boost::int16_t)((*src++) * 2.56f);
        bb = (boost::int16_t)(*src++);
        aa = (boost::int16_t)((*src++) * 2.56f);
        ab = (boost::int16_t)(*src++);
        return *this;
    }
    
    /// Read RGB from the SWF input stream.
    void read_rgb(stream& in);

    // TODO: temp hack, should drop!
    void read_rgb(stream* in) { read_rgb(*in); }
    
    /// Read RGBA from the SWF input stream.
    void read_rgba(stream& in);

    // TODO: temp hack, should drop!
    void read_rgba(stream* in) { read_rgba(*in); }
        
    /// Debug log.
    void print() const;
    
    /// Returns true when the cxform equals identity (no transform)
    bool is_identity() const;
    
    /// Returns true when the cxform leads to alpha == 0
    bool is_invisible() const;
    
    std::string toString() const;
};


inline bool operator== (const cxform& a, const cxform& b)
{
	return	
		a.ra == b.ra &&
		a.rb == b.rb &&
		a.ga == b.ga &&
		a.gb == b.gb &&
		a.ba == b.ba &&
		a.bb == b.bb &&
		a.aa == b.aa &&
		a.ab == b.ab;
}

}	// namespace gnash

#endif // GNASHCXFORM_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
