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

//  $Id:

#include "MediaDecoderSdl.h"
#include "AudioDecoderNellymoser.h"

#ifdef USE_FFMPEG
#include "AudioDecoderFfmpeg.h"
#include "VideoDecoderFfmpeg.h"
#include "MediaParserFfmpeg.h"
#endif

#ifdef USE_MAD
#include "AudioDecoderMad.h"
#endif

#include "log.h"

#include <boost/scoped_array.hpp>

#if defined(_WIN32) || defined(WIN32)
# include <windows.h>	// for sleep()
# define usleep(x) Sleep(x/1000)
#else
# include "unistd.h" // for usleep()
#endif

namespace gnash {

MediaDecoderSdl::MediaDecoderSdl(tu_file* stream, MediaBuffer* buffer, uint16_t swfVersion, int format)
	:
	MediaDecoder(stream, buffer, swfVersion, format),

	_decodeThread(NULL),

	_running(true),
	_audioDecoder(NULL),
	_videoDecoder(NULL)
{
	// Buffer a bit to make sure the stream is accessable
	if (_stream->set_position(512) != 0) {
		_error = streamError;
		pushOnStatus(streamNotFound);
		return;
	}
	_lastConfirmedPosition = 512;
	_streamSize = _stream->get_size();

	// Check if the file is a FLV, in which case we use our own parser
	char head[4] = {0, 0, 0, 0};
	_stream->set_position(0);
	_stream->read_bytes(head, 3);
	_stream->set_position(0);

	// Setup the decoding and parser
	bool ret = false;
	if (std::string(head) == "FLV") {
		_parser.reset(new FLVParser(_stream));
#ifdef USE_FFMPEG
	} else {
		_parser.reset(new MediaParserFfmpeg(_stream));
#endif
	}

	ret = _parser->setupParser();

	if (ret) {
		ret = setupDecoding();
	} else {
		log_error("Setup of media parser failed");
		return;
	}

	// If the setup failed, there is no reason to start the decoding thread
	if (ret) {
		// Start the decoding thread which will also setup the decoder and parser
		_decodeThread = new boost::thread(boost::bind(MediaDecoderSdl::decodeThread, this)); 
	} else {
		log_error("Setup of media decoder failed");
	}
}

MediaDecoderSdl::~MediaDecoderSdl()
{
	_running = false;
printf("waiting for thread to stop\n");
	_decodeThread->join();
printf("thread stopped\n");

	delete _decodeThread;
	printf("MediaDecoderSdl deleted!");
}

void MediaDecoderSdl::pause()
{
}

void MediaDecoderSdl::decode()
{
}

bool MediaDecoderSdl::setupDecoding()
{
	bool video = false;
	bool audio = false;

	std::auto_ptr<VideoInfo> vInfo = _parser->getVideoInfo();
	if (vInfo.get() != NULL) {
#ifdef USE_FFMPEG
		_videoDecoder.reset(new VideoDecoderFfmpeg());
#endif
		if (_videoDecoder.get() != NULL) {
			if (!_videoDecoder->setup(vInfo.get())) {
				_videoDecoder.reset(NULL); // Delete the videoDecoder if it is of no use
				log_error("No video decoder could be created, since no decoder for this format is available.");
			}
			video = true;
		} else {
			log_error("No video decoder could be created, since no decoder is enabled.");
		}
	}

	std::auto_ptr<AudioInfo> aInfo = _parser->getAudioInfo();
	if (aInfo.get() != NULL) {
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
			if (!_audioDecoder->setup(aInfo.get())) {
				_audioDecoder.reset(NULL); // Delete the audioDecoder if it is of no use
				log_error("No audio decoder could be created, since no decoder for this format is available.");
			}
			audio = true;
		} else {
			log_error("No audio decoder could be created, since no decoder is enabled.");
		}
	}

	// We don't need both audio and video to be happy :)
	return (audio || video);
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
	// The decode loop
	while (decoder->_running) {

		// If the buffer is not full, put something into it!
		if (!decoder->_buffer->isFull()) {
			decoder->decodeAndBufferFrame();
			//log_debug("decoded a frame");

		// "Warm up" the data.
		} else if (decoder->_streamSize > decoder->_lastConfirmedPosition) {
			if (decoder->_stream->set_position(decoder->_lastConfirmedPosition+10000) != 0) {
				// We assume we're done now
				// TODO: check for errors
				decoder->_lastConfirmedPosition = decoder->_streamSize;
			} else {
				decoder->_lastConfirmedPosition += 10000;
			}
			//log_debug("warming up the file");

		}
		usleep(1); // task switch, to avoid 100% CPU
	}
	log_debug("Left the decoding loop");
}


void MediaDecoderSdl::decodeAndBufferFrame()
{

	MediaFrame* frame = _parser->parseMediaFrame();
	uint32_t parserPosition = _parser->getLastParsedPos();
	if (parserPosition > _lastConfirmedPosition) _lastConfirmedPosition = parserPosition;

	if (frame == NULL) {
		if (_lastConfirmedPosition+1 >= _streamSize)
		{
#ifdef GNASH_DEBUG_THREADS
			log_debug("decodeFLVFrame: load completed, stopping");
#endif
			// Stop!
			//m_go = false;
		} else {
			log_error("FLV parsing problems! stopping buffering.");
			_running = false;
		}
//log_error("FLV parsing problems!");
		return;
	}

	if (frame->tag == 9) {
		decodeVideo(frame);
	} else {
		decodeAudio(frame);
	}

}

void MediaDecoderSdl::decodeAudio(MediaFrame* packet)
{
	// We don't handle audio
	if (!_audioDecoder.get()) return;

	uint32_t datasize;
	uint32_t bufsize;

	uint8_t* ptr = _audioDecoder->decode(packet->data, packet->dataSize, bufsize, datasize, false);

	if (bufsize > 0 && ptr != NULL)
	{
	  	raw_mediadata_t* raw = new raw_mediadata_t();
		
		raw->m_data = ptr;
		raw->m_ptr = raw->m_data;
		raw->m_size = bufsize;
		raw->m_pts = packet->timestamp;
		_buffer->pushAudio(raw);
		return;
	}
	log_debug(_("Problems decoding audio frame."));
	/*_running = false;
	_error = decodingError;
	pushOnStatus(playStop);*/
}

void MediaDecoderSdl::decodeVideo(MediaFrame* packet)
{
	// We don't handle video decoding today
	if (!_videoDecoder.get()) return;

	uint32_t bufsize;

	uint8_t* ptr = _videoDecoder->decode(packet->data, packet->dataSize, bufsize);

	if (bufsize > 0 && ptr != NULL)
	{
	  	raw_mediadata_t* raw = new raw_mediadata_t();

		raw->m_data = ptr;
		raw->m_ptr = raw->m_data;
		raw->m_size = bufsize;
		raw->m_pts = packet->timestamp;
		_buffer->pushVideo(raw);
		return;
	}
	log_debug(_("Problems decoding video frame."));
/*	_running = false;
	_error = decodingError;
	pushOnStatus(playStop);*/
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
	

} // namespace gnash
