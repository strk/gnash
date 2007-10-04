// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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
//
//

/* $Id: DoActionTag.h,v 1.6 2007/10/04 22:55:53 strk Exp $ */

#ifndef GNASH_SWF_DOACTIONTAG_H
#define GNASH_SWF_DOACTIONTAG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "execute_tag.h" // for inheritance
#include "swf.h" // for tag_type definition
#include "action_buffer.h" // for composition
#include "sprite_instance.h" // for inlines
//#include "tu_types.h" // for uint16_t, uint32_t etc. definition 

// Forward declarations
namespace gnash {
	class stream;
	class movie_definition;
}

namespace gnash {
namespace SWF {

/// SWF Tag DoAction (12) and DoInitAction (59)
//
/// Thin wrapper around action_buffer.
///
class DoActionTag : public execute_tag
{
public:

	DoActionTag(bool isInitAction)
		:
		_isInitAction(isInitAction)
	{}

	/// Read a DoAction block from the stream
	//
	void read(stream* in)
	{
	    m_buf.readFullTag(in);
	}

	virtual void execute(sprite_instance* m) const
	{
		if ( _isInitAction )
		{
			m->execute_action(m_buf);
		}
		else
		{
			//m->queueActions(m_buf);
	    		m->add_action_buffer(&m_buf);
		}
	}

	// Tell the caller that we are an action tag.
	virtual bool is_action_tag() const
	{
	    return true;
	}

	static void doActionLoader(stream* in, tag_type tag, movie_definition* m)
	{
		DoActionTag* da = new DoActionTag(false);
		da->read(in);

		IF_VERBOSE_PARSE (
		log_parse(_("tag %d: do_action_loader"), tag);
		log_parse(_("-- actions in frame " SIZET_FMT), m->get_loading_frame());
		);

		m->add_execute_tag(da); // ownership transferred
	}

	static void doInitActionLoader(stream* in, tag_type tag, movie_definition* m)
	{
		DoActionTag* da = new DoActionTag(true);
		int cid = in->read_u16();
		da->read(in);

		IF_VERBOSE_PARSE (
		log_parse(_("  tag %d: do_init_action_loader"), tag);
		log_parse(_("  -- init actions for sprite %d"), cid);
		);

		m->add_init_action(da, cid); // ownership transferred
	}

private:

	bool _isInitAction;

	action_buffer m_buf;
};

} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_DOACTIONTAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
