// sound.h   -- Thatcher Ulrich, Vitaly Alexeev

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

#ifndef GNASH_SOUND_H
#define GNASH_SOUND_H


namespace gnash {
	class movie_definition;
}

#include "impl.h"
#include "execute_tag.h" // for sound tags

namespace gnash {

	class sound_sample_impl : public sound_sample
	{
	public:
		int	m_sound_handler_id;

		sound_sample_impl(int id)
			:
			m_sound_handler_id(id)
		{
		}

		virtual ~sound_sample_impl();
	};

	/// SWF Tag StartSound (15) 
	class start_sound_tag : public execute_tag
	{
	public:
		uint16_t	m_handler_id;
		int	m_loop_count;
		bool	m_stop_playback;
		std::vector<sound_handler::sound_envelope> m_envelopes;

		// envelopes for the current sound instance
		uint32_t* envelopes;

		start_sound_tag()
			:
			m_handler_id(0),
			m_loop_count(0),
			m_stop_playback(false)
		{
		}


		/// \brief
		/// Initialize this StartSound tag from
		/// the stream  & given sample.
		//
		/// Insert ourself into the movie.
		void read(stream* in, int tag_type,
			movie_definition* m, const sound_sample_impl* sam);

		/// StartSound is a "state" tag.
		void	execute_state(sprite_instance* m);

		/// This implementation of 'execute' should likely
		/// be the default one.
		void execute(sprite_instance* m)
		{
			execute_state(m);
		}

		/// TODO: provide execute_reverse ?
		/// (for StartSound would StopSound and vice-versa)
	};

	/// SWF Tag SoundStreamBlock (19) 
	class start_stream_sound_tag : public execute_tag
	{
	public:
		uint16_t	m_handler_id;
		long		m_start;
		int		latency;

		start_stream_sound_tag()
			:
			m_handler_id(0),
			m_start(0),
			latency(0)
		{
		}


		/// \brief
		/// Initialize this StartSound tag
		/// from the stream & given sample.
		//
		/// Insert ourself into the movie.
		void	read(movie_definition* m, int handler_id, long start);


		void	execute(sprite_instance* m);
	};

}


#endif // GNASH_SOUND_H
