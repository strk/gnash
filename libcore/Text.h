// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

// Code for the text tags.


#ifndef GNASH_TEXT_H
#define GNASH_TEXT_H

#include "TextRecord.h"

namespace gnash {
	class SWFMatrix;

}

namespace gnash {

/// Render the given glyph records.
//
/// @param useEmbeddedGlyphs
///	If true, the font will be queried for embedded glyphs.
///	Otherwise, the font will be queried for device fonts.
///
void display_glyph_records(const SWFMatrix& this_mat, character* inst,
    const std::vector<SWF::TextRecord>& records,
    bool useEmbeddedGlyphs);

} // namespace gnash

#endif // GNASH_TEXT_H
