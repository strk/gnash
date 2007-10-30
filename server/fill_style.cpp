// fill_style.cpp:  Graphical region filling styles, for Gnash.
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
//

// Based on work of Thatcher Ulrich <tu@tulrich.com> 2003

#include "fill_style.h"
#include "impl.h"
#include "log.h"
#include "render.h"
#include "stream.h"
#include "movie_definition.h"
#include "swf.h"
#include "GnashException.h"

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
    in->ensureBytes(1);
    m_ratio = in->read_u8();
    m_color.read(in, tag_type);
}


//
// fill_style
//


fill_style::fill_style()
    :
    m_type(SWF::FILL_SOLID),
    m_color(), // FF.FF.FF.FF
    m_gradient_bitmap_info(0),
    m_bitmap_character(0)
{
    assert(m_gradients.size() == 0);
}


fill_style::~fill_style()
{
}

void
fill_style::read(stream* in, int tag_type, movie_definition* md,
	fill_style *pOther)
{
	const bool is_morph = pOther != NULL;

    in->ensureBytes(1);
    m_type = in->read_u8();
	if (is_morph)
		pOther->m_type = m_type;

		IF_VERBOSE_PARSE
		(
    log_parse("  fill_style read type = 0x%X", m_type);
    		);

    if (m_type == SWF::FILL_SOLID)
    {
        // 0x00: solid fill
        if ( tag_type == SWF::DEFINESHAPE3 || tag_type == SWF::DEFINESHAPE4
			|| tag_type == SWF::DEFINESHAPE4_ || is_morph)
        {
            m_color.read_rgba(in);
			if (is_morph)
				pOther->m_color.read_rgba(in);
        }
        else 
        {
            // For DefineMorphShape tags we should use morph_fill_style 
            assert( tag_type == SWF::DEFINESHAPE
		|| tag_type == SWF::DEFINESHAPE2 );
            m_color.read_rgb(in);
        }

		IF_VERBOSE_PARSE
		(
        log_parse("  color: %s", m_color.toString().c_str());
		);
    }
    else if (m_type == SWF::FILL_LINEAR_GRADIENT
            || m_type == SWF::FILL_RADIAL_GRADIENT
			|| m_type == SWF::FILL_FOCAL_GRADIENT)
    {
        // 0x10: linear gradient fill
        // 0x12: radial gradient fill
        // 0x13: focal gradient fill

        matrix	input_matrix;
        input_matrix.read(in);

        // shouldn't this be in initializer's list ?
        m_gradient_matrix.set_identity();
        if (m_type == SWF::FILL_LINEAR_GRADIENT)
        {
            m_gradient_matrix.concatenate_translation(128.f, 0.f);
            m_gradient_matrix.concatenate_scale(1.0f / 128.0f);
        }
        else // FILL_RADIAL_GRADIENT or FILL_FOCAL_GRADIENT
        {
            m_gradient_matrix.concatenate_translation(32.f, 32.f);
            m_gradient_matrix.concatenate_scale(1.0f / 512.0f);
        }

        matrix	m;
        m.set_inverse(input_matrix);

		if (is_morph)
		{
			pOther->m_gradient_matrix = m_gradient_matrix;
		}
        m_gradient_matrix.concatenate(m);
		
		if (is_morph)
		{
			input_matrix.read(in);
			m.set_inverse(input_matrix);
			pOther->m_gradient_matrix.concatenate(m);
		}
		
        // GRADIENT
        in->ensureBytes(1);
		// num_gradients is not 8 bits, it is only the last 4.
		// at the moment, the first four are unused, so we may
		// mask, but this needs to be changed.
        uint8_t num_gradients = in->read_u8() & 15;
        if ( ! num_gradients )
	{
		IF_VERBOSE_MALFORMED_SWF(
			log_swferror(_("num gradients 0"));
		);
		return;
	}

        if ( num_gradients > 8 + ((tag_type == SWF::DEFINESHAPE4 ||
			tag_type == SWF::DEFINESHAPE4_) ? 7 : 0))
        {
            // see: http://sswf.sourceforge.net/SWFalexref.html#swf_gradient
            log_error(_("Unexpected num gradients (%d), expected 1 to 8"),
                    num_gradients);
        }			

		if (is_morph)
			pOther->m_gradients.resize(num_gradients);

        m_gradients.resize(num_gradients);
   	    for (int i = 0; i < num_gradients; i++)	{
       	    m_gradients[i].read(in, tag_type);
			if (is_morph)
				pOther->m_gradients[i].read(in, tag_type);
        }

		// A focal gradient also has a focal point.
		if (m_type == SWF::FILL_FOCAL_GRADIENT)
		{
			m_focal_point = in->read_short_sfixed();
			if (m_focal_point < -1.0f)
				m_focal_point = -1.0f;
			else if (m_focal_point > 1.0f)
				m_focal_point = 1.0f;
		}

		if (is_morph)
			pOther->m_focal_point = m_focal_point;

		IF_VERBOSE_PARSE
		(
        log_parse("  gradients: num_gradients = %d", num_gradients);
		);

        // @@ hack.
        if (num_gradients > 0) {
            m_color = m_gradients[0].m_color;
			if (is_morph)
				pOther->m_color = pOther->m_gradients[0].m_color;
        }

        if (md->get_create_bitmaps() == DO_LOAD_BITMAPS) {
            m_gradient_bitmap_info = create_gradient_bitmap();
			if (is_morph)
			{
				pOther->m_gradient_bitmap_info = 
					pOther->create_gradient_bitmap();
				md->add_bitmap_info(pOther->m_gradient_bitmap_info.get());
			}
        // Make sure our movie_def_impl knows about this bitmap.
        md->add_bitmap_info(m_gradient_bitmap_info.get());
        }
    }
    else if (m_type == SWF::FILL_TILED_BITMAP
          || m_type == SWF::FILL_CLIPPED_BITMAP
          || m_type == SWF::FILL_TILED_BITMAP_HARD
          || m_type == SWF::FILL_CLIPPED_BITMAP_HARD)
    {
        // 0x40: tiled bitmap fill
        // 0x41: clipped bitmap fill
        // 0x42: tiled bitmap fill with hard edges
        // 0x43: clipped bitmap fill with hard edges

        in->ensureBytes(2);
        int	bitmap_char_id = in->read_u16();
	IF_VERBOSE_PARSE
	(
        	log_parse("  bitmap_char = %d", bitmap_char_id);
	);

        // Look up the bitmap character.
        m_bitmap_character = md->get_bitmap_character_def(bitmap_char_id);
	IF_VERBOSE_MALFORMED_SWF(
	if ( m_bitmap_character == NULL )
	{
		static bool warned_about_invalid_char=false;
		if ( ! warned_about_invalid_char )
		{
			log_swferror(_("Bitmap fill specifies '%d' as associated"
				" bitmap character id,"
				" but that character is not found"
				" in the Characters Dictionary."
				" It seems common to find such "
				" malformed SWF, so we'll only warn once "
				"about this."),
				bitmap_char_id);
			warned_about_invalid_char=true;
		}
	}
	);

        matrix	m;
        m.read(in);

        // For some reason, it looks like they store the inverse of the
        // TWIPS-to-texcoords matrix.
        m_bitmap_matrix.set_inverse(m);

		if (is_morph)
		{
			pOther->m_bitmap_character = m_bitmap_character;
			m.read(in);
			pOther->m_bitmap_matrix.set_inverse(m);
		}
        IF_VERBOSE_PARSE(
            m_bitmap_matrix.print();
        );
    }
    else
    {
	stringstream ss;
	ss << "Unknown fill style type " << m_type;
        //log_unimpl("Unsupported fill style type: 0x%X", m_type);
        // This is a fatal error, we'll be leaving the stream
        // read pointer in an unknown position.
        throw ParserException(ss.str()); // "Unsupported fill style (Malformed SWF?)");
    }
}


bitmap_info* 
fill_style::get_bitmap_info() const 
{    
  assert(m_type != SWF::FILL_SOLID);
  
  if (m_type == SWF::FILL_TILED_BITMAP
   || m_type == SWF::FILL_CLIPPED_BITMAP
   || m_type == SWF::FILL_TILED_BITMAP_HARD
   || m_type == SWF::FILL_CLIPPED_BITMAP_HARD) {

   if (m_bitmap_character!=NULL)
     return m_bitmap_character->get_bitmap_info();
   else
     return NULL;
   
  } else
  if (m_type == SWF::FILL_LINEAR_GRADIENT
   || m_type == SWF::FILL_RADIAL_GRADIENT) {
   
   return need_gradient_bitmap();
   
  } else {
    log_error(_("Unknown fill style %d"), m_type);
    abort();
  }  
}

matrix
fill_style::get_bitmap_matrix() const 
{
  assert(m_type != SWF::FILL_SOLID);
  return m_bitmap_matrix;
}

matrix
fill_style::get_gradient_matrix() const 
{
  // TODO: Why do we separate bitmap and gradient matrices? 
  return m_gradient_matrix;
}

rgba
fill_style::sample_gradient(uint8_t ratio) const
{
	assert(m_type == SWF::FILL_LINEAR_GRADIENT
		|| m_type == SWF::FILL_RADIAL_GRADIENT
		|| m_type == SWF::FILL_FOCAL_GRADIENT);

	assert(m_gradients.size());

	// By specs, first gradient should *always* be 0, 
	// anyway a malformed SWF could break this,
	// so we cannot rely on that information...
	if (ratio < m_gradients[0].m_ratio)
	{
		IF_VERBOSE_MALFORMED_SWF(
			static bool warned=false;
			if ( ! warned ) {
			log_swferror(
				_("First gradient in a fill_style "
				"have position==%d (expected 0)."
				" This seems to be common, so will"
				" warn only once."),
			        m_gradients[0].m_ratio);
			warned=true;
			}
		);
		return m_gradients[0].m_color;
	}

	if ( ratio >= m_gradients.back().m_ratio )
	{
		return m_gradients.back().m_color;
	}
		
	for (size_t i = 1, n = m_gradients.size(); i < n; ++i)
	{
		const gradient_record& gr1 = m_gradients[i];
		if (gr1.m_ratio < ratio) continue;

		const gradient_record& gr0 = m_gradients[i - 1];
		if (gr0.m_ratio > ratio) continue;

		float f = 0.0f;

		if ( gr0.m_ratio != gr1.m_ratio )
		{
			f = (ratio - gr0.m_ratio) / float(gr1.m_ratio - gr0.m_ratio);
		}
		else
		{
			// Ratios are equal IFF first and second gradient_record
			// have the same ratio. This would be a malformed SWF.
			IF_VERBOSE_MALFORMED_SWF(
				log_swferror(
					_("two gradients in a fill_style "
					"have the same position/ratio: %d"),
					gr0.m_ratio);
			);
		}

		rgba	result;
		result.set_lerp(gr0.m_color, gr1.m_color, f);
		return result;
	}

	// Assuming gradients are ordered by m_ratio? see start comment
	return m_gradients.back().m_color;
}

gnash::bitmap_info*
fill_style::create_gradient_bitmap() const
{
    assert(m_type == SWF::FILL_LINEAR_GRADIENT
        || m_type == SWF::FILL_RADIAL_GRADIENT
		|| m_type == SWF::FILL_FOCAL_GRADIENT);

    image::rgba*	im = NULL;

    if (m_type == SWF::FILL_LINEAR_GRADIENT)
    {
        // Linear gradient.
        im = image::create_rgba(256, 1);

        for (size_t i = 0; i < im->width(); i++)
	{
            rgba	sample = sample_gradient(i);
            im->set_pixel(i, 0, sample.m_r, sample.m_g, sample.m_b, sample.m_a);
        }
    }
    else if (m_type == SWF::FILL_RADIAL_GRADIENT)
    {
        // Radial gradient.
        im = image::create_rgba(64, 64);

        for (size_t j = 0; j < im->height(); j++) {
            for (size_t i = 0; i < im->width(); i++) {
                float	radius = (im->height() - 1) / 2.0f;
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
	else if (m_type == SWF::FILL_FOCAL_GRADIENT)
	{
		// Focal gradient.
		im = image::create_rgba(64, 64);

		for (size_t j = 0; j < im->height(); j++)
		{
			for (size_t i = 0; i < im->width(); i++)
			{
				float radiusy = (im->height() - 1) / 2.0f;
				float radiusx = radiusy + abs(radiusy * m_focal_point);
				float y = (j - radiusy) / radiusy;
				float x = (i - radiusx) / radiusx;
				int ratio = (int) floorf(255.5f * sqrt(x*x + y*y));
				if (ratio > 255)
				{
					ratio = 255;
				}
				rgba sample = sample_gradient(ratio);
				im->set_pixel(i, j, sample.m_r, sample.m_g, sample.m_b, sample.m_a);
			}
		}
	}
		
    gnash::bitmap_info*	bi = gnash::render::create_bitmap_info_rgba(im);
    delete im;

    return bi;
}


gnash::bitmap_info*
fill_style::need_gradient_bitmap() const 
{

  if (m_gradient_bitmap_info==NULL) {
    fill_style*	this_non_const = const_cast<fill_style*>(this);
    this_non_const->m_gradient_bitmap_info = create_gradient_bitmap();
  }
  
  return m_gradient_bitmap_info.get();

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
    for (size_t j=0, nj=m_gradients.size(); j<nj; ++j)
    {
        m_gradients[j].m_ratio =
            (uint8_t) frnd(
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


int 
fill_style::get_color_stop_count() const 
{
  return m_gradients.size();
}

const gradient_record& 
fill_style::get_color_stop(int index) const
{
  return m_gradients[index];
}

fill_style::fill_style(bitmap_character_def* bitmap)
{
	m_bitmap_character = bitmap;
	m_type = SWF::FILL_CLIPPED_BITMAP;
}

void
fill_style::setSolid(const rgba& color)
{
	m_type = SWF::FILL_SOLID;
	m_color = color;
}


#ifdef GNASH_USE_GC
void
fill_style::markReachableResources() const
{
	if ( m_gradient_bitmap_info ) m_gradient_bitmap_info->setReachable();
	if ( m_bitmap_character ) m_bitmap_character->setReachable();
}
#endif // GNASH_USE_GC

} // end of namespace


// Local Variables:
// mode: C++
// End:
