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


#ifndef GNASH_GRADIENTGLOWFILTER_H
#define GNASH_GRADIENTGLOWFILTER_H

#include "BitmapFilter.h"

#include <vector>
#include <boost/cstdint.hpp> // For C99 int types


namespace gnash {

// A gradient glow effect filter.
class GradientGlowFilter : public BitmapFilter
{
public:
    typedef enum
    {
        INNER_GLOW = 2,
        OUTER_GLOW = 1,
        FULL_GLOW = 3
    } glow_types;

    // Fill from a SWFStream. See parser/filter_factory.cpp for the implementations.
    virtual bool read(SWFStream& in);

    virtual ~GradientGlowFilter() { return; }

    GradientGlowFilter() : 
        m_distance(0.0f), m_angle(0.0f), m_colors(), m_alphas(), m_ratios(),
        m_blurX(0.0f), m_blurY(0.0f),  m_strength(0.0f), m_quality(0),
        m_type(INNER_GLOW), m_knockout(false)
    { return; }

    GradientGlowFilter(float distance, float angle,
        std::vector<boost::uint32_t> colors,
        std::vector<boost::uint8_t> alphas,
        std::vector<boost::uint8_t> ratios,
        float blurX, float blurY, float strength,
        boost::uint8_t quality, glow_types type, bool knockout) :
        m_distance(distance), m_angle(angle), m_colors(colors), m_alphas(alphas),
        m_ratios(ratios), m_blurX(blurX), m_blurY(blurY), m_strength(strength),
        m_quality(quality), m_type(type), m_knockout(knockout)
    { return; }

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

#endif // GNASH_GRADIENTGLOWFILTER_H
