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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

/* $Id: NetStream.cpp,v 1.4 2006/10/27 15:36:47 nihilus Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "NetStream.h"
#include "fn_call.h"

#include "streamProvider.h"	
#include "render.h"	
#include "movie_root.h"

#if defined(_WIN32) || defined(WIN32)
	#include <Windows.h>	// for sleep()
#endif

namespace gnash {


namespace globals { // gnash::globals

	/// global StreamProvider
	extern StreamProvider streamProvider;

} // namespace gnash::global


NetStream::NetStream():
	m_video_index(-1),
	m_audio_index(-1),
	m_time_remainder(0),
	m_VCodecCtx(NULL),
	m_ACodecCtx(NULL),
	m_FormatCtx(NULL),
	m_AudioStreams(0),
	m_go(false),
//	m_thread(NULL),
	m_frame_time(0.0f),
	m_yuv(NULL),
	m_Frame(NULL)
{
}

NetStream::~NetStream()
{
	close();
}

void NetStream::close()
{
	// terminate thread
	m_go = false;
//	if (m_thread) SDL_WaitThread(m_thread, NULL);
//	m_thread = NULL;

	sound_handler* s = get_sound_handler();
	if (s)
	{
//		s->stop_streamer();
	}

	if (m_Frame) av_free(m_Frame);
	m_Frame = NULL;

	if (m_VCodecCtx) avcodec_close(m_VCodecCtx);
	m_VCodecCtx = NULL;

	if (m_ACodecCtx) avcodec_close(m_ACodecCtx);
	m_ACodecCtx = NULL;

	if (m_FormatCtx) av_close_input_file(m_FormatCtx);
	m_FormatCtx = NULL;

	render::delete_YUV_video(m_yuv);
	m_yuv = NULL;

	while (m_qvideo.size() > 0)
	{
		delete m_qvideo.front();
		m_qvideo.pop();
	}

	while (m_qaudio.size() > 0)
	{
		delete m_qaudio.front();
		m_qaudio.pop();
	}

}

void
NetStream::play(const char* c_url)
{
#ifdef USE_FFMPEG
/*	URL url(c_url);

	tu_file* in = globals::streamProvider.getStream(url);
	if (in == NULL)
	{
	    log_error("failed to open '%s'; can't create movie.\n", c_url);
	    return;
	}
	else if (in->get_error())
	{
	    log_error("streamProvider opener can't open '%s'\n", c_url);
	    return;
	}
*/

	// This registers all available file formats and codecs 
	// with the library so they will be used automatically when
	// a file with the corresponding format/codec is opened

	if (m_FormatCtx)
	{
		return;
	}

	av_register_all();

	// Open video file
	// The last three parameters specify the file format, buffer size and format parameters;
	// by simply specifying NULL or 0 we ask libavformat to auto-detect the format 
	// and use a default buffer size
	if (av_open_input_file(&m_FormatCtx, c_url, NULL, 0, NULL) != 0)
	{
	  printf("Couldn't open file\n");
		return;
	}

	// Next, we need to retrieve information about the streams contained in the file
	// This fills the streams field of the AVFormatContext with valid information
	if (av_find_stream_info(m_FormatCtx) < 0)
	{
    printf("Couldn't find stream information\n");
		return;
	}
	m_FormatCtx->pb.eof_reached = 0;

	av_read_play(m_FormatCtx);

	// Find the first video & audio stream
	m_video_index = -1;
	m_audio_index = -1;
	for (int i = 0; i < m_FormatCtx->nb_streams; i++)
	{
		AVCodecContext *enc = m_FormatCtx->streams[i]->codec;
		switch (enc->codec_type)
		{
			case CODEC_TYPE_AUDIO:
				if (m_audio_index < 0)
				{
					m_audio_index = i;
				}
				break;

			case CODEC_TYPE_VIDEO:
				if (m_video_index < 0)
				{
					m_video_index = i;
				}
				break;
    }
	}

	if (m_video_index < 0)
	{
		printf("Didn't find a video stream\n");
		return;
	}

	// Get a pointer to the codec context for the video stream
	m_VCodecCtx = m_FormatCtx->streams[m_video_index]->codec;

	// Find the decoder for the video stream
	AVCodec* pCodec = avcodec_find_decoder(m_VCodecCtx->codec_id);
	if (pCodec == NULL)
	{
		printf("Codec not found\n");
		return;
	}

	// Open codec
	if (avcodec_open(m_VCodecCtx, pCodec) < 0)
	{
		printf("Could not open codec\n");
	}

	// Allocate a frame to store the decoded frame in
	m_Frame = avcodec_alloc_frame();
	
	// Determine required buffer size and allocate buffer
	m_yuv = render::create_YUV_video(m_VCodecCtx->width,	m_VCodecCtx->height);
//			avpicture_get_size(PIX_FMT_YUV420P, m_VCodecCtx->width,	m_VCodecCtx->height));

	if (m_audio_index >= 0)
	{

	  //Find the number of audio streams (channels)
    for (int i=0; i < m_FormatCtx->nb_streams; i++)
		{
			if (m_FormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO)
			{
				m_AudioStreams++;
			}
		}

		// Get a pointer to the audio codec context for the video stream
		m_ACodecCtx = m_FormatCtx->streams[m_audio_index]->codec;
    
    // Find the decoder for the audio stream
    AVCodec* pACodec = avcodec_find_decoder(m_ACodecCtx->codec_id);
    if(pACodec == NULL)
		{
      printf("No available AUDIO codec to process MPEG file: %s", "aqwedqw");
		}
        
    // Open codec
    if(avcodec_open(m_ACodecCtx, pACodec) < 0)
		{
			printf("Could not open AUDIO codec");// Could not open codec
		}
	
//		int samplerate = m_ACodecCtx->sample_rate;

		sound_handler* s = get_sound_handler();
		if (s)
		{
//			s->start_streamer(this);
		}
	}

	m_frame_time = (float)m_VCodecCtx->time_base.num / (float)m_VCodecCtx->time_base.den;

//	m_thread = SDL_CreateThread(NetStream::av_streamer, this);
	pthread_create(&m_thread, NULL, NetStream::av_streamer, this);
#endif
}

// decoder thread
void* NetStream::av_streamer(void* arg)
{
#ifdef USE_FFMPEG
	NetStream* ns = static_cast<NetStream*>(arg);
	raw_videodata_t* unqueued_data = NULL;

	ns->m_go = true;
	uint32_t last_ticks = 0;
	while (ns->m_go)
	{
		uint32_t ticks = tu_timer::get_ticks();
		int	delta_ticks = ticks - last_ticks;
		last_ticks = ticks;
		float	delta_t = delta_ticks / 1000.0f;

		unqueued_data = ns->read_frame(unqueued_data);
		ns->advance(delta_t);

		// Don't hog the CPU.
		if (unqueued_data)
		{
			//SDL_Delay(25);	
			Sleep(25);
		}
	}
	ns->m_go = false;

	pthread_cancel(pthread_self());
  pthread_testcancel();

	return 0;
#endif
}

// audio callback is running in sound handler thread
void NetStream::audio_streamer(uint8_t *stream, int len)
{
#ifdef USE_FFMPEG
	while (len > 0 && m_qaudio.size() > 0)
	{
		raw_videodata_t* samples = m_qaudio.front();
		if (len >= samples->m_size)
		{
			int n = samples->m_size;
			memcpy(stream, samples->m_ptr, n);
			stream += n;
			samples->m_ptr += n;
			len -= n;
			samples->m_size -= n;
			m_qaudio.pop();
			delete samples;
		}
		else
		{
			int n = len;
			memcpy(stream, samples->m_ptr, n);
			stream += n;
			samples->m_ptr += n;
			len -= n;
			samples->m_size -= n;
		}
	}
#endif
}

void NetStream::advance(float delta_time)
{
#ifdef USE_FFMPEG
	m_time_remainder += delta_time;
	if (m_time_remainder >= m_frame_time)
	{
		m_time_remainder -= m_frame_time;
		if (m_qvideo.size() > 0)
		{
			raw_videodata_t* video = m_qvideo.front();
			m_yuv->update(video->m_data);
			m_qvideo.pop();
			delete video;
		}
		m_time_remainder = fmod(m_time_remainder, m_frame_time);
	}
#endif
}

#ifdef USE_FFMPEG
raw_videodata_t* NetStream::read_frame(raw_videodata_t* unqueued_data)
{
	raw_videodata_t* ret = NULL;
	if (unqueued_data)
	{
		if (unqueued_data->m_stream_index == m_audio_index)
		{
			sound_handler* s = get_sound_handler();
			if (s)
			{
//				ret = m_qaudio.push(unqueued_data) ? NULL : unqueued_data;
			}
		}
		else
		{
			ret = m_qvideo.push(unqueued_data) ? NULL : unqueued_data;
		}

		return ret;
	}

	AVPacket packet;
	if (av_read_frame(m_FormatCtx, &packet) >= 0)
	{
		if (packet.stream_index == m_audio_index)
		{
			int frame_size;
			uint8_t* ptr = (uint8_t*) malloc((AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2);
		  if (avcodec_decode_audio(m_ACodecCtx, (int16_t *)ptr, &frame_size, packet.data, packet.size) >= 0)
			{
				sound_handler* s = get_sound_handler();
				if (s)
				{
					int16_t*	adjusted_data = 0;
					int	n = 0;

//					bool stereo = pACodecCtx->channels == 1 ? false : true;
//					s->convert_raw_data(&adjusted_data, &n, ptr, frame_size >> 1, 2, m_ACodecCtx->sample_rate, false);

			    raw_videodata_t* samples = new raw_videodata_t;
					samples->m_data = (uint8_t*) adjusted_data;
					samples->m_ptr = samples->m_data;
					samples->m_size = n;
					samples->m_stream_index = m_audio_index;

//					ret = m_qaudio.push(samples) ? NULL : samples;
				}
			}
			free(ptr);
		}
		else
		if (packet.stream_index == m_video_index)
		{
			int got = 0;
		  avcodec_decode_video(m_VCodecCtx, m_Frame, &got, packet.data, packet.size);
			if (got)
			{
				if (m_VCodecCtx->pix_fmt != PIX_FMT_YUV420P)
				{
//				img_convert((AVPicture*) pFrameYUV, PIX_FMT_YUV420P, (AVPicture*) pFrame, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
					assert(0);	// TODO
				}

				raw_videodata_t* video = new raw_videodata_t;
				video->m_data = (uint8_t*) malloc(m_yuv->size());
				video->m_ptr = video->m_data;
				video->m_stream_index = m_video_index;

				int copied = 0;
				uint8_t* ptr = video->m_data;
				for (int i = 0; i < 3 ; i++)
				{
					int shift = (i == 0 ? 0 : 1);
					uint8_t* yuv_factor = m_Frame->data[i];
					int h = m_VCodecCtx->height >> shift;
					int w = m_VCodecCtx->width >> shift;
					for (int j = 0; j < h; j++)
					{
						copied += w;
						assert(copied <= m_yuv->size());
						memcpy(ptr, yuv_factor, w);
						yuv_factor += m_Frame->linesize[i];
						ptr += w;
					}
				}
				video->m_size = copied;
				ret = m_qvideo.push(video) ? NULL : video;
			}
		}
		av_free_packet(&packet);
	}

	return ret;
}
#endif

YUV_video* NetStream::get_video()
{
	return m_yuv;
}

void
NetStream::seek()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
NetStream::setBufferTime()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
netstream_new(const fn_call& fn)
{
    netstream_as_object *netstream_obj = new netstream_as_object;

    netstream_obj->set_member("close", &netstream_close);
    netstream_obj->set_member("pause", &netstream_pause);
    netstream_obj->set_member("play", &netstream_play);
    netstream_obj->set_member("seek", &netstream_seek);
    netstream_obj->set_member("setbuffertime", &netstream_setbuffertime);

//		netstream_obj->add_ref();
//		movie_root* mr = (movie_root*) get_current_root();
//		mr->add_net_stream(netstream_obj);

    fn.result->set_as_object(netstream_obj);
}

void netstream_close(const fn_call& fn)
{
	assert(dynamic_cast<netstream_as_object*>(fn.this_ptr));
	netstream_as_object* ns = static_cast<netstream_as_object*>(fn.this_ptr);
	ns->obj.close();
}

void netstream_pause(const fn_call& /*fn*/) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void netstream_play(const fn_call& fn)
{
	assert(dynamic_cast<netstream_as_object*>(fn.this_ptr));
	netstream_as_object* ns = static_cast<netstream_as_object*>(fn.this_ptr);
	
	if (fn.nargs < 1)
	{
    log_error("NetStream play needs args\n");
    return;
	}

	// start gstreamer
	ns->obj.play(fn.arg(0).to_string());

}

void netstream_seek(const fn_call& /*fn*/) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void netstream_setbuffertime(const fn_call& /*fn*/) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

} // end of gnash namespace

