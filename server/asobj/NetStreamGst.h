// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

/* $Id: NetStreamGst.h,v 1.29 2007/12/04 11:45:31 strk Exp $ */

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
	void play(const std::string& source);
	void seek(boost::uint32_t pos);
	boost::int32_t time();
	void advance();

	// Used for gstreamer data read and seek callbacks
	static int readPacket(void* opaque, char* buf, int buf_size);
	static int seekMedia(void *opaque, int offset, int whence);

	void startPlayback();

	static void playbackStarter(NetStreamGst* ns);
	static void callback_output (GstElement* /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data);
	static void callback_newpad (GstElement *decodebin, GstPad *pad, gboolean last, gpointer data);
	static void video_callback_handoff (GstElement* /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data);
	static void audio_callback_handoff (GstElement* /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data);

private:

	/// Creates the sound decoder and source element for playing FLVs
	//
	/// Does NOT Lock the pipeline mutex during operations, use buildFLVPipeline() for that.
	///
	/// @return true on success, false on failure
	///
	/// @param sound
	///	Determines if sound should be setup. It is passed by reference 
	///     and might be changed.
	///
	bool buildFLVSoundPipeline(bool &sound);

	/// Creates the video decoder and source element for playing FLVs
	//
	/// Does NOT Lock the pipeline mutex during operations, use buildFLVPipeline() for that.
	///
	/// @return true on success, false on failure
	///
	/// @param video
	///	Determines if video should be setup. It is passed by reference 
	///     and might be changed.
	///
	bool buildFLVVideoPipeline(bool &video);

	/// Creates the decoder and source element for playing FLVs
	//
	/// Locks the pipeline mutex during operations
	///
	///
	/// @param video
	///	Determines if video should be setup. It is passed by reference 
	/// 	and might be changed.
	///
	/// @param sound
	///	Determines if sound should be setup. It is passed by reference 
	/// 	and might be changed.
	///
	/// @return true on success, false on failure
	///
	bool buildFLVPipeline(bool& video, bool& audio);

	/// Creates the decoder and source element for playing non-FLVs
	//
	/// Locks the pipeline mutex during operations
	///
	/// @return true on success, false on failure
	///
	bool buildPipeline();

	/// Unrefs (deletes) all the gstreamer elements. Used when the setup failed.
	//
	/// Locks the pipeline mutex during operations
	///
	void unrefElements();

	/// Connect the video "handoff" signal
	//
	/// @return true on success, false on failure
	///
	bool connectVideoHandoffSignal();

	/// Connect the audio "handoff" signal
	//
	/// @return true on success, false on failure
	///
	bool connectAudioHandoffSignal();

	/// Disconnect the video "handoff" signal
	//
	/// @return true on success, false on failure
	///
	bool disconnectVideoHandoffSignal();

	/// Disconnect the audio "handoff" signal
	//
	/// @return true on success, false on failure
	///
	bool disconnectAudioHandoffSignal();

	/// \brief
	/// Set pipeline state to GST_STATE_NULL
	/// and disconnect handoff signals
	//
	/// Locks the pipeline mutex during operations
	///
	/// If the call needs be asynchronous, we'll wait for it.
	/// TODO: implement the above
	///
	bool disablePipeline();

	/// \brief
	/// Set pipeline state to GST_STATE_PLAYING,
	/// connect handoff signals, send appropriate
	/// notifications.
	//
	/// Locks the pipeline mutex during operations
	///
	/// If the call needs be asynchronous, we'll wait for it.
	/// TOOD: implement the above
	///
	bool playPipeline();

	/// \brief
	/// Set pipeline state to GST_STATE_PAUSE,
	/// connect handoff signals if not connected already
	//
	/// Locks the pipeline mutex during operations
	///
	/// If the call needs be asynchronous, we'll wait for it.
	/// TOOD: implement the above
	///
	bool pausePipeline(bool startOnBuffer);

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

	// Mutex protecting pipeline control
	boost::mutex _pipelineMutex;

	// used only for FLV
	GstElement *audiosource;
	GstElement *videosource;
	GstElement *source;
	GstElement *videodecoder;
	GstElement *audiodecoder;
	GstElement *videoinputcaps;
	GstElement *audioinputcaps;

	// Signal handlers id
	gulong _handoffVideoSigHandler;
	gulong _handoffAudioSigHandler;

#ifndef DISABLE_START_THREAD
	boost::thread *startThread;
#endif

	// video info
	int videowidth;
	int videoheight;

	// Used when seeking. To make the gst-pipeline more cooperative
	// we don't tell it when we seek, but just add m_clock_offset to
	// make it believe the search never happend. A better aproach whould
	// probably be to make a dedicated gstreamer source element.
	volatile long m_clock_offset;

	// On next advance() should we pause?
	volatile bool m_pausePlayback;

};

} // gnash namespace

#endif // SOUND_GST

#endif //  __NETSTREAMGST_H__
