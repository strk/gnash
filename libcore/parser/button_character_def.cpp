// button_character_def.cpp:  Mouse-sensitive SWF buttons, for Gnash.
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


// Based on the public domain work of Thatcher Ulrich <tu@tulrich.com> 2003

#include "smart_ptr.h" // GNASH_USE_GC
#include "button_character_def.h"
#include "button_character_instance.h" // for create_character_instance()

#include "SWFStream.h" // for read()
#include "movie_definition.h"
#include "action_buffer.h"
#include "filter_factory.h"
#include "GnashKey.h" // for gnash::key::codeMap

namespace gnash {

//
// button_action
//


button_action::button_action(SWFStream& in, int tag_type, unsigned long endPos, movie_definition& mdef)
	:
	m_actions(mdef)
{
	// Read condition flags.
	if (tag_type == SWF::DEFINEBUTTON) // 7
	{
		m_conditions = OVER_DOWN_TO_OVER_UP;
	}
	else
	{
		assert(tag_type == SWF::DEFINEBUTTON2); // 34

		if ( in.tell()+2 > endPos ) 
		{
			IF_VERBOSE_MALFORMED_SWF(
			log_swferror(_("Premature end of button action input: can't read conditions"));
			);
			return;
		}
		in.ensureBytes(2);
		m_conditions = in.read_u16();
	}

	IF_VERBOSE_PARSE (
	log_parse(_("   button actions for conditions 0x%x"), m_conditions); // @@ need more info about which actions
	);

	// Read actions.
	m_actions.read(in, endPos);
}

bool
button_action::triggeredBy(const event_id& ev) const
{
	switch ( ev.id() )
	{
		case event_id::ROLL_OVER: return m_conditions & IDLE_TO_OVER_UP;
		case event_id::ROLL_OUT: return m_conditions & OVER_UP_TO_IDLE;
		case event_id::PRESS: return m_conditions & OVER_UP_TO_OVER_DOWN;
		case event_id::RELEASE: return m_conditions & OVER_DOWN_TO_OVER_UP;
		case event_id::DRAG_OUT: return m_conditions & OVER_DOWN_TO_OUT_DOWN;
		case event_id::DRAG_OVER: return m_conditions & OUT_DOWN_TO_OVER_DOWN;
		case event_id::RELEASE_OUTSIDE: return m_conditions & OUT_DOWN_TO_IDLE;
		case event_id::KEY_PRESS:
		{
			int keycode = getKeyCode();
			if ( ! keycode ) return false; // not a keypress event
			return key::codeMap[ev.keyCode][key::SWF] == keycode;
		}
		default: return false;
	}
}

//
// button_record
//

bool
button_record::is_valid()
{
	return (m_character_def != NULL);
}

static std::string
computeButtonStatesString(int flags)
{
	std::string ret;
	if ( flags & (1<<3) ) ret += "hit";
	if ( flags & (1<<2) ) { if ( ! ret.empty() ) ret += ","; ret += "down"; }
	if ( flags & (1<<1) ) { if ( ! ret.empty() ) ret += ","; ret += "over"; }
	if ( flags & (1<<0) ) { if ( ! ret.empty() ) ret += ","; ret += "up"; }
	return ret;
}

bool
button_record::read(SWFStream& in, int tag_type,
		movie_definition& m, unsigned long endPos)
{
	// caller should check this
	if (in.tell()+1 > endPos)
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("   premature end of button record input stream, can't read flags"));
		);
		return false;
	}

	in.ensureBytes(1);
	int	flags = in.read_u8();
	if (flags == 0)
	{
		return false;
	}

	// Upper 4 bits are:
	//
	//   ButtonReserved = readBits (f, 2);
	bool buttonHasBlendMode = flags & (1<<5); 
	bool buttonHasFilterList = flags & (1<<4);
	m_hit_test = flags & (1<<3); // 8 ? true : false;
	m_down     = flags & (1<<2); // 4 ? true : false;
	m_over     = flags & (1<<1); // 2 ? true : false;
	m_up       = flags & (1<<0); // 1 ? true : false;

	if (in.tell()+2 > endPos)
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("   premature end of button record input stream, can't read character id"));
		);
		return false;
	}
	in.ensureBytes(2);
	m_character_id = in.read_u16();

	// Get character definition now (safer)
	m_character_def = m.get_character_def(m_character_id);

	// If no character with given ID is found in the movie
	// definition, we print an error, but keep parsing.
	if ( ! m_character_def )
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("   button record for states [%s] refer to "
			"character with id %d, which is not found "
			"in the chars dictionary"), computeButtonStatesString(flags), m_character_id);
		);
	}
	else
	{
		IF_VERBOSE_PARSE(
		log_parse(_("   button record for states [%s] contain "
			"character %d (%s)"), computeButtonStatesString(flags), m_character_id,
		        typeName(*m_character_def));
		);
	}

	if (in.tell()+2 > endPos)
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("   premature end of button record input stream, can't read button layer (depth?)"));
		);
		return false;
	}
	in.ensureBytes(2);
	m_button_layer = in.read_u16();

    // matrix::read() checks the length of the stream
	m_button_matrix.read(in);

	if (tag_type == SWF::DEFINEBUTTON2)
	{
		// cxform::read_rgba() checks the length of the stream.
		m_button_cxform.read_rgba(in);
	}

	if ( buttonHasFilterList )
	{
		filter_factory::read(in, true, &_filters);
		LOG_ONCE(
			log_unimpl("Button filters"); 
		);
	}

	if ( buttonHasBlendMode )
	{
		in.ensureBytes(1);
        _blendMode = in.read_u8();
		LOG_ONCE(
			log_unimpl("Button blend mode");
		);
	}

	return true;
}


//
// button_character_definition
//

button_character_definition::button_character_definition(movie_definition& m)
	:
	m_sound(NULL),
	_movieDef(m)

// Constructor.
{
}

button_character_definition::~button_character_definition()
{
	for (ButtonActVect::iterator i=m_button_actions.begin(),
			ie=m_button_actions.end();
			i != ie; ++i )
	{
		delete *i;
	}
}


void button_character_definition::sound_info::read(SWFStream& in)
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

void
button_character_definition::readDefineButton(SWFStream& in, movie_definition& m)
{

	// Character ID has been read already

	// Old button tag.

	unsigned long endTagPos = in.get_tag_end_position();

	// Read button character records.
	for (;;)
	{
		button_record r;
		if (r.read(in, SWF::DEFINEBUTTON, m, endTagPos) == false)
		{
			// Null record; marks the end of button records.
			break;
		}

		// SAFETY CHECK:
		// if the button_record is corrupted, discard it
		if ( r.is_valid() )
		{
			m_button_records.push_back(r);
		}
	}

	if ( in.tell() >= endTagPos )
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("Premature end of DEFINEBUTTON tag, won't read actions"));
		);
		return;
	}

	// Read actions.
	m_button_actions.push_back(new button_action(in, SWF::DEFINEBUTTON, endTagPos, m));

}

void
button_character_definition::readDefineButtonCxform(SWFStream& in, movie_definition& /*m*/)
{
    // A simple rgb cxform for SWF2 buttons, superseded by DefineButton2.
    for (ButtonRecVect::iterator i = m_button_records.begin(), e = m_button_records.end();
            i != e; ++i)
    {
        (*i).m_button_cxform.read_rgb(in);
        IF_VERBOSE_PARSE(
            log_parse("Read DefineButtonCxform: %s", (*i).m_button_cxform);
        );
    }
}

void
button_character_definition::readDefineButton2(SWFStream& in, movie_definition& m)
{
	// Character ID has been read already

	in.ensureBytes(1 + 2); // flags + actions offset

	// Read the menu flag
	// (this is a single bit, the other 7 bits are reserved)
	m_menu = in.read_u8() != 0;
	if ( m_menu ) LOG_ONCE(log_unimpl("DEFINEBUTTON2 'menu' flag"));

	// Read the action offset
	unsigned button_2_action_offset = in.read_u16();

	unsigned long tagEndPosition = in.get_tag_end_position();
	unsigned next_action_pos = in.tell() + button_2_action_offset - 2;

	if ( next_action_pos > tagEndPosition )
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("Next Button2 actionOffset (%u) points past the end of tag (%lu)"),
			button_2_action_offset, tagEndPosition);
		);
		return;
	}

	unsigned long endOfButtonRecords = tagEndPosition;
	if ( ! button_2_action_offset  ) endOfButtonRecords = tagEndPosition;

	// Read button records.
	// takes at least 1 byte for the end mark button record, so 
	// we don't attempt to parse at all unless we have at least 1 byte left
	while ( in.tell() < endOfButtonRecords )
	{
		button_record	r;
		if (r.read(in, SWF::DEFINEBUTTON2, m, endOfButtonRecords) == false)
		{
			// Null record; marks the end of button records.
			break;
		}

		// SAFETY CHECK:
		// if the button_record is corrupted, discard it
		if ( r.is_valid() )
		{
			m_button_records.push_back(r);
		}
	}

	if ( button_2_action_offset )
	{
		in.seek(next_action_pos);

		// Read Button2ActionConditions
		// Don't read past tag end
		while ( in.tell() < tagEndPosition ) 
		{
			in.ensureBytes(2);
			unsigned next_action_offset = in.read_u16();
			if ( next_action_offset )
			{
				next_action_pos = in.tell() + next_action_offset - 2;
				if ( next_action_pos > tagEndPosition )
				{
					IF_VERBOSE_MALFORMED_SWF(
					log_swferror(_("Next action offset (%u) in Button2ActionConditions points past the end of tag"),
						next_action_offset);
					);
					next_action_pos = tagEndPosition;
				}
			}

			unsigned long endActionPos = next_action_offset ? next_action_pos : tagEndPosition;

			m_button_actions.push_back(new button_action(in, SWF::DEFINEBUTTON2, endActionPos, m));

			if (next_action_offset == 0 )
			{
				// done.
				break;
			}

			// seek to next action.
			in.seek(next_action_pos);
		}
	}
}

void
button_character_definition::readDefineButtonSound(SWFStream& in, movie_definition& m)
{
	// Character ID has been read already

	if ( m_sound )
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("Attempt to redefine button sound ignored"));
		);
		return;
	}

	m_sound.reset( new button_sound_def() );

	IF_VERBOSE_PARSE(
	log_parse(_("button sound options: "));
	);

	for (int i = 0; i < 4; i++)
	{
		button_sound_info& bs = m_sound->m_button_sounds[i];
		in.ensureBytes(2);
		bs.m_sound_id = in.read_u16();
		if (bs.m_sound_id)
		{
			bs.m_sam = m.get_sound_sample(bs.m_sound_id);
			if ( ! bs.m_sam )
			{
				IF_VERBOSE_MALFORMED_SWF(
				log_swferror(_("sound tag not found, sound_id=%d, button state #=%i"), bs.m_sound_id, i);
				);
			}
			IF_VERBOSE_PARSE(
			log_parse("\tsound_id = %d", bs.m_sound_id);
			);
			bs.m_sound_style.read(in);
		}
	}
}


void
button_character_definition::read(SWFStream& in, int tag_type, movie_definition& m)
{
	// Character ID has been read already

	switch (tag_type)
	{
		case SWF::DEFINEBUTTON:
			readDefineButton(in, m);
			break;
		case SWF::DEFINEBUTTONSOUND:
			readDefineButtonSound(in, m);
			break;
		case SWF::DEFINEBUTTON2:
			readDefineButton2(in, m);
			break;
		default:
			abort();
	}
}


character*
button_character_definition::create_character_instance(
		character* parent, int id)
{
	character* ch = new button_character_instance(this, parent, id);
	return ch;
}

#ifdef GNASH_USE_GC
void
button_character_definition::button_sound_info::markReachableResources() const
{
	if ( m_sam ) m_sam->setReachable();
}
#endif // GNASH_USE_GC

int
button_character_definition::getSWFVersion() const
{
	return _movieDef.get_version();
}

bool
button_character_definition::hasKeyPressHandler() const
{
	for (size_t i = 0, e = m_button_actions.size(); i < e; ++i)
	{
		const button_action& ba = *(m_button_actions[i]);
		if ( ba.triggeredByKeyPress() ) return true;
	}
	return false;
}

} // namespace gnash

// Local Variables:
// mode: C++
// c-basic-offset: 8
// tab-width: 8
// indent-tabs-mode: t
// End:
