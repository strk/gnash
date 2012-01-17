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


#ifndef GNASH_RECT_H
#define GNASH_RECT_H

#include <string>
#include <cassert> 
#include <ostream> 
#include <boost/cstdint.hpp>

#include "dsodefs.h"
#include "Range2d.h"

// Forward decl
namespace gnash {
    class SWFMatrix;
    namespace geometry {
        class Point2d;
    }
}

namespace gnash {

/// \brief
/// Rectangle class, see swf defined rectangle record.
///
class SWFRect
{

public:

    static const boost::int32_t rectNull = 0x80000000;
    static const boost::int32_t rectMax = 0x7fffffff;
    
    /// Ouput operator
    friend std::ostream& operator<< (std::ostream& os, const SWFRect& SWFRect);

    /// Construct a NULL rectangle
    SWFRect()
        :
       _xMin(rectNull),
       _yMin(rectNull),
       _xMax(rectNull),
       _yMax(rectNull)
    {}

    /// Construct a rectangle with given coordinates
    SWFRect(int xmin, int ymin, int xmax, int ymax)
        :
        _xMin(xmin),
        _yMin(ymin),
        _xMax(xmax),
        _yMax(ymax)
    {
    }

    /// returns true if this is a NULL rectangle
    bool is_null() const
    {
        return (_xMin == rectNull && _xMax == rectNull);
    }

    /// set the rectangle to the NULL value
    void set_null()
    {
        _xMin = _yMin = _xMax = _yMax = rectNull;
    }

    /// TODO: deprecate this 'world' concept.
    bool is_world() const
    {
        return _xMin == (- rectMax >> 9) 
            && _yMin == (- rectMax >> 9) 
            && _xMax == (rectMax >> 9)
            && _yMax == (rectMax >> 9);
    }

    /// set the rectangle to the WORLD value
    void set_world()
    {
        _xMin = _yMin = - rectMax >> 9;
        _xMax = _yMax = rectMax >> 9;
    }

    /// Return width of this rectangle in TWIPS
    boost::int32_t width() const
    {
        return _xMax - _xMin;
    }

    /// Return height of this rectangle in TWIPS
    boost::int32_t height() const
    {
        return _yMax - _yMin;
    }

    /// Get the x coordinate of the left-up corner.
    boost::int32_t get_x_min() const
    {
        assert(!is_null());
        return _xMin;
    }

    /// Get the x coordinate of the right-down corner.
    boost::int32_t get_x_max() const
    {
        assert(!is_null());
        return _xMax;
    }

    /// Get the y coordinate of the left-up corner.
    boost::int32_t get_y_min() const
    {
        assert(!is_null());
        return _yMin;
    }

    /// Get the y coordinate of the right-down corner.
    boost::int32_t get_y_max() const
    {
        assert(!is_null());
        return _yMax;
    }
    
    /// Return true if the given point is inside this SWFRect.
    bool point_test(boost::int32_t x, boost::int32_t y) const
    {
        if (is_null()) return false;
        
        if (x < _xMin || x > _xMax || y < _yMin || y > _yMax) {
            return false;
        } 
        return true;
    }

    /// Set ourself to bound the given point
    void set_to_point(boost::int32_t x, boost::int32_t y)
    {
        _xMin = _xMax = x;
        _yMin = _yMax = y;
    }

    
    void set_to_rect(boost::int32_t x1, boost::int32_t y1, boost::int32_t x2,
            boost::int32_t y2)
    {
        _xMin = x1;
        _yMin = y1;
        _xMax = x2;
        _yMax = y2;
    }
    
    /// Expand this rectangle to enclose the given point.
    void expand_to_point(boost::int32_t x, boost::int32_t y)
    {
        if (is_null()) {
            set_to_point(x, y);
        } else {
            expand_to(x, y);
        }
    }

    /// Set ourself to bound a rectangle that has been transformed by m.  
    /// This is an axial bound of an oriented (and/or
    /// sheared, scaled, etc) box.
    void enclose_transformed_rect(const SWFMatrix& m, const SWFRect& r);
    
    /// Expand this rectangle to enclose the given circle.
    void expand_to_circle(boost::int32_t x, boost::int32_t y,
            boost::int32_t radius)
    {
        // I know it's easy to make code work for minus radius.
        // would do that untill I see the requirement for a SWF RECTANGLE.
        assert(radius >= 0); 
        if (is_null()) {
            _xMin = x - radius;
            _yMin = y - radius;
            _xMax = x + radius;
            _yMax = y + radius;
        } else {
            _xMin = std::min(_xMin, x - radius);
            _yMin = std::min(_yMin, y - radius);
            _xMax = std::max(_xMax, x + radius);
            _yMax = std::max(_yMax, y + radius);
        }
    }
      
    /// Same as enclose_transformed_rect but expanding the current SWFRect
    /// instead of replacing it.
    DSOEXPORT void expand_to_transformed_rect(const SWFMatrix& m,
            const SWFRect& r);
    
    /// Makes union of the given and the current SWFRect
    DSOEXPORT void expand_to_rect(const SWFRect& r);
    
    void set_lerp(const SWFRect& a, const SWFRect& b, float t);

    /// \brief
    /// Make sure that the given point falls in this rectangle, 
    /// modifying it's coordinates if needed.
    void clamp(geometry::Point2d& p) const;

    /// Construct and return a Range2d object.
    // TODO: deprecate this.
    geometry::Range2d<boost::int32_t> getRange() const
    {
        if (is_null())
        {
           // Range2d has a differnt idea about what is a null SWFRect.
           return geometry::Range2d<boost::int32_t>(geometry::nullRange); 
        }
        else if( is_world() ) 
        {
            return geometry::Range2d<boost::int32_t>(geometry::worldRange);
        }
        else
        {
            return geometry::Range2d<boost::int32_t>(_xMin, _yMin,
                    _xMax, _yMax);
        }
    }

    /// Return a string representation for this rectangle
    std::string toString() const;

private:

    // make ourself to enclose the given point.
    void expand_to(boost::int32_t x, boost::int32_t y)
    {
        _xMin = std::min(_xMin, x);
        _yMin = std::min(_yMin, y);
        _xMax = std::max(_xMax, x);
        _yMax = std::max(_yMax, y);
    }

    boost::int32_t _xMin; // TWIPS
    boost::int32_t _yMin; // TWIPS
    boost::int32_t _xMax; // TWIPS
    boost::int32_t _yMax; // TWIPS
};


inline std::ostream&
operator<<(std::ostream& os, const SWFRect& r)
{
    if (!r.is_null()) {
        os << "RECT(" 
           << r.get_x_min() << "," 
           << r.get_y_min() << "," 
           << r.get_x_max() << "," 
           << r.get_y_max() << ")";
    }
    else {
        os << "NULL RECT!";
    }

    return os;
}

}   // namespace gnash

#endif // GNASH_RECT_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
