// styles.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Fill and line style types.


#include "styles.h"
#include "impl.h"
#include "log.h"
#include "render.h"
#include "stream.h"
#include "movie_definition.h"

namespace gnash {
//
// gradient_record
//

gradient_record::gradient_record()
    :
    m_ratio(0)
{
}


void
gradient_record::read(stream* in, int tag_type)
{
    m_ratio = in->read_u8();
    m_color.read(in, tag_type);
}


//
// fill_style
//


fill_style::fill_style()
    :
    m_type(0),
    m_gradient_bitmap_info(0),
    m_bitmap_character(0)
{
    assert(m_gradients.size() == 0);
}


fill_style::~fill_style()
{
}

void
fill_style::read(stream* in, int tag_type, movie_definition* md)
{
    m_type = in->read_u8();

    IF_VERBOSE_PARSE(log_msg("  fill_style read type = 0x%X\n", m_type));

    if (m_type == 0x00) {
        // 0x00: solid fill
        if (tag_type <= 22) {
            m_color.read_rgb(in);
        } else {
            m_color.read_rgba(in);
        }

        IF_VERBOSE_PARSE(log_msg("  color: ");
                         m_color.print());
    } else if (m_type == 0x10 || m_type == 0x12) {
        // 0x10: linear gradient fill
        // 0x12: radial gradient fill

        matrix	input_matrix;
        input_matrix.read(in);

        if (m_type == 0x10) {
            m_gradient_matrix.set_identity();
            m_gradient_matrix.concatenate_translation(128.f, 0.f);
            m_gradient_matrix.concatenate_scale(1.0f / 128.0f);
        } else {
            m_gradient_matrix.set_identity();
            m_gradient_matrix.concatenate_translation(32.f, 32.f);
            m_gradient_matrix.concatenate_scale(1.0f / 512.0f);
        }


        matrix	m;
        m.set_inverse(input_matrix);
        m_gradient_matrix.concatenate(m);
				
        // GRADIENT
        int	num_gradients = in->read_u8();
        if (num_gradients < 1 || num_gradients > 8) {
            fprintf(stderr, "WARNING: %s (%d): %d read bad gradient value!\n",
                    __PRETTY_FUNCTION__, __LINE__,
                    num_gradients);
        }			
        m_gradients.resize(num_gradients);
        for (int i = 0; i < num_gradients; i++)	{
            m_gradients[i].read(in, tag_type);
        }

        IF_VERBOSE_PARSE(log_msg("  gradients: num_gradients = %d\n", num_gradients));

        // @@ hack.
        if (num_gradients > 0) {
            m_color = m_gradients[0].m_color;
        }

        if (md->get_create_bitmaps() == DO_LOAD_BITMAPS) {
            m_gradient_bitmap_info = create_gradient_bitmap();
        } else {
            m_gradient_bitmap_info = render::create_bitmap_info_empty();
        }

        // Make sure our movie_def_impl knows about this bitmap.
        md->add_bitmap_info(m_gradient_bitmap_info.get_ptr());
    } else if (m_type == 0x40 || m_type == 0x41) {
        // 0x40: tiled bitmap fill
        // 0x41: clipped bitmap fill

        int	bitmap_char_id = in->read_u16();
        IF_VERBOSE_PARSE(log_msg("  bitmap_char = %d\n", bitmap_char_id));

        // Look up the bitmap character.
        m_bitmap_character = md->get_bitmap_character(bitmap_char_id);

        matrix	m;
        m.read(in);

        // For some reason, it looks like they store the inverse of the
        // TWIPS-to-texcoords matrix.
        m_bitmap_matrix.set_inverse(m);
        IF_VERBOSE_PARSE(m_bitmap_matrix.print());
    }
}


rgba
fill_style::sample_gradient(int ratio) const
    // Return the color at the specified ratio into our gradient.
    // Ratio is in [0, 255].
{
    assert(ratio >= 0 && ratio <= 255);
    assert(m_type == 0x10 || m_type == 0x12);
    assert(m_gradients.size() > 0);

    if (ratio < m_gradients[0].m_ratio) {
        return m_gradients[0].m_color;
    }
                
		
    for (unsigned int i = 1; i < m_gradients.size(); i++) {
        if (m_gradients[i].m_ratio >= ratio) {
            const gradient_record& gr0 = m_gradients[i - 1];
            const gradient_record& gr1 = m_gradients[i];
            float	f = 0.0f;
            if (gr0.m_ratio != gr1.m_ratio)	{
                f = (ratio - gr0.m_ratio) / float(gr1.m_ratio - gr0.m_ratio);
            }

            rgba	result;
            result.set_lerp(m_gradients[i - 1].m_color, m_gradients[i].m_color, f);
            return result;
        }
    }
    return m_gradients.back().m_color;
}

gnash::bitmap_info*
fill_style::create_gradient_bitmap() const
    // Make a bitmap_info* corresponding to our gradient.
    // We can use this to set the gradient fill style.
{
    assert(m_type == 0x10 || m_type == 0x12);

    image::rgba*	im = NULL;

    if (m_type == 0x10) {
        // Linear gradient.
        im = image::create_rgba(256, 1);

        for (int i = 0; i < im->m_width; i++) {
            rgba	sample = sample_gradient(i);
            im->set_pixel(i, 0, sample.m_r, sample.m_g, sample.m_b, sample.m_a);
        }
    } else if (m_type == 0x12) {
        // Radial gradient.
        im = image::create_rgba(64, 64);

        for (int j = 0; j < im->m_height; j++) {
            for (int i = 0; i < im->m_width; i++) {
                float	radius = (im->m_height - 1) / 2.0f;
                float	y = (j - radius) / radius;
                float	x = (i - radius) / radius;
                int	ratio = (int) floorf(255.5f * sqrt(x * x + y * y));
                if (ratio > 255) {
                    ratio = 255;
                }
                rgba	sample = sample_gradient( ratio );
                im->set_pixel(i, j, sample.m_r, sample.m_g, sample.m_b, sample.m_a);
            }
        }
    }

    gnash::bitmap_info*	bi = gnash::render::create_bitmap_info_rgba(im);
    delete im;

    return bi;
}


void
fill_style::apply(int fill_side, float ratio) const
    // Push our style parameters into the renderer.
{
//            GNASH_REPORT_FUNCTION;
            
    UNUSED(ratio);
    if (m_type == 0x00) {
        // 0x00: solid fill
        gnash::render::fill_style_color(fill_side, m_color);
    } else if (m_type == 0x10 || m_type == 0x12) {
        // 0x10: linear gradient fill
        // 0x12: radial gradient fill

        if (m_gradient_bitmap_info == NULL) {
            // This can happen when morphing gradient styles.
            // assert(morphing???);
            // log an error?
            fill_style*	this_non_const = const_cast<fill_style*>(this);
            this_non_const->m_gradient_bitmap_info = create_gradient_bitmap();
        }

        if (m_gradient_bitmap_info != NULL) {
            gnash::render::fill_style_bitmap(
                fill_side,
                m_gradient_bitmap_info.get_ptr(),
                m_gradient_matrix,
                gnash::render_handler::WRAP_CLAMP);
        }
    } else if (m_type == 0x40 || m_type == 0x41) {
        // bitmap fill (either tiled or clipped)
        gnash::bitmap_info*	bi = NULL;
        if (m_bitmap_character != NULL)	{
            bi = m_bitmap_character->get_bitmap_info();
            if (bi != NULL)	{
                gnash::render_handler::bitmap_wrap_mode	wmode = gnash::render_handler::WRAP_REPEAT;
                if (m_type == 0x41) {
                    wmode = gnash::render_handler::WRAP_CLAMP;
                }
                gnash::render::fill_style_bitmap(
                    fill_side,
                    bi,
                    m_bitmap_matrix,
                    wmode);
            }
        }
    }
}


void
fill_style::set_lerp(const fill_style& a, const fill_style& b, float t)
    // Sets this style to a blend of a and b.  t = [0,1]
{
    assert(t >= 0 && t <= 1);

    // fill style type
    m_type = a.get_type();
    assert(m_type == b.get_type());

    // fill style color
    m_color.set_lerp(a.get_color(), b.get_color(), t);

    // fill style gradient matrix
    //
    // @@ TODO morphed gradients don't come out exactly
    // right; they shift around some.  Not sure where the
    // problem is.
    m_gradient_matrix.set_lerp(a.m_gradient_matrix, b.m_gradient_matrix, t);

    // fill style gradients
    assert(m_gradients.size() == a.m_gradients.size());
    assert(m_gradients.size() == b.m_gradients.size());
    for (unsigned int j=0; j < m_gradients.size(); j++) {
        m_gradients[j].m_ratio =
            (Uint8) frnd(
                flerp(a.m_gradients[j].m_ratio, b.m_gradients[j].m_ratio, t)
                );
        m_gradients[j].m_color.set_lerp(a.m_gradients[j].m_color, b.m_gradients[j].m_color, t);
    }
    m_gradient_bitmap_info = NULL;

    // fill style bitmap ID
    m_bitmap_character = a.m_bitmap_character;
    assert(m_bitmap_character == b.m_bitmap_character);

    // fill style bitmap matrix
    m_bitmap_matrix.set_lerp(a.m_bitmap_matrix, b.m_bitmap_matrix, t);
}


//
// line_style
//

	
line_style::line_style()
    :
    m_width(0)
{
}


void
line_style::read(stream* in, int tag_type)
{
    m_width = in->read_u16();
    m_color.read(in, tag_type);
}


void
line_style::apply(float ratio) const
{
    UNUSED(ratio);
    gnash::render::line_style_color(m_color);
    gnash::render::line_style_width(m_width);
}

// end of namespace
}


// Local Variables:
// mode: C++
// End:
