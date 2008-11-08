// DefineFontAlignZonesTag.cpp:  for Gnash.
//
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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


#include "font.h"
#include "log.h"
#include "shape.h"
#include "SWFStream.h"
#include "movie_definition.h"
#include "DefineFontAlignZonesTag.h"

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
DefineFontAlignZonesTag::loader(SWFStream& in, tag_type tag,
        movie_definition& m, const RunInfo& /*r*/)
{
	assert(tag == SWF::DEFINEALIGNZONES);

    in.ensureBytes(1);
	unsigned short ref = in.read_u8(); // must reference a valid DEFINEFONT3 tag
	font* referencedFont = m.get_font(ref);
	if ( ! referencedFont )
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("DefineFontAlignZones tag references an undefined "
                "font %d"), ref);
		);
		in.skip_to_tag_end();
		return;
	}

    in.ensureBytes(1);
	unsigned flags = in.read_u8(); // 2bits are cms table, 6bits are reserved

	// TODO:
	// 	- parse swf_zone_array
	// 	- construct a DefineFontAlignZonesTag class
	// 	- register the tag with the referencedFont

	IF_VERBOSE_PARSE (
	log_parse(_("  DefineFontAlignZones: font=%d, flags=%d"), ref, flags);
	);

	in.skip_to_tag_end();
	LOG_ONCE(log_unimpl(_("DefineFontAlignZoneTag")));

}


} // namespace gnash::SWF
} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
