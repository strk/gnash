// DefineTextTag.cpp:  Read StaticText definitions, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

// Derived from text.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

#include "RunResources.h"
#include "DefineTextTag.h"
#include "SWFStream.h"
#include "log.h"
#include "SWF.h"
#include "TextRecord.h"
#include "Font.h"
#include "StaticText.h"
#include "GnashAlgorithm.h"
#include "Global_as.h"
#include "movie_definition.h"

#include <algorithm>
#include <numeric>

namespace gnash {
namespace SWF {

void
DefineTextTag::loader(SWFStream& in, TagType tag, movie_definition& m,
        const RunResources& /*r*/)
{
    assert(tag == DEFINETEXT);

    in.ensureBytes(2);
    const boost::uint16_t id = in.read_u16();

    std::auto_ptr<DefineTextTag> t(new DefineTextTag(in, m, tag, id));
    IF_VERBOSE_PARSE(
        log_parse(_("DefineTextTag, id = %d"), id);
    );

    m.addDisplayObject(id, t.release());
}

DisplayObject*
DefineTextTag::createDisplayObject(Global_as& gl, DisplayObject* parent)
    const
{
    return new StaticText(getRoot(gl), 0, this, parent);
}

bool
DefineTextTag::extractStaticText(std::vector<const TextRecord*>& to,
        size_t& numChars) const
{
    if (_textRecords.empty()) return false;

    /// Insert pointers to all our TextRecords into to.
    std::transform(_textRecords.begin(), _textRecords.end(),
            std::back_inserter(to), CreatePointer<TextRecord>());

    /// Count the number of DisplayObjects in this definition's text records.
    numChars = std::accumulate(_textRecords.begin(), _textRecords.end(),
            0, TextRecord::RecordCounter());

    return true;
}

void
DefineText2Tag::loader(SWFStream& in, TagType tag, movie_definition& m,
        const RunResources& /*r*/)
{
    assert(tag == DEFINETEXT2);

    in.ensureBytes(2);
    const boost::uint16_t id = in.read_u16();

    std::auto_ptr<DefineTextTag> t(new DefineTextTag(in, m, tag, id));
    IF_VERBOSE_PARSE(
        log_parse(_("Text DisplayObject, id = %d"), id);
    );

    m.addDisplayObject(id, t.release());
}

void
DefineTextTag::read(SWFStream& in, movie_definition&m, TagType tag)
{
	assert(tag == DEFINETEXT || tag == DEFINETEXT2);

	_rect.read(in);
	_matrix = readSWFMatrix(in);

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
DefineTextTag::display(Renderer& renderer, const StaticText& inst) const
{

    SWFMatrix mat = inst.getWorldMatrix();
    mat.concatenate(_matrix);

    TextRecord::displayRecords(renderer, mat, inst.get_world_cxform(),
            _textRecords);
}


}
}	// end namespace gnash
