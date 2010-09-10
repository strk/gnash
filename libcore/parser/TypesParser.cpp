// TypesParser.cpp: read SWF types from a stream
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#include "SWFStream.h"
#include "RGBA.h"
#include "SWFMatrix.h"
#include "SWFRect.h"
#include "log.h"

namespace gnash {

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
    return SWFRect(minx, maxx, miny, maxy);
}

}
