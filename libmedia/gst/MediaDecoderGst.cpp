// MediaDecoderGst.cpp: Media decoding using Gstreamer
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

// $Id: MediaDecoderGst.cpp,v 1.1 2007/11/30 00:13:01 tgc Exp $

#include "MediaDecoderGst.h"
#include "AudioDecoderNellymoser.h"
#include "AudioDecoderSimple.h"

#include "AudioDecoderGst.h"
#include "VideoDecoderGst.h"

#include "log.h"

#include "gnash.h"

namespace gnash {
namespace media {

MediaDecoderGst::MediaDecoderGst(boost::shared_ptr<tu_file> stream, MediaBuffer* buffer, uint16_t swfVersion, int format)
	:
	MediaDecoder(stream, buffer, swfVersion, format)
{
	// Start the decoding thread which will also setup the decoder and parser
	_decodeThread = new boost::thread(boost::bind(MediaDecoderGst::decodeThread, this)); 
}

MediaDecoderGst::~MediaDecoderGst()
{
	_running = false;

	if (_decodeThread) {
		wakeUp();
		_decodeThread->join();
		delete _decodeThread;
		_decodeThread = NULL;
	}
}

bool MediaDecoderGst::setupParser()
{
	// Buffer a bit to make sure the stream is accessable
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
		return _parser->setupParser();
	} else {
		return false;
	}
}

bool MediaDecoderGst::setupDecoding()
{
	std::auto_ptr<VideoInfo> vInfo = _parser->getVideoInfo();
	if (vInfo.get() != NULL) {

		_videoDecoder.reset(new VideoDecoderGst());

		if (_videoDecoder.get() != NULL) {
			if (!_videoDecoder->setup(vInfo.get())) {
				_videoDecoder.reset(NULL); // Delete the videoDecoder if it is of no use
				log_error("No video decoder could be created, since no decoder for this format is available.");
			}
			_video = true;
		} else {
			log_error("No video decoder could be created, since no decoder is enabled.");
		}
	}

	std::auto_ptr<AudioInfo> aInfo = _parser->getAudioInfo();
	if (get_sound_handler() && aInfo.get() != NULL) {

		if (_parser->isAudioNellymoser()) {
			_audioDecoder.reset(new AudioDecoderNellymoser());
		}

		if (_audioDecoder.get() == NULL) _audioDecoder.reset(new AudioDecoderGst());

		if (_audioDecoder.get() != NULL) {
			if (!_audioDecoder->setup(aInfo.get())) {
				_audioDecoder.reset(NULL); // Delete the audioDecoder if it is of no use
				log_error("No audio decoder could be created, since no decoder for this format is available.");
			}
			_audio = true;
		} else {
			log_error("No audio decoder could be created, since no decoder is enabled.");
		}
	}

	// We don't need both audio and video to be happy :)
	return (_audio || _video);
}


uint32_t MediaDecoderGst::seek(uint32_t pos)
{
	uint32_t ret = 0;
	if (_parser.get()) ret = _parser->seek(pos);
	else ret = 0;

	// Flush the buffer
	_buffer->flush();

	return ret;
}

void MediaDecoderGst::decodeThread(MediaDecoderGst* decoder)
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

	// If the parser setup failed, it is perhaps because it is not a FLV file,
	// so we set up an gstreamer pipeline instead.
	} else {
/*		if (!decoder->setupGstPipeline()) {
			decoder->pushOnStatus(streamNotFound);*/
			log_error("Setup of media parser failed");
			return;
//		}
	}

	// Everything is setup, so let's play!

	decoder->pushOnStatus(playStart);

	decoder->decodingLoop();
}

std::pair<uint32_t, uint32_t>
MediaDecoderGst::getWidthAndHeight()
{
	if (_parser.get()) {
		std::auto_ptr<VideoInfo> vInfo = _parser->getVideoInfo();
		if (vInfo.get()) return std::pair<uint32_t, uint32_t>(vInfo->width, vInfo->height);
	}
	return std::pair<uint32_t, uint32_t>(0,0);
}
	

} // namespace media

} // namespace gnash
