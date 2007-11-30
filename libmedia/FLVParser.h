// FLVParser.h:  Flash Video file format parser, for Gnash.
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
//

// $Id: FLVParser.h,v 1.4 2007/11/30 00:13:01 tgc Exp $

// Information about the FLV format can be found at http://osflash.org/flv

#ifndef __FLVPARSER_H__
#define __FLVPARSER_H__

#include <tu_file.h>
#include <vector>
#include <boost/thread/mutex.hpp>
#include "MediaParser.h"

namespace gnash {
namespace media {

enum {
	CONTAINS_VIDEO = 1,
	CONTAINS_AUDIO = 4,
};

class FLVVideoFrame
{
public:
	uint16_t frameType;
	uint32_t dataSize;
	uint64_t dataPosition;

	/// in milliseconds 
	uint32_t timestamp;

	/// Return true if this video frame is a key frame
	bool isKeyFrame() const
	{
		return frameType == KEY_FRAME;
	}

};

class FLVAudioFrame
{
public:
	uint32_t dataSize;
	uint64_t dataPosition;

	/// in milliseconds 
	uint32_t timestamp;

};

/// \brief
/// The FLVParser class parses an FLV stream, buffers information about 
/// audio/video frames and provides cursor-based access to them.
//
/// Cursor-based access allow seeking as close as possible to a specified time
/// and fetching frames from there on, sequentially.
/// See seek(), nextVideoFrame(), nextAudioFrame() and nextMediaFrame().
///
/// Input is received from a tu_file object.
///
class DSOEXPORT FLVParser : public MediaParser
{

public:

	/// \brief
	/// Create an FLV parser reading input from
	/// the given tu_file
	//
	/// @param stream
	/// 	tu_file to use for input.
	/// 	Ownership left to the caller.
	///
	FLVParser(boost::shared_ptr<tu_file> stream);

	/// Kills the parser...
	~FLVParser();

	/// Return next media frame
	//
	/// Locks the _mutex
	///
	MediaFrame* parseMediaFrame();

	/// \brief
	/// Returns the next audio frame in the parsed buffer.
	//
	/// If no frame has been played before the first frame is returned.
	/// If there is no more frames in the parsed buffer NULL is returned,
	/// you can check with parsingCompleted() to know wheter this is due to 
	/// EOF reached.
	///
	/// Locks the _mutex
	///
	MediaFrame* nextAudioFrame();

	/// \brief
	/// Returns the next video frame in the parsed buffer.
	//
	/// If no frame has been played before the first frame is returned.
	/// If there is no more frames in the parsed buffer NULL is returned.
	/// you can check with parsingCompleted() to know wheter this is due to 
	/// EOF reached.
	///
	/// Locks the _mutex
	///
	MediaFrame* nextVideoFrame();

	/// Return true of parsing is completed
	//
	/// If this function returns true, any call to nextVideoFrame() or nextAudioFrame
	/// will always return NULL
	///
	bool parsingCompleted() const { return _parsingComplete; }

	/// Returns a VideoInfo class about the videostream
	//
	/// Locks the _mutex
	///
	std::auto_ptr<VideoInfo> getVideoInfo();

	/// Returns a AudioInfo class about the audiostream
	//
	/// Locks the _mutex
	///
	std::auto_ptr<AudioInfo> getAudioInfo();

	/// \brief
	/// Asks if a frame with with a timestamp larger than
	/// the given time is available.
	//
	/// If such a frame is not
	/// available in list of already the parsed frames, we
	/// parse some more. This is used to check how much is buffered.
	///
	/// Locks the _mutex
	///
	/// @param time
	///	Timestamp, in milliseconds.
	///
	bool isTimeLoaded(uint32_t time);

	/// \brief
	/// Seeks to the closest possible position the given position,
	/// and returns the new position.
	//
	/// Locks the _mutex
	///
	uint32_t seek(uint32_t);

	/// Returns the framedelay from the last to the current
	/// audioframe in milliseconds. This is used for framerate.
	//
	/// Locks the _mutex
	///
	uint32_t audioFrameDelay();

	/// \brief
	/// Returns the framedelay from the last to the current
	/// videoframe in milliseconds. 
	//
	/// Locks the _mutex
	///
	uint32_t videoFrameDelay();

	/// Returns the framerate of the video
	//
	/// Locks the _mutex
	///
	uint16_t videoFrameRate();

	/// Returns the "bufferlength", meaning the differens between the
	/// current frames timestamp and the timestamp of the last parseable
	/// frame. Returns the difference in milliseconds.
	//
	/// Locks the _mutex
	///
	uint32_t getBufferLength();

	/// Setup the parser
	//
	/// @return whether we'll be able to parse the file.
	bool setupParser() { return true; }

	uint32_t getLastParsedPos() { return _lastParsedPosition; }

private:

	/// seeks to the closest possible position the given position,
	/// and returns the new position.
	uint32_t seekAudio(uint32_t time);

	/// seeks to the closest possible position the given position,
	/// and returns the new position.
	uint32_t seekVideo(uint32_t time);


	/// Parses next frame from the file, returns true if a frame
	/// was succesfully parsed, or false if not enough data was present.
	bool parseNextFrame();

	/// Parses the header of the file
	bool parseHeader();

	// Functions used to extract numbers from the file
	inline uint32_t getUInt24(uint8_t* in);

	/// The interface to the file, externally owned
//	tu_file* _stream;

	typedef std::vector<FLVVideoFrame*> VideoFrames;

	/// list of videoframes, does no contain the frame data.
	VideoFrames _videoFrames;

	typedef std::vector<FLVAudioFrame*> AudioFrames;

	/// list of audioframes, does no contain the frame data.
	AudioFrames _audioFrames;

	/// The position where the parsing should continue from.
	uint32_t _lastParsedPosition;

	/// Whether the parsing is complete or not
	bool _parsingComplete;

	/// Info about the video stream
	std::auto_ptr<VideoInfo> _videoInfo;

	/// Info about the audio stream
	std::auto_ptr<AudioInfo> _audioInfo;

	/// Last audio frame returned
	size_t _nextAudioFrame;

	/// Last video frame returned
	size_t _nextVideoFrame;

	/// Audio stream is present
	bool _audio;

	/// Audio stream is present
	bool _video;

	/// Mutex to avoid problems with threads using the parser
	boost::mutex _mutex;
};

} // gnash.media namespace 
} // end of gnash namespace

#endif // __FLVPARSER_H__
