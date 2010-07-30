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
        /// Only smooth when _quality >= BEST
        //
        /// This is the policy for bitmap fills
        /// defined by SWF up to version 7:
        ///  - SWF::FILL_CLIPPED_BITMAP
        ///  - SWF::FILL_TILED_BITMAP
        SMOOTHING_UNSPECIFIED,

        /// Always smooth if _quality > LOW
        //
        /// This is the policy for non-hard bitmap fills
        /// defined by SWF 8 and higher:
        ///  - SWF::FILL_CLIPPED_BITMAP
        ///  - SWF::FILL_TILED_BITMAP
        SMOOTHING_ON,

        /// Never smooth
        ///
        /// MovieClip.forceSmoothing can force this to
        /// behave like SMOOTHING_ON 
        ///
        /// This is the policy for hard bitmap fills
        /// introduced in SWF 8:
        ///  - SWF::FILL_CLIPPED_BITMAP_HARD
        ///  - SWF::FILL_TILED_BITMAP_HARD
        SMOOTHING_OFF

    };
    
    enum Type {
        CLIPPED,
        TILED,
        CLIPPED_HARD,
        TILED_HARD
    };

    Type type;

    boost::intrusive_ptr<const BitmapInfo> bitmapInfo;

    SmoothingPolicy smoothingPolicy;

    SWFMatrix matrix;
};

struct GradientFill
{
    enum Type
    {
        LINEAR,
        RADIAL,
        FOCAL
    };

    explicit GradientFill(Type t) : type(t) {}
    GradientFill() {}

    Type type;

    SWFMatrix matrix;
    float focalPoint;
    std::vector<gradient_record> gradients;
    SWF::SpreadMode spreadMode;
    SWF::InterpolationMode interpolation;
    rgba color;
};

struct SolidFill
{
    explicit SolidFill(const rgba& c) : color(c) {}
    rgba color;
};

/// For the interior of outline shapes.
class DSOEXPORT fill_style 
{
public:

    typedef boost::variant<BitmapFill, SolidFill, GradientFill> Fill;

    /// Create a solid opaque white fill.
    fill_style();

    /// Construct a clipped bitmap fill style, for
    /// use by bitmap shape DisplayObject.
    ///
    /// TODO: use a subclass for this
    /// TODO: provide a setBitmap, for consisteny with other setType() methods
    ///
    /// @param bitmap
    ///    The bitmap DisplayObject definition to use with this bitmap fill.
    ///
    /// @param mat
    ///    The SWFMatrix to apply to the bitmap.
    ///
    fill_style(const BitmapInfo* const bitmap, const SWFMatrix& mat);

    ~fill_style() {}

    /// Turn this fill style into a solid fill.
    //
    /// This is used for dynamic gradient generation.
    //
    void setSolid(const rgba& color);

    /// Turn this fill style into a linear gradient
    //
    /// This is used for dynamic gradient generation.
    //
    /// Note: passing only one gradient record will result in a solid fill
    /// style, not a gradient. This is for compatibility with dynamic
    /// fill style generation.
    //
    /// @param gradients    Gradient records.
    /// @param mat          Gradient SWFMatrix.
    void setLinearGradient(const std::vector<gradient_record>& gradients, 
            const SWFMatrix& mat);

    /// Turn this fill style into a radial gradient
    //
    /// This is used for dynamic gradient generation.
    //
    /// Note: passing only one gradient record will result in a solid fill
    /// style, not a gradient. This is for compatibility with dynamic
    /// fill style generation.
    //
    /// @param gradients    Gradient records.
    /// @param mat          Gradient SWFMatrix.
    void setRadialGradient(const std::vector<gradient_record>& gradients,
            const SWFMatrix& mat);
    
    /// Read the fill style from a stream
    //
    /// TODO: use a subclass for this (swf_fill_style?)
    ///
    /// Throw a ParserException if there's no enough bytes in the
    /// currently opened tag for reading. See stream::ensureBytes()
    void read(SWFStream& in, SWF::TagType t, movie_definition& m,
            const RunResources& r, fill_style *pOther = 0);
    
    rgba get_color() const;

    void set_color(rgba new_color);

    /// Get fill type, see SWF::fill_style_type
    boost::uint8_t get_type() const {
        return m_type;
    }

    SWF::SpreadMode get_gradient_spread_mode() const {
        return boost::get<GradientFill>(_fill).spreadMode;
    }

    SWF::InterpolationMode get_gradient_interpolation_mode() const {
        return boost::get<GradientFill>(_fill).interpolation;
    }
    
    /// Sets this style to a blend of a and b.  t = [0,1] (for shape morphing)
    void set_lerp(const fill_style& a, const fill_style& b, float t);
    
    /// Returns the bitmap info for all styles except solid fills
    //
    /// NOTE: calling this method against a solid fill style will
    ///       result in a failed assertion.
    /// 
    /// NOTE2: this function can return NULL if the DisplayObject_id
    ///        specified for the style in the SWF does not resolve
    ///        to a DisplayObject defined in the DisplayObjects dictionary.
    ///        (it happens..)
    ///
    const BitmapInfo* get_bitmap_info(Renderer& renderer) const;

    BitmapFill::SmoothingPolicy getBitmapSmoothingPolicy() const {
        return boost::get<BitmapFill>(_fill).smoothingPolicy;
    }
    
    /// Returns the bitmap transformation SWFMatrix
    const SWFMatrix& getBitmapMatrix() const; 
    
    /// Returns the gradient transformation SWFMatrix
    const SWFMatrix& getGradientMatrix() const; 
    
    /// Returns the number of color stops in the gradient
    size_t get_color_stop_count() const;
    
    /// Returns the color stop value at a specified index
    const gradient_record& get_color_stop(size_t index) const;

    /// Get and set the focal point for gradient focal fills.
    /// This should be from -1.0 to 1.0, representing the left
    /// and right edges of the rectangle.
    float get_focal_point() const {
        return boost::get<GradientFill>(_fill).focalPoint;
    }
    
    Fill _fill;

private:

    /// Fill type, see SWF::fill_style_type
    boost::uint8_t m_type;

};

DSOEXPORT std::ostream& operator<<(std::ostream& os,
        const BitmapFill::SmoothingPolicy& p);

} // namespace gnash

#endif 


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
