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

struct BitmapFill
{
    enum SmoothingPolicy {
        SMOOTHING_UNSPECIFIED,
        SMOOTHING_ON,
        SMOOTHING_OFF
    };
    
    enum Type {
        CLIPPED,
        TILED
    };

    /// Construct a BitmapFill from arbitrary bitmap data.
    BitmapFill(Type t, const BitmapInfo* bi, const SWFMatrix& m)
        :
        type(t),
        matrix(m),
        _bitmapInfo(bi),
        _md(0),
        _id(0)
    {}

    /// Construct a static BitmapFill using a SWF tag.
    BitmapFill(Type t, movie_definition* md, boost::uint16_t id,
            const SWFMatrix& m)
        :
        type(t),
        matrix(m),
        _bitmapInfo(0),
        _md(md),
        _id(id)
    {}

    /// Copy a BitmapFill
    //
    /// The copied BitmapFill refers to the same bitmap id in the same
    /// movie_definition as the original.
    BitmapFill(const BitmapFill& other)
        :
        type(other.type),
        matrix(other.matrix),
        _bitmapInfo(other._bitmapInfo),
        _md(other._md),
        _id(other._id)
    {}

    Type type;

    SmoothingPolicy smoothingPolicy;

    SWFMatrix matrix;

    /// Get the bitmap.
    const BitmapInfo* bitmap() const;

private:

    /// A Bitmap, used for dynamic fills and to cache parsed bitmaps.
    mutable boost::intrusive_ptr<const BitmapInfo> _bitmapInfo;

    /// The movie definition containing the bitmap
    movie_definition* _md;

    // The id of the tag containing the bitmap
    boost::uint16_t _id;
};

struct GradientFill
{
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

    GradientFill() {}

    Type type;

    GradientRecords gradients;
    SWFMatrix matrix;
    double focalPoint;
    SWF::SpreadMode spreadMode;
    SWF::InterpolationMode interpolation;
    rgba color;
};

struct SolidFill
{
    explicit SolidFill(const rgba& c)
        :
        color(c)
    {}

    SolidFill(const SolidFill& other)
        :
        color(other.color)
    {}

    rgba color;
};

/// For the interior of outline shapes.
class DSOEXPORT fill_style 
{
public:

    typedef boost::variant<BitmapFill, SolidFill, GradientFill> Fill;
    fill_style(const Fill& f = SolidFill(rgba())) : fill(f) {}

    /// Read the fill style from a stream
    //
    /// TODO: use a subclass for this (swf_fill_style?)
    ///
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
