// fill_style.cpp:  Graphical region filling styles, for Gnash.
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
//

// Based on work of Thatcher Ulrich <tu@tulrich.com> 2003

#include "smart_ptr.h"
#include "fill_style.h"
#include "log.h"
#include "SWFStream.h"
#include "movie_definition.h"
#include "SWF.h"
#include "GnashException.h"
#include "GnashNumeric.h"
#include "Renderer.h"
#include "RunResources.h"
#include "GnashImage.h"

#include <cmath> // sqrt, floor
#include <iostream> // for output operator
#include <boost/variant.hpp>

namespace gnash {

// Forward declarations
namespace {
    rgba sampleGradient(const GradientFill& fill, boost::uint8_t ratio);
    SolidFill readSolidFill(SWFStream& in, SWF::TagType t, fill_style* morph);
    BitmapFill readBitmapFill(SWFStream& in, SWF::FillType type,
            movie_definition& md, fill_style* morph);
}

namespace {

/// Create a lerped version of two other fill_styles.
//
/// The two fill styles must have exactly the same types. Callers are 
/// responsible for ensuring this.
class SetLerp : public boost::static_visitor<>
{
public:
    SetLerp(const fill_style::Fill& a, const fill_style::Fill& b, double ratio)
        :
        _a(a),
        _b(b),
        _ratio(ratio)
    {
    }

    template<typename T> void operator()(T& f) const {
        const T& a = boost::get<T>(_a);
        const T& b = boost::get<T>(_b);
        f.setLerp(a, b, _ratio);
    }

private:
    const fill_style::Fill& _a;
    const fill_style::Fill& _b;
    const double _ratio;

};

}
    
BitmapFill::BitmapFill(SWF::FillType t, movie_definition* md,
        boost::uint16_t id, const SWFMatrix& m)
    :
    _type(),
    _smoothingPolicy(),
    _matrix(m),
    _bitmapInfo(0),
    _md(md),
    _id(id)
{
    assert(md);

    _smoothingPolicy = md->get_version() >= 8 ? 
        BitmapFill::SMOOTHING_ON : BitmapFill::SMOOTHING_UNSPECIFIED;

    switch (t) {
        case SWF::FILL_TILED_BITMAP_HARD:
            _type = BitmapFill::TILED;
            _smoothingPolicy = BitmapFill::SMOOTHING_OFF;
            break;

        case SWF::FILL_TILED_BITMAP:
            _type = BitmapFill::TILED;
            break;

        case SWF::FILL_CLIPPED_BITMAP_HARD:
            _type = BitmapFill::CLIPPED;
            _smoothingPolicy = BitmapFill::SMOOTHING_OFF;
            break;

        case SWF::FILL_CLIPPED_BITMAP:
            _type = BitmapFill::CLIPPED;
            break;

        default:
            std::abort();
    }
}

const BitmapInfo*
BitmapFill::bitmap() const
{
    if (_bitmapInfo) return _bitmapInfo.get();
    if (!_md) return 0;
    _bitmapInfo = _md->getBitmap(_id);

    // May still be 0!
    return _bitmapInfo.get();
}
    
void
GradientFill::setLerp(const GradientFill& a, const GradientFill& b,
        double ratio)
{
    assert(type == a.type);
    assert(gradients.size() == a.gradients.size());
    assert(gradients.size() == b.gradients.size());

    for (size_t i = 0, e = gradients.size(); i < e; ++i) {
        gradients[i].m_ratio = frnd(flerp(a.gradients[i].m_ratio,
                    b.gradients[i].m_ratio, ratio));
        gradients[i].m_color.set_lerp(a.gradients[i].m_color,
                b.gradients[i].m_color, ratio);
    }
    matrix.set_lerp(a.matrix, b.matrix, ratio);
}
    
void
BitmapFill::setLerp(const BitmapFill& a, const BitmapFill& b, double ratio)
{
    _matrix.set_lerp(a.matrix(), b.matrix(), ratio);
}

void
gradient_record::read(SWFStream& in, SWF::TagType tag)
{
    in.ensureBytes(1);
    m_ratio = in.read_u8();
    m_color.read(in, tag);
}

void
fill_style::read(SWFStream& in, SWF::TagType t, movie_definition& md,
        const RunResources& /*r*/, fill_style *pOther)
{
    const bool is_morph = (pOther != NULL);

    in.ensureBytes(1);
    const SWF::FillType type = static_cast<SWF::FillType>(in.read_u8());
        
    IF_VERBOSE_PARSE(
        log_parse("  fill_style read type = 0x%X", +type);
    );

    switch (type) {

        case SWF::FILL_SOLID:
            fill = readSolidFill(in, t, pOther);
            return;

        case SWF::FILL_TILED_BITMAP_HARD:
        case SWF::FILL_CLIPPED_BITMAP_HARD:
        case SWF::FILL_TILED_BITMAP:
        case SWF::FILL_CLIPPED_BITMAP:
            fill = readBitmapFill(in, type, md, pOther);
            return;

        case SWF::FILL_LINEAR_GRADIENT:
        case SWF::FILL_RADIAL_GRADIENT:
        case SWF::FILL_FOCAL_GRADIENT:
        {

            GradientFill gf;

            SWFMatrix base;

            if (type == SWF::FILL_LINEAR_GRADIENT) {
                gf.type = GradientFill::LINEAR;
                base.set_translation(128, 0);
                base.set_scale(1.0 / 128, 1.0 / 128);
            }
            else {
                // FILL_RADIAL_GRADIENT or FILL_FOCAL_GRADIENT
                base.set_translation(32, 32);
                base.set_scale(1.0 / 512, 1.0 / 512);
                gf.type = type == SWF::FILL_FOCAL_GRADIENT ?
                    GradientFill::FOCAL : GradientFill::RADIAL;
            }

            // Set base transform for both matrices.
            gf.matrix = base;
            if (is_morph) {
                pOther->fill = GradientFill();
                boost::get<GradientFill>(pOther->fill).matrix = base;
            }

            SWFMatrix m;
            m.read(in);
            m.invert();

            gf.matrix.concatenate(m);
            
            if (is_morph) {
                SWFMatrix m2;
                m2.read(in);
                m2.invert();
                boost::get<GradientFill>(pOther->fill).matrix.concatenate(m2);
            }
            
            // GRADIENT
            in.ensureBytes(1);
            const boost::uint8_t grad_props = in.read_u8();
        
            if (t == SWF::DEFINESHAPE4 || t == SWF::DEFINESHAPE4_) {

                const boost::uint8_t spread_mode = grad_props >> 6;
                switch(spread_mode) {
                    case 0:
                        gf.spreadMode = SWF::GRADIENT_SPREAD_PAD;
                        break;
                    case 1:
                        gf.spreadMode = SWF::GRADIENT_SPREAD_REFLECT;
                        break;
                    case 2:
                        gf.spreadMode = SWF::GRADIENT_SPREAD_REPEAT;
                        break;
                    default: 
                        IF_VERBOSE_MALFORMED_SWF(
                            log_swferror("Illegal spread mode in gradient "
                                "definition.");
                        );
                }
        
                // TODO: handle in GradientFill.
                const boost::uint8_t interpolation = (grad_props >> 4) & 3;
                switch (interpolation) {
                    case 0: 
                        gf.interpolation = SWF::GRADIENT_INTERPOLATION_NORMAL;
                        break;
                    case 1:
                        gf.interpolation = SWF::GRADIENT_INTERPOLATION_LINEAR;
                        break;
                    default:
                        IF_VERBOSE_MALFORMED_SWF(
                            log_swferror("Illegal interpolation mode in "
                                "gradient definition.");
                        );
                }
            }
        
            const boost::uint8_t num_gradients = grad_props & 0xF;
            if (!num_gradients) {
                IF_VERBOSE_MALFORMED_SWF(
                    log_swferror(_("num gradients 0"));
                );
                return;
            }
        
            if (num_gradients > 8 + ((t == SWF::DEFINESHAPE4 ||
                t == SWF::DEFINESHAPE4_) ? 7 : 0)) {
               // see: http://sswf.sourceforge.net/SWFalexref.html#swf_gradient
                IF_VERBOSE_MALFORMED_SWF(
                    log_swferror(_("Unexpected num gradients (%d), "
                            "expected 1 to 8"), static_cast<int>(num_gradients));
                );
            }
        
            if (is_morph) {
                boost::get<GradientFill>(pOther->fill).gradients.resize(
                        num_gradients);
            }
                    
            gf.gradients.resize(num_gradients);
            for (size_t i = 0; i < num_gradients; ++i) {
                gf.gradients[i].read(in, t);
                if (is_morph) {
                    boost::get<GradientFill>(pOther->fill).gradients[i].read(
                            in, t);
                }
            }
        
            // A focal gradient also has a focal point.
            if (type == SWF::FILL_FOCAL_GRADIENT) {
               in.ensureBytes(2);
               gf.focalPoint = in.read_short_sfixed();
               if (gf.focalPoint < -1.0f) gf.focalPoint = -1.0f;
               else if (gf.focalPoint > 1.0f) gf.focalPoint = 1.0f;
            }
        
            if (is_morph) {
                boost::get<GradientFill>(pOther->fill).focalPoint =
                    gf.focalPoint;
            }
        
            IF_VERBOSE_PARSE(
               log_parse("  gradients: num_gradients = %d", +num_gradients);
            );
        
            // Do this before creating bitmap!
            fill = gf;
        
            return;
        }

        default:
        {
            std::stringstream ss;
            ss << "Unknown fill style type " << +type;    
            // This is a fatal error, we'll be leaving the stream
            // read pointer in an unknown position.
            throw ParserException(ss.str()); 
        }
    }
}

// Sets this style to a blend of a and b.  t = [0,1]
void
fill_style::set_lerp(const fill_style& a, const fill_style& b, float t)
{
    assert(t >= 0 && t <= 1);
    fill = a.fill;
    boost::apply_visitor(SetLerp(a.fill, b.fill, t), fill);
}

namespace {

SolidFill
readSolidFill(SWFStream& in, SWF::TagType t, fill_style* morph)
{
    rgba color;

    // 0x00: solid fill
    if (t == SWF::DEFINESHAPE3 || t == SWF::DEFINESHAPE4 ||
            t == SWF::DEFINESHAPE4_ || morph) {
        color.read_rgba(in);
        if (morph) {
            rgba othercolor;
            othercolor.read_rgba(in);
            morph->fill = SolidFill(othercolor);
        }
    }
    else {
        // For DefineMorphShape tags we should use morphfill_style 
        assert(t == SWF::DEFINESHAPE || t == SWF::DEFINESHAPE2);
        color.read_rgb(in);
    }

    IF_VERBOSE_PARSE(
        log_parse("  color: %s", color);
    );
    return SolidFill(color);
}

BitmapFill
readBitmapFill(SWFStream& in, SWF::FillType type, movie_definition& md,
        fill_style* morph)
{

    in.ensureBytes(2);
    const boost::uint16_t id = in.read_u16();

    SWFMatrix m;
    m.read(in);

    if (morph) {
        SWFMatrix m2;
        m2.read(in);
        m2.invert();
        morph->fill = BitmapFill(type, &md, id, m2);
    }

    // For some reason, it looks like they store the inverse of the
    // TWIPS-to-texcoords SWFMatrix.
    m.invert();
    return BitmapFill(type, &md, id, m);
}

}

std::ostream&
operator<<(std::ostream& os, const BitmapFill::SmoothingPolicy& p)
{
    switch (p) {
        case BitmapFill::SMOOTHING_UNSPECIFIED:
            os << "unspecified";
            break;
        case BitmapFill::SMOOTHING_ON:
            os << "on";
            break;
        case BitmapFill::SMOOTHING_OFF:
            os << "off";
            break;
        default:
            // cast to int required to avoid infinite recursion
            os << "unknown " << +p;
            break;
    }
    return os;
}

} // end of namespace


// Local Variables:
// mode: C++
// End:
