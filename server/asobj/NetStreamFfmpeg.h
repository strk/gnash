// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/* $Id: NetStreamFfmpeg.h,v 1.29 2007/05/23 07:42:16 tgc Exp $ */

#ifndef __NETSTREAMFFMPEG_H__
#define __NETSTREAMFFMPEG_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
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

#include <ffmpeg/avformat.h>

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
	uint32_t m_size;
	uint8_t* m_data;
	uint8_t* m_ptr;
	double m_pts;	// presentation timestamp in sec
};

template<class T>
class multithread_queue
{
	public:

	multithread_queue()
		{
		}

	~multithread_queue()
		{
			boost::mutex::scoped_lock lock(_mutex);
			while (m_queue.size() > 0)
			{
				T x = m_queue.front();
				m_queue.pop();
				delete x;
			}
		}

		size_t size()
		{
			boost::mutex::scoped_lock lock(_mutex);
			size_t n = m_queue.size();
			return n;
		}

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

		T front()
		{
			boost::mutex::scoped_lock lock(_mutex);
			T member = NULL;
			if (m_queue.size() > 0)
			{
				member = m_queue.front();
			}
			return member;
		}

		void pop()
		{
			boost::mutex::scoped_lock lock(_mutex);
			if (m_queue.size() > 0)
			{
				m_queue.pop();
			}
		}

	private:

		boost::mutex _mutex;
		std::queue < T > m_queue;
};


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
	
	bool init(AVCodecContext* ctx)
	{
		if (ctx->sample_rate != 44100 && ctx->channels != 2) {
			if (!_context) {
				_context = audio_resample_init(2,  ctx->channels, 
					44100, ctx->sample_rate);
 			}
			return true;
		}
		return false;
	}
	
	int resample(int16_t* input, int16_t* output, int samples)
	{
		return audio_resample (_context, output, input, samples);
	}
	
	// The timestamp of the last decoded video frame
	volatile double m_last_video_timestamp;

private:
	ReSampleContext* _context;
};

class NetStreamFfmpeg: public NetStream {
public:
	NetStreamFfmpeg();
	~NetStreamFfmpeg();
	void close();
	void pause(int mode);
	int play(const std::string& source);
	void seek(double pos);
	int64_t time();
	void advance();

	// Used for ffmpeg data read and seek callbacks
	static int readPacket(void* opaque, uint8_t* buf, int buf_size);
	static offset_t seekMedia(void *opaque, offset_t offset, int whence);

	static void av_streamer(NetStreamFfmpeg* ns);
	static bool audio_streamer(void *udata, uint8_t *stream, int len);

private:

	// Setups the playback
	bool startPlayback();

	// Pauses the decoding - don't directly modify m_pause!!
	void pauseDecoding();

	// Unpauses/resumes the decoding - don't directly modify m_pause!!
	void unpauseDecoding();

	// Check is we need to update the video frame
	void refreshVideoFrame();

	// Used to decode and push the next available (non-FLV) frame to the audio or video queue
	bool decodeMediaFrame();

	// Used to decode push the next available FLV frame to the audio or video queue
	bool decodeFLVFrame();

	// Used to decode a video frame and push it on the videoqueue
	bool decodeVideo(AVPacket* packet);

	// Used to decode a audio frame and push it on the audioqueue
	bool decodeAudio(AVPacket* packet);

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

	// Mutex used to make sure things arn't messed up when seeking and decoding (is it really necessary?)
	boost::mutex decoding_mutex;

	// Mutex and condition used to pause the decoding thread/loop when
	// playback is paused, or when the video and audio queues are full
	boost::mutex decode_wait_mutex;
	boost::condition decode_wait;

	// The timestamp of the last decoded video frame
	volatile double m_last_video_timestamp;

	// The timestamp of the last decoded audio frame
	volatile double m_last_audio_timestamp;

	// The timestamp of the last played audio (default) or video (if no audio) frame
	double m_current_timestamp;

	// The queues of audio and video data.
	multithread_queue <raw_mediadata_t*> m_qaudio;
	multithread_queue <raw_mediadata_t*> m_qvideo;

	// The time we started playing
	volatile double m_start_clock;

	// When the queues are full, this is where we keep the audio/video frame
	// there wasen't room for on its queue
	raw_mediadata_t* m_unqueued_data;

	ByteIOContext ByteIOCxt;

	// Time of when pause started
	double m_time_of_pause;
};

} // gnash namespace

#endif // USE_FFMPEG

#endif //  __NETSTREAMFFMPEG_H__
