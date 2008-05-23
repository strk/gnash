// MediaParser.h: Base class for media parsers
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


#ifndef __MEDIAPARSER_H__
#define __MEDIAPARSER_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/shared_ptr.hpp>
#include "tu_file.h"

#ifdef USE_FFMPEG
#ifdef HAVE_FFMPEG_AVCODEC_H
extern "C" {
# include "ffmpeg/avcodec.h"
}
#endif

#ifdef HAVE_LIBAVCODEC_AVCODEC_H
extern "C" {
# include "libavcodec/avcodec.h"
}
#endif
#endif // USE_FFMPEG

#include <memory>

namespace gnash {
namespace media {

/// Video frame types
enum videoFrameType
{
	/// Key frames
	KEY_FRAME = 1,

	/// Interlaced frames
	INTER_FRAME = 2,

	/// Disposable interlaced frames
	DIS_INTER_FRAME = 3
};

/// The type of the codec id passed in the AudioInfo or VideoInfo class
enum codecType
{
	/// The internal flash codec ids
	FLASH,

	/// Ffmpegs codecs ids
	FFMPEG
};

/// Video codec ids as defined in flash
enum videoCodecType
{
	/// H263/SVQ3 video codec
	VIDEO_CODEC_H263 = 2,

	/// Screenvideo codec
	VIDEO_CODEC_SCREENVIDEO = 3,

	/// On2 VP6 video codec
	VIDEO_CODEC_VP6 = 4,

	/// On2 VP6 Alpha video codec
	VIDEO_CODEC_VP6A = 5,

	/// Screenvideo2 codec
	VIDEO_CODEC_SCREENVIDEO2 = 6
};

/// Audio codec ids as defined in flash
enum audioCodecType
{
	/// Raw format.  Useful for 8-bit sounds???
	AUDIO_CODEC_RAW = 0,	

	/// ADPCM format, flash's ADPCM is a bit different for normal ADPCM
	AUDIO_CODEC_ADPCM = 1,

	/// Mp3 format
	AUDIO_CODEC_MP3 = 2,

	/// 16 bits/sample, little-endian
	AUDIO_CODEC_UNCOMPRESSED = 3,

	/// Proprietary simple format
	AUDIO_CODEC_NELLYMOSER_8HZ_MONO = 5,

	/// Proprietary simple format
	AUDIO_CODEC_NELLYMOSER = 6
};

/// Type of frame in FLVs. Also type of the frame contained in the MediaFrame class.
enum tagType
{
	/// Audio frame
	AUDIO_TAG = 0x08,

	/// Video frame
	VIDEO_TAG = 0x09,

	/// Meta frame
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
	AudioInfo(int codeci, boost::uint16_t sampleRatei, boost::uint16_t sampleSizei, bool stereoi, boost::uint64_t durationi, codecType typei)
		: codec(codeci),
		sampleRate(sampleRatei),
		sampleSize(sampleSizei),
		stereo(stereoi),
		duration(durationi),
#ifdef USE_FFMPEG
		audioCodecCtx(NULL),
#endif
		type(typei)
		{
		}

	int codec;
	boost::uint16_t sampleRate;
	boost::uint16_t sampleSize;
	bool stereo;
	boost::uint64_t duration;
#ifdef USE_FFMPEG
	AVCodecContext* audioCodecCtx; // If videoCodecCtx is ugly, so is this.
#endif
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
	VideoInfo(int codeci, boost::uint16_t widthi, boost::uint16_t heighti, boost::uint16_t frameRatei, boost::uint64_t durationi, codecType typei)
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
	boost::uint16_t width;
	boost::uint16_t height;
	boost::uint16_t frameRate;
	boost::uint64_t duration;
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
	boost::uint32_t dataSize;
	boost::uint8_t* data;
	boost::uint64_t timestamp;
	boost::uint8_t tag;
};

/// \brief
/// The MediaParser class detects the format of the input file, and parses it on demand.
///
class MediaParser
{
public:
	MediaParser(boost::shared_ptr<tu_file> stream)
	:
	_isAudioMp3(false),
	_isAudioNellymoser(false),
	_stream(stream)
	{}

	// Classes with virtual methods (virtual classes)
	// must have a virtual destructor, or the destructors
	// of subclasses will never be invoked, tipically resulting
	// in memory leaks..
	//
	virtual ~MediaParser() {};

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
	virtual boost::uint32_t seek(boost::uint32_t) { return 0; }

	/// Returns the framedelay from the last to the current
	/// audioframe in milliseconds. This is used for framerate.
	//
	/// @return the diff between the current and last frame
	virtual boost::uint32_t audioFrameDelay() { return 0; }

	/// Returns the framedelay from the last to the current
	/// videoframe in milliseconds. 
	//
	/// @return the diff between the current and last frame
	virtual boost::uint32_t videoFrameDelay() { return 0; }

	/// Returns the framerate of the video
	//
	/// @return the framerate of the video
	virtual boost::uint16_t videoFrameRate() { return 0; }

	/// Returns the last parsed position in the file in bytes
	virtual boost::uint32_t getLastParsedPos() { return 0; }

protected:

	/// Is the input audio MP3?
	bool _isAudioMp3;

	/// Is the input audio Nellymoser?
	bool _isAudioNellymoser;

	/// The stream used to access the file
	boost::shared_ptr<tu_file> _stream;
};


} // gnash.media namespace 
} // namespace gnash

#endif // __MEDIAPARSER_H__
