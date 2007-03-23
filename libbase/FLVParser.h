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

// $Id: FLVParser.h,v 1.1 2007/03/23 00:30:10 tgc Exp $

// Information about the FLV format can be found at http://osflash.org/flv

#ifndef __FLVPARSER_H__
#define __FLVPARSER_H__

#include "LoadThread.h"
#include <vector>

enum videoCodecType
{
	VIDEO_CODEC_H263 = 2,	// H263/SVQ3 video codec
	VIDEO_CODEC_SCREENVIDEO = 3,	// Screenvideo codec
	VIDEO_CODEC_VP6 = 4,		// On2 VP6 video codec
	VIDEO_CODEC_VP6A = 5,		// On2 VP6 Alpha video codec
	VIDEO_CODEC_SCREENVIDEO2 = 6	// Screenvideo2 codec
};

enum audioCodecType
{
	AUDIO_CODEC_RAW = 0,		// unspecified format.  Useful for 8-bit sounds???
	AUDIO_CODEC_ADPCM = 1,	// gnash doesn't pass this through; it uncompresses and sends FORMAT_NATIVE16
	AUDIO_CODEC_MP3 = 2,
	AUDIO_CODEC_UNCOMPRESSED = 3,	// 16 bits/sample, little-endian
	AUDIO_CODEC_NELLYMOSER = 6	// Mystery proprietary format; see nellymoser.com
};

enum tagType
{
	AUDIO_TAG = 0x08,
	VIDEO_TAG = 0x09,
	META_TAG = 0x12
};

enum videoFrameType
{
	KEY_FRAME = 1,
	INTER_FRAME = 2,
	DIS_INTER_FRAME = 3
};

/// \brief
/// The FLVFrame class contains a video or audio frame, its size, its
/// timestamp, 
class FLVFrame
{
public:
	int dataSize;
	uint8_t* data;
	uint32_t timestamp;
	uint8_t tag;
};

/// \brief
/// The FLVAudioInfo class contains information about the audiostream
/// in the FLV being parsed. The information stored is codec-type,
/// samplerate, samplesize, stereo and duration.
/// timestamp, 
class FLVAudioInfo
{
public:
	FLVAudioInfo(int codeci, int sampleRatei, int sampleSizei, bool stereoi, long durationi)
		: codec(codeci),
		sampleRate(sampleRatei),
		sampleSize(sampleSizei),
		stereo(stereoi),
		duration(durationi)
		{
		}

	int codec;
	int sampleRate;
	int sampleSize;
	bool stereo;
	long duration;
};

/// \brief
/// The FLVVideoInfo class contains information about the videostream
/// in the FLV being parsed. The information stored is codec-type,
/// width, height, framerate and duration.
/// timestamp, 
class FLVVideoInfo
{
public:
	FLVVideoInfo(int codeci, int widthi, int heighti, int frameRatei, long durationi)
		: codec(codeci),
		width(widthi),
		height(heighti),
		frameRate(frameRatei),
		duration(durationi)
		{
		}

	uint16_t codec;
	uint16_t width;
	uint16_t height;
	uint16_t frameRate;
	long duration;
};


class FLVVideoFrame
{
public:
	uint16_t frameType;
	uint32_t dataSize;
	long dataPosition;
	uint32_t timestamp;
	
};

class FLVAudioFrame
{
public:
	uint32_t dataSize;
	long dataPosition;
	uint32_t timestamp;
	
};

/// \brief
/// The FLVParser class parses a FLV file, and can return
/// video or audio frames for a specific time, or just 
/// get the next in the timeline.

class FLVParser
{

public:
	/// Creating the object...
	FLVParser();

	/// Kills the parser...
	~FLVParser();

	FLVFrame* nextMediaFrame();

	/// Returns the next audio frame in the timeline. If no frame has been
	/// played before the first frame is returned. If there is no more frames
	/// in the timeline NULL is returned.
	FLVFrame* nextAudioFrame();

	/// Returns the next video frame in the timeline. If no frame has been
	/// played before the first frame is returned. If there is no more frames
	/// in the timeline NULL is returned.
	FLVFrame* nextVideoFrame();

	/// Returns a FLVVideoInfo class about the videostream
	FLVVideoInfo* getVideoInfo();

	/// Returns a FLVAudioInfo class about the audiostream
	FLVAudioInfo* getAudioInfo();

	/// Sets the LoadThread which is used as interface
	void setLoadThread(LoadThread* lt);

	/// Asks if a frame with with a timestamp larger than
	/// the given time is available. If such a frame is not
	/// available in list of already the parsed frames, we
	/// parse some more. This is used to check how much is buffered.
	bool isTimeLoaded(uint32_t time);

	/// seeks to the closest possible position the given position,
	/// and returns the new position.
	uint32_t seek(uint32_t);

	/// Returns the framedelay from the last to the current
	/// videoframe in milliseconds. This is used for framerate.
	uint32_t videoFrameDelay();

private:

	/// seeks to the closest possible position the given position,
	/// and returns the new position.
	uint32_t seekAudio(uint32_t time);

	/// seeks to the closest possible position the given position,
	/// and returns the new position.
	uint32_t seekVideo(uint32_t time);


	/// Parses next frame from the file, returns true is a frame
	/// was succesfully parsed, or false if not enough data was present.
	bool parseNextFrame();

	/// Parses the header of the file
	bool parseHeader();

	// Functions used to extract numbers from the file
	inline uint32_t getUInt24(uint8_t* in);

	/// The interface to the file
	LoadThread* _lt;

	/// list of videoframes, does no contain the frame data.
	std::vector<FLVVideoFrame*> _videoFrames;

	/// list of audioframes, does no contain the frame data.
	std::vector<FLVAudioFrame*> _audioFrames;

	/// The position where the parsing should continue from.
	long _lastParsedPosition;

	/// Whether the parsing is complete or not
	bool _parsingComplete;

	/// Info about the video stream
	FLVVideoInfo* _videoInfo;

	/// Info about the audio stream
	FLVAudioInfo* _audioInfo;

	/// Last audio frame returned
	int _lastAudioFrame;

	/// Last video frame returned
	int _lastVideoFrame;

	/// Audio stream is present
	bool _audio;

	/// Audio stream is present
	bool _video;
};

#endif // __FLVPARSER_H__
