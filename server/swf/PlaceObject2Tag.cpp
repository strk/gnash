// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/* $Id: PlaceObject2Tag.cpp,v 1.1 2007/04/11 14:20:21 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "PlaceObject2Tag.h"
#include "character.h"
#include "sprite_instance.h"
#include "swf_event.h"
#include "log.h"
#include "stream.h"

namespace gnash {
namespace SWF {
namespace tag_loaders {

void
PlaceObject2Tag::readPlaceObject(stream* in)
{
	// Original place_object tag; very simple.
	m_character_id = in->read_u16();
	m_depth = in->read_u16()+character::staticDepthOffset;
	m_matrix.read(in);

	IF_VERBOSE_PARSE
	(
		log_parse("  char_id = %d", m_character_id);
		log_parse("  depth = %d (%d)", m_depth, m_depth-character::staticDepthOffset);
		m_matrix.print();
	);

	if (in->get_position() < in->get_tag_end_position())
	{
		m_color_transform.read_rgb(in);

		IF_VERBOSE_PARSE
		(
			log_parse("  cxform:");
			m_color_transform.print();
		);

	}
}

// read placeObject2 actions
void
PlaceObject2Tag::readPlaceActions(stream* in, int movie_version)
{

	uint16_t reserved = in->read_u16();
	assert(reserved == 0);	// must be 0

	// The logical 'or' of all the following handlers.
	// I don't think we care about this...
	uint32_t all_flags = (movie_version >= 6) ?
		in->read_u32() : in->read_u16();
	UNUSED(all_flags);

	IF_VERBOSE_PARSE (
		log_parse("  actions: flags = 0x%X", all_flags);
	);

	// Read swf_events.
	for (;;)
	{
		// Read event.
		in->align();

		uint32_t flags = (movie_version >= 6) ? in->read_u32() : in->read_u16();

		if (flags == 0)
		{
			// Done with events.
			break;
		}

		uint32_t event_length = in->read_u32();
		if ( in->get_tag_end_position()-in->get_position() <  event_length )
		{
			IF_VERBOSE_MALFORMED_SWF(
			log_swferror("swf_event::read(), "
				"even_length = %u, but only %lu bytes left "
				"to the end of current tag."
				" Breaking for safety.",
				event_length, in->get_tag_end_position()-in->get_position());
			);
			break;
		}

		uint8 ch = key::INVALID;

		if (flags & (1 << 17))	// has keypress event
		{
			ch = in->read_u8();
			event_length--;
		}

		// Read the actions for event(s)
		std::auto_ptr<action_buffer> action(new action_buffer);
		action->read(in);

		size_t readlen = action->size();
		if (readlen > event_length)
		{
			IF_VERBOSE_MALFORMED_SWF(
			log_swferror("swf_event::read(), "
				"event_length = %d, "
				"but read " SIZET_FMT
				". Breaking for safety.",
				event_length, readlen);
			);
			// or should we just continue here ?
			break;
		}
		else if ( readlen < event_length )
		{
			IF_VERBOSE_MALFORMED_SWF(
			log_swferror("swf_event::read(), "
				"event_length = %d, "
				"but read " SIZET_FMT 
				". Skipping excessive bytes.",
				event_length, readlen);
			);
			in->skip_bytes(event_length - readlen);
		}

		// 13 bits reserved, 19 bits used
		static const event_id s_code_bits[19] =
		{
			event_id::LOAD,
			event_id::ENTER_FRAME,
			event_id::UNLOAD,
			event_id::MOUSE_MOVE,
			event_id::MOUSE_DOWN,
			event_id::MOUSE_UP,
			event_id::KEY_DOWN,
			event_id::KEY_UP,

			event_id::DATA,
			event_id::INITIALIZE,
			event_id::PRESS,
			event_id::RELEASE,
			event_id::RELEASE_OUTSIDE,
			event_id::ROLL_OVER,
			event_id::ROLL_OUT,
			event_id::DRAG_OVER,

			event_id::DRAG_OUT,
			event_id(event_id::KEY_PRESS, key::CONTROL),
			event_id::CONSTRUCT
		};

		// Let's see if the event flag we received is for an event that we know of
		if ((pow(2.0, int( sizeof(s_code_bits) / sizeof(s_code_bits[0]) )) - 1) < flags)
		{
			IF_VERBOSE_MALFORMED_SWF(
			log_swferror("swf_event::read() -- unknown / unhandled event type received, flags = 0x%x", flags);
			);
		}

		for (int i = 0, mask = 1; i < int(sizeof(s_code_bits)/sizeof(s_code_bits[0])); i++, mask <<= 1)
		{
			if (flags & mask)
			{
				std::auto_ptr<swf_event> ev ( new swf_event(s_code_bits[i], action) );
				//log_action("---- actions for event %s", ev->event().get_function_name().c_str());

				// hack
				if (i == 17)	// has keypress event ?
				{
					ev->event().setKeyCode(ch);
				}

				m_event_handlers.push_back(ev.release());
			}
		}
	}
}

// read SWF::PLACEOBJECT2
void
PlaceObject2Tag::readPlaceObject2(stream* in, int movie_version)
{
	in->align();

	bool	has_actions = in->read_uint(1) ? true : false;
	bool	has_clip_bracket = in->read_uint(1) ? true : false;
	bool	has_name = in->read_uint(1) ? true : false;
	bool	has_ratio = in->read_uint(1) ? true : false;
	bool	has_cxform = in->read_uint(1) ? true : false;
	bool	has_matrix = in->read_uint(1) ? true : false;
	bool	has_char = in->read_uint(1) ? true : false;
	bool	flag_move = in->read_uint(1) ? true : false;

	m_depth = in->read_u16()+character::staticDepthOffset;

	if (has_char) m_character_id = in->read_u16();

	if (has_matrix)
	{
		m_has_matrix = true;
		m_matrix.read(in);
	}

	if (has_cxform)
	{
		m_has_cxform = true;
		m_color_transform.read_rgba(in);
	}
			
	if (has_ratio) m_ratio = (float)in->read_u16() / (float)65535;

	if (has_name) m_name = in->read_string();

	if (has_clip_bracket) m_clip_depth = in->read_u16()+character::staticDepthOffset; 

	if (has_actions)
	{
		readPlaceActions(in, movie_version);
	}


	if (has_char == true && flag_move == true)
	{
		// Remove whatever's at m_depth, and put m_character there.
		m_place_type = REPLACE;
	}
	else if (has_char == false && flag_move == true)
	{
		// Moves the object at m_depth to the new location.
		m_place_type = MOVE;
	}
	else if (has_char == true && flag_move == false)
	{
		// Put m_character at m_depth.
		m_place_type = PLACE;
	}

	IF_VERBOSE_PARSE (
		log_parse("  PLACEOBJECT2: depth = %d (%d)", m_depth, m_depth-character::staticDepthOffset);
		if ( has_char ) log_parse("  char id = %d", m_character_id);
		if ( has_matrix ) 
		{
			log_parse("  mat:");
			m_matrix.print();
		}
		if ( has_cxform )
		{
			log_parse("  cxform:");
			m_color_transform.print();
		}
		if ( has_ratio ) log_parse("  ratio: %f", m_ratio);
		if ( has_name ) log_parse("  name = %s", m_name ? m_name : "<null>");
		if ( has_clip_bracket ) log_parse("  clip_depth = %d (%d)", m_clip_depth, m_clip_depth-character::staticDepthOffset);
		log_parse(" m_place_type: %d", m_place_type);
	);
			

			
	//log_msg("place object at depth %i", m_depth);
}

void
PlaceObject2Tag::read(stream* in, tag_type tag, int movie_version)
{

	m_tag_type = tag;

	if (tag == SWF::PLACEOBJECT)
	{
		readPlaceObject(in);
	}
	else
	{
		readPlaceObject2(in, movie_version);
	}
}

/// Place/move/whatever our object in the given movie.
void
PlaceObject2Tag::execute(sprite_instance* m)
{
    switch (m_place_type) {

      case PLACE:
          m->add_display_object(
	      m_character_id,
	      m_name,
	      m_event_handlers,
	      m_depth,
	      m->is_reverse_execution(),	// place_object doesn't do replacement when not in reverse execution
	      m_color_transform,
	      m_matrix,
	      m_ratio,
	      m_clip_depth);
	  break;
	  
      case MOVE:
	  m->move_display_object(
	      m_depth,
	      m_has_cxform,
	      m_color_transform,
	      m_has_matrix,
	      m_matrix,
	      m_ratio,
	      m_clip_depth);
	  break;
	  
      case REPLACE:
	  m->replace_display_object(
	      m_character_id,
	      m_name,
	      m_depth,
	      m_has_cxform,
	      m_color_transform,
	      m_has_matrix,
	      m_matrix,
	      m_ratio,
	      m_clip_depth);
	  break;
    }
}

PlaceObject2Tag::~PlaceObject2Tag()
{
	delete [] m_name;
	m_name = NULL;
	for(size_t i=0; i<m_event_handlers.size(); ++i)
	{
		delete m_event_handlers[i];
	}
}

void
PlaceObject2Tag::execute_state_reverse(sprite_instance* m, int frame)
{
    switch (m_place_type) {
      case PLACE:
	  // reverse of add is remove
	  m->remove_display_object(m_depth, m_tag_type == 4 ? m_character_id : -1);
	  break;
	  
      case MOVE:
	  // reverse of move is move
	  m->move_display_object(
	      m_depth,
	      m_has_cxform,
	      m_color_transform,
	      m_has_matrix,
	      m_matrix,
	      m_ratio,
	      m_clip_depth);
	  break;
	  
      case REPLACE:
      {
	  // reverse of replace is to re-add the previous object.
	  execute_tag*	last_add = m->find_previous_replace_or_add_tag(frame, m_depth, -1);
	  if (last_add) {
	      last_add->execute_state(m);
	  } else {
	      log_error("reverse REPLACE can't find previous replace or add tag(%d, %d)",
			frame, m_depth);
	      
	  }
	  break;
      }
    }
}

uint32
PlaceObject2Tag::get_depth_id_of_replace_or_add_tag() const
{
	uint32 depthid = 0;
	if (m_place_type == PLACE || m_place_type == REPLACE)
	{
		int id = -1;
		if (m_tag_type == SWF::PLACEOBJECT)
		{
		    // Old-style PlaceObject; the corresponding Remove
		    // is specific to the character_id.
		    id = m_character_id;
		}
		depthid = ((m_depth & 0x0FFFF) << 16) | (id & 0x0FFFF);
	}
	return depthid;
}

} // namespace gnash::SWF::tag_loaders
} // namespace gnash::SWF
} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
