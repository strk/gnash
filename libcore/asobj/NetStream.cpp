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

#include "NetStream.h"
#include "CharacterProxy.h"

#include "smart_ptr.h" // GNASH_USE_GC
#include "log.h"
#ifdef SOUND_GST
# include "NetStreamGst.h"
#elif defined(USE_FFMPEG)
# include "NetStreamFfmpeg.h"
#endif
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

#include <boost/algorithm/string/case_conv.hpp> // for PROPNAME

// Define the following macro to have status notification handling debugged
//#define GNASH_DEBUG_STATUS

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

NetStream::NetStream()
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
	_lastStatus(invalidStatus),
	_advanceTimer(0)
{
}

static as_value
netstream_new(const fn_call& fn)
{

	boost::intrusive_ptr<NetStream> netstream_obj;
       
#ifdef SOUND_GST
	netstream_obj = new NetStreamGst();
#elif defined(USE_FFMPEG)
	netstream_obj = new NetStreamFfmpeg();
#else
	netstream_obj = new NetStream();
#endif


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
	boost::intrusive_ptr<NetStream> ns = ensureType<NetStream>(fn.this_ptr);
	ns->close();
	return as_value();
}

static as_value netstream_pause(const fn_call& fn)
{
	boost::intrusive_ptr<NetStream> ns = ensureType<NetStream>(fn.this_ptr);
	
	// mode: -1 ==> toogle, 0==> pause, 1==> play
	NetStream::PauseMode mode = NetStream::pauseModeToggle;
	if (fn.nargs > 0)
	{
		mode = fn.arg(0).to_bool() ? NetStream::pauseModePause :
		                             NetStream::pauseModeUnPause;
	}
	ns->pause(mode);	// toggle mode
	return as_value();
}

static as_value netstream_play(const fn_call& fn)
{
	boost::intrusive_ptr<NetStream> ns = ensureType<NetStream>(fn.this_ptr);

	if (fn.nargs < 1)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("NetStream play needs args"));
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
	boost::intrusive_ptr<NetStream> ns = ensureType<NetStream>(fn.this_ptr);

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

	boost::intrusive_ptr<NetStream> ns = ensureType<NetStream>(fn.this_ptr);

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
	boost::intrusive_ptr<NetStream> ns = ensureType<NetStream>(fn.this_ptr);
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
	boost::intrusive_ptr<NetStream> ns = ensureType<NetStream>(fn.this_ptr);
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
	boost::intrusive_ptr<NetStream> ns = ensureType<NetStream>(fn.this_ptr);
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
	boost::intrusive_ptr<NetStream> ns = ensureType<NetStream>(fn.this_ptr);
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
	boost::intrusive_ptr<NetStream> ns = ensureType<NetStream>(fn.this_ptr);
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
	boost::intrusive_ptr<NetStream> ns = ensureType<NetStream>(fn.this_ptr);
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

	boost::intrusive_ptr<NetStream> ns = ensureType<NetStream>(fn.this_ptr);

	assert(fn.nargs == 0); // we're a getter
	return as_value(double(ns->time()/1000.0));
}

// Both a getter and a (do-nothing) setter for bytesLoaded
static as_value
netstream_bytesloaded(const fn_call& fn)
{
	boost::intrusive_ptr<NetStream> ns = ensureType<NetStream>(fn.this_ptr);

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
	boost::intrusive_ptr<NetStream> ns = ensureType<NetStream>(fn.this_ptr);

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
	boost::intrusive_ptr<NetStream> ns = ensureType<NetStream>(fn.this_ptr);

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
	boost::intrusive_ptr<NetStream> ns = ensureType<NetStream>(fn.this_ptr);

	// NetStream::bufferLength returns milliseconds, we want
	// to return *fractional* seconds.
	double ret = ns->bufferLength()/1000.0;
	return as_value(ret);
}

// Both a getter and a (do-nothing) setter for bufferTime
static as_value
netstream_bufferTime(const fn_call& fn)
{
	boost::intrusive_ptr<NetStream> ns = ensureType<NetStream>(fn.this_ptr);

	// We return bufferTime in seconds
	double ret = ns->bufferTime()/1000.0;
	return as_value(ret);
}

// Both a getter and a (do-nothing) setter for liveDelay
static as_value
netstream_liveDelay(const fn_call& fn)
{
	boost::intrusive_ptr<NetStream> ns = ensureType<NetStream>(fn.this_ptr);

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
NetStream::processNotify(const std::string& funcname, as_object* info_obj)
{
	// TODO: check for System.onStatus too ! use a private getStatusHandler() method for this.

#ifdef GNASH_DEBUG_METADATA
  log_debug(" Invoking onMetaData");
#endif

        string_table::key func = getVM().getStringTable().find(PROPNAME(funcname));

	callMethod(func, as_value(info_obj));
}


void
NetStream::processStatusNotifications()
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
NetStream::setStatus(StatusCode status)
{
	// Get a lock to avoid messing with statuses while processing them
	boost::mutex::scoped_lock lock(statusMutex);

	// status unchanged
	if ( _lastStatus == status) return;

	_lastStatus = status;
	_statusQueue.push_back(status);
}

void
NetStream::setBufferTime(boost::uint32_t time)
{
	// The argument is in milliseconds,
	m_bufferTime = time;
	if ( m_parser.get() ) m_parser->setBufferTime(time);
}

long
NetStream::bufferLength()
{
	if (m_parser.get() == NULL) return 0;
	return m_parser->getBufferLength();
}

bool
NetStream::newFrameReady()
{
	if (m_newFrameReady) {
		m_newFrameReady = false;
		return true;
	} else {
		return false;
	}
}

std::auto_ptr<image::ImageBase>
NetStream::get_video()
{
	boost::mutex::scoped_lock lock(image_mutex);

	if (!m_imageframe.get()) return std::auto_ptr<image::ImageBase>(0);

	// TODO: inspect if we could return m_imageframe directly...
	return m_imageframe;	
}

std::pair<const char*, const char*>
NetStream::getStatusCodeInfo(StatusCode code)
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
NetStream::getStatusObject(StatusCode code)
{
	// code, level
	std::pair<const char*, const char*> info = getStatusCodeInfo(code);

	boost::intrusive_ptr<as_object> o = new as_object(getObjectInterface());
	o->init_member("code",  info.first,  0); // enumerable, deletable
	o->init_member("level", info.second, 0); // enumerable, deletable

	return o;
}

NetStream::StatusCode
NetStream::popNextPendingStatusNotification()
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
NetStream::clearStatusQueue()
{
	// Get an exclusive lock on the queue
	boost::mutex::scoped_lock lock(statusMutex);

	_statusQueue.clear();
}

void
NetStream::setAudioController(character* ch)
{
	_audioController.reset(new CharacterProxy(ch));
}

#ifdef GNASH_USE_GC
void
NetStream::markReachableResources() const
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
	_state(PLAY_PLAYING),
	_availableConsumers(0),
	_positionConsumers(0),
	_clockSource(clockSource)
{
	_clockOffset = _clockSource->elapsed();
}

void
PlayHead::init(bool hasVideo, bool hasAudio)
{
	boost::uint64_t now = _clockSource->elapsed();
	if ( hasVideo ) _availableConsumers |= CONSUMER_VIDEO;
	if ( hasAudio ) _availableConsumers |= CONSUMER_AUDIO;
	_positionConsumers = 0;

	_position = 0;
	_clockOffset = now;
	assert(now-_clockOffset == _position);
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
		assert( now-_clockOffset == _position ); // check if we did the right thing

		return PLAY_PAUSED;
	}
	else
	{
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
NetStream::advanceWrapper(const fn_call& fn)
{
        boost::intrusive_ptr<NetStream> ptr = ensureType<NetStream>(fn.this_ptr);
        ptr->advance();
        return as_value();
}

void
NetStream::stopAdvanceTimer()
{
	if ( _advanceTimer )
	{
		VM& vm = getVM();
		vm.getRoot().clear_interval_timer(_advanceTimer);
		_advanceTimer = 0;
	}
}

void
NetStream::startAdvanceTimer()
{
	boost::intrusive_ptr<builtin_function> advanceCallback = \
		new builtin_function(&NetStream::advanceWrapper);
	std::auto_ptr<Timer> timer(new Timer);
	unsigned long delayMS = 50; // TODO: base on media file FPS !!!
	timer->setInterval(*advanceCallback, delayMS, this);
	_advanceTimer = getVM().getRoot().add_interval_timer(timer, true);
}

} // end of gnash namespace
