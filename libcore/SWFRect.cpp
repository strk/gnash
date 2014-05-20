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

#include "SWFRect.h"

#include <sstream> 

#include "SWFMatrix.h"
#include "Point2d.h"
#include "GnashNumeric.h" // for flerp, clamp...


namespace gnash {

const std::int32_t SWFRect::rectNull;
const std::int32_t SWFRect::rectMax;


// Set ourself to bound a rectangle that has been transformed by m.  
void
SWFRect::enclose_transformed_rect(const SWFMatrix& m, const SWFRect& r)
{   
    std::int32_t  x1 = r.get_x_min();
    std::int32_t  y1 = r.get_y_min();
    std::int32_t  x2 = r.get_x_max();
    std::int32_t  y2 = r.get_y_max();

    point  p0(x1, y1);
    point  p1(x2, y1);
    point  p2(x2, y2);
    point  p3(x1, y2);
    
    m.transform(p0);
    m.transform(p1);
    m.transform(p2);
    m.transform(p3);

    set_to_point(p0.x, p0.y);
    expand_to(p1.x, p1.y);
    expand_to(p2.x, p2.y);
    expand_to(p3.x, p3.y);
}

void
SWFRect::expand_to_rect(const SWFRect& r) 
// Expand ourself to enclose the given SWFRect.
{    
    if( r.is_null() ) {
        return;
    }
    
    if( is_null() ) {
        *this = r;
    }else {
        _xMin = std::min(_xMin, r.get_x_min());
        _yMin = std::min(_yMin, r.get_y_min());
        _xMax = std::max(_xMax, r.get_x_max());
        _yMax = std::max(_yMax, r.get_y_max());
    }
}   

void
SWFRect::expand_to_transformed_rect(const SWFMatrix& m, const SWFRect& r)
{   
    if (r.is_null()) {
         return;
    }

    const std::int32_t x1 = r.get_x_min();
    const std::int32_t y1 = r.get_y_min();
    const std::int32_t x2 = r.get_x_max();
    const std::int32_t y2 = r.get_y_max();

    point p0(x1, y1);
    point p1(x2, y1);
    point p2(x2, y2);
    point p3(x1, y2);
    
    m.transform(p0);
    m.transform(p1);
    m.transform(p2);
    m.transform(p3);

    if (is_null()) {
        set_to_point(p0.x, p0.y);   
    }
    else {
        expand_to(p0.x, p0.y);
    }
    expand_to(p1.x, p1.y);
    expand_to(p2.x, p2.y);
    expand_to(p3.x, p3.y);
}

void
SWFRect::set_lerp(const SWFRect& a, const SWFRect& b, float t)
// Set this to the lerp of a and b.
{
    assert( !a.is_null() );
    assert( !b.is_null() );
    
    _xMin = lerp<float>(a.get_x_min(), b.get_x_min(), t);
    _yMin = lerp<float>(a.get_y_min(), b.get_y_min(), t);
    _xMax = lerp<float>(a.get_x_max(), b.get_x_max(), t);
    _yMax = lerp<float>(a.get_y_max(), b.get_y_max(), t);
}

void
SWFRect::clamp(point& p) const
{
    assert( !is_null() );
    p.x = gnash::clamp<std::int32_t>(p.x, _xMin, _xMax);
    p.y = gnash::clamp<std::int32_t>(p.y, _yMin, _yMax);
}

std::string
SWFRect::toString() const
{
    std::stringstream ss;
    ss << *this;
    return ss.str();
}

}   // end namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
