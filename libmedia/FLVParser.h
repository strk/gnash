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

#include <boost/thread/mutex.hpp>

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


/// Information about an FLV Video Frame
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
	/// the given IOChannel
	//
	/// @param lt
	/// 	IOChannel to use for input.
	/// 	Ownership transferred.
	///
	FLVParser(std::auto_ptr<IOChannel> lt);

	/// Kills the parser...
	~FLVParser();

	// see dox in MediaParser.h
	virtual bool seek(boost::uint32_t&);

	/// Parses next tag from the file
	//
	/// Returns true if something was parsed, false otherwise.
	/// Sets _parsingComplete=true on end of file.
	///
	bool parseNextTag();

	/// Return number of bytes parsed so far
	boost::uint64_t getBytesLoaded() const;

private:

	virtual bool parseNextChunk();

	/// Parses the header of the file
	bool parseHeader();

	// Functions used to extract numbers from the file
	inline boost::uint32_t getUInt24(boost::uint8_t* in);

	/// The position where the parsing should continue from.
	/// Will be reset on seek, and will be protected by the _streamMutex
	boost::uint64_t _lastParsedPosition;

	/// On seek, this flag will be set, while holding a lock on _streamMutex.
	/// The parser, when obtained a lock on _streamMutex, will check this
	/// flag, if found to be true will clear the buffers and reset to false.
	bool _seekRequested;

	/// Audio frame cursor position 
	//
	/// This is the video frame number that will
	/// be referenced by nextVideoFrame and nextVideoFrameTimestamp
	///
	size_t _nextAudioFrame;

	/// Video frame cursor position 
	//
	/// This is the video frame number that will
	/// be referenced by nextVideoFrame and nextVideoFrameTimestamp
	///
	size_t _nextVideoFrame;

	/// Audio stream is present
	bool _audio;

	/// Audio stream is present
	bool _video;

	std::auto_ptr<EncodedAudioFrame> readAudioFrame(boost::uint32_t dataSize, boost::uint32_t timestamp);

	std::auto_ptr<EncodedVideoFrame> readVideoFrame(boost::uint32_t dataSize, boost::uint32_t timestamp);
};

} // end of gnash::media namespace
} // end of gnash namespace

#endif // __FLVPARSER_H__
