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

#ifndef GNASH_SWF_DOACTIONTAG_H
#define GNASH_SWF_DOACTIONTAG_H

#include "ControlTag.h" // for inheritance
#include "SWF.h" // for TagType definition
#include "action_buffer.h" // for composition
#include "MovieClip.h" // for inlines
#include "SWFStream.h" // for inlines

// Forward declarations
namespace gnash {
	class movie_definition;
}

namespace gnash {
namespace SWF {

/// SWF Tag DoAction (12) 
//
/// Thin wrapper around action_buffer.
///
class DoActionTag : public ControlTag
{
public:

	/// Read a DoAction block from the stream
	//
	void read(SWFStream& in) {
        m_buf.read(in, in.get_tag_end_position());
	}

	virtual void executeActions(MovieClip* m, DisplayList& /* dlist */) const {
        m->add_action_buffer(&m_buf);
	}

	static void loader(SWFStream& in, TagType tag, movie_definition& m,
            const RunResources& /*r*/)
	{
        if (m.isAS3()) {
            IF_VERBOSE_MALFORMED_SWF(
                log_swferror("SWF contains DoAction tag, but is an "
                    "AS3 SWF!");
            );
            throw ParserException("DoAction tag found in AS3 SWF!");
        }
        
		boost::intrusive_ptr<DoActionTag> da(new DoActionTag(m));
		da->read(in);

		IF_VERBOSE_PARSE(
            log_parse(_("tag %d: do_action_loader"), tag);
            log_parse(_("-- actions in frame %d"), m.get_loading_frame());
		);

		m.addControlTag(da); // ownership transferred
	}

private:

	DoActionTag(movie_definition& md)
		:
		m_buf(md)
	{}

	action_buffer m_buf;
};

} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_DOACTIONTAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
