// DefineFontTag.h   Read DefineFont and DefineFont2 tags
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

// Based on the public domain work of Thatcher Ulrich <tu@tulrich.com> 2003


#ifndef GNASH_SWF_DEFINEFONTTAG_H
#define GNASH_SWF_DEFINEFONTTAG_H

#include "smart_ptr.h" // GC
#include "swf.h"
#include "Font.h"
#include <vector>
#include <map>

// Forward declarations
namespace gnash {
    class SWFStream;
    class movie_definition;
    class RunInfo;
}

namespace gnash {
namespace SWF {

class DefineFontTag
{
public:
    static void loader(SWFStream& in, tag_type tag, movie_definition& m,
            const RunInfo& r);

    const Font::GlyphInfoRecords& glyphTable() const {
        return _glyphTable;
    }

    bool hasCodeTable() const {
        return _codeTable.get();
    }
    
    boost::shared_ptr<const Font::CodeTable> getCodeTable() const {
        return _codeTable;
    }

    bool ansiChars() const { return _ansiChars; }
    bool shiftJISChars() const { return _shiftJISChars; }
    bool unicodeChars() const { return _unicodeChars; }
    bool italic() const { return _italic; }
    bool bold() const { return _bold; }
    bool wideCodes() const { return _wideCodes; }
    bool subpixelFont() const { return _subpixelFont; }
    bool leading() const { return _leading; }
    bool ascent() const { return _ascent; }
    bool descent() const { return _descent; }
    const std::string& name() const { return _name; }

#ifdef GNASH_USE_GC
        /// Mark glyph resources as reachable
        void markReachableResources() const;
#endif

    /// Read the table that maps from glyph indices to character codes.
	static void readCodeTable(SWFStream& in, Font::CodeTable& table,
            bool wideCodes, size_t glyphCount);

private:

    DefineFontTag(SWFStream& in, movie_definition& m, tag_type tag);

    void readDefineFont(SWFStream& in, movie_definition & m);

    void readDefineFont2Or3(SWFStream& in, movie_definition& m);

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

}
}

#endif
