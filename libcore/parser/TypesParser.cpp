// TypesParser.cpp: read SWF types from a stream
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "TypesParser.h"

#include <boost/optional.hpp>
#include <utility>

#include "GnashException.h"
#include "SWF.h"
#include "SWFStream.h"
#include "RGBA.h"
#include "SWFMatrix.h"
#include "SWFRect.h"
#include "SWFCxForm.h"
#include "FillStyle.h"
#include "log.h"
#include "movie_definition.h"

namespace gnash {

// Forward declarations
namespace {
    OptionalFillPair readSolidFill(SWFStream& in, SWF::TagType t,
            bool readMorph);
    OptionalFillPair readBitmapFill(SWFStream& in, SWF::FillType type,
            movie_definition& md, bool readMorph);
    GradientRecord readGradientRecord(SWFStream& in, SWF::TagType tag);
}


SWFMatrix
readSWFMatrix(SWFStream& in)
{
    in.align();

    in.ensureBits(1);
    const bool has_scale = in.read_bit(); 

    boost::int32_t sx = 65536;
    boost::int32_t sy = 65536;
    if (has_scale) {
        in.ensureBits(5);
        const boost::uint8_t scale_nbits = in.read_uint(5);
        in.ensureBits(scale_nbits * 2);
        sx = in.read_sint(scale_nbits);
        sy = in.read_sint(scale_nbits);
    }

    in.ensureBits(1);
    const bool has_rotate = in.read_bit();
    boost::int32_t shx = 0;
    boost::int32_t shy = 0;
    if (has_rotate) {
        in.ensureBits(5);
        int rotate_nbits = in.read_uint(5);

        in.ensureBits(rotate_nbits * 2);
        shx = in.read_sint(rotate_nbits);
        shy = in.read_sint(rotate_nbits);
    }

    in.ensureBits(5);
    const boost::uint8_t translate_nbits = in.read_uint(5);
    boost::int32_t tx = 0;
    boost::int32_t ty = 0;
    if (translate_nbits) {
        in.ensureBits(translate_nbits * 2);
        tx = in.read_sint(translate_nbits);
        ty = in.read_sint(translate_nbits);
    }
    return SWFMatrix(sx, shx, shy, sy, tx, ty);
}

rgba
readRGBA(SWFStream& in)
{
    in.ensureBytes(4);
    const boost::uint8_t r = in.read_u8();
    const boost::uint8_t g = in.read_u8();
    const boost::uint8_t b = in.read_u8();
    const boost::uint8_t a = in.read_u8();
    return rgba(r, g, b, a);
}

rgba
readRGB(SWFStream& in)
{
    in.ensureBytes(3);
    const boost::uint8_t r = in.read_u8();
    const boost::uint8_t g = in.read_u8();
    const boost::uint8_t b = in.read_u8();
    const boost::uint8_t a = 0xff;
    return rgba(r, g, b, a);
}

/// Format of the bit-packed rectangle is:
///
/// bits  | name  | description
/// ------+-------+-------------------------
///   5   | nbits | number of bits used in subsequent values
/// nbits | xmin  | minimum X value
/// nbits | xmax  | maximum X value
/// nbits | ymin  | minimum Y value
/// nbits | ymax  | maximum Y value
///
/// If max values are less then min values the SWF is malformed;
/// in this case this method will raise an swf_error and set the
/// rectangle to the NULL rectangle. See is_null().
SWFRect
readRect(SWFStream& in)
{
    in.align();
    in.ensureBits(5);
    const int nbits = in.read_uint(5);
    in.ensureBits(nbits*4);
    
    const int minx = in.read_sint(nbits);
    const int maxx = in.read_sint(nbits);
    const int miny = in.read_sint(nbits);
    const int maxy = in.read_sint(nbits);

    // Check if this SWFRect is valid.
    if (maxx < minx || maxy < miny) {
        // We set invalid rectangles to NULL, but we might instead
        // want to actually swap the values if the proprietary player
        // does so. TODO: check it out.
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror("Invalid rectangle: "
                "minx=%g maxx=%g miny=%g maxy=%g", minx, maxx, miny, maxy);
        );
        return SWFRect();
    } 
    return SWFRect(minx, miny, maxx, maxy);
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
               log_parse("  gradients count: %d", +num_gradients);
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
        
                const SWF::InterpolationMode i =
                    static_cast<SWF::InterpolationMode>((grad_props >> 4) & 3);

                switch (i) {
                    case SWF::GRADIENT_INTERPOLATION_NORMAL:
                        gf.interpolation = GradientFill::RGB;
                        break;
                    case SWF::GRADIENT_INTERPOLATION_LINEAR:
                        gf.interpolation = GradientFill::LINEAR_RGB;
                        break;
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

SWFCxForm
readCxFormRGB(SWFStream& in)
{
    in.align();

    in.ensureBits(6);
    const boost::uint8_t field =  in.read_uint(6);
    const bool has_add = field & (1 << 5);
    const bool has_mult = field & (1 << 4);
    const boost::uint8_t nbits = field & 0x0f;
    const size_t reads = has_mult + has_add; // 0, 1 or 2

    SWFCxForm ret;

    if (!reads) return ret;

    if (has_mult) {
        ret.ra = in.read_sint(nbits);
        ret.ga = in.read_sint(nbits);
        ret.ba = in.read_sint(nbits);
        // aa is already 256.
    }

    if (has_add) {
        ret.rb = in.read_sint(nbits);
        ret.gb = in.read_sint(nbits);
        ret.bb = in.read_sint(nbits);
        // ab is already 0.
    }
    return ret;
}

SWFCxForm
readCxFormRGBA(SWFStream& in)
{
    in.align();

    in.ensureBits(6);
    const boost::uint8_t field =  in.read_uint(6);
    const bool has_add = field & (1 << 5);
    const bool has_mult = field & (1 << 4);
    const boost::uint8_t nbits = field & 0x0f;
    const size_t reads = has_mult + has_add; // 0, 1 or 2

    SWFCxForm ret;

    if (!reads) return ret;
    
    in.ensureBits(nbits * reads * 4);

    // Default is 256 for these values.
    if (has_mult) {
        ret.ra = in.read_sint(nbits);
        ret.ga = in.read_sint(nbits);
        ret.ba = in.read_sint(nbits);
        ret.aa = in.read_sint(nbits);
    }

    if (has_add) {
        ret.rb = in.read_sint(nbits);
        ret.gb = in.read_sint(nbits);
        ret.bb = in.read_sint(nbits);
        ret.ab = in.read_sint(nbits);
    }
    return ret;
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

}
