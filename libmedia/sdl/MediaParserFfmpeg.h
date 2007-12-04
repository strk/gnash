// MediaParserFfmpeg.h: Media parser using ffmpeg
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

// $Id: MediaParserFfmpeg.h,v 1.7 2007/12/04 11:45:27 strk Exp $

#ifndef __MEDIAPARSERFFMPEG_H__
#define __MEDIAPARSERFFMPEG_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#include "MediaParser.h"
#include "MediaBuffer.h"

extern "C" {
#include <ffmpeg/avformat.h>
}

namespace gnash {
namespace media {



/// \brief
/// The MediaParser class detects the format of the input file, and parses it on demand.
///
class MediaParserFfmpeg : public MediaParser
{
public:
	MediaParserFfmpeg(boost::shared_ptr<tu_file> stream);
	~MediaParserFfmpeg();

	/// Setup the parser
	//
	/// @return whether we'll be able to parse the file.
	bool setupParser();

	/// Used to parse the next media frame in the stream and return it
	MediaFrame* parseMediaFrame();

	/// Try to seek to the given millisecond. Returns the millisecond where the
	/// seek got to.
	boost::uint32_t seek(boost::uint32_t);

	/// Returns a VideoInfo class about the videostream
	//
	/// @return a VideoInfo class about the videostream
	std::auto_ptr<VideoInfo> getVideoInfo();

	/// Returns a AudioInfo class about the audiostream
	//
	/// @return a AudioInfo class about the audiostream
	std::auto_ptr<AudioInfo> getAudioInfo();

	// Used for ffmpeg data read and seek callbacks
	static int readPacket(void* opaque, uint8_t* buf, int buf_size);

	// Used for ffmpeg data read and seek callbacks
	static offset_t seekMedia(void *opaque, offset_t offset, int whence);

	/// Returns the last parsed position in the file in bytes
	boost::uint32_t getLastParsedPos() { return _maxInputPos; }

private:

	// Used to calculate a decimal value from a ffmpeg fraction
	inline double as_double(AVRational time)
	{
		return time.num / (double) time.den;
	}

	int _videoIndex;
	int _audioIndex;
	
	// video
	AVCodecContext* _videoCodecCtx;
	AVStream* _videoStream;

	// audio
	AVCodecContext* _audioCodecCtx;
	AVStream* _audioStream;

	// the format (mp3, avi, etc.)
	AVFormatContext *_formatCtx;

	// A ffmpeg thingy
	ByteIOContext _byteIOCxt;

	// The timestamp of the last parsed video frame, in milliseconds.
	boost::uint32_t _lastVideoTimestamp;

	// The timestamp of the last parsed audio frame, in seconds.
	boost::uint32_t _lastAudioTimestamp;

	// The position of the parserhead.
	boost::uint32_t _inputPos;

	// The max value inputPos ever had.
	boost::uint32_t _maxInputPos;
};



} // gnash.media namespace 
} // namespace gnash

#endif // __MEDIAPARSERFFMPEG_H__
