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

/* $Id: filters.cpp,v 1.1 2007/08/24 05:44:18 cmusick Exp $ */

#include "filters.h"

#include "cxform.h"
#include "log.h"
#include "stream.h"

namespace gnash {
namespace effect_filters {

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

class drop_shadow_filter : public effect_filter
{
public:
    friend class filter_factory;

    bool read(stream* in);

private:
    drop_shadow_filter() : 
        m_rgba(), m_blur_horizontal(), m_blur_vertical(),
        m_radian_angle(), m_distance(), m_strength(),
        m_inner_shadow(), m_knock_out(), m_composite_source()
   { return; }

    cxform m_rgba;

    float m_blur_horizontal;
    float m_blur_vertical;

    float m_radian_angle;
    float m_distance;

    float m_strength;

    bool m_inner_shadow;
    bool m_knock_out;
    bool m_composite_source;
};

class blur_filter : public effect_filter
{
public:
    friend class filter_factory;

    bool read(stream* in);

private:
    blur_filter() :
        m_blur_horizontal(), m_blur_vertical(), m_passes()
    { return; }

    float m_blur_horizontal;
    float m_blur_vertical;

    uint8_t m_passes;
};

class glow_filter : public effect_filter
{
public:
    friend class filter_factory;

    bool read(stream* in);

private:
    glow_filter() :
        m_rgba(), m_blur_horizontal(), m_blur_vertical(), m_strength(),
        m_inner_shadow(), m_knock_out(), m_composite_source()
    { return; }

    cxform m_rgba;

    float m_blur_horizontal;
    float m_blur_vertical;

    float m_strength;

    bool m_inner_shadow;
    bool m_knock_out;
    bool m_composite_source;
};

class bevel_filter : public effect_filter
{
public:
    friend class filter_factory;

    bool read(stream* in);

private:
    bevel_filter() :
        m_rgba(), m_highlight_rgba(), m_blur_horizontal(), m_blur_vertical(),
        m_radian_angle(), m_distance(), m_strength(), m_inner_shadow(),
        m_composite_source(), m_on_top()
    { return; }

    cxform m_rgba;
    cxform m_highlight_rgba;

    float m_blur_horizontal;
    float m_blur_vertical;

    float m_radian_angle;
    float m_distance;

    float m_strength;

    bool m_inner_shadow;
    bool m_knock_out;
    bool m_composite_source;
    bool m_on_top; // Overlay.
};

class gradient_glow_filter : public effect_filter
{
public:
    friend class filter_factory;

    bool read(stream* in);

private:
    gradient_glow_filter() :
        m_count(), m_rgba_array(), m_positions(), m_blur_horizontal(),
        m_blur_vertical(), m_radian_angle(), m_distance(), m_strength(),
        m_inner_shadow(), m_knock_out(), m_composite_source(),
        m_passes()
    { return; }

    uint8_t m_count;
    std::vector<boost::shared_ptr<cxform> > m_rgba_array;
    std::vector<uint8_t> m_positions;

    float m_blur_horizontal;
    float m_blur_vertical;

    float m_radian_angle;
    float m_distance;

    float m_strength;

    bool m_inner_shadow;
    bool m_knock_out;
    bool m_composite_source;

    uint8_t m_passes;
};

class convolution_filter : public effect_filter
{
public:
    friend class filter_factory;

    bool read(stream* in);

private:
    convolution_filter() :
         m_columns(), m_rows(), m_divisor(), m_bias(), m_weights(),
         m_default_rgba(), m_clamp(), m_reserve_alpha()
    { return; }

    uint8_t m_columns;
    uint8_t m_rows;

    float m_divisor;
    float m_bias;

    std::vector<float> m_weights;

    cxform m_default_rgba;

    bool m_clamp;
    bool m_reserve_alpha;
};

class color_matrix_filter : public effect_filter
{
public:
    friend class filter_factory;

    bool read(stream* in);

private:
    color_matrix_filter() :
        m_matrix()
    { return; }

    float m_matrix[25]; // Fixed size for color_matrix filters.
};

class gradient_bevel_filter : public effect_filter
{
public:
    friend class filter_factory;

    bool read(stream* in);

private:
    gradient_bevel_filter() :
        m_count(), m_rgba_array(), m_positions(), m_blur_horizontal(),
        m_blur_vertical(), m_radian_angle(), m_distance(), m_strength(),
        m_inner_shadow(), m_knock_out(), m_composite_source(),
        m_passes()
    { return; }

    uint8_t m_count;
    std::vector<boost::shared_ptr<cxform> > m_rgba_array;
    std::vector<uint8_t> m_positions;

    float m_blur_horizontal;
    float m_blur_vertical;

    float m_radian_angle;
    float m_distance;

    float m_strength;

    bool m_inner_shadow;
    bool m_knock_out;
    bool m_composite_source;

    uint8_t m_passes;
};

int const filter_factory::read(stream* in, int /* movie_version */,
    bool read_multiple, effect_filters_vec *store)
{
    int count = 1;

    if (read_multiple)
    {
        in->ensureBytes(1);
        count = static_cast<int> (in->read_u8());
    }

    for (int i = 0; i < count; ++i)
    {
        effect_filter *the_filter = NULL;

        uint8_t filter_type = in->read_u8();

        switch (filter_type)
        {
            case DROP_SHADOW:
                the_filter = new drop_shadow_filter;
                break;
            case BLUR:
                the_filter = new blur_filter; 
                break;
            case GLOW:
                the_filter = new glow_filter;
                break;
            case BEVEL:
                the_filter = new bevel_filter;
                break;
            case GRADIENT_GLOW:
                the_filter = new gradient_glow_filter;
                break;
            case CONVOLUTION:
                the_filter = new convolution_filter;
                break;
            case COLOR_MATRIX:
                the_filter = new color_matrix_filter;
                break;
            case GRADIENT_BEVEL:
                the_filter = new gradient_bevel_filter;
                break;
            default:
                IF_VERBOSE_MALFORMED_SWF(
                log_swferror(_("Invalid filter type %d."), filter_type);
                );
                return i; // We're already broken, no need to cause more pain.
        }

        // Protect against exceptions and such by storing before we read.
        effect_filter_ptr p(the_filter);
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

bool drop_shadow_filter::read(stream* in)
{
    m_rgba.read_rgba(in);

    m_blur_horizontal = in->read_fixed();
    m_blur_vertical = in->read_fixed();

    m_radian_angle = in->read_fixed();
    m_distance = in->read_fixed();

    m_strength = in->read_short_fixed();

    m_inner_shadow = in->read_uint(1) ? true : false;
    m_knock_out = in->read_uint(1) ? true : false;
    m_composite_source = in->read_uint(1) ? true : false;

    static_cast<void> (in->read_uint(5)); // Throw these away on purpose.

    return true;
}

bool blur_filter::read(stream* in)
{
    m_blur_horizontal = in->read_fixed();
    m_blur_vertical = in->read_fixed();

    m_passes = static_cast<uint8_t> (in->read_uint(5));

    static_cast<void> (in->read_uint(3)); // Throw these away.

    return true;
}

bool glow_filter::read(stream* in)
{
    m_rgba.read_rgba(in);

    m_blur_horizontal = in->read_fixed();
    m_blur_vertical = in->read_fixed();

    m_strength = in->read_short_fixed();

    m_inner_shadow = in->read_uint(1) ? true : false;
    m_knock_out = in->read_uint(1) ? true : false;
    m_composite_source = in->read_uint(1) ? true : false;

    static_cast<void> (in->read_uint(5)); // Throw these away.

    return true;
}

bool bevel_filter::read(stream* in)
{
    m_rgba.read_rgba(in);
    m_highlight_rgba.read_rgba(in);

    m_blur_horizontal = in->read_fixed();
    m_blur_vertical = in->read_fixed();

    m_radian_angle = in->read_fixed();
    m_distance = in->read_fixed();
    
    m_strength = in->read_short_fixed();

    m_inner_shadow = in->read_uint(1) ? true : false;
    m_knock_out = in->read_uint(1) ? true : false;
    m_composite_source = in->read_uint(1) ? true : false;
    m_on_top = in->read_uint(1) ? true : false;

    static_cast<void> (in->read_uint(4)); // Throw these away.

    return true;
}

bool gradient_glow_filter::read(stream* in)
{
    m_count = in->read_u8();

    m_rgba_array.reserve(m_count);
    for (int i = 0; i < m_count; ++i)
    {
        boost::shared_ptr<cxform> rgba(new cxform);
        rgba->read_rgba(in);
        m_rgba_array.push_back(rgba);
    }

    m_positions.reserve(m_count);
    for (int i = 0; i < m_count; ++i)
    {
        m_positions.push_back(in->read_u8());
    }

    m_blur_horizontal = in->read_fixed();
    m_blur_vertical = in->read_fixed();

    m_radian_angle = in->read_fixed();
    m_distance = in->read_fixed();

    m_strength = in->read_short_fixed();

    m_inner_shadow = in->read_uint(1) ? true : false;
    m_knock_out = in->read_uint(1) ? true : false;
    m_composite_source = in->read_uint(1) ? true : false;

    static_cast<void> (in->read_uint(1)); // Throw away.

    m_passes = static_cast<uint8_t> (in->read_uint(4));

    return true;
}

// TODO: Be sure that read_fixed is doing the right thing with
// values that are stored as floats rather than as fixed point
// values! If they are not, define such a function in the
// stream and remove this #define.
#define read_float read_fixed

bool convolution_filter::read(stream* in)
{
    m_columns = in->read_u8();
    m_rows = in->read_u8();

    m_divisor = in->read_float();
    m_bias = in->read_float();

    m_weights.reserve(m_columns * m_rows);
    for (int i = 0; i < m_columns * m_rows; ++i)
    {
        m_weights.push_back(in->read_float());
    }

    m_default_rgba.read_rgba(in);

    static_cast<void> (in->read_uint(6)); // Throw away.

    m_clamp = in->read_uint(1) ? true : false;
    m_reserve_alpha = in->read_uint(1) ? true : false;

    return true;
}

bool color_matrix_filter::read(stream* in)
{
    for (int i = 0; i < 20; ++i)
    {
        m_matrix[i] = in->read_float();
    }

    m_matrix[20] = m_matrix[21] = m_matrix[22] = m_matrix[23] = 0.0;
    m_matrix[24] = 1.0;

    return true;
}

#undef read_float

// This is the same code as gradient_glow_filter,
// however, there is no intrinsic reason this must be the case,
// so these two functions are separated.
bool gradient_bevel_filter::read(stream* in)
{
    m_count = in->read_u8();

    m_rgba_array.reserve(m_count);
    for (int i = 0; i < m_count; ++i)
    {
        boost::shared_ptr<cxform> rgba(new cxform);
        rgba->read_rgba(in);
        m_rgba_array.push_back(rgba);
    }

    m_positions.reserve(m_count);
    for (int i = 0; i < m_count; ++i)
    {
        m_positions.push_back(in->read_u8());
    }

    m_blur_horizontal = in->read_fixed();
    m_blur_vertical = in->read_fixed();

    m_radian_angle = in->read_fixed();
    m_distance = in->read_fixed();

    m_strength = in->read_short_fixed();

    m_inner_shadow = in->read_uint(1) ? true : false;
    m_knock_out = in->read_uint(1) ? true : false;
    m_composite_source = in->read_uint(1) ? true : false;

    static_cast<void> (in->read_uint(1)); // Throw away.

    m_passes = static_cast<uint8_t> (in->read_uint(4));

    return true;
}

} // effect_filters namespace
} // gnash namespace
