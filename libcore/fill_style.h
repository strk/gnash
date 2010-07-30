// 
//   Copyright (C) 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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

// Based on work of Thatcher Ulrich <tu@tulrich.com> 2003


#ifndef GNASH_FILL_STYLE_H
#define GNASH_FILL_STYLE_H

#include "SWFMatrix.h"
#include "BitmapInfo.h"
#include "SWF.h"
#include "RGBA.h" // for rgba type

#include <boost/variant.hpp>
#include <vector> 
#include <iosfwd> 

#include <boost/intrusive_ptr.hpp>

namespace gnash {

class SWFStream;
class movie_definition;
class Renderer;
class RunResources;

class gradient_record
{
public:
    gradient_record()
        :
        m_ratio(0),
        m_color()
    {}

    gradient_record(boost::uint8_t ratio, const rgba& color)
        :
        m_ratio(ratio),
        m_color(color)
    {}

    void read(SWFStream& in, SWF::TagType tag);
    
    //data:
    boost::uint8_t m_ratio;
    rgba m_color;
};


/// A BitmapFill
//
/// BitmapFills can refer to a parsed bitmap tag or be constructed from
/// bitmap data. They are used for Bitmap characters.
//
/// Presently all members are immutable after construction. It is of course
/// possible to change the appearance of the fill by changing the BitmapInfo
/// it refers to.
//
/// TODO: check the following:
//
/// It may be necessary to allow setting the smoothing policy; the use of
/// this should certainly be extended to non-static BitmapFills.
class DSOEXPORT BitmapFill
{
public:

    /// How to smooth the bitmap.
    enum SmoothingPolicy {
        SMOOTHING_UNSPECIFIED,
        SMOOTHING_ON,
        SMOOTHING_OFF
    };
    
    /// Whether the fill is tiled or clipped.
    //
    /// Clipped fills use the edge pixels to fill any area outside the bounds
    /// of the image.
    enum Type {
        CLIPPED,
        TILED
    };

    /// Construct a BitmapFill from arbitrary bitmap data.
    //
    /// TODO: check the smoothing policy here!
    BitmapFill(Type t, const BitmapInfo* bi, const SWFMatrix& m)
        :
        _type(t),
        _smoothingPolicy(SMOOTHING_UNSPECIFIED),
        _matrix(m),
        _bitmapInfo(bi),
        _md(0),
        _id(0)
    {
    }

    /// Construct a static BitmapFill using a SWF tag.
    BitmapFill(SWF::FillType t, movie_definition* md, boost::uint16_t id,
            const SWFMatrix& m);

    /// Copy a BitmapFill
    //
    /// The copied BitmapFill refers to the same bitmap id in the same
    /// movie_definition as the original.
    BitmapFill(const BitmapFill& other)
        :
        _type(other._type),
        _smoothingPolicy(other._smoothingPolicy),
        _matrix(other._matrix),
        _bitmapInfo(other._bitmapInfo),
        _md(other._md),
        _id(other._id)
    {}

    /// Set this fill to a lerp of two other BitmapFills.
    void setLerp(const BitmapFill& a, const BitmapFill& b, double ratio);

    /// Get the Type of this BitmapFill
    //
    /// BitmapFills are either tiled or clipped.
    Type type() const {
        return _type;
    }

    /// Get the smoothing policy of this BitmapFill.
    SmoothingPolicy smoothingPolicy() const {
        return _smoothingPolicy;
    }

    /// Get the actual Bitmap data.
    const BitmapInfo* bitmap() const;

    /// Get the matrix of this BitmapFill.
    const SWFMatrix& matrix() const {
        return _matrix;
    }

private:

    Type _type;

    SmoothingPolicy _smoothingPolicy;

    SWFMatrix _matrix;
    
    /// A Bitmap, used for dynamic fills and to cache parsed bitmaps.
    mutable boost::intrusive_ptr<const BitmapInfo> _bitmapInfo;

    /// The movie definition containing the bitmap
    movie_definition* _md;

    // The id of the tag containing the bitmap
    boost::uint16_t _id;
};

class DSOEXPORT GradientFill
{
public:

    enum Type
    {
        LINEAR,
        RADIAL,
        FOCAL
    };

    typedef std::vector<gradient_record> GradientRecords;

    GradientFill(Type t, const GradientRecords& recs, const SWFMatrix& m)
        :
        type(t),
        gradients(recs),
        matrix(m),
        focalPoint(0.0),
        spreadMode(SWF::GRADIENT_SPREAD_PAD),
        interpolation(SWF::GRADIENT_INTERPOLATION_NORMAL)
    {
        assert(recs.size() > 1);
    }
    
    /// Set this fill to a lerp of two other GradientFills.
    void setLerp(const GradientFill& a, const GradientFill& b, double ratio);

    GradientFill() {}

    Type type;

    GradientRecords gradients;
    SWFMatrix matrix;
    double focalPoint;
    SWF::SpreadMode spreadMode;
    SWF::InterpolationMode interpolation;
};

/// A SolidFill containing one color.
//
/// SolidFills are the simplest fill, containing only a single color.
struct DSOEXPORT SolidFill
{
public:

    /// Construct a SolidFill.
    explicit SolidFill(const rgba& c)
        :
        _color(c)
    {}

    /// Copy a SolidFill.
    SolidFill(const SolidFill& other)
        :
        _color(other._color)
    {}

    /// Set this fill to a lerp of two other SolidFills.
    void setLerp(const SolidFill& a, const SolidFill& b, double ratio) {
        _color.set_lerp(a.color(), b.color(), ratio);
    }

    /// Get the color of the fill.
    rgba color() const {
        return _color;
    }

private:
    rgba _color;
};

/// For the interior of outline shapes.
class DSOEXPORT fill_style 
{
public:

    typedef boost::variant<BitmapFill, SolidFill, GradientFill> Fill;
    fill_style(const Fill& f = SolidFill(rgba())) : fill(f) {}

    /// Read the fill style from a stream
    //
    /// Throw a ParserException if there's no enough bytes in the
    /// currently opened tag for reading. See stream::ensureBytes()
    void read(SWFStream& in, SWF::TagType t, movie_definition& m,
            const RunResources& r, fill_style *pOther = 0);
    
    /// Sets this style to a blend of a and b.  t = [0,1] (for shape morphing)
    void set_lerp(const fill_style& a, const fill_style& b, float t);

    Fill fill;

};

DSOEXPORT std::ostream& operator<<(std::ostream& os,
        const BitmapFill::SmoothingPolicy& p);

} // namespace gnash

#endif 


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
