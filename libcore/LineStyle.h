// LineStyle.h   Line style types.
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
// Based on public domain work by Thatcher Ulrich <tu@tulrich.com> 2003

#ifndef GNASH_LINESTYLE_H
#define GNASH_LINESTYLE_H

#include "RGBA.h"
#include "SWF.h"
#include <utility>

namespace gnash {
    class SWFStream;
    class movie_definition;
    class RunResources;
}

namespace gnash {

enum CapStyle {
    CAP_ROUND = 0,
    CAP_NONE = 1,
    CAP_SQUARE = 2
};

enum JoinStyle {
    JOIN_ROUND = 0,
    JOIN_BEVEL = 1,
    JOIN_MITER = 2
};
  
/// For the outside of outline shapes, or just bare lines.
class LineStyle 
{
public:

    /// Construct a default LineStyle.
    LineStyle();

    /// Construct a line style with explicit values
    ///
    /// @param width        Thickness of line in twips. 
    ///                     Zero for hair line
    ///
    /// @param color        Line color
    /// @param scaleThicknessVertically
    /// @param scaleThicknessHorizontally
    /// @param noClose
    /// @param startCapStyle
    /// @param endCapStyle
    /// @param joinStyle
    /// @param miterLimitFactor
    LineStyle(std::uint16_t width, rgba color,
            bool scaleThicknessVertically=true,
            bool scaleThicknessHorizontally=true,
            bool pixelHinting=false,
            bool noClose=false,
            CapStyle startCapStyle=CAP_ROUND,
            CapStyle endCapStyle=CAP_ROUND,
            JoinStyle joinStyle=JOIN_ROUND,
            float miterLimitFactor=1.0f
        )
        :
        m_width(width),
        m_color(std::move(color)),
        _scaleVertically(scaleThicknessVertically),
        _scaleHorizontally(scaleThicknessHorizontally),
        _pixelHinting(pixelHinting),
        _noClose(noClose),
        _startCapStyle(startCapStyle),
        _endCapStyle(endCapStyle),
        _joinStyle(joinStyle),
        _miterLimitFactor(miterLimitFactor)
    {
    }

    /// Read the line style from an SWF stream
    //
    /// Stream is assumed to be positioned at 
    /// the right place.
    ///
    /// Throw a ParserException if there's no enough bytes in the
    /// currently opened tag for reading. See stream::ensureBytes()
    void read(SWFStream& in, SWF::TagType t, movie_definition& md,
            const RunResources& r);
    
    /// Read two lines styles from the SWF stream
    /// at the same time -- this is used in morphing.
    void read_morph(SWFStream& in, SWF::TagType t, movie_definition& md,
            const RunResources& r, LineStyle *pOther);

    /// Return thickness of the line, in TWIPS
    std::uint16_t getThickness() const {
        return m_width;
    }

    /// Return true if line thickness should be scaled vertically
    bool scaleThicknessVertically() const {
        return _scaleVertically;
    }

    /// Return true if line thickness should be scaled horizontally
    bool scaleThicknessHorizontally() const {
        return _scaleHorizontally;
    }
    
    /// Return the start cap style
    CapStyle startCapStyle() const {
        return _startCapStyle;
    }
    
    /// Return the end cap style
    CapStyle endCapStyle() const {
        return _endCapStyle;
    }
    
    /// Return the join style
    JoinStyle joinStyle() const {
        return _joinStyle;
    }
    
    /// Return the miter limit factor
    float miterLimitFactor() const {
        return _miterLimitFactor;
    }
  
    /// Return true if stroke should not be closed if the stroke's last point
    /// matches the first point. Caps should be applied instead of a join
    bool noClose() const {
        return _noClose;
    }

    /// Return true if pixel hinting should be activated
    bool doPixelHinting() const {
        return _pixelHinting;
    }

    /// Return line color and alpha
    const rgba& get_color() const { return m_color; }

    /// Set this style to the interpolation of the given one
    //
    /// @param ls1      First LineStyle to interpolate.
    /// @param ls2      Second LineStyle to interpolate.
    /// @ratio          The interpolation factor (0..1).
    ///                 When 0, this will be equal to ls1, when 1
    ///                 this will be equal to ls2.
    void set_lerp(const LineStyle& ls1, const LineStyle& ls2, float ratio);
    
private:
    
    /// Width in twips.
    std::uint16_t m_width;

    rgba m_color;

    bool _scaleVertically;

    bool _scaleHorizontally;

    bool _pixelHinting;

    bool _noClose;

    CapStyle _startCapStyle;

    CapStyle _endCapStyle;

    JoinStyle _joinStyle;

    float _miterLimitFactor;
};
    
inline void
setLerp(LineStyle& s, const LineStyle& ls1, const LineStyle& ls2, double ratio) 
{
    s.set_lerp(ls1, ls2, ratio);
}

} // namespace gnash

#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
