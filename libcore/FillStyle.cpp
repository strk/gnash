// FillStyle.cpp:  Graphical region filling styles, for Gnash.
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

#include "FillStyle.h"

#include <iostream> 
#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include "smart_ptr.h"
#include "log.h"
#include "SWFStream.h"
#include "movie_definition.h"
#include "SWF.h"
#include "GnashException.h"
#include "GnashNumeric.h"
#include "Renderer.h"
#include "RunResources.h"
#include "GnashImage.h"


namespace gnash {

// Forward declarations
namespace {
    rgba sampleGradient(const GradientFill& fill, boost::uint8_t ratio);
    OptionalFillPair readSolidFill(SWFStream& in, SWF::TagType t,
            bool readMorph);
    OptionalFillPair readBitmapFill(SWFStream& in, SWF::FillType type,
            movie_definition& md, bool readMorph);
    GradientRecord readGradientRecord(SWFStream& in, SWF::TagType tag);
}

namespace {

/// Create a lerped version of two other FillStyles.
//
/// The two fill styles must have exactly the same types. Callers are 
/// responsible for ensuring this.
class SetLerp : public boost::static_visitor<>
{
public:
    SetLerp(const FillStyle::Fill& a, const FillStyle::Fill& b, double ratio)
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
    const FillStyle::Fill& _a;
    const FillStyle::Fill& _b;
    const double _ratio;

};

}

SWFMatrix
gradientMatrix(GradientFill::Type t, const SWFMatrix& m)
{
    SWFMatrix base;
    switch (t) {
        case GradientFill::LINEAR:
            base.set_translation(128, 0);
            base.set_scale(1.0 / 128, 1.0 / 128);
            break;
        case GradientFill::RADIAL:
            base.set_translation(32, 32);
            base.set_scale(1.0 / 512, 1.0 / 512);
            break;
    }
    base.concatenate(m);
    return base;
}

GradientFill::GradientFill(Type t, const SWFMatrix& m,
        const GradientRecords& recs)
    :
    spreadMode(PAD),
    interpolation(SWF::GRADIENT_INTERPOLATION_NORMAL),
    _focalPoint(0.0),
    _gradients(recs),
    _type(t),
    _matrix(gradientMatrix(t, m))
{
    assert(recs.empty() || recs.size() > 1);
}
    
void
GradientFill::setFocalPoint(double d)
{
    _focalPoint = clamp<float>(d, -1, 1); 
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

const CachedBitmap*
BitmapFill::bitmap() const
{
    if (_bitmapInfo) {
        return  _bitmapInfo->disposed() ? 0 : _bitmapInfo.get();
    }
    if (!_md) return 0;
    _bitmapInfo = _md->getBitmap(_id);

    // May still be 0!
    return _bitmapInfo.get();
}
    
void
GradientFill::setLerp(const GradientFill& a, const GradientFill& b,
        double ratio)
{
    assert(type() == a.type());
    assert(_gradients.size() == a.recordCount());
    assert(_gradients.size() == b.recordCount());

    for (size_t i = 0, e = _gradients.size(); i < e; ++i) {
        const GradientRecord& ra = a.record(i);
        const GradientRecord& rb = b.record(i);
        _gradients[i].ratio = frnd(lerp<float>(ra.ratio, rb.ratio, ratio));
        _gradients[i].color.set_lerp(ra.color, rb.color, ratio);
    }
    _matrix.set_lerp(a.matrix(), b.matrix(), ratio);
}
    
void
BitmapFill::setLerp(const BitmapFill& a, const BitmapFill& b, double ratio)
{
    _matrix.set_lerp(a.matrix(), b.matrix(), ratio);
}

OptionalFillPair
readFills(SWFStream& in, SWF::TagType t, movie_definition& md, bool readMorph)
{

    in.ensureBytes(1);
    const SWF::FillType type = static_cast<SWF::FillType>(in.read_u8());
        
    IF_VERBOSE_PARSE(
        log_parse("  FillStyle read type = 0x%X", +type);
    );

    switch (type) {

        case SWF::FILL_SOLID:
            return readSolidFill(in, t, readMorph);

        case SWF::FILL_TILED_BITMAP_HARD:
        case SWF::FILL_CLIPPED_BITMAP_HARD:
        case SWF::FILL_TILED_BITMAP:
        case SWF::FILL_CLIPPED_BITMAP:
            return readBitmapFill(in, type, md, readMorph);

        case SWF::FILL_LINEAR_GRADIENT:
        case SWF::FILL_RADIAL_GRADIENT:
        case SWF::FILL_FOCAL_GRADIENT:
        {

            GradientFill::Type gr;
            switch (type) {
                case SWF::FILL_LINEAR_GRADIENT:
                    gr = GradientFill::LINEAR;
                    break;
                case SWF::FILL_RADIAL_GRADIENT:
                case SWF::FILL_FOCAL_GRADIENT:
                    gr = GradientFill::RADIAL;
                    break;
                default:
                    std::abort();
            }

            SWFMatrix m = readSWFMatrix(in).invert();
            GradientFill gf(gr, m);

            boost::optional<FillStyle> morph;
            if (readMorph) {
                SWFMatrix m2 = readSWFMatrix(in).invert();
                morph = GradientFill(gr, m2);
            }
            
            in.ensureBytes(1);
            const boost::uint8_t grad_props = in.read_u8();
            
            const boost::uint8_t num_gradients = grad_props & 0xF;
            IF_VERBOSE_PARSE(
               log_parse("  gradients: num_gradients = %d", +num_gradients);
            );
        
            if (!num_gradients) {
                IF_VERBOSE_MALFORMED_SWF(
                    log_swferror(_("No gradients!"));
                );
                throw ParserException();
            }
        
            GradientFill::GradientRecords recs;
            recs.reserve(num_gradients);

            GradientFill::GradientRecords morphrecs;
            morphrecs.reserve(num_gradients);

            for (size_t i = 0; i < num_gradients; ++i) {
                recs.push_back(readGradientRecord(in, t));
                if (readMorph) {
                    morphrecs.push_back(readGradientRecord(in, t));
                }
            }
        
            // A GradientFill may never have fewer than 2 colour stops. We've
            // no tests to show what happens in that case for static fills.
            // Dynamic fills are tested to display as a solid fill. In either
            // case the renderer will bork if there is only 1 stop in a 
            // GradientFill.
            if (num_gradients == 1) {
                const rgba c1 = recs[0].color;
                if (readMorph) {
                    const rgba c2 = morphrecs[0].color;
                    morph = SolidFill(c2);
                }
                return std::make_pair(SolidFill(c1), morph);
            }
        
            gf.setRecords(recs);
            if (readMorph) {
                boost::get<GradientFill>(morph->fill).setRecords(morphrecs);
            }


            if (t == SWF::DEFINESHAPE4 || t == SWF::DEFINESHAPE4_) {

                const SWF::SpreadMode spread =
                    static_cast<SWF::SpreadMode>(grad_props >> 6);

                switch (spread) {
                    case SWF::GRADIENT_SPREAD_PAD:
                        gf.spreadMode = GradientFill::PAD;
                        break;
                    case SWF::GRADIENT_SPREAD_REFLECT:
                        gf.spreadMode = GradientFill::REFLECT;
                        break;
                    case SWF::GRADIENT_SPREAD_REPEAT:
                        gf.spreadMode = GradientFill::REPEAT;
                        break;
                    default: 
                        IF_VERBOSE_MALFORMED_SWF(
                            log_swferror("Illegal spread mode in gradient "
                                "definition.");
                        );
                }
        
                // TODO: handle in GradientFill.
                const SWF::InterpolationMode i =
                    static_cast<SWF::InterpolationMode>((grad_props >> 4) & 3);

                switch (i) {
                    case SWF::GRADIENT_INTERPOLATION_NORMAL:
                    case SWF::GRADIENT_INTERPOLATION_LINEAR:
                        gf.interpolation = i;
                    default:
                        IF_VERBOSE_MALFORMED_SWF(
                            log_swferror("Illegal interpolation mode in "
                                "gradient definition.");
                        );
                }
            }

            // A focal gradient also has a focal point.
            if (type == SWF::FILL_FOCAL_GRADIENT) {
               in.ensureBytes(2);
               gf.setFocalPoint(in.read_short_sfixed());
            }
        
            if (readMorph) {
                boost::get<GradientFill>(morph->fill).
                    setFocalPoint(gf.focalPoint());
            }

            return std::make_pair(gf, morph);
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
setLerp(FillStyle& f, const FillStyle& a, const FillStyle& b, double t)
{
    assert(t >= 0 && t <= 1);
    f.fill = a.fill;
    boost::apply_visitor(SetLerp(a.fill, b.fill, t), f.fill);
}

namespace {

OptionalFillPair
readSolidFill(SWFStream& in, SWF::TagType t, bool readMorph)
{
    rgba color;

    boost::optional<FillStyle> morph;

    // 0x00: solid fill
    if (t == SWF::DEFINESHAPE3 || t == SWF::DEFINESHAPE4 ||
            t == SWF::DEFINESHAPE4_ || readMorph) {
        color = readRGBA(in);
        if (readMorph) {
            rgba othercolor;
            othercolor = readRGBA(in);
            morph = SolidFill(othercolor);
        }
    }
    else {
        // For DefineMorphShape tags we should use morphFillStyle 
        assert(t == SWF::DEFINESHAPE || t == SWF::DEFINESHAPE2);
        color = readRGB(in);
    }

    IF_VERBOSE_PARSE(
        log_parse("  color: %s", color);
    );
    return std::make_pair(SolidFill(color), morph);
}

OptionalFillPair
readBitmapFill(SWFStream& in, SWF::FillType type, movie_definition& md,
        bool readMorph)
{

    in.ensureBytes(2);
    const boost::uint16_t id = in.read_u16();

    SWFMatrix m = readSWFMatrix(in).invert();

    boost::optional<FillStyle> morph;
    if (readMorph) {
        SWFMatrix m2 = readSWFMatrix(in).invert();
        morph = BitmapFill(type, &md, id, m2);
    }

    // For some reason, it looks like they store the inverse of the
    // TWIPS-to-texcoords SWFMatrix.
    return std::make_pair(BitmapFill(type, &md, id, m), morph);
}

GradientRecord
readGradientRecord(SWFStream& in, SWF::TagType tag)
{
    in.ensureBytes(1);
    const boost::uint8_t ratio = in.read_u8();

    switch (tag) {
        case SWF::DEFINESHAPE:
        case SWF::DEFINESHAPE2:
        {
            const rgba color = readRGB(in);
            return GradientRecord(ratio, color);
        }
        default:
            break;
    }
    const rgba color = readRGBA(in);
    return GradientRecord(ratio, color);
}

} // anonymous namespace

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

} // namespace gnash


// Local Variables:
// mode: C++
// End:
