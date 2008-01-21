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

/* $Id: NetStreamFfmpeg.h,v 1.56 2008/01/21 20:55:57 rsavoye Exp $ */

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
#include "video_stream_instance.h"

extern "C" {
#include <ffmpeg/avformat.h>
}

#include "image.h"
#include "StreamProvider.h"	
#include "NetStream.h" // for inheritance

#include "FLVParser.h"

namespace gnash {
  
class raw_mediadata_t
{
public:
	raw_mediadata_t():
	m_stream_index(-1),
	m_size(0),
	m_data(NULL),
	m_ptr(NULL),
	m_pts(0)
	{
	}

	~raw_mediadata_t()
	{
		if (m_data) delete [] m_data;
	}

	int m_stream_index;
	boost::uint32_t m_size;
	boost::uint8_t* m_data;
	boost::uint8_t* m_ptr;
	boost::uint32_t m_pts;	// presentation timestamp in millisec
};

/// Threadsafe elements-owning queue
//
/// This class is a threadsafe queue, using std:queue and locking.
/// It is used to store decoded audio and video data which are waiting to be "played"
/// Elements of the queue are owned by instances of this class.
///
template<class T>
class multithread_queue
{
	public:

	multithread_queue()
		{
		}

	// Destroy all elements of the queue. Locks.
	~multithread_queue()
	{
		clear();
	}

	// Destroy all elements of the queue. Locks.
	void clear()
	{
		boost::mutex::scoped_lock lock(_mutex);
		while (!m_queue.empty())
		{
			T x = m_queue.front();
			m_queue.pop();
			delete x;
		}
	}

	/// Returns the size if the queue. Locks.
	//
	/// @return the size of the queue
	///
	size_t size()
	{
		boost::mutex::scoped_lock lock(_mutex);
		size_t n = m_queue.size();
		return n;
	}

	/// Pushes an element to the queue. Locks.
	//
	/// @param member
	/// The element to be pushed unto the queue.
	///
	/// @return true if queue isn't full and the element was pushed to the queue,
	/// or false if the queue was full, and the element wasn't push unto it.
	///
	bool push(T member)
	{
		bool rc = false;
		boost::mutex::scoped_lock lock(_mutex);

		// We only keep max 20 items in the queue.
		// If it's "full" the item must wait, see calls
		// to this function in read_frame() to see how it is done.
		if (m_queue.size() < 20)
		{
			m_queue.push(member);
			rc = true;
		}
		return rc;
	}

	/// Returns a pointer to the first element on the queue. Locks.
	//
	/// If no elements are available this function returns NULL.
	///
	/// @return a pointer to the first element on the queue, NULL if queue is empty.
	///
	T front()
	{
		boost::mutex::scoped_lock lock(_mutex);
		T member = NULL;
		if (!m_queue.empty())
		{
			member = m_queue.front();
		}
		return member;
	}

	/// Pops the first element from the queue. Locks.
	//
	/// If no elements are available this function is
	/// a noop. 
	///
	void pop()
	{
		boost::mutex::scoped_lock lock(_mutex);
		if (!m_queue.empty())
		{
			m_queue.pop();
		}
	}

private:

	// Mutex used for locking
	boost::mutex _mutex;

	// The actual queue.
	std::queue < T > m_queue;
};

/// This class is used to provide an easy interface to libavcodecs audio resampler.
///
class AudioResampler
{
public:
	AudioResampler() : _context(NULL) {}
	~AudioResampler()
	{ 
		if(_context) {
			audio_resample_close (_context);
		}
	}
	
	/// Initializes the resampler
	//
	/// @param ctx
	/// The audio format container.
	///
	/// @return true if resampling is needed, if not false
	///
	bool init(AVCodecContext* ctx)
	{
		if (ctx->sample_rate != 44100 || ctx->channels != 2) {
			if (!_context) {
				_context = audio_resample_init(2,  ctx->channels, 
					44100, ctx->sample_rate);
 			}
			return true;
		}
		return false;
	}
	
	/// Resamples audio
	//
	/// @param input
	/// A pointer to the audio data that needs resampling
	///
	/// @param output
	/// A pointer to where the resampled output should be placed
	///
	/// @param samples
	/// Number of samples in the audio
	///
	/// @return the number of samples in the output data.
	///
	int resample(boost::int16_t* input, boost::int16_t* output, int samples)
	{
		return audio_resample (_context, output, input, samples);
	}

private:
	// The container of the resample format information.
	ReSampleContext* _context;
};

class NetStreamFfmpeg: public NetStream {
public:
	NetStreamFfmpeg();
	~NetStreamFfmpeg();

	// See dox in NetStream.h
	void close();

	// See dox in NetStream.h
	void pause(int mode);

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

private:

	/// A C++ wrapper around ffmpeg's AVPacket structure
	//
	/// Used in decodeVideo() and decodeAudio(). 
	//
	/// Use Pkt.get() to access.
	///
	class AvPkt
	{
	public:
	
		/// Constructs and initialize an AVPacket 
		AvPkt ()
		{
			av_init_packet(&_pkt);
		}
		
		/// Properly deinitialize the owned AVPacket 
		~AvPkt ()
		{
			av_free_packet(&_pkt);
		}

		/// @ return AVPacket* owned by this instance
		AVPacket* get ()
		{
			return &_pkt;
	
		}
		
		/// @ return AVPacket* owned by this instance
		AVPacket* operator-> ()
		{
			return &_pkt;
		}

	private:
		AVPacket _pkt;
		AvPkt(const AvPkt&);
		AvPkt& operator= (const AvPkt&);
	};

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
	bool decodeVideo(AvPkt& packet);

	// Used to decode a audio frame and push it on the audioqueue
	bool decodeAudio(AvPkt& packet);

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
	AudioResampler _resampler;

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
	multithread_queue <raw_mediadata_t*> m_qaudio;
	multithread_queue <raw_mediadata_t*> m_qvideo;

	// The time we started playing in seconds (since VM start ?)
	volatile boost::uint64_t m_start_clock;

	// When the queues are full, this is where we keep the audio/video frame
	// there wasn't room for on its queue
	raw_mediadata_t* m_unqueued_data;

	ByteIOContext ByteIOCxt;

	// Time of when pause started, in seconds since VM started
	volatile boost::uint64_t m_time_of_pause;
};

} // gnash namespace

#endif // USE_FFMPEG

#endif //  __NETSTREAMFFMPEG_H__
