// AudioDecoderSimple.cpp: Audio decoding using "simple" internal decoders.
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

#include <boost/scoped_array.hpp>

#include "AudioDecoderSimple.h"
#include "utility.h"

namespace gnash {

class BitReader 
{
public:
	typedef unsigned char byte;

	/// Ownership of buffer left to caller
	BitReader(byte* input, size_t len)
		:
		start(input),
		ptr(input),
		end(ptr+len),
		usedBits(0)
	{
	}

	~BitReader() {}

	/// Set a new buffer to work with
	void setBuffer(byte* input, size_t len)
	{
		start = ptr = input;
		end = start+len;
		usedBits = 0;
	}

	bool read_bit()
	{
		bool ret = (*ptr&(128>>usedBits));
		if ( ++usedBits == 8 ) advanceToNextByte();
		return ret;
	}

	unsigned int read_uint(unsigned short bitcount)
	{
		assert(bitcount <= 32);

		uint32_t value = 0;

		unsigned short bits_needed = bitcount;
		do
		{
			int unusedMask = 0xFF >> usedBits;
			int unusedBits = 8-usedBits;

			if (bits_needed == unusedBits)
			{
				// Consume all the unused bits.
				value |= (*ptr&unusedMask);
				advanceToNextByte();
				break;

			}
			else if (bits_needed > unusedBits)
			{
				// Consume all the unused bits.

				bits_needed -= unusedBits; // assert(bits_needed>0)

				value |= ((*ptr&unusedMask) << bits_needed);
				advanceToNextByte();
			}
			else
			{
				assert(bits_needed <= unusedBits);

				// Consume some of the unused bits.

				unusedBits -= bits_needed;

				value |= ((*ptr&unusedMask) >> unusedBits);

				usedBits += bits_needed;
				if ( usedBits >= 8 ) advanceToNextByte();

				// We're done.
				break;
			}
		}
		while (bits_needed > 0);

		return value;

	}


	int32_t read_sint(unsigned short bitcount)
	{
		int32_t	value = int32_t(read_uint(bitcount));

		// Sign extend...
		if (value & (1 << (bitcount - 1))) 
			value |= -1 << bitcount;

		return value;
	}


private:

	void advanceToNextByte()
	{
		if ( ++ptr == end )
		{
			log_debug("Going round");
			ptr=start;
		}
		usedBits=0;
	}

	/// Pointer to first byte
	byte* start;

	/// Pointer to current byte
	byte* ptr;

	/// Pointer to one past last byte
	byte* end;

	/// Number of used bits in current byte
	unsigned usedBits;

};

// ----------------------------------------------------------------------------
// ADPCMDecoder class
// ----------------------------------------------------------------------------

/// ADPCM decoder utilities
//
/// Algo from http://www.circuitcellar.com/pastissues/articles/richey110/text.htm
/// And also Jansen.
/// Here's another reference: http://www.geocities.com/SiliconValley/8682/aud3.txt
/// Original IMA spec doesn't seem to be on the web :(
///
/// TODO: move in it's own file
///
class ADPCMDecoder {

private:

	// Data from Alexis' SWF reference
	static int _index_update_table_2bits[2];
	static int _index_update_table_3bits[4];
	static int _index_update_table_4bits[8];
	static int _index_update_table_5bits[16];

	static int* s_index_update_tables[4];

	// Data from Jansen.  http://homepages.cwi.nl/~jack/
	// Check out his Dutch retro punk songs, heh heh :)
	static const int STEPSIZE_CT = 89;
	static int s_stepsize[STEPSIZE_CT];


	static void doSample(int n_bits, int& sample, int& stepsize_index, int raw_code)
	{
		assert(raw_code >= 0 && raw_code < (1 << n_bits));								
																
		static const int	HI_BIT = (1 << (n_bits - 1));								
		int*	index_update_table = s_index_update_tables[n_bits - 2];							
																
		/* Core of ADPCM. */												
																
		int	code_mag = raw_code & (HI_BIT - 1);									
		bool	code_sign_bit = (raw_code & HI_BIT) ? 1 : 0;								
		int	mag = (code_mag << 1) + 1;	/* shift in LSB (they do this so that pos & neg zero are different)*/	
																
		int	stepsize = s_stepsize[stepsize_index];									
																
		/* Compute the new sample.  It's the predicted value			*/					
		/* (i.e. the previous value), plus a delta.  The delta			*/					
		/* comes from the code times the stepsize.  going for			*/					
		/* something like: delta = stepsize * (code * 2 + 1) >> code_bits	*/					
		int	delta = (stepsize * mag) >> (n_bits - 1);								
		if (code_sign_bit) delta = -delta;										
																
		sample += delta;												
		sample = iclamp(sample, -32768, 32767);										
																
		/* Update our stepsize index.  Use a lookup table. */								
		stepsize_index += index_update_table[code_mag];									
		stepsize_index = iclamp(stepsize_index, 0, STEPSIZE_CT - 1);							
	}

	/* Uncompress 4096 mono samples of ADPCM. */									
	static void doMonoBlock(int16_t** out_data, int n_bits, int sample_count, BitReader* in, int sample, int stepsize_index)
	{
		/* First sample doesn't need to be decompressed. */								
		sample_count--;													
		*(*out_data)++ = (int16_t) sample;										
																
		while (sample_count--)												
		{														
			int	raw_code = in->read_uint(n_bits);								
			doSample(n_bits, sample, stepsize_index, raw_code);	/* sample & stepsize_index are in/out params */	
			*(*out_data)++ = (int16_t) sample;									
		}														
	}


	/* Uncompress 4096 stereo sample pairs of ADPCM. */									
	static void doStereoBlock(
			int16_t** out_data,	// in/out param
			int n_bits,
			int sample_count,
			BitReader* in,
			int left_sample,
			int left_stepsize_index,
			int right_sample,
			int right_stepsize_index
			)
	{
		/* First samples don't need to be decompressed. */								
		sample_count--;													
		*(*out_data)++ = (int16_t) left_sample;										
		*(*out_data)++ = (int16_t) right_sample;										
																
		while (sample_count--)												
		{														
			int	left_raw_code = in->read_uint(n_bits);								
			doSample(n_bits, left_sample, left_stepsize_index, left_raw_code);					
			*(*out_data)++ = (int16_t) left_sample;									
																
			int	right_raw_code = in->read_uint(n_bits);								
			doSample(n_bits, right_sample, right_stepsize_index, right_raw_code);					
			*(*out_data)++ = (int16_t) right_sample;									
		}														
	}

public:

	// Utility function: uncompress ADPCM data from in BitReader to
	// out_data[].	The output buffer must have (sample_count*2)
	// bytes for mono, or (sample_count*4) bytes for stereo.
	static void adpcm_expand(
		unsigned char* &data,
		BitReader* in,
		unsigned int sample_count,	// in stereo, this is number of *pairs* of samples
		bool stereo)
	{
		int16_t* out_data = new int16_t[stereo ? sample_count*2 : sample_count];
		data = reinterpret_cast<unsigned char *>(out_data);

		// Read header.
		//in->ensureBytes(1); // nbits
		unsigned int n_bits = in->read_uint(2) + 2;	// 2 to 5 bits (TODO: use unsigned...)

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
#define DO_MONO(n) doMonoBlock(&out_data, n, samples_this_block, in, sample, stepsize_index)

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
		doStereoBlock(				\
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

};

int ADPCMDecoder::_index_update_table_2bits[2] = { -1,  2 };
int ADPCMDecoder::_index_update_table_3bits[4] = { -1, -1,  2,  4 };
int ADPCMDecoder::_index_update_table_4bits[8] = { -1, -1, -1, -1,  2,  4,  6,  8 };
int ADPCMDecoder::_index_update_table_5bits[16] = { -1, -1, -1, -1, -1, -1, -1, -1, 1,  2,  4,  6,  8, 10, 13, 16 };

int* ADPCMDecoder::s_index_update_tables[4] = {
	ADPCMDecoder::_index_update_table_2bits,
	ADPCMDecoder::_index_update_table_3bits,
	ADPCMDecoder::_index_update_table_4bits,
	ADPCMDecoder::_index_update_table_5bits
};

int ADPCMDecoder::s_stepsize[STEPSIZE_CT] = {
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

// ----------------------------------------------------------------------------
// END OF ADPCMDecoder class
// ----------------------------------------------------------------------------

//
// Unsigned 8-bit expansion (128 is silence)
//
// u8_expand allocates the memory for its "data" pointer.
//

static void u8_expand(
	unsigned char * &data,
	unsigned char* input,
	uint32_t input_size) // This is also the number of u8bit samples
{
	boost::scoped_array<uint8_t> in_data ( new uint8_t[input_size] );
	int16_t	*out_data = new int16_t[input_size];

	memcpy((char *)in_data.get(), input, input_size);

	// Convert 8-bit to 16
	uint8_t *inp = in_data.get();
	int16_t *outp = out_data;
	for (unsigned int i = input_size; i>0; i--) {
		*outp++ = ((int16_t)(*inp++) - 128) * 256;
	}
	
	data = (unsigned char *)out_data;
}


AudioDecoderSimple::AudioDecoderSimple ()
	:
	_sampleRate(0),
	_sampleCount(0),
	_stereo(false),
	_is16bit(true)
{
}

AudioDecoderSimple::~AudioDecoderSimple()
{

}

bool AudioDecoderSimple::setup(SoundInfo* info)
{
	if (info->getFormat() == AUDIO_CODEC_ADPCM || info->getFormat() == AUDIO_CODEC_RAW || info->getFormat() == AUDIO_CODEC_UNCOMPRESSED) {
		_codec = info->getFormat();
		_sampleRate = info->getSampleRate();
		_sampleCount = info->getSampleCount();
		_stereo = info->isStereo();
		_is16bit = info->is16bit();
		return true;
	}
	return false;
}

bool AudioDecoderSimple::setup(AudioInfo* info)
{
	if (info->type == FLASH && (info->codec == AUDIO_CODEC_ADPCM || info->codec == AUDIO_CODEC_RAW || info->codec == AUDIO_CODEC_UNCOMPRESSED)) {
		_codec = static_cast<audioCodecType>(info->codec);
		_sampleRate = info->sampleRate;
		_stereo = info->stereo;
		_is16bit = true; // Fix this?
		return true;
	} else {
		return false;
	}
}

uint8_t* AudioDecoderSimple::decode(uint8_t* input, uint32_t inputSize, uint32_t& outputSize, uint32_t& decodedBytes, bool /*parse*/)
{

	unsigned char* decodedData = NULL;
	int outsize = 0;

    switch (_codec) {
	case AUDIO_CODEC_ADPCM:
		{
		uint32_t sample_count = inputSize * ( _stereo ? 1 : 2 ); //(_sampleCount == 0 ? inputSize / ( _stereo ? 4 : 2 ) : _sampleCount);
		ADPCMDecoder::adpcm_expand(decodedData, new BitReader(input,inputSize), sample_count, _stereo);
		outsize = sample_count * (_stereo ? 4 : 2);
		}
		break;
	case AUDIO_CODEC_RAW:
		if (_is16bit) {
			// FORMAT_RAW 16-bit is exactly what we want!
			decodedData = new unsigned char[inputSize];
			memcpy(decodedData, input, inputSize);
			outsize = inputSize;
		} else {
			// Convert 8-bit signed to 16-bit range
			// Allocate as many shorts as there are samples
			u8_expand(decodedData, input, inputSize);
			outsize = inputSize * (_stereo ? 4 : 2);
		}
		break;
	case AUDIO_CODEC_UNCOMPRESSED:
		// 8- or 16-bit mono or stereo little-endian audio
		// Convert to 16-bit host-endian.
		if (!_is16bit)
		{
			// Convert 8-bit signed to 16-bit range
			// Allocate as many shorts as there are 8-bit samples
			u8_expand(decodedData, input, inputSize);
			outsize = inputSize * (_stereo ? 4 : 2);

		} else {
			// Read 16-bit data into buffer
			decodedData = new unsigned char[inputSize];
			memcpy((char *)decodedData, input, inputSize);

			// Convert 16-bit little-endian data to host-endian.

			// Runtime detection of host endianness costs almost
			// nothing and is less of a continual maintenance headache
			// than compile-time detection.
			union u {
				uint16_t s;
				struct {
					uint8_t c0;
					uint8_t c1;
				} c;
			} u = { 0x0001 };

			switch (u.c.c0) {
				case 0x01:	// Little-endian host: sample is already native.
					break;
				case 0x00:  // Big-endian host
					// Swap sample bytes to get big-endian format.
					assert(inputSize & 1 == 0);
					for (unsigned i = 0; i < inputSize; i+=2)
					{
						swap(&decodedData[i], &decodedData[i+1]);
					}
					break;
    			default:	// Impossible
					log_error(_("Host endianness not detected in AudioDecoderSimple"));
					// Just carry on anyway...
			}
		}
		break;
		default:
		break;
		// ???, this should only decode ADPCM, RAW and UNCOMPRESSED
	}

	uint8_t* tmp_raw_buffer = decodedData;
	uint32_t tmp_raw_buffer_size = 0;

	// If we need to convert samplerate or/and from mono to stereo...
	if (outsize > 0 && (_sampleRate != 44100 || !_stereo)) {

		int16_t* adjusted_data = 0;
		int	adjusted_size = 0;
		int sample_count = outsize / 2;// samples are of size 2

		// Convert to needed samplerate - this converter only support standard flash samplerates
		convert_raw_data(&adjusted_data, &adjusted_size, tmp_raw_buffer, sample_count, 0, 
				_sampleRate, _stereo,
				44100,  true /* stereo */);

		// Hopefully this wont happen
		if (!adjusted_data) {
			log_error(_("Error in sound sample conversion"));
			delete[] tmp_raw_buffer;
			outputSize = 0;
			decodedBytes = 0;
			return NULL;
		}

		// Move the new data to the sound-struct
		delete[] tmp_raw_buffer;
		tmp_raw_buffer = reinterpret_cast<uint8_t*>(adjusted_data);
		tmp_raw_buffer_size = adjusted_size;

	} else {
		tmp_raw_buffer_size = outsize;
	}

	outputSize = tmp_raw_buffer_size;
	decodedBytes = inputSize;
	return tmp_raw_buffer;
}

} // gnash namespace
