// NetStreamFfmpeg.cpp:  Network streaming for FFMPEG video library, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifdef USE_FFMPEG

#include "NetStreamFfmpeg.h"
#include "log.h"
#include "fn_call.h"
#include "NetStream.h"
#include "render.h"	
#include "movie_root.h"
#include "sound_handler.h"

#include "MediaParser.h" 
#include "VideoDecoder.h"
#include "AudioDecoder.h"
#include "MediaHandler.h"
#include "VM.h"

#include "SystemClock.h"
#include "gnash.h" // get_sound_handler()


#include <boost/scoped_array.hpp>
#include <algorithm> // std::min

/// Define this to add debugging prints for locking
//#define GNASH_DEBUG_THREADS

// Define the following macro to have status notification handling debugged
//#define GNASH_DEBUG_STATUS

// Define the following macro to have decoding activity  debugged
//#define GNASH_DEBUG_DECODING 1

namespace gnash {

static void
cleanQueue(NetStreamFfmpeg::AudioQueue::value_type data)
{
    delete data;
}

// AS-volume adjustment
void adjust_volume(boost::int16_t* data, int size, int volume)
{
	for (int i=0; i < size*0.5; i++) {
		data[i] = data[i] * volume/100;
	}
}


NetStreamFfmpeg::NetStreamFfmpeg()
	:

	_decoding_state(DEC_NONE),

	// TODO: if audio is available, use _audioClock instead of SystemClock
	// as additional source
	_playbackClock(new InterruptableVirtualClock(new SystemClock)),
	_playHead(_playbackClock.get()), 

	_soundHandler(get_sound_handler()),
	_mediaHandler(media::MediaHandler::get()),
	_audioQueueSize(0)
{
}

NetStreamFfmpeg::~NetStreamFfmpeg()
{
	close(); // close will also detach from sound handler
}


void NetStreamFfmpeg::pause( PauseMode mode )
{
	log_debug("::pause(%d) called ", mode);
	switch ( mode )
	{
		case pauseModeToggle:
			if ( _playHead.getState() == PlayHead::PLAY_PAUSED) unpausePlayback();
			else pausePlayback();
			break;
		case pauseModePause:
			pausePlayback();
			break;
		case pauseModeUnPause:
			unpausePlayback();
			break;
		default:
			break;
	}

}

void NetStreamFfmpeg::close()
{
	GNASH_REPORT_FUNCTION;

    // Delete any samples in the audio queue.
	{
		boost::mutex::scoped_lock lock(_audioQueueMutex);
		std::for_each(_audioQueue.begin(), _audioQueue.end(), &cleanQueue);
    }

	// When closing gnash before playback is finished, the soundhandler 
	// seems to be removed before netstream is destroyed.
	detachAuxStreamer();

	m_imageframe.reset();

	stopAdvanceTimer();

}

void
NetStreamFfmpeg::play(const std::string& c_url)
{
	// Is it already playing ?
	if ( m_parser.get() )
	{
		// TODO: check what to do in these cases
		log_error("FIXME: NetStream.play() called while already streaming");
		return;
	}

	// Does it have an associated NetConnection ?
	if ( ! _netCon )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("No NetConnection associated with this NetStream, won't play"));
		);
		return;
	}

	url = c_url;

	// Remove any "mp3:" prefix. Maybe should use this to mark as audio-only
	if (url.compare(0, 4, std::string("mp3:")) == 0)
	{
		url = url.substr(4);
	}

	// TODO: check what is this needed for, I'm not sure it would be needed..
	url = _netCon->validateURL(url);
	if (url.empty())
	{
		log_error("Couldn't load URL %s", c_url);
		return;
	}

	log_security( _("Connecting to movie: %s"), url );

	StreamProvider& streamProvider = StreamProvider::getDefaultInstance();
	_inputStream = streamProvider.getStream(url);

	if ( ! _inputStream.get() )
	{
		log_error( _("Gnash could not open this url: %s"), url );
		setStatus(streamNotFound);
		return;
	}

	// We need to start playback
	if (!startPlayback())
	{
		log_error("NetStream.play(%s): failed starting playback", c_url);
		return;
	}

	// We need to restart the audio
	attachAuxStreamer();

	return;
}

void
NetStreamFfmpeg::initVideoDecoder(media::MediaParser& parser)
{
	// Get video info from the parser
	media::VideoInfo* videoInfo = parser.getVideoInfo();
	if (!videoInfo) {
		log_debug("No video in NetStream stream");
		return;
	}

	assert ( _mediaHandler ); // caller should check this

    try {
	    _videoDecoder = _mediaHandler->createVideoDecoder(*videoInfo);
	}
	catch (MediaException& e) {
	    log_error("NetStream: Could not create Video decoder: %s", e.what());
	}

}


/* private */
void
NetStreamFfmpeg::initAudioDecoder(media::MediaParser& parser)
{
	// Get audio info from the parser
	media::AudioInfo* audioInfo =  parser.getAudioInfo();
	if (!audioInfo) {
		log_debug("No audio in NetStream input");
		return;
	}

	assert ( _mediaHandler ); // caller should check this

    try {
	    _audioDecoder = _mediaHandler->createAudioDecoder(*audioInfo);
	}
	catch (MediaException& e) {
	    log_error("Could not create Audio decoder: %s", e.what());
	}

}


bool
NetStreamFfmpeg::startPlayback()
{
	assert(_inputStream.get());
	assert(_inputStream->tell() == 0);

	inputPos = 0;

	if ( ! _mediaHandler )
	{
		LOG_ONCE( log_error(_("No Media handler registered, can't "
			"parse NetStream input")) );
		return false;
	}
	m_parser = _mediaHandler->createMediaParser(_inputStream);
	assert(!_inputStream.get());

	if ( ! m_parser.get() )
	{
		log_error(_("Unable to create parser for NetStream input"));
		// not necessarely correct, the stream might have been found...
		setStatus(streamNotFound);
		return false;
	}

	m_parser->setBufferTime(m_bufferTime);

	initVideoDecoder(*m_parser); 
	initAudioDecoder(*m_parser); 

	_playHead.init(_videoDecoder.get(), _audioDecoder.get());
	_playHead.setState(PlayHead::PLAY_PLAYING);

	decodingStatus(DEC_BUFFERING);
	_playbackClock->pause(); // NOTE: should be paused already

	// Register ::advance callback
	startAdvanceTimer();

#ifdef GNASH_DEBUG_STATUS
	log_debug("Setting playStart status");
#endif // GNASH_DEBUG_STATUS
	setStatus(playStart);

	return true;
}


// audio callback is running in sound handler thread
bool NetStreamFfmpeg::audio_streamer(void *owner, boost::uint8_t *stream, int len)
{
	//GNASH_REPORT_FUNCTION;

	NetStreamFfmpeg* ns = static_cast<NetStreamFfmpeg*>(owner);

	boost::mutex::scoped_lock lock(ns->_audioQueueMutex);

#if 0
	log_debug("audio_streamer called, audioQueue size: %d, "
		"requested %d bytes of fill-up",
		ns->_audioQueue.size(), len);
#endif


	while (len > 0)
	{

		if ( ns->_audioQueue.empty() )
		{
			break;
		}

    		media::raw_mediadata_t* samples = ns->_audioQueue.front();

		int n = std::min<int>(samples->m_size, len);
		memcpy(stream, samples->m_ptr, n);
		stream += n;
		samples->m_ptr += n;
		samples->m_size -= n;
		len -= n;

		if (samples->m_size == 0)
		{
			delete samples;
			ns->_audioQueue.pop_front();
		}

		ns->_audioQueueSize -= n; // we consumed 'n' bytes here 

	}

	return true;
}

std::auto_ptr<image::ImageBase> 
NetStreamFfmpeg::getDecodedVideoFrame(boost::uint32_t ts)
{
	assert(_videoDecoder.get()); // caller should check this

	std::auto_ptr<image::ImageBase> video;

	assert(m_parser.get());
	if ( ! m_parser.get() )
	{
		log_error("getDecodedVideoFrame: no parser available");
		return video; // no parser, no party
	}

	boost::uint64_t nextTimestamp;
	bool parsingComplete = m_parser->parsingCompleted();
	if ( ! m_parser->nextVideoFrameTimestamp(nextTimestamp) )
	{
#ifdef GNASH_DEBUG_DECODING
		log_debug("getDecodedVideoFrame(%d): "
			"no more video frames in input "
			"(nextVideoFrameTimestamp returned false, "
			"parsingComplete=%d)",
			ts, parsingComplete);
#endif // GNASH_DEBUG_DECODING

		if ( parsingComplete )
		{
			decodingStatus(DEC_STOPPED);
#ifdef GNASH_DEBUG_STATUS
			log_debug("getDecodedVideoFrame setting playStop status (parsing complete and nextVideoFrameTimestamp() returned false)");
#endif
			setStatus(playStop);
		}
		return video;
	}

	if ( nextTimestamp > ts )
	{
#ifdef GNASH_DEBUG_DECODING
		log_debug("%p.getDecodedVideoFrame(%d): next video frame is in the future (%d)",
			this, ts, nextTimestamp);
#endif // GNASH_DEBUG_DECODING
		return video; // next frame is in the future
	}

	// Loop until a good frame is found
	while ( 1 )
	{
    		video = decodeNextVideoFrame();
		if ( ! video.get() )
		{
			log_error("nextVideoFrameTimestamp returned true, "
				"but decodeNextVideoFrame returned null, "
				"I don't think this should ever happen");
			break;
		}

		if ( ! m_parser->nextVideoFrameTimestamp(nextTimestamp) )
		{
			// the one we decoded was the last one
#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.getDecodedVideoFrame(%d): last video frame decoded "
				"(should set playback status to STOP?)", this, ts);
#endif // GNASH_DEBUG_DECODING
			break;
		}
		if ( nextTimestamp > ts )
		{
			// the next one is in the future, we'll return this one.
#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.getDecodedVideoFrame(%d): "
				"next video frame is in the future, "
				"we'll return this one",
				this, ts);
#endif // GNASH_DEBUG_DECODING
			break; // the one we decoded
		}
	}

	return video;
}

std::auto_ptr<image::ImageBase> 
NetStreamFfmpeg::decodeNextVideoFrame()
{
	std::auto_ptr<image::ImageBase> video;

	if ( ! m_parser.get() )
	{
		log_error("decodeNextVideoFrame: no parser available");
		return video; // no parser, no party
	}

	std::auto_ptr<media::EncodedVideoFrame> frame = m_parser->nextVideoFrame(); 
	if ( ! frame.get() )
	{
#ifdef GNASH_DEBUG_DECODING
		log_debug("%p.decodeNextVideoFrame(): "
			"no more video frames in input",
			this);
#endif // GNASH_DEBUG_DECODING
		return video;
	}

#if 0 // TODO: check if the video is a cue point, if so, call processNotify(onCuePoint, object..)
      // NOTE: should only be done for SWF>=8 ?
	if ( 1 ) // frame->isKeyFrame() )
	{
		as_object* infoObj = new as_object();
		string_table& st = getVM().getStringTable();
		infoObj->set_member(st.find("time"), as_value(double(frame->timestamp())));
		infoObj->set_member(st.find("type"), as_value("navigation"));
		processNotify("onCuePoint", infoObj);
	}
#endif

	assert( _videoDecoder.get() ); // caller should check this
	assert( ! _videoDecoder->peek() ); // everything we push, we'll pop too..

	_videoDecoder->push(*frame);
	video = _videoDecoder->pop();
	if ( ! video.get() )
	{
		// TODO: tell more about the failure
		log_error("Error decoding encoded video frame in NetSTream input");
	}

	return video;
}

media::raw_mediadata_t*
NetStreamFfmpeg::decodeNextAudioFrame()
{
	assert ( m_parser.get() );

	std::auto_ptr<media::EncodedAudioFrame> frame = m_parser->nextAudioFrame(); 
	if ( ! frame.get() )
	{
#ifdef GNASH_DEBUG_DECODING
		log_debug("%p.decodeNextAudioFrame: "
			"no more video frames in input",
			this);
#endif // GNASH_DEBUG_DECODING
		return 0;
	}

    	media::raw_mediadata_t* raw = new media::raw_mediadata_t();
	raw->m_data = _audioDecoder->decode(*frame, raw->m_size); 

	if ( _audioController ) // TODO: let the sound_handler do this .. sounds cleaner
	{
		character* ch = _audioController->get();
		if ( ch )
		{
			int vol = ch->getWorldVolume();
			if ( vol != 100 )
			{
				// NOTE: adjust_volume assumes samples 
				// are 16 bits in size, and signed.
				// Size is still given in bytes..
				adjust_volume(reinterpret_cast<boost::int16_t*>(raw->m_data), raw->m_size, vol);
			}
		}
	}

#ifdef GNASH_DEBUG_DECODING
	log_debug("NetStreamFfmpeg::decodeNextAudioFrame: "
		"%d bytes of encoded audio "
		"decoded to %d bytes",
		frame->dataSize,
		raw->m_size);
#endif // GNASH_DEBUG_DECODING

	raw->m_ptr = raw->m_data; // no idea what this is needed for
	raw->m_pts = frame->timestamp;

	return raw;
}

bool NetStreamFfmpeg::decodeMediaFrame()
{
	return false;
}

void
NetStreamFfmpeg::seek(boost::uint32_t posSeconds)
{
	GNASH_REPORT_FUNCTION;

	// We'll mess with the input here
	if ( ! m_parser.get() )
	{
		log_debug("NetStreamFfmpeg::seek(%d): no parser, no party", posSeconds);
		return;
	}

	// Don't ask me why, but NetStream::seek() takes seconds...
	boost::uint32_t pos = posSeconds*1000;

	// We'll pause the clock source and mark decoders as buffering.
	// In this way, next advance won't find the source time to 
	// be a lot of time behind and chances to get audio buffer
	// overruns will reduce.
	// ::advance will resume the playbackClock if DEC_BUFFERING...
	//
	_playbackClock->pause();

	// Seek to new position
	boost::uint32_t newpos = pos;
	if ( ! m_parser->seek(newpos) )
	{
		//log_error("Seek to invalid time");
#ifdef GNASH_DEBUG_STATUS
		log_debug("Setting invalidTime status");
#endif
		setStatus(invalidTime);
		_playbackClock->resume(); // we won't be *BUFFERING*, so resume now
		return;
	}
	log_debug("m_parser->seek(%d) returned %d", pos, newpos);

	{ // cleanup audio queue, so won't be consumed while seeking
		boost::mutex::scoped_lock lock(_audioQueueMutex);
		std::for_each(_audioQueue.begin(), _audioQueue.end(), &cleanQueue);
		_audioQueue.clear();
	}
	
	// 'newpos' will always be on a keyframe (supposedly)
	_playHead.seekTo(newpos);
	decodingStatus(DEC_BUFFERING); 
	
	refreshVideoFrame(true);
}

void
NetStreamFfmpeg::parseNextChunk()
{
	// If we parse too much we might block
	// the main thread, if we parse too few
	// we'll get bufferEmpty often.
	// I guess 2 chunks (frames) would be fine..
	//
	m_parser->parseNextChunk();
	m_parser->parseNextChunk();
}

void
NetStreamFfmpeg::refreshAudioBuffer()
{
	assert ( m_parser.get() );

#ifdef GNASH_DEBUG_DECODING
	// bufferLength() would lock the mutex (which we already hold),
	// so this is to avoid that.
	boost::uint32_t parserTime = m_parser->getBufferLength();
	boost::uint32_t playHeadTime = time();
	boost::uint32_t bufferLen = parserTime > playHeadTime ? parserTime-playHeadTime : 0;
#endif

	if ( _playHead.getState() == PlayHead::PLAY_PAUSED )
	{
#ifdef GNASH_DEBUG_DECODING
		log_debug("%p.refreshAudioBuffer: doing nothing as playhead is paused - "
			"bufferLength=%d/%d",
			this, bufferLength(), m_bufferTime);
#endif // GNASH_DEBUG_DECODING
		return;
	}

	if ( _playHead.isAudioConsumed() ) 
	{
#ifdef GNASH_DEBUG_DECODING
		log_debug("%p.refreshAudioBuffer: doing nothing "
			"as current position was already decoded - "
			"bufferLength=%d/%d",
			this, bufferLen, m_bufferTime);
#endif // GNASH_DEBUG_DECODING
		return;
	}

	// Calculate the current time
	boost::uint64_t curPos = _playHead.getPosition();

#ifdef GNASH_DEBUG_DECODING
	log_debug("%p.refreshAudioBuffer: currentPosition=%d, playHeadState=%d, bufferLength=%d, bufferTime=%d",
		this, curPos, _playHead.getState(), bufferLen, m_bufferTime);
#endif // GNASH_DEBUG_DECODING

	// TODO: here we should fetch all frames up to the one with timestamp >= curPos
	//       and push them into the buffer to be consumed by audio_streamer
	pushDecodedAudioFrames(curPos);
}

void
NetStreamFfmpeg::pushDecodedAudioFrames(boost::uint32_t ts)
{
	assert(m_parser.get());

	// nothing to do if we don't have an audio decoder
	if ( ! _audioDecoder.get() ) return;

	bool consumed = false;

	boost::uint64_t nextTimestamp;
	while ( 1 )
	{

		boost::mutex::scoped_lock lock(_audioQueueMutex);

		// The sound_handler mixer will pull decoded
		// audio frames off the _audioQueue whenever 
		// new audio has to be played.
		// This is done based on the output frequency,
		// currently hard-coded to be 44100 samples per second.
		//
		// Our job here would be to provide that much data.
		// We're in an ::advance loop, so must provide enough
		// data for the mixer to fetch till next advance.
		// Assuming we know the ::advance() frame rate (which we don't
		// yet) the computation would be something along these lines:
		//
		//    44100/1 == samplesPerAdvance/secsPerAdvance
		//    samplesPerAdvance = secsPerAdvance*(44100/1)
		//
		// For example, at 12FPS we have:
		//
		//   secsPerAdvance = 1/12 = .083333
		//   samplesPerAdvance = .08333*44100 =~ 3675
		//
		// Now, to know how many samples are on the queue
		// we need to know the size in bytes of each sample.
		// If I'm not wrong this is again hard-coded to 2 bytes,
		// so we'd have:
		//
		//   bytesPerAdvance = samplesPerAdvance / sampleSize
		//   bytesPerAdvance = 3675 / 2 =~ 1837
		//
		// Finally we'll need to find number of bytes in the
		// queue to really tell how many there are (don't think
		// it's a fixed size for each element).
		//
		// For now we use the hard-coded value of 20, arbitrarely
		// assuming there is an average of 184 samples per frame.
		//
		// - If we push too few samples, we'll hear silence gaps (underrun)
		// - If we push too many samples the audio mixer consumer
		//   won't be able to consume all before our next filling
		//   iteration (overrun)
		//
		// For *underrun* conditions we kind of have an handling, that is
		// sending the BufferEmpty event and closing the time tap (this is
		// done by ::advance directly).
		//
		// For *overrun* conditions we currently don't have any handling.
		// One possibility could be closing the time tap till we've done
		// consuming the queue.
		//
		//

		float swfFPS = 25; // TODO: get this host app (gnash -d affects this)
		double msecsPerAdvance = 10000/swfFPS;

		//static const int outSampleSize = 2;     // <--- 2 is output sample size
		//static const int outSampleFreq = 44100; // <--- 44100 is output audio frequency
		//int samplesPerAdvance = (int)std::floor(secsPerAdvance*outSampleFreq); // round up
		//unsigned int bufferLimit = outSampleSize*samplesPerAdvance;

		unsigned int bufferLimit = 20;
		unsigned int bufferSize = _audioQueue.size();
		if ( bufferSize > bufferLimit )
		{
			// we won't buffer more then 'bufferLimit' frames in the queue
			// to avoid ending up with a huge queue which will take some
			// time before being consumed by audio mixer, but still marked
			// as "consumed". Keeping decoded frames buffer low would also
			// reduce memory use.
			//
			// The alternative would be always decode on demand from the
			// audio consumer thread, but would introduce a lot of thread-safety
			// issues: playhead would need protection, input would need protection.
			//
//#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.pushDecodedAudioFrames(%d) : buffer overrun (%d/%d).",
				this, ts, bufferSize, bufferLimit);
//#endif // GNASH_DEBUG_DECODING

			// we may want to pause the playbackClock here...
			_playbackClock->pause();

			return;
		}

		lock.unlock(); // no need to keep the audio queue locked while decoding..

		bool parsingComplete = m_parser->parsingCompleted();
		if ( ! m_parser->nextAudioFrameTimestamp(nextTimestamp) )
		{
#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.pushDecodedAudioFrames(%d): "
				"no more audio frames in input "
				"(nextAudioFrameTimestamp returned false, parsingComplete=%d)",
				this, ts, parsingComplete);
#endif // GNASH_DEBUG_DECODING

			if ( parsingComplete )
			{
				consumed = true;
				decodingStatus(DEC_STOPPED);
#ifdef GNASH_DEBUG_STATUS
				log_debug("pushDecodedAudioFrames setting playStop status (parsing complete and nextAudioFrameTimestamp returned false)");
#endif
				setStatus(playStop);
			}

			break;
		}

		if ( nextTimestamp > ts )
		{
#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.pushDecodedAudioFrames(%d): "
				"next audio frame is in the future (%d)",
				this, ts, nextTimestamp);
#endif // GNASH_DEBUG_DECODING
			consumed = true;

			if ( nextTimestamp > ts+msecsPerAdvance ) break; // next frame is in the future
		}

		media::raw_mediadata_t* audio = decodeNextAudioFrame();
		if ( ! audio )
		{
			log_error("nextAudioFrameTimestamp returned true, "
				"but decodeNextAudioFrame returned null, "
				"I don't think this should ever happen");
			break;
		}

		lock.lock(); // now needs locking

#ifdef GNASH_DEBUG_DECODING
		// this one we might avoid :) -- a less intrusive logging could
		// be take note about how many things we're pushing over
		log_debug("pushDecodedAudioFrames(%d) pushing %dth frame with timestamp %d", ts, _audioQueue.size()+1, nextTimestamp); 
#endif

		if ( _auxStreamerAttached )
		{
			_audioQueue.push_back(audio);
			_audioQueueSize += audio->m_size;
		}
		else // don't bother pushing audio to the queue, nobody would consume it...
		{
			delete audio;
		}
	}

	// If we consumed audio of current position, feel free to advance if needed,
	// resuming playbackClock too..
	if ( consumed )
	{
		// resume the playback clock, assuming the
		// only reason for it to be paused is we
		// put in pause mode due to buffer overrun
		// (ie: the sound handler is slow at consuming
		// the audio data).
#ifdef GNASH_DEBUG_DECODING
		log_debug("resuming playback clock on audio consume");
#endif // GNASH_DEBUG_DECODING
		assert(decodingStatus()!=DEC_BUFFERING);
		_playbackClock->resume();

		_playHead.setAudioConsumed();
	}

}


void
NetStreamFfmpeg::refreshVideoFrame(bool alsoIfPaused)
{
	assert ( m_parser.get() );

	// nothing to do if we don't have a video decoder
	if ( ! _videoDecoder.get() )
	{
		//log_debug("refreshVideoFrame: no video decoder, nothing to do");
		return;
	}

#ifdef GNASH_DEBUG_DECODING
	// bufferLength() would lock the mutex (which we already hold),
	// so this is to avoid that.
	boost::uint32_t parserTime = m_parser->getBufferLength();
	boost::uint32_t playHeadTime = time();
	boost::uint32_t bufferLen = bufferLength();
#endif

	if ( ! alsoIfPaused && _playHead.getState() == PlayHead::PLAY_PAUSED )
	{
#ifdef GNASH_DEBUG_DECODING
		log_debug("%p.refreshVideoFrame: doing nothing as playhead is paused - "
			"bufferLength=%d, bufferTime=%d",
			this, bufferLen, m_bufferTime);
#endif // GNASH_DEBUG_DECODING
		return;
	}

	if ( _playHead.isVideoConsumed() ) 
	{
#ifdef GNASH_DEBUG_DECODING
		log_debug("%p.refreshVideoFrame: doing nothing "
			"as current position was already decoded - "
			"bufferLength=%d, bufferTime=%d",
			this, bufferLen, m_bufferTime);
#endif // GNASH_DEBUG_DECODING
		return;
	}

	// Calculate the current time
	boost::uint64_t curPos = _playHead.getPosition();

#ifdef GNASH_DEBUG_DECODING
	log_debug("%p.refreshVideoFrame: currentPosition=%d, playHeadState=%d, bufferLength=%d, bufferTime=%d",
		this, curPos, _playHead.getState(), bufferLen, m_bufferTime);
#endif // GNASH_DEBUG_DECODING

	// Get next decoded video frame from parser, will have the lowest timestamp
	std::auto_ptr<image::ImageBase> video = getDecodedVideoFrame(curPos);

	// to be decoded or we're out of data
	if (!video.get())
	{
		if ( decodingStatus() == DEC_STOPPED )
		{
#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.refreshVideoFrame(): "
				"no more video frames to decode "
				"(DEC_STOPPED, null from getDecodedVideoFrame)",
				this);
#endif // GNASH_DEBUG_DECODING
		}
		else
		{
#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.refreshVideoFrame(): "
				"last video frame was good enough "
				"for current position",
				this);
#endif // GNASH_DEBUG_DECODING
			// There no video but decoder is still running
			// not much to do here except wait for next call
			//assert(decodingStatus() == DEC_BUFFERING);
		}

	}
	else
	{
		m_imageframe = video; // ownership transferred
		assert(!video.get());
		// A frame is ready for pickup
		if ( _invalidatedVideoCharacter )
		{
			_invalidatedVideoCharacter->set_invalidated();

			// NOTE: setting the newFrameReady flag this is not needed anymore,
			// we don't realy on newFrameReady() call anyore to invalidate the video character
			//m_newFrameReady = true;
		}
	}

	// We consumed video of current position, feel free to advance if needed
	_playHead.setVideoConsumed();


}


void
NetStreamFfmpeg::advance()
{
	// Check if there are any new status messages, and if we should
	// pass them to a event handler
	processStatusNotifications();

	// Nothing to do if we don't have a parser
	if ( ! m_parser.get() ) return;

	if ( decodingStatus() == DEC_STOPPED )
	{
		//log_debug("NetStreamFfmpeg::advance: dec stopped...");
		// nothing to do if we're stopped...
		return;
	}

	bool parsingComplete = m_parser->parsingCompleted();
#ifndef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	if ( ! parsingComplete ) parseNextChunk();
#endif

	size_t bufferLen = bufferLength();

	// Check decoding status 
	if ( decodingStatus() == DEC_DECODING && bufferLen == 0 )
	{
		if ( ! parsingComplete )
		{
#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.advance: buffer empty while decoding,"
				" setting buffer to buffering and pausing playback clock",
				this);
#endif // GNASH_DEBUG_DECODING
#ifdef GNASH_DEBUG_STATUS
			log_debug("Setting bufferEmpty status");
#endif
			setStatus(bufferEmpty);
			decodingStatus(DEC_BUFFERING);
			_playbackClock->pause();
		}
		else
		{
#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.advance : bufferLength=%d, parsing completed",
 				this, bufferLen);
#endif // GNASH_DEBUG_DECODING
			// set playStop ? (will be done later for now)
		}
	}

	if ( decodingStatus() == DEC_BUFFERING )
	{
		if ( bufferLen < m_bufferTime && ! parsingComplete )
		{
#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.advance: buffering"
				" - position=%d, buffer=%d/%d",
				this, _playHead.getPosition(), bufferLen, m_bufferTime);
#endif // GNASH_DEBUG_DECODING

			// The very first video frame we want to provide
			// as soon as possible (if not paused),
			// reguardless bufferLength...
			if ( ! m_imageframe.get() && _playHead.getState() != PlayHead::PLAY_PAUSED )
			{
				refreshVideoFrame(true);
			}

			return;
		}

#ifdef GNASH_DEBUG_DECODING
		log_debug("%p.advance: buffer full (or parsing completed), resuming playback clock"
			" - position=%d, buffer=%d/%d",
			this, _playHead.getPosition(), bufferLen, m_bufferTime);
#endif // GNASH_DEBUG_DECODING

		setStatus(bufferFull);
		decodingStatus(DEC_DECODING);
		_playbackClock->resume();
	}

	// Find video frame with the most suited timestamp in the video queue,
	// and put it in the output image frame.
	refreshVideoFrame();

	// Refill audio buffer to consume all samples
	// up to current playhead
	refreshAudioBuffer();

	// Process media tags
	m_parser->processTags(_playHead.getPosition(), this, getVM());
}

boost::int32_t
NetStreamFfmpeg::time()
{
	return _playHead.getPosition();
}

void NetStreamFfmpeg::pausePlayback()
{
	GNASH_REPORT_FUNCTION;

	PlayHead::PlaybackStatus oldStatus = _playHead.setState(PlayHead::PLAY_PAUSED);

	// Disconnect the soundhandler if we were playing before
	if ( oldStatus == PlayHead::PLAY_PLAYING )
	{
		detachAuxStreamer();
	}
}

void NetStreamFfmpeg::unpausePlayback()
{
	GNASH_REPORT_FUNCTION;

	PlayHead::PlaybackStatus oldStatus = _playHead.setState(PlayHead::PLAY_PLAYING);

	// Re-connect to the soundhandler if we were paused before
	if ( oldStatus == PlayHead::PLAY_PAUSED )
	{
		attachAuxStreamer();
	}
}


long
NetStreamFfmpeg::bytesLoaded ()
{
  	if ( ! m_parser.get() )
	{
		log_debug("bytesLoaded: no parser, no party");
		return 0;
  	}

	return m_parser->getBytesLoaded();
}

long
NetStreamFfmpeg::bytesTotal ()
{
  	if ( ! m_parser.get() )
	{
		log_debug("bytesTotal: no parser, no party");
		return 0;
  	}

	return m_parser->getBytesTotal();
}

NetStreamFfmpeg::DecodingState
NetStreamFfmpeg::decodingStatus(DecodingState newstate)
{
	boost::mutex::scoped_lock lock(_state_mutex);

	if (newstate != DEC_NONE) {
		_decoding_state = newstate;
	}

	return _decoding_state;
}

void
NetStreamFfmpeg::attachAuxStreamer()
{
	if ( ! _soundHandler ) return;
	if ( _auxStreamerAttached )
	{
		log_debug("attachAuxStreamer called while already attached");
		// we do nonetheless, isn't specified by SoundHandler.h
		// whether or not this is legal...
	}

	try {
		_soundHandler->attach_aux_streamer(audio_streamer, (void*) this);
		_auxStreamerAttached = true;
	} catch (SoundException& e) {
		log_error("Could not attach NetStream aux streamer to sound handler: %s", e.what());
	}
}

void
NetStreamFfmpeg::detachAuxStreamer()
{
	if ( ! _soundHandler ) return;
	if ( !_auxStreamerAttached )
	{
		log_debug("detachAuxStreamer called while not attached");
		// we do nonetheless, isn't specified by SoundHandler.h
		// whether or not this is legal...
	}
	_soundHandler->detach_aux_streamer(this);
	_auxStreamerAttached = false;
}

} // gnash namespcae

#endif // USE_FFMPEG

