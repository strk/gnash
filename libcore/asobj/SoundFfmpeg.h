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

#ifndef GNASH_SOUNDFFMPEG_H
#define GNASH_SOUNDFFMPEG_H

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
#include "log.h"
#include "Sound.h" // for inheritance
#include "MediaHandler.h"
#include "MediaParser.h"
#include "AudioDecoder.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/bind.hpp> 
#include <boost/thread/mutex.hpp>
#include <boost/scoped_ptr.hpp>

#ifdef HAVE_FFMPEG_AVFORMAT_H
extern "C" {
#include <ffmpeg/avformat.h>
}
#endif

#ifdef HAVE_LIBAVFORMAT_AVFORMAT_H
extern "C" {
#include <libavformat/avformat.h>
}
#endif

namespace gnash {

// Forward declarations
class fn_call;
  
class SoundFfmpeg : public Sound {
public:

	SoundFfmpeg();

	~SoundFfmpeg();

	void loadSound(const std::string& file, bool streaming);
	void start(int offset, int loops);
	void stop(int si);
	unsigned int getDuration();
	unsigned int getPosition();

	// see dox in Sound.h
	virtual long getBytesLoaded();

	// see dox in Sound.h
	virtual long getBytesTotal();

private:

	media::MediaHandler* _mediaHandler;
	boost::scoped_ptr<media::MediaParser> _mediaParser;
	boost::scoped_ptr<media::AudioDecoder> _audioDecoder;

	/// Number of milliseconds into the sound to start it
	//
	/// This is set by start()
	boost::uint64_t _startTime;

	boost::scoped_array<boost::uint8_t> _leftOverData;
	boost::uint8_t* _leftOverPtr;
	boost::uint32_t _leftOverSize;

	static bool getAudioWrapper(void *owner, boost::uint8_t *stream, int len);

	bool getAudio(boost::uint8_t *stream, int len);

	// Are this sound attached to the soundhandler?
	bool isAttached;

	int remainingLoops;
};


} // end of gnash namespace

// __ASSOUND_H__
#endif

