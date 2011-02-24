// AudioDecoderSimple.cpp: Audio decoding using "simple" internal decoders.
// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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


#include "AudioDecoderSimple.h"
#include "AudioResampler.h"
#include "BitsReader.h"
#include "SoundInfo.h"
#include "MediaParser.h" // for AudioInfo definition..
#include "GnashNumeric.h" // for clamp

#include "log.h"

#include <boost/scoped_array.hpp>
#include <algorithm> // for std::swap

namespace gnash {
namespace media {

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
		sample = clamp<int>(sample, -32768, 32767);

		/* Update our stepsize index.  Use a lookup table. */
		stepsize_index += index_update_table[code_mag];
		stepsize_index = clamp<int>(stepsize_index, 0, STEPSIZE_CT - 1);
	}

	/* Uncompress 4096 mono samples of ADPCM. */
	static boost::uint32_t doMonoBlock(boost::int16_t** out_data, int n_bits, BitsReader& in, int sample, int stepsize_index)
	{
		/* First sample doesn't need to be decompressed. */
		boost::uint32_t sample_count = 1;
		*(*out_data)++ = (boost::int16_t) sample;

		while (sample_count < 4096 && in.gotBits(n_bits))
		{
			int	raw_code = in.read_uint(n_bits);
			doSample(n_bits, sample, stepsize_index, raw_code);	/* sample & stepsize_index are in/out params */
			*(*out_data)++ = (boost::int16_t) sample;

			sample_count++;
		}	
		return sample_count;
	}


	/* Uncompress 4096 stereo sample pairs of ADPCM. */
	static int doStereoBlock(
			boost::int16_t** out_data,	// in/out param
			int n_bits,
			BitsReader& in,
			int left_sample,
			int left_stepsize_index,
			int right_sample,
			int right_stepsize_index
			)
	{
		/* First samples don't need to be decompressed. */
		boost::uint32_t sample_count = 2;
		*(*out_data)++ = (boost::int16_t) left_sample;
		*(*out_data)++ = (boost::int16_t) right_sample;

		unsigned bitsNeeded = n_bits*2;
		while (sample_count < 4096 && in.gotBits(bitsNeeded))
		{														
			int	left_raw_code = in.read_uint(n_bits);
			doSample(n_bits, left_sample, left_stepsize_index, left_raw_code);
			*(*out_data)++ = (boost::int16_t) left_sample;

			int	right_raw_code = in.read_uint(n_bits);
			doSample(n_bits, right_sample, right_stepsize_index, right_raw_code);
			*(*out_data)++ = (boost::int16_t) right_sample;

			sample_count++;
		}
		return sample_count;
	}

public:

	// Utility function: uncompress ADPCM data from in BitReader to
	// out_data[]. Returns the output samplecount.
	static boost::uint32_t adpcm_expand(
		unsigned char* &data,
		BitsReader& in,
		unsigned int insize,
		bool stereo)
	{
		// Read header.
		if ( ! in.gotBits(2) )
		{
			IF_VERBOSE_MALFORMED_SWF(
			log_swferror(_("corrupted ADPCM header"));
			);
			return 0;
		}
		unsigned int n_bits = in.read_uint(2) + 2; // 2 to 5 bits 

		// The compression ratio is 4:1, so this should be enough...
		boost::int16_t* out_data = new boost::int16_t[insize * 5];
		data = reinterpret_cast<unsigned char *>(out_data);

		boost::uint32_t sample_count = 0;

		while ( in.gotBits(22) )
		{
			// Read initial sample & index values.

			int	sample = in.read_sint(16);

			int	stepsize_index = in.read_uint(6);
			assert(STEPSIZE_CT >= (1 << 6));	// ensure we don't need to clamp.

			if (stereo == false)
			{

				if (n_bits == 0) {
					abort();
				} else if (n_bits == 2) {
					sample_count += doMonoBlock(&out_data, 2, in, sample, stepsize_index);
				} else if (n_bits == 3) {
					sample_count += doMonoBlock(&out_data, 3, in, sample, stepsize_index);
				} else if (n_bits == 4) {
					sample_count += doMonoBlock(&out_data, 4, in, sample, stepsize_index);
				} else if (n_bits == 5) {
					sample_count += doMonoBlock(&out_data, 5, in, sample, stepsize_index);
				}
			}
			else
			{
				// Stereo.

				// Got values for left channel; now get initial sample
				// & index for right channel.
				int	right_sample = in.read_sint(16);

				int	right_stepsize_index = in.read_uint(6);
				assert(STEPSIZE_CT >= (1 << 6));	// ensure we don't need to clamp.

				if (n_bits == 0) {
					abort();
				} else if (n_bits == 2) {
					sample_count += doStereoBlock(&out_data, 2, in, sample, stepsize_index, right_sample, right_stepsize_index);
				} else if (n_bits == 3) {
					sample_count += doStereoBlock(&out_data, 3, in, sample, stepsize_index, right_sample, right_stepsize_index);
				} else if (n_bits == 4) {
					sample_count += doStereoBlock(&out_data, 4, in, sample, stepsize_index, right_sample, right_stepsize_index);
				} else if (n_bits == 5) {
					sample_count += doStereoBlock(&out_data, 5, in, sample, stepsize_index, right_sample, right_stepsize_index);
				}

			}
		}
		return sample_count;

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

static void
u8_expand(unsigned char * &data,
	const unsigned char* input,
	boost::uint32_t input_size) // This is also the number of u8bit samples
{
	boost::int16_t	*out_data = new boost::int16_t[input_size];

	// Convert 8-bit to 16
	const boost::uint8_t *inp = input;
	boost::int16_t *outp = out_data;
	for (unsigned int i = input_size; i>0; i--) {
		*outp++ = ((boost::int16_t)(*inp++) - 128) * 256;
	}
	
	data = (unsigned char *)out_data;
}


AudioDecoderSimple::AudioDecoderSimple(const AudioInfo& info)
	:
	_sampleRate(0),
	_sampleCount(0),
	_stereo(false),
	_is16bit(true)
{
    setup(info);

  	log_debug(_("AudioDecoderSimple: initialized flash codec %s (%d)"),
		(int)_codec, _codec);
}

AudioDecoderSimple::AudioDecoderSimple(const SoundInfo& info)
	:
	_sampleRate(0),
	_sampleCount(0),
	_stereo(false),
	_is16bit(true)
{
    setup(info);

  	log_debug(_("AudioDecoderSimple: initialized flash codec %s (%d)"),
		(int)_codec, _codec);
}


AudioDecoderSimple::~AudioDecoderSimple()
{
}

void
AudioDecoderSimple::setup(const SoundInfo& info)
{
	_codec = info.getFormat();
    switch (_codec)
    {
        case AUDIO_CODEC_ADPCM:
        case AUDIO_CODEC_RAW:
        case AUDIO_CODEC_UNCOMPRESSED:
		    _sampleRate = info.getSampleRate();
		    _sampleCount = info.getSampleCount();
		    _stereo = info.isStereo();
		    _is16bit = info.is16bit();
            break;

        default:
            boost::format err = boost::format(
                _("AudioDecoderSimple: unsupported flash codec %d (%s)"))
                % (int)_codec % _codec;
            throw MediaException(err.str());
	}
}

void
AudioDecoderSimple::setup(const AudioInfo& info)
{
    if (info.type != CODEC_TYPE_FLASH) {
        boost::format err = boost::format(
            _("AudioDecoderSimple: unable to intepret custom audio codec id %s"))
            % info.codec;
        throw MediaException(err.str());
    }

    _codec = static_cast<audioCodecType>(info.codec);
    switch (_codec)
    {
        case AUDIO_CODEC_ADPCM:
        case AUDIO_CODEC_RAW:
        case AUDIO_CODEC_UNCOMPRESSED:
            _sampleRate = info.sampleRate;
            _stereo = info.stereo;
            _is16bit = (info.sampleSize==2); // check this!
            if ( info.sampleSize > 2 ) // troubles...
                log_unimpl("Sample size > 2 in %s sound!", _codec);
            break;

        default:
            boost::format err = boost::format(
                _("AudioDecoderSimple: unsupported flash codec %d (%s)"))
                % (int)_codec % _codec;
            throw MediaException(err.str());
	}
}

boost::uint8_t*
AudioDecoderSimple::decode(const boost::uint8_t* input, boost::uint32_t inputSize,
        boost::uint32_t& outputSize, boost::uint32_t& decodedBytes,
        bool /*parse*/)
{

	unsigned char* decodedData = NULL;
	int outsize = 0;

    switch (_codec) {
	case AUDIO_CODEC_ADPCM:
		{
		//boost::uint32_t sample_count = inputSize * ( _stereo ? 1 : 2 ); //(_sampleCount == 0 ? inputSize / ( _stereo ? 4 : 2 ) : _sampleCount);
		BitsReader br(input, inputSize);
		boost::uint32_t sample_count = ADPCMDecoder::adpcm_expand(decodedData, br, inputSize, _stereo);
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
				boost::uint16_t s;
				struct {
					boost::uint8_t c0;
					boost::uint8_t c1;
				} c;
			} u = { 0x0001 };

			switch (u.c.c0) {
				case 0x01:	// Little-endian host: sample is already native.
					break;
				case 0x00:  // Big-endian host
					// Swap sample bytes to get big-endian format.
					assert((inputSize & 1) == 0);
					for (unsigned i = 0; i < inputSize; i+=2)
					{
						std::swap(decodedData[i], decodedData[i+1]);
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

	boost::uint8_t* tmp_raw_buffer = decodedData;
	boost::uint32_t tmp_raw_buffer_size = 0;

	// If we need to convert samplerate or/and from mono to stereo...
	if (outsize > 0 && (_sampleRate != 44100 || !_stereo)) {

		boost::int16_t* adjusted_data = 0;
		int	adjusted_size = 0;
		int sample_count = outsize / (_stereo ? 4 : 2); // samples are of size 2

		// Convert to needed samplerate - this converter only support standard flash samplerates
		AudioResampler::convert_raw_data(&adjusted_data, &adjusted_size,
                tmp_raw_buffer,
                sample_count, 2,  // input sample size is 2 !
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
		tmp_raw_buffer = reinterpret_cast<boost::uint8_t*>(adjusted_data);
		tmp_raw_buffer_size = adjusted_size;

	} else {
		tmp_raw_buffer_size = outsize;
	}

	outputSize = tmp_raw_buffer_size;

	decodedBytes = inputSize;
	return tmp_raw_buffer;
}

} // gnash.media namespace 
} // gnash namespace
