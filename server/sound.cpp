// sound.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Code to handle SWF sound-related tags.


#include "sound.h"
#include "stream.h"
#include "impl.h"
#include "execute_tag.h" // for start_sound_tag inheritance
#include "movie_definition.h"

namespace gnash {

namespace globals {

	// Callback interface to host, for handling sounds.  If it's NULL,
	// sound is ignored.
	sound_handler*	s_sound_handler = 0;

} // namespace gnash::global


	void	set_sound_handler(sound_handler* s)
	// Called by host, to set a handler for all sounds.
	// Can pass in 0 to disable sound.
	{
		globals::s_sound_handler = s;
	}


	sound_handler*	get_sound_handler()
	{
		return globals::s_sound_handler;
	}


	sound_sample_impl::~sound_sample_impl()
	{
		if (globals::s_sound_handler)
		{
			globals::s_sound_handler->delete_sound(m_sound_handler_id);
		}
	}



//
// SWF Tag StartSound (15) 
//

void
start_sound_tag::read(stream* in, int /* tag_type */, movie_definition* m,
		const sound_sample_impl* sam)
{
	assert(sam);

	in->read_uint(2);	// skip reserved bits.
	m_stop_playback = in->read_uint(1) ? true : false;
	bool	no_multiple = in->read_uint(1) ? true : false;
	bool	has_envelope = in->read_uint(1) ? true : false;
	bool	has_loops = in->read_uint(1) ? true : false;
	bool	has_out_point = in->read_uint(1) ? true : false;
	bool	has_in_point = in->read_uint(1) ? true : false;

	UNUSED(no_multiple);
	UNUSED(has_envelope);
	
	uint32_t	in_point = 0;
	uint32_t	out_point = 0;
	if (has_in_point) { in_point = in->read_u32(); }
	if (has_out_point) { out_point = in->read_u32(); }
	if (has_loops) { m_loop_count = in->read_u16(); }
	// if (has_envelope) { env_count = read_uint8(); read envelope entries; }

	m_handler_id = sam->m_sound_handler_id;
	m->add_execute_tag(this);
}


void
start_sound_tag::execute(movie* /* m */)
{
	using globals::s_sound_handler;

	if (s_sound_handler)
	{
		if (m_stop_playback)
		{
			s_sound_handler->stop_sound(m_handler_id);
		}
		else
		{
			s_sound_handler->play_sound(m_handler_id, m_loop_count, 0,0);
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
start_stream_sound_tag::execute(movie* /* m */)
{
	using globals::s_sound_handler;
	if (s_sound_handler)
	{
		s_sound_handler->play_sound(m_handler_id, 0, 0, m_start);
	}
}


	// void	define_button_sound(...) ???


// @@ currently not implemented
//	void	sound_stream_loader(stream* in, int tag_type, movie_definition* m)
//	// Load the various stream-related tags: SoundStreamHead,
//	// SoundStreamHead2, SoundStreamBlock.
//	{
//	}


	//
	// ADPCM
	//


	// Data from Alexis' SWF reference
	static int	s_index_update_table_2bits[2] = { -1,  2 };
	static int	s_index_update_table_3bits[4] = { -1, -1,  2,  4 };
	static int	s_index_update_table_4bits[8] = { -1, -1, -1, -1,  2,  4,  6,  8 };
	static int	s_index_update_table_5bits[16] = { -1, -1, -1, -1, -1, -1, -1, -1, 1,  2,  4,  6,  8, 10, 13, 16 };

	static int*	s_index_update_tables[4] = {
		s_index_update_table_2bits,
		s_index_update_table_3bits,
		s_index_update_table_4bits,
		s_index_update_table_5bits,
	};

	// Data from Jansen.  http://homepages.cwi.nl/~jack/
	// Check out his Dutch retro punk songs, heh heh :)
	const int STEPSIZE_CT = 89;
	static int s_stepsize[STEPSIZE_CT] = {
		7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
		19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
		50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
		130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
		337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
		876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
		2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
		5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
		15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
	};


	// Algo from http://www.circuitcellar.com/pastissues/articles/richey110/text.htm
	// And also Jansen.
	// Here's another reference: http://www.geocities.com/SiliconValley/8682/aud3.txt
	// Original IMA spec doesn't seem to be on the web :(


	// @@ lots of macros here!  It seems that VC6 can't correctly
	// handle integer template args, although it's happy to
	// compile them?!

//	void DO_SAMPLE(int n_bits, int& sample, int& stepsize_index, int raw_code)
#define DO_SAMPLE(n_bits, sample, stepsize_index, raw_code)									\
	{															\
		assert(raw_code >= 0 && raw_code < (1 << n_bits));								\
																\
		static const int	HI_BIT = (1 << (n_bits - 1));								\
		int*	index_update_table = s_index_update_tables[n_bits - 2];							\
																\
		/* Core of ADPCM. */												\
																\
		int	code_mag = raw_code & (HI_BIT - 1);									\
		bool	code_sign_bit = (raw_code & HI_BIT) ? 1 : 0;								\
		int	mag = (code_mag << 1) + 1;	/* shift in LSB (they do this so that pos & neg zero are different)*/	\
																\
		int	stepsize = s_stepsize[stepsize_index];									\
																\
		/* Compute the new sample.  It's the predicted value			*/					\
		/* (i.e. the previous value), plus a delta.  The delta			*/					\
		/* comes from the code times the stepsize.  going for			*/					\
		/* something like: delta = stepsize * (code * 2 + 1) >> code_bits	*/					\
		int	delta = (stepsize * mag) >> (n_bits - 1);								\
		if (code_sign_bit) delta = -delta;										\
																\
		sample += delta;												\
		sample = iclamp(sample, -32768, 32767);										\
																\
		/* Update our stepsize index.  Use a lookup table. */								\
		stepsize_index += index_update_table[code_mag];									\
		stepsize_index = iclamp(stepsize_index, 0, STEPSIZE_CT - 1);							\
	}


	struct in_stream
	{
		const unsigned char*	m_in_data;
		int	m_current_bits;
		int	m_unused_bits;

		in_stream(const unsigned char* data)
			:
			m_in_data(data),
			m_current_bits(0),
			m_unused_bits(0)
		{
		}
	};


//	void DO_MONO_BLOCK(int16_t** out_data, int n_bits, int sample_count, stream* in, int sample, int stepsize_index)
#define DO_MONO_BLOCK(out_data, n_bits, sample_count, in, sample, stepsize_index)						\
	{															\
		/* First sample doesn't need to be decompressed. */								\
		sample_count--;													\
		*(*out_data)++ = (int16_t) sample;										\
																\
		while (sample_count--)												\
		{														\
			int	raw_code = in->read_uint(n_bits);								\
			DO_SAMPLE(n_bits, sample, stepsize_index, raw_code);	/* sample & stepsize_index are in/out params */	\
			*(*out_data)++ = (int16_t) sample;									\
		}														\
	}


//	void do_stereo_block(
//		int16_t** out_data,	// in/out param
//		int n_bits,
//		int sample_count,
//		stream* in,
//		int left_sample,
//		int left_stepsize_index,
//		int right_sample,
//		int right_stepsize_index
//		)
#define DO_STEREO_BLOCK(out_data, n_bits, sample_count, in, left_sample, left_stepsize_index, right_sample, right_stepsize_index) \
	/* Uncompress 4096 stereo sample pairs of ADPCM. */									  \
	{															  \
		/* First samples don't need to be decompressed. */								  \
		sample_count--;													  \
		*(*out_data)++ = (int16_t) left_sample;										  \
		*(*out_data)++ = (int16_t) right_sample;										  \
																  \
		while (sample_count--)												  \
		{														  \
			int	left_raw_code = in->read_uint(n_bits);								  \
			DO_SAMPLE(n_bits, left_sample, left_stepsize_index, left_raw_code);					  \
			*(*out_data)++ = (int16_t) left_sample;									  \
																  \
			int	right_raw_code = in->read_uint(n_bits);								  \
			DO_SAMPLE(n_bits, right_sample, right_stepsize_index, right_raw_code);					  \
			*(*out_data)++ = (int16_t) right_sample;									  \
		}														  \
	}


	// Utility function: uncompress ADPCM data from in stream to
	// out_data[].	The output buffer must have (sample_count*2)
	// bytes for mono, or (sample_count*4) bytes for stereo.
	void	sound_handler::adpcm_expand(
		void* out_data_void,
		stream* in,
		int sample_count,	// in stereo, this is number of *pairs* of samples
		bool stereo)
	{
		int16_t*	out_data = (int16_t*) out_data_void;

		// Read header.
		int	n_bits = in->read_uint(2) + 2;	// 2 to 5 bits

		while (sample_count)
		{
			// Read initial sample & index values.
			int	sample = in->read_sint(16);

			int	stepsize_index = in->read_uint(6);
			assert(STEPSIZE_CT >= (1 << 6));	// ensure we don't need to clamp.

			int	samples_this_block = imin(sample_count, 4096);
			sample_count -= samples_this_block;

			if (stereo == false)
			{
#define DO_MONO(n) DO_MONO_BLOCK(&out_data, n, samples_this_block, in, sample, stepsize_index)

				switch (n_bits)
				{
				default: assert(0); break;
				case 2: DO_MONO(2); break;
				case 3: DO_MONO(3); break;
				case 4: DO_MONO(4); break;
				case 5: DO_MONO(5); break;
				}
			}
			else
			{
				// Stereo.

				// Got values for left channel; now get initial sample
				// & index for right channel.
				int	right_sample = in->read_sint(16);

				int	right_stepsize_index = in->read_uint(6);
				assert(STEPSIZE_CT >= (1 << 6));	// ensure we don't need to clamp.

#define DO_STEREO(n)					\
	DO_STEREO_BLOCK(				\
		&out_data, n, samples_this_block,	\
		in, sample, stepsize_index,		\
		right_sample, right_stepsize_index)
			
				switch (n_bits)
				{
				default: assert(0); break;
				case 2: DO_STEREO(2); break;
				case 3: DO_STEREO(3); break;
				case 4: DO_STEREO(4); break;
				case 5: DO_STEREO(5); break;
				}
			}
		}
	}

} // namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
