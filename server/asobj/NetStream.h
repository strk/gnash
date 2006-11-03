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

// 
//
//

/*  $Id: NetStream.h,v 1.12 2006/11/03 15:47:34 nihilus Exp $ */

#ifndef __NETSTREAM_H__
#define __NETSTREAM_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <queue>
#include <pthread.h>
#include "impl.h"
#include "video_stream_instance.h"
#ifdef USE_FFMPEG
#include <ffmpeg/avformat.h>
#endif

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
		if (m_data)
		{
			delete m_data;
		}
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
			pthread_mutex_init(&m_mutex, NULL);
		};

    ~multithread_queue()
		{
			while (m_queue.size() > 0)
			{
				T x = m_queue.front();
				m_queue.pop();
				delete x;
			}

			pthread_mutex_destroy(&m_mutex);
		}

		size_t size()
		{
			lock();
			size_t n = m_queue.size();
			unlock();
			return n;
		}

		bool push(T member)
		{
			bool rc = false;
			lock();
			if (m_queue.size() < 40)	// hack
			{
				m_queue.push(member);
				rc = true;
			}
			unlock();
			return rc;
		}

		T front()
		{
			lock();
			T member = NULL;
			if (m_queue.size() > 0)
			{
				member = m_queue.front();
			}
			unlock();
			return member;
		}

		void pop()
		{
			lock();
			if (m_queue.size() > 0)
			{
				m_queue.pop();
			}
			unlock();
		}

	private:

		inline void lock()
		{
			pthread_mutex_lock(&m_mutex);
		}

		inline void unlock()
		{
			pthread_mutex_unlock(&m_mutex);
		}

		pthread_mutex_t m_mutex;
		std::queue < T > m_queue;
};

class NetStream {
public:
    NetStream();
    ~NetStream();
   void close();
   void pause();
   int play(const char* source);
   void seek();
   void setBufferTime();

	 raw_videodata_t* read_frame(raw_videodata_t* vd);
	 YUV_video* get_video();

	 inline bool playing()
	 {
		 return m_go;
	 }

#ifdef USE_FFMPEG
	inline double as_double(AVRational time)
	{
		return time.num / (double) time.den;
	}
#endif
	 static void* av_streamer(void* arg);
	 static void audio_streamer(void *udata, uint8 *stream, int len);

private:
    bool _bufferLength;
    bool _bufferTime;
    bool _bytesLoaded;
    bool _bytesTotal;
    bool _currentFps;
    bool _onStatus;
    bool _time;
#ifdef USE_FFMPEG
		AVFormatContext *m_FormatCtx;

		// video
		AVCodecContext* m_VCodecCtx;
		AVStream* m_video_stream;

		// audio
		AVCodecContext *m_ACodecCtx;
		AVStream* m_audio_stream;

		AVFrame* m_Frame;
#endif
		double m_video_clock;
		YUV_video* m_yuv;

		pthread_t m_thread;
		int m_video_index;
		int m_audio_index;
		multithread_queue <raw_videodata_t*> m_qaudio;
		multithread_queue <raw_videodata_t*> m_qvideo;
		volatile bool m_go;
};

class netstream_as_object : public as_object
{
public:
    NetStream obj;
};

void netstream_new(const fn_call& fn);
void netstream_close(const fn_call& fn);
void netstream_pause(const fn_call& fn);
void netstream_play(const fn_call& fn);
void netstream_seek(const fn_call& fn);
void netstream_setbuffertime(const fn_call& fn);

} // end of gnash namespace

// __NETSTREAM_H__
#endif

