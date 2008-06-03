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

#include "dsodefs.h"
#include "MediaParser.h" // for inheritance

#include <vector>
#include <memory>

namespace gnash {
namespace media {

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
//
/// TODO: drop
///
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
//
/// TODO: drop
///
class FLVVideoInfo
{
public:
	FLVVideoInfo(boost::uint16_t codeci, boost::uint16_t widthi, boost::uint16_t heighti, boost::uint16_t frameRatei, boost::uint64_t durationi)
		:
		codec(codeci),
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

/// The FLVParser class parses FLV streams
class DSOEXPORT FLVParser : public MediaParser
{

public:

	/// \brief
	/// Create an FLV parser reading input from
	/// the given tu_file
	//
	/// @param lt
	/// 	tu_file to use for input.
	/// 	Ownership transferred.
	///
	FLVParser(std::auto_ptr<tu_file> lt);

	/// Kills the parser...
	~FLVParser();

	// see dox in MediaParser.h
	bool nextAudioFrameTimestamp(boost::uint64_t& ts);

	// see dox in MediaParser.h
	bool nextVideoFrameTimestamp(boost::uint64_t& ts);

	// see dox in MediaParser.h
	std::auto_ptr<EncodedAudioFrame> nextAudioFrame();

	// see dox in MediaParser.h
	std::auto_ptr<EncodedVideoFrame> nextVideoFrame();

	/// Returns information about video in the stream.
	//
	/// The returned object is owned by the FLVParser object.
	/// Can return NULL if video contains NO video frames.
	/// Will block till either parsing finished or a video frame is found.
	///
	VideoInfo* getVideoInfo();

	/// Returns a FLVAudioInfo class about the audiostream
	//
	/// TODO: return a more abstract AudioInfo
	///
	AudioInfo* getAudioInfo();

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

	virtual bool parseNextChunk() {
		return parseNextTag();
	}

	/// Parses next tag from the file
	//
	/// Returns true if something was parsed, false otherwise.
	/// Sets _parsingComplete=true on end of file.
	///
	bool parseNextTag();

	/// Return number of bytes parsed so far
	boost::uint64_t getBytesLoaded() const;

private:

	FLVAudioFrameInfo* peekNextAudioFrameInfo();

	FLVVideoFrameInfo* peekNextVideoFrameInfo();

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

	/// Info about the video stream (if any)
	std::auto_ptr<VideoInfo> _videoInfo;

	/// Info about the audio stream (if any)
	std::auto_ptr<AudioInfo> _audioInfo;

	/// Last audio frame returned
	size_t _nextAudioFrame;

	/// Last video frame returned
	size_t _nextVideoFrame;

	/// Audio stream is present
	bool _audio;

	/// Audio stream is present
	bool _video;
};

} // end of gnash::media namespace
} // end of gnash namespace

#endif // __FLVPARSER_H__
