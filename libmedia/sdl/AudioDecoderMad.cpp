// AudioDecoderMad.cpp: Audio decoding using the mad library.
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

// $Id: AudioDecoderMad.cpp,v 1.3 2007/10/04 09:41:46 tgc Exp $

#include "AudioDecoderMad.h"
#include "utility.h"
#ifdef USE_FFMPEG
#include <ffmpeg/avcodec.h>
#endif

namespace gnash {
	
AudioDecoderMad::AudioDecoderMad ()
{
	// Init the mad decoder
	mad_stream_init(&_stream);
	mad_frame_init(&_frame);
	mad_synth_init(&_synth);
}

AudioDecoderMad::~AudioDecoderMad()
{
	mad_synth_finish(&_synth);
	mad_frame_finish(&_frame);
	mad_stream_finish(&_stream);
}


bool AudioDecoderMad::setup(SoundInfo* info)
{
	if (info->getFormat() == FORMAT_MP3) return true;
	else return false;
}

bool AudioDecoderMad::setup(AudioInfo* info)
{
#ifdef USE_FFMPEG
	if (info->type == FFMPEG && info->codec == CODEC_ID_MP3) {
		return true;
	} 
#endif
	if (info->type == FLASH && info->codec == AUDIO_CODEC_MP3) {
		return true;
	} else {
		return false;
	}
}

uint8_t* AudioDecoderMad::decode(uint8_t* input, uint32_t inputSize, uint32_t& outputSize, uint32_t& decodedBytes, bool parse)
{
	// Setup the mad decoder
	mad_stream_buffer(&_stream, input, inputSize);

	int ret;
	const unsigned char* old_next_frame = _stream.next_frame;
	int loops = 0;
	while(true) {

		ret = mad_frame_decode(&_frame, &_stream);
		loops++;
		
		// There is always some junk in front of the data, 
		// so we continue until we get past it.
		if (ret && _stream.error == MAD_ERROR_LOSTSYNC) continue;
		
		// Error handling is done by relooping (max. 8 times) and just hooping that it will work...
		if (loops > 8) break;
		if (ret == -1 && _stream.error != MAD_ERROR_BUFLEN && MAD_RECOVERABLE(_stream.error)) {
			log_debug(_("Recoverable error while decoding MP3-stream, MAD error: %s"), mad_stream_errorstr (&_stream));
			continue;
		}
		
		break;
	}

	if (ret == -1 && _stream.error != MAD_ERROR_BUFLEN) {
		log_error(_("Unrecoverable error while decoding MP3-stream, MAD error: %s"), mad_stream_errorstr (&_stream));
		outputSize = 0;
		decodedBytes = 0;
		return NULL;
	} else if (ret == -1 && _stream.error == MAD_ERROR_BUFLEN) {
		// the buffer is empty, no more to decode!
		decodedBytes = inputSize;
	} else {
		decodedBytes = _stream.next_frame - old_next_frame;
	}

	mad_synth_frame (&_synth, &_frame);
	
	uint32_t outsize = _synth.pcm.length * _synth.pcm.channels * 2;

	uint8_t* tmp_raw_buffer = new uint8_t[outsize];
	uint32_t tmp_raw_buffer_size = 0;
	int sample;
	
	int16_t* dst = reinterpret_cast<int16_t*>(tmp_raw_buffer);

	// transfer the decoded samples into the sound-struct, and do some
	// scaling while we're at it.
	for(int f = 0; f < _synth.pcm.length; f++)
	{
		for (int e = 0; e < _synth.pcm.channels; e++) { // channels (stereo/mono)

			mad_fixed_t mad_sample = _synth.pcm.samples[e][f];

			// round
			mad_sample += (1L << (MAD_F_FRACBITS - 16));

			// clip
			if (mad_sample >= MAD_F_ONE) mad_sample = MAD_F_ONE - 1;
			else if (mad_sample < -MAD_F_ONE) mad_sample = -MAD_F_ONE;

			// quantize
			sample = mad_sample >> (MAD_F_FRACBITS + 1 - 16);

			if ( sample != static_cast<int16_t>(sample) ) sample = sample < 0 ? -32768 : 32767;

			*dst++ = sample;
		}
	}
	
	// If we need to convert samplerate or/and from mono to stereo...
	if (outsize > 0 && ( _synth.pcm.samplerate != 44100 || _synth.pcm.channels != 2)) {

		int16_t* adjusted_data = 0;
		int	adjusted_size = 0;
		int sample_count = outsize / 2; // samples are of size 2

		// Convert to needed samplerate - this converter only support standard flash samplerates
		convert_raw_data(&adjusted_data, &adjusted_size, tmp_raw_buffer, sample_count, 0, 
				_synth.pcm.samplerate, (_synth.pcm.channels == 2 ? true : false),
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
	return tmp_raw_buffer;
}

} // gnash namespace

