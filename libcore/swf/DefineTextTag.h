// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_SWF_DEFINETEXTTAG_H
#define GNASH_SWF_DEFINETEXTTAG_H

#include "character_def.h" // for inheritance
#include "styles.h" 
#include "rect.h" // for composition
#include "swf.h"
#include "movie_definition.h"
#include "SWFMatrix.h"
#include "TextRecord.h"

namespace gnash {
    class movie_definition;
    class SWFStream;
    class RunInfo;
}

namespace gnash {
namespace SWF {


/// Text character 
//
/// This is either read from SWF stream 
/// or (hopefully) created with scripting
///
class DefineTextTag : public character_def
{
public:

    static void loader(SWFStream& in, TagType tag, movie_definition& m, 
            const RunInfo& r);

	/// Draw the string.
	void display(character* inst);
	
	const rect&	get_bound() const {
        // TODO: There is a _matrix field in the definition(!) that's
        // currently ignored. Don't know if it needs to be transformed... 
        return _rect; 
    }

private:

    /// DefineText2Tag::loader also constructs a DefineTextTag.
    friend class DefineText2Tag;

    /// Construct a DefineTextTag.
    //
    /// This should only be constructed using the loader() functions.
	DefineTextTag(SWFStream& in, movie_definition& m, TagType tag)
    {
        read(in, m, tag);
    }

	rect _rect;

    SWFMatrix _matrix;

	void read(SWFStream& in, movie_definition& m, TagType tag);
	
    std::vector<TextRecord> _textRecords;
};

/// Parse a DefineText2Tag.
//
/// This creates a DefineTextTag, as there are only minor differences.
class DefineText2Tag
{
public:
    static void loader(SWFStream& in, TagType tag, movie_definition& m, 
            const RunInfo& r);
};

} // namespace SWF
} // namespace gnash

#endif 
