// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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
//

#include "cxform.h"
#include "RGBA.h" 
#include "SWFStream.h" // for reading from SWF
#include "log.h"
#include "GnashNumeric.h"
#include <iomanip>

namespace gnash {

using boost::uint8_t;
using boost::int16_t;

cxform::cxform()
// Initialize to identity transform.    
{
    ra = ga = ba = aa = 256;
    rb = gb = bb = ab = 0;
}

// Concatenate cxform c onto ours.  When
// transforming colors, c's transform is applied
// first, then ours.  
void    cxform::concatenate(const cxform& c)  
{
	// enbrace all the overflows intentionally.
    rb += (ra * c.rb >> 8);
    gb += (ga * c.gb >> 8);
    bb += (ba * c.bb >> 8);
    ab += (aa * c.ab >> 8);

    ra = ra * c.ra >> 8;
    ga = ga * c.ga >> 8;
    ba = ba * c.ba >> 8;
    aa = aa * c.aa >> 8;
}


rgba    cxform::transform(const rgba& in) const
// Apply our transform to the given color; return the result.
{
    rgba    result(in.m_r, in.m_g, in.m_b, in.m_a);
    
    transform(result.m_r, result.m_g, result.m_b, result.m_a);
    return result;
}

// transform the given color with our cxform.
void    cxform::transform(boost::uint8_t& r, boost::uint8_t& g, boost::uint8_t& b, boost::uint8_t& a) const
{
    // force conversion to int16 first, kind of optimization.
    int16_t rt = (int16_t)r;
    int16_t gt = (int16_t)g;
    int16_t bt = (int16_t)b;
    int16_t at = (int16_t)a;
    
    rt = (rt * ra >> 8) + rb;
    gt = (gt * ga >> 8) + gb;
    bt = (bt * ba >> 8) + bb;
    at = (at * aa >> 8) + ab;

    r = (uint8_t)(clamp<int16_t>(rt, 0, 255));
    g = (uint8_t)(clamp<int16_t>(gt, 0, 255));
    b = (uint8_t)(clamp<int16_t>(bt, 0, 255));
    a = (uint8_t)(clamp<int16_t>(at, 0, 255));
}

void    cxform::read_rgb(SWFStream& in)
{
    in.align();

    in.ensureBits(6);
    int  field =  in.read_uint(6);
    bool has_add  =  field & (1 << 5);
    bool has_mult =  field & (1 << 4);
    int  nbits = field & 0x0f;

    int reads = has_mult + has_add; // 0, 1 or 2
    assert(reads <= 2);
    if ( reads ) {
        in.ensureBits(nbits*reads*3);
    }
    else {
        return;
    }

    if (has_mult) {
        ra = in.read_sint(nbits);
        ga = in.read_sint(nbits);
        ba = in.read_sint(nbits);
        aa = 256;
    }
    else {
        ra = ga = ba = aa = 256; 
    }
    if (has_add) {
        rb = in.read_sint(nbits);
        gb = in.read_sint(nbits);
        bb = in.read_sint(nbits);
        ab = 0;
    }
    else {
        rb = gb = bb = ab = 0; 
    }
}

void    cxform::read_rgba(SWFStream& in)
{
    in.align();

    in.ensureBits(6);
    int  field =  in.read_uint(6);
    bool has_add  =  field & (1 << 5);
    bool has_mult =  field & (1 << 4);
    int  nbits = field & 0x0f;

    int reads = has_mult + has_add; // 0, 1 or 2
    assert(reads <= 2);
    if ( reads ) {
        in.ensureBits(nbits*reads*4);
    }
    else {
        return;
    }

    if (has_mult) {
        ra = in.read_sint(nbits);
        ga = in.read_sint(nbits);
        ba = in.read_sint(nbits);
        aa = in.read_sint(nbits);
    }
    else {
        ra = ga = ba = aa = 256; 
    }
    if (has_add) {
        rb = in.read_sint(nbits);
        gb = in.read_sint(nbits);
        bb = in.read_sint(nbits);
        ab = in.read_sint(nbits);
    }
    else {
        rb = gb = bb = ab = 0; 
    }
}

std::string
cxform::toString() const
{
    std::stringstream ss;
    ss << *this;
    return ss.str();
}

std::ostream&
operator<< (std::ostream& os, const cxform& cx) 
{
    // For integers up to 256
    const short fieldWidth = 3;

    os
    << std::endl
    << "| r: * " <<  std::setw(fieldWidth) << cx.ra 
    << " + " << std::setw(fieldWidth) << cx.rb << " |"
    << std::endl
    << "| g: * " << std::setw(fieldWidth) << cx.ga 
    << " + "  << std::setw(fieldWidth) << cx.gb << " |"
    << std::endl
    << "| b: * " << std::setw(fieldWidth) << cx.ba 
    << " + " << std::setw(fieldWidth) << cx.bb << " |"
    << std::endl
    << "| a: * " << std::setw(fieldWidth) << cx.aa 
    << " + " << std::setw(fieldWidth) << cx.ab << " |";  

    return os;
}

bool    cxform::is_identity() const
// Returns true when the cxform equals identity (no transform)
{      
    return 
        ra == 256 &&
        rb == 0   &&
        ga == 256 &&
        gb == 0   &&
        ba == 256 &&
        bb == 0   &&
        aa == 256 &&
        ab == 0;
}

bool    cxform::is_invisible() const
// Returns true when the cxform leads to alpha == 0
{
    return (255 * aa >> 8) + ab == 0;    
}


}   // end namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
