// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/* $Id: DefineFontAlignZonesTag.cpp,v 1.2 2007/04/04 20:30:45 bjacques Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "font.h"
#include "log.h"
#include "shape.h"
#include "stream.h"
#include "movie_definition.h"
#include "DefineFontAlignZonesTag.h"

namespace gnash {
namespace SWF {
namespace tag_loaders {

DefineFontAlignZonesTag::DefineFontAlignZonesTag(movie_definition& /* m */, 
	stream& /* in */)
{
}

/* public static */
void
DefineFontAlignZonesTag::loader(stream* in, tag_type tag, movie_definition* m)
{
	assert(tag == SWF::DEFINEALIGNZONES); // 73

	unsigned short ref = in->read_u8(); // must reference a valid DEFINEFONT3 tag
	font* referencedFont = m->get_font(ref);
	if ( ! referencedFont )
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror("DefineFontAlignZones tag references an undefined font %d", ref);
		);
		in->skip_to_tag_end();
		return;
	}

	unsigned flags = in->read_u8(); // 2bits are cms table, 6bits are reserved

	// TODO:
	// 	- parse swf_zone_array
	// 	- construct a DefineFontAlignZonesTag class
	// 	- register the tag with the referencedFont

	IF_VERBOSE_PARSE (
	log_parse("  DefinFontAlignZones: font=%d, flags=%d", ref, flags);
	);

	in->skip_to_tag_end();
	log_error("FIXME: DefineFontAlignZoneTag unfinished ");


}


} // namespace gnash::SWF::tag_loaders
} // namespace gnash::SWF
} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
