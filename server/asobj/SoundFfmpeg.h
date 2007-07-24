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

#ifndef __SOUNDFFMPEG_H__
#define __SOUNDFFMPEG_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
#include "log.h"
#include "Sound.h" // for inheritance
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp> 
#include <boost/thread/mutex.hpp>

extern "C" {
#include <ffmpeg/avformat.h>
}

namespace gnash {

// Forward declarations
class fn_call;
  
class SoundFfmpeg : public Sound {
public:

	SoundFfmpeg()
		: // REMEMBER TO ALWAYS INITIALIZE ALL MEMBERS !
		audioCodecCtx(NULL),
		audioStream(NULL),
		formatCtx(NULL),
		audioFrame(NULL),
		resampleCtx(NULL),
		setupThread(NULL),
		lock(NULL), 
		inputPos(0),
		ByteIOCxt(), // ?
		audioIndex(-1),
		leftOverData(NULL),
		leftOverSize(0),
		isAttached(false),
		remainingLoops(0)
	{}

	~SoundFfmpeg();

	void loadSound(std::string file, bool streaming);
	void start(int offset, int loops);
	void stop(int si);
	unsigned int getDuration();
	unsigned int getPosition();

	// Used for ffmpeg data read and seek callbacks
	static int readPacket(void* opaque, uint8_t* buf, int buf_size);
	static offset_t seekMedia(void *opaque, offset_t offset, int whence);

	static void setupDecoder(SoundFfmpeg* so);
	static bool getAudio(void *owner, uint8_t *stream, int len);
private:

	// audio
	AVCodecContext *audioCodecCtx;
	AVStream* audioStream;

	AVFormatContext *formatCtx;

	AVFrame* audioFrame;

	ReSampleContext *resampleCtx;

	boost::thread *setupThread;
	boost::mutex setupMutex;

	// TODO: it makes NO SENSE for a scoped_lock to be allocated on the heap !
	boost::mutex::scoped_lock *lock;

	long inputPos;

	ByteIOContext ByteIOCxt;

	// The stream in the file that we use
	int audioIndex;

	// If the decoded data doesn't fit the buffer we put the leftovers here
	uint8_t* leftOverData;
	int leftOverSize;

	// Are this sound attached to the soundhandler?
	bool isAttached;

	int remainingLoops;
};


} // end of gnash namespace

// __ASSOUND_H__
#endif

