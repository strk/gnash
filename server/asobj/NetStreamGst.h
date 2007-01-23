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

/* $id$ */

#ifndef __NETSTREAMGST_H__
#define __NETSTREAMGST_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef SOUND_GST

#include <pthread.h>
#include "impl.h"
#include "video_stream_instance.h"
#include <gst/gst.h>
#include "image.h"

namespace gnash {
  
class netstream_as_object;

class NetStreamGst {
public:
	NetStreamGst();
	~NetStreamGst();
	void close();
	void pause(int mode);
	int play(const char* source);
	void seek(double pos);
	void setBufferTime();
	void set_status(const char* code);
	void setNetCon(as_object* nc);
	long time();

	// Used for gstreamer data read and seek callbacks
	static int readPacket(void* opaque, char* buf, int buf_size);
	static int seekMedia(void *opaque, int offset, int whence);

	image::image_base* get_video();

	inline bool playing()
	{
		return m_go;
	}

	inline void set_parent(netstream_as_object* ns)
	{
		m_netstream_object = ns;
	}

	static void* startPlayback(void* arg);
	static void callback_output (GstElement* /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data);
	static void callback_newpad (GstElement *decodebin, GstPad *pad, gboolean last, gpointer data);
private:

	bool _bufferLength;
	bool _bufferTime;
	bool _bytesLoaded;
	bool _bytesTotal;
	bool _currentFps;
	bool _onStatus;
	bool _time;

	// gstreamer pipeline objects
	GstElement *pipeline;
	GstElement *audiosink;
	GstElement *videosink;
	GstElement *source;
	GstElement *decoder;
	GstElement *volume;
	GstElement *colorspace;
	GstElement *videorate;
	GstElement *videocaps;
	GstElement *videoflip;
	GstElement *audioconv;

	volatile bool m_go;

	image::image_base* m_imageframe;

	pthread_t startThread;
	bool m_pause;
	netstream_as_object* m_netstream_object;

	as_object* netCon;
	long inputPos;
	std::string url;

	// video info
	int videowidth;
	int videoheight;
};

} // gnash namespace

#endif // SOUND_GST

#endif //  __NETSTREAMGST_H__
