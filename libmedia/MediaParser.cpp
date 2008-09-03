// MediaParser.cpp:  Media file parser, for Gnash.
//
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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
//


#include "MediaParser.h"
#include "log.h"

#include <unistd.h>             // for usleep()
#include <boost/bind.hpp>

#ifdef _WIN32
#include <windows.h>
#define usleep(usec) ((void) Sleep((usec) / 1000))
#endif

namespace gnash {
namespace media {

MediaParser::MediaParser(std::auto_ptr<IOChannel> stream)
	:
	_stream(stream),
	_parsingComplete(false),
	_bufferTime(100), // 100 ms 
	_parserThread(0),
	_parserThreadStartBarrier(2),
	_parserThreadKillRequested(false),
	_seekRequest(false),
	_bytesLoaded(0)
{
}

/*protected*/
void
MediaParser::startParserThread()
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	log_debug("Starting MediaParser thread");
	_parserThread.reset( new boost::thread(boost::bind(parserLoopStarter, this)) );
	_parserThreadStartBarrier.wait();
#endif
}

boost::uint64_t
MediaParser::getBufferLength() const
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	boost::mutex::scoped_lock lock(_qMutex);
#endif
	return getBufferLengthNoLock();
}

boost::uint64_t
MediaParser::getBufferLengthNoLock() const
{
	bool hasVideo = _videoInfo.get();
	bool hasAudio = _audioInfo.get();

	//log_debug("MediaParser::getBufferLength: %d video %d audio frames", _videoFrames.size(), _audioFrames.size());

	if ( hasVideo && hasAudio )
	{
		return std::min(audioBufferLength(), videoBufferLength());
	}
	else if ( hasVideo )
	{
		return videoBufferLength();
	}
	else if ( hasAudio )
	{
		return audioBufferLength();
	}
	else return 0;
}

boost::uint64_t
MediaParser::videoBufferLength() const
{
	if (_videoFrames.empty()) return 0;
#if 0 // debugging
	log_debug("videoBufferLength: %d - %d == %d",
		_videoFrames.back()->timestamp(), _videoFrames.front()->timestamp(),
		_videoFrames.back()->timestamp() - _videoFrames.front()->timestamp());
#endif
	return _videoFrames.back()->timestamp() - _videoFrames.front()->timestamp(); 
}

boost::uint64_t
MediaParser::audioBufferLength() const
{
	if (_audioFrames.empty()) return 0;
#if 0 // debugging
	log_debug("audioBufferLength: %d - %d == %d",
		_audioFrames.back()->timestamp, _audioFrames.front()->timestamp,
		_audioFrames.back()->timestamp - _audioFrames.front()->timestamp);
#endif
	return _audioFrames.back()->timestamp - _audioFrames.front()->timestamp; 
}

const EncodedVideoFrame*
MediaParser::peekNextVideoFrame() const
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	boost::mutex::scoped_lock lock(_qMutex);
#else // ndef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	while (!parsingCompleted() && _videoInfo.get() && _videoFrames.empty())
	{
		const_cast<MediaParser*>(this)->parseNextChunk();
	}
#endif

	if (!_videoInfo.get() || _videoFrames.empty()) return 0;
	return _videoFrames.front();
}

bool
MediaParser::nextVideoFrameTimestamp(boost::uint64_t& ts) const
{
	const EncodedVideoFrame* ef = peekNextVideoFrame();
	if ( ! ef ) return false;
	ts = ef->timestamp();
	return true;
}

std::auto_ptr<EncodedVideoFrame>
MediaParser::nextVideoFrame()
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	boost::mutex::scoped_lock lock(_qMutex);
#else // ndef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	while (!parsingCompleted() && _videoInfo.get() && _videoFrames.empty())
	{
		const_cast<MediaParser*>(this)->parseNextChunk();
	}
#endif

	std::auto_ptr<EncodedVideoFrame> ret;
	if (_videoFrames.empty()) return ret;
	ret.reset(_videoFrames.front());
	_videoFrames.pop_front();
#ifdef GNASH_DEBUG_MEDIAPARSER
	log_debug("nextVideoFrame: waking up parser (in case it was sleeping)");
#endif // GNASH_DEBUG_MEDIAPARSER
	_parserThreadWakeup.notify_all(); // wake it up, to refill the buffer, SHOULDN'T WE HOLD A LoCK HERE?
	return ret;
}

std::auto_ptr<EncodedAudioFrame>
MediaParser::nextAudioFrame()
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	boost::mutex::scoped_lock lock(_qMutex);
#else // ndef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	while (!parsingCompleted() && _audioInfo.get() && _audioFrames.empty())
	{
		const_cast<MediaParser*>(this)->parseNextChunk();
	}
#endif

	std::auto_ptr<EncodedAudioFrame> ret;
	if (_audioFrames.empty()) return ret;
	ret.reset(_audioFrames.front());
	_audioFrames.pop_front();
#ifdef GNASH_DEBUG_MEDIAPARSER
	log_debug("nextAudioFrame: waking up parser (in case it was sleeping)");
#endif // GNASH_DEBUG_MEDIAPARSER
	_parserThreadWakeup.notify_all(); // wake it up, to refill the buffer, SHOULDN'T WE HOLD A LoCK HERE?
	return ret;
}

bool
MediaParser::nextAudioFrameTimestamp(boost::uint64_t& ts) const
{
	const EncodedAudioFrame* ef = peekNextAudioFrame();
	if ( ! ef ) return false;
	ts = ef->timestamp;
	return true;
}

const EncodedAudioFrame*
MediaParser::peekNextAudioFrame() const
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	boost::mutex::scoped_lock lock(_qMutex);
#else // ndef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	while (!parsingCompleted() && _audioInfo.get() && _audioFrames.empty())
	{
		const_cast<MediaParser*>(this)->parseNextChunk();
	}
#endif
	if (!_audioInfo.get() || _audioFrames.empty()) return 0;
	return _audioFrames.front();
}

MediaParser::~MediaParser()
{
	if ( _parserThread.get() )
	{
		requestParserThreadKill();
		_parserThread->join();
	}

	for (VideoFrames::iterator i=_videoFrames.begin(),
		e=_videoFrames.end(); i!=e; ++i)
	{
		delete (*i);
	}

	for (AudioFrames::iterator i=_audioFrames.begin(),
		e=_audioFrames.end(); i!=e; ++i)
	{
		delete (*i);
	}
}

void
MediaParser::clearBuffers()
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	boost::mutex::scoped_lock lock(_qMutex);
#endif

	for (VideoFrames::iterator i=_videoFrames.begin(),
		e=_videoFrames.end(); i!=e; ++i)
	{
		delete (*i);
	}

	for (AudioFrames::iterator i=_audioFrames.begin(),
		e=_audioFrames.end(); i!=e; ++i)
	{
		delete (*i);
	}

	_audioFrames.clear();
	_videoFrames.clear();

	_parserThreadWakeup.notify_all(); // wake it up, to refill the buffer
}

void
MediaParser::pushEncodedAudioFrame(std::auto_ptr<EncodedAudioFrame> frame)
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	boost::mutex::scoped_lock lock(_qMutex);
#endif

	// If last frame on queue has a timestamp > then this one, that's either due
	// to seek-back (most commonly) or a wierdly encoded media file.
	// In any case, we'll flush the queue and restart from the new timestamp
	if ( ! _audioFrames.empty() && _audioFrames.back()->timestamp > frame->timestamp )
	{
		log_debug("Timestamp of last audio frame in queue (%d) "
			"greater then timestamp in the frame being "
			"pushed to it (%d). Flushing %d queue elements.",
			_audioFrames.back()->timestamp, frame->timestamp,
			_audioFrames.size());
		for (AudioFrames::iterator i=_audioFrames.begin(),
			e=_audioFrames.end(); i!=e; ++i)
		{
			delete (*i);
		}
		_audioFrames.clear();
	}
	
	//log_debug("Pushing audio frame with timestamp %d", frame->timestamp);
	_audioFrames.push_back(frame.release());
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	waitIfNeeded(lock); // if the push reaches a "buffer full" condition, wait to be waken up
#endif
}

void
MediaParser::pushEncodedVideoFrame(std::auto_ptr<EncodedVideoFrame> frame)
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	boost::mutex::scoped_lock lock(_qMutex);
#endif

	// If last frame on queue has a timestamp > then this one, that's either due
	// to seek-back (most commonly) or a wierdly encoded media file.
	// In any case, we'll flush the queue and restart from the new timestamp
	if ( ! _videoFrames.empty() && _videoFrames.back()->timestamp() > frame->timestamp() )
	{
		log_debug("Timestamp of last video frame in queue (%d) "
			"greater then timestamp in the frame being "
			"pushed to it (%d). Flushing %d queue elements.",
			_videoFrames.back()->timestamp(), frame->timestamp(),
			_videoFrames.size());
		for (VideoFrames::iterator i=_videoFrames.begin(),
			e=_videoFrames.end(); i!=e; ++i)
		{
			delete (*i);
		}
		_videoFrames.clear();
	}

	//log_debug("Pushing video frame with timestamp %d", frame->timestamp());
	_videoFrames.push_back(frame.release());
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	waitIfNeeded(lock); // if the push reaches a "buffer full" condition, wait to be waken up
#endif
}

void
MediaParser::waitIfNeeded(boost::mutex::scoped_lock& lock) 
{
	//  We hold a lock on the queue here...
	bool pc=parsingCompleted();
	bool ic=indexingCompleted();
	bool bf=bufferFull();
	if ( pc || (bf && ic) ) // TODO: or seekRequested ?
	{
#ifdef GNASH_DEBUG_MEDIAPARSER
		log_debug("Parser thread waiting on wakeup lock, parsingComplete=%d, bufferFull=%d", pc, bf);
#endif // GNASH_DEBUG_MEDIAPARSER
		_parserThreadWakeup.wait(lock);
#ifdef GNASH_DEBUG_MEDIAPARSER
		log_debug("Parser thread finished waiting on wakeup lock");
#endif // GNASH_DEBUG_MEDIAPARSER
	}
}

bool
MediaParser::bufferFull() const
{
	// Callers are expected to hold a lock on _qMutex
	int bl = getBufferLengthNoLock();
	int bt = getBufferTime();
#ifdef GNASH_DEBUG_MEDIAPARSER
	log_debug("bufferFull: %d/%d", bl, bt);
#endif // GNASH_DEBUG_MEDIAPARSER
	return bl > bt;
}

void
MediaParser::parserLoop()
{
	_parserThreadStartBarrier.wait();
	while (!parserThreadKillRequested())
	{
		parseNextChunk();
		usleep(100); // no rush....
	}
}


void
MediaParser::processTags(boost::uint64_t /*ts*/, as_object* /*thisPtr*/, VM& /*env*/)
{
}

std::ostream& operator << (std::ostream& os, const VideoInfo& vi)
{
	os << "codec:" << vi.codec << " (type " << vi.type << ") - "
	   << "size:" << vi.width << "x" << vi.height << " - "
	   << "frameRate:" << vi.frameRate << " - "
	   << "duration:" << vi.duration;
	return os;
}


} // end of gnash::media namespace
} // end of gnash namespace

#undef PADDING_BYTES
#undef READ_CHUNKS 
