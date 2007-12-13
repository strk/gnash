// PlaceObject2Tag.cpp:  for Gnash.
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

/* $Id: PlaceObject2Tag.cpp,v 1.28 2007/12/13 10:58:10 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "PlaceObject2Tag.h"
#include "character.h"
#include "sprite_instance.h"
#include "swf_event.h"
#include "log.h"
#include "stream.h"
#include "filter_factory.h"

namespace gnash {
namespace SWF {

void
PlaceObject2Tag::readPlaceObject(stream* in)
{
	// Original place_object tag; very simple.
	m_character_id = in->read_u16();
	m_depth = in->read_u16()+character::staticDepthOffset;
	m_matrix.read(in);

	IF_VERBOSE_PARSE
	(
		log_parse(_("  char_id = %d"), m_character_id);
		log_parse(_("  depth = %d (%d)"), m_depth, m_depth-character::staticDepthOffset);
		m_matrix.print();
	);

	if (in->get_position() < in->get_tag_end_position())
	{
		m_color_transform.read_rgb(in);

		IF_VERBOSE_PARSE
		(
			log_parse(_("  cxform:"));
			m_color_transform.print();
		);

	}
}

// read placeObject2 actions
void
PlaceObject2Tag::readPlaceActions(stream* in, int movie_version)
{

	boost::uint16_t reserved = in->read_u16();
	IF_VERBOSE_MALFORMED_SWF (
		if ( reserved != 0 ) // must be 0
		{
			log_swferror(_("Reserved field in PlaceObject actions == %u (expected 0)"), reserved);
		}
	);
	
	// The logical 'or' of all the following handlers.
	all_event_flags = (movie_version >= 6) ? in->read_u32() : in->read_u16();

	IF_VERBOSE_PARSE (
		log_parse(_("  actions: flags = 0x%X"), all_event_flags);
	);

	// Read swf_events.
	for (;;)
	{
		// Read event.
		in->align();

		boost::uint32_t flags = (movie_version >= 6) ? in->read_u32() : in->read_u16();

		if (flags == 0) // no other events
		{
			break;
		}

		boost::uint32_t event_length = in->read_u32();
		if ( in->get_tag_end_position()-in->get_position() <  event_length )
		{
			IF_VERBOSE_MALFORMED_SWF(
			log_swferror(_("swf_event::read(), "
				"even_length = %u, but only %lu bytes left "
				"to the end of current tag."
				" Breaking for safety."),
				event_length, in->get_tag_end_position()-in->get_position());
			);
			break;
		}

		boost::uint8_t ch = key::INVALID;

		if (flags & (1 << 17))	// has KeyPress event
		{
			ch = in->read_u8();
			event_length--;
		}

		// Read the actions for event(s)
		action_buffer* action = new action_buffer();
		_actionBuffers.push_back(action); // take ownership
		action->read(*in, in->get_position()+event_length);

		assert(action->size() == event_length); 

		// 13 bits reserved, 19 bits used
		const int total_known_events = 19;
		static const event_id s_code_bits[total_known_events] =
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

		// Integrity check: all reserved bits should be zero
		if( flags >> total_known_events ) 
		{
			IF_VERBOSE_MALFORMED_SWF(
			log_swferror(_("swf_event::read() -- unknown / unhandled event type received, flags = 0x%x"), flags);
			);
		}

		// Aah! same action for multiple events !
		for (int i = 0, mask = 1; i < total_known_events; i++, mask <<= 1)
		{
			if (flags & mask)
			{
				std::auto_ptr<swf_event> ev ( new swf_event(s_code_bits[i], *action) );
				IF_VERBOSE_PARSE (
				log_parse("---- actions for event %s", ev->event().get_function_name().c_str());
				);

				if (i == 17)	// has KeyPress event
				{
					ev->event().setKeyCode(ch);
				}

				m_event_handlers.push_back(ev.release());
			}
		}
	} //end of for(;;)
}

// read SWF::PLACEOBJECT2
void
PlaceObject2Tag::readPlaceObject2(stream* in, int movie_version)
{
    in->align();

    // PlaceObject2 specific flags
    boost::uint8_t has_flags2 = in->read_u8();

    bool    has_actions    = has_flags2 & (1 << 7); 
    bool    has_clip_depth = has_flags2 & (1 << 6); 
    bool    has_name       = has_flags2 & (1 << 5); 
    bool    has_ratio      = has_flags2 & (1 << 4); 
    bool    has_cxform     = has_flags2 & (1 << 3); 
    bool    has_matrix     = has_flags2 & (1 << 2);
    bool    has_char       = has_flags2 & (1 << 1);
    bool    flag_move      = has_flags2 & (1 << 0); 

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

    if (has_ratio) 
        m_ratio = in->read_u16();
    else
        m_ratio = character::noRatioValue;

    if (has_name) m_name = in->read_string();

    if (has_clip_depth)
        m_clip_depth = in->read_u16()+character::staticDepthOffset;
    else
        m_clip_depth = character::noClipDepthValue;

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
    else if (has_char == false && flag_move == false)
    {
        m_place_type = REMOVE;
    }

    IF_VERBOSE_PARSE (
        log_parse(_("  PLACEOBJECT2: depth = %d (%d)"), m_depth, m_depth-character::staticDepthOffset);
        if ( has_char ) log_parse(_("  char id = %d"), m_character_id);
        if ( has_matrix )
        {
            log_parse(_("  mat:"));
            m_matrix.print();
        }
        if ( has_cxform )
        {
            log_parse(_("  cxform:"));
            m_color_transform.print();
        }
        if ( has_ratio ) log_parse(_("  ratio: %d"), m_ratio);
        if ( has_name ) log_parse(_("  name = %s"), m_name ? m_name : "<null>");
        if ( has_clip_depth ) log_parse(_("  clip_depth = %d (%d)"), m_clip_depth, m_clip_depth-character::staticDepthOffset);
        log_parse(_(" m_place_type: %d"), m_place_type);
    );

    //log_msg("place object at depth %i", m_depth);
}

// read SWF::PLACEOBJECT3
void
PlaceObject2Tag::readPlaceObject3(stream* in, int movie_version)
{
    in->align();

    // PlaceObject2 specific flags
    boost::uint8_t has_flags2 = in->read_u8();

    bool    has_actions    = has_flags2 & (1 << 7); 
    bool    has_clip_depth = has_flags2 & (1 << 6); 
    bool    has_name       = has_flags2 & (1 << 5); 
    bool    has_ratio      = has_flags2 & (1 << 4); 
    bool    has_cxform     = has_flags2 & (1 << 3); 
    bool    has_matrix     = has_flags2 & (1 << 2);
    bool    has_char       = has_flags2 & (1 << 1);
    bool    flag_move      = has_flags2 & (1 << 0); 

    // PlaceObject3 specific flags, first 3 bits are unused
    boost::uint8_t has_flags3 = in->read_u8();

    bool    hasImage           = has_flags3 & (1 << 4); 
    bool    hasClassName       = has_flags3 & (1 << 3); 
    bool    has_bitmap_caching = has_flags3 & (1 << 2); 
    bool    has_blend_mode     = has_flags3 & (1 << 1); 
    bool    has_filters        = has_flags3 & (1 << 0);

    boost::uint8_t blend_mode = 0;
    boost::uint8_t bitmask = 0;
    std::string className;

    m_depth = in->read_u16()+character::staticDepthOffset;

    if (has_char)
    {
        m_character_id = in->read_u16();
    }

    if (hasClassName || (hasImage && has_char) )
    {
        log_unimpl("PLACEOBJECT3 with associated class name");
        in->read_string(className);
    }

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

    if (has_ratio) 
        m_ratio = in->read_u16();
    else
        m_ratio = character::noRatioValue;

    if (has_name) m_name = in->read_string();

    if (has_clip_depth)
        m_clip_depth = in->read_u16()+character::staticDepthOffset;
    else
        m_clip_depth = character::noClipDepthValue;

    if (has_filters)
    {
        Filters v; // TODO: Attach the filters to the display object.
        filter_factory::read(in, movie_version, true, &v);
    }

    if (has_blend_mode)
    {
        blend_mode = in->read_u8();
    }

    if (has_bitmap_caching)
    {
        // It is not certain that this actually exists, so if this reader
        // is broken, it is probably here!
        bitmask = in->read_u8();
    }

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
    else if (has_char == false && flag_move == false)
    {
         m_place_type = REMOVE;
    }

    IF_VERBOSE_PARSE (
        log_parse(_("  PLACEOBJECT3: depth = %d (%d)"), m_depth, m_depth-character::staticDepthOffset);
        if ( has_char ) log_parse(_("  char id = %d"), m_character_id);
        if ( has_matrix )
        {
            log_parse(_("  mat:"));
            m_matrix.print();
        }
        if ( has_cxform )
        {
            log_parse(_("  cxform:"));
            m_color_transform.print();
        }
        if ( has_ratio ) log_parse(_("  ratio: %d"), m_ratio);
        if ( has_name ) log_parse(_("  name = %s"), m_name ? m_name : "<null>");
        if ( hasClassName ) log_parse(_("  class name = %s"), className.c_str());
        if ( has_clip_depth ) log_parse(_("  clip_depth = %d (%d)"), m_clip_depth, m_clip_depth-character::staticDepthOffset);
        log_parse(_(" m_place_type: %d"), m_place_type);
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
	else if ( tag == SWF::PLACEOBJECT2 )
	{
		readPlaceObject2(in, movie_version);
	}
	else
	{
		readPlaceObject3(in, movie_version);
	}
}

/// Place/move/whatever our object in the given movie.
void
PlaceObject2Tag::execute(sprite_instance* m) const
{
    switch (m_place_type) {

      case PLACE:
          m->add_display_object(
	      m_character_id,
	      m_name,
	      m_event_handlers,
	      m_depth,
	      m_color_transform,
	      m_matrix,
	      m_ratio,
	      m_clip_depth);
	  break;

      case MOVE:
	  m->move_display_object(
	      m_depth,
	      m_has_cxform ? &m_color_transform : NULL,
	      m_has_matrix ? &m_matrix : NULL,
	      m_ratio,
	      m_clip_depth);
	  break;

      case REPLACE:
	  m->replace_display_object(
	      m_character_id,
	      m_name,
	      m_depth,
	      m_has_cxform ? &m_color_transform : NULL,
	      m_has_matrix ? &m_matrix : NULL,
	      m_ratio,
	      m_clip_depth);
	  break;

      case REMOVE:
          m->remove_display_object(m_depth, 0); // 0 since it is unused.
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

	for(size_t i=0; i<_actionBuffers.size(); ++i)
	{
		delete _actionBuffers[i];
	}
}

/* public static */
void
PlaceObject2Tag::loader(stream* in, tag_type tag, movie_definition* m)
{
    assert(tag == SWF::PLACEOBJECT || tag == SWF::PLACEOBJECT2 || tag == SWF::PLACEOBJECT3);

    IF_VERBOSE_PARSE
    (
	log_parse(_("  place_object_2"));
    );

    // TODO: who owns and is going to remove this tag ?
    PlaceObject2Tag* ch = new PlaceObject2Tag(*m);
    ch->read(in, tag, m->get_version());

    m->addControlTag(ch);

    int depth = ch->getDepth();
    if ( depth < 0 && depth >= character::staticDepthOffset )
    {
    	m->addTimelineDepth(depth);
    }
    else
    {
	log_debug("PlaceObject2Tag depth %d is out of static depth zone. Won't register its TimelineDepth.", depth);
    }
}

} // namespace gnash::SWF
} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
