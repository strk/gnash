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
#include "sound_definition.h" // for sound_sample
#include "movie_definition.h"
#include "sprite_instance.h"
#include "fn_call.h"
#include "GnashException.h"
#include "builtin_function.h"
#include "Object.h" // for getObjectInterface
#include "VM.h"

#ifdef SOUND_GST
#include "SoundGst.h"
#elif defined(USE_FFMPEG)
#include "SoundFfmpeg.h"
#elif defined(USE_MAD)
#include "SoundMad.h"
#endif

#include <string>

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
	_soundHandler(get_sound_handler())
{
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
	LOG_ONCE( log_unimpl("Sound.getBytesLoaded() [default impl]") );
	return 0;
}

long
Sound::getBytesTotal()
{
	LOG_ONCE( log_unimpl("Sound.getBytesTotal() [default impl]") );
	return -1;
}

void
Sound::getPan()
{
	static bool warned = false;
	if ( ! warned )
	{
		log_unimpl (__FUNCTION__);
		warned = true;
	}
}

void
Sound::getTransform()
{
	static bool warned = false;
	if ( ! warned )
	{
		log_unimpl (__FUNCTION__);
		warned = true;
	}
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
    else log_debug("Sound has NO attached character, _soundHandler is %p, soundId is %d", _soundHandler, soundId);

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
Sound::loadSound(const std::string& file, bool /*streaming*/)
{
	log_debug(_("%s is still testing!"), __FUNCTION__);

	if (connection) {
		log_error(_("%s: This sound already has a connection?  (We try to handle this by overriding the old one...)"), __FUNCTION__);
	}
	externalURL = file;
}

void
Sound::setPan()
{
	static bool warned = false;
	if ( ! warned )
	{
		log_unimpl (__FUNCTION__);
		warned = true;
	}
}

void
Sound::setTransform()
{
	static bool warned = false;
	if ( ! warned )
	{
		log_unimpl (__FUNCTION__);
		warned = true;
	}
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
    if ( soundId == -1 )
    {
        // FIXME: find out what to do here
        log_error("Sound.start() called against a Sound that has no sound handle attached");
        return;
    }

    if (_soundHandler)
    {
        _soundHandler->play_sound(soundId, loops, offset, 0, NULL);
    }
}

void
Sound::stop(int si)
{
    if ( soundId == -1 )
    {
        // FIXME: find out what to do here
        log_error("Sound.stop() called against a Sound that has no sound handle attached");
        return;
    }

	if (_soundHandler)
	{
	    if (si > -1) {
			_soundHandler->stop_sound(soundId);
		} else {
			_soundHandler->stop_sound(si);
		}
	}
}

unsigned int
Sound::getDuration()
{
	static bool warned = false;
	if ( ! warned )
	{
		log_error(_("%s: only works when ffmpeg, gstreamer or libmad is enabled"), __FUNCTION__);
		warned = true;
	}
	return 0;
}

unsigned int
Sound::getPosition()
{
	static bool warned = false;
	if ( ! warned )
	{
		log_error(_("%s: only works when ffmpeg, gstreamer or libmad is enabled"), __FUNCTION__);
		warned = true;
	}
	return 0;
}


as_value
sound_new(const fn_call& fn)
{
	Sound* sound_obj;

#ifdef SOUND_GST
	sound_obj = new SoundGst();
#elif defined(USE_FFMPEG)
	sound_obj = new SoundFfmpeg();
#elif defined(USE_MAD)
	sound_obj = new SoundMad();
#else
	sound_obj = new Sound();
#endif

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
		boost::intrusive_ptr<resource> res = def->get_exported_resource(name);
		if (res == NULL)
		{
			IF_VERBOSE_MALFORMED_SWF(
		    log_swferror(_("import error: resource '%s' is not exported"), name.c_str());
		    	);
		    return as_value();
		}

		// FIXME: shouldn't we use dynamic_cast here (or rely on sound_sample interface) ?
		sound_sample* ss = res->cast_to_sound_sample();

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
	movie_definition* def = so->getVM().getRoot().get_movie_definition();
	assert(def);
	boost::intrusive_ptr<resource> res = def->get_exported_resource(name);
	if (res == NULL)
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("import error: resource '%s' is not exported"), name.c_str());
			);
		return as_value();
	}

	int si = 0;
	sound_sample* ss = res->cast_to_sound_sample();

	if (ss != NULL)
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
			log_aserror(_("Sound.loadSound(%s): arguments after first 2 discarded"), ss.str());
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
	// TODO: I guess this would have to do with premissions (crossdomain stuff)
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

	o.init_member("attachSound", new builtin_function(sound_attachsound), fl_hpc);
	o.init_member("getPan", new builtin_function(sound_getpan), fl_hpc);
	o.init_member("setPan", new builtin_function(sound_setpan), fl_hpc);
	o.init_member("start", new builtin_function(sound_start), fl_hpc);
	o.init_member("stop", new builtin_function(sound_stop), fl_hpc);
	o.init_member("getTransform", new builtin_function(sound_gettransform), fl_hpc);
	o.init_member("setTransform", new builtin_function(sound_settransform), fl_hpc);
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

#ifdef GNASH_USE_GC
void
Sound::markReachableResources() const
{
	if ( connection ) connection->setReachable();
	if ( attachedCharacter ) attachedCharacter->setReachable();
}
#endif // GNASH_USE_GC

} // end of gnash namespace
