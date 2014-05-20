// DefineFontTag.cpp: read DefineFont2 and DefineFont tags.
// 
//   Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include "DefineFontTag.h"

#include <memory>
#include <string>

#include "TypesParser.h"
#include "SWFStream.h"
#include "Font.h"
#include "RunResources.h"
#include "SWF.h"
#include "movie_definition.h"
#include "ShapeRecord.h"
#include "log.h"

// Based on the public domain work of Thatcher Ulrich <tu@tulrich.com> 2003

namespace gnash {
namespace SWF {

void
DefineFontTag::loader(SWFStream& in, TagType tag, movie_definition& m,
            const RunResources& r)
{
    assert(tag == DEFINEFONT || tag == DEFINEFONT2 || tag == DEFINEFONT3);

    in.ensureBytes(2);
    const boost::uint16_t fontID = in.read_u16();

    std::unique_ptr<DefineFontTag> ft(new DefineFontTag(in, m, tag, r));
    boost::intrusive_ptr<Font> f(new Font(std::move(ft)));

    m.add_font(fontID, f);
}

void
DefineFontTag::readCodeTable(SWFStream& in, Font::CodeTable& table,
        bool wideCodes, size_t glyphCount)
{
    IF_VERBOSE_PARSE(
        log_parse(_("reading code table at offset %1%, "
                "%2% glyphs"), in.tell(), glyphCount);
    );

    // Good. We can only do this once.
    assert(table.empty());

    if (wideCodes) {
        in.ensureBytes(2 * glyphCount);
        // Code table is made of boost::uint16_t's.
        for (size_t i=0; i < glyphCount; ++i) {
            const boost::uint16_t code = in.read_u16();
            table.insert(std::make_pair(code, i));
        }
    }
    else {
        // Code table is made of bytes.
        in.ensureBytes(1 * glyphCount);
        for (size_t i = 0; i < glyphCount; ++i) {
            const boost::uint8_t code = in.read_u8();
            table.insert(std::make_pair(code, i));
        }
    }
}


DefineFontTag::DefineFontTag(SWFStream& in, movie_definition& m, TagType tag,
        const RunResources& r)
    :
    _subpixelFont(tag == DEFINEFONT3 ? true : false),
    _unicodeChars(false),
    _shiftJISChars(false),
    _ansiChars(true),
    _italic(false),
    _bold(false),
    _ascent(0),
    _descent(0),
    _leading(0)
{
    switch (tag)
    {
        default:
            std::abort();
            break;
        case DEFINEFONT:
            readDefineFont(in, m, r);
            break;
        case DEFINEFONT2:
        case DEFINEFONT3:
            readDefineFont2Or3(in, m, r);
            break;
    }
}

void
DefineFontTag::readDefineFont(SWFStream& in, movie_definition& m,
        const RunResources& r)
{
    IF_VERBOSE_PARSE(
        log_parse(_("reading DefineFont"));
    );

    unsigned long table_base = in.tell();

    // Read the glyph offsets.  Offsets
    // are measured from the start of the
    // offset table.
    std::vector<unsigned> offsets;
    in.ensureBytes(2);
    offsets.push_back(in.read_u16());

    IF_VERBOSE_PARSE (
        log_parse("offset[0] = %d", offsets[0]);
    );

    const size_t count = offsets[0] >> 1;
    if (count > 0) {
        in.ensureBytes(count*2);
        for (size_t i = 1; i < count; ++i) {
            offsets.push_back(in.read_u16());

            IF_VERBOSE_PARSE (
                log_parse("offset[%d] = %d", i, offsets[i]);
            );
        }
    }

    _glyphTable.resize(count);

    // Read the glyph shapes.
    for (size_t i = 0; i < count; ++i) {
        // Seek to the start of the shape data.
        unsigned long new_pos = table_base + offsets[i];

        if ( ! in.seek(new_pos) )
        {
            throw ParserException(_("Glyphs offset table corrupted "
                        "in DefineFont tag"));
        }

        // Create & read the shape.
        _glyphTable[i].glyph.reset(new ShapeRecord(in, SWF::DEFINEFONT, m, r)); 
    }
}

// Read a DefineFont2 or DefineFont3 tag
void
DefineFontTag::readDefineFont2Or3(SWFStream& in, movie_definition& m,
        const RunResources& r)
{
    IF_VERBOSE_PARSE(
        log_parse(_("reading DefineFont2 or DefineFont3"));
    );

    in.ensureBytes(2); // 1 for the flags, 1 unknown
    const int flags = in.read_u8();
    const bool has_layout = flags & (1 << 7);
    _shiftJISChars = flags & (1 << 6);
    _unicodeChars = flags & (1 << 5);
    _ansiChars = flags & (1 << 4);
    const bool wide_offsets = flags & (1 << 3);
    const bool wideCodes = flags & (1 << 2);
    _italic = flags & (1 << 1);
    _bold = flags & (1 << 0);

    // Next is language code, always 0 for SWF5 or previous
    const int languageCode = in.read_u8();
    if (languageCode) {
        LOG_ONCE(log_unimpl("LanguageCode (%d) in DefineFont tag",
                    languageCode));
    }

    in.read_string_with_length(_name);

    in.ensureBytes(2); 
    const boost::uint16_t glyph_count = in.read_u16();

    IF_VERBOSE_PARSE (
        log_parse(" has_layout = %d", has_layout);
        log_parse(" shift_jis_chars = %d", _shiftJISChars);
        log_parse(" m_unicode_chars = %d", _unicodeChars);
        log_parse(" m_ansi_chars = %d", _ansiChars);
        log_parse(" wide_offsets = %d", wide_offsets);
        log_parse(" wide_codes = %d", wideCodes);
        log_parse(" is_italic = %d", _italic);
        log_parse(" is_bold = %d", _bold);
        log_parse(" name = %s", _name);
        log_parse(" glyphs count = %d", glyph_count);
    );
    
    const unsigned long table_base = in.tell();

    // Read the glyph offsets.  Offsets
    // are measured from the start of the
    // offset table. Make sure wide offsets fit into elements
    std::vector<boost::uint32_t> offsets;
    int	font_code_offset;

    if (wide_offsets) {
        // 32-bit offsets.
        in.ensureBytes(4*glyph_count + 4); 
        for (size_t i = 0; i < glyph_count; ++i) {
            const boost::uint32_t off = in.read_u32();	
            IF_VERBOSE_PARSE (
                log_parse(_("Glyph %d at offset %u"), i, off);
            );
            offsets.push_back(off);
        }
        font_code_offset = in.read_u32();
    }
    else {
        // 16-bit offsets.
        in.ensureBytes(2*glyph_count + 2); 
        for (size_t i = 0; i < glyph_count; ++i) {
            const boost::uint16_t off = in.read_u16();	
            IF_VERBOSE_PARSE(
                log_parse(_("Glyph %d at offset %u"), i, off);
            );
            offsets.push_back(off);
        }
        font_code_offset = in.read_u16();
    }

    _glyphTable.resize(glyph_count);

    // Read the glyph shapes.
    for (size_t i = 0; i < glyph_count; ++i) {
        // Seek to the start of the shape data.
        unsigned long new_pos = table_base + offsets[i];

        // It seems completely possible to
        // have such seeks-back, see bug #16311
        if (!in.seek(new_pos)) {
            throw ParserException(_("Glyphs offset table corrupted in "
                        "DefineFont2/3 tag"));
        }

        // Create & read the shape.
        _glyphTable[i].glyph.reset(new ShapeRecord(in, SWF::DEFINEFONT2, m, r));
    }

    const unsigned long current_position = in.tell();
    if (font_code_offset + table_base != current_position) {
        // Bad offset!  Don't try to read any more.
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("Bad offset in DefineFont2"));
        );
        return;
    }

    std::unique_ptr<Font::CodeTable> table(new Font::CodeTable);

    readCodeTable(in, *table, wideCodes, _glyphTable.size());
    _codeTable.reset(table.release());

    // Read layout info for the glyphs.
    if (has_layout) {
        in.ensureBytes(6);
        _ascent = in.read_s16();
        _descent = in.read_s16();
        _leading = in.read_s16();
        
        // Advance table; i.e. how wide each DisplayObject is.
        const size_t nGlyphs = _glyphTable.size();
        in.ensureBytes(nGlyphs*2);

        for (size_t i = 0; i < nGlyphs; i++) {
            // This is documented to be signed, but then we get negative
            // advances for subpixel fonts because the advance overflows
            // int16_t.
            _glyphTable[i].advance = static_cast<float>(in.read_u16());
        }

        for (size_t i = 0; i < nGlyphs; i++) {
            LOG_ONCE(log_unimpl("Bounds table in DefineFont2Tag"));
            readRect(in);
        }

        // Kerning pairs.
        in.ensureBytes(2);
        const boost::uint16_t kerning_count = in.read_u16();

        in.ensureBytes(kerning_count * (wideCodes ? 6 : 4));

        for (int i = 0; i < kerning_count; ++i) {
            boost::uint16_t	char0, char1;
            if (wideCodes) {
                char0 = in.read_u16();
                char1 = in.read_u16();
            }
            else {
                char0 = in.read_u8();
                char1 = in.read_u8();
            }
            const boost::int16_t adjustment = in.read_s16();

            kerning_pair k;
            k.m_char0 = char0;
            k.m_char1 = char1;

            // Remember this adjustment; we can look it up quickly
            // later using the DisplayObject pair as the key.
            if (!_kerningPairs.insert(std::make_pair(k, adjustment)).second) {
                IF_VERBOSE_MALFORMED_SWF(
                    log_swferror(_("Repeated kerning pair found - ignoring"));
                );
            }
        }
    }
}

void
DefineFontInfoTag::loader(SWFStream& in, TagType tag, movie_definition& m,
            const RunResources& /*r*/)
{
    assert(tag == DEFINEFONTINFO || tag == DEFINEFONTINFO2); 

    in.ensureBytes(2);
    const boost::uint16_t fontID = in.read_u16();

    Font* f = m.get_font(fontID);
    if (!f) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("DefineFontInfo tag loader: "
                   "can't find font with id %d"), fontID);
        );
        return;
    }

    if (tag == DEFINEFONTINFO2) {
        // See: SWFalexref/SWFalexref.html#tag_definefont2
        LOG_ONCE(log_unimpl(_("DefineFontInfo2 partially implemented")));
    }

    std::string name;
    in.read_string_with_length(name);

    in.ensureBytes(1);
    const boost::uint8_t flags = in.read_u8();

    const bool wideCodes = flags & (1 << 0);

    std::unique_ptr<Font::CodeTable> table(new Font::CodeTable);

    DefineFontTag::readCodeTable(in, *table, wideCodes, f->glyphCount());

    f->setName(name);
    f->setFlags(flags);
    f->setCodeTable(std::move(table));
}


}
}

