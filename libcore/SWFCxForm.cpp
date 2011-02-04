// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "SWFCxForm.h"

#include <iomanip>

#include "RGBA.h" 
#include "log.h"
#include "GnashNumeric.h"

namespace gnash {

// Concatenate SWFCxForm c onto ours.  When
// transforming colors, c's transform is applied
// first, then ours.  
void
SWFCxForm::concatenate(const SWFCxForm& c)  
{
	// embrace all the overflows intentionally.
    rb += (ra * c.rb >> 8);
    gb += (ga * c.gb >> 8);
    bb += (ba * c.bb >> 8);
    ab += (aa * c.ab >> 8);

    ra = ra * c.ra >> 8;
    ga = ga * c.ga >> 8;
    ba = ba * c.ba >> 8;
    aa = aa * c.aa >> 8;
}


rgba
SWFCxForm::transform(const rgba& in) const
{
    rgba result(in.m_r, in.m_g, in.m_b, in.m_a);
    
    transform(result.m_r, result.m_g, result.m_b, result.m_a);
    return result;
}

// transform the given color with our SWFCxForm.
void
SWFCxForm::transform(boost::uint8_t& r, boost::uint8_t& g, boost::uint8_t& b,
        boost::uint8_t& a) const
{
    // force conversion to int16 first, kind of optimization.
    boost::int16_t rt = r;
    boost::int16_t gt = g;
    boost::int16_t bt = b;
    boost::int16_t at = a;
    
    rt = (rt * ra >> 8) + rb;
    gt = (gt * ga >> 8) + gb;
    bt = (bt * ba >> 8) + bb;
    at = (at * aa >> 8) + ab;

    r = clamp<boost::int16_t>(rt, 0, 255);
    g = clamp<boost::int16_t>(gt, 0, 255);
    b = clamp<boost::int16_t>(bt, 0, 255);
    a = clamp<boost::int16_t>(at, 0, 255);
}

std::ostream&
operator<<(std::ostream& os, const SWFCxForm& cx) 
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


} // namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
