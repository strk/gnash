// StartSoundTag.cpp:  for Gnash.
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

/* $Id: StartSoundTag.cpp,v 1.1 2007/11/23 22:23:25 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "StartSoundTag.h"
#include "stream.h"
#include "movie_definition.h"
#include "log.h" // for log_parse
#include "sound_definition.h" // for sound_sample

namespace gnash {
namespace SWF {

void
StartSoundTag::loader(stream* in, tag_type tag, movie_definition* m)
{
    sound_handler* handler = get_sound_handler();

    assert(tag == SWF::STARTSOUND); // 15 

    uint16_t	sound_id = in->read_u16();

    sound_sample* sam = m->get_sound_sample(sound_id);
    if (sam)
    {
	StartSoundTag*	sst = new StartSoundTag();
	sst->read(in, tag, m, sam);

	IF_VERBOSE_PARSE
	(
	    log_parse(_("start_sound tag: id=%d, stop = %d, loop ct = %d"),
		      sound_id, int(sst->m_stop_playback), sst->m_loop_count);
	);
    }
    else
    {
	if (handler)
	{
	    IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("start_sound_loader: sound_id %d is not defined"), sound_id);
	    );
	}
    }
}

void
StartSoundTag::read(stream* in, int /* tag_type */, movie_definition* m,
		const sound_sample* sam)
{
	assert(sam);

	in->read_uint(2);	// skip reserved bits.
	m_stop_playback = in->read_bit(); 
	bool	no_multiple = in->read_bit(); 
	bool	has_envelope = in->read_bit();
	bool	has_loops = in->read_bit(); 
	bool	has_out_point = in->read_bit(); 
	bool	has_in_point = in->read_bit(); 

	UNUSED(no_multiple);
	UNUSED(has_envelope);
	
	uint32_t	in_point = 0;
	uint32_t	out_point = 0;
	if (has_in_point) { in_point = in->read_u32(); }
	if (has_out_point) { out_point = in->read_u32(); }
	if (has_loops) { m_loop_count = in->read_u16(); }

	if (has_envelope)
	{
		int nPoints = in->read_u8();
		m_envelopes.resize(nPoints);
		for (int i=0; i < nPoints; i++)
		{
			m_envelopes[i].m_mark44 = in->read_u32();
			m_envelopes[i].m_level0 = in->read_u16();
			m_envelopes[i].m_level1 = in->read_u16();
		}
	}
	else
	{
		m_envelopes.resize(0);
	}

	m_handler_id = sam->m_sound_handler_id;
	m->addControlTag(this);
}


void
StartSoundTag::execute(sprite_instance* /* m */) const
{
	// Make static ?
	sound_handler* handler = get_sound_handler();

	//GNASH_REPORT_FUNCTION;

	if (handler)
	{
		if (m_stop_playback)
		{
			handler->stop_sound(m_handler_id);
		}
		else
		{
			handler->play_sound(m_handler_id, m_loop_count, 0,0, (m_envelopes.size() == 0 ? NULL : &m_envelopes));
		}
	}
}

} // namespace gnash::SWF
} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
