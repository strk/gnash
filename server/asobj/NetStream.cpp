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

/* $Id: NetStream.cpp,v 1.13 2006/11/03 15:28:51 alexeev Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "NetStream.h"
#include "fn_call.h"

#include "StreamProvider.h"	
#include "render.h"	
#include "movie_root.h"

#if defined(_WIN32) || defined(WIN32)
	#include <Windows.h>	// for sleep()
	#define sleep Sleep
#else
  #include "unistd.h" // for sleep()
#endif

namespace gnash {
 
namespace globals { // gnash::globals

	/// global StreamProvider
	extern StreamProvider streamProvider;

} // namespace gnash::global


NetStream::NetStream():
	m_video_index(-1),
	m_audio_index(-1),
#ifdef USE_FFMPEG
	m_VCodecCtx(NULL),
	m_ACodecCtx(NULL),
	m_FormatCtx(NULL),
	m_Frame(NULL),
#endif
	m_go(false),
	m_yuv(NULL),
	m_video_clock(0)
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

	// wait till thread is complete before main continues
	pthread_join(m_thread, NULL);

	sound_handler* s = get_sound_handler();
	if (s)
	{
		s->detach_aux_streamer((void*) this);
	}

#ifdef USE_FFMPEG

	if (m_Frame) av_free(m_Frame);
	m_Frame = NULL;

	if (m_VCodecCtx) avcodec_close(m_VCodecCtx);
	m_VCodecCtx = NULL;

	if (m_ACodecCtx) avcodec_close(m_ACodecCtx);
	m_ACodecCtx = NULL;

	if (m_FormatCtx) av_close_input_file(m_FormatCtx);
	m_FormatCtx = NULL;

#endif

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

int
NetStream::play(const char* c_url)
{

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

#ifdef USE_FFMPEG

	// Is it already playing ?
	if (m_FormatCtx)
	{
		return 0;
	}

	av_register_all();

	// Open video file
	// The last three parameters specify the file format, buffer size and format parameters;
	// by simply specifying NULL or 0 we ask libavformat to auto-detect the format 
	// and use a default buffer size
	if (av_open_input_file(&m_FormatCtx, c_url, NULL, 0, NULL) != 0)
	{
	  log_error("Couldn't open file '%s'", c_url);
		return -1;
	}

	// Next, we need to retrieve information about the streams contained in the file
	// This fills the streams field of the AVFormatContext with valid information
	if (av_find_stream_info(m_FormatCtx) < 0)
	{
    log_error("Couldn't find stream information from '%s'", c_url);
		return -1;
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
					m_audio_stream = m_FormatCtx->streams[i];
				}
				break;

			case CODEC_TYPE_VIDEO:
				if (m_video_index < 0)
				{
					m_video_index = i;
					m_video_stream = m_FormatCtx->streams[i];
				}
				break;
    }
	}

	if (m_video_index < 0)
	{
		log_error("Didn't find a video stream from '%s'", c_url);
		return -1;
	}

	// Get a pointer to the codec context for the video stream
	m_VCodecCtx = m_FormatCtx->streams[m_video_index]->codec;

	// Find the decoder for the video stream
	AVCodec* pCodec = avcodec_find_decoder(m_VCodecCtx->codec_id);
	if (pCodec == NULL)
	{
		log_error("Decoder not found");
		return -1;
	}

	// Open codec
	if (avcodec_open(m_VCodecCtx, pCodec) < 0)
	{
		log_error("Could not open codec");
	}

	// Allocate a frame to store the decoded frame in
	m_Frame = avcodec_alloc_frame();
	
	// Determine required buffer size and allocate buffer
	m_yuv = render::create_YUV_video(m_VCodecCtx->width,	m_VCodecCtx->height);


	sound_handler* s = get_sound_handler();
	if (m_audio_index >= 0 && s != NULL)
	{
		// Get a pointer to the audio codec context for the video stream
		m_ACodecCtx = m_FormatCtx->streams[m_audio_index]->codec;
    
    // Find the decoder for the audio stream
    AVCodec* pACodec = avcodec_find_decoder(m_ACodecCtx->codec_id);
    if(pACodec == NULL)
		{
      log_error("No available AUDIO decoder to process MPEG file: '%s'", c_url);
			return -1;
		}
        
    // Open codec
    if (avcodec_open(m_ACodecCtx, pACodec) < 0)
		{
			log_error("Could not open AUDIO codec");
			return -1;
		}
	
		s->attach_aux_streamer(audio_streamer, (void*) this);

	}

#endif

	pthread_create(&m_thread, NULL, NetStream::av_streamer, this);

	return 0;
}

// decoder thread
void* NetStream::av_streamer(void* arg)
{
	NetStream* ns = static_cast<NetStream*>(arg);
	raw_videodata_t* unqueued_data = NULL;
	raw_videodata_t* video = NULL;

	ns->m_video_clock = 0;

	int delay = 0;
	double start_clock = tu_timer::ticks_to_seconds(tu_timer::get_ticks());
	ns->m_go = true;
	while (ns->m_go)
	{
		unqueued_data = ns->read_frame(unqueued_data);
		if (ns->m_qvideo.size() > 0)
		{
			video = ns->m_qvideo.front();
			double clock = tu_timer::ticks_to_seconds(tu_timer::get_ticks()) - start_clock;
			double video_clock = video->m_pts;

			if (clock >= video_clock)
			{
				ns->m_yuv->update(video->m_data);
				ns->m_qvideo.pop();
				delete video;
				delay = 0;
			}
			else
			{
				delay = int(1000 * (video_clock - clock));
			}
		}

		// Don't hog the CPU.
		// Queues have filled, video frame have shown
		// now it is possible and to have a rest
		if (unqueued_data && delay > 0)
		{
			sleep(delay);
		}
	}
	return 0;
}

// audio callback is running in sound handler thread
void NetStream::audio_streamer(void *owner, uint8 *stream, int len)
{
	NetStream* ns = static_cast<NetStream*>(owner);

	while (len > 0 && ns->m_qaudio.size() > 0)
	{
		raw_videodata_t* samples = ns->m_qaudio.front();

		int n = imin(samples->m_size, len);
		memcpy(stream, samples->m_ptr, n);
		stream += n;
		samples->m_ptr += n;
		samples->m_size -= n;
		len -= n;

		if (samples->m_size == 0)
		{
			ns->m_qaudio.pop();
			delete samples;
		}
	}
}

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
				ret = m_qaudio.push(unqueued_data) ? NULL : unqueued_data;
			}
		}
		else
		{
			ret = m_qvideo.push(unqueued_data) ? NULL : unqueued_data;
		}

		return ret;
	}

#ifdef USE_FFMPEG

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
					int n = 0;

					bool stereo = m_ACodecCtx->channels > 1 ? true : false;
					s->convert_raw_data(&adjusted_data, &n, ptr, frame_size >> 1, 2, m_ACodecCtx->sample_rate, stereo);
			    raw_videodata_t* samples = new raw_videodata_t;
					samples->m_data = (uint8_t*) adjusted_data;
					samples->m_ptr = samples->m_data;
					samples->m_size = n;
					samples->m_stream_index = m_audio_index;

//					samples->m_pts = m_audio_clock;
//					n = 2 * m_audio_stream->codec->channels;
//					m_audio_clock += (double) frame_size / (double)(n * m_audio_stream->codec->sample_rate);

					ret = m_qaudio.push(samples) ? NULL : samples;
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

				// set presentation timestamp
				if (packet.dts != AV_NOPTS_VALUE)
				{
					video->m_pts = as_double(m_video_stream->time_base) * packet.dts;
				}

				if (video->m_pts != 0)
				{	
					// update video clock with pts, if present
					m_video_clock = video->m_pts;
				}
				else
				{
					video->m_pts = m_video_clock;
				}

				// update video clock for next frame
				double frame_delay = as_double(m_video_stream->codec->time_base);

				// for MPEG2, the frame can be repeated, so we update the clock accordingly
				frame_delay += m_Frame->repeat_pict * (frame_delay * 0.5);

				m_video_clock += frame_delay;

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

#endif

	return ret;
}


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

	if (ns->obj.play(fn.arg(0).to_string()) != 0)
	{
		ns->obj.close();
	};
}

void netstream_seek(const fn_call& /*fn*/) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void netstream_setbuffertime(const fn_call& /*fn*/) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

} // end of gnash namespace

