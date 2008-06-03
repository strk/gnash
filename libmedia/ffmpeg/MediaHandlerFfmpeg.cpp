// MediaHandlerFfmpeg.cpp: FFMPEG media handler, for Gnash
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


#include "MediaHandlerFfmpeg.h"
#include "VideoDecoderFfmpeg.h"
#include "AudioDecoderFfmpeg.h"

#include "tu_file.h" // for visibility of destructor
#include "MediaParser.h" // for visibility of destructor

namespace gnash { 
namespace media {

std::auto_ptr<MediaParser>
MediaHandlerFfmpeg::createMediaParser(std::auto_ptr<tu_file> stream)
{
	// TODO: support more then just FLV...
	return MediaHandler::createMediaParser(stream);
}

std::auto_ptr<VideoDecoder>
MediaHandlerFfmpeg::createVideoDecoder(VideoInfo& info)
{
	if ( info.type != FLASH )
	{
		log_error("Non-flash video encoding not supported yet by FFMPEG VideoDecoder");
		return std::auto_ptr<VideoDecoder>(0);
	}
	videoCodecType format = static_cast<videoCodecType>(info.codec);
	int width = info.width;
	int height = info.height;
	std::auto_ptr<VideoDecoder> ret( new VideoDecoderFfmpeg(format, width, height) );
	return ret;
}

std::auto_ptr<AudioDecoder>
MediaHandlerFfmpeg::createAudioDecoder(AudioInfo& info)
{
	std::auto_ptr<AudioDecoder> ret( new AudioDecoderFfmpeg() );
	ret->setup(&info);
	return ret;
}

} // gnash.media namespace 
} // gnash namespace
