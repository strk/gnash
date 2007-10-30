// sound.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Code to handle SWF sound-related tags.


#include "sound_definition.h"
#include "stream.h"
#include "impl.h"
#include "execute_tag.h" // for start_sound_tag inheritance
#include "movie_definition.h"
#include "sprite_instance.h"

namespace gnash {


sound_sample::~sound_sample()
{
	sound_handler* handler = get_sound_handler();
	if (handler)
	{
		handler->delete_sound(m_sound_handler_id);
	}
}



//
// SWF Tag StartSound (15) 
//
void
start_sound_tag::read(stream* in, int /* tag_type */, movie_definition* m,
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
	m->add_execute_tag(this);
}


void
start_sound_tag::execute(sprite_instance* /* m */) const
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

//
// SWF Tag SoundStreamBlock (19) 
//

// Initialize this StartSound tag from the stream & given sample.
// Insert ourself into the movie.
void
start_stream_sound_tag::read(movie_definition* m, int handler_id, long start)
{
	m_handler_id = handler_id;
	m_start = start;
	m->add_execute_tag(this);
}


void
start_stream_sound_tag::execute(sprite_instance* m) const
{
	// Make static ?
	sound_handler* handler = get_sound_handler();
	if (handler)
	{
		// This makes it possible to stop only the stream when framejumping.
		m->set_sound_stream_id(m_handler_id);
		handler->play_sound(m_handler_id, 0, 0, m_start, NULL);
	}
}

} // namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
