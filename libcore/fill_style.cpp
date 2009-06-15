// fill_style.cpp:  Graphical region filling styles, for Gnash.
// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "smart_ptr.h" // GNASH_USE_GC
#include "fill_style.h"
#include "log.h"
#include "render.h"
#include "SWFStream.h"
#include "movie_definition.h"
#include "SWF.h"
#include "GnashException.h"
#include "GnashNumeric.h"
#include <cmath> // sqrt, floor
#include <iostream> // for output operator

namespace gnash {

//
// gradient_record
//

void
gradient_record::read(SWFStream& in, SWF::TagType tag)
{
    in.ensureBytes(1);
    m_ratio = in.read_u8();
    m_color.read(in, tag);
}

//
// fill_style
//
fill_style::fill_style()
    :
    _bitmapInfo(0),
    m_color(), // FF.FF.FF.FF
    m_spread_mode(SWF::GRADIENT_SPREAD_PAD),
    m_interpolation(SWF::GRADIENT_INTERPOL_NORMAL),
    m_type(SWF::FILL_SOLID),
    _bitmapSmoothingPolicy(BITMAP_SMOOTHING_UNSPECIFIED)
{
}


void
fill_style::read(SWFStream& in, SWF::TagType t, movie_definition& md,
    fill_style *pOther)
{
    const bool is_morph = (pOther != NULL);

    in.ensureBytes(1);
    m_type = in.read_u8();
    if (is_morph)
    {
        pOther->m_type = m_type;
    }
        
    IF_VERBOSE_PARSE(
        log_parse("  fill_style read type = 0x%X", (int)m_type);
    );

    if (m_type == SWF::FILL_SOLID)
    {
        // 0x00: solid fill
        if ( t == SWF::DEFINESHAPE3 || t == SWF::DEFINESHAPE4
            || t == SWF::DEFINESHAPE4_ || is_morph)
        {
            m_color.read_rgba(in);
            if (is_morph)   pOther->m_color.read_rgba(in);
        }
        else 
        {
            // For DefineMorphShape tags we should use morph_fill_style 
            assert( t == SWF::DEFINESHAPE ||
                    t == SWF::DEFINESHAPE2 );
            m_color.read_rgb(in);
        }

        IF_VERBOSE_PARSE
        (
            log_parse("  color: %s", m_color.toString());
        );
    }
    else if (m_type == SWF::FILL_LINEAR_GRADIENT
            || m_type == SWF::FILL_RADIAL_GRADIENT
            || m_type == SWF::FILL_FOCAL_GRADIENT)
    {
        // 0x10: linear gradient fill
        // 0x12: radial gradient fill
        // 0x13: focal gradient fill

        SWFMatrix  input_matrix;
        input_matrix.read(in);

        // shouldn't this be in initializer's list ?
        _matrix.set_identity();
        if (m_type == SWF::FILL_LINEAR_GRADIENT)
        {
            _matrix.set_translation(128, 0);
            _matrix.set_scale(1.0/128, 1.0/128);
        }
        else // FILL_RADIAL_GRADIENT or FILL_FOCAL_GRADIENT
        {
            _matrix.set_translation(32, 32);
            _matrix.set_scale(1.0/512, 1.0/512);
        }

        SWFMatrix m = input_matrix;
        m.invert();

        if (is_morph)
        {
            pOther->_matrix = _matrix;
        }
        _matrix.concatenate(m);
        
        if (is_morph)
        {
            input_matrix.read(in);
            m = input_matrix;
            m.invert();
            pOther->_matrix.concatenate(m);
        }
        
        // GRADIENT
        in.ensureBytes(1);

        uint8_t grad_props = in.read_u8();
    
        if (t == SWF::DEFINESHAPE4 ||
            t == SWF::DEFINESHAPE4_) {
            uint8_t spread_mode = grad_props >> 6;
            switch(spread_mode) {
                case 0:
                    m_spread_mode = SWF::GRADIENT_SPREAD_PAD;
                    break;
                case 1:
                    m_spread_mode = SWF::GRADIENT_SPREAD_REFLECT;
                    break;
                case 2:
                    m_spread_mode = SWF::GRADIENT_SPREAD_REPEAT;
                    break;
                default: 
                IF_VERBOSE_MALFORMED_SWF(
                    log_swferror("Illegal spread mode in gradient definition.");
                );
                }
    
            uint8_t interpolation = (grad_props >> 4) & 3;
            switch(interpolation) {
                case 0: 
                    m_interpolation = SWF::GRADIENT_INTERPOL_NORMAL;
                    break;
                case 1:
                    m_interpolation = SWF::GRADIENT_INTERPOL_LINEAR;
                    break;
                default:
                    IF_VERBOSE_MALFORMED_SWF(
                        log_swferror("Illegal interpolation mode in gradient "
                            "definition.");
                    );
            }
        }
    
        uint8_t num_gradients = grad_props & 0xF;
        if ( ! num_gradients )
        {
            IF_VERBOSE_MALFORMED_SWF(
                log_swferror(_("num gradients 0"));
            );
            return;
        }
    
        if ( num_gradients > 8 + ((t == SWF::DEFINESHAPE4 ||
            t == SWF::DEFINESHAPE4_) ? 7 : 0))
        {
           // see: http://sswf.sourceforge.net/SWFalexref.html#swf_gradient
            IF_VERBOSE_MALFORMED_SWF(
                log_swferror(_("Unexpected num gradients (%d), "
                        "expected 1 to 8"), static_cast<int>(num_gradients));
            );
        }
    
        if (is_morph) {
            pOther->m_gradients.resize(num_gradients);
        }
                
        m_gradients.resize(num_gradients);
        for (unsigned int i = 0; i < num_gradients; i++) {
            m_gradients[i].read(in, t);
            if (is_morph) {
                pOther->m_gradients[i].read(in, t);
            }
        }
    
        // A focal gradient also has a focal point.
        if (m_type == SWF::FILL_FOCAL_GRADIENT)
        {
           in.ensureBytes(2);
           m_focal_point = in.read_short_sfixed();
           if (m_focal_point < -1.0f) m_focal_point = -1.0f;
           else if (m_focal_point > 1.0f) m_focal_point = 1.0f;
        }
    
        if (is_morph) {
                pOther->m_focal_point = m_focal_point;
        }
    
        IF_VERBOSE_PARSE (
           log_parse("  gradients: num_gradients = %d",
               static_cast<int>(num_gradients));
        );
    
        // @@ hack. What is it supposed to do?
        if (num_gradients > 0) {
            m_color = m_gradients[0].m_color;
            if (is_morph)
               pOther->m_color = pOther->m_gradients[0].m_color;
        }
    
        _bitmapInfo = create_gradient_bitmap();
        if (is_morph)
        {
            pOther->_bitmapInfo = pOther->need_gradient_bitmap();
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

        if ( m_type == SWF::FILL_TILED_BITMAP_HARD ||
             m_type == SWF::FILL_CLIPPED_BITMAP_HARD )
        {
            _bitmapSmoothingPolicy = BITMAP_SMOOTHING_OFF;
        }
        else if ( md.get_version() >= 8 )
        {
            _bitmapSmoothingPolicy = BITMAP_SMOOTHING_ON;
        }
        else
        {
            _bitmapSmoothingPolicy = BITMAP_SMOOTHING_UNSPECIFIED;
        }

        in.ensureBytes(2);
        int bitmap_char_id = in.read_u16();
        IF_VERBOSE_PARSE
        (
            log_parse("  bitmap_char = %d, smoothing_policy = %s",
                bitmap_char_id, _bitmapSmoothingPolicy);
        );

        // Look up the bitmap DisplayObject.
        _bitmapInfo = md.getBitmap(bitmap_char_id);
        IF_VERBOSE_MALFORMED_SWF(
            if (!_bitmapInfo)
            {
                LOG_ONCE(
                    log_swferror(_("Bitmap fill specifies '%d' as associated"
                        " bitmap DisplayObject id,"
                        " but that DisplayObject is not found"
                        " in the Characters Dictionary."
                        " It seems common to find such "
                        " malformed SWF, so we'll only warn once "
                        "about this."), bitmap_char_id);
                );
            }
        );

        SWFMatrix  m;
        m.read(in);

        // For some reason, it looks like they store the inverse of the
        // TWIPS-to-texcoords SWFMatrix.
        _matrix = m.invert();

        if (is_morph)
        {
            pOther->_bitmapInfo = _bitmapInfo;
            m.read(in);
            pOther->_matrix = m.invert();
        }
        IF_VERBOSE_PARSE(
           log_parse("SWFMatrix: %s", _matrix);
        );
    }
    else
    {
        std::stringstream ss;
        ss << "Unknown fill style type " << m_type;    
        // This is a fatal error, we'll be leaving the stream
        // read pointer in an unknown position.
        throw ParserException(ss.str()); 
    }
}


const BitmapInfo* 
fill_style::get_bitmap_info() const 
{    
    assert(m_type != SWF::FILL_SOLID);

    switch (m_type)
    {
        case SWF::FILL_TILED_BITMAP:
        case SWF::FILL_CLIPPED_BITMAP:
        case SWF::FILL_TILED_BITMAP_HARD:
        case SWF::FILL_CLIPPED_BITMAP_HARD:
            if (_bitmapInfo)
            {
                return _bitmapInfo.get();
            }
            return NULL;
   
        case SWF::FILL_LINEAR_GRADIENT:
        case SWF::FILL_RADIAL_GRADIENT:
            return need_gradient_bitmap();
        default:
            log_error(_("Unknown fill style %d"), m_type);
            // Seems a bit drastic...
            std::abort();
    }
}

const SWFMatrix&
fill_style::getBitmapMatrix() const 
{
  assert(m_type != SWF::FILL_SOLID);
  return _matrix;
}

const SWFMatrix&
fill_style::getGradientMatrix() const 
{
  // TODO: Why do we separate bitmap and gradient matrices? 
  return _matrix;
}

rgba
fill_style::sample_gradient(boost::uint8_t ratio) const
{
    assert(m_type == SWF::FILL_LINEAR_GRADIENT
        || m_type == SWF::FILL_RADIAL_GRADIENT
        || m_type == SWF::FILL_FOCAL_GRADIENT);

    if ( m_gradients.empty() )
    {
        static const rgba black;
        return black;
    }

    // By specs, first gradient should *always* be 0, 
    // anyway a malformed SWF could break this,
    // so we cannot rely on that information...
    if (ratio < m_gradients[0].m_ratio)
    {
        IF_VERBOSE_MALFORMED_SWF(
            LOG_ONCE(
                log_swferror(
                    _("First gradient in a fill_style "
                    "have position==%d (expected 0)."
                    " This seems to be common, so will"
                    " warn only once."),
                    static_cast<int>(m_gradients[0].m_ratio));
            );
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

        rgba result;
        result.set_lerp(gr0.m_color, gr1.m_color, f);
        return result;
    }

    // Assuming gradients are ordered by m_ratio? see start comment
    return m_gradients.back().m_color;
}

const BitmapInfo*
fill_style::create_gradient_bitmap() const
{
    assert(m_type == SWF::FILL_LINEAR_GRADIENT
        || m_type == SWF::FILL_RADIAL_GRADIENT
        || m_type == SWF::FILL_FOCAL_GRADIENT);

    std::auto_ptr<ImageRGBA> im;

    switch (m_type)
    {
        case SWF::FILL_LINEAR_GRADIENT:
            // Linear gradient.
            im.reset(new ImageRGBA(256, 1));

            for (size_t i = 0; i < im->width(); i++)
            {
                rgba sample = sample_gradient(i);
                im->setPixel(i, 0, sample.m_r, sample.m_g,
                        sample.m_b, sample.m_a);
            }
            break;
        case SWF::FILL_RADIAL_GRADIENT:
            // Radial gradient.
            im.reset(new ImageRGBA(64, 64));

            for (size_t j = 0; j < im->height(); j++) {
                for (size_t i = 0; i < im->width(); i++) {
                    float   radius = (im->height() - 1) / 2.0f;
                    float   y = (j - radius) / radius;
                    float   x = (i - radius) / radius;
                    int ratio = static_cast<int>(
                            std::floor(255.5f * std::sqrt(x * x + y * y)));
                    if (ratio > 255) {
                        ratio = 255;
                    }
                    rgba sample = sample_gradient(ratio);
                    im->setPixel(i, j, sample.m_r, sample.m_g,
                            sample.m_b, sample.m_a);
                }
            }
            break;
        case SWF::FILL_FOCAL_GRADIENT:
            // Focal gradient.
            im.reset(new ImageRGBA(64, 64));

            for (size_t j = 0; j < im->height(); j++)
            {
                for (size_t i = 0; i < im->width(); i++)
                {
                    float radiusy = (im->height() - 1) / 2.0f;
                    float radiusx = radiusy + std::abs(radiusy * m_focal_point);
                    float y = (j - radiusy) / radiusy;
                    float x = (i - radiusx) / radiusx;
                    int ratio = static_cast<int>(std::floor(255.5f *
                                std::sqrt(x*x + y*y)));
                    
                    if (ratio > 255) ratio = 255;

                    rgba sample = sample_gradient(ratio);
                    im->setPixel(i, j, sample.m_r, sample.m_g,
                            sample.m_b, sample.m_a);
                }
            }
            break;
    }

    const BitmapInfo* bi = render::createBitmapInfo(
                    static_cast<std::auto_ptr<GnashImage> >(im));

    return bi;
}


const BitmapInfo*
fill_style::need_gradient_bitmap() const 
{

  if (!_bitmapInfo) {
    fill_style* this_non_const = const_cast<fill_style*>(this);
    this_non_const->_bitmapInfo = create_gradient_bitmap();
  }
  
  return _bitmapInfo.get();

}


// Sets this style to a blend of a and b.  t = [0,1]
void
fill_style::set_lerp(const fill_style& a, const fill_style& b, float t)
{
    assert(t >= 0 && t <= 1);

    // fill style type
    m_type = a.get_type();
    assert(m_type == b.get_type());

    // fill style color (TODO: only for solid fills ?)
    m_color.set_lerp(a.get_color(), b.get_color(), t);

    bool usesMatrix = false;

    switch (m_type)
    {
        case SWF::FILL_LINEAR_GRADIENT:
        case SWF::FILL_RADIAL_GRADIENT:
        case SWF::FILL_FOCAL_GRADIENT:
        {
            usesMatrix = true;

            // fill style gradients
            assert(m_gradients.size() == a.m_gradients.size());
            assert(m_gradients.size() == b.m_gradients.size());
            for (size_t j=0, nj=m_gradients.size(); j<nj; ++j)
            {
                m_gradients[j].m_ratio =
                    (boost::uint8_t) frnd( flerp(a.m_gradients[j].m_ratio,
                            b.m_gradients[j].m_ratio, t)
                        );
                m_gradients[j].m_color.set_lerp(a.m_gradients[j].m_color,
                        b.m_gradients[j].m_color, t);
            }
            _bitmapInfo = NULL;
            break;
        }

        case SWF::FILL_TILED_BITMAP:
        case SWF::FILL_CLIPPED_BITMAP:
        case SWF::FILL_TILED_BITMAP_HARD:
        case SWF::FILL_CLIPPED_BITMAP_HARD:
        {
            usesMatrix = true;

            // fill style bitmap ID
            _bitmapInfo = a._bitmapInfo;
            assert(_bitmapInfo == b._bitmapInfo);
            break;
        }

        default:
            break;
    }

    // fill style bitmap or gradient SWFMatrix
    if ( usesMatrix ) _matrix.set_lerp(a._matrix, b._matrix, t);
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

fill_style::fill_style(const BitmapInfo* const bitmap, const SWFMatrix& mat)
    :
    _matrix(mat),
    _bitmapInfo(bitmap),
    m_type(SWF::FILL_CLIPPED_BITMAP),
    _bitmapSmoothingPolicy(BITMAP_SMOOTHING_UNSPECIFIED)
{
}

void
fill_style::setSolid(const rgba& color)
{
    m_type = SWF::FILL_SOLID;
    m_color = color;
}

void
fill_style::setLinearGradient(const std::vector<gradient_record>& gradients,
        const SWFMatrix& mat)
{
    m_type = SWF::FILL_LINEAR_GRADIENT;
    m_gradients = gradients;
    _matrix = mat;
    _bitmapInfo = 0;
}

void
fill_style::setRadialGradient(const std::vector<gradient_record>& gradients,
        const SWFMatrix& mat)
{
    m_type = SWF::FILL_RADIAL_GRADIENT;
    m_gradients = gradients;
    _matrix = mat;
    _bitmapInfo = 0;
}

std::ostream& operator << (std::ostream& os,
        const fill_style::BitmapSmoothingPolicy& p)
{
    switch (p)
    {
        case fill_style::BITMAP_SMOOTHING_UNSPECIFIED:
            os << "unspecified";
            break;
        case fill_style::BITMAP_SMOOTHING_ON:
            os << "on";
            break;
        case fill_style::BITMAP_SMOOTHING_OFF:
            os << "off";
            break;
        default:
            // cast to int required to avoid infinite recursion
            os << "unknown " << (int)p;
            break;
    }
    return os;
}


#ifdef GNASH_USE_GC
void
fill_style::markReachableResources() const
{
    if ( _bitmapInfo ) _bitmapInfo->setReachable();
}
#endif // GNASH_USE_GC

} // end of namespace


// Local Variables:
// mode: C++
// End:
