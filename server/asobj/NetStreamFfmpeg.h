// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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


#ifndef __NETSTREAMFFMPEG_H__
#define __NETSTREAMFFMPEG_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifdef USE_FFMPEG

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#include <queue>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp> 
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

#include "impl.h"

extern "C" {
#include <ffmpeg/avformat.h>
}

#include "image.h"
#include "StreamProvider.h"	
#include "NetStream.h" // for inheritance

#include "ffmpegNetStreamUtil.h"

namespace gnash {
  

class NetStreamFfmpeg: public NetStream {
public:
	NetStreamFfmpeg();
	~NetStreamFfmpeg();

	// See dox in NetStream.h
	void close();

	// See dox in NetStream.h
	void pause( PauseMode mode );

	// See dox in NetStream.h
	void play(const std::string& source);

	// See dox in NetStream.h
	//
	// Users:
	//	- ::advance (VM), itself locking
	//	- ::startPlayback() non locking but called by av_streamer which locks
	//	- ::seekMedia() set as a callback with init_put_byte (??)
	//
	void seek(boost::uint32_t pos);

	// See dox in NetStream.h
	boost::int32_t time();

	// See dox in NetStream.h
	void advance();

	// Used for ffmpeg data read and seek callbacks with non-FLV
	static int readPacket(void* opaque, boost::uint8_t* buf, int buf_size);

	// Used for ffmpeg data read and seek callbacks with non-FLV
	static offset_t seekMedia(void *opaque, offset_t offset, int whence);

	/// The decoding thread. Sets up the decoder, and decodes.
	static void av_streamer(NetStreamFfmpeg* ns);

	/// Callback used by sound_handler to get audio data
	//
	/// This is a sound_handler::aux_streamer_ptr type.
	///
	/// It will be invoked by a separate thread (neither main, nor decoder thread).
	///
	static bool audio_streamer(void *udata, boost::uint8_t *stream, int len);

	long bytesLoaded();

	long bytesTotal();

private:

	// Setups the playback
	bool startPlayback();

	// Pauses the playhead 
	//
	// Users:
	// 	- ::decodeFLVFrame() 
	// 	- ::pause() 
	// 	- ::play() 
	//
	void pausePlayback();

	// Resumes the playback 
	//
	// Users:
	// 	- ::av_streamer() 
	// 	- ::play() 
	// 	- ::startPlayback() 
	// 	- ::advance() 
	//
	void unpausePlayback();

	/// Update the image/videoframe to be returned by next get_video() call.
	//
	/// Uses by ::advance().
	///
	/// Note that get_video will be called by video_stream_instance::display, which
	/// is usually called right after video_stream_instance::advance, so  the result
	/// is that  refreshVideoFrame() is called right before get_video(). This is important
	/// to ensure timing is correct..
	///
	void refreshVideoFrame();

	// Used to decode and push the next available (non-FLV) frame to the audio or video queue
	bool decodeMediaFrame();

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
	bool decodeFLVFrame();

	// Used to decode a video frame and push it on the videoqueue
	bool decodeVideo( AVPacket* packet );

	// Used to decode a audio frame and push it on the audioqueue
	bool decodeAudio( AVPacket* packet );

	// Used to calculate a decimal value from a ffmpeg fraction
	inline double as_double(AVRational time)
	{
		return time.num / (double) time.den;
	}

	int m_video_index;
	int m_audio_index;
	
	// video
	AVCodecContext* m_VCodecCtx;
	AVStream* m_video_stream;

	// audio
	AVCodecContext *m_ACodecCtx;
	AVStream* m_audio_stream;

	// the format (mp3, avi, etc.)
	AVFormatContext *m_FormatCtx;

	AVFrame* m_Frame;

	// Use for resampling audio
	media::AudioResampler _resampler;

	// The decoding thread
	boost::thread* _decodeThread;

	// The timestamp of the last decoded video frame, in seconds.
	volatile boost::uint32_t m_last_video_timestamp;

	// The timestamp of the last decoded audio frame, in seconds.
	volatile boost::uint32_t m_last_audio_timestamp;

	// The timestamp of the last played audio (default) or video (if no audio) frame.
	// Misured in seconds.
	boost::uint32_t m_current_timestamp;

	// The queues of audio and video data.
	media::multithread_queue <media::raw_mediadata_t*> m_qaudio;
	media::multithread_queue <media::raw_mediadata_t*> m_qvideo;

	// The time we started playing in seconds (since VM start ?)
	volatile boost::uint64_t m_start_clock;

	// When the queues are full, this is where we keep the audio/video frame
	// there wasn't room for on its queue
	media::raw_mediadata_t* m_unqueued_data;

	ByteIOContext ByteIOCxt;

	// Time of when pause started, in seconds since VM started
	volatile boost::uint64_t m_time_of_pause;

	// Decoder buffer
	boost::uint8_t* _decoderBuffer;

	// Current sound handler
	media::sound_handler* _soundHandler;
};


} // gnash namespace


#endif // USE_FFMPEG

#endif //  __NETSTREAMFFMPEG_H__
