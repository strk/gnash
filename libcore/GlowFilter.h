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


#ifndef GNASH_GLOWFILTER_H
#define GNASH_GLOWFILTER_H

#include "BitmapFilter.h"

#include <boost/cstdint.hpp> // for boost::uint32_t

namespace gnash {

// A glow effect filter.
class GlowFilter : public BitmapFilter
{
public:
    // Fill from a SWFStream. See parser/filter_factory.cpp for the implementations.
    virtual bool read(SWFStream& in);

    virtual ~GlowFilter() { return; }

    GlowFilter() : 
        m_color(0), m_alpha(0),
        m_blurX(0.0f), m_blurY(0.0f),  m_strength(0.0f), m_quality(0),
        m_inner(false), m_knockout(false)
    { return; }

    GlowFilter(boost::uint32_t color,
        boost::uint8_t alpha, float blurX, float blurY, float strength,
        boost::uint8_t quality, bool inner, bool knockout) :
        m_color(color),
        m_alpha(alpha), m_blurX(blurX), m_blurY(blurY), m_strength(strength),
        m_quality(quality), m_inner(inner), m_knockout(knockout)
    { return; }

    boost::uint32_t m_color; // RGB color.
    boost::uint8_t m_alpha; // Alpha strength, as a percentage(?)
    float m_blurX; // horizontal blur
    float m_blurY; // vertical blur
    float m_strength; // How strong is the filter.
    boost::uint8_t m_quality; // How many times to apply the filter.
    bool m_inner; // Is this an inner shadow?
    bool m_knockout; // If true, render only the filter effect.
};

} // Namespace gnash

#endif // GNASH_GLOWFILTER_H
