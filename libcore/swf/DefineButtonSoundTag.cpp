// DefineButtonSoundTag.cpp: sounds for Button characters.
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

#include "DefineButtonSoundTag.h"
#include <vector>
#include "movie_definition.h"
#include "SWFStream.h"
#include "button_character_def.h"

namespace gnash {
namespace SWF {

DefineButtonSoundTag::DefineButtonSoundTag(SWFStream& in, movie_definition& m)
    :
    _sounds(4, ButtonSound())
{
    read(in, m);
}

void
DefineButtonSoundTag::loader(SWFStream& in, tag_type tag, movie_definition& m,
        const RunInfo& /*r*/)
{
    assert(tag == SWF::DEFINEBUTTONSOUND);

    in.ensureBytes(2);
    int id = in.read_u16();
    character_def* chdef = m.get_character_def(id);
    if (!chdef)
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("DEFINEBUTTONSOUND refers to an unknown "
                "character def %d"), id);
        );
        return;
    }

    button_character_definition* button = 
        dynamic_cast<button_character_definition*> (chdef);

    if (!button)
    {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("DEFINEBUTTONSOUND refers to character id "
                "%d, a %s (expected a button character)"),
                id, typeName(*chdef));
        );
        return;
    }

    if (button->hasSound())
    {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("Attempt to redefine button sound ignored"));
        );
        return;
    }

    std::auto_ptr<DefineButtonSoundTag> bs(
            new DefineButtonSoundTag(in, m));
    button->addSoundTag(bs);
}


void
DefineButtonSoundTag::read(SWFStream& in, movie_definition& m)
{

	for (Sounds::iterator i = _sounds.begin(), e = _sounds.end(); i != e; ++i)
	{
		ButtonSound& sound = *i;
		in.ensureBytes(2);
		sound.soundID = in.read_u16();
		if (sound.soundID)
		{
			sound.sample = m.get_sound_sample(sound.soundID);
			if (!sound.sample)
			{
				IF_VERBOSE_MALFORMED_SWF(
				log_swferror(_("sound tag not found, sound_id=%d, "
                        "button state #=%i"), sound.soundID);
				);
			}
			IF_VERBOSE_PARSE(
                log_parse("\tsound_id = %d", sound.soundID);
			);
			sound.soundInfo.read(in);
		}
	}
}


// TODO: surely this is duplicated elsewhere, isn't it?
void DefineButtonSoundTag::SoundInfo::read(SWFStream& in)
{
	in.ensureBytes(1);
	m_in_point = m_out_point = m_loop_count = 0;
    
    // highest 2 bits are reserved(unused).
    int flags = in.read_u8();
	m_stop_playback = flags & (1 << 5); 
	m_no_multiple   = flags & (1 << 4); 
	m_has_envelope  = flags & (1 << 3); 
	m_has_loops     = flags & (1 << 2);  
	m_has_out_point = flags & (1 << 1); 
	m_has_in_point  = flags & (1 << 0);  
    
	if (m_has_in_point)
	{
		in.ensureBytes(4);
		m_in_point = in.read_u32();
	}
	if (m_has_out_point)
	{
		in.ensureBytes(4);
		m_out_point = in.read_u32();
	}
	if (m_has_loops)
	{
		in.ensureBytes(2);
		m_loop_count = in.read_u16();
	}
	if (m_has_envelope)
	{
		in.ensureBytes(1);
		int nPoints = in.read_u8();
		m_envelopes.resize(nPoints);
		in.ensureBytes(8 * nPoints);
		for (int i=0; i < nPoints; i++)
		{
			m_envelopes[i].m_mark44 = in.read_u32();
			m_envelopes[i].m_level0 = in.read_u16();
			m_envelopes[i].m_level1 = in.read_u16();
		}
	}
	else
	{
		m_envelopes.resize(0);
	}


	IF_VERBOSE_PARSE(
	log_parse("	has_envelope = %d", m_has_envelope);
	log_parse("	has_loops = %d", m_has_loops);
	log_parse("	has_out_point = %d", m_has_out_point);
	log_parse("	has_in_point = %d", m_has_in_point);
	log_parse("	in_point = %d", m_in_point);
	log_parse("	out_point = %d", m_out_point);
	log_parse("	loop_count = %d", m_loop_count);
	log_parse("	envelope size = %d", m_envelopes.size());
	);
}

}
}
