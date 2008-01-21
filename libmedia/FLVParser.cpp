// FLVParser.cpp:  Flash Video file parser, for Gnash.
//
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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

// $Id: FLVParser.cpp,v 1.6 2008/01/21 23:10:13 rsavoye Exp $

#include "FLVParser.h"
#include "amf.h"
#include "log.h"
#include "BitsReader.h"

#define PADDING_BYTES 8

// Define the following macro the have seek() operations printed
//#define GNASH_DEBUG_SEEK 1

namespace gnash {
namespace media {

FLVParser::FLVParser(boost::shared_ptr<tu_file> stream)
	:
	MediaParser(stream),
	_lastParsedPosition(0),
	_parsingComplete(false),
	_videoInfo(NULL),
	_audioInfo(NULL),
	_nextAudioFrame(0),
	_nextVideoFrame(0),
	_audio(false),
	_video(false)
{
}

FLVParser::~FLVParser()
{
	_videoFrames.clear();

	_audioFrames.clear();
}


boost::uint32_t FLVParser::getBufferLength()
{
	boost::mutex::scoped_lock lock(_mutex);

	if (_video) {
		size_t size = _videoFrames.size();
		if (size > 1 && size > _nextVideoFrame) {
			return _videoFrames.back()->timestamp - _videoFrames[_nextVideoFrame]->timestamp;
		}
	}
	if (_audio) {
		size_t size = _audioFrames.size();
		if (size > 1 && size > _nextAudioFrame) {
			return _audioFrames.back()->timestamp - _audioFrames[_nextAudioFrame]->timestamp;
		}
	}
	return 0;
}
boost::uint16_t FLVParser::videoFrameRate()
{
	boost::mutex::scoped_lock lock(_mutex);

	// Make sure that there are parsed some frames
	while(_videoFrames.size() < 2 && !_parsingComplete) {
		parseNextFrame();
	}

	if (_videoFrames.size() < 2) return 0;

 	boost::uint32_t framedelay = _videoFrames[1]->timestamp - _videoFrames[0]->timestamp;

	return static_cast<boost::int16_t>(1000 / framedelay);
}


boost::uint32_t FLVParser::videoFrameDelay()
{
	boost::mutex::scoped_lock lock(_mutex);

	// If there are no video in this FLV return 0
	if (!_video && _lastParsedPosition > 0) return 0;

	// Make sure that there are parsed some frames
	while(_videoFrames.size() < 2 && !_parsingComplete) {
		parseNextFrame();
	}

	// If there is no video data return 0
	if (_videoFrames.size() == 0 || !_video || _nextVideoFrame < 2) return 0;

	return _videoFrames[_nextVideoFrame-1]->timestamp - _videoFrames[_nextVideoFrame-2]->timestamp;
}

boost::uint32_t FLVParser::audioFrameDelay()
{
	boost::mutex::scoped_lock lock(_mutex);

	// If there are no audio in this FLV return 0
	if (!_audio && _lastParsedPosition > 0) return 0;

	// Make sure that there are parsed some frames
	while(_audioFrames.size() < 2 && !_parsingComplete) {
		parseNextFrame();
	}

	// If there is no video data return 0
	if (_audioFrames.size() == 0 || !_audio || _nextAudioFrame < 2) return 0;

	return _audioFrames[_nextAudioFrame-1]->timestamp - _audioFrames[_nextAudioFrame-2]->timestamp;
}

MediaFrame* FLVParser::parseMediaFrame()
{
	boost::mutex::scoped_lock lock(_mutex);

	boost::uint32_t video_size = _videoFrames.size();
	boost::uint32_t audio_size = _audioFrames.size();

	if (_audio && audio_size <= _nextAudioFrame)
	{
		// Parse a media frame if any left or if needed
		while(_audioFrames.size() <= _nextAudioFrame && !_parsingComplete) {
			if (!parseNextFrame()) break;
		}
	}

	if (_video && video_size <= _nextVideoFrame)
	{
		// Parse a media frame if any left or if needed
		while(_videoFrames.size() <= _nextVideoFrame && !_parsingComplete) {
			if (!parseNextFrame()) break;
		}
	}

	// Find the next frame in the file
	bool audioReady = _audioFrames.size() > _nextAudioFrame;
	bool videoReady = _videoFrames.size() > _nextVideoFrame;
	bool useAudio = false;

	if (audioReady && videoReady) {
		useAudio = _audioFrames[_nextAudioFrame]->dataPosition < _videoFrames[_nextVideoFrame]->dataPosition;
	} else if (!audioReady && videoReady) {
		useAudio = false;
	} else if (audioReady && !videoReady) {
		useAudio = true;
	} else {
		// If no frames are next we have reached EOF
		return NULL;
	}

	// Find the next frame in the file a return it

	if (useAudio) {

		MediaFrame* frame = new MediaFrame;
		frame->dataSize = _audioFrames[_nextAudioFrame]->dataSize;
		frame->timestamp = _audioFrames[_nextAudioFrame]->timestamp;

		_stream->set_position(_audioFrames[_nextAudioFrame]->dataPosition);
		frame->data = new boost::uint8_t[frame->dataSize + PADDING_BYTES];
		size_t bytesread = _stream->read_bytes(frame->data, frame->dataSize);
		memset(frame->data + bytesread, 0, PADDING_BYTES);

		frame->tag = 8;
		_nextAudioFrame++;

		return frame;

	} else {
		MediaFrame* frame = new MediaFrame;
		frame->dataSize = _videoFrames[_nextVideoFrame]->dataSize;
		frame->timestamp = _videoFrames[_nextVideoFrame]->timestamp;

		_stream->set_position(_videoFrames[_nextVideoFrame]->dataPosition);
		frame->data = new boost::uint8_t[frame->dataSize + PADDING_BYTES];
		size_t bytesread  = _stream->read_bytes(frame->data, frame->dataSize);
		memset(frame->data + bytesread, 0, PADDING_BYTES);

		frame->tag = 9;
		_nextVideoFrame++;

		return frame;
	}
}

MediaFrame* FLVParser::nextAudioFrame()
{
	boost::mutex::scoped_lock lock(_mutex);

	// If there are no audio in this FLV return NULL
	if (!_audio && _lastParsedPosition > 0) return NULL;

	// Make sure that there are parsed enough frames to return the need frame
	while(_audioFrames.size() <= _nextAudioFrame && !_parsingComplete) {
		if (!parseNextFrame()) break;
	}

	// If the needed frame can't be parsed (EOF reached) return NULL
	if (_audioFrames.size() <= _nextAudioFrame || _audioFrames.size() == 0) return NULL;

	MediaFrame* frame = new MediaFrame;
	frame->dataSize = _audioFrames[_nextAudioFrame]->dataSize;
	frame->timestamp = _audioFrames[_nextAudioFrame]->timestamp;
	frame->tag = 8;

	_stream->set_position(_audioFrames[_nextAudioFrame]->dataPosition);
	frame->data = new boost::uint8_t[_audioFrames[_nextAudioFrame]->dataSize +
				  PADDING_BYTES];
	size_t bytesread = _stream->read_bytes(frame->data, _audioFrames[_nextAudioFrame]->dataSize);
	memset(frame->data + bytesread, 0, PADDING_BYTES);

	_nextAudioFrame++;
	return frame;

}

MediaFrame* FLVParser::nextVideoFrame()
{
	boost::mutex::scoped_lock lock(_mutex);

	// If there are no video in this FLV return NULL
	if (!_video && _lastParsedPosition > 0)
	{
		//gnash::log_debug("no video, or lastParserPosition > 0");
		return NULL;
	}

	// Make sure that there are parsed enough frames to return the need frame
	while(_videoFrames.size() <= static_cast<boost::uint32_t>(_nextVideoFrame) && !_parsingComplete)
	{
		if (!parseNextFrame()) break;
	}

	// If the needed frame can't be parsed (EOF reached) return NULL
	if (_videoFrames.size() <= _nextVideoFrame || _videoFrames.size() == 0)
	{
		//gnash::log_debug("The needed frame (%d) can't be parsed (EOF reached)", _lastVideoFrame);
		return NULL;
	}


	MediaFrame* frame = new MediaFrame;
	frame->dataSize = _videoFrames[_nextVideoFrame]->dataSize;
	frame->timestamp = _videoFrames[_nextVideoFrame]->timestamp;
	frame->tag = 9;

	_stream->set_position(_videoFrames[_nextVideoFrame]->dataPosition);
	frame->data = new boost::uint8_t[_videoFrames[_nextVideoFrame]->dataSize + 
				  PADDING_BYTES];
	size_t bytesread = _stream->read_bytes(frame->data, _videoFrames[_nextVideoFrame]->dataSize);
	memset(frame->data + bytesread, 0, PADDING_BYTES);

	_nextVideoFrame++;
	return frame;
}


boost::uint32_t FLVParser::seekAudio(boost::uint32_t time)
{

	// Make sure that there are parsed some frames
	while(_audioFrames.size() < 1 && !_parsingComplete) {
		parseNextFrame();
	}

	// If there is no audio data return NULL
	if (_audioFrames.size() == 0) return 0;

	// Make sure that there are parsed some enough frames
	// to get the right frame.
	while(_audioFrames.back()->timestamp < time && !_parsingComplete) {
		parseNextFrame();
	}

	// If there are no audio greater than the given time
	// the last audioframe is returned
	FLVAudioFrame* lastFrame = _audioFrames.back();
	if (lastFrame->timestamp < time) {
		_nextAudioFrame = _audioFrames.size() - 1;
		return lastFrame->timestamp;
	}

	// We try to guess where in the vector the audioframe
	// with the correct timestamp is
	size_t numFrames = _audioFrames.size();
	double tpf = lastFrame->timestamp / numFrames; // time per frame
	size_t guess = size_t(time / tpf);

	// Here we test if the guess was ok, and adjust if needed.
	size_t bestFrame = iclamp(guess, 0, _audioFrames.size()-1);

	// Here we test if the guess was ok, and adjust if needed.
	long diff = _audioFrames[bestFrame]->timestamp - time;
	if ( diff > 0 ) // our guess was too long
	{
		while ( bestFrame > 0 && _audioFrames[bestFrame-1]->timestamp > time ) --bestFrame;
	}
	else // our guess was too short
	{
		while ( bestFrame < _audioFrames.size()-1 && _audioFrames[bestFrame+1]->timestamp < time ) ++bestFrame;
	}

#ifdef GNASH_DEBUG_SEEK
	gnash::log_debug("Seek (audio): " SIZET_FMT "/" SIZET_FMT " (%u/%u)", bestFrame, numFrames, _audioFrames[bestFrame]->timestamp, time);
#endif
	_nextAudioFrame = bestFrame;
	return _audioFrames[bestFrame]->timestamp;

}


boost::uint32_t FLVParser::seekVideo(boost::uint32_t time)
{
	// Make sure that there are parsed some frames
	while(_videoFrames.size() < 1 && !_parsingComplete) {
		parseNextFrame();
	}

	// If there is no video data return NULL
	if (_videoFrames.size() == 0) return 0;

	// Make sure that there are parsed some enough frames
	// to get the right frame.
	while(_videoFrames.back()->timestamp < time && !_parsingComplete) {
		parseNextFrame();
	}

	// If there are no videoframe greater than the given time
	// the last key videoframe is returned
	FLVVideoFrame* lastFrame = _videoFrames.back();
	size_t numFrames = _videoFrames.size();
	if (lastFrame->timestamp < time)
	{
		size_t lastFrameNum = numFrames -1;
		while (! lastFrame->isKeyFrame() )
		{
			lastFrameNum--;
			lastFrame = _videoFrames[lastFrameNum];
		}

		_nextVideoFrame = lastFrameNum;
		return lastFrame->timestamp;

	}

	// We try to guess where in the vector the videoframe
	// with the correct timestamp is
	double tpf = lastFrame->timestamp / numFrames; // time per frame
	size_t guess = size_t(time / tpf);

	size_t bestFrame = iclamp(guess, 0, _videoFrames.size()-1);

	// Here we test if the guess was ok, and adjust if needed.
	long diff = _videoFrames[bestFrame]->timestamp - time;
	if ( diff > 0 ) // our guess was too long
	{
		while ( bestFrame > 0 && _videoFrames[bestFrame-1]->timestamp > time ) --bestFrame;
	}
	else // our guess was too short
	{
		while ( bestFrame < _videoFrames.size()-1 && _videoFrames[bestFrame+1]->timestamp < time ) ++bestFrame;
	}

#if 0
	boost::uint32_t diff = abs(_videoFrames[bestFrame]->timestamp - time);
	while (true)
	{
		if (bestFrame+1 < numFrames && static_cast<boost::uint32_t>(abs(_videoFrames[bestFrame+1]->timestamp - time)) < diff) {
			diff = abs(_videoFrames[bestFrame+1]->timestamp - time);
			bestFrame = bestFrame + 1;
		} else if (bestFrame > 0 && static_cast<boost::uint32_t>(abs(_videoFrames[bestFrame-1]->timestamp - time)) < diff) {
			diff = abs(_videoFrames[bestFrame-1]->timestamp - time);
			bestFrame = bestFrame - 1;
		} else {
			break;
		}
	}
#endif


	// Find closest backward keyframe  
	size_t rewindKeyframe = bestFrame;
	while ( rewindKeyframe && ! _videoFrames[rewindKeyframe]->isKeyFrame() )
	{
		rewindKeyframe--;
	}

	// Find closest forward keyframe 
	size_t forwardKeyframe = bestFrame;
	size_t size = _videoFrames.size();
	while (size > forwardKeyframe+1 && ! _videoFrames[forwardKeyframe]->isKeyFrame() )
	{
		forwardKeyframe++;
	}

	// We can't ensure we were able to find a key frame *after* the best position
	// in that case we just use any previous keyframe instead..
	if ( ! _videoFrames[forwardKeyframe]->isKeyFrame() )
	{
		bestFrame = rewindKeyframe;
	}
	else
	{
		boost::int32_t forwardDiff = _videoFrames[forwardKeyframe]->timestamp - time;
		boost::int32_t rewindDiff = time - _videoFrames[rewindKeyframe]->timestamp;

		if (forwardDiff < rewindDiff) bestFrame = forwardKeyframe;
		else bestFrame = rewindKeyframe;
	}

#ifdef GNASH_DEBUG_SEEK
	gnash::log_debug("Seek (video): " SIZET_FMT "/" SIZET_FMT " (%u/%u)", bestFrame, numFrames, _videoFrames[bestFrame]->timestamp, time);
#endif

	_nextVideoFrame = bestFrame;
	assert( _videoFrames[bestFrame]->isKeyFrame() );
	return _videoFrames[bestFrame]->timestamp;
}



std::auto_ptr<VideoInfo> FLVParser::getVideoInfo()
{
	boost::mutex::scoped_lock lock(_mutex);

	// If there are no video in this FLV return NULL
	if (!_video && _lastParsedPosition > 0) return std::auto_ptr<VideoInfo>(NULL);

	// Make sure that there are parsed some video frames
	while(_videoInfo.get() == NULL && !_parsingComplete && !(!_video && _lastParsedPosition > 0)) {
		if (parseNextFrame() == false) break;
	}

	// If there are no video data return NULL
	if (_videoInfo.get() == NULL) {
		log_debug("No audio data");
		return std::auto_ptr<VideoInfo>(NULL);
	}

	std::auto_ptr<VideoInfo> info(new VideoInfo(_videoInfo->codec, _videoInfo->width, _videoInfo->height, _videoInfo->frameRate, _videoInfo->duration, FLASH));
	return info;

}

std::auto_ptr<AudioInfo> FLVParser::getAudioInfo()
{

	boost::mutex::scoped_lock lock(_mutex);

	// If there are no audio in this FLV return NULL
	if (!_audio && _lastParsedPosition > 0) return std::auto_ptr<AudioInfo>(NULL);

	// Make sure that there are parsed some audio frames
	while(_audioInfo.get() == NULL && !_parsingComplete && !(!_video && _lastParsedPosition > 0)) {
		if (parseNextFrame() == false) break;
	}

	// If there are no audio data return NULL
	if (_audioInfo.get() == NULL) return std::auto_ptr<AudioInfo>(NULL);

	if (_audioInfo->codec == AUDIO_CODEC_MP3) _isAudioMp3 = true;
	else if (_audioInfo->codec == AUDIO_CODEC_NELLYMOSER || _audioInfo->codec == AUDIO_CODEC_NELLYMOSER_8HZ_MONO) _isAudioNellymoser = true;

	std::auto_ptr<AudioInfo> info(new AudioInfo(_audioInfo->codec, _audioInfo->sampleRate, _audioInfo->sampleSize, _audioInfo->stereo, _audioInfo->duration, FLASH));
	return info;

}

bool FLVParser::isTimeLoaded(boost::uint32_t time)
{
	boost::mutex::scoped_lock lock(_mutex);

	// Parse frames until the need time is found, or EOF
	while (!_parsingComplete) {
		if (!parseNextFrame()) break;
		if ((_videoFrames.size() > 0 && _videoFrames.back()->timestamp >= time)
			|| (_audioFrames.size() > 0 && _audioFrames.back()->timestamp >= time)) {
			return true;
		}
	}

	if (_videoFrames.size() > 0 && _videoFrames.back()->timestamp >= time) {
		return true;
	}

	if (_audioFrames.size() > 0 && _audioFrames.back()->timestamp >= time) {
		return true;
	}

	return false;

}

boost::uint32_t FLVParser::seek(boost::uint32_t time)
{
	boost::mutex::scoped_lock lock(_mutex);

	if (time == 0) {
		if (_video) _nextVideoFrame = 0;
		if (_audio) _nextAudioFrame = 0;
	}

	if (_video)	time = seekVideo(time);
	if (_audio)	time = seekAudio(time);
	return time;
}

#define HEADER_SKIP 15

bool FLVParser::parseNextFrame()
{
	// Parse the header if not done already. If unsuccesfull return false.
	if (_lastParsedPosition == 0 && !parseHeader()) return false;

	// Seek to next frame and skip the size of the last tag,
	// return false on error
	if (_stream->set_position(_lastParsedPosition+4) != 0) return false;

	// Read the tag info
	boost::uint8_t tag[12];
	_stream->read_bytes(tag, 12);

	// Extract length and timestamp
	boost::uint32_t bodyLength = getUInt24(&tag[1]);
	boost::uint32_t timestamp = getUInt24(&tag[4]);

	_lastParsedPosition += HEADER_SKIP + bodyLength;

	// check for empty tag
	if (bodyLength == 0) {
		return true;
	}

	if (tag[0] == AUDIO_TAG) {
		FLVAudioFrame* frame = new FLVAudioFrame;
		frame->dataSize = bodyLength - 1;
		frame->timestamp = timestamp;
		frame->dataPosition = _stream->get_position();
		_audioFrames.push_back(frame);

		// If this is the first audioframe no info about the
		// audio format has been noted, so we do that now
		if (_audioInfo.get() == NULL) {
			int samplerate = (tag[11] & 0x0C) >> 2;
			if (samplerate == 0) samplerate = 5500;
			else if (samplerate == 1) samplerate = 11000;
			else if (samplerate == 2) samplerate = 22050;
			else if (samplerate == 3) samplerate = 44100;

			int samplesize = (tag[11] & 0x02) >> 1;
			if (samplesize == 0) samplesize = 1;
			else samplesize = 2;

			_audioInfo.reset(new AudioInfo(static_cast<audioCodecType>((tag[11] & 0xf0) >> 4), samplerate, samplesize, (tag[11] & 0x01) >> 0, 0, FLASH));
		}

	} else if (tag[0] == VIDEO_TAG) {
		FLVVideoFrame* frame = new FLVVideoFrame;
		frame->dataSize = bodyLength - 1;
		frame->timestamp = timestamp;
		frame->dataPosition = _stream->get_position();
		frame->frameType = (tag[11] & 0xf0) >> 4;
		_videoFrames.push_back(frame);


		// If this is the first videoframe no info about the
		// video format has been noted, so we do that now
		if (_videoInfo.get() == NULL) {
			videoCodecType codec = static_cast<videoCodecType>((tag[11] & 0x0f) >> 0);
			// Set standard guessed size...
			boost::uint16_t width = 320;
			boost::uint16_t height = 240;

			// Extract the video size from the videodata header
			_stream->set_position(frame->dataPosition);
			boost::uint8_t videohead[12];
			_stream->read_bytes(videohead, 12);

			if (codec == VIDEO_CODEC_H263) {
				bool sizebit1 = (videohead[3] & 0x02);
				bool sizebit2 = (videohead[3] & 0x01);
				bool sizebit3 = (videohead[4] & 0x80);

				// First some predefined sizes
				if (!sizebit1 && sizebit2 && !sizebit3 ) {
					width = 352;
					height = 288;
				} else if (!sizebit1 && sizebit2 && sizebit3 ) {
					width = 176;
					height = 144;
				} else if (sizebit1 && !sizebit2 && !sizebit3 ) {
					width = 128;
					height = 96;
				} else if (sizebit1 && !sizebit2 && sizebit3 ) {
					width = 320;
					height = 240;
				} else if (sizebit1 && sizebit2 && !sizebit3 ) {
					width = 160;
					height = 120;

				// Then the custom sizes (1 byte)
				} else if (!sizebit1 && !sizebit2 && !sizebit3 ) {
					BitsReader* br = new BitsReader(&videohead[4], 8);
					br->read_bit();
					width = br->read_uint(8);
					height = br->read_uint(8);
					delete br;

				// Then the custom sizes (2 byte)
				} else if (!sizebit1 && !sizebit2 && sizebit3 ) {
					BitsReader* br = new BitsReader(&videohead[4], 8);
					br->read_bit();
					width = br->read_uint(16);
					height = br->read_uint(16);
					delete br;
				}
			} else if (codec == VIDEO_CODEC_VP6) {
				if (!(videohead[0] & 0x80)) {
					boost::uint32_t index = 0;
					if ((videohead[index] & 1) || !(videohead[index+1] & 0x06)) {
						index += 2;
					}
					index += 2;
					width = videohead[index++] * 16;
					height = videohead[index] * 16;

				}
			} else if (codec == VIDEO_CODEC_SCREENVIDEO) {
				BitsReader* br = new BitsReader(&videohead[0], 12);
				br->read_uint(4);
				width = br->read_uint(12);
				br->read_uint(4);
				height = br->read_uint(12);
				delete br;
			}
			

			// Create the videoinfo
			_videoInfo.reset(new VideoInfo(codec, width, height, 0 /*frameRate*/, 0 /*duration*/, FLASH));
		}

	} else if (tag[0] == META_TAG) {
		//log_debug("meta tag");
		// Extract information from the meta tag
		/*_lt.seek(_lastParsedPosition+16);
		char* metaTag = new char[bodyLength];
		_lt.read(metaTag, bodyLength);
		amf::AMF* amfParser = new amf::AMF();
		amfParser->parseAMF(metaTag);*/

	} else {
		//log_debug("no tag - the end?");
		// We can't be sure that the parsing is really complete,
		// maybe it's a corrupt FLV.
		_parsingComplete = true;
		return false;
	}

	return true;
}

bool FLVParser::parseHeader()
{
	// seek to the begining of the file
	_stream->set_position(0); //_lt.seek(0);

	// Read the header
	boost::uint8_t header[9];
	_stream->read_bytes(header, 9); //_lt.read(header, 9);

	// Check if this is really a FLV file
	if (header[0] != 'F' || header[1] != 'L' || header[2] != 'V') return false;

	_audio = false;
	_video = false;

	// Parse the audio+video bitmask
	if (header[4] & CONTAINS_AUDIO) {
		_audio = true;
	} 
	if (header[4] & CONTAINS_VIDEO) {
		_video = true;
	} 

	gnash::log_debug("FLV bit mask: %#x", header[4]);

	_lastParsedPosition = 9;
	return true;
}

inline boost::uint32_t FLVParser::getUInt24(boost::uint8_t* in)
{
	// The bits are in big endian order
	return (in[0] << 16) | (in[1] << 8) | in[2];
}

} // gnash.media namespace 
} // end of gnash namespace

#undef PADDING_BYTES
