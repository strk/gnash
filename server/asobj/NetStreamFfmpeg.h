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
#include <boost/thread/barrier.hpp>

#include <memory>
#include <cassert>

#include "impl.h"

#ifdef HAVE_FFMPEG_AVFORMAT_H
extern "C" {
#include <ffmpeg/avformat.h>
}
#endif

#ifdef HAVE_LIBAVFORMAT_AVFORMAT_H
extern "C" {
#include <libavformat/avformat.h>
}
#endif

#include "image.h"
#include "StreamProvider.h"	
#include "NetStream.h" // for inheritance
#include "VirtualClock.h"

#include "ffmpegNetStreamUtil.h"

/// Uncomment the following to load media in a separate thread
//#define LOAD_MEDIA_IN_A_SEPARATE_THREAD

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

#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	/// The parsing thread. Sets up the decoder, and decodes.
	static void parseAllInput(NetStreamFfmpeg* ns);
#endif

	/// Callback used by sound_handler to get audio data
	//
	/// This is a sound_handler::aux_streamer_ptr type.
	///
	/// It will be invoked by a separate thread (neither main, nor decoder thread).
	///
	static bool audio_streamer(void *udata, boost::uint8_t *stream, int len);

	long bytesLoaded();

	long bytesTotal();

	long bufferLength();
private:

	enum PlaybackState {
		PLAY_NONE,
		PLAY_STOPPED,
		PLAY_PLAYING,
		PLAY_PAUSED
	};

	enum DecodingState {
		DEC_NONE,
		DEC_STOPPED,
		DEC_DECODING,
		DEC_BUFFERING,
	};

	DecodingState _decoding_state;

	// Mutex protecting _playback_state and _decoding_state
	// (not sure a single one is appropriate)
	boost::mutex _state_mutex;

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
	/// @param alsoIfPaused
	///	If true, video is consumed/refreshed even if playhead is paused.
	///	By default this is false, but will be used on ::seek (user-reguested)
	///
	void refreshVideoFrame(bool alsoIfPaused=false);

	/// Refill audio buffers, so to contain new frames since last run
	/// and up to current timestamp
	void refreshAudioBuffer();

	// Used to decode and push the next available (non-FLV) frame to the audio or video queue
	bool decodeMediaFrame();

	/// Used to push decoded version of next available FLV frame to the audio or video queue
	//
	/// Called by ::av_streamer to buffer more a/v frames when possible.
	///
	/// Will call decodeVideo or decodeAudio depending on frame type, and return
	/// what they return.
	/// Will set decodingStatus to DEC_BUFFERING when starving on input
	///
	/// This is a blocking call.
	//
	/// @returns :
	/// 	If next frame is video and:
	///		- we have no video decoding context
	///		- or there is a decoding error
	///		- or there is a conversion error
	///		- or renderer requested format is NONE
	/// 	... false will be returned.
	/// 	In any other case, true is returned.
	///
	/// NOTE: if EOF is reached, true is returned by decodingStatus is set to DEC_STOPPED
	///
	/// NOTE: (FIXME) if we succeeded decoding but the relative queue was full,
	///       true will be returned but nothing would be pushed on the queues.
	/// 
	/// TODO: return a more informative value to tell what happened.
	/// TODO: make it simpler !
	///
	bool decodeFLVFrame();

	/// Decode next video frame fetching it MediaParser cursor
	//
	/// @return 0 on EOF or error, a decoded video otherwise
	///
	media::raw_mediadata_t* decodeNextVideoFrame();

	/// Decode next audio frame fetching it MediaParser cursor
	//
	/// @return 0 on EOF or error, a decoded audio frame otherwise
	///
	media::raw_mediadata_t* decodeNextAudioFrame();

	/// \brief
	/// Decode input audio frames with timestamp <= ts
	/// and push them to the output audio queue
	void pushDecodedAudioFrames(boost::uint32_t ts);

	/// Decode input frames up to the one with timestamp <= ts.
	//
	/// Decoding starts from "next" element in the parser cursor.
	///
	/// Return 0 if:
	///	1. there's no parser active.
	///	2. parser cursor is already on last frame.
	///	3. next element in cursor has timestamp > tx
	///	4. there was an error decoding
	///
	media::raw_mediadata_t* getDecodedVideoFrame(boost::uint32_t ts);

	/// Used to decode a video frame 
	//
	/// This is a blocking call.
	/// If no Video decoding context exists (m_VCodecCtx), 0 is returned.
	/// On decoding (or converting) error, 0 is returned.
	/// If renderer requested video format is render::NONE, 0 is returned.
	/// In any other case, a decoded video frame is returned.
	///
	/// TODO: return a more informative value to tell what happened.
	///
	media::raw_mediadata_t* decodeVideo( AVPacket* packet );

	/// Used to decode an audio frame 
	//
	/// This is a blocking call.
	/// If no Video decoding context exists (m_ACodecCtx), 0 is returned.
	/// In any other case, a decoded audio frame is returned.
	///
	/// TODO: return a more informative value to tell what happened.
	///
	media::raw_mediadata_t* decodeAudio( AVPacket* packet );

	// Used to calculate a decimal value from a ffmpeg fraction
	inline double as_double(AVRational time)
	{
		return time.num / (double) time.den;
	}

	DecodingState decodingStatus(DecodingState newstate = DEC_NONE);

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

#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	/// The parser thread
	boost::thread* _parserThread;

	/// Barrier to synchronize thread and thread starter
	boost::barrier _parserThreadBarrier;

	/// Mutex serializing access to parser,
	/// when reading from a separate thread
	boost::mutex _parserMutex;

	/// Kill decoder thread, if any
	//
	/// POSTCONDITIONS:
	/// 	_decodeThread is NULL
	/// 	decoder thread is not running
	///
	/// Uses the _qMutex
	///
	void killParserThread();

	/// Return true if kill of parser thread was requested
	bool parserThreadKillRequested();

	/// Protected by _parserKillRequestMutex
	bool _parserKillRequest;

	/// Mutex protecting _parserKillRequest
	boost::mutex _parserKillRequestMutex;

#endif // LOAD_MEDIA_IN_A_SEPARATE_THREAD


	// The timestamp of the last decoded video frame, in seconds.
	volatile boost::uint32_t m_last_video_timestamp;

	// The timestamp of the last decoded audio frame, in seconds.
	volatile boost::uint32_t m_last_audio_timestamp;

	/// Queues filler will wait on this condition when queues are full
	boost::condition _qFillerResume;

	/// Virtual clock used as playback clock source
	std::auto_ptr<InterruptableVirtualClock> _playbackClock;

	/// Playback control device 
	PlayHead _playHead;

	// When the queues are full, this is where we keep the audio/video frame
	// there wasn't room for on its queue
	media::raw_mediadata_t* m_unqueued_data;

	ByteIOContext ByteIOCxt;

	// Decoder buffer
	boost::uint8_t* _decoderBuffer;

	// Current sound handler
	media::sound_handler* _soundHandler;

	/// Parse a chunk of input
	/// Currently blocks, ideally should parse as much
	/// as possible w/out blocking
	void parseNextChunk();

	/// Input stream
	//
	/// This should just be a temporary variable, transferred
	/// to MediaParser constructor.
	///
	std::auto_ptr<tu_file> _inputStream;

        typedef std::deque<media::raw_mediadata_t*> AudioQueue;

	/// This is where audio frames are pushed by ::advance
	/// and consumed by sound_handler callback (audio_streamer)
        AudioQueue _audioQueue;

	/// The queue needs to be protected as sound_handler callback
	/// is invoked by a separate thread (dunno if it makes sense actually)
	boost::mutex _audioQueueMutex;

};


} // gnash namespace


#endif // USE_FFMPEG

#endif //  __NETSTREAMFFMPEG_H__
