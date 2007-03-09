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

/* $Id: NetStreamFfmpeg.h,v 1.14 2007/03/09 14:38:29 tgc Exp $ */

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

#include "impl.h"
#include "video_stream_instance.h"

#include <ffmpeg/avformat.h>

#include "image.h"
#include "StreamProvider.h"	
#include "NetStream.h" // for inheritance

namespace gnash {
  
struct raw_videodata_t
{
	raw_videodata_t():
	m_stream_index(-1),
	m_size(0),
	m_data(NULL),
	m_ptr(NULL),
	m_pts(0)
	{
	};

	~raw_videodata_t()
	{
		delete [] m_data;
	};

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
			// So.. if there are 20 items in the queue...
			// disregard the next item? WTF?

			if (m_queue.size() < 20)	// hack
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

class NetStreamFfmpeg: public NetStream {
public:
	NetStreamFfmpeg();
	~NetStreamFfmpeg();
	void close();
	void pause(int mode);
	int play(const char* source);
	void seek(double pos);
	void setBufferTime();
	void set_status(const char* code);
	void setNetCon(as_object* nc);
	int64_t time();
	long bytesLoaded();
	long bytesTotal();

	// Used for ffmpeg data read and seek callbacks
	static int readPacket(void* opaque, uint8_t* buf, int buf_size);
	static offset_t seekMedia(void *opaque, offset_t offset, int whence);

	bool read_frame();

	image::image_base* get_video();

	bool playing()
	{
		return m_go;
	}

	inline double as_double(AVRational time)
	{
		return time.num / (double) time.den;
	}

	static void startPlayback(NetStreamFfmpeg* ns);
	static void av_streamer(NetStreamFfmpeg* ns);
	static bool audio_streamer(void *udata, uint8 *stream, int len);

private:

	bool _bufferLength;
	bool _bufferTime;
	bool _bytesLoaded;
	bool _bytesTotal;
	bool _currentFps;
	bool _onStatus;
	bool _time;

	int m_video_index;
	int m_audio_index;
	
	// video
	AVCodecContext* m_VCodecCtx;
	AVStream* m_video_stream;

	// audio
	AVCodecContext *m_ACodecCtx;
	AVStream* m_audio_stream;

	AVFormatContext *m_FormatCtx;

	AVFrame* m_Frame;

	ReSampleContext *m_Resample;

	boost::thread *m_thread;
	boost::thread *startThread;
	boost::mutex decoding_mutex;
	boost::mutex start_mutex;
	boost::mutex::scoped_lock *lock;

	volatile bool m_go;
	unsigned int runtime;

	image::image_base* m_imageframe;

	double m_video_clock;

	multithread_queue <raw_videodata_t*> m_qaudio;
	multithread_queue <raw_videodata_t*> m_qvideo;
	bool m_pause;
	double m_start_clock;
	raw_videodata_t* m_unqueued_data;

	ByteIOContext ByteIOCxt;
	tu_file* input;
	long inputPos;
	StreamProvider streamProvider;
	std::string url;
};

} // gnash namespace

#endif // USE_FFMPEG

#endif //  __NETSTREAMFFMPEG_H__
