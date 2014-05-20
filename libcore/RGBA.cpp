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

#include "RGBA.h"

#include <sstream> 

#include "GnashNumeric.h"
#include "log.h"

namespace gnash {

rgba
lerp(const rgba& a, const rgba& b, float f)
{
    return rgba(
            frnd(lerp<float>(a.m_r, b.m_r, f)),
            frnd(lerp<float>(a.m_g, b.m_g, f)),
            frnd(lerp<float>(a.m_b, b.m_b, f)),
            frnd(lerp<float>(a.m_a, b.m_a, f))
            );
}

rgba
colorFromHexString(const std::string& color)
{
    std::stringstream ss(color);
    std::uint32_t hexnumber;
    
    if (!(ss >> std::hex >> hexnumber)) {
	    log_error(_("Failed to convert string to RGBA value! This is a "
			"Gnash bug"));
        return rgba();
    }

    rgba ret;
    ret.parseRGB(hexnumber);
    return ret;
}

std::ostream&
operator<<(std::ostream& os, const rgba& r)
{
    return os << "rgba: " << +r.m_r << "," << +r.m_g << "," << +r.m_b << ","
        << +r.m_a;
}

} // namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
