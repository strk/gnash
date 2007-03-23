// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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

// $Id: FLVParser.cpp,v 1.1 2007/03/23 00:30:10 tgc Exp $

#include "FLVParser.h"
#include "amf.h"

FLVParser::FLVParser()
	:
	_lt(NULL),
	_lastParsedPosition(0),
	_parsingComplete(false),
	_videoInfo(NULL),
	_audioInfo(NULL),
	_lastAudioFrame(-1),
	_lastVideoFrame(-1),
	_audio(false),
	_video(false)
{
}

FLVParser::~FLVParser()
{
	_videoFrames.clear();

	_audioFrames.clear();
}

uint32_t FLVParser::videoFrameDelay()
{
	if (!_video || _lastVideoFrame < 1) return 0;

	return _videoFrames[_lastVideoFrame]->timestamp - _videoFrames[_lastVideoFrame-1]->timestamp;
}

FLVFrame* FLVParser::nextMediaFrame()
{
	uint32_t video_size = _videoFrames.size();
	uint32_t audio_size = _audioFrames.size();
	

	// Parse a media frame if any left or if needed
	while(video_size == _videoFrames.size() && audio_size == _audioFrames.size() && !_parsingComplete) {
		parseNextFrame();
	}

	// Find the next frame in the file
	bool audioReady = _audioFrames.size() > _lastAudioFrame+1;
	bool videoReady = _videoFrames.size() > _lastVideoFrame+1;
	bool useAudio = false;

	if (audioReady && videoReady) {
		useAudio = _audioFrames[_lastAudioFrame+1]->dataPosition < _videoFrames[_lastVideoFrame+1]->dataPosition;
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
		_lastAudioFrame++;

		FLVFrame* frame = new FLVFrame;
		frame->dataSize = _audioFrames[_lastAudioFrame]->dataSize;
		frame->timestamp = _audioFrames[_lastAudioFrame]->timestamp;

		_lt->seek(_audioFrames[_lastAudioFrame]->dataPosition);
		frame->data = new uint8_t[_audioFrames[_lastAudioFrame]->dataSize];
		_lt->read(frame->data, _audioFrames[_lastAudioFrame]->dataSize);
		frame->tag = 8;
		return frame;
	
	} else {
		_lastVideoFrame++;

		FLVFrame* frame = new FLVFrame;
		frame->dataSize = _videoFrames[_lastVideoFrame]->dataSize;
		frame->timestamp = _videoFrames[_lastVideoFrame]->timestamp;

		_lt->seek(_videoFrames[_lastVideoFrame]->dataPosition);
		frame->data = new uint8_t[_videoFrames[_lastVideoFrame]->dataSize];
		_lt->read(frame->data, _videoFrames[_lastVideoFrame]->dataSize);
		frame->tag = 9;
		return frame;
	}

}

FLVFrame* FLVParser::nextAudioFrame()
{
	// If there are no audio in this FLV return NULL
	if (!_audio && _lastParsedPosition > 0) return NULL;

	// Make sure that there are parsed enough frames to return the need frame
	while(_audioFrames.size() <= static_cast<uint32_t>(_lastAudioFrame+1) && !_parsingComplete) {
		parseNextFrame();
	}

	// If the needed frame can't be parsed (EOF reached) return NULL
	if (_audioFrames.size() < static_cast<uint32_t>(_lastAudioFrame+1)) return NULL;

	_lastAudioFrame++;

	FLVFrame* frame = new FLVFrame;
	frame->dataSize = _audioFrames[_lastAudioFrame]->dataSize;
	frame->timestamp = _audioFrames[_lastAudioFrame]->timestamp;

	_lt->seek(_audioFrames[_lastAudioFrame]->dataPosition);
	frame->data = new uint8_t[_audioFrames[_lastAudioFrame]->dataSize];
	_lt->read(frame->data, _audioFrames[_lastAudioFrame]->dataSize);
	return frame;

}

FLVFrame* FLVParser::nextVideoFrame()
{
	// If there are no video in this FLV return NULL
	if (!_video && _lastParsedPosition > 0) return NULL;

	// Make sure that there are parsed enough frames to return the need frame
	while(_videoFrames.size() <= static_cast<uint32_t>(_lastVideoFrame+1) && !_parsingComplete) {
		parseNextFrame();
	}

	// If the needed frame can't be parsed (EOF reached) return NULL
	if (_videoFrames.size() < static_cast<uint32_t>(_lastVideoFrame+1)) return NULL;

	_lastVideoFrame++;

	FLVFrame* frame = new FLVFrame;
	frame->dataSize = _videoFrames[_lastVideoFrame]->dataSize;
	frame->timestamp = _videoFrames[_lastVideoFrame]->timestamp;

	_lt->seek(_videoFrames[_lastVideoFrame]->dataPosition);
	frame->data = new uint8_t[_videoFrames[_lastVideoFrame]->dataSize];
	_lt->read(frame->data, _videoFrames[_lastVideoFrame]->dataSize);
	return frame;

}


uint32_t FLVParser::seekAudio(uint32_t time)
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
	if (_audioFrames.back()->timestamp < time) {
		_lastVideoFrame = _audioFrames.size() - 2;
		return _audioFrames.back()->timestamp;
	}

	// We try to guess where in the vector the audioframe
	// with the correct timestamp is
	uint32_t numFrames = _audioFrames.size();
	uint32_t guess = _audioFrames[numFrames-1]->timestamp / numFrames * time;

	// Here we test if the guess was ok, and adjust if needed.
	uint32_t bestFrame = guess;
	uint32_t diff = abs(_audioFrames[bestFrame]->timestamp - time);
	while (true) {
		if (bestFrame+1 < numFrames && static_cast<uint32_t>(abs(_audioFrames[bestFrame+1]->timestamp - time)) < diff) {
			bestFrame = bestFrame + 1;
			diff = abs(_audioFrames[bestFrame+1]->timestamp - time);
		} else if (bestFrame-1 > 0 && static_cast<uint32_t>(abs(_audioFrames[bestFrame+1]->timestamp - time)) < diff) {
			bestFrame = bestFrame - 1;
			diff = abs(_audioFrames[bestFrame-1]->timestamp - time);
		} else {
			break;
		}
	}

	_lastAudioFrame = bestFrame -1;
	return _audioFrames[bestFrame]->timestamp;

}


uint32_t FLVParser::seekVideo(uint32_t time)
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
	uint32_t numFrames = _audioFrames.size();
	if (lastFrame->timestamp < time) {
		uint32_t lastFrameNum = numFrames -1;
		while (lastFrame->frameType != KEY_FRAME) {
			lastFrameNum--;
			lastFrame = _videoFrames[lastFrameNum];
		}

		_lastVideoFrame = lastFrameNum-1;
		return lastFrame->timestamp;

	}

	// We try to guess where in the vector the videoframe
	// with the correct timestamp is
	uint32_t guess = lastFrame->timestamp / numFrames * time;

	// Here we test if the guess was ok, and adjust if needed.
	uint32_t bestFrame = guess;
	uint32_t diff = abs(_audioFrames[bestFrame]->timestamp - time);
	while (true) {
		if (bestFrame+1 < numFrames && static_cast<uint32_t>(abs(_audioFrames[bestFrame+1]->timestamp - time)) < diff) {
			bestFrame = bestFrame + 1;
			diff = abs(_audioFrames[bestFrame+1]->timestamp - time);
		} else if (bestFrame-1 > 0 && static_cast<uint32_t>(abs(_audioFrames[bestFrame+1]->timestamp - time)) < diff) {
			bestFrame = bestFrame - 1;
			diff = abs(_audioFrames[bestFrame-1]->timestamp - time);
		} else {
			break;
		}
	}

	uint32_t rewindKeyframe = bestFrame;
	uint32_t forwardKeyframe = bestFrame;

	// Rewind to the lastest keyframe
	while (_videoFrames[rewindKeyframe]->frameType != KEY_FRAME) {
		rewindKeyframe--;
	}

	// Forward to the next keyframe
	uint32_t size = _videoFrames.size();
	while (size > forwardKeyframe && _videoFrames[forwardKeyframe]->frameType != KEY_FRAME) {
		forwardKeyframe++;
	}

	int32_t forwardDiff = _videoFrames[forwardKeyframe]->timestamp - time;
	int32_t rewindDiff = time - _videoFrames[rewindKeyframe]->timestamp;

	if (forwardDiff < rewindDiff) bestFrame = forwardKeyframe;
	else bestFrame = rewindKeyframe;

	_lastVideoFrame = bestFrame - 1;
	return _audioFrames[bestFrame]->timestamp;
}



FLVVideoInfo* FLVParser::getVideoInfo()
{
	// If there are no video in this FLV return NULL
	if (!_video && _lastParsedPosition > 0) return NULL;

	// Make sure that there are parsed some video frames
	while(_videoInfo == NULL && !_parsingComplete) {
		parseNextFrame();
	}

	// If there are no audio data return NULL
	if (_videoInfo == NULL) return NULL;

	FLVVideoInfo* info = new FLVVideoInfo(_videoInfo->codec, _videoInfo->width, _videoInfo->height, _videoInfo->frameRate, _videoInfo->duration);
	return info;

}

FLVAudioInfo* FLVParser::getAudioInfo()
{
	// If there are no audio in this FLV return NULL
	if (!_audio && _lastParsedPosition > 0) return NULL;

	// Make sure that there are parsed some audio frames
	while(_audioInfo == NULL && !_parsingComplete) {
		parseNextFrame();
	}

	// If there are no audio data return NULL
	if (_audioInfo == NULL) return NULL;

	FLVAudioInfo* info = new FLVAudioInfo(_audioInfo->codec, _audioInfo->sampleRate, _audioInfo->sampleSize, _audioInfo->stereo, _audioInfo->duration);
	return info;

}

bool FLVParser::isTimeLoaded(uint32_t time)
{
	// Parse frames until the need time is found, or EOF
	while (!_parsingComplete && (_videoFrames.size() > 0 && _videoFrames.back()->timestamp < time) && (_audioFrames.size() > 0 && _audioFrames.back()->timestamp < time)) {
		parseNextFrame();
	}

	if (_videoFrames.size() > 0 && _videoFrames.back()->timestamp >= time) {
		return true;
	}

	if (_audioFrames.size() > 0 && _audioFrames.back()->timestamp >= time) {
		return true;
	}
	return false;

}

uint32_t FLVParser::seek(uint32_t time)
{

	if (_video)	time = seekVideo(time);
	if (_audio)	time = seekAudio(time);
	return time;
}

bool FLVParser::parseNextFrame()
{

	// Parse the header if not done already. If unsuccesfull return false.
	if (_lastParsedPosition == 0 && !parseHeader()) return false;

	// Check if there is enough data to parse the header of the frame
	if (!_lt->isPositionConfirmed(_lastParsedPosition+14)) return false;

	// Seek to next frame and skip the size of the last tag
	_lt->seek(_lastParsedPosition+4);

	// Read the tag info
	uint8_t tag[12];
	_lt->read(tag, 12);

	// Extract length and timestamp
	uint32_t bodyLength = getUInt24(&tag[1]);
	uint32_t timestamp = getUInt24(&tag[4]);

	// Check if there is enough data to parse the body of the frame
	if (!_lt->isPositionConfirmed(_lastParsedPosition+15+bodyLength)) return false;

	if (tag[0] == AUDIO_TAG) {
		FLVAudioFrame* frame = new FLVAudioFrame;
		frame->dataSize = bodyLength - 1;
		frame->timestamp = timestamp;
		frame->dataPosition = _lt->tell();
		_audioFrames.push_back(frame);
		
		// If this is the first audioframe no info about the
		// audio format has been noted, so we do that now
		if (_audioInfo == NULL) {
			_audioInfo = new FLVAudioInfo((tag[11] & 0xf0) >> 4, (tag[11] & 0x0C) >> 2, (tag[11] & 0x02) >> 1, (tag[11] & 0x01) >> 0, 0);
		}
		_lastParsedPosition += 15 + bodyLength;

	} else if (tag[0] == VIDEO_TAG) {
		FLVVideoFrame* frame = new FLVVideoFrame;
		frame->dataSize = bodyLength - 1;
		frame->timestamp = timestamp;
		frame->dataPosition = _lt->tell();
		frame->frameType = (tag[11] & 0xf0) >> 4;
		_videoFrames.push_back(frame);
		
		// If this is the first videoframe no info about the
		// video format has been noted, so we do that now
		if (_videoInfo == NULL) {

			// TODO: parse the video frame header to extract info about the width and height.
			_videoInfo = new FLVVideoInfo((tag[11] & 0x0f) >> 0, 0 /*width*/, 0 /*height*/, 0 /*frameRate*/, 0 /*duration*/);
		}
		_lastParsedPosition += 15 + bodyLength;

	} else if (tag[0] == META_TAG) {
		// Extract information from the meta tag
		/*_lt->seek(_lastParsedPosition+16);
		char* metaTag = new char[bodyLength];
		_lt->read(metaTag, bodyLength);
		amf::AMF* amfParser = new amf::AMF();
		amfParser->parseAMF(metaTag);*/

		_lastParsedPosition += 15 + bodyLength;
	} else {
		_parsingComplete = true;
	}
	return true;
}

bool FLVParser::parseHeader()
{
	// seek to the begining of the file
	_lt->seek(0);
	
	// Read the header
	uint8_t header[9];
	_lt->read(header, 9);

	// Check if this is really a FLV file
	if (header[0] != 'F' || header[1] != 'L' || header[2] != 'V') return false;

	// Parse the audio+video bitmask
	if (header[4] == 5) {
		_audio = true;
		_video = true;
	} else if (header[4] == 4) {
		_audio = true;
		_video = false;
	} else if (header[4] == 4) {
		_audio = false;
		_video = true;
	} else {
		printf("Weird bit mask\n");
	}

	_lastParsedPosition = 9;
	return true;
}

inline uint32_t FLVParser::getUInt24(uint8_t* in)
{
	// The bits are in big endian order
	return (in[0] << 16) | (in[1] << 8) | in[2];
}

void FLVParser::setLoadThread(LoadThread* lt)
{
	_lt = lt;
}
