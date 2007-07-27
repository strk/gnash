// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifndef __SOUNDGST_H__
#define __SOUNDGST_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "impl.h"
#include "as_object.h"
#include "NetConnection.h"
#include "Sound.h" // for inheritance
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp> 
#include <boost/thread/mutex.hpp>

#include <gst/gst.h>

namespace gnash {

// Forward declarations
class fn_call;
  
class SoundGst : public Sound {
public:
	SoundGst()
		:
		pipeline(NULL),
		audiosink(NULL),
		source(NULL),
		decoder(NULL),
		volume(NULL),
		audioconv(NULL),
		setupThread(NULL),
		lock(NULL), 
		inputPos(0),
		isAttached(false),
		remainingLoops(0)
	{}
	~SoundGst();

	void loadSound(std::string file, bool streaming);
	void start(int offset, int loops);
	void stop(int si);
	unsigned int getDuration();
	unsigned int getPosition();

	// Used for ffmpeg data read and seek callbacks
	static int readPacket(void* opaque, char* buf, int buf_size);
	static int seekMedia(void *opaque, int offset, int whence);

	static void setupDecoder(SoundGst* so);
	static bool getAudio(void *owner, uint8_t *stream, int len);

	static void callback_newpad (GstElement *decodebin, GstPad *pad, gboolean last, gpointer data);
private:

	// gstreamer pipeline objects
	GstElement *pipeline;
	GstElement *audiosink;
	GstElement *source;
	GstElement *decoder;
	GstElement *volume;
	GstElement *audioconv;

	boost::thread *setupThread;
	boost::mutex setupMutex;
	boost::mutex::scoped_lock *lock;

	long inputPos;

	// Are this sound attached to the soundhandler?
	bool isAttached;

	int remainingLoops;
};


} // end of gnash namespace

// __SOUNDGST_H__
#endif

