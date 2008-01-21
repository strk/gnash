// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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
#include "gnashconfig.h"
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
		_pipeline(NULL),
		_audiosink(NULL),
		_source(NULL),
		_decoder(NULL),
		_volume(NULL),
		_audioconv(NULL),
		_inputPos(0),
		_isAttached(false),
		_remainingLoops(0)
	{}
	~SoundGst();

	void loadSound(const std::string& file, bool streaming);
	void start(int offset, int loops);
	void stop(int si);
	unsigned int getDuration();
	unsigned int getPosition();

	void setupDecoder(const std::string& url);

	static void callback_newpad (GstElement *decodebin, GstPad *pad, gboolean last, gpointer data);
private:

	// gstreamer pipeline objects
	GstElement* _pipeline;
	GstElement* _audiosink;
	GstElement* _source;
	GstElement* _decoder;
	GstElement* _volume;
	GstElement* _audioconv;

	long _inputPos;

	// Is this sound attached to the soundhandler?
	bool _isAttached;

	int _remainingLoops;
};


} // end of gnash namespace

// __SOUNDGST_H__
#endif

