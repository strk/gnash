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

/// Define DISABLE_START_THREAD to avoid using threads 
///
/// TODO: use a globally-defined thread-disabling routine
///       when available
///
//#define DISABLE_START_THREAD


namespace gnash {
  
class NetStreamGst: public NetStream {
public:
	NetStreamGst();
	~NetStreamGst();
	void close();
	void pause(int mode);
	int play(const std::string& source);
	void seek(double pos);
	int64_t time();
	void advance();

	// Used for gstreamer data read and seek callbacks
	static int readPacket(void* opaque, char* buf, int buf_size);
	static int seekMedia(void *opaque, int offset, int whence);

	static void startPlayback(NetStreamGst* ns);
	static void callback_output (GstElement* /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data);
	static void callback_newpad (GstElement *decodebin, GstPad *pad, gboolean last, gpointer data);
	static void video_callback_handoff (GstElement* /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data);
	static void audio_callback_handoff (GstElement* /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data);

private:

	/// Set pipeline state to GST_STATE_NULL 
	/// (the NULL state or initial state of an elemen)
	///
	/// If the call needs be asynchronous, we'll wait for it.
	///
	bool resetPipeline();

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

#ifndef DISABLE_START_THREAD
	boost::thread *startThread;
#endif

	// video info
	int videowidth;
	int videoheight;

	volatile long m_clock_offset;

	// On next advance() should we pause?
	volatile bool m_pausePlayback;

};

} // gnash namespace

#endif // SOUND_GST

#endif //  __NETSTREAMGST_H__
