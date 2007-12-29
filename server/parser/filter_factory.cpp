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

/* $Id: filter_factory.cpp,v 1.7 2007/12/29 20:15:25 strk Exp $ */

#include "filter_factory.h"
#include "BitmapFilter.h"
#include "BlurFilter.h"
#include "DropShadowFilter.h"
#include "GlowFilter.h"
#include "BevelFilter.h"
#include "GradientGlowFilter.h"
#include "ConvolutionFilter.h"
#include "ColorMatrixFilter.h"
#include "GradientBevelFilter.h"

#include "log.h"
#include "stream.h"

namespace gnash {

typedef enum
{
    DROP_SHADOW = 0,
    BLUR = 1,
    GLOW = 2,
    BEVEL = 3,
    GRADIENT_GLOW = 4,
    CONVOLUTION = 5,
    COLOR_MATRIX = 6,
    GRADIENT_BEVEL = 7
} filter_types;

int const filter_factory::read(stream& in, 
    bool read_multiple, Filters* store)
{
    int count = 1;

    if (read_multiple)
    {
        in.ensureBytes(1);
        count = static_cast<int> (in.read_u8());
    }

    for (int i = 0; i < count; ++i)
    {
        BitmapFilter *the_filter = NULL;

        filter_types filter_type = static_cast<filter_types> (in.read_u8());

        switch (filter_type)
        {
            case DROP_SHADOW:
                the_filter = new DropShadowFilter;
                break;
            case BLUR:
                the_filter = new BlurFilter; 
                break;
            case GLOW:
                the_filter = new GlowFilter;
                break;
            case BEVEL:
                the_filter = new BevelFilter;
                break;
            case GRADIENT_GLOW:
                the_filter = new GradientGlowFilter;
                break;
            case CONVOLUTION:
                the_filter = new ConvolutionFilter;
                break;
            case COLOR_MATRIX:
                the_filter = new ColorMatrixFilter;
                break;
            case GRADIENT_BEVEL:
                the_filter = new GradientBevelFilter;
                break;
            default:
                IF_VERBOSE_MALFORMED_SWF(
                log_swferror(_("Invalid filter type %d."), filter_type);
                );
                return i; // We're already broken, no need to cause more pain.
        }

        // Protect against exceptions and such by storing before we read.
        Filter p(the_filter);
        if (!p->read(in))
        {
            IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("Filter %d could not read."), filter_type);
            );
            return i; // We're already broken.
        }
        store->push_back(p);
    }

    return count;
}

bool DropShadowFilter::read(stream& in)
{
    m_color = in.read_u8() << 16 + in.read_u8() << 8 + in.read_u8();
    m_alpha = in.read_u8();

    m_blurX = in.read_fixed();
    m_blurY = in.read_fixed();

    m_angle = in.read_fixed();
    m_distance = in.read_fixed();

    m_strength = in.read_short_sfixed();

    m_inner = in.read_bit(); 
    m_knockout = in.read_bit(); 
    m_hideObject = in.read_bit(); 

    static_cast<void> (in.read_uint(5)); // Throw these away on purpose.

    return true;
}

bool BlurFilter::read(stream& in)
{
    m_blurX = in.read_ufixed();
    m_blurY = in.read_ufixed();

    m_quality = static_cast<boost::uint8_t> (in.read_uint(5));

    static_cast<void> (in.read_uint(3)); // Throw these away.

    return true;
}

bool GlowFilter::read(stream& in)
{
    m_color = in.read_u8() << 16 + in.read_u8() << 8 + in.read_u8();
    m_alpha = in.read_u8();

    m_blurX = in.read_fixed();
    m_blurY = in.read_fixed();

    m_strength = in.read_short_sfixed();

    m_inner = in.read_bit(); 
    m_knockout = in.read_bit(); 

    static_cast<void> (in.read_uint(6)); // Throw these away.

    return true;
}

bool BevelFilter::read(stream& in)
{
    // TODO: It is possible that the order of these two should be reversed.
    // highlight might come first. Find out for sure and then fix and remove
    // this comment.
    m_shadowColor = in.read_u8() << 16 + in.read_u8() << 8 + in.read_u8();
    m_shadowAlpha = in.read_u8();

    m_highlightColor = in.read_u8() << 16 + in.read_u8() << 8 + in.read_u8();
    m_highlightAlpha = in.read_u8();

    m_blurX = in.read_fixed();
    m_blurY = in.read_fixed();

    m_angle = in.read_fixed();
    m_distance = in.read_fixed();
    
    m_strength = in.read_short_sfixed();

    bool inner_shadow = in.read_bit(); 
    m_knockout = in.read_bit(); 
    in.read_bit();  // reserved ?
    bool on_top = in.read_bit(); 

    // Set the bevel type. top and inner is full, top is outer, inner is inner
    m_type = on_top ? (inner_shadow ? FULL_BEVEL : OUTER_BEVEL) : INNER_BEVEL;
    
    static_cast<void> (in.read_uint(4)); // Throw these away.

    return true;
}

bool GradientGlowFilter::read(stream& in)
{
    boost::uint8_t count = in.read_u8(); // How many colorings.

    m_colors.reserve(count);
    m_alphas.reserve(count);
    m_ratios.reserve(count);
    for (int i = 0; i < count; ++i)
    {
        m_colors.push_back(in.read_u8() << 16 + in.read_u8() << 8 + in.read_u8());
        m_alphas.push_back(in.read_u8());
    }

    for (int i = 0; i < count; ++i)
    {
        m_ratios.push_back(in.read_u8());
    }

    m_blurX = in.read_fixed();
    m_blurY = in.read_fixed();

    m_angle = in.read_fixed();
    m_distance = in.read_fixed();

    m_strength = in.read_short_sfixed();

    bool inner = in.read_bit();
    m_knockout = in.read_bit();
    in.read_bit(); // reserved ?
    bool outer = in.read_bit(); 

    m_type = outer ? (inner ? FULL_GLOW : OUTER_GLOW) : INNER_GLOW;

    m_quality = static_cast<boost::uint8_t> (in.read_uint(4));

    return true;
}

bool ConvolutionFilter::read(stream& in)
{
    m_matrixX = in.read_u8();
    m_matrixY = in.read_u8();

    m_divisor = in.read_float();
    m_bias = in.read_float();

    m_matrix.reserve(m_matrixX * m_matrixY);
    for (int i = 0; i < m_matrixX * m_matrixY; ++i)
    {
        m_matrix.push_back(in.read_float());
    }

    m_color = in.read_u8() << 16 + in.read_u8() << 8 + in.read_u8();
    m_alpha = in.read_u8();

    static_cast<void> (in.read_uint(6)); // Throw away.

    m_clamp = in.read_bit(); 
    m_preserveAlpha = in.read_bit(); 

    return true;
}

bool ColorMatrixFilter::read(stream& in)
{
    m_matrix.reserve(20);
    for (int i = 0; i < 20; ++i)
    {
        m_matrix.push_back(in.read_float());
    }

    return true;
}

bool GradientBevelFilter::read(stream& in)
{
    boost::uint8_t count = in.read_u8(); // How many colorings.

    m_colors.reserve(count);
    m_alphas.reserve(count);
    m_ratios.reserve(count);
    for (int i = 0; i < count; ++i)
    {
        m_colors.push_back(in.read_u8() << 16 + in.read_u8() << 8 + in.read_u8());
        m_alphas.push_back(in.read_u8());
    }

    for (int i = 0; i < count; ++i)
    {
        m_ratios.push_back(in.read_u8());
    }

    m_blurX = in.read_fixed();
    m_blurY = in.read_fixed();

    m_angle = in.read_fixed();
    m_distance = in.read_fixed();

    m_strength = in.read_short_sfixed();

    bool inner = in.read_bit();
    m_knockout = in.read_bit();
    in.read_bit(); // reserved ?
    bool outer = in.read_bit();

    m_type = outer ? (inner ? FULL_BEVEL : OUTER_BEVEL) : INNER_BEVEL;

    m_quality = static_cast<boost::uint8_t> (in.read_uint(4));

    return true;
}

} // gnash namespace
