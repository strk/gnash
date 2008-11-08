// font.cpp:  ActionScript font handling, for Gnash.
// 
//   Copyright (C) 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include "smart_ptr.h" // GNASH_USE_GC
#include "font.h"
#include "SWFStream.h"
#include "log.h"
#include "movie_definition.h"
#include "shape_character_def.h"
#include "swf.h"
#include "GnashException.h"
#include "utility.h" // for UNUSED

#include <utility> // for std::make_pair

namespace gnash {


font::GlyphInfo::GlyphInfo()
	:
	glyph(),
	advance(0)
{}

font::GlyphInfo::GlyphInfo(boost::intrusive_ptr<shape_character_def> glyph,
        float advance)
	:
	glyph(glyph.get()),
	advance(advance)
{}

font::GlyphInfo::GlyphInfo(const GlyphInfo& o)
	:
	glyph(o.glyph.get()),
	advance(o.advance)
{}

#ifdef GNASH_USE_GC
void
font::GlyphInfo::markReachableResources() const
{
	if ( glyph ) glyph->setReachable();
}
#endif

font::font()
    :
    m_name(),
            m_display_name(),
            m_copyright_name(),
    m_unicode_chars(false),
    m_shift_jis_chars(false),
    m_ansi_chars(true),
    m_is_italic(false),
    m_is_bold(false),
    m_wide_codes(false),
    m_subpixel_font(false),
    m_ascent(0.0f),
    m_descent(0.0f),
    m_leading(0.0f)
{
}

font::font(const std::string& name, bool bold, bool italic)
    :
    m_name(name),
            m_display_name(),
            m_copyright_name(),
    m_unicode_chars(false),
    m_shift_jis_chars(false),
    m_ansi_chars(true),
    m_is_italic(italic),
    m_is_bold(bold),
    m_wide_codes(false),
    m_subpixel_font(false),
    m_ascent(0.0f),
    m_descent(0.0f),
    m_leading(0.0f)
{
    assert(!m_name.empty());
}

font::~font()
{
}

shape_character_def*	font::get_glyph(int index, bool embedded) const
{
    const GlyphInfoVect& lookup = embedded ? _embedGlyphTable : _deviceGlyphTable;

    if (index >= 0 && (size_t)index < lookup.size())
    {
        return lookup[index].glyph.get();
    }
    else
    {
        // TODO: should we log an error here ?
        return NULL;
    }
}


void	font::read(SWFStream& in, SWF::tag_type tag, movie_definition& m)
{
    if (tag == SWF::DEFINEFONT)
    {
        readDefineFont(in, m);
    }
    else
    {
        assert (tag == SWF::DEFINEFONT2 || tag == SWF::DEFINEFONT3);
        readDefineFont2_or_3(in, m);
        if (tag == SWF::DEFINEFONT3)
        {
            m_subpixel_font = true;
        }
    }

}

// Read a DefineFont tag
void font::readDefineFont(SWFStream& in, movie_definition& m)
{
    IF_VERBOSE_PARSE (
    log_parse(_("reading DefineFont"));
    );

    unsigned long table_base = in.tell();

    // Read the glyph offsets.  Offsets
    // are measured from the start of the
    // offset table.
    std::vector<unsigned>	offsets;
    in.ensureBytes(2);
    offsets.push_back(in.read_u16());

    IF_VERBOSE_PARSE (
    log_parse("offset[0] = %d", offsets[0]);
    );

    int	count = offsets[0] >> 1;
    if ( count > 0 )
    {
        in.ensureBytes(count*2);
        for (int i = 1; i < count; i++)
        {
            offsets.push_back(in.read_u16());

            IF_VERBOSE_PARSE (
            log_parse("offset[%d] = %d", i, offsets[i]);
            );
        }
    }
    else
    {
        log_error("Negative embedded glyph table size: %d", count);
    }

    _embedGlyphTable.resize(count);

    // Read the glyph shapes.
    for (int i = 0; i < count; i++)
    {
        // Seek to the start of the shape data.
        unsigned long new_pos = table_base + offsets[i];

        if ( ! in.seek(new_pos) )
        {
            throw ParserException(_("Glyphs offset table corrupted "
                        "in DefineFont tag"));
        }

        // Create & read the shape.
        shape_character_def* s = new shape_character_def;
        s->read(in, SWF::DEFINEFONT, false, m); 

        _embedGlyphTable[i].glyph = s;
    }
}

// Read a DefineFont2 or DefineFont3 tag
void font::readDefineFont2_or_3(SWFStream& in, movie_definition& m)
{
    IF_VERBOSE_PARSE (
    log_parse(_("reading DefineFont2 or DefineFont3"));
    );

    in.ensureBytes(2); // 1 for the flags, 1 unknown
    int flags = in.read_u8();
    bool  has_layout   = flags & (1 << 7);
    m_shift_jis_chars  = flags & (1 << 6);
    m_unicode_chars    = flags & (1 << 5);
    m_ansi_chars       = flags & (1 << 4);
    bool  wide_offsets = flags & (1 << 3);
    m_wide_codes       = flags & (1 << 2);
    m_is_italic        = flags & (1 << 1);
    m_is_bold          = flags & (1 << 0);

    // Next is language code, always 0 for SWF5 or previous
    int languageCode = in.read_u8();
    if (languageCode) {
        LOG_ONCE(log_unimpl("LanguageCode in DefineFont (2 or 3)"));
    }

    in.read_string_with_length(m_name);

    in.ensureBytes(2); 
    boost::uint16_t glyph_count = in.read_u16();

    IF_VERBOSE_PARSE (
        log_parse(" has_layout = %d", has_layout);
    log_parse(" shift_jis_chars = %d", m_shift_jis_chars);
        log_parse(" m_unicode_chars = %d", m_unicode_chars);
        log_parse(" m_ansi_chars = %d", m_ansi_chars);
        log_parse(" wide_offsets = %d", wide_offsets);
        log_parse(" wide_codes = %d", m_wide_codes);
        log_parse(" is_italic = %d", m_is_italic);
        log_parse(" is_bold = %d", m_is_bold);
        log_parse(" name = %s", m_name);
        log_parse(" glyphs count = %d", glyph_count);
    );

    
    unsigned long table_base = in.tell();

    // Read the glyph offsets.  Offsets
    // are measured from the start of the
    // offset table. Make sure wide offsets fit into elements
    std::vector<boost::uint32_t>	offsets;
    int	font_code_offset;
    if (wide_offsets)
    {
        // 32-bit offsets.
        in.ensureBytes(4*glyph_count + 4); 
        for (unsigned int i = 0; i < glyph_count; i++)
        {
            boost::uint32_t off = in.read_u32();	

            IF_VERBOSE_PARSE (
            log_parse(_("Glyph %d at offset %u"), i, off);
            );

            offsets.push_back(off);
        }
        font_code_offset = in.read_u32();
    }
    else
    {
        // 16-bit offsets.
        in.ensureBytes(2*glyph_count + 2); 
        for (unsigned int i = 0; i < glyph_count; i++)
        {
            boost::uint16_t off = in.read_u16();	

            IF_VERBOSE_PARSE (
            log_parse(_("Glyph %d at offset %u"), i, off);
            );

            offsets.push_back(off);
        }
        font_code_offset = in.read_u16();
    }

    _embedGlyphTable.resize(glyph_count);

    // Read the glyph shapes.
    for (int i = 0; i < glyph_count; i++)
    {
        // Seek to the start of the shape data.
        unsigned long new_pos = table_base + offsets[i];

        // It seems completely possible to
        // have such seeks-back, see bug #16311
        //assert(new_pos >= in.tell());

        if ( ! in.seek(new_pos) )
        {
            throw ParserException(_("Glyphs offset table corrupted in DefineFont2/3 tag"));
        }

        // Create & read the shape.
        shape_character_def* s = new shape_character_def;
        s->read(in, SWF::DEFINEFONT2, false, m); // .. or DEFINEFONT3 actually..

        _embedGlyphTable[i].glyph = s;
    }

    unsigned long current_position = in.tell();
    if (font_code_offset + table_base != current_position)
    {
        // Bad offset!  Don't try to read any more.
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("Bad offset in DefineFont2"));
        );
        return;
    }

    read_code_table(in);

    // Read layout info for the glyphs.
    if (has_layout)
    {
        in.ensureBytes(6);
        m_ascent = (float) in.read_s16();
        m_descent = (float) in.read_s16();
        m_leading = (float) in.read_s16();
        
        // Advance table; i.e. how wide each character is.
        size_t nGlyphs = _embedGlyphTable.size();
        in.ensureBytes(nGlyphs*2);
        for (size_t i = 0; i < nGlyphs; i++)
        {
            _embedGlyphTable[i].advance = (float) in.read_s16();
        }

        // Bounds table.
        {
            rect	dummy_rect;
            // TODO: shouldn't we log_unimpl here ??
            for (size_t i = 0; i < nGlyphs; i++) dummy_rect.read(in);
        }

        // Kerning pairs.
        in.ensureBytes(2);
        int	kerning_count = in.read_u16();
        if ( m_wide_codes )
        {
            in.ensureBytes(6*kerning_count); // includes the adjustment 
        }
        else
        {
            in.ensureBytes(4*kerning_count); // includes the adjustment 
        }

        for (int i = 0; i < kerning_count; i++)
        {
            boost::uint16_t	char0, char1;
            if (m_wide_codes)
            {
                char0 = in.read_u16();
                char1 = in.read_u16();
            }
            else
            {
                char0 = in.read_u8();
                char1 = in.read_u8();
            }
            float	adjustment = (float) in.read_s16();

            kerning_pair	k;
            k.m_char0 = char0;
            k.m_char1 = char1;

// Remember this adjustment; we can look it up quickly
// later using the character pair as the key.
if ( ! m_kerning_pairs.insert(std::make_pair(k, adjustment)).second )
{
    IF_VERBOSE_MALFORMED_SWF(
    log_swferror(_("Repeated kerning pair found - ignoring"));
    );
}

        }
    }
}

// Read the font name, display and legal, from a DefineFontName tag.
void font::read_font_name(SWFStream& in, SWF::tag_type tag,
    movie_definition& /*m*/) 
{
    assert(tag == SWF::DEFINEFONTNAME);
    in.read_string(m_display_name);
    in.read_string(m_copyright_name);
}

// Read additional information about this font, from a
// DefineFontInfo tag.  The caller has already read the tag
// type and font id.
void	font::read_font_info(SWFStream& in, SWF::tag_type tag,
        movie_definition& /*m*/)
{
    assert(tag == SWF::DEFINEFONTINFO || tag == SWF::DEFINEFONTINFO2); 

    if ( tag == SWF::DEFINEFONTINFO2 )
    {
        // See: SWFalexref/SWFalexref.html#tag_definefont2
        LOG_ONCE(log_unimpl(_("DefineFontInfo2 partially implemented")));
    }

    in.read_string_with_length(m_name);

    in.ensureBytes(1);
    int	flags = in.read_u8();
    // highest two bits are reserved.
    m_unicode_chars   = flags & (1 << 5); //???
    m_shift_jis_chars = flags & (1 << 4);
    m_ansi_chars      = flags & (1 << 3);
    m_is_italic       = flags & (1 << 2);
    m_is_bold         = flags & (1 << 1);
    m_wide_codes      = flags & (1 << 0);

    read_code_table(in);
}

void	font::read_code_table(SWFStream& in)
{
    IF_VERBOSE_PARSE (
    log_parse(_("reading code table at offset %lu"), in.tell());
    );

    assert(_embedded_code_table.empty());

    size_t nGlyphs = _embedGlyphTable.size();
    if (m_wide_codes)
    {
        in.ensureBytes(2*nGlyphs);
        // Code table is made of boost::uint16_t's.
        for (size_t i=0; i<nGlyphs; ++i)
        {
            boost::uint16_t code = in.read_u16();
            _embedded_code_table.insert(std::make_pair(code, i));
        }
    }
    else
    {
        // Code table is made of bytes.
        in.ensureBytes(1*nGlyphs);
        for (size_t i=0; i<nGlyphs; ++i)
        {
            boost::uint8_t code = in.read_u8();
            _embedded_code_table.insert(std::make_pair(code, i));
        }
    }
}

int	font::get_glyph_index(boost::uint16_t code, bool embedded) const
{
    const code_table& ctable = embedded ? _embedded_code_table : _device_code_table;

    int glyph_index = -1;
    code_table::const_iterator it = ctable.find(code);
    if ( it != ctable.end() )
    {
        glyph_index = it->second;
        return glyph_index;
    }

    // Try adding an os font, of possible
    if ( ! embedded )
    {
        glyph_index = const_cast<font*>(this)->add_os_glyph(code);
    }
    return glyph_index;
}

float	font::get_advance(int glyph_index, bool embedded) const
{
    const GlyphInfoVect& lookup = embedded ? _embedGlyphTable : _deviceGlyphTable;

    if (glyph_index <= -1)
    {
        // Default advance.
        return 512.0f;
    }

    if ((size_t)glyph_index < lookup.size())
    {
        assert(glyph_index >= 0);
        return lookup[glyph_index].advance;
    }
    else
    {
        // Bad glyph index.  Due to bad data file?
        abort();
        return 0;
    }
}


// Return the adjustment in advance between the given two
// characters.  Normally this will be 0; i.e. the 
float	font::get_kerning_adjustment(int last_code, int code) const
{
    kerning_pair	k;
    k.m_char0 = last_code;
    k.m_char1 = code;
    kernings_table::const_iterator it = m_kerning_pairs.find(k);
    if ( it != m_kerning_pairs.end() )
    {
        float	adjustment = it->second;
        return adjustment;
    }
    return 0;
}

unsigned short int font::unitsPerEM(bool embed) const
{
    // the EM square is 1024 x 1024 for DefineFont up to 2
    // and 20 as much for DefineFont3 up
    if ( embed )
    {
        if ( m_subpixel_font ) return 1024*20;
        else return 1024;
    }
    else
    {
        if ( ! _ftProvider.get() )
        {
            if ( ! initDeviceFontProvider() )
            {
                log_error("Device font provider was not initialized, "
                        "can't get unitsPerEM");
                return 0; // can't query it..
            }
        }
        return _ftProvider->unitsPerEM();
    }
}

int
font::add_os_glyph(boost::uint16_t code)
{
    if ( ! _ftProvider.get() )
    {
        if ( ! initDeviceFontProvider() )
        {
            log_error("Device font provider was not initialized, can't "
                    "get unitsPerEM");
            return -1; // can't provide it...
        }
    }

    assert(_device_code_table.find(code) == _device_code_table.end());

    float advance;

    // Get the vectorial glyph
    boost::intrusive_ptr<shape_character_def> sh = _ftProvider->getGlyph(code, advance);

    if ( ! sh )
    {
        log_error("Could not create shape "
                "glyph for character code %u (%c) with "
                "device font %s (%p)", code, code, m_name.c_str(),
                _ftProvider.get());
        return -1;
    }

    // Find new glyph offset
    int newOffset = _deviceGlyphTable.size();

    // Add the new glyph id
    _device_code_table[code] = newOffset;

    _deviceGlyphTable.push_back(GlyphInfo(sh, advance));

    testInvariant();

    return newOffset;
}

bool
font::initDeviceFontProvider() const
{
    if ( m_name.empty() )
    {
        log_error("No name associated with this font, can't use device fonts (should I use a default one?)");
        return false;
    }

    _ftProvider = FreetypeGlyphsProvider::createFace(m_name.c_str(), m_is_bold, m_is_italic);
    if ( ! _ftProvider.get() )
    {
        log_error("Could not create a freetype face %s", m_name.c_str());
        return false;
    }
    return true;
}

bool
font::matches(const std::string& name, bool bold, bool italic) const
{
	if ( bold != isBold() ) return false;
	if ( italic != isItalic() ) return false;
 	if ( name != get_name() ) return false;

	return true;
}

#ifdef GNASH_USE_GC
/// Mark reachable resources (for the GC)
//
/// Reachable resources are:
///	- shape_character_defs (vector glyphs, devide and embeded)
///
void
font::markReachableResources() const
{
	// Mark embed glyphs (textured and vector)
	for (GlyphInfoVect::const_iterator i=_embedGlyphTable.begin(), e=_embedGlyphTable.end(); i!=e; ++i)
	{
		i->markReachableResources();
	}

	// Mark device glyphs (textured and vector)
	for (GlyphInfoVect::const_iterator i=_deviceGlyphTable.begin(), e=_deviceGlyphTable.end(); i!=e; ++i)
	{
		i->markReachableResources();
	}

}
#endif // GNASH_USE_GC


}	// end namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
