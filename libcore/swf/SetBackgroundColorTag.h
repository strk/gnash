// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#ifndef GNASH_SWF_SETBACKGROUNDCOLOR_TAG_H
#define GNASH_SWF_SETBACKGROUNDCOLOR_TAG_H

#include "ControlTag.h" 
#include "TypesParser.h"
#include "SWF.h" 
#include "MovieClip.h" 
#include "movie_definition.h" 
#include "log.h" 
#include "RGBA.h" 

// Forward declarations
namespace gnash {
	class SWFStream;
	class movie_definition;
}

namespace gnash {
namespace SWF {

/// SWF Tag SetBackgroundColor (9)
//
class SetBackgroundColorTag : public ControlTag
{

private:

	/// Actual color. So far only RGB values are used,
	/// as alpha is not encoded in the tag itself
	rgba m_color;

	/// Read SetBackgroundColorTag from the given stream
	//
	/// Tag header is assumed to have been read already
	///
	/// Can throw ParserException on premature end of input stream
	///
	void read(SWFStream& in)
	{
		// may throw ParserException
		m_color = readRGB(in);

		IF_VERBOSE_PARSE(
            log_parse(_("  SetBackgroundColor: %s"), m_color);
		);
	}


public:

	/// \brief
	/// Construct a SetBackgroundColorTag by reading it 
	/// from the given SWF stream.
	//
	/// Tag header is assumed to have been read already
	///
	/// Can throw ParserException on premature end of input stream
	SetBackgroundColorTag(SWFStream& in)
    {
		read(in);
	}

	void executeState(MovieClip* m, DisplayList& /*dlist*/) const {
		m->set_background_color(m_color);
	}

	/// Set background color tag loader (SWF::SETBACKGROUNDCOLOR)
	static void loader(SWFStream& in, TagType tag, movie_definition& m, 
            const RunResources& /*r*/)
	{
		assert(tag == SWF::SETBACKGROUNDCOLOR); // 9

		// this one may throw, we'll let caller catch it
		boost::intrusive_ptr<ControlTag> t(new SetBackgroundColorTag(in));
		m.addControlTag(t); // takes ownership
	}
};




} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_SETBACKGROUNDCOLOR_TAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
