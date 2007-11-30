// MediaDecoder.cpp: Media decoding generic code
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

// $Id: MediaDecoder.cpp,v 1.1 2007/11/30 00:24:00 tgc Exp $

#include "MediaDecoder.h"

#include "log.h"

namespace gnash {
namespace media {

std::vector<StatusCode> MediaDecoder::getOnStatusEvents()
{
	boost::mutex::scoped_lock lock(_onStatusMutex);

	const std::vector<StatusCode> statusQueue(_onStatusQueue);
	_onStatusQueue.clear();
	return statusQueue;
}

	bool decodingFailed = false;

void MediaDecoder::decodingLoop()
{
	// The decode loop
	while (_running) {

		// If the buffer is not full, put something into it!
		if (!_buffer->isFull()) {
			while (!_buffer->isFull()) {
				if (!decodeAndBufferFrame()) {
					decodingFailed = true;
					break;
				}
				//log_debug("decoded a frame");
			}
		
		// "Warm up" the data.
		} else if (_streamSize > _lastConfirmedPosition) {
			if (_stream->set_position(_lastConfirmedPosition+2048) != 0) {
				// We assume we're done now
				// TODO: check for errors
				_lastConfirmedPosition = _streamSize;
			} else {
				_lastConfirmedPosition += 2048;
			}
			//log_debug("warming up the file");

		}
		if (_buffer->isFull()) {
			pushOnStatus(bufferFull);
			
			// If download is complete there is nothing to do, so we take a break.
			if (_streamSize <= _lastConfirmedPosition) {
				relax();
				continue;
			}
		}

		// If decoding failed, there's a good chance playback has ended, so
		// we take a breake until someone tells us to wake up.
		if (decodingFailed) {
			relax();
		}
	}
	log_debug("Left the decoding loop");
}

bool MediaDecoder::decodeAndBufferFrame()
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
			pushOnStatus(playStop);
		} else {
			pushOnStatus(streamNotFound);
			log_error("FLV parsing problems! stopping buffering.");
			_running = false;
		}
		return false;
	}

	if (frame->tag == 9) {
		decodeVideo(frame);
	} else {
		decodeAudio(frame);
	}
	return true;

}


void MediaDecoder::decodeAudio(MediaFrame* packet)
{
	// We don't handle audio
	if (!_audioDecoder.get()) return;

	uint32_t datasize;
	uint32_t bufsize;

	uint8_t* ptr = _audioDecoder->decode(packet->data, packet->dataSize, bufsize, datasize, false);

	if (bufsize > 0 && ptr != NULL)
	{
	  	raw_audiodata_t* raw = new raw_audiodata_t();
		
		raw->m_data = ptr;
		raw->m_ptr = raw->m_data;
		raw->m_size = bufsize;
		raw->m_pts = packet->timestamp;
		_buffer->pushAudio(raw);
		return;
	}
	log_debug(_("Problems decoding audio frame."));

}

void MediaDecoder::decodeVideo(MediaFrame* packet)
{
	// We don't handle video decoding today
	if (!_videoDecoder.get()) return;

	std::auto_ptr<image::image_base> img = _videoDecoder->decodeToImage(packet->data, packet->dataSize);

	if (img.get() != NULL)
	{
	  	raw_videodata_t* raw = new raw_videodata_t();

		raw->image = img;
		raw->timestamp = packet->timestamp;
		_buffer->pushVideo(raw);
		return;
	}
	log_debug(_("Problems decoding video frame."));

}

} // media namespace
} // gnash namespace

