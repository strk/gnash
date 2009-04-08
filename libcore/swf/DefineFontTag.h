// DefineFontTag.h   Read DefineFont and DefineFontInfo tags
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
//

// Based on the public domain work of Thatcher Ulrich <tu@tulrich.com> 2003

/// This file handles DefineFont, DefineFontInfo, DefineFontInfo2,
/// DefineFont2, and DefineFont3.
//
/// They are all handled in one file because a DefineFont2 or 3 tag
/// contains practically the same as a DefineFont plus DefineFontInfo
/// or DefineFontInfo2.

#ifndef GNASH_SWF_DEFINEFONTTAG_H
#define GNASH_SWF_DEFINEFONTTAG_H

#include "smart_ptr.h" // GC
#include "swf.h"
#include "Font.h"
#include <vector>

// Forward declarations
namespace gnash {
    class SWFStream;
    class movie_definition;
    class RunInfo;
}

namespace gnash {
namespace SWF {

/// Read and store DefineFont and DefineFont2 tag.
class DefineFontTag
{
public:

    /// Load a DefineFont tag.
    //
    /// A corresponding Font is created and added to the movie_definition.
    static void loader(SWFStream& in, TagType tag, movie_definition& m,
            const RunInfo& r);

    /// Return the glyphs read from the DefineFont tag.
    const Font::GlyphInfoRecords& glyphTable() const {
        return _glyphTable;
    }

    /// Check for the existence of a Font::CodeTable.
    //
    /// @return     True if the tag has a Font::CodeTable.
    //
    /// This can only be true for a DefineFont2 tag.
    bool hasCodeTable() const {
        return _codeTable.get();
    }
    
    /// Retrieve the tag's Font::CodeTable.
    //
    /// This DefineFontTag always retains ownership, and the CodeTable
    /// may not be altered.
    //
    /// @return     shared ownership of the tag's Font::CodeTable. This
    ///             may be NULL, which can be checked first with
    ///             hasCodeTable().
    boost::shared_ptr<const Font::CodeTable> getCodeTable() const {
        return _codeTable;
    }

    bool ansiChars() const { return _ansiChars; }
    bool shiftJISChars() const { return _shiftJISChars; }
    bool unicodeChars() const { return _unicodeChars; }
    bool italic() const { return _italic; }
    bool bold() const { return _bold; }
    bool subpixelFont() const { return _subpixelFont; }
    bool leading() const { return _leading; }
    bool ascent() const { return _ascent; }
    bool descent() const { return _descent; }
    const std::string& name() const { return _name; }

#ifdef GNASH_USE_GC
        /// Mark glyph resources as reachable
        void markReachableResources() const;
#endif

    /// Read Font::CodeTable, which maps glyph indices to DisplayObject codes.
	static void readCodeTable(SWFStream& in, Font::CodeTable& table,
            bool wideCodes, size_t glyphCount);

private:

    DefineFontTag(SWFStream& in, movie_definition& m, TagType tag);

    /// Read a DefineFont tag.
    void readDefineFont(SWFStream& in, movie_definition & m);

    /// Read a DefineFont2 or DefineFont3 tag.
    void readDefineFont2Or3(SWFStream& in, movie_definition& m);

    /// The GlyphInfo records contained in the tag.
    Font::GlyphInfoRecords _glyphTable;

    std::string _name;
    bool _subpixelFont;
	bool _hasLayout;
	bool _unicodeChars;
	bool _shiftJISChars;
	bool _ansiChars;
	bool _italic;
	bool _bold;
	bool _wideCodes;
    float _ascent;
    float _descent;
    float _leading;

	typedef std::map<kerning_pair, float> kernings_table;
	kernings_table m_kerning_pairs;

    boost::shared_ptr<const Font::CodeTable> _codeTable;
};


class DefineFontInfoTag
{
public:
    /// Load a DefineFontInfo tag.
    //
    /// Adds a CodeTable and other information to a Font created by a
    /// DefineFont tag. The information is already contained in a 
    /// DefineFont2 or DefineFont3 tag.
    static void loader(SWFStream& in, TagType tag, movie_definition& m,
            const RunInfo& r);
};

}
}

#endif
