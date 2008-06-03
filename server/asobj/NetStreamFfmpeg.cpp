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

#include "SystemClock.h"
#include "gnash.h" // get_sound_handler()


#include <boost/scoped_array.hpp>
#include <algorithm> // std::min


#if defined(_WIN32) || defined(WIN32)
# include <windows.h>	// for sleep()
# define usleep(x) Sleep(x/1000)
#else
# include "unistd.h" // for usleep()
#endif

/// Define this to add debugging prints for locking
//#define GNASH_DEBUG_THREADS

// Define the following macro to have status notification handling debugged
//#define GNASH_DEBUG_STATUS

// Define the following macro to have decoding activity  debugged
//#define GNASH_DEBUG_DECODING

namespace {

// Used to free data in the AVPackets we create our self
void avpacket_destruct(AVPacket* av)
{
	delete [] av->data;
}

} // anonymous namespace


namespace gnash {


NetStreamFfmpeg::NetStreamFfmpeg()
	:

	_decoding_state(DEC_NONE),

#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	_parserThread(NULL),
	_parserThreadBarrier(2), // main and decoder threads
	_parserKillRequest(false),
#endif

	m_last_video_timestamp(0),
	m_last_audio_timestamp(0),

	_playbackClock(new InterruptableVirtualClock(new SystemClock)),
	_playHead(_playbackClock.get()), 

	m_unqueued_data(NULL),

	_decoderBuffer(0),
	_soundHandler(get_sound_handler())
{

	ByteIOCxt.buffer = NULL;
}

NetStreamFfmpeg::~NetStreamFfmpeg()
{
	if ( _decoderBuffer ) delete [] _decoderBuffer;

	close(); // close will also detach from sound handler

	delete m_imageframe;
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

#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	killParserThread();
#endif

	// When closing gnash before playback is finished, the soundhandler 
	// seems to be removed before netstream is destroyed.
	if (_soundHandler)
	{
		_soundHandler->detach_aux_streamer(this);
	}

	delete m_imageframe;
	m_imageframe = NULL;

	delete m_unqueued_data;
	m_unqueued_data = NULL;

	delete [] ByteIOCxt.buffer;

}

// ffmpeg callback function
int 
NetStreamFfmpeg::readPacket(void* opaque, boost::uint8_t* buf, int buf_size)
{

	NetStreamFfmpeg* ns = static_cast<NetStreamFfmpeg*>(opaque);

	assert( ns->_inputStream.get() );
	tu_file& in = *(ns->_inputStream);

	size_t ret = in.read_bytes(static_cast<void*>(buf), buf_size);
	ns->inputPos += ret; // what for ??
	return ret;

}

// ffmpeg callback function
offset_t 
NetStreamFfmpeg::seekMedia(void *opaque, offset_t offset, int whence)
{

	NetStreamFfmpeg* ns = static_cast<NetStreamFfmpeg*>(opaque);

	tu_file& in = *(ns->_inputStream);

	// Offset is absolute new position in the file
	if (whence == SEEK_SET)
	{	
		in.set_position(offset);
		ns->inputPos = offset; // what for ?!

	// New position is offset + old position
	}
	else if (whence == SEEK_CUR)
	{
		in.set_position(ns->inputPos + offset);
		ns->inputPos = ns->inputPos + offset; // what for ?!

	// New position is offset + end of file
	}
	else if (whence == SEEK_END)
	{
		// This is (most likely) a streamed file, so we can't seek to the end!
		// Instead we seek to 50.000 bytes... seems to work fine...
		in.set_position(50000);
		ns->inputPos = 50000; // what for ?!

	}

	return ns->inputPos; // ah, thats why ! :/
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
	_inputStream.reset( streamProvider.getStream( url ) );

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
	if (_soundHandler)
		_soundHandler->attach_aux_streamer(audio_streamer, this);

#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	// This starts the parser thread
	_parserThread = new boost::thread(boost::bind(NetStreamFfmpeg::parseAllInput, this)); 
	_parserThreadBarrier.wait();
#endif // LOAD_MEDIA_IN_A_SEPARATE_THREAD

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

	media::MediaHandler* mh = media::MediaHandler::get();
	assert ( mh ); // caller should check this

	_videoDecoder = mh->createVideoDecoder(*videoInfo);
	if ( ! _videoDecoder.get() )
		log_error(_("Could not create video decoder for codec %d"), videoInfo->codec);
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

	media::MediaHandler* mh = media::MediaHandler::get();
	assert ( mh ); // caller should check this

	_audioDecoder = mh->createAudioDecoder(*audioInfo);
	if ( ! _audioDecoder.get() )
		log_error(_("Could not create audio decoder for codec %d"), audioInfo->codec);
}


/// Probe the stream and try to figure out what the format is.
//
/// @param ns the netstream to use for reading
/// @return a pointer to the AVInputFormat structure containing
///         information about the input format, or NULL.
static AVInputFormat*
probeStream(NetStreamFfmpeg* ns)
{
	boost::scoped_array<boost::uint8_t> buffer(new boost::uint8_t[2048]);

	// Probe the file to detect the format
	AVProbeData probe_data;
	probe_data.filename = "";
	probe_data.buf = buffer.get();
	probe_data.buf_size = 2048;

	if (ns->readPacket(ns, probe_data.buf, probe_data.buf_size) < 1)
	{
 		log_error(_("Gnash could not read from movie url"));
 		return NULL;
	}

	return av_probe_input_format(&probe_data, 1);
}

bool
NetStreamFfmpeg::startPlayback()
{
	assert(_inputStream.get());
	assert(_inputStream->get_position() == 0);

	inputPos = 0;

	media::MediaHandler* mh = media::MediaHandler::get();
	if ( ! mh )
	{
		LOG_ONCE( log_error(_("No Media handler registered, can't "
			"parse NetStream input")) );
		return false;
	}
	m_parser = mh->createMediaParser(_inputStream);
	assert(!_inputStream.get());

	if ( ! m_parser.get() )
	{
		log_error(_("Unable to create parser for NetStream input"));
		// not necessarely correct, the stream might have been found...
		setStatus(streamNotFound);
		return false;
	}

	initVideoDecoder(*m_parser); 
	initAudioDecoder(*m_parser); 

	_playHead.init(_videoDecoder.get(), _audioDecoder.get());
	_playHead.setState(PlayHead::PLAY_PLAYING);

	decodingStatus(DEC_BUFFERING);

#ifdef GNASH_DEBUG_STATUS
	log_debug("Setting playStart status");
#endif // GNASH_DEBUG_STATUS
	setStatus(playStart);

	return true;
}


/// Copy RGB data from a source raw_mediadata_t to a destination image::rgb.
/// @param dst the destination image::rgb, which must already be initialized
///            with a buffer of size of at least src.m_size.
/// @param src the source raw_mediadata_t to copy data from. The m_size member
///            of this structure must be initialized.
/// @param width the width, in bytes, of a row of video data.
static void
rgbcopy(image::rgb* dst, media::raw_mediadata_t* src, int width)
{
  assert( src->m_size <= static_cast<boost::uint32_t>(dst->width() * dst->height() * 3) ); 

  boost::uint8_t* dstptr = dst->data();

  boost::uint8_t* srcptr = src->m_data;
  boost::uint8_t* srcend = src->m_data + src->m_size;

  while (srcptr < srcend) {
    memcpy(dstptr, srcptr, width);
    dstptr += dst->pitch();
    srcptr += width;
  }
}

#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
// to be run in parser thread
void NetStreamFfmpeg::parseAllInput(NetStreamFfmpeg* ns)
{
	//GNASH_REPORT_FUNCTION;

	ns->_parserThreadBarrier.wait();

	// Parse in a thread...
	while ( 1 )
	{
		// this one will lock _parserKillRequestMutex
		if ( ns->parserThreadKillRequested() ) break;

		{
			boost::mutex::scoped_lock lock(ns->_parserMutex);
			if ( ns->m_parser->parsingCompleted() ) break;
			ns->m_parser->parseNextTag();
		}

		usleep(10); // task switch (after lock was released!)
	}
}

void
NetStreamFfmpeg::killParserThread()
{
	GNASH_REPORT_FUNCTION;

	{
		boost::mutex::scoped_lock lock(_parserKillRequestMutex);
		_parserKillRequest = true;
	}

	// might as well be never started
	if ( _parserThread )
	{
		_parserThread->join();
	}

	delete _parserThread;
	_parserThread = NULL;
}

bool
NetStreamFfmpeg::parserThreadKillRequested()
{
	boost::mutex::scoped_lock lock(_parserKillRequestMutex);
	return _parserKillRequest;
}

#endif // LOAD_MEDIA_IN_A_SEPARATE_THREAD

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

	}

	return true;
}

std::auto_ptr<image::rgb> 
NetStreamFfmpeg::getDecodedVideoFrame(boost::uint32_t ts)
{
	assert(_videoDecoder.get()); // caller should check this

	std::auto_ptr<image::rgb> video;

	assert(m_parser.get());
	if ( ! m_parser.get() )
	{
		log_error("getDecodedVideoFrame: no parser available");
		return video; // no parser, no party
	}

	boost::uint64_t nextTimestamp;
	if ( ! m_parser->nextVideoFrameTimestamp(nextTimestamp) )
	{
#ifdef GNASH_DEBUG_DECODING
		log_debug("getDecodedVideoFrame(%d): no more video frames in input (nextVideoFrameTimestamp returned false)");
#endif // GNASH_DEBUG_DECODING
		decodingStatus(DEC_STOPPED);
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

std::auto_ptr<image::rgb> 
NetStreamFfmpeg::decodeNextVideoFrame()
{
	std::auto_ptr<image::rgb> video;

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
	boost::uint32_t decodedData=0;
	bool parseAudio = true; // I don't get this...
	raw->m_data = _audioDecoder->decode(frame->data.get(), frame->dataSize, raw->m_size, decodedData, parseAudio);

	if ( decodedData != frame->dataSize )
	{
		log_error("FIXME: not all data in EncodedAudioFrame was decoded, just %d/%d",
			frame->dataSize, decodedData);
	}

	//raw->m_stream_index = m_audio_index; // no idea what this is needed for
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

#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	boost::mutex::scoped_lock lock(_parserMutex);
#endif // LOAD_MEDIA_IN_A_SEPARATE_THREAD

	// We'll mess with the input here
	if ( ! m_parser.get() )
	{
		log_debug("NetStreamFfmpeg::seek(%d): no parser, no party", posSeconds);
		return;
	}

	// Don't ask me why, but NetStream::seek() takes seconds...
	boost::uint32_t pos = posSeconds*1000;

	long newpos = 0;
	double timebase = 0;

	// We'll pause the clock source and mark decoders as buffering.
	// In this way, next advance won't find the source time to 
	// be a lot of time behind and chances to get audio buffer
	// overruns will reduce.
	// ::advance will resume the playbackClock if DEC_BUFFERING...
	//
	_playbackClock->pause();
	decodingStatus(DEC_BUFFERING); 

	// Seek to new position
	newpos = m_parser->seek(pos);
	log_debug("m_parser->seek(%d) returned %d", pos, newpos);

	m_last_audio_timestamp = m_last_video_timestamp = newpos;

	{ // cleanup audio queue, so won't be consumed while seeking
		boost::mutex::scoped_lock lock(_audioQueueMutex);
		for (AudioQueue::iterator i=_audioQueue.begin(), e=_audioQueue.end();
				i!=e; ++i)
		{
			delete (*i);
		}
		_audioQueue.clear();
	}
	
	// 'newpos' will always be on a keyframe (supposedly)
	_playHead.seekTo(newpos);
	_qFillerResume.notify_all(); // wake it decoder is sleeping
	
	refreshVideoFrame(true);
}

void
NetStreamFfmpeg::parseNextChunk()
{
	// TODO: parse as much as possible w/out blocking
	//       (will always block currently..)
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
			"bufferLength=%d, bufferTime=%d",
			this, bufferLen, m_bufferTime);
#endif // GNASH_DEBUG_DECODING
		return;
	}

	if ( _playHead.isAudioConsumed() ) 
	{
#ifdef GNASH_DEBUG_DECODING
		log_debug("%p.refreshAudioBuffer: doing nothing "
			"as current position was already decoded - "
			"bufferLength=%d, bufferTime=%d",
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

	bool consumed = false;

	boost::uint64_t nextTimestamp;
	while ( 1 )
	{
		if ( ! m_parser->nextAudioFrameTimestamp(nextTimestamp) )
		{
#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.pushDecodedAudioFrames(%d): "
				"no more audio frames in input "
				"(nextAudioFrameTimestamp returned false)",
				this, ts);
#endif // GNASH_DEBUG_DECODING
			consumed = true;
			decodingStatus(DEC_STOPPED);
#ifdef GNASH_DEBUG_STATUS
			log_debug("Setting playStop status");
#endif
			setStatus(playStop);
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
			break; // next frame is in the future
		}

		boost::mutex::scoped_lock lock(_audioQueueMutex);

		static const int bufferLimit = 20;
		if ( _audioQueue.size() > bufferLimit )
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
			log_debug("%p.pushDecodedAudioFrames(%d) : queue size over limit (%d), "
				"audio won't be consumed (buffer overrun?)",
				this, ts, bufferLimit);
//#endif // GNASH_DEBUG_DECODING
			return;
		}

		media::raw_mediadata_t* audio = decodeNextAudioFrame();
		if ( ! audio )
		{
			log_error("nextAudioFrameTimestamp returned true, "
				"but decodeNextAudioFrame returned null, "
				"I don't think this should ever happen");
			break;
		}

#ifdef GNASH_DEBUG_DECODING
		// this one we might avoid :) -- a less intrusive logging could
		// be take note about how many things we're pushing over
		log_debug("pushDecodedAudioFrames(%d) pushing frame with timestamp %d", ts, nextTimestamp); 
#endif
		_audioQueue.push_back(audio);
	}

	// If we consumed audio of current position, feel free to advance if needed
	if ( consumed ) _playHead.setAudioConsumed();
}


void
NetStreamFfmpeg::refreshVideoFrame(bool alsoIfPaused)
{
	assert ( m_parser.get() );

#ifdef GNASH_DEBUG_DECODING
	// bufferLength() would lock the mutex (which we already hold),
	// so this is to avoid that.
	boost::uint32_t parserTime = m_parser->getBufferLength();
	boost::uint32_t playHeadTime = time();
	boost::uint32_t bufferLen = parserTime > playHeadTime ? parserTime-playHeadTime : 0;
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
	std::auto_ptr<image::rgb> video = getDecodedVideoFrame(curPos);

	// to be decoded or we're out of data
	if (!video.get())
	{
		if ( decodingStatus() == DEC_STOPPED )
		{
#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.refreshVideoFrame(): "
				"no more video frames to decode, "
				"sending STOP event",
				this);
#endif // GNASH_DEBUG_DECODING
#ifdef GNASH_DEBUG_STATUS
			log_debug("Setting playStop status");
#endif
			setStatus(playStop);
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
		m_imageframe = video.release(); // ownership transferred
		// A frame is ready for pickup
		m_newFrameReady = true;
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

#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	// stop parser thread while advancing
	boost::mutex::scoped_lock lock(_parserMutex);
#endif // LOAD_MEDIA_IN_A_SEPARATE_THREAD

	// Nothing to do if we don't have a parser
	if ( ! m_parser.get() ) return;

#ifndef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	// Parse some input no matter what
	parseNextChunk(); 
#endif // LOAD_MEDIA_IN_A_SEPARATE_THREAD

	// bufferLength() would lock the mutex (which we already hold),
	// so this is to avoid that.
	boost::uint32_t parserTime = m_parser->getBufferLength();
	boost::uint32_t playHeadTime = time();
	boost::uint32_t bufferLen = parserTime > playHeadTime ? parserTime-playHeadTime : 0;


	// Check decoding status 
	if ( decodingStatus() == DEC_DECODING && bufferLen == 0 )
	{
		if ( ! m_parser->parsingCompleted() )
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
		if ( bufferLen < m_bufferTime && ! m_parser->parsingCompleted() )
		{
#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.advance: buffering"
				" - position=%d, buffer=%d/%d",
				this, _playHead.getPosition(), bufferLen, m_bufferTime);
#endif // GNASH_DEBUG_DECODING
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
	if ( oldStatus == PlayHead::PLAY_PLAYING && _soundHandler )
	{
		_soundHandler->detach_aux_streamer((void*)this);
	}
}

void NetStreamFfmpeg::unpausePlayback()
{
	GNASH_REPORT_FUNCTION;

	PlayHead::PlaybackStatus oldStatus = _playHead.setState(PlayHead::PLAY_PLAYING);

	// Re-connect to the soundhandler if we were paused before
	if ( oldStatus == PlayHead::PLAY_PAUSED && _soundHandler )
	{
		_soundHandler->attach_aux_streamer(audio_streamer, (void*) this);
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
NetStreamFfmpeg::bufferLength ()
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	boost::mutex::scoped_lock lock(_parserMutex);
#endif // LOAD_MEDIA_IN_A_SEPARATE_THREAD

  	if ( ! m_parser.get() )
	{
		log_debug("bytesTotal: no parser, no party");
		return 0;
  	}

	boost::uint32_t maxTimeInBuffer = m_parser->getBufferLength();
	boost::uint64_t curPos = _playHead.getPosition();

	if ( maxTimeInBuffer < curPos ) return 0;
	return maxTimeInBuffer-curPos;
}

long
NetStreamFfmpeg::bytesTotal ()
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	boost::mutex::scoped_lock lock(_parserMutex);
#endif // LOAD_MEDIA_IN_A_SEPARATE_THREAD

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

} // gnash namespcae

#endif // USE_FFMPEG

