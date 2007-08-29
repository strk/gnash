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

/* $Id: DropShadowFilter.h,v 1.3 2007/08/29 03:32:58 cmusick Exp $ */

#ifndef GNASH_DROPSHADOWFILTER_H
#define GNASH_DROPSHADOWFILTER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "BitmapFilter.h"

namespace gnash {

// A drop shadow effect filter.
class DropShadowFilter : public BitmapFilter
{
public:
    // Fill from a stream. See parser/filter_factory.cpp for the implementations.
    virtual bool read(stream* in);

    virtual ~DropShadowFilter() { return; }

    DropShadowFilter() : 
        m_distance(0.0f), m_angle(0.0f), m_color(0), m_alpha(0),
        m_blurX(0.0f), m_blurY(0.0f),  m_strength(0.0f), m_quality(0),
        m_inner(false), m_knockout(false), m_hideObject(false)
    { return; }

    DropShadowFilter(float distance, float angle, uint32_t color,
        uint8_t alpha, float blurX, float blurY, float strength,
        uint8_t quality, bool inner, bool knockout, bool hideObject) :
        m_distance(distance), m_angle(angle), m_color(color),
        m_alpha(alpha), m_blurX(blurX), m_blurY(blurY), m_strength(strength),
        m_quality(quality), m_inner(inner), m_knockout(knockout),
        m_hideObject(hideObject)
    { return; }

protected:
    float m_distance; // Distance of the filter in pixels.
    float m_angle; // Angle of the filter.
    uint32_t m_color; // RGB color.
    uint8_t m_alpha; // Alpha strength, as a percentage(?)
    float m_blurX; // horizontal blur
    float m_blurY; // vertical blur
    float m_strength; // How strong is the filter.
    uint8_t m_quality; // How many times to apply the filter.
    bool m_inner; // Is this an inner shadow?
    bool m_knockout; // If true, render only the filter effect.
    bool m_hideObject; // Does this hide the object?
};

} // Namespace gnash

#endif // GNASH_DROPSHADOWFILTER_H
