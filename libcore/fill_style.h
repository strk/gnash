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

#ifndef GNASH_FILL_STYLE_H
#define GNASH_FILL_STYLE_H

#include <boost/variant.hpp>
#include <vector> 
#include <iosfwd> 
#include <boost/optional.hpp>
#include <boost/intrusive_ptr.hpp>
#include <cassert>

#include "SWFMatrix.h"
#include "BitmapInfo.h"
#include "SWF.h"
#include "RGBA.h" // for rgba type

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


/// A GradientFill
//
/// TODO: clean this up!
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

    /// Construct a GradientFill
    //
    /// Optionally the records can be passed here.
    //
    /// The actual matrix of the gradient depends on the type; the constructor
    /// handles this, and users should just pass the user matrix.
    GradientFill(Type t, const SWFMatrix& m,
            const GradientRecords& = GradientRecords());

    Type type() const {
        return _type;
    }

    const SWFMatrix& matrix() const {
        return _matrix;
    }

    /// Set this fill to a lerp of two other GradientFills.
    void setLerp(const GradientFill& a, const GradientFill& b, double ratio);
    
    void setRecords(const GradientRecords& recs) {
        assert(recs.size() > 1);
        gradients = recs;
    }

    GradientRecords gradients;
    double focalPoint;
    SWF::SpreadMode spreadMode;
    SWF::InterpolationMode interpolation;

private:
    Type _type;
    SWFMatrix _matrix;
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
    
    fill_style(const Fill& f) : fill(f) {}

    fill_style(const fill_style& other)
        :
        fill(other.fill)
    {}

    Fill fill;

};
 
/// Set the fill_style to a lerp of a and b.
//
/// Callers must ensure that all fill_styles have exactly the same type! Most
/// errors are caught by type-checking and will throw an unhandled exception.
void setLerp(fill_style& f, const fill_style& a, const fill_style& b, double t);

/// Either a single or a morph-pair fill_style.
typedef std::pair<fill_style, boost::optional<fill_style> > OptionalFillPair;

/// Read fill_styles from a stream
//
/// Read either single or morph-pair fill styles from a stream. 
OptionalFillPair readFills(SWFStream& in, SWF::TagType t, movie_definition& m,
        bool readMorph);

DSOEXPORT std::ostream& operator<<(std::ostream& os,
        const BitmapFill::SmoothingPolicy& p);

} // namespace gnash

#endif 


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
