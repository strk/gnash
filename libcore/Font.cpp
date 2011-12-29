// Font.cpp:  ActionScript Font handling, for Gnash.
// 
//   Copyright (C) 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "Font.h"

#include <utility> 
#include <memory>

#include "log.h"
#include "ShapeRecord.h"
#include "DefineFontTag.h"
#include "FreetypeGlyphsProvider.h"

namespace gnash {

namespace {

/// Reverse lookup of Glyph in CodeTable.
//
/// Inefficient, which is probably why TextSnapshot was designed like it
/// is.
class CodeLookup
{
public:
    CodeLookup(const int glyph) : _glyph(glyph) {}

    bool operator()(const std::pair<const boost::uint16_t, int>& p) const {
        return p.second == _glyph;
    }

private:
    const int _glyph;
};

}

Font::GlyphInfo::GlyphInfo()
    :
    advance(0)
{}

Font::GlyphInfo::GlyphInfo(std::auto_ptr<SWF::ShapeRecord> glyph,
        float advance)
    :
    glyph(glyph.release()),
    advance(advance)
{}

Font::GlyphInfo::GlyphInfo(const GlyphInfo& o)
    :
    glyph(o.glyph),
    advance(o.advance)
{}


Font::Font(std::auto_ptr<SWF::DefineFontTag> ft)
    :
    _fontTag(ft.release()),
    _name(_fontTag->name()),
    _unicodeChars(_fontTag->unicodeChars()),
    _shiftJISChars(_fontTag->shiftJISChars()),
    _ansiChars(_fontTag->ansiChars()),
    _italic(_fontTag->italic()),
    _bold(_fontTag->bold())
{
    if (_fontTag->hasCodeTable()) {
        _embeddedCodeTable = _fontTag->getCodeTable();
    }
}

Font::Font(const std::string& name, bool bold, bool italic)
    :
    _fontTag(0),
    _name(name),
    _unicodeChars(false),
    _shiftJISChars(false),
    _ansiChars(true),
    _italic(italic),
    _bold(bold)
{
    assert(!_name.empty());
}

Font::~Font()
{
}

SWF::ShapeRecord*
Font::get_glyph(int index, bool embedded) const
{
    // What to do if embedded is true and this is a
    // device-only font?
    const GlyphInfoRecords& lookup = (embedded && _fontTag) ? 
            _fontTag->glyphTable() : _deviceGlyphTable;

    if (index >= 0 && (size_t)index < lookup.size()) {
        return lookup[index].glyph.get();
    }

    // TODO: should we log an error here ?
    return 0;
}

void
Font::addFontNameInfo(const FontNameInfo& fontName)
{
    if (!_displayName.empty() || !_copyrightName.empty())
    {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("Attempt to set font display or copyright name "
                    "again. This should mean there is more than one "
                    "DefineFontName tag referring to the same Font. Don't "
                    "know what to do in this case, so ignoring."));
        );
        return;
    }

    _displayName = fontName.displayName;
    _copyrightName = fontName.copyrightName;
}


Font::GlyphInfoRecords::size_type
Font::glyphCount() const
{
        assert(_fontTag);
        return _fontTag->glyphTable().size();
}


void
Font::setFlags(boost::uint8_t flags)
{
    _shiftJISChars = flags & (1 << 6);
    _unicodeChars = flags & (1 << 5);
    _ansiChars = flags & (1 << 4);
    _italic = flags & (1 << 1);
    _bold = flags & (1 << 0);
}


void
Font::setCodeTable(std::auto_ptr<CodeTable> table)
{
    if (_embeddedCodeTable) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("Attempt to add an embedded glyph CodeTable to "
                    "a font that already has one. This should mean there "
                    "are several DefineFontInfo tags, or a DefineFontInfo "
                    "tag refers to a font created by DefineFont2 or "
                    "DefineFont3. Don't know what should happen in this "
                    "case, so ignoring."));
        );
        return;
    }
    _embeddedCodeTable.reset(table.release());
}

    
void
Font::setName(const std::string& name)
{
    _name = name;
}


boost::uint16_t
Font::codeTableLookup(int glyph, bool embedded) const
{
    const CodeTable& ctable = (embedded && _embeddedCodeTable) ? 
        *_embeddedCodeTable : _deviceCodeTable;
    
    CodeTable::const_iterator it = std::find_if(ctable.begin(), ctable.end(),
            CodeLookup(glyph));

    if (it == ctable.end()) {
        // NB: this occurs with a long and complex SWF (bug #32537)
        // that defines the same font twice and ends up with a glyph
        // table shorter than the number of glyphs. We don't know
        // whether it's a SWF or a Gnash bug.
        log_error(_("Failed to find glyph %s in %s font %s"),
                glyph, embedded ? "embedded" : "device", _name);
        return 0;
    }
    return it->first;
}

int
Font::get_glyph_index(boost::uint16_t code, bool embedded) const
{
    const CodeTable& ctable = (embedded && _embeddedCodeTable) ? 
        *_embeddedCodeTable : _deviceCodeTable;

    int glyph_index = -1;
    CodeTable::const_iterator it = ctable.find(code);
    if (it != ctable.end()) {
        glyph_index = it->second;
        return glyph_index;
    }

    // Try adding an os font, if possible
    if (!embedded) {
        glyph_index = const_cast<Font*>(this)->add_os_glyph(code);
    }
    return glyph_index;
}

float
Font::get_advance(int glyph_index, bool embedded) const
{
    // What to do if embedded is true and this is a
    // device-only font?
    const GlyphInfoRecords& lookup = (embedded && _fontTag) ? 
            _fontTag->glyphTable() : _deviceGlyphTable;

    if (glyph_index < 0) {
        // Default advance.
        return 512.0f;
    }

    assert(static_cast<size_t>(glyph_index) < lookup.size());
    assert(glyph_index >= 0);
    
    return lookup[glyph_index].advance;
}


// Return the adjustment in advance between the given two
// DisplayObjects.  Normally this will be 0; i.e. the 
float
Font::get_kerning_adjustment(int last_code, int code) const
{
    kerning_pair k;
    k.m_char0 = last_code;
    k.m_char1 = code;
    kernings_table::const_iterator it = m_kerning_pairs.find(k);
    if (it != m_kerning_pairs.end()) {
        float adjustment = it->second;
        return adjustment;
    }
    return 0;
}

size_t
Font::unitsPerEM(bool embed) const
{
    // the EM square is 1024 x 1024 for DefineFont up to 2
    // and 20 as much for DefineFont3 up
    if (embed) {
        if ( _fontTag && _fontTag->subpixelFont() ) return 1024 * 20.0;
        else return 1024;
    }
    
    FreetypeGlyphsProvider* ft = ftProvider();
    if (!ft) {
        log_error(_("Device font provider was not initialized, "
                    "can't get unitsPerEM"));
        return 0; 
    }

    return ft->unitsPerEM();
}

int
Font::add_os_glyph(boost::uint16_t code)
{
    FreetypeGlyphsProvider* ft = ftProvider();
    if (!ft) return -1;

    assert(_deviceCodeTable.find(code) == _deviceCodeTable.end());

    float advance;

    // Get the vectorial glyph
    std::auto_ptr<SWF::ShapeRecord> sh = ft->getGlyph(code, advance);

    if (!sh.get()) {
        log_error(_("Could not create shape "
                "glyph for DisplayObject code %u (%c) with "
		    "device font %s (%p)"), code, code, _name, ft);
        return -1;
    }

    // Find new glyph offset
    int newOffset = _deviceGlyphTable.size();

    // Add the new glyph id
    _deviceCodeTable[code] = newOffset;

    _deviceGlyphTable.push_back(GlyphInfo(sh, advance));

    return newOffset;
}

bool
Font::matches(const std::string& name, bool bold, bool italic) const
{
    return (_bold == bold && _italic == italic && name ==_name);
}

float
Font::leading() const {
    return _fontTag ? _fontTag->leading() : 0.0f;
}

FreetypeGlyphsProvider*
Font::ftProvider() const 
{
    if (_ftProvider.get()) return _ftProvider.get();

    if (_name.empty()) {
        log_error(_("No name associated with this font, can't use device "
		    "fonts (should I use a default one?)"));
        return 0;
    }

    _ftProvider = FreetypeGlyphsProvider::createFace(_name, _bold, _italic);
    
    if (!_ftProvider.get()) {
        log_error(_("Could not create a freetype face %s"), _name);
        return 0;
    }
    
    return _ftProvider.get();
}

float
Font::ascent(bool embedded) const
{
    if (embedded && _fontTag) return _fontTag->ascent();
    FreetypeGlyphsProvider* ft = ftProvider();
    if (ft) return ft->ascent();
    return 0;
}

float
Font::descent(bool embedded) const
{
    if (embedded && _fontTag) return _fontTag->descent();
    FreetypeGlyphsProvider* ft = ftProvider();
    if (ft) return ft->descent();
    return 0;
}

} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
