// LineStyle.h    -- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// line style types.


#ifndef GNASH_LINESTYLE_H
#define GNASH_LINESTYLE_H

#include "RGBA.h"
#include "SWF.h"

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
class line_style 
{
public:

    /// Construct a default LineStyle.
    line_style();

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
    line_style(boost::uint16_t width, const rgba& color,
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
        m_color(color),
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
            const RunResources& r, line_style *pOther);

    /// Return thickness of the line, in TWIPS
    boost::uint16_t getThickness() const {
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
    /// @param ls1      First line_style to interpolate.
    ///
    /// @param ls2      Second line_style to interpolate.
    ///
    /// @ratio          The interpolation factor (0..1).
    ///                 When 0, this will be equal to ls1, when 1
    ///                 this will be equal to ls2.
    void set_lerp(const line_style& ls1, const line_style& ls2, float ratio);
    
private:
    
    /// Width in twips.
    boost::uint16_t m_width;

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

} // namespace gnash


#endif // GNASH_STYLES_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
