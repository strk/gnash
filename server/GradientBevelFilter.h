// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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

/* $Id: GradientBevelFilter.h,v 1.4 2007/09/26 19:00:46 strk Exp $ */

#ifndef GNASH_GRADIENTBEVELFILTER_H
#define GNASH_GRADIENTBEVELFILTER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "BitmapFilter.h"

#include <vector>
#include <boost/cstdint.hpp> // for XintXX_t

namespace gnash {

// A gradient bevel effect filter.
class GradientBevelFilter : public BitmapFilter
{
public:
    typedef enum
    {
        INNER_BEVEL = 2,
        OUTER_BEVEL = 1,
        FULL_BEVEL = 3
    } glow_types;

    // Fill from a stream. See parser/filter_factory.cpp for the implementations.
    virtual bool read(stream* in);

    virtual ~GradientBevelFilter() { return; }

    GradientBevelFilter() : 
        m_distance(0.0f), m_angle(0.0f), m_colors(), m_alphas(), m_ratios(),
        m_blurX(0.0f), m_blurY(0.0f),  m_strength(0.0f), m_quality(0),
        m_type(INNER_BEVEL), m_knockout(false)
    { return; }

    GradientBevelFilter(float distance, float angle,
        std::vector<uint32_t> colors,
        std::vector<uint8_t> alphas,
        std::vector<uint8_t> ratios,
        float blurX, float blurY, float strength,
        uint8_t quality, glow_types type, bool knockout) :
        m_distance(distance), m_angle(angle),
        m_colors(colors), m_alphas(alphas), m_ratios(ratios),
        m_blurX(blurX), m_blurY(blurY), m_strength(strength),
        m_quality(quality), m_type(type), m_knockout(knockout)
    { return; }

protected:
    float m_distance; // Distance of the filter in pixels.
    float m_angle; // Angle of the filter.
    std::vector<uint32_t> m_colors; // Colors of the gradients.
    std::vector<uint8_t> m_alphas; // Alphas of the gradients.
    std::vector<uint8_t> m_ratios; // Ratios of the gradients.
    float m_blurX; // horizontal blur
    float m_blurY; // vertical blur
    float m_strength; // How strong is the filter.
    uint8_t m_quality; // How many times to apply the filter.
    glow_types m_type; // What type of effect.
    bool m_knockout; // If true, render only the filter effect.
};

} // Namespace gnash

#endif // GNASH_GRADIENTBEVELFILTER_H
