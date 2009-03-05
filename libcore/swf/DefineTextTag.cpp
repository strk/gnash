// DefineTextTag.cpp:  Read text character definitions, for Gnash.

// Derived from text.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Code for the text tags.

#include "DefineTextTag.h"
#include "SWFStream.h"
#include "log.h"
#include "swf.h"
#include "TextRecord.h"
#include "Font.h"

namespace gnash {
namespace SWF {

void
DefineTextTag::loader(SWFStream& in, TagType tag, movie_definition& m,
        const RunInfo& /*r*/)
{
    assert(tag == DEFINETEXT);

    in.ensureBytes(2);
    boost::uint16_t id = in.read_u16();

    std::auto_ptr<DefineTextTag> t(new DefineTextTag(in, m, tag));
    IF_VERBOSE_PARSE(
        log_parse(_("Text character, id = %d"), id);
    );

    m.add_character(id, t.release());
}

class DecodeRecord
{
public:

    DecodeRecord(std::string& to) : _to(to) {}

    void operator()(const TextRecord& tr) {

        const Font* font = tr.getFont();
        if (!font) return;
        
        const TextRecord::Glyphs& glyphs = tr.glyphs();

        for (TextRecord::Glyphs::const_iterator it = glyphs.begin(),
                e = glyphs.end(); it != e; ++it) {
            _to += font->codeTableLookup(it->index, true);
        }
    }
private:
    std::string& _to;
};

void
DefineTextTag::extractStaticText(std::string& to)
{
    std::for_each(_textRecords.begin(), _textRecords.end(), DecodeRecord(to)); 
}

void
DefineText2Tag::loader(SWFStream& in, TagType tag, movie_definition& m,
        const RunInfo& /*r*/)
{
    assert(tag == DEFINETEXT2);

    in.ensureBytes(2);
    boost::uint16_t id = in.read_u16();

    std::auto_ptr<DefineTextTag> t(new DefineTextTag(in, m, tag));
    IF_VERBOSE_PARSE(
        log_parse(_("Text character, id = %d"), id);
    );

    m.add_character(id, t.release());
}

void
DefineTextTag::read(SWFStream& in, movie_definition&m, TagType tag)
{
	assert(tag == DEFINETEXT || tag == DEFINETEXT2);

	_rect.read(in);
	_matrix.read(in);

	in.ensureBytes(2); // glyph_bits + advance_bits
	int glyphBits = in.read_u8();
	int advanceBits = in.read_u8();

	IF_VERBOSE_PARSE(
	    log_parse(_("begin text records for DefineTextTag %p"), (void*)this);
	);

    /// Parse until there are no more records. Each new TextRecord
    /// uses the values from the previous one unless they are overridden.
    TextRecord text;
	for (;;)
	{
        if (!text.read(in, m, glyphBits, advanceBits, tag)) break;
        _textRecords.push_back(text);
	}
}

void
DefineTextTag::display(character* inst)
{

	const bool useEmbeddedGlyphs = true;

    TextRecord::displayRecords(_matrix, inst, _textRecords,
            useEmbeddedGlyphs); 
}


}
}	// end namespace gnash
