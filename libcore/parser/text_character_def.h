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

// 
//
//

// Code for the text tags.


#ifndef GNASH_PARSER_TEXT_CHARACTER_DEF_H
#define GNASH_PARSER_TEXT_CHARACTER_DEF_H

#include "character_def.h" // for inheritance
#include "text.h" // for text_glyph_record
#include "styles.h" 
#include "rect.h" // for composition

namespace gnash {

class movie_definition; // for read signature
class SWFStream; // for read signature

/// Text character 
//
/// This is either read from SWF stream 
/// or (hopefully) created with scripting
///
class text_character_def : public character_def
{
public:
	rect	m_rect;
	SWFMatrix	m_matrix;
	std::vector<text_glyph_record>	m_text_glyph_records;

	text_character_def() {}

	void read(SWFStream& in, int tag_type, movie_definition& m);

	/// Draw the string.
	void display(character* inst);
	
	const rect&	get_bound() const {
    // TODO: There is a m_matrix field in the definition(!) that's currently 
    // ignored. Don't know if it needs to be transformed... 
    return m_rect; 
  }
	
};


} // namespace gnash

#endif // GNASH_PARSER_TEXT_CHARACTER_DEF_H
