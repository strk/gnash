// MediaParserFfmpeg.h: FFMPG media parsers, for Gnash
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


#ifndef __MEDIAPARSER_FFMPEG_H__
#define __MEDIAPARSER_FFMPEG_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "MediaParser.h" // for inheritance

#include <vector>
#include <boost/scoped_array.hpp>
#include <memory>

#ifdef HAVE_FFMPEG_AVFORMAT_H
extern "C" {
#include <ffmpeg/avformat.h>
}
#endif

// Forward declaration
class tu_file;

namespace gnash {
namespace media {

/// FFMPEG based MediaParser
///
class MediaParserFfmpeg: public MediaParser
{
public:

	/// Construct a ffmpeg-based media parser for given stream
	//
	/// Can throw a GnashException if input format couldn't be detected
	///
	MediaParserFfmpeg(std::auto_ptr<tu_file> stream);

	~MediaParserFfmpeg();

	// See dox in MediaParser.h
	virtual boost::uint32_t getBufferLength();

	// See dox in MediaParser.h
	virtual bool nextVideoFrameTimestamp(boost::uint64_t& ts);

	// See dox in MediaParser.h
	virtual std::auto_ptr<EncodedVideoFrame> nextVideoFrame();

	// See dox in MediaParser.h
	virtual bool nextAudioFrameTimestamp(boost::uint64_t& ts);

	// See dox in MediaParser.h
	virtual std::auto_ptr<EncodedAudioFrame> nextAudioFrame();

	// See dox in MediaParser.h
	virtual VideoInfo* getVideoInfo();

	// See dox in MediaParser.h
	virtual AudioInfo* getAudioInfo();

	// See dox in MediaParser.h
	virtual boost::uint32_t seek(boost::uint32_t);

	// See dox in MediaParser.h
	virtual bool parseNextChunk();

	// See dox in MediaParser.h
	virtual boost::uint64_t getBytesLoaded() const;

private:

	/// Information about an FFMPEG Video Frame
	class VideoFrameInfo
	{
	public:

		VideoFrameInfo()
			:
			isKeyFrame(false),
			dataSize(0),
			dataPosition(0),
			timestamp(0)
		{}

		/// Type of this frame 
		bool isKeyFrame;

		/// Size of the frame in bytes (needed?)
		boost::uint32_t dataSize;

		/// Start of frame data in stream
		boost::uint64_t dataPosition;

		/// Timestamp in milliseconds 
		boost::uint32_t timestamp;

	};

	/// Information about an FFMPEG Audio Frame
	class AudioFrameInfo
	{
	public:

		AudioFrameInfo()
			:
			dataSize(0),
			dataPosition(0),
			timestamp(0)
		{}

		/// Size of the frame in bytes (needed?)
		boost::uint32_t dataSize;

		/// Start of frame data in stream
		boost::uint64_t dataPosition;

		/// Timestamp in milliseconds 
		boost::uint32_t timestamp;

	};

	// NOTE: VideoFrameInfo is a relatively small structure,
	//       chances are keeping by value here would reduce
	//       memory fragmentation with no big cost
	typedef std::vector<VideoFrameInfo*> VideoFrames;

	/// list of videoframes, does no contain the frame data.
	//
	/// Elements owned by this class.
	///
	VideoFrames _videoFrames;

	/// Video frame cursor position 
	//
	/// This is the video frame number that will
	/// be referenced by nextVideoFrame and nextVideoFrameTimestamp
	///
	size_t _nextVideoFrame;

	/// Info about the video stream (if any)
	std::auto_ptr<VideoInfo> _videoInfo;

	// NOTE: AudioFrameInfo is a relatively small structure,
	//       chances are keeping by value here would reduce
	//       memory fragmentation with no big cost
	typedef std::vector<AudioFrameInfo*> AudioFrames;

	/// list of audioframes, does no contain the frame data.
	//
	/// Elements owned by this class.
	AudioFrames _audioFrames;

	/// Audio frame cursor position 
	//
	/// This is the video frame number that will
	/// be referenced by nextVideoFrame and nextVideoFrameTimestamp
	///
	size_t _nextAudioFrame;

	/// Info about the audio stream (if any)
	std::auto_ptr<AudioInfo> _audioInfo;

	/// Parse next media frame
	//
	/// @return false on error or eof, true otherwise
	///
	bool parseNextFrame();

	/// Parse a video frame
	//
	/// Basically create a VideoFrameInfo out of the AVPacket and push
	/// it on the container.
	///
	/// @return false on error 
	///
	bool parseVideoFrame(AVPacket& packet);

	/// Parse an audio frame
	//
	/// Basically create a AudioFrameInfo out of the AVPacket and push
	/// it on the container.
	///
	/// @return false on error 
	///
	bool parseAudioFrame(AVPacket& packet);

	/// Input chunk reader, to be called by ffmpeg parser
	int readPacket(boost::uint8_t* buf, int buf_size);

	/// ffmpeg callback function
	static int readPacketWrapper(void* opaque, boost::uint8_t* buf, int buf_size);

	/// Input stream seeker, to be called by ffmpeg parser
	offset_t seekMedia(offset_t offset, int whence);

	/// ffmpeg callback function
	static offset_t seekMediaWrapper(void *opaque, offset_t offset, int whence);

	/// Read some of the input to figure an AVInputFormat
	AVInputFormat* probeStream();

	AVInputFormat* _inputFmt;

	/// the format (mp3, avi, etc.)
	AVFormatContext *_formatCtx;

	/// Index of the video stream in input
	int _videoStreamIndex;

	/// Video input stream
	AVStream* _videoStream;

	/// Index of the audio stream in input
	int _audioStreamIndex;

	// audio
	AVStream* _audioStream;

	/// ?
	ByteIOContext ByteIOCxt;

	/// Size of the ByteIO context buffer
	//
	/// This seems to be the size of chunks read
	/// by av_read_frame.
	///
	static const size_t byteIOBufferSize = 1024; // 500000;

	boost::scoped_array<unsigned char> _byteIOBuffer;

	/// The last parsed position, for getBytesLoaded
	boost::uint64_t _lastParsedPosition;

	/// Return sample size from SampleFormat
	//
	/// TODO: move somewhere in ffmpeg utils..
	///
	boost::uint16_t SampleFormatToSampleSize(SampleFormat fmt);

};


} // gnash.media namespace 
} // namespace gnash

#endif // __MEDIAPARSER_FFMPEG_H__
