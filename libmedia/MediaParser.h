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

#ifndef __MEDIAPARSER_H__
#define __MEDIAPARSER_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_file.h"

#ifdef USE_FFMPEG
#include <ffmpeg/avcodec.h>
#endif
namespace gnash {

enum codecType
{
	FLASH,
	FFMPEG
};

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
	AUDIO_CODEC_NELLYMOSER_8HZ_MONO = 5,	// According to ffmpeg
	AUDIO_CODEC_NELLYMOSER = 6	// Mystery proprietary format; see nellymoser.com
};

enum tagType
{
	AUDIO_TAG = 0x08,
	VIDEO_TAG = 0x09,
	META_TAG = 0x12
};

/// \brief
/// The AudioInfo class contains information about the audiostream
/// in the file being parsed. The information stored is codec-id,
/// samplerate, samplesize, stereo, duration and codec-type.
/// timestamp,
class AudioInfo
{
public:
	AudioInfo(int codeci, uint16_t sampleRatei, uint16_t sampleSizei, bool stereoi, uint64_t durationi, codecType typei)
		: codec(codeci),
		sampleRate(sampleRatei),
		sampleSize(sampleSizei),
		stereo(stereoi),
		duration(durationi),
		type(typei)
		{
		}

	int codec;
	uint16_t sampleRate;
	uint16_t sampleSize;
	bool stereo;
	uint64_t duration;
	codecType type;
};

/// \brief
/// The VideoInfo class contains information about the videostream
/// in the file being parsed. The information stored is codec-id,
/// width, height, framerate, duration and codec-type.
/// timestamp,
class VideoInfo
{
public:
	VideoInfo(int codeci, uint16_t widthi, uint16_t heighti, uint16_t frameRatei, uint64_t durationi, codecType typei)
		: codec(codeci),
		width(widthi),
		height(heighti),
		frameRate(frameRatei),
		duration(durationi),
#ifdef USE_FFMPEG
		videoCodecCtx(NULL),
#endif
		type(typei)
		{
		}

	int codec;
	uint16_t width;
	uint16_t height;
	uint16_t frameRate;
	uint64_t duration;
#ifdef USE_FFMPEG
	AVCodecContext* videoCodecCtx; // UGLY!!
#endif
	codecType type;
};

/// \brief
/// The MediaFrame class contains a video or audio frame, its size, its
/// timestamp. Ownership of the data is in the parser.
class MediaFrame
{
public:
	uint32_t dataSize;
	uint8_t* data;
	uint64_t timestamp;
	uint8_t tag;
};

/// \brief
/// The MediaParser class detects the format of the input file, and parses it on demand.
///
class MediaParser
{
public:
	MediaParser(tu_file* stream)
	:
	_isAudioMp3(false),
	_isAudioNellymoser(false),
	_stream(stream)
	{}

	/// Used to parse the next media frame in the stream and return it
	//
	/// @return a pointer to a MediaFrame in which the undecoded frame data is.
	virtual MediaFrame* parseMediaFrame() { return NULL; }

	/// Is the input MP3?
	//
	/// @return if the input audio is MP3
	bool isAudioMp3() { return _isAudioMp3; }

	/// Is the input Nellymoser?
	//
	/// @return if the input audio is Nellymoser
	bool isAudioNellymoser() { return _isAudioNellymoser; }

	/// Setup the parser
	//
	/// @return whether we'll be able to parse the file.
	virtual bool setupParser() { return false; }

	/// Returns a VideoInfo class about the videostream
	//
	/// @return a VideoInfo class about the videostream
	virtual std::auto_ptr<VideoInfo> getVideoInfo() { return std::auto_ptr<VideoInfo>(NULL); }

	/// Returns a AudioInfo class about the audiostream
	//
	/// @return a AudioInfo class about the audiostream
	virtual std::auto_ptr<AudioInfo> getAudioInfo() { return std::auto_ptr<AudioInfo>(NULL); }

	/// Seeks to the closest possible position the given position,
	/// and returns the new position.
	//
	/// @return the position the seek reached
	virtual uint32_t seek(uint32_t) { return 0; }

	/// Returns the framedelay from the last to the current
	/// audioframe in milliseconds. This is used for framerate.
	//
	/// @return the diff between the current and last frame
	virtual uint32_t audioFrameDelay() { return 0; }

	/// Returns the framedelay from the last to the current
	/// videoframe in milliseconds. 
	//
	/// @return the diff between the current and last frame
	virtual uint32_t videoFrameDelay() { return 0; }

	/// Returns the framerate of the video
	//
	/// @return the framerate of the video
	virtual uint16_t videoFrameRate() { return 0; }

	/// Returns the last parsed position in the file in bytes
	virtual uint32_t getLastParsedPos() { return 0; }

protected:

	/// Is the input audio MP3?
	bool _isAudioMp3;

	/// Is the input audio Nellymoser?
	bool _isAudioNellymoser;

	/// The stream used to access the file
	tu_file* _stream;
};


} // namespace gnash

#endif // __MEDIAPARSER_H__
