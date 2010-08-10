// ASConversions.cpp	Conversions between AS and SWF types.
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

#include "ASConversions.h"

#include <boost/cstdint.hpp>
#include <cmath>

#include "as_object.h"
#include "log.h"
#include "Global_as.h"
#include "GnashNumeric.h"
#include "namedStrings.h"
#include "as_value.h"
#include "SWFMatrix.h"
#include "SWFCxForm.h"

namespace gnash {

SWFMatrix
toSWFMatrix(as_object& m)
{
    // This is case sensitive.
    if (m.getMember(NSV::PROP_MATRIX_TYPE).to_string() == "box") {
        
        const double x = pixelsToTwips(m.getMember(NSV::PROP_X).to_number());
        const double y = pixelsToTwips(m.getMember(NSV::PROP_Y).to_number());
        const double w = pixelsToTwips(m.getMember(NSV::PROP_W).to_number());
        const double h = pixelsToTwips(m.getMember(NSV::PROP_H).to_number()); 
        const double r = m.getMember(NSV::PROP_R).to_number();
        const double a = std::cos(r) * w * 2;
        const double b = std::sin(r) * h * 2;
        const double c = -std::sin(r) * w * 2;
        const double d = std::cos(r) * h * 2;

        return SWFMatrix(a, b, c, d, x + w / 2.0, y + h / 2.0);
        
    }

    // Convert input matrix to SWFMatrix.
    const boost::int32_t a = truncateWithFactor<65536>(
            m.getMember(NSV::PROP_A).to_number());
    const boost::int32_t b = truncateWithFactor<65536>(
            m.getMember(NSV::PROP_B).to_number());
    const boost::int32_t c = truncateWithFactor<65536>(
            m.getMember(NSV::PROP_C).to_number());
    const boost::int32_t d = truncateWithFactor<65536>(
            m.getMember(NSV::PROP_D).to_number());

    const boost::int32_t tx = pixelsToTwips(
            m.getMember(NSV::PROP_TX).to_number());
    const boost::int32_t ty = pixelsToTwips(
            m.getMember(NSV::PROP_TY).to_number());
    return SWFMatrix(a, b, c, d, tx, ty);

}

SWFCxForm
toCxForm(as_object& o)
{
    string_table& st = getStringTable(o);

    const as_value& am = o.getMember(st.find("alphaMultiplier"));
    const as_value& ao = o.getMember(st.find("alphaOffset"));
    const as_value& bm = o.getMember(st.find("blueMultiplier"));
    const as_value& bo = o.getMember(st.find("blueOffset"));
    const as_value& gm = o.getMember(st.find("greenMultiplier"));
    const as_value& go = o.getMember(st.find("greenOffset"));
    const as_value& rm = o.getMember(st.find("redMultiplier"));
    const as_value& ro = o.getMember(st.find("redOffset"));

    SWFCxForm cx;

    const size_t factor = 256;

    cx.aa = toInt(am) * factor;
    cx.ra = toInt(rm) * factor;
    cx.ba = toInt(bm) * factor;
    cx.ga = toInt(gm) * factor;
    cx.ab = toInt(ao);
    cx.rb = toInt(ro);
    cx.bb = toInt(bo);
    cx.gb = toInt(go);
    return cx;

}

} // namespace gnash 
