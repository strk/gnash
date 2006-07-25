// sound_handler_sdl.cpp	-- Thatcher Ulrich http://tulrich.com 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A sound_handler that uses SDL_mixer for output

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_SDL_MIXER_H
#include "gnash.h"
#include "container.h"
#include "SDL_mixer.h"
#include "log.h"
#include "types.h"	// for IF_VERBOSE_* macros

using namespace gnash;

// Use SDL_mixer to handle sounds.
struct SDL_sound_handler : gnash::sound_handler
{
	bool	m_opened;
	bool	m_stereo;
	int	m_sample_rate;
	uint16_t m_format;
	std::vector<Mix_Chunk*>	m_samples;

	#define	SAMPLE_RATE 44100
	#define MIX_CHANNELS 8
	#define CHANNELS 2		//stereo - 2, mono - 1
	#define BUFSIZE 4096		// for 44100 bufsize 1024 is small

	SDL_sound_handler()
		:
		m_opened(false),
		m_stereo(false),
		m_sample_rate(0),
		m_format(0)
	{
		// !!! some drivers on Linux always open audio with channels=2
		if (Mix_OpenAudio(SAMPLE_RATE, AUDIO_S16SYS, CHANNELS, BUFSIZE) != 0)
		{
			gnash::log_error("can't open SDL_mixer: %s\n", Mix_GetError());
		}
		else
		{
			m_opened = true;     
			Mix_AllocateChannels(MIX_CHANNELS);
			Mix_Volume(-1, MIX_MAX_VOLUME);

			// get and print the audio format in use
			int channels;
			int num_times_opened = Mix_QuerySpec(&m_sample_rate, &m_format, &channels);
			UNUSED(num_times_opened);
			m_stereo = channels == 2 ? true : false;
			
			const char *format_str = "Unknown";
			switch (m_format)
			{
			case AUDIO_U8: format_str = "U8"; break;
			case AUDIO_S8: format_str = "S8"; break;
			case AUDIO_U16LSB: format_str = "U16LSB"; break;
			case AUDIO_S16LSB: format_str = "S16LSB"; break;
			case AUDIO_U16MSB: format_str = "U16MSB"; break;
			case AUDIO_S16MSB: format_str = "S16MSB"; break;
			}
	    
			// @@ Usually I don't care about this.  Need a IF_VERBOSE_SOUND or something.
			// IF_VERBOSE_DEBUG(
			// 	gnash::log_msg("SDL_mixer: opened %d times, frequency=%dHz, format=%s, stereo=%s\n",
			// 		     num_times_opened,
			// 		     m_sample_rate,
			// 		     format_str,
			// 		     m_stereo ? "yes" : "no"));
			// char name[32];
			// IF_VERBOSE_DEBUG(
			// 	gnash::log_msg("Using audio driver: %s\n", SDL_AudioDriverName(name, 32)));
		}
	}

	~SDL_sound_handler()
	{
		if (m_opened)
		{
			Mix_CloseAudio();
			for (int i = 0, n = m_samples.size(); i < n; i++)
			{
				if (m_samples[i])
				{
					Mix_FreeChunk(m_samples[i]);
				}
			}
		}
		else
		{
			assert(m_samples.size() == 0);
		}
	}


	virtual int	create_sound(
		void* data,
		int data_bytes,
		int sample_count,
		format_type format,
		int sample_rate,
		bool stereo)
	// Called to create a sample.  We'll return a sample ID that
	// can use for playing it.
	{
		if (m_opened == false)
		{
			return 0;
		}

		int16_t*	adjusted_data = 0;
		int	adjusted_size = 0;
		Mix_Chunk*	sample = 0;

		switch (format)
		{
		case FORMAT_RAW:
			convert_raw_data(&adjusted_data, &adjusted_size, data, sample_count, 1, sample_rate, stereo);
			break;

		case FORMAT_NATIVE16:
			convert_raw_data(&adjusted_data, &adjusted_size, data, sample_count, 2, sample_rate, stereo);
			break;

		case FORMAT_MP3:
#ifdef HAVE_MAD_H
			extern void convert_mp3_data(int16_t **adjusted_data, int *adjusted_size, void *data, const int sample_count, const int sample_size, const int sample_rate, const bool stereo);
			if (1) {
				int16_t*	x_adjusted_data = 0;
				int	x_adjusted_size = 0;
// 				Mix_Chunk*	x_sample = 0;
				convert_mp3_data(&x_adjusted_data, &x_adjusted_size, data, sample_count, 0, sample_rate, stereo);
				/* convert_mp3_data doesn't ACTUALLY convert
				   samplerate, so... */
				convert_raw_data(&adjusted_data, &adjusted_size, x_adjusted_data, sample_count, 0, sample_rate, stereo);
				if (x_adjusted_data) {
					delete x_adjusted_data;
				}
			} else {
				convert_mp3_data(&adjusted_data, &adjusted_size, data, sample_count, 0, sample_rate, stereo);
			}
#else
			log_error("mp3 format sound requested; this demo does not handle mp3\n");
#endif
			break;

		default:
			// Unhandled format.
			log_error("unknown format sound requested; this demo does not handle it\n");
			break;
		}

		if (adjusted_data)
		{
			sample = Mix_QuickLoad_RAW((unsigned char*) adjusted_data, adjusted_size);
			Mix_VolumeChunk(sample, MIX_MAX_VOLUME);	// full volume by default
		}

		m_samples.push_back(sample);
		return m_samples.size() - 1;
	}

	virtual void	play_sound(int sound_handle, int loop_count, int secondOffset, long start_position)
	// Play the index'd sample.
	{
	  if (m_opened && (sound_handle >= 0) && sound_handle < (int) m_samples.size())
		{
			if (m_samples[sound_handle])
			{
				// Play this sample on the first available channel.
				Mix_PlayChannel(-1, m_samples[sound_handle], loop_count);
			}
		}
	}

	virtual void	stop_all_sounds()
	{
		if (m_opened)
		{
			for (int i = 0; i < MIX_CHANNELS; i++)
			{
				if (Mix_Playing(i))
				{
					Mix_HaltChannel(i);
				}
			}
		}
	}

	virtual int	get_volume(int sound_handle)
	{
		int previous_volume = 100;
		if (m_opened && (sound_handle >= 0) && 
		    (unsigned int) sound_handle < m_samples.size())
		{
			//	if you passed a negative value for volume then
			//	this volume is still the current volume for the chunk
			previous_volume = Mix_VolumeChunk(m_samples[sound_handle], -1);
		}
		return previous_volume;
	}

	virtual void	set_volume(int sound_handle, int volume)
	{
		if (m_opened && sound_handle >= 0 && 
		    (unsigned int) sound_handle < m_samples.size())
		{
			int vol = (MIX_MAX_VOLUME / 100) * volume;
			Mix_VolumeChunk(m_samples[sound_handle], vol);
		}
	}
	
	virtual void	stop_sound(int sound_handle)
	{
		if (m_opened && sound_handle >= 0 && 
		    (unsigned int) sound_handle < m_samples.size())
		{
			for (int i = 0; i < MIX_CHANNELS; i++)
			{
				Mix_Chunk*	playing_chunk = Mix_GetChunk(i);
				if (playing_chunk == m_samples[sound_handle])
				{
					// Stop this channel.
					Mix_HaltChannel(i);
				}
			}
		}
	}


	virtual void	delete_sound(int sound_handle)
	// this gets called this when it's done with a sample.
	{
	  if (sound_handle >= 0 && sound_handle < (int) m_samples.size())
		{
			Mix_Chunk*	chunk = m_samples[sound_handle];
			if (chunk)
			{
				delete [] (chunk->abuf);
				Mix_FreeChunk(chunk);
				m_samples[sound_handle] = 0;
			}
		}
	}

	virtual void convert_raw_data(
		int16_t** adjusted_data,
		int* adjusted_size,
		void* data,
		int sample_count,
		int sample_size,
		int sample_rate,
		bool stereo)
	// VERY crude sample-rate & sample-size conversion.  Converts
	// input data to the SDL_mixer output format (SAMPLE_RATE,
	// stereo, 16-bit native endianness)
	{
// 		// xxxxx debug pass-thru
// 		{
// 			int	output_sample_count = sample_count * (stereo ? 2 : 1);
// 			int16_t*	out_data = new int16_t[output_sample_count];
// 			*adjusted_data = out_data;
// 			*adjusted_size = output_sample_count * 2;	// 2 bytes per sample
// 			memcpy(out_data, data, *adjusted_size);
// 			return;
// 		}
// 		// xxxxx

		// simple hack to handle dup'ing mono to stereo
		if ( !stereo && m_stereo)
		{
			sample_rate >>= 1;
		}

		 // simple hack to lose half the samples to get mono from stereo
		if ( stereo && !m_stereo)
		{
			sample_rate <<= 1; 
		}

		// Brain-dead sample-rate conversion: duplicate or
		// skip input samples an integral number of times.
		int	inc = 1;	// increment
		int	dup = 1;	// duplicate
		if (sample_rate > m_sample_rate)
		{
			inc = sample_rate / m_sample_rate;
		}
		else if (sample_rate < m_sample_rate)
		{
			dup = m_sample_rate / sample_rate;
		}

		int	output_sample_count = (sample_count * dup) / inc;
		int16_t*	out_data = new int16_t[output_sample_count];
		*adjusted_data = out_data;
		*adjusted_size = output_sample_count * 2;	// 2 bytes per sample

		if (sample_size == 1)
		{
			// Expand from 8 bit to 16 bit.
			uint8_t*	in = (uint8_t*) data;
			for (int i = 0; i < output_sample_count; i++)
			{
				uint8_t	val = *in;
				for (int j = 0; j < dup; j++)
				{
					*out_data++ = (int(val) - 128);
				}
				in += inc;
			}
		}
		else
		{
			// 16-bit to 16-bit conversion.
			int16_t*	in = (int16_t*) data;
			for (int i = 0; i < output_sample_count; i += dup)
			{
				int16_t	val = *in;
				for (int j = 0; j < dup; j++)
				{
					*out_data++ = val;
				}
				in += inc;
			}
		}
	}

};


gnash::sound_handler*	gnash::create_sound_handler_sdl()
// Factory.
{
	return new SDL_sound_handler;
}

#endif

// Local Variables:
// mode: C++
// End:
