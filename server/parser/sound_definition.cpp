// sound.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Code to handle SWF sound-related tags.


#include "sound_definition.h"
#include "stream.h"
#include "impl.h"
//#include "ControlTag.h" // for start_sound_tag inheritance
#include "movie_definition.h"
#include "sprite_instance.h"

namespace gnash {


sound_sample::~sound_sample()
{
	media::sound_handler* handler = get_sound_handler();
	if (handler)
	{
		handler->delete_sound(m_sound_handler_id);
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
	m->addControlTag(this);
}


void
start_stream_sound_tag::execute(sprite_instance* m) const
{
	// Make static ?
	media::sound_handler* handler = get_sound_handler();
	if (handler)
	{
		// This makes it possible to stop only the stream when framejumping.
		m->setStreamSoundId(m_handler_id);
		handler->play_sound(m_handler_id, 0, 0, m_start, NULL);
	}
}

void
start_stream_sound_tag::execute_state(sprite_instance* /* m */) const
{
}

} // namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
