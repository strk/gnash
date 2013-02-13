// MediaParserFfmpeg.h: FFMEPG media parsers, for Gnash
// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#ifndef GNASH_MEDIAPARSER_FFMPEG_H
#define GNASH_MEDIAPARSER_FFMPEG_H

#include <boost/scoped_array.hpp>
#include <memory>
#include <boost/optional.hpp>

#include "MediaParser.h" // for inheritance
#include "ffmpegHeaders.h"
#include "Id3Info.h"

// Forward declaration
namespace gnash {
	class IOChannel;
}

namespace gnash {
namespace media {
namespace ffmpeg {

/// Extra info found in audio stream by the parser.
//
/// The info will be needed for proper initialization of decoder.
///
class ExtraAudioInfoFfmpeg : public AudioInfo::ExtraInfo
{
public:
	ExtraAudioInfoFfmpeg(boost::uint8_t* nData, size_t nDataSize)
		:
		data(nData),
		dataSize(nDataSize)
	{
	}
	boost::uint8_t* data;
	size_t dataSize;
};

/// Extra info found in video stream by the parser.
//
/// The info will be needed for proper initialization of decoder.
///
class ExtraVideoInfoFfmpeg : public VideoInfo::ExtraInfo
{
public:
	ExtraVideoInfoFfmpeg(boost::uint8_t* nData, size_t nDataSize)
		:
		data(nData),
		dataSize(nDataSize)
	{
	}
	boost::uint8_t* data;
	size_t dataSize;
};

/// FFMPEG based MediaParser
class MediaParserFfmpeg: public MediaParser
{
public:

	/// Construct a ffmpeg-based media parser for given stream
	//
	/// Can throw a GnashException if input format couldn't be detected
	///
	MediaParserFfmpeg(std::auto_ptr<IOChannel> stream);

	~MediaParserFfmpeg();

	// See dox in MediaParser.h
	virtual bool seek(boost::uint32_t&);

	// See dox in MediaParser.h
	virtual bool parseNextChunk();

	// See dox in MediaParser.h
	virtual boost::uint64_t getBytesLoaded() const;

    virtual boost::optional<Id3Info> getId3Info() const;

private:

	/// Initialize parser, figuring format and 
	/// creating VideoInfo and AudioInfo objects
	void initializeParser();

	/// Video frame cursor position 
	//
	/// This is the video frame number that will
	/// be referenced by nextVideoFrame and nextVideoFrameTimestamp
	///
	size_t _nextVideoFrame;

	/// Audio frame cursor position 
	//
	/// This is the video frame number that will
	/// be referenced by nextVideoFrame and nextVideoFrameTimestamp
	///
	size_t _nextAudioFrame;

	/// Parse next media frame
	//
	/// @return false on error or eof, true otherwise
	///
	bool parseNextFrame();

	/// Input chunk reader, to be called by ffmpeg parser
	int readPacket(boost::uint8_t* buf, int buf_size);

	/// ffmpeg callback function
	static int readPacketWrapper(void* opaque, boost::uint8_t* buf, int buf_size);

	/// Input stream seeker, to be called by ffmpeg parser
	boost::int64_t seekMedia(boost::int64_t offset, int whence);

	/// ffmpeg callback function
	static boost::int64_t seekMediaWrapper(void *opaque, boost::int64_t offset, int whence);

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
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(52,107,0)
// AVIOContext was introduced a bit earlier but without version bump, so let's
// be safe
        ByteIOContext _byteIOCxt;
#else
        AVIOContext* _avIOCxt;
#endif

	/// Size of the ByteIO context buffer
	//
	/// This seems to be the size of chunks read
	/// by av_read_frame.
	///
	static const size_t byteIOBufferSize = 1024;

	boost::scoped_array<unsigned char> _byteIOBuffer;

	/// The last parsed position, for getBytesLoaded
	boost::uint64_t _lastParsedPosition;

	/// Return sample size from SampleFormat
	//
	/// TODO: move somewhere in ffmpeg utils..
	///
	boost::uint16_t SampleFormatToSampleSize(AVSampleFormat fmt);

	/// Make an EncodedVideoFrame from an AVPacket and push to buffer
	//
	bool parseVideoFrame(AVPacket& packet);

	/// Make an EncodedAudioFrame from an AVPacket and push to buffer
	bool parseAudioFrame(AVPacket& packet);

    boost::optional<Id3Info> _id3Object;
};


} // gnash.media.ffmpeg namespace 
} // gnash.media namespace 
} // namespace gnash

#endif // __MEDIAPARSER_FFMPEG_H__
