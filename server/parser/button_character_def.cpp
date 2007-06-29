// button_character_def.cpp:  Mouse-sensitive SWF buttons, for Gnash.
//
//   Copyright (C) 2006, 2007 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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

/* $Id: button_character_def.cpp,v 1.15 2007/06/29 20:37:51 strk Exp $ */

// Based on the public domain work of Thatcher Ulrich <tu@tulrich.com> 2003

#include "button_character_def.h"
#include "button_character_instance.h" // for create_character_instance()

#include "stream.h" // for read()
#include "movie_definition.h"
#include "action_buffer.h"


namespace gnash {

//
// button_action
//


button_action::~button_action()
{
	for (ActionList::iterator i=m_actions.begin(), e=m_actions.end();
			i != e; ++i)
	{
		// We can NOT delete action_buffers here becase they 
		// may contain the action currently being executed and
		// triggering the deletion.
		// I'm not really sure about whether this is the problem,
		// anyway clip_as_button2.swf fails on segfault when clicking
		// the upper-right button if we delete here.
		//
		// TODO: properly implement management of these resources
		//       which are otherwise just leaking..
		//
		//delete (*i);
	}
	m_actions.clear(); // this is useless, will be done automatically
}

void	button_action::read(stream* in, int tag_type)
{
	// Read condition flags.
	if (tag_type == SWF::DEFINEBUTTON) // 7
	{
		m_conditions = OVER_DOWN_TO_OVER_UP;
	}
	else
	{
		assert(tag_type == SWF::DEFINEBUTTON2); // 34
		m_conditions = in->read_u16();
	}

	IF_VERBOSE_PARSE (
	log_parse(_("-- actions in button\n")); // @@ need more info about which actions
	);

	// Read actions.
	action_buffer*	a = new action_buffer;
	a->read(in);
	m_actions.push_back(a);
}

//
// button_record
//

bool
button_record::is_valid()
{
	return (m_character_def != NULL);
}

bool
button_record::read(stream* in, int tag_type,
		movie_definition* m)
{
	int	flags = in->read_u8();
	if (flags == 0)
	{
		return false;
	}
	m_hit_test = flags & 8 ? true : false;
	m_down = flags & 4 ? true : false;
	m_over = flags & 2 ? true : false;
	m_up = flags & 1 ? true : false;

	m_character_id = in->read_u16();

	// Get character definition now (safer)
	m_character_def = m->get_character_def(m_character_id);
	// If no character with given ID is found in the movie
	// definition, we print an error, but keep parsing.
	if ( ! m_character_def )
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("button record refer to "
			"character with id %d, which is not found "
			"in the chars dictionary"), m_character_id);
		);
	}

	m_button_layer = in->read_u16();
	m_button_matrix.read(in);

	if (tag_type == 34)
	{
		m_button_cxform.read_rgba(in);
	}

	return true;
}


//
// button_character_definition
//

button_character_definition::button_character_definition()
	:
	m_min_layer(0),
	m_max_layer(0),
	m_sound(NULL)

// Constructor.
{
}

button_character_definition::~button_character_definition()
{
	delete m_sound;
}


void button_character_definition::sound_info::read(stream* in)
{
	m_in_point = m_out_point = m_loop_count = 0;
	in->read_uint(2);	// skip reserved bits.
	m_stop_playback = in->read_uint(1) ? true : false;
	m_no_multiple = in->read_uint(1) ? true : false;
	m_has_envelope = in->read_uint(1) ? true : false;
	m_has_loops = in->read_uint(1) ? true : false;
	m_has_out_point = in->read_uint(1) ? true : false;
	m_has_in_point = in->read_uint(1) ? true : false;
	if (m_has_in_point) m_in_point = in->read_u32();
	if (m_has_out_point) m_out_point = in->read_u32();
	if (m_has_loops) m_loop_count = in->read_u16();

	if (m_has_envelope)
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


	IF_VERBOSE_PARSE(
	log_parse("	has_envelope = %d", m_has_envelope);
	log_parse("	has_loops = %d", m_has_loops);
	log_parse("	has_out_point = %d", m_has_out_point);
	log_parse("	has_in_point = %d", m_has_in_point);
	log_parse("	in_point = %d", m_in_point);
	log_parse("	out_point = %d", m_out_point);
	log_parse("	loop_count = %d", m_loop_count);
	log_parse("	envelope size = " SIZET_FMT , m_envelopes.size());
	);
}



void
button_character_definition::read(stream* in, int tag_type, movie_definition* m)
// Initialize from the given stream.
{
	// Character ID has been read already

	assert(
		tag_type == SWF::DEFINEBUTTON		// 7
		|| tag_type == SWF::DEFINEBUTTONSOUND	// 17
		|| tag_type == SWF::DEFINEBUTTON2	// 34
	 );

	if (tag_type == SWF::DEFINEBUTTON)
	{
		// Old button tag.

		// Read button character records.
		for (;;)
		{
			button_record	r;
			if (r.read(in, tag_type, m) == false)
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

		// Read actions.
		button_action actions;
		actions.read(in, tag_type);
		m_button_actions.push_back(actions);
	}
	else if (tag_type == SWF::DEFINEBUTTONSOUND)
	{
		assert(m_sound == NULL);	// redefinition button sound is error
		m_sound = new button_sound_def();
		IF_VERBOSE_PARSE(
		log_parse(_("button sound options: "));
		);
		for (int i = 0; i < 4; i++)
		{
			button_sound_info& bs = m_sound->m_button_sounds[i];
			bs.m_sound_id = in->read_u16();
			if (bs.m_sound_id > 0)
			{
				bs.m_sam = m->get_sound_sample(bs.m_sound_id);
				if (bs.m_sam == NULL)
				{
//						printf("sound tag not found, sound_id=%d, button state #=%i", bs.sound_id, i);
				}
				IF_VERBOSE_PARSE(
				log_parse("\n	sound_id = %d", bs.m_sound_id);
				);
				bs.m_sound_style.read(in);
			}
		}
	}
	else if (tag_type == SWF::DEFINEBUTTON2)
	{
		// Read the menu flag
		// (this is a single bit, the other 7 bits are reserved)
		m_menu = in->read_u8() != 0;

		unsigned button_2_action_offset = in->read_u16();
		unsigned next_action_pos = in->get_position() + button_2_action_offset - 2;

		// Read button records.
		for (;;)
		{
			button_record	r;
			if (r.read(in, tag_type, m) == false)
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

		if ( next_action_pos >= in->get_tag_end_position() )
		{
			IF_VERBOSE_MALFORMED_SWF(
			log_swferror(_("Next Button2 actionOffset (%u) points past the end of tag"), button_2_action_offset);
			);
			return;
		}

		in->set_position(next_action_pos);

		// Read Button2ActionConditions
		for (;;)
		{
			unsigned next_action_offset = in->read_u16();
			next_action_pos = in->get_position() + next_action_offset - 2;

			m_button_actions.resize(m_button_actions.size() + 1);
			m_button_actions.back().read(in, tag_type);

			if (next_action_offset == 0 )
			{
				// done.
				break;
			}

			//was: in->get_position() >= in->get_tag_end_position()
			if ( next_action_pos >= in->get_tag_end_position() )
			{
				IF_VERBOSE_MALFORMED_SWF(
				log_swferror(_("Next action offset (%u) in Button2ActionConditions points past the end of tag"),
					next_action_offset);
				);
				break;
			}

			// seek to next action.
			in->set_position(next_action_pos);
		}
	}


	// detect min/max layer number
	m_min_layer=0;
	m_max_layer=0;
	for (unsigned int i=0; i<m_button_records.size(); i++)
	{
	  int this_layer = m_button_records[i].m_button_layer;

	  if ((i==0) || (this_layer < m_min_layer))  m_min_layer=this_layer;
	  if ((i==0) || (this_layer > m_max_layer))  m_max_layer=this_layer;
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

} // namespace gnash

// Local Variables:
// mode: C++
// c-basic-offset: 8
// tab-width: 8
// indent-tabs-mode: t
// End:
