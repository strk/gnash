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

#ifndef __MEDIADECODERFFMPEG_H__
#define __MEDIADECODERFFMPEG_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#include "MediaDecoder.h"
#include "MediaParser.h"
#include "AudioDecoder.h"
#include "VideoDecoder.h"

#include "image.h"

namespace gnash {

class MediaDecoderSdl: public MediaDecoder {
public:
	MediaDecoderSdl(tu_file* stream, MediaBuffer* buffer, uint16_t swfVersion, int format);
	~MediaDecoderSdl();

	/// Pause decoding (needed ?)
	void pause();

	/// Resume/start decoding (needed ?)
	void decode();

	/// Seeks to pos
	uint32_t seek(uint32_t pos);

	std::pair<uint32_t, uint32_t> getWidthAndHeight();

/*	// Used for ffmpeg data read and seek callbacks with non-FLV
	static int readPacket(void* opaque, uint8_t* buf, int buf_size);

	// Used for ffmpeg data read and seek callbacks with non-FLV
	static offset_t seekMedia(void *opaque, offset_t offset, int whence);*/

private:
	/// The decoding thread. Sets up the decoder, and decodes.
	static void decodeThread(MediaDecoderSdl* decoder);

	/// Decodes a frame and push it unto the buffer
	void decodeAndBufferFrame();

	// Used to decode a video frame and push it on the videoqueue
	void decodeVideo(MediaFrame* packet);

	// Used to decode a audio frame and push it on the audioqueue
	void decodeAudio(MediaFrame* packet);

	/// Sets up the decoder and parser
	bool setupDecoding();

#if 0
	/// Sets up the decoder and parser for FLVs
	bool setupFLVdecoding();

	// Used to decode and push the next available (non-FLV) frame to the audio or video queue
	void decodeMediaFrame();

	/// Used to decode push the next available FLV frame to the audio or video queue
	//
	/// Called by ::av_streamer to buffer more a/v frames when possible.
	///
	/// This is a non-blocking call, if data isn't available in the parser no
	/// new frame is decoded and this method returns false. Note that this doesn't
	/// necessarely means the FLV stream is ended, rather it is possible the loader
	/// thread is starving. 
	///
	/// TODO: return a more informative value to distinguish between EOF and starving
	///       conditions ?
	///
	void decodeFLVFrame();

	// Used to decode a video frame and push it on the videoqueue
	void decodeVideo(AVPacket* packet);

	// Used to decode a audio frame and push it on the audioqueue
	void decodeAudio(AVPacket* packet);

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

	// A ffmpeg frame
	AVFrame* _frame;
#endif
	// The decoding thread
	boost::thread* _decodeThread;

	// The timestamp of the last decoded video frame, in seconds.
	volatile uint32_t _lastVideoTimestamp;

	// The timestamp of the last decoded audio frame, in seconds.
	volatile uint32_t _lastAudioTimestamp;

	// The time we started playing in seconds (since VM start ?)
	volatile uint64_t _startClock;

	// A ffmpeg thingy
	//ByteIOContext _byteIOCxt;

	// Should the decode loop run or not
	volatile bool _running;
	
	// An audio decoder, either using ffmpeg or mad
	std::auto_ptr<AudioDecoder> _audioDecoder;

	// A video decoder, using ffmpeg
	std::auto_ptr<VideoDecoder> _videoDecoder;
};

} // namespace gnash

#endif // __MEDIADECODERFFMPEG_H__
