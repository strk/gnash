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

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp> 
#include <boost/thread/mutex.hpp>
#include "impl.h"
#include "video_stream_instance.h"
#include <gst/gst.h>
#include "image.h"
#include "NetStream.h" // for inheritance
#include "FLVParser.h"

namespace gnash {
  
class NetStreamGst: public NetStream {
public:
	NetStreamGst();
	~NetStreamGst();
	void close();
	void pause(int mode);
	int play(const char* source);
	void seek(double pos);
	void setBufferTime(double time);
	void set_status(const char* code);
	void setNetCon(as_object* nc);
	int64_t time();
	long bytesLoaded();
	long bytesTotal();
	void advance();
	bool newFrameReady();
	as_function* getStatusHandler();
	void setStatusHandler(as_function*);

	// Used for gstreamer data read and seek callbacks
	static int readPacket(void* opaque, char* buf, int buf_size);
	static int seekMedia(void *opaque, int offset, int whence);

	image::image_base* get_video();

	inline bool playing()
	{
		return m_go;
	}

	static void startPlayback(NetStreamGst* ns);
	static void callback_output (GstElement* /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data);
	static void callback_newpad (GstElement *decodebin, GstPad *pad, gboolean last, gpointer data);
	static void video_callback_handoff (GstElement* /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data);
	static void audio_callback_handoff (GstElement* /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data);

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
	GstElement *decoder;
	GstElement *volume;
	GstElement *colorspace;
	GstElement *videorate;
	GstElement *videocaps;
	GstElement *videoflip;
	GstElement *audioconv;

	// used only for FLV
	GstElement *audiosource;
	GstElement *videosource;
	GstElement *source;
	GstElement *videodecoder;
	GstElement *audiodecoder;
	GstElement *videoinputcaps;
	GstElement *audioinputcaps;

	// Are the playing loop running or not
	volatile bool m_go;

	// The image/videoframe which is given to the renderer
	image::image_base* m_imageframe;

	boost::thread *startThread;
	bool m_pause;

	long inputPos;
	std::string url;

	// video info
	int videowidth;
	int videoheight;

	volatile bool m_newFrameReady;
	
	// The size of the buffer in milliseconds
	uint32_t m_bufferTime;

	// The status message
	std::string m_status;

	// Has the status message been updated?
	volatile bool m_statusChanged;

	// The handler which is invoked on status change
	boost::intrusive_ptr<as_function> m_statusHandler;

	// Are we decoding a FLV?
	bool m_isFLV;

	// The parser for FLV
	FLVParser* m_parser;
};

} // gnash namespace

#endif // SOUND_GST

#endif //  __NETSTREAMGST_H__
