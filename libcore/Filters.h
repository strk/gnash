// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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


#ifndef GNASH_FILTERS_H
#define GNASH_FILTERS_H

#include <boost/cstdint.hpp> 
#include <vector>

namespace gnash {
    class SWFStream;
}

namespace gnash {

// The common base class for AS display filters.
class BitmapFilter
{
public:
    virtual bool read(SWFStream& /*in*/) {
        return true;
    }
    BitmapFilter() {}
    virtual ~BitmapFilter() {}
};

// A bevel effect filter.
class BevelFilter : public BitmapFilter
{
public:
    enum bevel_type
    {
        OUTER_BEVEL = 1,
        INNER_BEVEL = 2,
        FULL_BEVEL = 3
    };

    // Fill from a SWFStream. See parser/filter_factory.cpp for the implementations.
    virtual bool read(SWFStream& in);

    virtual ~BevelFilter() {}

    BevelFilter()
        : 
        m_distance(0.0f),
        m_angle(0.0f),
        m_highlightColor(0),
        m_highlightAlpha(0),
        m_shadowColor(0),
        m_shadowAlpha(0),
        m_blurX(0.0f),
        m_blurY(0.0f),
        m_strength(0.0f),
        m_quality(0),
        m_type(FULL_BEVEL),
        m_knockout(false)
    {}

    BevelFilter(float distance, float angle, boost::uint32_t hcolor,
        boost::uint8_t halpha, boost::uint32_t scolor, boost::uint8_t salpha,
        float blurX, float blurY, float strength,
        boost::uint8_t quality, bevel_type type, bool knockout) :
        m_distance(distance), m_angle(angle), m_highlightColor(hcolor),
        m_highlightAlpha(halpha), m_shadowColor(scolor), m_shadowAlpha(salpha),
        m_blurX(blurX), m_blurY(blurY), m_strength(strength),
        m_quality(quality), m_type(type), m_knockout(knockout)
    {}

    float m_distance; // Distance of the filter in pixels.
    float m_angle; // Angle of the filter.
    boost::uint32_t m_highlightColor; // Color of the highlight.
    boost::uint8_t m_highlightAlpha; // Alpha of the highlight.
    boost::uint32_t m_shadowColor; // RGB color.
    boost::uint8_t m_shadowAlpha; // Alpha strength, as a percentage(?)
    float m_blurX; // horizontal blur
    float m_blurY; // vertical blur
    float m_strength; // How strong is the filter.
    boost::uint8_t m_quality; // How many times to apply the filter.
    bevel_type m_type; // The type of filter. (Rendered as string in AS)
    bool m_knockout; // If true, render only the filter effect.
};

// A blur effect filter.
class BlurFilter : public BitmapFilter
{
public:
    // Fill from a SWFStream. See parser/filter_factory.cpp for the implementations.
    virtual bool read(SWFStream& in);

    virtual ~BlurFilter() {}

    BlurFilter() : 
        m_blurX(0.0f), m_blurY(0.0f), m_quality(0)
    {}

    BlurFilter(float blurX, float blurY, boost::uint8_t quality) :
        m_blurX(blurX), m_blurY(blurY), m_quality(quality)
    {}

    float m_blurX; // How much horizontal blur.
    float m_blurY; // How much vertical blur.
    boost::uint8_t m_quality; // How many passes to take.
};

// A color SWFMatrix effect filter.
class ColorMatrixFilter : public BitmapFilter
{
public:
    // Fill from a SWFStream. See parser/filter_factory.cpp for the implementations.
    virtual bool read(SWFStream& in);

    virtual ~ColorMatrixFilter() {}

    ColorMatrixFilter() : 
        m_matrix()
    {}

    ColorMatrixFilter(std::vector<float> a_matrix) :
        m_matrix(a_matrix)
    {}

protected:
    std::vector<float> m_matrix; // The color SWFMatrix
};

// A convolution effect filter.
class ConvolutionFilter : public BitmapFilter
{
public:
    // Fill from a SWFStream. See parser/filter_factory.cpp for
    // the implementations.
    virtual bool read(SWFStream& in);

    virtual ~ConvolutionFilter() {}

    ConvolutionFilter()
        :
        _matrixX(),
        _matrixY(),
        _matrix(),
        _divisor(),
        _bias(),
        _preserveAlpha(false),
        _clamp(false),
        _color(),
        _alpha()
    {}

    ConvolutionFilter(boost::uint8_t matrixX, boost::uint8_t matrixY, 
        const std::vector<float>& _matrix, float divisor, float bias,
        bool preserveAlpha, bool clamp, boost::uint32_t color,
        boost::uint8_t alpha)
        :
        _matrixX(matrixX),
        _matrixY(matrixY),
        _matrix(_matrix),
        _divisor(divisor),
        _bias(bias),
        _preserveAlpha(preserveAlpha),
        _clamp(clamp),
        _color(color),
        _alpha(alpha)
    {}

protected:
    boost::uint8_t _matrixX; // Number of columns
    boost::uint8_t _matrixY; // Number of rows
    std::vector<float> _matrix; // The convolution matrix
    float _divisor;
    float _bias;
    bool _preserveAlpha; // If true, don't convolute the alpha channel
    bool _clamp; // Whether or not to clamp
    boost::uint32_t _color; // For off-image pixels
    boost::uint8_t _alpha; // For off-image pixels
};

// A drop shadow effect filter.
class DropShadowFilter : public BitmapFilter
{
public:
    // Fill from a SWFStream. See parser/filter_factory.cpp for the implementations.
    virtual bool read(SWFStream& in);

    virtual ~DropShadowFilter() {}

    DropShadowFilter() : 
        m_distance(0.0f), m_angle(0.0f), m_color(0), m_alpha(0),
        m_blurX(0.0f), m_blurY(0.0f),  m_strength(0.0f), m_quality(0),
        m_inner(false), m_knockout(false), m_hideObject(false)
    {}

    DropShadowFilter(float distance, float angle, boost::uint32_t color,
        boost::uint8_t alpha, float blurX, float blurY, float strength,
        boost::uint8_t quality, bool inner, bool knockout, bool hideObject) :
        m_distance(distance), m_angle(angle), m_color(color),
        m_alpha(alpha), m_blurX(blurX), m_blurY(blurY), m_strength(strength),
        m_quality(quality), m_inner(inner), m_knockout(knockout),
        m_hideObject(hideObject)
    {}

    float m_distance; // Distance of the filter in pixels.
    float m_angle; // Angle of the filter.
    boost::uint32_t m_color; // RGB color.
    boost::uint8_t m_alpha; // Alpha strength, as a percentage(?)
    float m_blurX; // horizontal blur
    float m_blurY; // vertical blur
    float m_strength; // How strong is the filter.
    boost::uint8_t m_quality; // How many times to apply the filter.
    bool m_inner; // Is this an inner shadow?
    bool m_knockout; // If true, render only the filter effect.
    bool m_hideObject; // Does this hide the object?
};


// A glow effect filter.
class GlowFilter : public BitmapFilter
{
public:
    // Fill from a SWFStream. See parser/filter_factory.cpp for the implementations.
    virtual bool read(SWFStream& in);

    virtual ~GlowFilter() {}

    GlowFilter() : 
        m_color(0), m_alpha(0),
        m_blurX(0.0f), m_blurY(0.0f),  m_strength(0.0f), m_quality(0),
        m_inner(false), m_knockout(false)
    {}

    GlowFilter(boost::uint32_t color,
        boost::uint8_t alpha, float blurX, float blurY, float strength,
        boost::uint8_t quality, bool inner, bool knockout) :
        m_color(color),
        m_alpha(alpha), m_blurX(blurX), m_blurY(blurY), m_strength(strength),
        m_quality(quality), m_inner(inner), m_knockout(knockout)
    {}

    boost::uint32_t m_color; // RGB color.
    boost::uint8_t m_alpha; // Alpha strength, as a percentage(?)
    float m_blurX; // horizontal blur
    float m_blurY; // vertical blur
    float m_strength; // How strong is the filter.
    boost::uint8_t m_quality; // How many times to apply the filter.
    bool m_inner; // Is this an inner shadow?
    bool m_knockout; // If true, render only the filter effect.
};


// A gradient bevel effect filter.
class GradientBevelFilter : public BitmapFilter
{
public:
    enum glow_types
    {
        INNER_BEVEL = 2,
        OUTER_BEVEL = 1,
        FULL_BEVEL = 3
    };

    // Fill from a SWFStream. See parser/filter_factory.cpp for the implementations.
    virtual bool read(SWFStream& in);

    virtual ~GradientBevelFilter() {}

    GradientBevelFilter() : 
        m_distance(0.0f), m_angle(0.0f), m_colors(), m_alphas(), m_ratios(),
        m_blurX(0.0f), m_blurY(0.0f),  m_strength(0.0f), m_quality(0),
        m_type(INNER_BEVEL), m_knockout(false)
    {}

    GradientBevelFilter(float distance, float angle,
        std::vector<boost::uint32_t> colors,
        std::vector<boost::uint8_t> alphas,
        std::vector<boost::uint8_t> ratios,
        float blurX, float blurY, float strength,
        boost::uint8_t quality, glow_types type, bool knockout) :
        m_distance(distance), m_angle(angle),
        m_colors(colors), m_alphas(alphas), m_ratios(ratios),
        m_blurX(blurX), m_blurY(blurY), m_strength(strength),
        m_quality(quality), m_type(type), m_knockout(knockout)
    {}

    float m_distance; // Distance of the filter in pixels.
    float m_angle; // Angle of the filter.
    std::vector<boost::uint32_t> m_colors; // Colors of the gradients.
    std::vector<boost::uint8_t> m_alphas; // Alphas of the gradients.
    std::vector<boost::uint8_t> m_ratios; // Ratios of the gradients.
    float m_blurX; // horizontal blur
    float m_blurY; // vertical blur
    float m_strength; // How strong is the filter.
    boost::uint8_t m_quality; // How many times to apply the filter.
    glow_types m_type; // What type of effect.
    bool m_knockout; // If true, render only the filter effect.
};

// A gradient glow effect filter.
class GradientGlowFilter : public BitmapFilter
{
public:
    enum glow_types
    {
        INNER_GLOW = 2,
        OUTER_GLOW = 1,
        FULL_GLOW = 3
    };

    // Fill from a SWFStream. See parser/filter_factory.cpp for the implementations.
    virtual bool read(SWFStream& in);

    virtual ~GradientGlowFilter() {}

    GradientGlowFilter() : 
        m_distance(0.0f), m_angle(0.0f), m_colors(), m_alphas(), m_ratios(),
        m_blurX(0.0f), m_blurY(0.0f),  m_strength(0.0f), m_quality(0),
        m_type(INNER_GLOW), m_knockout(false)
    {}

    GradientGlowFilter(float distance, float angle,
        std::vector<boost::uint32_t> colors,
        std::vector<boost::uint8_t> alphas,
        std::vector<boost::uint8_t> ratios,
        float blurX, float blurY, float strength,
        boost::uint8_t quality, glow_types type, bool knockout) :
        m_distance(distance), m_angle(angle), m_colors(colors), m_alphas(alphas),
        m_ratios(ratios), m_blurX(blurX), m_blurY(blurY), m_strength(strength),
        m_quality(quality), m_type(type), m_knockout(knockout)
    {}

    float m_distance; // Distance of the filter in pixels.
    float m_angle; // Angle of the filter.
    std::vector<boost::uint32_t> m_colors; // Colors of the gradients.
    std::vector<boost::uint8_t> m_alphas; // Alphas of the gradients.
    std::vector<boost::uint8_t> m_ratios; // Ratios of the gradients.
    float m_blurX; // horizontal blur
    float m_blurY; // vertical blur
    float m_strength; // How strong is the filter.
    boost::uint8_t m_quality; // How many times to apply the filter.
    glow_types m_type; // What type of effect.
    bool m_knockout; // If true, render only the filter effect.
};

} // Namespace gnash

#endif
