// MediaDecoderSdl.cpp: Media decoding using libs, used with sdl soundhandler.
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

// $Id: MediaDecoderSdl.cpp,v 1.6 2007/12/03 16:50:25 bwy Exp $

#include "MediaDecoderSdl.h"
#include "AudioDecoderNellymoser.h"
#include "AudioDecoderSimple.h"

#ifdef USE_FFMPEG
#include "AudioDecoderFfmpeg.h"
#include "VideoDecoderFfmpeg.h"
#include "MediaParserFfmpeg.h"
#endif

#ifdef USE_MAD
#include "AudioDecoderMad.h"
#endif

#include "log.h"

#include "gnash.h"

namespace gnash {
namespace media {

MediaDecoderSdl::MediaDecoderSdl(boost::shared_ptr<tu_file> stream, MediaBuffer* buffer, uint16_t swfVersion, int format)
	:
	MediaDecoder(stream, buffer, swfVersion, format)
{
	// Start the decoding thread which will also setup the decoder and parser
	_decodeThread = new boost::thread(boost::bind(MediaDecoderSdl::decodeThread, this)); 
}

MediaDecoderSdl::~MediaDecoderSdl()
{
	_running = false;

	if (_decodeThread) {
		wakeUp();
		_decodeThread->join();
		delete _decodeThread;
		_decodeThread = NULL;
	}
}

bool MediaDecoderSdl::setupDecoding()
{
	std::auto_ptr<VideoInfo> vInfo = _parser->getVideoInfo();
	if (vInfo.get() != NULL) {
#ifdef USE_FFMPEG
		_videoDecoder.reset(new VideoDecoderFfmpeg());
#endif
		if (_videoDecoder.get() != NULL) {
			if (!_videoDecoder->setup(vInfo.get())) {
				_videoDecoder.reset(NULL); // Delete the videoDecoder if it is of no use
				log_error("No video decoder could be created, "
					"since no decoder for this format "
					"is available.");
			}
			else
			{
				// Video decoder setup succeeded
				_video = true;
			}
		}
		else
		{
			log_error("No video decoder could be created, since no decoder is enabled.");
		}
	}

	std::auto_ptr<AudioInfo> aInfo = _parser->getAudioInfo();
	if (get_sound_handler() && aInfo.get() != NULL) {
#ifdef USE_MAD
		if (_parser->isAudioMp3()) {
			_audioDecoder.reset(new AudioDecoderMad());
		}
#endif
		if (_parser->isAudioNellymoser()) {
			_audioDecoder.reset(new AudioDecoderNellymoser());
		}

#ifdef USE_FFMPEG
		if (_audioDecoder.get() == NULL) _audioDecoder.reset(new AudioDecoderFfmpeg());
#endif
		if (_audioDecoder.get() != NULL) {
			if (!_audioDecoder->setup(aInfo.get()))
			{
				_audioDecoder.reset(NULL); // Delete the audioDecoder if it is of no use
				log_error("No audio decoder could be created, "
					"since no decoder for this"
					" format is available.");
			}
			else
			{
				// Audio decoder setup succeeded
				_audio = true;
			}
		}
		else
		{
			log_error("No audio decoder could be created, since no decoder is enabled.");
		}
	}

	// We don't need both audio and video to be happy :)
	return (_audio || _video);
}

bool MediaDecoderSdl::setupParser()
{
	// Buffer a bit to make sure the stream is accessible
	if (_stream->set_position(512) != 0) {
		_error = streamError;
		pushOnStatus(streamNotFound);
		return false;
	}

	_lastConfirmedPosition = 512;
	_streamSize = _stream->get_size();

	// Check if the file is a FLV, in which case we use our own parser
	char head[4] = {0, 0, 0, 0};
	_stream->set_position(0);
	_stream->read_bytes(head, 3);
	_stream->set_position(0);

	// Setup the decoding and parser
	if (std::string(head) == "FLV") {
		_parser.reset(new FLVParser(_stream));
#ifdef USE_FFMPEG
	} else {
		_parser.reset(new MediaParserFfmpeg(_stream));
#endif
	}
	
	return _parser->setupParser();
}

uint32_t MediaDecoderSdl::seek(uint32_t pos)
{
	uint32_t ret = 0;
	if (_parser.get()) ret = _parser->seek(pos);
	else ret = 0;

	// Flush the buffer
	_buffer->flush();

	return ret;
}

void MediaDecoderSdl::decodeThread(MediaDecoderSdl* decoder)
{
printf("\t in the decode thread\n");

	// If the destructor has been called at this point, exit the thread
	if (!decoder->_running) return;

	// Setup the decoder and parser
	if (decoder->setupParser()) {
		if (!decoder->setupDecoding()) {
			decoder->pushOnStatus(streamNotFound);
			log_error("Setup of media decoder failed");
			return;
		}
	} else {
		decoder->pushOnStatus(streamNotFound);
		log_error("Setup of media parser failed");
		return;
	}

	// Everything is setup, so let's play!

	decoder->pushOnStatus(playStart);

	decoder->decodingLoop();
}

std::pair<uint32_t, uint32_t>
MediaDecoderSdl::getWidthAndHeight()
{
	if (_parser.get()) {
		std::auto_ptr<VideoInfo> vInfo = _parser->getVideoInfo();
		if (vInfo.get()) return std::pair<uint32_t, uint32_t>(vInfo->width, vInfo->height);
	}
	return std::pair<uint32_t, uint32_t>(0,0);
}
	

} // gnash.media namespace 
} // namespace gnash

