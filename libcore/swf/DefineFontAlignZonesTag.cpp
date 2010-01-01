// DefineFontAlignZonesTag.cpp:  for Gnash.
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


#include "Font.h"
#include "log.h"
#include "SWFStream.h"
#include "movie_definition.h"
#include "DefineFontAlignZonesTag.h"
#include <iostream>

namespace gnash {
namespace SWF {

DefineFontAlignZonesTag::DefineFontAlignZonesTag(movie_definition& /* m */,
	SWFStream& /* in */)
    :
    _csm_table_int(2)
{
}

/* public static */
void
DefineFontAlignZonesTag::loader(SWFStream& in, TagType tag,
        movie_definition& m, const RunResources& /*r*/)
{
	assert(tag == SWF::DEFINEALIGNZONES);

	in.ensureBytes(2);

    // must reference a valid DEFINEFONT3 tag
    const boost::uint16_t ref = in.read_u16(); 
	Font* referencedFont = m.get_font(ref);
	if (!referencedFont) {
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("DefineFontAlignZones tag references an undefined "
               "font %d"), ref);
		);
		in.skip_to_tag_end();
		return;
	}

	in.ensureBytes(1);
    // 2bits are cms table, 6bits are reserved
	const boost::uint8_t flags = in.read_u8();

	// TODO:
    // The first thing to to is test what this does. According to some
    // sources, the tag is ignored and merely turns on the player's
    // font engine.
	IF_VERBOSE_PARSE (
        log_parse(_(" ** DefineFontAlignZones: font=%d, flags=%d"), ref, flags);
	);

	const boost::uint16_t csm_table_int_temp = flags >> 6;
			
	Font::GlyphInfoRecords::size_type glyphs_count_temp =
        referencedFont->glyphCount();

	for (size_t i = 0; i != glyphs_count_temp; ++i) {
        in.ensureBytes(1);
        
        in.read_u8();		
        
        for (int j = 0; j != 2; ++j) {
            in.ensureBytes(2);
            float f_zone_position_temp = in.read_u16();
            in.ensureBytes(2);
            float f_zone_size_temp = in.read_u16();
        }		
        in.ensureBytes(1);
        
        // What is this?
        boost::uint8_t u = in.read_u8();
        const boost::uint32_t f_zone_x_temp = u & 0x0001;
        unsigned f_zone_y_temp = (u >> 1) & 0x0001;
			
    }
	in.skip_to_tag_end();
	LOG_ONCE(log_unimpl(_("*** DefineFontAlignZoneTag")));

}


} // namespace gnash::SWF
} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End
