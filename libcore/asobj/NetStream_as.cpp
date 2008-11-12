// NetStream.cpp:  ActionScript class for streaming audio/video, for Gnash.
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

#include "NetStream_as.h"
#include "CharacterProxy.h"

#include "smart_ptr.h" // GNASH_USE_GC
#include "log.h"

#include "fn_call.h"
#include "builtin_function.h"
#include "timers.h" // for registering the advance timer
#include "GnashException.h"
#include "NetConnection.h"
#include "Object.h" // for getObjectInterface
#include "VM.h"
#include "namedStrings.h"
#include "movie_root.h"

#include "VirtualClock.h" // for PlayHead
#include "SystemClock.h"

#include "MediaHandler.h"
#include "StreamProvider.h"
#include "sound_handler.h"

#include <boost/algorithm/string/case_conv.hpp> // for PROPNAME

// Define the following macro to have status notification handling debugged
//#define GNASH_DEBUG_STATUS

// Define the following macro to enable decoding debugging
//#define GNASH_DEBUG_DECODING

namespace gnash {
 
static as_value netstream_new(const fn_call& fn);
static as_value netstream_close(const fn_call& fn);
static as_value netstream_pause(const fn_call& fn);
static as_value netstream_play(const fn_call& fn);
static as_value netstream_seek(const fn_call& fn);
static as_value netstream_setbuffertime(const fn_call& fn);
static as_value netstream_time(const fn_call& fn);

static as_value netstream_attachAudio(const fn_call& fn);
static as_value netstream_attachVideo(const fn_call& fn);
static as_value netstream_publish(const fn_call& fn);
static as_value netstream_receiveAudio(const fn_call& fn);
static as_value netstream_receiveVideo(const fn_call& fn);
static as_value netstream_send(const fn_call& fn);

static as_object* getNetStreamInterface();

NetStream_as::NetStream_as()
	:
	as_object(getNetStreamInterface()),
	_netCon(NULL),
	m_bufferTime(100), // The default size needed to begin playback of media is 100 miliseconds
	m_newFrameReady(false),
	m_imageframe(),
	m_parser(NULL),
	m_isFLV(false),
	inputPos(0),
	_invalidatedVideoCharacter(0),
	_decoding_state(DEC_NONE),

	_videoDecoder(0),
	_videoInfoKnown(false),

	_audioDecoder(0),
	_audioInfoKnown(false),

	// TODO: if audio is available, use _audioClock instead of SystemClock
	// as additional source
	_playbackClock(new InterruptableVirtualClock(new SystemClock)),
	_playHead(_playbackClock.get()), 

	_soundHandler(_vm.getRoot().runInfo().soundHandler()),
	_mediaHandler(media::MediaHandler::get()),
	_audioQueueSize(0),
	_auxStreamer(0),
	_lastStatus(invalidStatus),
	_advanceTimer(0)
{
}

static as_value
netstream_new(const fn_call& fn)
{

	boost::intrusive_ptr<NetStream_as> netstream_obj;
       
	netstream_obj = new NetStream_as();

	if (fn.nargs > 0)
	{
		boost::intrusive_ptr<NetConnection> ns = boost::dynamic_pointer_cast<NetConnection>(fn.arg(0).to_object());
		if ( ns )
		{
			netstream_obj->setNetCon(ns);
		}
		else
		{
			IF_VERBOSE_ASCODING_ERRORS(
				log_aserror(_("First argument "
					"to NetStream constructor "
					"doesn't cast to a NetConnection (%s)"),
					fn.arg(0));
			);
		}
	}
	return as_value(netstream_obj.get());

}

static as_value netstream_close(const fn_call& fn)
{
	boost::intrusive_ptr<NetStream_as> ns = ensureType<NetStream_as>(fn.this_ptr);
	ns->close();
	return as_value();
}

static as_value
netstream_pause(const fn_call& fn)
{
	boost::intrusive_ptr<NetStream_as> ns = 
        ensureType<NetStream_as>(fn.this_ptr);
	
	// mode: -1 ==> toogle, 0==> pause, 1==> play
	NetStream_as::PauseMode mode = NetStream_as::pauseModeToggle;
	if (fn.nargs > 0)
	{
		mode = fn.arg(0).to_bool() ? NetStream_as::pauseModePause :
		                             NetStream_as::pauseModeUnPause;
	}
	ns->pause(mode);	// toggle mode
	return as_value();
}

static as_value
netstream_play(const fn_call& fn)
{
	boost::intrusive_ptr<NetStream_as> ns = ensureType<NetStream_as>(fn.this_ptr);

	if (fn.nargs < 1)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("NetStream_as play needs args"));
		);
		return as_value();
	}

	if ( ! ns->isConnected() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("NetStream.play(%s): stream is not connected"), fn.arg(0));
		);
		return as_value();
	}

	ns->play(fn.arg(0).to_string());

	return as_value();
}

static as_value netstream_seek(const fn_call& fn) {
	boost::intrusive_ptr<NetStream_as> ns = ensureType<NetStream_as>(fn.this_ptr);

	boost::uint32_t time = 0;
	if (fn.nargs > 0)
	{
		time = static_cast<boost::uint32_t>(fn.arg(0).to_number());
	}
	ns->seek(time);

	return as_value();
}

static as_value netstream_setbuffertime(const fn_call& fn)
{

	//GNASH_REPORT_FUNCTION;

	boost::intrusive_ptr<NetStream_as> ns = ensureType<NetStream_as>(fn.this_ptr);

	// TODO: should we do anything if given no args ?
	//       are we sure setting bufferTime to 0 is what we have to do ?
	double time = 0;
	if (fn.nargs > 0)
	{
		time = fn.arg(0).to_number();
	}

	// TODO: don't allow a limit < 100 

	ns->setBufferTime(boost::uint32_t(time*1000));

	return as_value();
}

static as_value netstream_attachAudio(const fn_call& fn)
{
	boost::intrusive_ptr<NetStream_as> ns = ensureType<NetStream_as>(fn.this_ptr);
	UNUSED(ns);

	bool warned = false;
	if ( ! warned ) {
		log_unimpl("NetStream.attachAudio");
		warned = true;
	}

	return as_value();
}

static as_value netstream_attachVideo(const fn_call& fn)
{
	boost::intrusive_ptr<NetStream_as> ns = ensureType<NetStream_as>(fn.this_ptr);
	UNUSED(ns);

	bool warned = false;
	if ( ! warned ) {
		log_unimpl("NetStream.attachVideo");
		warned = true;
	}

	return as_value();
}

static as_value netstream_publish(const fn_call& fn)
{
	boost::intrusive_ptr<NetStream_as> ns = ensureType<NetStream_as>(fn.this_ptr);
	UNUSED(ns);

	bool warned = false;
	if ( ! warned ) {
		log_unimpl("NetStream.publish");
		warned = true;
	}

	return as_value();
}

static as_value netstream_receiveAudio(const fn_call& fn)
{
	boost::intrusive_ptr<NetStream_as> ns = ensureType<NetStream_as>(fn.this_ptr);
	UNUSED(ns);

	bool warned = false;
	if ( ! warned ) {
		log_unimpl("NetStream.receiveAudio");
		warned = true;
	}

	return as_value();
}

static as_value netstream_receiveVideo(const fn_call& fn)
{
	boost::intrusive_ptr<NetStream_as> ns = ensureType<NetStream_as>(fn.this_ptr);
	UNUSED(ns);

	bool warned = false;
	if ( ! warned ) {
		log_unimpl("NetStream.receiveVideo");
		warned = true;
	}

	return as_value();
}

static as_value netstream_send(const fn_call& fn)
{
	boost::intrusive_ptr<NetStream_as> ns = ensureType<NetStream_as>(fn.this_ptr);
	UNUSED(ns);

	bool warned = false;
	if ( ! warned ) {
		log_unimpl("NetStream.send");
		warned = true;
	}

	return as_value();
}

// Both a getter and a (do-nothing) setter for time
static as_value
netstream_time(const fn_call& fn)
{
	//GNASH_REPORT_FUNCTION;

	boost::intrusive_ptr<NetStream_as> ns = ensureType<NetStream_as>(fn.this_ptr);

	assert(fn.nargs == 0); // we're a getter
	return as_value(double(ns->time()/1000.0));
}

// Both a getter and a (do-nothing) setter for bytesLoaded
static as_value
netstream_bytesloaded(const fn_call& fn)
{
	boost::intrusive_ptr<NetStream_as> ns = ensureType<NetStream_as>(fn.this_ptr);

	if ( ! ns->isConnected() )
	{
		return as_value();
	}
	long ret = ns->bytesLoaded();
	return as_value(ret);
}

// Both a getter and a (do-nothing) setter for bytesTotal
static as_value
netstream_bytestotal(const fn_call& fn)
{
	boost::intrusive_ptr<NetStream_as> ns = ensureType<NetStream_as>(fn.this_ptr);

	if ( ! ns->isConnected() )
	{
		return as_value();
	}
	long ret = ns->bytesTotal();
	return as_value(ret);
}

// Both a getter and a (do-nothing) setter for currentFPS
static as_value
netstream_currentFPS(const fn_call& fn)
{
	boost::intrusive_ptr<NetStream_as> ns = ensureType<NetStream_as>(fn.this_ptr);

	if ( ! ns->isConnected() )
	{
		return as_value();
	}

	double fps = ns->getCurrentFPS();

#if 0
	if (fps <= 0) {
		return as_value(); // undef
	}
#endif

	return as_value(fps);
}

// read-only property bufferLength: amount of time buffered before playback
static as_value
netstream_bufferLength(const fn_call& fn)
{
	boost::intrusive_ptr<NetStream_as> ns = ensureType<NetStream_as>(fn.this_ptr);

	// NetStream_as::bufferLength returns milliseconds, we want
	// to return *fractional* seconds.
	double ret = ns->bufferLength()/1000.0;
	return as_value(ret);
}

// Both a getter and a (do-nothing) setter for bufferTime
static as_value
netstream_bufferTime(const fn_call& fn)
{
	boost::intrusive_ptr<NetStream_as> ns = ensureType<NetStream_as>(fn.this_ptr);

	// We return bufferTime in seconds
	double ret = ns->bufferTime()/1000.0;
	return as_value(ret);
}

// Both a getter and a (do-nothing) setter for liveDelay
static as_value
netstream_liveDelay(const fn_call& fn)
{
	boost::intrusive_ptr<NetStream_as> ns = ensureType<NetStream_as>(fn.this_ptr);

	bool warned = false;
	if ( ! warned ) {
		log_unimpl("NetStream.liveDelay getter/setter");
		warned = true;
	}

	if ( fn.nargs == 0 ) // getter
	{
		return as_value();
	}
	else // setter
	{
		return as_value();
	}
}

void
attachNetStreamInterface(as_object& o)
{

	o.init_member("close", new builtin_function(netstream_close));
	o.init_member("pause", new builtin_function(netstream_pause));
	o.init_member("play", new builtin_function(netstream_play));
	o.init_member("seek", new builtin_function(netstream_seek));
	o.init_member("setBufferTime", new builtin_function(netstream_setbuffertime));

	o.init_member("attachAudio", new builtin_function(netstream_attachAudio));
	o.init_member("attachVideo", new builtin_function(netstream_attachVideo));
	o.init_member("publish", new builtin_function(netstream_publish));
	o.init_member("receiveAudio", new builtin_function(netstream_receiveAudio));
	o.init_member("receiveVideo", new builtin_function(netstream_receiveVideo));
	o.init_member("send", new builtin_function(netstream_send));

	// Properties
	// TODO: attach to each instance rather then to the class ? check it ..

	o.init_readonly_property("time", &netstream_time);
	o.init_readonly_property("bytesLoaded", &netstream_bytesloaded);
	o.init_readonly_property("bytesTotal", &netstream_bytestotal);
	o.init_readonly_property("currentFps", &netstream_currentFPS);
	o.init_readonly_property("bufferLength", &netstream_bufferLength);
	o.init_readonly_property("bufferTime", &netstream_bufferTime);
	o.init_readonly_property("liveDelay", &netstream_liveDelay);

}

static as_object*
getNetStreamInterface()
{

	static boost::intrusive_ptr<as_object> o;
	if ( o == NULL )
	{
		o = new as_object(getObjectInterface());
		attachNetStreamInterface(*o);
	}

	return o.get();
}

// extern (used by Global.cpp)
void netstream_class_init(as_object& global)
{

	// This is going to be the global NetStream "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&netstream_new, getNetStreamInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachNetStreamInterface(*cl);
		     
	}

	// Register _global.String
	global.init_member("NetStream", cl.get());

}


void
NetStream_as::processNotify(const std::string& funcname, as_object* info_obj)
{
	// TODO: check for System.onStatus too ! use a private getStatusHandler() method for this.

#ifdef GNASH_DEBUG_METADATA
  log_debug(" Invoking onMetaData");
#endif

        string_table::key func = getVM().getStringTable().find(PROPNAME(funcname));

	callMethod(func, as_value(info_obj));
}


void
NetStream_as::processStatusNotifications()
{
	// TODO: check for System.onStatus too ! use a private getStatusHandler() method for this.

	StatusCode code;
	while (1)
	{
		code = popNextPendingStatusNotification();
		if ( code == invalidStatus ) break; // no more pending notifications

#ifdef GNASH_DEBUG_STATUS
		log_debug(" Invoking onStatus(%s)", getStatusCodeInfo(code).first);
#endif

		// TODO: optimize by reusing the same as_object ?
		boost::intrusive_ptr<as_object> o = getStatusObject(code);

		callMethod(NSV::PROP_ON_STATUS, as_value(o.get()));
	}
}

void
NetStream_as::setStatus(StatusCode status)
{
	// Get a lock to avoid messing with statuses while processing them
	boost::mutex::scoped_lock lock(statusMutex);

	// status unchanged
	if ( _lastStatus == status) return;

	_lastStatus = status;
	_statusQueue.push_back(status);
}

void
NetStream_as::setBufferTime(boost::uint32_t time)
{
	// The argument is in milliseconds,
	m_bufferTime = time;
	if ( m_parser.get() ) m_parser->setBufferTime(time);
}

long
NetStream_as::bufferLength()
{
	if (m_parser.get() == NULL) return 0;
	return m_parser->getBufferLength();
}

bool
NetStream_as::newFrameReady()
{
	if (m_newFrameReady) {
		m_newFrameReady = false;
		return true;
	} else {
		return false;
	}
}

std::auto_ptr<GnashImage>
NetStream_as::get_video()
{
	boost::mutex::scoped_lock lock(image_mutex);

	return m_imageframe;	
}

std::pair<const char*, const char*>
NetStream_as::getStatusCodeInfo(StatusCode code)
{
	switch (code)
	{
	
		case bufferEmpty:
			return std::pair<const char*, const char*>("NetStream.Buffer.Empty", "status");

		case bufferFull:
			return std::pair<const char*, const char*>("NetStream.Buffer.Full", "status");

		case bufferFlush:
			return std::pair<const char*, const char*>("NetStream.Buffer.Flush", "status");

		case playStart:
			return std::pair<const char*, const char*>("NetStream.Play.Start", "status");

		case playStop:
			return std::pair<const char*, const char*>("NetStream.Play.Stop", "status");

		case seekNotify:
			return std::pair<const char*, const char*>("NetStream.Seek.Notify", "status");

		case streamNotFound:
			return std::pair<const char*, const char*>("NetStream.Play.StreamNotFound", "error");

		case invalidTime:
			return std::pair<const char*, const char*>("NetStream.Seek.InvalidTime", "error");

		default:
			return std::pair<const char*, const char*>("","");
	}
}

boost::intrusive_ptr<as_object>
NetStream_as::getStatusObject(StatusCode code)
{
	// code, level
	std::pair<const char*, const char*> info = getStatusCodeInfo(code);

	boost::intrusive_ptr<as_object> o = new as_object(getObjectInterface());
	o->init_member("code",  info.first,  0); // enumerable, deletable
	o->init_member("level", info.second, 0); // enumerable, deletable

	return o;
}

NetStream_as::StatusCode
NetStream_as::popNextPendingStatusNotification()
{
	// Get an exclusive lock on the queue
	boost::mutex::scoped_lock lock(statusMutex);

	// No queued statuses to notify ...
	if ( _statusQueue.empty() ) return invalidStatus;

	StatusCode nextCode = _statusQueue.front();
	_statusQueue.pop_front();
	return nextCode;
}

void
NetStream_as::clearStatusQueue()
{
	// Get an exclusive lock on the queue
	boost::mutex::scoped_lock lock(statusMutex);

	_statusQueue.clear();
}

void
NetStream_as::setAudioController(character* ch)
{
	_audioController.reset(new CharacterProxy(ch));
}

#ifdef GNASH_USE_GC
void
NetStream_as::markReachableResources() const
{

	if ( _netCon ) _netCon->setReachable();

	if ( m_statusHandler ) m_statusHandler->setReachable();

	if ( _audioController ) _audioController->setReachable();

	if ( _invalidatedVideoCharacter ) _invalidatedVideoCharacter->setReachable();

	// Invoke generic as_object marker
	markAsObjectReachable();
}
#endif // GNASH_USE_GC

// ------- PlayHead class --------
PlayHead::PlayHead(VirtualClock* clockSource)
	:
	_position(0),
	_state(PLAY_PAUSED),
	_availableConsumers(0),
	_positionConsumers(0),
	_clockSource(clockSource)
{
	// NOTE: we construct in PAUSE mode so do not
	// really *need* to query _clockSource here.
	// We initialize it to an arbitrary value just
	// to be polite.
	_clockOffset = 0; // _clockSource->elapsed();
}

PlayHead::PlaybackStatus
PlayHead::setState(PlaybackStatus newState)
{
	if ( _state == newState ) return _state; // nothing to do

	if ( _state == PLAY_PAUSED )
	{
		assert(newState == PLAY_PLAYING);
		_state = PLAY_PLAYING;

		// if we go from PAUSED to PLAYING, reset
		// _clockOffset to yank current position
		// when querying clock source *now*
		boost::uint64_t now = _clockSource->elapsed();
		_clockOffset = ( now - _position );

		// check if we did the right thing
		// TODO: wrap this in PARANOIA_LEVEL > 1
		assert( now-_clockOffset == _position );

		return PLAY_PAUSED;
	}
	else
	{
		// TODO: wrap these in PARANOIA_LEVEL > 1 (or > 2)
		assert(_state == PLAY_PLAYING);
		assert(newState == PLAY_PAUSED);

		_state = PLAY_PAUSED;

		// When going from PLAYING to PAUSED
		// we do nothing with _clockOffset
		// as we'll update it when getting back to PLAYING
		return PLAY_PLAYING;
	}
}

PlayHead::PlaybackStatus
PlayHead::toggleState()
{
	if ( _state == PLAY_PAUSED ) return setState(PLAY_PLAYING);
	else return setState(PLAY_PAUSED);
}

void
PlayHead::advanceIfConsumed()
{
	if ( (_positionConsumers & _availableConsumers) != _availableConsumers)
	{
		// not all available consumers consumed current position,
		// won't advance
#if 0
		log_debug("PlayHead::advance(): "
			"not all consumers consumed current position, "
			"won't advance");
#endif
		return;
	}

	// Advance position
	boost::uint64_t now = _clockSource->elapsed();
	_position = now-_clockOffset;

	// Reset consumers state
	_positionConsumers = 0;
}

void
PlayHead::seekTo(boost::uint64_t position)
{
	boost::uint64_t now = _clockSource->elapsed();
	_position = position;

	_clockOffset = ( now - _position );
	assert( now-_clockOffset == _position ); // check if we did the right thing

	// Reset consumers state
	_positionConsumers = 0;
}

/*private static*/
as_value
NetStream_as::advanceWrapper(const fn_call& fn)
{
        boost::intrusive_ptr<NetStream_as> ptr = ensureType<NetStream_as>(fn.this_ptr);
        ptr->advance();
        return as_value();
}

void
NetStream_as::stopAdvanceTimer()
{
	if ( _advanceTimer )
	{
		VM& vm = getVM();
		vm.getRoot().clear_interval_timer(_advanceTimer);
		_advanceTimer = 0;
	}
}

void
NetStream_as::startAdvanceTimer()
{
	boost::intrusive_ptr<builtin_function> advanceCallback = 
		new builtin_function(&NetStream_as::advanceWrapper);
	std::auto_ptr<Timer> timer(new Timer);
	unsigned long delayMS = 50; // TODO: base on media file FPS !!!
	timer->setInterval(*advanceCallback, delayMS, this);
	_advanceTimer = getVM().getRoot().add_interval_timer(timer, true);
}


// AS-volume adjustment
void adjust_volume(boost::int16_t* data, int size, int volume)
{
	for (int i=0; i < size*0.5; i++) {
		data[i] = data[i] * volume/100;
	}
}


NetStream_as::~NetStream_as()
{
	close(); // close will also detach from sound handler
}


void NetStream_as::pause( PauseMode mode )
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

void NetStream_as::close()
{
	GNASH_REPORT_FUNCTION;

    // Delete any samples in the audio queue.
    cleanAudioQueue();

	// When closing gnash before playback is finished, the soundhandler 
	// seems to be removed before netstream is destroyed.
	detachAuxStreamer();

	m_imageframe.reset();

	stopAdvanceTimer();

}

void
NetStream_as::play(const std::string& c_url)
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
		log_aserror(_("No NetConnection associated with this NetStream, "
                "won't play"));
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

    const movie_root& mr = _vm.getRoot();

	StreamProvider& streamProvider = mr.runInfo().streamProvider();
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
NetStream_as::initVideoDecoder(const media::VideoInfo& info)
{
	assert ( _mediaHandler ); // caller should check this
	assert ( !_videoInfoKnown ); // caller should check this
	assert ( !_videoDecoder.get() ); // caller should check this

	_videoInfoKnown = true; 

	try {
	    _videoDecoder = _mediaHandler->createVideoDecoder(info);
            assert ( _videoDecoder.get() ); // PARANOIA_LEVEL ?
	}
	catch (MediaException& e) {
	    log_error("NetStream: Could not create Video decoder: %s", e.what());
	}

    log_debug("NetStream_as::initVideoDecoder: hot-plugging video consumer");
    _playHead.setVideoConsumerAvailable();
}


/* private */
void
NetStream_as::initAudioDecoder(const media::AudioInfo& info)
{
	assert ( _mediaHandler ); // caller should check this
	assert ( !_audioInfoKnown ); // caller should check this
	assert ( !_audioDecoder.get() ); // caller should check this

	_audioInfoKnown = true; 

	try {
	    _audioDecoder = _mediaHandler->createAudioDecoder(info);
            assert ( _audioDecoder.get() ); // PARANOIA_LEVE ?
	}
	catch (MediaException& e) {
	    log_error("Could not create Audio decoder: %s", e.what());
	}

    log_debug("NetStream_as::initAudioDecoder: hot-plugging audio consumer");
    _playHead.setAudioConsumerAvailable();
}


bool
NetStream_as::startPlayback()
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

	// TODO:
	// We do NOT want to initialize decoders right after construction
	// of the MediaParser, but rather construct them when needed, which
	// is when we have something to decode.
	// Postponing this will allow us NOT to block while probing
	// for stream contents.
	//

	decodingStatus(DEC_BUFFERING);
	_playbackClock->pause(); // NOTE: should be paused already

	_playHead.setState(PlayHead::PLAY_PLAYING);

	// Register ::advance callback
	startAdvanceTimer();

#ifdef GNASH_DEBUG_STATUS
	log_debug("Setting playStart status");
#endif // GNASH_DEBUG_STATUS
	setStatus(playStart);

	return true;
}


// audio callback, possibly running in a separate thread
unsigned int
NetStream_as::audio_streamer(void *owner, boost::int16_t* samples, unsigned int nSamples, bool& eof)
{
	//GNASH_REPORT_FUNCTION;

	boost::uint8_t* stream = reinterpret_cast<boost::uint8_t*>(samples);
	int len = nSamples*2;

	NetStream_as* ns = static_cast<NetStream_as*>(owner);

	boost::mutex::scoped_lock lock(ns->_audioQueueMutex);

#if 0
	log_debug("audio_streamer called, audioQueue size: %d, "
		"requested %d bytes of fill-up",
		ns->_audioQueue.size(), len);
#endif


	while (len)
	{
		if ( ns->_audioQueue.empty() )
		{
			break;
		}

		CursoredBuffer* samples = ns->_audioQueue.front();

		assert( ! (samples->m_size%2) ); 
		int n = std::min<int>(samples->m_size, len);
		std::copy(samples->m_ptr, samples->m_ptr+n, stream);
		//memcpy(stream, samples->m_ptr, n);
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

	assert( ! (len%2) ); 

	// currently never signalling EOF
	eof=false;
	return nSamples-(len/2);
}

std::auto_ptr<GnashImage> 
NetStream_as::getDecodedVideoFrame(boost::uint32_t ts)
{
	assert(_videoDecoder.get()); // caller should check this

	std::auto_ptr<GnashImage> video;

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
			log_error("nextVideoFrameTimestamp returned true (%d), "
				"but decodeNextVideoFrame returned null, "
				"I don't think this should ever happen", nextTimestamp);
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

std::auto_ptr<GnashImage> 
NetStream_as::decodeNextVideoFrame()
{
	std::auto_ptr<GnashImage> video;

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
		log_error(_("Error decoding encoded video frame in NetStream input"));
	}

	return video;
}

NetStream_as::CursoredBuffer*
NetStream_as::decodeNextAudioFrame()
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

	CursoredBuffer* raw = new CursoredBuffer();
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
	log_debug("NetStream_as::decodeNextAudioFrame: "
		"%d bytes of encoded audio "
		"decoded to %d bytes",
		frame->dataSize,
		raw->m_size);
#endif // GNASH_DEBUG_DECODING

	raw->m_ptr = raw->m_data;

	return raw;
}

bool NetStream_as::decodeMediaFrame()
{
	return false;
}

void
NetStream_as::seek(boost::uint32_t posSeconds)
{
	GNASH_REPORT_FUNCTION;

	// We'll mess with the input here
	if ( ! m_parser.get() )
	{
		log_debug("NetStream_as::seek(%d): no parser, no party", posSeconds);
		return;
	}

	// Don't ask me why, but NetStream_as::seek() takes seconds...
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

    // cleanup audio queue, so won't be consumed while seeking
    cleanAudioQueue();
	
	// 'newpos' will always be on a keyframe (supposedly)
	_playHead.seekTo(newpos);
	decodingStatus(DEC_BUFFERING); 
	
	refreshVideoFrame(true);
}

void
NetStream_as::parseNextChunk()
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
NetStream_as::refreshAudioBuffer()
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
NetStream_as::pushDecodedAudioFrames(boost::uint32_t ts)
{
	assert(m_parser.get());

	if ( ! _audioDecoder.get() )
	{
		// There are 3 possible reasons for _audioDecoder to not be here:
		//
		// 1: The stream does contain audio but we were unable to find
		//    an appropriate decoder for it
		//
		// 2: The stream does contain audio but we didn't try to construct
		//    a decoder for it yet.
		//
		// 3: The stream does NOT contain audio yet

		if ( _audioInfoKnown )
		{
			// case 1: we saw the audio info already,
			//         but couldn't construct a decoder

			// TODO: shouldn't we still flush any existing Audio frame
			//       in the encoded queue ?

			// log_debug("pushDecodedAudioFrames: no decoder for audio in stream, nothing to do");
			return;
		}

		media::AudioInfo* audioInfo = m_parser->getAudioInfo();
		if ( ! audioInfo )
		{
			// case 3: no audio found yet

			// assert(!parser.nextAudioFrameTimestamp); // if it was threadless...

			// log_debug("pushDecodedAudioFrames: no audio in stream (yet), nothing to do");
			return;
		}

		// case 2: here comes the audio !

		// try to create an AudioDecoder!
		initAudioDecoder(*audioInfo);

		// Don't go ahead if audio decoder construction failed
		if ( ! _audioDecoder.get() )
		{
			// TODO: we should still flush any existing Audio frame
			//       in the encoded queue...
			//       (or rely on next call)

			return; 
		}
	}

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

		CursoredBuffer* audio = decodeNextAudioFrame();
		if ( ! audio )
		{
			// Well, it *could* happen, why not ?
			log_error("nextAudioFrameTimestamp returned true (%d), "
				"but decodeNextAudioFrame returned null, "
				"I don't think this should ever happen", nextTimestamp);
			break;
		}

		if ( ! audio->m_size )
		{
			// Don't bother pushing an empty frame
			// to the audio queue...
			log_debug("pushDecodedAudioFrames(%d): Decoded audio frame contains no samples");
			delete audio;
			continue;
		}

		lock.lock(); // now needs locking

#ifdef GNASH_DEBUG_DECODING
		// this one we might avoid :) -- a less intrusive logging could
		// be take note about how many things we're pushing over
		log_debug("pushDecodedAudioFrames(%d) pushing %dth frame with timestamp %d", ts, _audioQueue.size()+1, nextTimestamp); 
#endif

		if ( _auxStreamer )
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
NetStream_as::refreshVideoFrame(bool alsoIfPaused)
{
	assert ( m_parser.get() );

	if ( ! _videoDecoder.get() )
	{
		// There are 3 possible reasons for _videoDecoder to not be here:
		//
		// 1: The stream does contain video but we were unable to find
		//    an appropriate decoder for it
		//
		// 2: The stream does contain video but we didn't try to construct
		//    a decoder for it yet.
		//
		// 3: The stream does NOT contain video yet
		//

		if ( _videoInfoKnown )
		{
			// case 1: we saw the video info already,
			//         but couldn't construct a decoder

			// TODO: shouldn't we still flush any existing Video frame
			//       in the encoded queue ?

			// log_debug("refreshVideoFrame: no decoder for video in stream, nothing to do");
			return;
		}

		media::VideoInfo* videoInfo = m_parser->getVideoInfo();
		if ( ! videoInfo )
		{
			// case 3: no video found yet

			// assert(!parser.nextVideoFrameTimestamp); // if it was threadless...

			// log_debug("refreshVideoFrame: no video in stream (yet), nothing to do");
			return;
		}

		// case 2: here comes the video !

		// Try to initialize the video decoder 
		initVideoDecoder(*videoInfo);

		// Don't go ahead if video decoder construction failed
		if ( ! _videoDecoder.get() )
		{
			// TODO: we should still flush any existing Video frame
			//       in the encoded queue...
			//       (or rely on next call)

			return; 
		}

	}

#ifdef GNASH_DEBUG_DECODING
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
	std::auto_ptr<GnashImage> video = getDecodedVideoFrame(curPos);

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

int
NetStream_as::videoHeight() const
{
    if (!_videoDecoder.get()) return 0;
    return _videoDecoder->height();
}

int
NetStream_as::videoWidth() const
{
    if (!_videoDecoder.get()) return 0;
    return _videoDecoder->width();
}

void
NetStream_as::advance()
{
	// Check if there are any new status messages, and if we should
	// pass them to a event handler
	processStatusNotifications();

	// Nothing to do if we don't have a parser
	// TODO: should we stopAdvanceTimer() ?
	if ( ! m_parser.get() ) return;

	if ( decodingStatus() == DEC_STOPPED )
	{
		//log_debug("NetStream_as::advance: dec stopped...");
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
                log_debug("refreshing video frame for the first time");
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

    // Advance PlayeHead position if current one was consumed
    // by all available consumers
    _playHead.advanceIfConsumed();

	// Process media tags
	m_parser->processTags(_playHead.getPosition(), this, getVM());
}

boost::int32_t
NetStream_as::time()
{
	return _playHead.getPosition();
}

void NetStream_as::pausePlayback()
{
	GNASH_REPORT_FUNCTION;

	PlayHead::PlaybackStatus oldStatus = _playHead.setState(PlayHead::PLAY_PAUSED);

	// Disconnect the soundhandler if we were playing before
	if ( oldStatus == PlayHead::PLAY_PLAYING )
	{
		detachAuxStreamer();
	}
}

void NetStream_as::unpausePlayback()
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
NetStream_as::bytesLoaded ()
{
  	if ( ! m_parser.get() )
	{
		log_debug("bytesLoaded: no parser, no party");
		return 0;
  	}

	return m_parser->getBytesLoaded();
}

long
NetStream_as::bytesTotal ()
{
  	if ( ! m_parser.get() )
	{
		log_debug("bytesTotal: no parser, no party");
		return 0;
  	}

	return m_parser->getBytesTotal();
}

NetStream_as::DecodingState
NetStream_as::decodingStatus(DecodingState newstate)
{
	boost::mutex::scoped_lock lock(_state_mutex);

	if (newstate != DEC_NONE) {
		_decoding_state = newstate;
	}

	return _decoding_state;
}

void
NetStream_as::attachAuxStreamer()
{
	if ( ! _soundHandler ) return;
	if ( _auxStreamer )
	{
		log_debug("attachAuxStreamer called while already attached");
        // Let's detach first..
	    _soundHandler->unplugInputStream(_auxStreamer);
        _auxStreamer=0;
	}

	try {
		_auxStreamer = _soundHandler->attach_aux_streamer(audio_streamer, (void*) this);
	} catch (SoundException& e) {
		log_error("Could not attach NetStream aux streamer to sound handler: %s", e.what());
	}
}

void
NetStream_as::detachAuxStreamer()
{
	if ( ! _soundHandler ) return;
	if ( !_auxStreamer )
	{
		log_debug("detachAuxStreamer called while not attached");
        return;
	}
	_soundHandler->unplugInputStream(_auxStreamer);
	_auxStreamer = 0;
}


void
NetStream_as::cleanAudioQueue()
{
    boost::mutex::scoped_lock lock(_audioQueueMutex);
    for (AudioQueue::iterator i=_audioQueue.begin(), e=_audioQueue.end(); i!=e; ++i)
    {
        delete *i;
    }
    _audioQueue.clear();
}



} // end of gnash namespace
