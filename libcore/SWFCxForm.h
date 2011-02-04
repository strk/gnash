// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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
#include <boost/cstdint.hpp> 

namespace gnash {
	class rgba;
}

namespace gnash {

/// Color transformation record
class DSOEXPORT SWFCxForm
{
public:

    /// Construct an identity CxForm
    SWFCxForm()
        :
        ra(256),
        ga(256),
        ba(256),
        aa(256),
        rb(0),
        gb(0),
        bb(0),
        ab(0)
    {
    }

    boost::int16_t ra; // RedMultTerm,   8.8 fixed point
    boost::int16_t ga; // GreenMultTerm  8.8 fixed point
    boost::int16_t ba; // BlueMultTerm   8.8 fixed point
    boost::int16_t aa; // AlphaMultTerm  8.8 fixed point
    boost::int16_t rb; // RedAddTerm,    16 bit integer(no fraction)
    boost::int16_t gb; // GreenAddTerm   16 bit integer(no fraction)
    boost::int16_t bb; // BlueAddTerm    16 bit integer(no fraction)
    boost::int16_t ab; // AlphaAddTerm   16 bit integer(no fraction)
    
    /// Concatenate SWFCxForm c onto ours. 
    //
    /// When transforming colors, c's transform is applied
    /// first, then ours.
    ///
    void concatenate(const SWFCxForm& c);
    
    /// Transform the given color, return the result.
    rgba transform(const rgba& in) const;
    
    /// Transform the given color.
    void transform(boost::uint8_t& r, boost::uint8_t& g, boost::uint8_t& b,
            boost::uint8_t& a) const;    
    
};

inline bool
operator==(const SWFCxForm& a, const SWFCxForm& b)
{
	return a.ra == b.ra &&
           a.rb == b.rb &&
           a.ga == b.ga &&
           a.gb == b.gb &&
           a.ba == b.ba &&
           a.bb == b.bb &&
           a.aa == b.aa &&
           a.ab == b.ab;
}

inline bool
operator!=(const SWFCxForm& a, const SWFCxForm& b)
{
    return !(a == b);
}

/// Returns true when the SWFCxForm leads to alpha == 0
//
/// Not the _alpha property, but the visible alpha related to dislpay. 
/// The two might be completely different. eg. mc._alpha ranges in
/// [-32768, 32767], but the alpha on screen ranges in [0, 255]
inline bool
invisible(const SWFCxForm& cx)
{
    return (255 * cx.aa >> 8) + cx.ab == 0;    
}

std::ostream& operator<<(std::ostream& os, const SWFCxForm& cx);

} // namespace gnash

#endif 


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
