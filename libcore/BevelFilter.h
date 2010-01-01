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


#ifndef GNASH_BEVELFILTER_H
#define GNASH_BEVELFILTER_H

#include "BitmapFilter.h"

#include <boost/cstdint.hpp> // for XintXX_t

namespace gnash {

// A bevel effect filter.
class BevelFilter : public BitmapFilter
{
public:
    typedef enum
    {
        OUTER_BEVEL = 1,
        INNER_BEVEL = 2,
        FULL_BEVEL = 3
    } bevel_type;

    // Fill from a SWFStream. See parser/filter_factory.cpp for the implementations.
    virtual bool read(SWFStream& in);

    virtual ~BevelFilter() { return; }

    BevelFilter() : 
        m_distance(0.0f), m_angle(0.0f), m_highlightColor(0),
        m_highlightAlpha(0), m_shadowColor(0), m_shadowAlpha(0),
        m_blurX(0.0f), m_blurY(0.0f),  m_strength(0.0f), m_quality(0),
        m_type(FULL_BEVEL), m_knockout(false)
    { return; }

    BevelFilter(float distance, float angle, boost::uint32_t hcolor,
        boost::uint8_t halpha, boost::uint32_t scolor, boost::uint8_t salpha,
        float blurX, float blurY, float strength,
        boost::uint8_t quality, bevel_type type, bool knockout) :
        m_distance(distance), m_angle(angle), m_highlightColor(hcolor),
        m_highlightAlpha(halpha), m_shadowColor(scolor), m_shadowAlpha(salpha),
        m_blurX(blurX), m_blurY(blurY), m_strength(strength),
        m_quality(quality), m_type(type), m_knockout(knockout)
    { return; }

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

} // Namespace gnash

#endif // GNASH_BEVELFILTER_H
