// AudioDecoderFfmpeg.cpp: Audio decoding using the FFMPEG library.
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

// $Id: AudioDecoderFfmpeg.cpp,v 1.9 2007/12/03 20:48:51 bwy Exp $

#include "AudioDecoderFfmpeg.h"

namespace gnash {
namespace media {
	
AudioDecoderFfmpeg::AudioDecoderFfmpeg ()
	:
	_audioCodec(NULL),
	_audioCodecCtx(NULL),
	_parser(NULL)
{}

AudioDecoderFfmpeg::~AudioDecoderFfmpeg()
{
	if (_audioCodecCtx)
	{
		avcodec_close(_audioCodecCtx);
		av_free(_audioCodecCtx);
	}
	if (_parser) av_parser_close(_parser);
}

bool AudioDecoderFfmpeg::setup(SoundInfo* info)
{
	// Init the avdecoder-decoder
	avcodec_init();
	avcodec_register_all();// change this to only register need codec?

	enum CodecID codec_id;

	switch(info->getFormat()) {
		case AUDIO_CODEC_RAW:
			codec_id = CODEC_ID_PCM_U16LE;
			break;
		case AUDIO_CODEC_ADPCM:
			codec_id = CODEC_ID_ADPCM_SWF;
			break;
		case AUDIO_CODEC_MP3:
			codec_id = CODEC_ID_MP3;
			// Init the parser
			_parser = av_parser_init(codec_id);

			if (!_parser) {	
				log_error(_("libavcodec can't parse the current audio format"));
				return false;
			}
			break;
		default:
			log_error(_("Unsupported audio codec %d"), static_cast<int>(info->getFormat()));
			return false;
	}
	_audioCodec = avcodec_find_decoder(codec_id);

	if (!_audioCodec) {
		log_error(_("libavcodec can't decode the current audio format"));
		return false;
	}

	_audioCodecCtx = avcodec_alloc_context();
	if (!_audioCodecCtx) {
		log_error(_("libavcodec couldn't allocate context"));
		return false;
	}

	int ret = avcodec_open(_audioCodecCtx, _audioCodec);
	if (ret < 0) {
		avcodec_close(_audioCodecCtx);
		log_error(_("libavcodec failed to initialize codec"));
		return false;
	}

	if (_audioCodecCtx->codec->id != CODEC_ID_MP3) {
		_audioCodecCtx->channels = (info->isStereo() ? 2 : 1);
		_audioCodecCtx->sample_rate = info->getSampleRate();
		_audioCodecCtx->sample_fmt = SAMPLE_FMT_S16;
	}

	return true;
}

bool AudioDecoderFfmpeg::setup(AudioInfo* info)
{
	// Init the avdecoder-decoder
	avcodec_init();
	avcodec_register_all();// change this to only register need codec?

	if (info->type == FLASH) {
		enum CodecID codec_id;

		switch(info->codec)
		{
			case AUDIO_CODEC_RAW:
				codec_id = CODEC_ID_PCM_U16LE;
				break;
			case AUDIO_CODEC_ADPCM:
				codec_id = CODEC_ID_ADPCM_SWF;
				break;
			case AUDIO_CODEC_MP3:
				codec_id = CODEC_ID_MP3;
				break;
			default:
				log_error(_("Unsupported audio codec %d"), static_cast<int>(info->codec));
				return false;
		}
		_audioCodec = avcodec_find_decoder(codec_id);
		// Init the parser
		_parser = av_parser_init(codec_id);
	}
	else if (info->type == FFMPEG)
	{
		_audioCodec = avcodec_find_decoder(static_cast<CodecID>(info->codec));
		// Init the parser
		_parser = av_parser_init(static_cast<CodecID>(info->codec));
	}
	else
	{
		return false;
	}

	if (!_audioCodec)
	{
		log_error(_("libavcodec can't decode the current audio format"));
		return false;
	}

	// Reuse the audioCodecCtx from the ffmpeg parser if exists/possible
	if (info->audioCodecCtx)
	{
		log_debug("re-using the parser's audioCodecCtx");
		_audioCodecCtx = info->audioCodecCtx;
	} 
	else
	{
		_audioCodecCtx = avcodec_alloc_context();
	}

	if (!_audioCodecCtx) {
		log_error(_("libavcodec couldn't allocate context"));
		return false;
	}

	int ret = avcodec_open(_audioCodecCtx, _audioCodec);
	if (ret < 0) {
		avcodec_close(_audioCodecCtx);
		log_error(_("libavcodec failed to initialize codec"));
		return false;
	}

	if (_audioCodecCtx->codec->id != CODEC_ID_MP3) {
		_audioCodecCtx->channels = (info->stereo ? 2 : 1);
		_audioCodecCtx->sample_rate = info->sampleRate;
		//_audioCodecCtx->sample_fmt = SAMPLE_FMT_S16;
	}

	return true;
}

uint8_t* AudioDecoderFfmpeg::decode(uint8_t* input, uint32_t inputSize, uint32_t& outputSize, uint32_t& decodedBytes, bool parse)
{

	long bytes_decoded = 0;
	int bufsize = (AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2;
	uint8_t* output = new uint8_t[bufsize];
	uint32_t orgbufsize = bufsize;
	decodedBytes = 0;

	if (parse) {
	
		if (!_parser)
		{	
			log_error(_("libavcodec can't parse the current audio format"));
			return NULL;
		}
	
	
		bufsize = 0;
		while (bufsize == 0 && decodedBytes < inputSize) {
			uint8_t* frame;
			int framesize;

			bytes_decoded = av_parser_parse(_parser, _audioCodecCtx, &frame, &framesize, input+decodedBytes, inputSize-decodedBytes, 0, 0); //the last 2 is pts & dts

			int tmp = 0;
#ifdef FFMPEG_AUDIO2
			bufsize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
			tmp = avcodec_decode_audio2(_audioCodecCtx, reinterpret_cast<int16_t*>(output), &bufsize, frame, framesize);
#else
			tmp = avcodec_decode_audio(_audioCodecCtx, reinterpret_cast<int16_t*>(output), &bufsize, frame, framesize);
#endif

			if (bytes_decoded < 0 || tmp < 0 || bufsize < 0) {
				log_error(_("Error while decoding audio data. Upgrading ffmpeg/libavcodec might fix this issue."));
				// Setting data position to data size will get the sound removed
				// from the active sound list later on.
				decodedBytes = inputSize;
				break;
			}

			decodedBytes += bytes_decoded;
		}

	} else {

		int tmp = 0;

#ifdef FFMPEG_AUDIO2
		tmp = avcodec_decode_audio2(_audioCodecCtx, reinterpret_cast<int16_t*>(output), &bufsize, input, inputSize);
#else
		tmp = avcodec_decode_audio(_audioCodecCtx, reinterpret_cast<int16_t*>(output), &bufsize, input, inputSize);
#endif



		if (bytes_decoded < 0 || tmp < 0 || bufsize < 0) {
			log_error(_("Error while decoding audio data. Upgrading ffmpeg/libavcodec might fix this issue."));
			// Setting data position to data size will get the sound removed
			// from the active sound list later on.
			decodedBytes = 0;
			outputSize = 0;
			delete [] output;
			return NULL;
		}

		decodedBytes = inputSize;
	}

	// Error handling
	if (bufsize < 1) {
		log_error(_("Error while decoding audio data."));
		delete [] output;
		decodedBytes = 0;
		outputSize = 0;
		return NULL;
	}

	// Resampling is needed.
	if (_resampler.init(_audioCodecCtx)) {
		bool stereo = _audioCodecCtx->channels > 1 ? true : false;
		int samples = stereo ? bufsize >> 2 : bufsize >> 1;

		uint8_t* tmp = new uint8_t[orgbufsize];
			
		samples = _resampler.resample(reinterpret_cast<int16_t*>(output),
						 reinterpret_cast<int16_t*>(tmp),
						 samples);
		outputSize = samples *2 *2; // the resampled audio has samplesize 2, and is stereo
		uint8_t* ret = new uint8_t[outputSize];
		memcpy(ret, tmp, outputSize);
		delete [] tmp;
		delete [] output;
		return ret;
	} else {
		outputSize = bufsize;
		uint8_t* ret = new uint8_t[outputSize];
		memcpy(ret, output, outputSize);
		delete [] output;
		return ret;		
	}
}


} // gnash.media namespace 
} // gnash namespace
