// FLVParser.h:  Flash Video file format parser, for Gnash.
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


// Information about the FLV format can be found at http://osflash.org/flv

#ifndef __FLVPARSER_H__
#define __FLVPARSER_H__

#include "LoadThread.h"
#include "dsodefs.h"
#include <vector>
#include <memory>

namespace gnash {

/// Frame type
enum FrameType {

	/// Video frame
	videoFrame,

	/// Audio frame
	audioFrame
};

/// \brief
/// The FLVFrame class contains an encoded
/// video or audio frame, its size, its
/// timestamp,
class FLVFrame
{
public:
	/// Size of the encoded frame
	boost::uint32_t dataSize;

	/// Encoded data
	boost::uint8_t* data;

	/// Frame timestamp, in milliseconds
	boost::uint64_t timestamp;

	/// Frame type (audio/video)
	FrameType type;
};

/// \brief
/// The FLVAudioInfo class contains information about the audiostream
/// in the FLV being parsed. The information stored is codec-type,
/// samplerate, samplesize, stereo flag and duration.
class FLVAudioInfo
{
public:
	FLVAudioInfo(boost::uint16_t codeci, boost::uint16_t sampleRatei, boost::uint16_t sampleSizei, bool stereoi, boost::uint64_t durationi)
		: codec(codeci),
		sampleRate(sampleRatei),
		sampleSize(sampleSizei),
		stereo(stereoi),
		duration(durationi)
		{
		}

	boost::uint16_t codec;
	boost::uint16_t sampleRate;
	boost::uint16_t sampleSize;
	bool stereo;
	boost::uint64_t duration;
};

/// \brief
/// The FLVVideoInfo class contains information about the videostream
/// in the FLV being parsed. The information stored is codec-type,
/// width, height, framerate and duration.
class FLVVideoInfo
{
public:
	FLVVideoInfo(boost::uint16_t codeci, boost::uint16_t widthi, boost::uint16_t heighti, boost::uint16_t frameRatei, boost::uint64_t durationi)
		: codec(codeci),
		width(widthi),
		height(heighti),
		frameRate(frameRatei),
		duration(durationi)
		{
		}

	boost::uint16_t codec;
	boost::uint16_t width;
	boost::uint16_t height;
	boost::uint16_t frameRate;
	boost::uint64_t duration;
};


/// Information about an FLV Audio Frame
class FLVVideoFrameInfo
{
public:

	/// Type of this frame (should likely be videoFrameType)
	boost::uint16_t frameType;

	/// Size of the frame in bytes
	boost::uint32_t dataSize;

	/// Start of frame data in stream
	boost::uint64_t dataPosition;

	/// Timestamp in milliseconds 
	boost::uint32_t timestamp;

	/// Return true if this video frame is a key frame
	bool isKeyFrame() const
	{
		return frameType == 1 /*KEY_FRAME*/;
	}

};

/// Information about an FLV Audio Frame
class FLVAudioFrameInfo
{
public:
	/// Size of the frame in bytes
	boost::uint32_t dataSize;

	/// Start of frame data in stream
	boost::uint64_t dataPosition;

	/// Timestamp in milliseconds 
	boost::uint32_t timestamp;

};

/// \brief
/// The FLVParser class parses an FLV stream, buffers audio/video frames
/// and provides cursor-based access to them.
//
/// Cursor-based access allow seeking as close as possible to a specified time
/// and fetching frames from there on, sequentially.
/// See seek(), nextVideoFrame(), nextAudioFrame() and nextMediaFrame().
///
/// Input is received from a tu_file object.
///
class DSOEXPORT FLVParser
{

public:

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

	enum videoFrameType
	{
		KEY_FRAME = 1,
		INTER_FRAME = 2,
		DIS_INTER_FRAME = 3
	};


	/// \brief
	/// Create an FLV parser reading input from
	/// the given LoadThread
	//
	/// @param lt
	/// 	LoadThread to use for input.
	/// 	Ownership left to the caller.
	///	TODO: take ownership
	///
	FLVParser(tu_file& lt);

	/// Kills the parser...
	~FLVParser();

	/// \brief
	/// Return the next audio frame info in the parsed buffer.
	//
	/// If no frame has been played before the first frame is returned.
	/// If there is no more frames in the parsed buffer NULL is returned,
	/// you can check with parsingCompleted() to know wheter this is due to 
	/// EOF reached.
	///
	/// TODO: return a more abstract EncodedAudioFrameInfo
	///
	FLVAudioFrameInfo* peekNextAudioFrameInfo();

	/// \brief
	/// Returns the next audio frame in the parsed buffer, advancing audio cursor.
	//
	/// If no frame has been played before the first frame is returned.
	/// If there is no more frames in the parsed buffer NULL is returned,
	/// you can check with parsingCompleted() to know wheter this is due to 
	/// EOF reached.
	///
	/// TODO: return a more abstract EncodedAudioFrame
	///
	FLVFrame* nextAudioFrame();

	/// Returns the next video frame info in the parsed buffer.
	//
	/// If no frame has been played before the first frame is returned.
	/// If there is no more frames in the parsed buffer NULL is returned.
	/// you can check with parsingCompleted() to know wheter this is due to 
	/// EOF reached.
	///
	/// TODO: return a more abstract EncodedVideoFrameInfo
	///
	FLVVideoFrameInfo* peekNextVideoFrameInfo();

	/// Returns the next video frame in the parsed buffer, advancing video cursor.
	//
	/// If no frame has been played before the first frame is returned.
	/// If there is no more frames in the parsed buffer NULL is returned.
	/// you can check with parsingCompleted() to know wheter this is due to 
	/// EOF reached.
	///
	/// TODO: return a more abstract EncodedVideoFrame
	///
	FLVFrame* nextVideoFrame();

	/// Return true of parsing is completed
	//
	/// If this function returns true, any call to nextVideoFrame() or nextAudioFrame
	/// will always return NULL
	///
	bool parsingCompleted() const { return _parsingComplete; }

	/// Returns information about video in the stream.
	//
	/// The returned object is owned by the FLVParser object.
	/// Can return NULL if video contains NO video frames.
	/// Will block till either parsing finished or a video frame is found.
	///
	/// TODO: return a more abstract VideoInfo
	///
	FLVVideoInfo* getVideoInfo();

	/// Returns a FLVAudioInfo class about the audiostream
	//
	/// TODO: return a more abstract AudioInfo
	///
	FLVAudioInfo* getAudioInfo();

	/// \brief
	/// Asks if a frame with with a timestamp larger than
	/// the given time is available.
	//
	/// If such a frame is not
	/// available in list of already the parsed frames, we
	/// parse some more. This is used to check how much is buffered.
	///
	/// @param time
	///	Timestamp, in milliseconds.
	///
	bool isTimeLoaded(boost::uint32_t time);

	/// \brief
	/// Seeks to the closest possible position the given position,
	/// and returns the new position.
	//
	///
	/// TODO: throw something for sending Seek.InvalidTime ?
	///       (triggered by seeks beyond the end of video or beyond what's
	///        downloaded so far)
	///
	boost::uint32_t seek(boost::uint32_t);

	/// Returns the framedelay from the last to the current
	/// audioframe in milliseconds. This is used for framerate.
	//
	boost::uint32_t audioFrameDelay();

	/// \brief
	/// Returns the framedelay from the last to the current
	/// videoframe in milliseconds. 
	//
	boost::uint32_t videoFrameDelay();

	/// Returns the framerate of the video
	//
	boost::uint16_t videoFrameRate();

	/// Returns the "bufferlength", meaning the differens between the
	/// current frames timestamp and the timestamp of the last parseable
	/// frame. Returns the difference in milliseconds.
	//
	boost::uint32_t getBufferLength();

	/// Parses next tag from the file
	//
	/// Returns true if something was parsed, false otherwise.
	/// Sets _parsingComplete=true on end of file.
	///
	bool parseNextTag();

	/// Return number of bytes parsed so far
	boost::uint64_t getBytesLoaded() const;

	/// Return total number of bytes in input
	boost::uint64_t getBytesTotal() const;

private:

	/// seeks to the closest possible position the given position,
	/// and returns the new position.
	boost::uint32_t seekAudio(boost::uint32_t time);

	/// seeks to the closest possible position the given position,
	/// and returns the new position.
	boost::uint32_t seekVideo(boost::uint32_t time);

	/// Parses the header of the file
	bool parseHeader();

	// Functions used to extract numbers from the file
	inline boost::uint32_t getUInt24(boost::uint8_t* in);

	/// The interface to the file, externally owned
	tu_file& _lt;

	// NOTE: FLVVideoFrameInfo is a relatively small structure,
	//       chances are keeping by value here would reduce
	//       memory fragmentation with no big cost
	typedef std::vector<FLVVideoFrameInfo*> VideoFrames;

	/// list of videoframes, does no contain the frame data.
	VideoFrames _videoFrames;

	// NOTE: FLVAudioFrameInfo is a relatively small structure,
	//       chances are keeping by value here would reduce
	//       memory fragmentation with no big cost
	typedef std::vector<FLVAudioFrameInfo*> AudioFrames;

	/// list of audioframes, does no contain the frame data.
	AudioFrames _audioFrames;

	/// The position where the parsing should continue from.
	boost::uint64_t _lastParsedPosition;

	/// Whether the parsing is complete or not
	bool _parsingComplete;

	/// Info about the video stream (if any)
	std::auto_ptr<FLVVideoInfo> _videoInfo;

	/// Info about the audio stream (if any)
	std::auto_ptr<FLVAudioInfo> _audioInfo;

	/// Last audio frame returned
	size_t _nextAudioFrame;

	/// Last video frame returned
	size_t _nextVideoFrame;

	/// Audio stream is present
	bool _audio;

	/// Audio stream is present
	bool _video;
};

} // end of gnash namespace

#endif // __FLVPARSER_H__
