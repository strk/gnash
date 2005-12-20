// sound.h   -- Thatcher Ulrich, Vitaly Alexeev

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

#ifndef GNASH_SOUND_H
#define GNASH_SOUND_H


#include "impl.h"


namespace gnash {
	struct sound_sample_impl : public sound_sample
	{
		int	m_sound_handler_id;

		sound_sample_impl(int id)
			:
			m_sound_handler_id(id)
		{
		}

		virtual ~sound_sample_impl();
	};
}


#endif // GNASH_SOUND_H
