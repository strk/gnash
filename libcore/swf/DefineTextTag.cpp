// DefineTextTag.cpp:  Read text character definitions, for Gnash.

// Derived from text.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Code for the text tags.

#include "DefineTextTag.h"
#include "SWFStream.h"
#include "log.h"
#include "swf.h"

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


void DefineTextTag::read(SWFStream& in, movie_definition& m, tag_type tag)
{
	assert(tag == DEFINETEXT || tag == DEFINETEXT2);

	m_rect.read(in);
	m_matrix.read(in);

	in.ensureBytes(2); // glyph_bits + advance_bits
	int glyph_bits = in.read_u8();
	int advance_bits = in.read_u8();

	IF_VERBOSE_PARSE(
	log_parse(_("begin text records for DefineTextTag %p"), (void*)this);
	);

	bool last_record_was_style_change = false;

	text_style	style;
	for (;;)
	{
		in.ensureBytes(1);
		unsigned int first_byte = in.read_u8();
		
		if (first_byte == 0)
		{
			// This is the end of the text records.
			IF_VERBOSE_PARSE(
			log_parse(_("end text records"));
			);
			break;
		}

		// Style changes and glyph records just alternate.
		// (Contrary to what most SWF references say!)
		if (last_record_was_style_change == false)
		{
			// This is a style change.

			last_record_was_style_change = true;

			bool	has_font = (first_byte >> 3) & 1;
			bool	has_color = (first_byte >> 2) & 1;
			bool	has_y_offset = (first_byte >> 1) & 1;
			bool	has_x_offset = (first_byte >> 0) & 1;

			IF_VERBOSE_PARSE(
                log_parse(_("  text style change"));
			);

			if (has_font)
			{
				in.ensureBytes(2);
				boost::uint16_t	font_id = in.read_u16();
				if ( ! style.setFont(font_id, m) )
				{
					// setFont would have already printed an swferror on failure
				}

				IF_VERBOSE_PARSE(
				log_parse(_("  has_font: font id = %d (%p)"), font_id,
                    (const void*)style.getFont());
				);
			} // else reuse previous record font

			if (has_color)
			{
				if (tag == DEFINETEXT) style.m_color.read_rgb(in);
				else style.m_color.read_rgba(in);

				IF_VERBOSE_PARSE(
				    log_parse(_("  has_color"));
				);
			} // else reuse previous record color

			if (has_x_offset)
			{
				in.ensureBytes(2);
				style.setXOffset(in.read_s16());
				IF_VERBOSE_PARSE(
				log_parse(_("  has_x_offset = %g"), style.getXOffset());
				);
			}
			else
			{
				// continue where previous record left
				style.dropXOffset();
			}

			if (has_y_offset)
			{
				in.ensureBytes(2);
				style.setYOffset(in.read_s16());
				IF_VERBOSE_PARSE(
				log_parse(_("  has_y_offset = %g"), style.getYOffset());
				);
			}
			else
			{
				// continue where previous record left
				style.dropYOffset();
			}
			if (has_font)
			{
				in.ensureBytes(2);
				style.m_text_height = in.read_u16();
				IF_VERBOSE_PARSE(
				log_parse(_("  text_height = %g"), style.m_text_height);
				);
			}
		}
		else
		{
			// Read the glyph record.

			last_record_was_style_change = false;

			unsigned int glyph_count = first_byte;

			m_text_glyph_records.resize(m_text_glyph_records.size() + 1);
			text_glyph_record& grecord = m_text_glyph_records.back();
			grecord.m_style = style; // copy current style
			grecord.read(in, glyph_count, glyph_bits, advance_bits);

			IF_VERBOSE_PARSE(
			log_parse(_("  glyph_records: count = %d"), glyph_count);
			for (unsigned int i = 0; i < glyph_count; i++)
			{
				text_glyph_record::glyph_entry& ge = grecord.m_glyphs[i];
				log_parse(_("   glyph%d: index=%d, advance=%g"), i,
                    ge.m_glyph_index, ge.m_glyph_advance);
			}
			);
		}
	}
}

void DefineTextTag::display(character* inst)
{

	const bool useEmbeddedGlyphs = true;

	display_glyph_records(m_matrix, inst, m_text_glyph_records,
            useEmbeddedGlyphs); 
}


}
}	// end namespace gnash
