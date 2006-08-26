// -- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// SWF buttons.  Mouse-sensitive update/display, actions, etc.


#include "button_character_def.h"
#include "button_character_instance.h" // for create_character_instance()

//#include "action.h"
//#include "render.h"
//#include "sound.h"
#include "stream.h" // for read()
#include "movie_definition.h"
//#include "sprite_instance.h"
//#include "movie_root.h"
#include "action_buffer.h"



namespace gnash {

//
// button_action
//


button_action::~button_action()
{
	m_actions.clear();
}

void	button_action::read(stream* in, int tag_type)
{
	// Read condition flags.
	if (tag_type == 7)
	{
		m_conditions = OVER_DOWN_TO_OVER_UP;
	}
	else
	{
		assert(tag_type == 34);
		m_conditions = in->read_u16();
	}

	IF_VERBOSE_PARSE (
	log_parse("-- actions in button\n"); // @@ need more info about which actions
	);

	// Read actions.
	action_buffer*	a = new action_buffer;
	a->read(in);
	m_actions.push_back(a);
}

//
// button_record
//

// Return true if we read a record; false if this is a null record.
bool
button_record::read(stream* in, int tag_type,
		movie_definition* /*m*/)
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
	m_character_def = NULL;
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
	log_parse("	envelope size = %u", m_envelopes.size());
	);
}



void	button_character_definition::read(stream* in, int tag_type, movie_definition* m)
// Initialize from the given stream.
{
	assert(tag_type == 7 || tag_type == 17 || tag_type == 34);

	if (tag_type == 7)
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
			m_button_records.push_back(r);
		}

		// Read actions.
		m_button_actions.resize(m_button_actions.size() + 1);
		m_button_actions.back().read(in, tag_type);
	}
	else if (tag_type == 17)
	{
		assert(m_sound == NULL);	// redefinition button sound is error
		m_sound = new button_sound_def();
		IF_VERBOSE_PARSE(
		log_parse("button sound options: ");
		);
		for (int i = 0; i < 4; i++)
		{
			button_sound_info& bs = m_sound->m_button_sounds[i];
			bs.m_sound_id = in->read_u16();
			if (bs.m_sound_id > 0)
			{
				bs.m_sam = (sound_sample_impl*) m->get_sound_sample(bs.m_sound_id);
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
	else if (tag_type == 34)
	{
		// Read the menu flag.
		m_menu = in->read_u8() != 0;

		int	button_2_action_offset = in->read_u16();
		int	next_action_pos = in->get_position() + button_2_action_offset - 2;

		// Read button records.
		for (;;)
		{
			button_record	r;
			if (r.read(in, tag_type, m) == false)
			{
				// Null record; marks the end of button records.
				break;
			}
			m_button_records.push_back(r);
		}

		if (button_2_action_offset > 0)
		{
			in->set_position(next_action_pos);

			// Read Button2ActionConditions
			for (;;)
			{
				int	next_action_offset = in->read_u16();
				next_action_pos = in->get_position() + next_action_offset - 2;

				m_button_actions.resize(m_button_actions.size() + 1);
				m_button_actions.back().read(in, tag_type);

				if (next_action_offset == 0
				    || in->get_position() >= in->get_tag_end_position())
				{
					// done.
					break;
				}

				// seek to next action.
				in->set_position(next_action_pos);
			}
		}
	}
}


character*
button_character_definition::create_character_instance(
		character* parent, int id)
{
	character* ch = new button_character_instance(this, parent, id);
	return ch;
}

}


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
