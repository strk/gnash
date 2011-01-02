// FillStyle.h: variant fill styles
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
#include <boost/intrusive_ptr.hpp>
#include <cassert>

#include "SWFMatrix.h"
#include "SWF.h"
#include "RGBA.h" 

namespace gnash {
    class movie_definition;
    class CachedBitmap;
}

namespace gnash {

class GradientRecord
{
public:
    GradientRecord(boost::uint8_t ratio, const rgba& color)
        :
        ratio(ratio),
        color(color)
    { }
    
    //data:
    boost::uint8_t ratio;
    rgba color;
};

/// A BitmapFill
//
/// BitmapFills can refer to a parsed bitmap tag or be constructed from
/// bitmap data. They are used for Bitmap characters.
//
/// Presently all members are immutable after construction. It is of course
/// possible to change the appearance of the fill by changing the CachedBitmap
/// it refers to.
//
/// Special member functions (ctor, dtor etc) are not inlined to avoid 
/// requiring the definition of movie_definition.
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
    BitmapFill(Type t, const CachedBitmap* bi, const SWFMatrix& m,
            SmoothingPolicy pol);

    /// Construct a static BitmapFill using a SWF tag.
    BitmapFill(SWF::FillType t, movie_definition* md, boost::uint16_t id,
            const SWFMatrix& m);

    /// Destructor
    ~BitmapFill();

    /// Copy a BitmapFill
    //
    /// The copied BitmapFill refers to the same bitmap id in the same
    /// movie_definition as the original.
    BitmapFill(const BitmapFill& other);
    
    BitmapFill& operator=(const BitmapFill& other);

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
    const CachedBitmap* bitmap() const;

    /// Get the matrix of this BitmapFill.
    const SWFMatrix& matrix() const {
        return _matrix;
    }

private:

    Type _type;

    SmoothingPolicy _smoothingPolicy;

    SWFMatrix _matrix;
    
    /// A Bitmap, used for dynamic fills and to cache parsed bitmaps.
    mutable boost::intrusive_ptr<const CachedBitmap> _bitmapInfo;

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

    /// The type of GradientFill
    //
    /// A Focal fill is a gradient fill with a focal point.
    enum Type {
        LINEAR,
        RADIAL
    };

    enum SpreadMode {
        PAD,
        REPEAT,
        REFLECT
    };

    typedef std::vector<GradientRecord> GradientRecords;

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
        _gradients = recs;
    }

    /// Get the number of records in this GradientFill
    size_t recordCount() const {
        return _gradients.size();
    }

    /// Query the GradientRecord at the specified index
    //
    /// There are recordCount() records.
    const GradientRecord& record(size_t i) const {
        assert(i < _gradients.size());
        return _gradients[i];
    }

    /// Set the focal point.
    //
    /// Value will be clamped to the range -1..1; callers don't need to check.
    void setFocalPoint(double d);

    /// Get the focal point of this GradientFill
    //
    /// If the focal point is 0.0, it is a simple radial fill.
    double focalPoint() const {
        return _focalPoint;
    }

    SpreadMode spreadMode;
    SWF::InterpolationMode interpolation;

private:

    double _focalPoint;
    GradientRecords _gradients;
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
    { }

    /// Copy a SolidFill.
    SolidFill(const SolidFill& other)
        :
        _color(other._color)
    { }

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

/// FillStyle describes the various fill styles for shapes
//
/// The FillStyle class is effectively a boost::variant, but to allow passing
/// FillStyles using a forward declaration (and reducing compile times),
/// it's necessary to use a class.
class DSOEXPORT FillStyle 
{
public:

    typedef boost::variant<BitmapFill, SolidFill, GradientFill> Fill;
    
    /// Construct a FillStyle from any Fill.
    //
    /// The non-explicit templated contructor allows the same syntax as a
    /// simple boost::variant:
    ///     FillStyle f = GradientFill();
    template<typename T> FillStyle(const T& f) : fill(f) {}

    FillStyle(const FillStyle& other)
        :
        fill(other.fill)
    { }

    Fill fill;

};
 
/// Set the FillStyle to a lerp of a and b.
//
/// Callers must ensure that all FillStyles have exactly the same type! Most
/// errors are caught by type-checking and will throw an unhandled exception.
void setLerp(FillStyle& f, const FillStyle& a, const FillStyle& b, double t);

DSOEXPORT std::ostream& operator<<(std::ostream& os,
        const BitmapFill::SmoothingPolicy& p);

} // namespace gnash

#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
