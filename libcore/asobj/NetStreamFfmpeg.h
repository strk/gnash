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


#ifndef GNASH_NETSTREAMFFMPEG_H
#define GNASH_NETSTREAMFFMPEG_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifdef USE_FFMPEG

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#include "impl.h" // what for ? drop ?
#include "VideoDecoder.h" // for visibility of dtor
#include "AudioDecoder.h" // for visibility of dtor

#include "image.h"
#include "StreamProvider.h"	
#include "NetStream.h" // for inheritance
#include "VirtualClock.h"

// TODO: drop ffmpeg-specific stuff
#include "ffmpegNetStreamUtil.h"


#include <queue>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp> 
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/barrier.hpp>

#include <memory>
#include <cassert>

// Forward declarations
namespace gnash {
	class IOChannel;
	namespace media {
		class sound_handler;
		class MediaHandler;
	}
}

namespace gnash {
  

class NetStreamFfmpeg: public NetStream {
public:

	typedef std::deque<media::raw_mediadata_t*> AudioQueue;

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
	//
	void seek(boost::uint32_t pos);

	// See dox in NetStream.h
	boost::int32_t time();

	// See dox in NetStream.h
	void advance();

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
		DEC_BUFFERING
	};

	/// Gets video info from the parser and initializes _videoDecoder
	//
	/// @param parser the parser to use to get video information.
	///
	void initVideoDecoder(media::MediaParser& parser);

	/// Gets audio info from the parser and initializes _audioDecoder
	//
	/// @param parser the parser to use to get audio information.
	///
	void initAudioDecoder(media::MediaParser& parser);

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

	/// Decode next video frame fetching it MediaParser cursor
	//
	/// @return 0 on EOF or error, a decoded video otherwise
	///
	std::auto_ptr<image::ImageBase> decodeNextVideoFrame();

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
	std::auto_ptr<image::ImageBase> getDecodedVideoFrame(boost::uint32_t ts);

	// Used to calculate a decimal value from a ffmpeg fraction
	inline double as_double(AVRational time)
	{
		return time.num / (double) time.den;
	}

	DecodingState decodingStatus(DecodingState newstate = DEC_NONE);

	/// Video decoder
	std::auto_ptr<media::VideoDecoder> _videoDecoder;

	/// Audio decoder
	std::auto_ptr<media::AudioDecoder> _audioDecoder;

	/// Virtual clock used as playback clock source
	std::auto_ptr<InterruptableVirtualClock> _playbackClock;

	/// Playback control device 
	PlayHead _playHead;

	// Current sound handler
	media::sound_handler* _soundHandler;

	// Current media handler
	media::MediaHandler* _mediaHandler;

	/// Parse a chunk of input
	/// Currently blocks, ideally should parse as much
	/// as possible w/out blocking
	void parseNextChunk();

	/// Input stream
	//
	/// This should just be a temporary variable, transferred
	/// to MediaParser constructor.
	///
	std::auto_ptr<IOChannel> _inputStream;

	/// This is where audio frames are pushed by ::advance
	/// and consumed by sound_handler callback (audio_streamer)
	AudioQueue _audioQueue;

	/// Number of bytes in the audio queue, protected by _audioQueueMutex
	size_t _audioQueueSize;

	/// The queue needs to be protected as sound_handler callback
	/// is invoked by a separate thread (dunno if it makes sense actually)
	boost::mutex _audioQueueMutex;

};


} // gnash namespace


#endif // USE_FFMPEG

#endif //  __NETSTREAMFFMPEG_H__
