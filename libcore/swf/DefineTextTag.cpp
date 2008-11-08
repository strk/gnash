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

namespace gnash {
namespace SWF {

void
DefineTextTag::loader(SWFStream& in, tag_type tag, movie_definition& m,
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

void
DefineText2Tag::loader(SWFStream& in, tag_type tag, movie_definition& m,
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
DefineTextTag::read(SWFStream& in, movie_definition&m, tag_type tag)
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

    /// Parse until there are no more records.
	for (;;)
	{
	    TextRecord text;
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
