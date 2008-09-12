// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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
#include "swf.h" // for tag_type definition
#include "action_buffer.h" // for composition
#include "sprite_instance.h" // for inlines
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

	DoActionTag(movie_definition& md)
		:
		m_buf(md)
	{}

	/// Read a DoAction block from the stream
	//
	void read(SWFStream& in)
	{
            m_buf.read(in, in.get_tag_end_position());
	}

	virtual void execute(sprite_instance* m, DisplayList& /* dlist */) const
	{
	    	m->add_action_buffer(&m_buf);
	}

	// Tell the caller that we are an action tag.
	virtual bool is_action_tag() const
	{
	    return true;
	}

	static void doActionLoader(SWFStream& in, tag_type tag, movie_definition& m)
	{
		DoActionTag* da = new DoActionTag(m);
		da->read(in);

		IF_VERBOSE_PARSE (
		log_parse(_("tag %d: do_action_loader"), tag);
		log_parse(_("-- actions in frame %d"), m.get_loading_frame());
		);

		m.addControlTag(da); // ownership transferred
	}

private:

	action_buffer m_buf;
};

} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_DOACTIONTAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
