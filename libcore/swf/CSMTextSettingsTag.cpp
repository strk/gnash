// CSMTextSettingsTag.cpp:  for Gnash.
//
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#include "RunResources.h"
#include "log.h"
#include "SWFStream.h"
#include "movie_definition.h"
#include "CSMTextSettingsTag.h"
#include "smart_ptr.h"

namespace gnash {
namespace SWF {

CSMTextSettingsTag::CSMTextSettingsTag(movie_definition& /* m */,
    SWFStream& /* in */)
{
}

void
CSMTextSettingsTag::loader(SWFStream& in, TagType tag, movie_definition& /*m*/,
        const RunResources& /*r*/)
{
    assert(tag == SWF::CSMTEXTSETTINGS); // 73

    // TextID (UI16) 16 bit int.
    // UseFlashType(UB[2]) 2 bits
    // GridFit UB[3] 3 bits
    // res (UB[3]) 3 bits
    // Thickness F32
    // Sharpness F32
    // res UI8 must be 0. 8 bit int? 

    in.ensureBytes(2 + 1 + 4 + 4 + 1);

    boost::uint16_t textID = in.read_u16();
    
    // Should be either 1 or 0. TODO: what if it's something else?
    bool flashType = in.read_uint(2); 
    
    // 0: no grid fitting.
    // 1: Pixel grid fit (only for left-aligned dynamic text)
    // 2: Sub-pixel grid fit.
    boost::uint8_t gridFit = in.read_uint(3);

    // Should be 0:
    boost::uint8_t reserved = in.read_uint(3);

    float thickness = in.read_long_float();
    
    float sharpness = in.read_long_float();
    
    // Should also be 0.
    reserved = in.read_u8();

    IF_VERBOSE_PARSE (
        log_parse(_("  CSMTextSettings: TextID=%d, FlashType=%d, "
                    "GridFit=%d, Thickness=%d, Sharpness=%d"),
                    textID, static_cast<int>(flashType),
                    static_cast<int>(gridFit), thickness, sharpness);
    );

    in.skip_to_tag_end();

    LOG_ONCE(log_unimpl(_("CSMTextSettings")));

}


} // namespace gnash::SWF
} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
