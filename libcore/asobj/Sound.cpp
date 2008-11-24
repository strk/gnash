// Sound.cpp:  ActionScript Sound output stub class, for Gnash.
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

#include "Sound.h"
#include "log.h"
#include "sound_handler.h"
#include "MediaHandler.h"
#include "sound_definition.h" // for sound_sample
#include "movie_definition.h"
#include "fn_call.h"
#include "GnashException.h"
#include "builtin_function.h"
#include "Object.h" // for getObjectInterface
#include "VM.h"
#include "timers.h" // for registering the probe timer
#include "namedStrings.h"
#include "ExportableResource.h"
#include "StreamProvider.h"


#include "Sound.h"
#include <string>

// Define the macro below to get some more DEBUG
// lines while Sound is at work
//#define GNASH_DEBUG_SOUND_AS

namespace gnash {

static as_value sound_new(const fn_call& fn);
static as_value sound_attachsound(const fn_call& fn);
static as_value sound_getbytesloaded(const fn_call& fn);
static as_value sound_getbytestotal(const fn_call& fn);
static as_value sound_getpan(const fn_call& fn);
static as_value sound_setpan(const fn_call& fn);
static as_value sound_getDuration(const fn_call& fn);
static as_value sound_setDuration(const fn_call& fn);
static as_value sound_gettransform(const fn_call& fn);
static as_value sound_getPosition(const fn_call& fn);
static as_value sound_getvolume(const fn_call& fn);
static as_value sound_loadsound(const fn_call& fn);
static as_value sound_settransform(const fn_call& fn);
static as_value sound_setvolume(const fn_call& fn);
static as_value sound_start(const fn_call& fn);
static as_value sound_stop(const fn_call& fn);
static as_value checkPolicyFile_getset(const fn_call& fn);
static as_object* getSoundInterface();

Sound::Sound() 
	:
	as_object(getSoundInterface()),
	attachedCharacter(0),
	soundId(-1),
	externalSound(false),
	isStreaming(false),
	_soundHandler(_vm.getRoot().runInfo().soundHandler()),
	_mediaHandler(media::MediaHandler::get()),
	_startTime(0),
	_leftOverData(),
	_leftOverPtr(0),
	_leftOverSize(0),
	_inputStream(0),
	remainingLoops(0),
    _probeTimer(0),
    _soundCompleted(false)
{
}

Sound::~Sound()
{
	//GNASH_REPORT_FUNCTION;

	if (_inputStream && _soundHandler)
	{
		_soundHandler->unplugInputStream(_inputStream);
        _inputStream=0;
	}

}

void
Sound::attachCharacter(character* attachTo) 
{
	attachedCharacter.reset(new CharacterProxy(attachTo));
}

void
Sound::attachSound(int si, const std::string& name)
{
	soundId = si;
	soundName = name;
}

long
Sound::getBytesLoaded()
{
	if ( _mediaParser ) return _mediaParser->getBytesLoaded();
	return 0;
}

long
Sound::getBytesTotal()
{
	if ( _mediaParser ) return _mediaParser->getBytesTotal();
	return -1;
}

void
Sound::getPan()
{
	LOG_ONCE(log_unimpl(__FUNCTION__));
}

void
Sound::getTransform()
{
	LOG_ONCE(log_unimpl(__FUNCTION__));
}

bool
Sound::getVolume(int& volume)
{
    // TODO: check what takes precedence in case we
    //       have both an attached character *and*
    //       some other sound...
    //
    if ( attachedCharacter )
    {
        log_debug("Sound has an attached character");
        character* ch = attachedCharacter->get();
        if ( ! ch )
        {
            log_debug("Character attached to Sound was unloaded and couldn't rebind");
            return false;
        }
        volume = ch->getVolume();
        return true;
    }
    //else log_debug("Sound has NO attached character, _soundHandler is %p, soundId is %d", _soundHandler, soundId);

    // If we're not attached to a character we'll need to query
    // sound_handler for volume. If we have no sound handler, we
    // can't do much, so we'll return false
    if (!_soundHandler)
    {
        log_debug("We have no sound handler here...");
        return false;
    }

    // Now, we may be controlling a specific sound or
    // the final output as a whole.
    // If soundId is -1 we're controlling as a whole
    //
    if ( soundId == -1 )
    {
        volume = _soundHandler->getFinalVolume();
    }
    else
    {
        volume = _soundHandler->get_volume(soundId);
    }

    return true;
}

void
Sound::loadSound(const std::string& file, bool streaming)
{
	if ( ! _mediaHandler || ! _soundHandler ) 
	{
		log_debug("No media or sound handlers, won't load any sound");
		return;
	}

	/// If we are already streaming stop doing so as we'll replace
	/// the media parser
	if ( _inputStream )
	{
		_soundHandler->unplugInputStream(_inputStream);
		_inputStream = 0;
	}

	/// Delete any media parser being used (make sure we have detached!)
	_mediaParser.reset();

	/// Start at offset 0, in case a previous ::start() call
	/// changed that.
	_startTime=0;

    const movie_root& mr = _vm.getRoot();
	URL url(file, mr.runInfo().baseURL());

	StreamProvider& streamProvider = mr.runInfo().streamProvider();
	std::auto_ptr<IOChannel> inputStream(streamProvider.getStream(url));
	if ( ! inputStream.get() )
	{
		log_error( _("Gnash could not open this url: %s"), url );
		return;
	}

	externalSound = true;
	isStreaming = streaming;

	_mediaParser.reset( _mediaHandler->createMediaParser(inputStream).release() );
	if ( ! _mediaParser )
	{
		log_error(_("Unable to create parser for Sound at %s"), url);
		// not necessarely correct, the stream might have been found...
		return;
	}
	_mediaParser->setBufferTime(60000); // one minute buffer... should be fine

	if ( isStreaming )
	{
		startProbeTimer();
	}
    else
    {
        LOG_ONCE(log_unimpl("Non-streaming Sound.loadSound: will behave as a streaming one"));
        // if not streaming, we'll probe on .start()
    }
}

sound::InputStream*
Sound::attachAuxStreamerIfNeeded()
{
	media::AudioInfo* audioInfo =  _mediaParser->getAudioInfo();
	if (!audioInfo) return 0;

    // the following may throw an exception
	_audioDecoder.reset(_mediaHandler->createAudioDecoder(*audioInfo).release());

	// start playing ASAP, a call to ::start will just change _startTime
#ifdef GNASH_DEBUG_SOUND_AS
	log_debug("Attaching the aux streamer");
#endif
	return _soundHandler->attach_aux_streamer(getAudioWrapper, (void*) this);
}

void
Sound::setPan()
{
	LOG_ONCE(log_unimpl(__FUNCTION__));
}

void
Sound::setTransform()
{
	LOG_ONCE(log_unimpl(__FUNCTION__));
}

void
Sound::setVolume(int volume)
{
    // TODO: check what takes precedence in case we
    //       have both an attached character *and*
    //       some other sound...
    //
    if ( attachedCharacter )
    {
        character* ch = attachedCharacter->get();
        if ( ! ch )
        {
            log_debug("Character attached to Sound was unloaded and couldn't rebind");
            return;
        }
        ch->setVolume(volume);
        return;
    }

    // If we're not attached to a character we'll need to use
    // sound_handler for volume. If we have no sound handler, we
    // can't do much, so we'll just return
    if (!_soundHandler)
    {
        return;
    }

    // Now, we may be controlling a specific sound or
    // the final output as a whole.
    // If soundId is -1 we're controlling as a whole
    //
    if ( soundId == -1 )
    {
        _soundHandler->setFinalVolume(volume);
    }
    else
    {
        _soundHandler->set_volume(soundId, volume);
    }
}

void
Sound::start(int offset, int loops)
{
	if ( ! _soundHandler )
	{
		log_error("No sound handler, nothing to start...");
		return;
	}

	if (externalSound)
	{
		if ( ! _mediaParser )
		{
			log_error("No MediaParser initialized, can't start an external sound");
			return;
		}

		if (offset > 0)
		{
			_startTime=offset*1000;
			boost::uint32_t seekms = boost::uint32_t(offset*1000);
			// TODO: boost::mutex::scoped_lock parserLock(_parserMutex);
			_mediaParser->seek(seekms); // well, we try...
		}

		if (isStreaming)
		{
			IF_VERBOSE_ASCODING_ERRORS(
			log_aserror(_("Sound.start() has no effect on a streaming Sound"));
			);
			return;
		}

		// Save how many loops to do (not when streaming)
		if (loops > 0)
		{
			remainingLoops = loops;
		}

		// TODO: we should really be waiting for the sound to be fully
		//       loaded before starting to play it (!isStreaming case)
		startProbeTimer();

		//if ( ! _inputStream ) {
		//	_inputStream=_soundHandler->attach_aux_streamer(getAudioWrapper, (void*) this);
		//}
	}
	else
	{
        _soundHandler->playSound(
                    soundId,
                    loops,
                    offset, // in seconds
                    0, // start position in bytes...
                    0, // envelopes
                    true // allow multiple instances (TODO: checkme)
                    );
	}
}

void
Sound::stop(int si)
{
	if ( ! _soundHandler )
	{
		log_error("No sound handler, nothing to stop...");
		return;
	}

	// stop the sound
	if (si < 0)
	{
        if (externalSound)
		{
            if ( _inputStream )
            {
                _soundHandler->unplugInputStream(_inputStream);
                _inputStream=0;
            }
        }
		else
		{
			_soundHandler->stop_sound(soundId);
		}
	}
	else
	{
		_soundHandler->stop_sound(si);
	}
}

unsigned int
Sound::getDuration()
{
	if ( ! _soundHandler )
	{
		log_error("No sound handler, can't check duration...");
		return 0;
	}

	// If this is a event sound get the info from the soundhandler
	if (!externalSound)
	{
		return _soundHandler->get_duration(soundId);
	}

	// If we have a media parser (we'd do for an externalSound)
	// try fetching duration from it
	if ( _mediaParser )
	{
		media::AudioInfo* info = _mediaParser->getAudioInfo();
		if ( info )
		{
			return info->duration;
		}
	}

	return 0;
}

unsigned int
Sound::getPosition()
{
	if ( ! _soundHandler )
	{
		log_error("No sound handler, can't check position (we're likely not playing anyway)...");
		return 0;
	}

	// If this is a event sound get the info from the soundhandler
	if (!externalSound)
	{
		return _soundHandler->tell(soundId);
	}

	if ( _mediaParser )
	{
		boost::uint64_t ts;
		if ( _mediaParser->nextAudioFrameTimestamp(ts) )
		{
			return ts;
		}
	}

	return 0;

}


unsigned int
Sound::getAudio(boost::int16_t* samples, unsigned int nSamples, bool& atEOF)
{
	boost::uint8_t* stream = reinterpret_cast<boost::uint8_t*>(samples);
	int len = nSamples*2;

	//GNASH_REPORT_FUNCTION;

	while (len)
	{
		if ( ! _leftOverData )
		{
			bool parsingComplete = _mediaParser->parsingCompleted(); // check *before* calling nextAudioFrame
			std::auto_ptr<media::EncodedAudioFrame> frame = _mediaParser->nextAudioFrame();
			if ( ! frame.get() )
			{
				// just wait some more if parsing isn't complete yet
				if ( ! parsingComplete )
				{
					//log_debug("Parsing not complete and no more audio frames in input, try again later");
					break;
				}

				// or detach and stop here...
				// (should really honour loopings if any,
                // but that should be only done for non-streaming sound!)
				//log_debug("Parsing complete and no more audio frames in input, detaching");

				markSoundCompleted(true);

                // Setting atEOF to true will detach us.
                // We should change _inputStream, but need thread safety!
                // So on probeAudio, if _soundCompleted is set
                // we'll consider ourselves detached already and set
                // _inputStream to zero
				atEOF=true;
				return nSamples-(len/2);
			}

			// if we've been asked to start at a specific time, skip
			// any frame with earlier timestamp
			if ( frame->timestamp < _startTime )
			{
				//log_debug("This audio frame timestamp (%d) < requested start time (%d)", frame->timestamp, _startTime);
				continue;
			}

			_leftOverData.reset( _audioDecoder->decode(*frame, _leftOverSize) );
			_leftOverPtr = _leftOverData.get();
			if ( ! _leftOverData )
			{
				log_error("No samples decoded from input of %d bytes", frame->dataSize);
				continue;
			}

			//log_debug(" decoded %d bytes of audio", _leftOverSize);
		}

		assert( !(_leftOverSize%2) );

		int n = std::min<int>(_leftOverSize, len);
		//log_debug(" consuming %d bytes of decoded audio", n);

		std::copy(_leftOverPtr, _leftOverPtr+n, stream);
		//memcpy(stream, _leftOverPtr, n);

		stream += n;
		_leftOverPtr += n;
		_leftOverSize -= n;
		len -= n;

		if (_leftOverSize == 0)
		{
			_leftOverData.reset();
			_leftOverPtr = 0;
		}

	}

	// drop any queued video frame
	while (_mediaParser->nextVideoFrame().get()) {};

	atEOF=false;
	return nSamples-(len/2);
}

// audio callback is running in sound handler thread
unsigned int
Sound::getAudioWrapper(void* owner, boost::int16_t* samples,
		unsigned int nSamples, bool& atEOF)
{
	Sound* so = static_cast<Sound*>(owner);
	return so->getAudio(samples, nSamples, atEOF);
}








as_value
sound_new(const fn_call& fn)
{
	Sound* sound_obj = new Sound();

    if ( fn.nargs )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        if ( fn.nargs > 1 )
        {
            std::stringstream ss; fn.dump_args(ss);
            log_aserror("new Sound(%d) : args after first one ignored", ss.str());
        }
        );

        const as_value& arg0 = fn.arg(0);
        if ( ! arg0.is_null() && ! arg0.is_undefined() )
        {
            as_object* obj = arg0.to_object().get();
            character* ch = obj ? obj->to_character() : 0;
            IF_VERBOSE_ASCODING_ERRORS(
            if ( ! ch )
            {
                std::stringstream ss; fn.dump_args(ss);
                log_aserror("new Sound(%s) : first argument isn't null "
                    "nor undefined, and doesn't cast to a character. "
                    "We'll take as an invalid character ref.",
                    ss.str());
            }
            );
            sound_obj->attachCharacter(ch);
        }
    }
       
	return as_value(sound_obj);
}

as_value
sound_start(const fn_call& fn)
{
	IF_VERBOSE_ACTION (
	log_action(_("-- start sound"));
	)
	boost::intrusive_ptr<Sound> so = ensureType<Sound>(fn.this_ptr);
	int loop = 0;
	int secondOffset = 0;

	if (fn.nargs > 0) {
		secondOffset = (int) fn.arg(0).to_number();

		if (fn.nargs > 1) {
			loop = (int) fn.arg(1).to_number() - 1;

			// -1 means infinite playing of sound
			// sanity check
			loop = loop < 0 ? -1 : loop;
		}
	}
	so->start(secondOffset, loop);
	return as_value();
}

as_value
sound_stop(const fn_call& fn)
{
	IF_VERBOSE_ACTION (
	log_action(_("-- stop sound "));
	)
	boost::intrusive_ptr<Sound> so = ensureType<Sound>(fn.this_ptr);

	int si = -1;

	if (fn.nargs > 0) {
		const std::string& name = fn.arg(0).to_string();

		// check the import.
		movie_definition* def = so->getVM().getRoot().get_movie_definition();
		assert(def);
		boost::intrusive_ptr<ExportableResource> res = 
            def->get_exported_resource(name);

        if (!res)
		{
			IF_VERBOSE_MALFORMED_SWF(
                log_swferror(_("import error: resource '%s' is not exported"),
                    name);
		    	);
		    return as_value();
		}

		sound_sample* ss = dynamic_cast<sound_sample*>(res.get());

		if (ss != NULL)
		{
		    si = ss->m_sound_handler_id;
		}
		else
		{
		    log_error(_("sound sample is NULL (doesn't cast to sound_sample)"));
		    return as_value();
		}

	}
	so->stop(si);
	return as_value();
}

as_value
sound_attachsound(const fn_call& fn)
{
    IF_VERBOSE_ACTION (
    log_action(_("-- attach sound"));
    )
    if (fn.nargs < 1)
	{
		IF_VERBOSE_ASCODING_ERRORS(
	    log_aserror(_("attach sound needs one argument"));
	    	);
	    return as_value();
	}

	boost::intrusive_ptr<Sound> so = ensureType<Sound>(fn.this_ptr);

    const std::string& name = fn.arg(0).to_string();
    if (name.empty()) {
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("attachSound needs a non-empty string"));
		);
		return as_value();
	}

	// check the import.
    // NOTE: we should be checking in the SWF containing the calling code
    // (see 'winter bell' from orisinal morning sunshine for a testcase)
	const movie_definition* def;
    if ( ! fn.callerDef ) {
        log_error("Function call to Sound.attachSound have no callerDef");
        def = so->getVM().getRoot().get_movie_definition();
    }
    else {
        def = fn.callerDef;
    }

	assert(def);
	boost::intrusive_ptr<ExportableResource> res = 
        def->get_exported_resource(name);
	if (!res)
	{
		IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("import error: resource '%s' is not exported"),
                name);
        );
		return as_value();
	}

	int si = 0;
	sound_sample* ss = dynamic_cast<sound_sample*>(res.get());

	if (ss)
	{
		si = ss->m_sound_handler_id;
	}
	else
	{
		log_error(_("sound sample is NULL (doesn't cast to sound_sample)"));
		return as_value();
	}

	// sanity check
	assert(si >= 0);
	so->attachSound(si, name);
	return as_value();
}

as_value
sound_getbytesloaded(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl ("Sound.getBytesLoaded()") );
	return as_value();
}

as_value
sound_getbytestotal(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl ("Sound.getBytesTotal()") );
	return as_value();
}

as_value
sound_getpan(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl ("Sound.getPan()") );
	return as_value();
}

as_value
sound_getDuration(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl ("Sound.getDuration()") );
	return as_value();
}

as_value
sound_setDuration(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl ("Sound.setDuration()") );
	return as_value();
}

as_value
sound_getPosition(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl ("Sound.getPosition()") );
	return as_value();
}

as_value
sound_setPosition(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl ("Sound.setPosition()") );
	return as_value();
}

as_value
sound_gettransform(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl ("Sound.getTransform()") );
	return as_value();
}

as_value
sound_getvolume(const fn_call& fn)
{

	boost::intrusive_ptr<Sound> so = ensureType<Sound>(fn.this_ptr);

	if ( fn.nargs )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_aserror("Sound.getVolume(%s) : arguments ignored");
		);
	}

	int volume;
	if ( so->getVolume(volume) ) return as_value(volume);
	return as_value();
}

as_value
sound_loadsound(const fn_call& fn)
{
	boost::intrusive_ptr<Sound> so = ensureType<Sound>(fn.this_ptr);

	if (!fn.nargs)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Sound.loadSound() needs at least 1 argument"));
	    	);
		return as_value();		
	}

	std::string url = fn.arg(0).to_string();

	bool streaming = false;
	if ( fn.nargs > 1 )
	{
		streaming = fn.arg(1).to_bool();

		IF_VERBOSE_ASCODING_ERRORS(
		if ( fn.nargs > 2 )
		{
			std::stringstream ss; fn.dump_args(ss);
			log_aserror(_("Sound.loadSound(%s): arguments after first 2 "
                    "discarded"), ss.str());
		}
		);
	}

	so->loadSound(url, streaming);

	return as_value();
}

as_value
sound_setpan(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl ("Sound.setPan()") );
	return as_value();
}

as_value
sound_settransform(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl ("Sound.setTransform()") );
	return as_value();
}

as_value
sound_setvolume(const fn_call& fn)
{
	if (fn.nargs < 1)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("set volume of sound needs one argument"));
		);
		return as_value();
	}

	boost::intrusive_ptr<Sound> so = ensureType<Sound>(fn.this_ptr);	
	int volume = (int) fn.arg(0).to_number();

	so->setVolume(volume);
	return as_value();
}

as_value
sound_duration(const fn_call& fn)
{
	boost::intrusive_ptr<Sound> so = ensureType<Sound>(fn.this_ptr);
	return as_value(so->getDuration());
}

as_value
checkPolicyFile_getset(const fn_call& /*fn*/)
{
	LOG_ONCE( log_unimpl ("Sound.checkPolicyFile") );
	return as_value();
}

as_value
sound_areSoundsInaccessible(const fn_call& /*fn*/)
{
	// TODO: I guess this would have to do with permissions (crossdomain stuff)
	// more then capability.
	// See http://www.actionscript.org/forums/showthread.php3?t=160028
	// 
	// naive test shows this always being undefined..
	//
	LOG_ONCE( log_unimpl ("Sound.areSoundsInaccessible()") );
	return as_value();
}

as_value
sound_position(const fn_call& fn)
{
	boost::intrusive_ptr<Sound> so = ensureType<Sound>(fn.this_ptr);

	return as_value(so->getPosition());
}

void
attachSoundInterface(as_object& o)
{

	int fl_hpc = as_prop_flags::dontEnum|as_prop_flags::dontDelete|as_prop_flags::readOnly;

	o.init_member("attachSound", new builtin_function(sound_attachsound),
            fl_hpc);
	o.init_member("getPan", new builtin_function(sound_getpan), fl_hpc);
	o.init_member("setPan", new builtin_function(sound_setpan), fl_hpc);
	o.init_member("start", new builtin_function(sound_start), fl_hpc);
	o.init_member("stop", new builtin_function(sound_stop), fl_hpc);
	o.init_member("getTransform", new builtin_function(sound_gettransform),
            fl_hpc);
	o.init_member("setTransform", new builtin_function(sound_settransform),
            fl_hpc);
	o.init_member("getVolume", new builtin_function(sound_getvolume), fl_hpc);
	o.init_member("setVolume", new builtin_function(sound_setvolume), fl_hpc);

	int fl_hpcn6 = fl_hpc|as_prop_flags::onlySWF6Up;

	o.init_member("getDuration", new builtin_function(sound_getDuration), fl_hpcn6);
	o.init_member("setDuration", new builtin_function(sound_setDuration), fl_hpcn6);
	o.init_member("loadSound", new builtin_function(sound_loadsound), fl_hpcn6);
	o.init_member("getPosition", new builtin_function(sound_getPosition), fl_hpcn6);
	o.init_member("setPosition", new builtin_function(sound_setPosition), fl_hpcn6);
	o.init_member("getBytesLoaded", new builtin_function(sound_getbytesloaded), fl_hpcn6);
	o.init_member("getBytesTotal", new builtin_function(sound_getbytestotal), fl_hpcn6);

	int fl_hpcn9 = as_prop_flags::dontEnum|as_prop_flags::dontDelete|as_prop_flags::readOnly|as_prop_flags::onlySWF9Up;
	o.init_member("areSoundsInaccessible", new builtin_function(sound_areSoundsInaccessible), fl_hpcn9);

	// Properties
	//there's no such thing as an ID3 member (swfdec shows)
	o.init_readonly_property("duration", &sound_duration);
	o.init_readonly_property("position", &sound_position);

	int fl_hp = as_prop_flags::dontEnum|as_prop_flags::dontDelete;
	o.init_property("checkPolicyFile", &checkPolicyFile_getset, &checkPolicyFile_getset, fl_hp);

}

static as_object*
getSoundInterface()
{

	static boost::intrusive_ptr<as_object> o;
	if ( o == NULL )
	{
		o = new as_object(getObjectInterface());
		attachSoundInterface(*o);

		// TODO: make this an additional second arg to as_object(__proto__) ctor !
		o->set_member_flags(NSV::PROP_uuPROTOuu, as_prop_flags::readOnly, 0);
	}

	return o.get();
}

// extern (used by Global.cpp)
void sound_class_init(as_object& global)
{

	// This is going to be the global Sound "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		as_object* iface = getSoundInterface();
		cl=new builtin_function(&sound_new, iface);
		iface->set_member_flags(NSV::PROP_CONSTRUCTOR, as_prop_flags::readOnly);
	}

	// Register _global.String
	global.init_member("Sound", cl.get());

}

/*private*/
void
Sound::startProbeTimer()
{
	boost::intrusive_ptr<builtin_function> cb = \
		new builtin_function(&Sound::probeAudioWrapper);
	std::auto_ptr<Timer> timer(new Timer);
	unsigned long delayMS = 500; // 2 times each second (83 would be 12 times each second)
	timer->setInterval(*cb, delayMS, this);
	_probeTimer = getVM().getRoot().add_interval_timer(timer, true);
}

/*private static*/
as_value
Sound::probeAudioWrapper(const fn_call& fn)
{
    //GNASH_REPORT_FUNCTION;

    boost::intrusive_ptr<Sound> ptr = ensureType<Sound>(fn.this_ptr);
    ptr->probeAudio();
    return as_value();
}

/*private*/
void
Sound::stopProbeTimer()
{
#ifdef GNASH_DEBUG_SOUND_AS
    log_debug("stopProbeTimer called");
#endif

	if ( _probeTimer )
	{
		VM& vm = getVM();
		vm.getRoot().clear_interval_timer(_probeTimer);
        log_debug(" clear_interval_timer(%d) called", _probeTimer);
		_probeTimer = 0;
	}
}

/*private*/
void
Sound::probeAudio()
{
    if ( isAttached() )
    {
#ifdef GNASH_DEBUG_SOUND_AS
        log_debug("Probing audio for end");
#endif

        boost::mutex::scoped_lock lock(_soundCompletedMutex);
        if (_soundCompleted)
        {
            // when _soundCompleted is true we're
            // NOT attached !
            _mediaParser.reset(); // no use for this anymore...
            _inputStream=0;
            _soundCompleted=false;
            stopProbeTimer();

            // dispatch onSoundComplete 
	        callMethod(NSV::PROP_ON_SOUND_COMPLETE);
        }
    }
    else
    {
#ifdef GNASH_DEBUG_SOUND_AS
        log_debug("Probing audio for start");
#endif

        bool parsingCompleted = _mediaParser->parsingCompleted();
        try {
            _inputStream = attachAuxStreamerIfNeeded();
        } catch (MediaException& e) {
            assert(!_inputStream);
            assert(!_audioDecoder.get());
            log_error(_("Could not create audio decoder: %s"), e.what());
            _mediaParser.reset(); // no use for this anymore...
            stopProbeTimer();
            return;
        }

        if ( ! _inputStream )
        {
            if ( parsingCompleted )
            {
                log_debug("No audio in Sound input.");
                stopProbeTimer();
                _mediaParser.reset(); // no use for this anymore...
            }
            else
            {
                // keep probing
            }
        }
        else
        {
            // An audio decoder was constructed, good!
            assert(_audioDecoder.get());
        }
    }
}

#ifdef GNASH_USE_GC
void
Sound::markReachableResources() const
{
	if ( connection ) connection->setReachable();
	if ( attachedCharacter ) attachedCharacter->setReachable();
	markAsObjectReachable();
}
#endif // GNASH_USE_GC

void
Sound::markSoundCompleted(bool completed)
{
    boost::mutex::scoped_lock lock(_soundCompletedMutex);
    _soundCompleted=completed;
}

} // end of gnash namespace
