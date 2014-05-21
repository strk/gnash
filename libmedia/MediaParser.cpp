// MediaParser.cpp:  Media file parser, for Gnash.
//
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#include <functional>

#include "log.h"
#include "GnashSleep.h" // for usleep.
#include "Id3Info.h"

// Define this to get debugging output from MediaParser
//#define GNASH_DEBUG_MEDIAPARSER

namespace gnash {
namespace media {

MediaParser::MediaParser(std::unique_ptr<IOChannel> stream)
	:
	_parsingComplete(false),
	_bytesLoaded(0),
	_stream(std::move(stream)),
	_bufferTime(100), // 100 ms 
	_parserThread(),
	_parserThreadStartBarrier(2),
	_parserThreadKillRequested(false),
	_seekRequest(false)
{
}

/*protected*/
void
MediaParser::startParserThread()
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	log_debug("Starting MediaParser thread");
	_parserThread.reset(new boost::thread(
                std::bind(parserLoopStarter, this)));
	_parserThreadStartBarrier.wait();
#endif
}

std::uint64_t
MediaParser::getBufferLength() const
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	boost::mutex::scoped_lock lock(_qMutex);
#endif
	return getBufferLengthNoLock();
}

/* public */
bool
MediaParser::isBufferEmpty() const
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	boost::mutex::scoped_lock lock(_qMutex);
#endif
	return _videoFrames.empty() && _audioFrames.empty();
}

boost::optional<Id3Info>
MediaParser::getId3Info() const
{
    log_error(_("No ID3 support implemented in this MediaParser"));
    return boost::optional<Id3Info>();
}

std::uint64_t
MediaParser::getBufferLengthNoLock() const
{
	bool hasVideo = _videoInfo.get();
	bool hasAudio = _audioInfo.get();

	//log_debug("MediaParser::getBufferLength: %d video %d audio frames", _videoFrames.size(), _audioFrames.size());

	if (hasVideo && hasAudio) {
		return std::min(audioBufferLength(), videoBufferLength());
	}
	
    if (hasVideo) return videoBufferLength();
	
    if (hasAudio) return audioBufferLength();
	
    return 0;
}

std::uint64_t
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

std::uint64_t
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

/*private*/
const EncodedVideoFrame*
MediaParser::peekNextVideoFrame() const
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
    // TODO: assert _qMutex is locked by this thread
#else // ndef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	while (!parsingCompleted() && _videoInfo.get() && _videoFrames.empty())
	{
		const_cast<MediaParser*>(this)->parseNextChunk();
	}
#endif

	if (!_videoInfo.get() || _videoFrames.empty()) return nullptr;
	return _videoFrames.front();
}

bool
MediaParser::nextFrameTimestamp(std::uint64_t& ts) const
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
    boost::mutex::scoped_lock lock(_qMutex);
#else // ndef LOAD_MEDIA_IN_A_SEPARATE_THREAD
    while (!parsingCompleted() && _videoInfo.get() && _videoFrames.empty())
    {
        const_cast<MediaParser*>(this)->parseNextChunk();
    }
#endif

    if (_videoFrames.empty())
    {
        if (_audioFrames.empty())
        {
            return false;
        }
        else
        {
            ts = _audioFrames.front()->timestamp;
            return true;
        }
    }
    else
    {
        if (_audioFrames.empty())
        {
            ts = _videoFrames.front()->timestamp();
            return true;
        }
        else
        {
            ts = std::min(_videoFrames.front()->timestamp(),
                          _audioFrames.front()->timestamp);
            return true;
        }
    }
}

bool
MediaParser::nextVideoFrameTimestamp(std::uint64_t& ts) const
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	boost::mutex::scoped_lock lock(_qMutex);
#endif // def LOAD_MEDIA_IN_A_SEPARATE_THREAD
	const EncodedVideoFrame* ef = peekNextVideoFrame();
	if ( ! ef ) return false;
	ts = ef->timestamp();
	return true;
}

std::unique_ptr<EncodedVideoFrame>
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

	std::unique_ptr<EncodedVideoFrame> ret;
	if (_videoFrames.empty()) return ret;
	ret.reset(_videoFrames.front());
	_videoFrames.pop_front();
#ifdef GNASH_DEBUG_MEDIAPARSER
	log_debug("nextVideoFrame: waking up parser (in case it was sleeping)");
#endif // GNASH_DEBUG_MEDIAPARSER
	_parserThreadWakeup.notify_all(); // wake it up, to refill the buffer, SHOULDN'T WE HOLD A LoCK HERE?
	return ret;
}

std::unique_ptr<EncodedAudioFrame>
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

	std::unique_ptr<EncodedAudioFrame> ret;
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
MediaParser::nextAudioFrameTimestamp(std::uint64_t& ts) const
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	boost::mutex::scoped_lock lock(_qMutex);
#endif // def LOAD_MEDIA_IN_A_SEPARATE_THREAD
	const EncodedAudioFrame* ef = peekNextAudioFrame();
	if ( ! ef ) return false;
	ts = ef->timestamp;
	return true;
}

/*private*/
const EncodedAudioFrame*
MediaParser::peekNextAudioFrame() const
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
    // TODO: assert _qMutex is locked by this thread
#else // ndef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	while (!parsingCompleted() && _audioInfo.get() && _audioFrames.empty())
	{
		const_cast<MediaParser*>(this)->parseNextChunk();
	}
#endif
	if (!_audioInfo.get() || _audioFrames.empty()) return nullptr;
	return _audioFrames.front();
}

void
MediaParser::stopParserThread()
{
	if ( _parserThread.get() )
	{
		requestParserThreadKill();
		_parserThread->join();
		_parserThread.reset();
	}
}

MediaParser::~MediaParser()
{
	stopParserThread();

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
MediaParser::pushEncodedAudioFrame(std::unique_ptr<EncodedAudioFrame> frame)
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	boost::mutex::scoped_lock lock(_qMutex);
#endif
	
    // Find location to insert this new frame to, so that
    // timestamps are sorted
    //
    AudioFrames::iterator loc = _audioFrames.end();
    if ( ! _audioFrames.empty() ) {
        size_t gap=0;
        AudioFrames::reverse_iterator i=_audioFrames.rbegin();
        for (AudioFrames::reverse_iterator e=_audioFrames.rend(); i!=e; ++i)
        {
            if ( (*i)->timestamp <= frame->timestamp ) break;
            ++gap;
        }

        loc = i.base();

        if ( gap ) {
            log_debug("Timestamp of last %d/%d audio frames in queue "
                "greater then timestamp in the frame being "
                "inserted to it (%d).", gap, _audioFrames.size(),
                frame->timestamp);
        }
    }

	//log_debug("Inserting audio frame with timestamp %d", frame->timestamp);
	_audioFrames.insert(loc, frame.release());

#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	// if the push reaches a "buffer full" condition, or if we find the parsing
	// to be completed, wait to be waken up
	waitIfNeeded(lock);
#endif
}

void
MediaParser::pushEncodedVideoFrame(std::unique_ptr<EncodedVideoFrame> frame)
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	boost::mutex::scoped_lock lock(_qMutex);
#endif

    // Find location to insert this new frame to, so that
    // timestamps are sorted
    //
    VideoFrames::iterator loc = _videoFrames.end();
    if ( ! _videoFrames.empty() ) {
        size_t gap=0;
        VideoFrames::reverse_iterator i=_videoFrames.rbegin();
        for (VideoFrames::reverse_iterator e=_videoFrames.rend(); i!=e; ++i)
        {
            if ( (*i)->timestamp() <= frame->timestamp() ) break;
            ++gap;
        }

        loc = i.base();

        if ( gap ) {
            log_debug("Timestamp of last %d/%d video frames in queue "
                "greater then timestamp() in the frame being "
                "inserted to it (%d).", gap, _videoFrames.size(),
                frame->timestamp());
        }
    }

	//log_debug("Pushing video frame with timestamp %d", frame->timestamp());
	_videoFrames.insert(loc, frame.release());

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
	if (( pc || (bf && ic)) && !parserThreadKillRequested()) // TODO: or seekRequested ?
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
	std::uint64_t bl = getBufferLengthNoLock();
	std::uint64_t bt = getBufferTime();
#ifdef GNASH_DEBUG_MEDIAPARSER
	log_debug("MediaParser::bufferFull: %d/%d", bl, bt);
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
		gnashSleep(100); // thread switch 

		// check for parsing complete
		// TODO: have a setParsingComplete() function
		//       exposed in base class for taking care
		//       of this on appropriate time.
		boost::mutex::scoped_lock lock(_qMutex);
		waitIfNeeded(lock);
	}
}


void
MediaParser::fetchMetaTags(OrderedMetaTags& /*tags*/, std::uint64_t /*ts*/)
{
}


std::ostream&
operator<< (std::ostream& os, const VideoInfo& vi)
{
	os << "codec:" << vi.codec << " (type " << vi.type << ") - "
	   << "size:" << vi.width << "x" << vi.height << " - "
	   << "frameRate:" << vi.frameRate << " - "
	   << "duration:" << vi.duration;
	return os;
}

std::ostream&
operator<< (std::ostream& os, const videoCodecType& t)
{
    switch (t)
    {
        case VIDEO_CODEC_H263:
            os << "H263";
            break;
        case VIDEO_CODEC_SCREENVIDEO:
            os << "Screenvideo";
            break;
        case VIDEO_CODEC_VP6:
            os << "VP6";
            break;
        case VIDEO_CODEC_VP6A:
            os << "VP6A";
            break;
        case VIDEO_CODEC_SCREENVIDEO2:
            os << "Screenvideo2";
            break;
        case VIDEO_CODEC_H264:
            os << "H264";
            break;
        default:
            os << "unknown/invalid codec " << static_cast<int>(t);
            break;
    }
    return os;
}

std::ostream&
operator<< (std::ostream& os, const audioCodecType& t)
{
    switch (t)
    {
        case AUDIO_CODEC_RAW:
            os << "Raw";
            break;
        case AUDIO_CODEC_ADPCM:
            os << "ADPCM";
            break;
        case AUDIO_CODEC_MP3:
            os << "MP3";
            break;
        case AUDIO_CODEC_UNCOMPRESSED:
            os << "Uncompressed";
            break;
        case AUDIO_CODEC_NELLYMOSER_8HZ_MONO:
            os << "Nellymoser 8Hz mono";
            break;
        case AUDIO_CODEC_NELLYMOSER:
            os << "Nellymoser";
            break;
        case AUDIO_CODEC_AAC:
            os << "Advanced Audio Coding";
            break;
        case AUDIO_CODEC_SPEEX:
            os << "Speex";
            break;
        default:
            os << "unknown/invalid codec " << static_cast<int>(t);
            break;
    }
    return os;
}

} // end of gnash::media namespace
} // end of gnash namespace

#undef PADDING_BYTES
#undef READ_CHUNKS 
