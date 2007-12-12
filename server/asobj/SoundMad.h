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

#ifndef __SOUNDMAD_H__
#define __SOUNDMAD_H__

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

#include <mad.h>

namespace gnash {

// Forward declarations
class fn_call;
  
class SoundMad : public Sound {
public:
	SoundMad()
		:
		inputPos(0),
		setupThread(NULL),
		lock(NULL), 
		bitrate(0),
		leftOverData(NULL),
		leftOverSize(0),
		isAttached(false),
		remainingLoops(0)
	{}
	~SoundMad();

	void loadSound(std::string file, bool streaming);
	void start(int offset, int loops);
	void stop(int si);
	unsigned int getDuration();
	unsigned int getPosition();

	static void setupDecoder(SoundMad* so);
	static bool getAudio(void *owner, boost::uint8_t *stream, int len);
private:

	long inputPos;

	boost::thread *setupThread;
	boost::mutex setupMutex;
	boost::mutex::scoped_lock *lock;

	// bitrate in bps
	unsigned long bitrate;

	int readPacket(boost::uint8_t* buf, int buf_size);
	int seekMedia(int offset, int whence);

	/// mad stuff
	mad_stream	stream;
	mad_frame	frame;
	mad_synth 	synth;

	// If the decoded data doesn't fit the buffer we put the leftovers here
	boost::uint8_t* leftOverData;
	int leftOverSize;

	// Are this sound attached to the soundhandler?
	bool isAttached;

	int remainingLoops;
};


} // end of gnash namespace

// __ASSOUND_H__
#endif

