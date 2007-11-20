// Sound.cpp:  ActionScript Sound output stub class, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
#include "config.h"
#endif

#include "Sound.h"
#include "log.h"
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
static as_value sound_gettransform(const fn_call& fn);
static as_value sound_getvolume(const fn_call& fn);
static as_value sound_loadsound(const fn_call& fn);
static as_value sound_setpan(const fn_call& fn);
static as_value sound_settransform(const fn_call& fn);
static as_value sound_setvolume(const fn_call& fn);
static as_value sound_start(const fn_call& fn);
static as_value sound_stop(const fn_call& fn);
static as_object* getSoundInterface();

Sound::Sound() 	:
	as_object(getSoundInterface()),
	connection(),
	soundId(-1),
	externalSound(false),
	isStreaming(false)
{
}

Sound::~Sound()
{
}


void
Sound::attachSound(int si, const std::string& name)
{
	soundId = si;
	soundName = name;
}

void
Sound::getBytesLoaded()
{
	static bool warned = false;
	if ( ! warned )
	{
		log_unimpl (__FUNCTION__);
		warned = true;
	}
}

void
Sound::getBytesTotal()
{
	static bool warned = false;
	if ( ! warned )
	{
		log_unimpl (__FUNCTION__);
		warned = true;
	}
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

int
Sound::getVolume()
{
	int volume = 0;
	sound_handler* s = get_sound_handler();
	if (s != NULL)
	{
		volume = s->get_volume(soundId);
	}
	return volume;
}

void
Sound::loadSound(std::string file, bool /*streaming*/)
{
	log_msg(_("%s is still testing!"), __FUNCTION__);

	if (connection) {
		log_error(_("%s: This sound already has a connection?  (We try to handle this by overriding the old one...)"), __FUNCTION__);
	}
	externalURL = file;

	connection = new NetConnection();
	connection->openConnection(externalURL);
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
	// sanity check
	if (volume >= 0 && volume <=100)
	{
		sound_handler* s = get_sound_handler();
		if (s != NULL)
		{
			s->set_volume(soundId, volume);
		}
	}
}

void
Sound::start(int offset, int loops)
{
	    sound_handler* s = get_sound_handler();
	    if (s) s->play_sound(soundId, loops, offset, 0, NULL);
    
}

void
Sound::stop(int si)
{

	sound_handler* s = get_sound_handler();
	if (s != NULL)
	{
	    if (si > -1) {
			s->stop_sound(soundId);
		} else {
			s->stop_sound(si);
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
sound_new(const fn_call& /* fn */)
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
		movie_definition* def = VM::get().getRoot().get_movie_definition();
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
	movie_definition* def = VM::get().getRoot().get_movie_definition();
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
	assert(si >= 0 && si < 1000);
	so->attachSound(si, name);
	return as_value();
}

as_value
sound_getbytesloaded(const fn_call& /*fn*/)
{
	static bool warned = false;
	if ( ! warned )
	{
		log_unimpl (__FUNCTION__);
		warned = true;
	}
	return as_value();
}

as_value
sound_getbytestotal(const fn_call& /*fn*/)
{
	static bool warned = false;
	if ( ! warned )
	{
		log_unimpl (__FUNCTION__);
		warned = true;
	}
	return as_value();
}

as_value
sound_getpan(const fn_call& /*fn*/)
{
	static bool warned = false;
	if ( ! warned )
	{
		log_unimpl (__FUNCTION__);
		warned = true;
	}
	return as_value();
}

as_value
sound_gettransform(const fn_call& /*fn*/)
{
	static bool warned = false;
	if ( ! warned )
	{
		log_unimpl (__FUNCTION__);
		warned = true;
	}
	return as_value();
}

as_value
sound_getvolume(const fn_call& fn)
{

	boost::intrusive_ptr<Sound> so = ensureType<Sound>(fn.this_ptr);

	int volume = so->getVolume();

	return as_value(volume);

}

as_value
sound_loadsound(const fn_call& fn)
{
	if (fn.nargs != 2) {
		IF_VERBOSE_ASCODING_ERRORS(
	    log_aserror(_("loadSound needs 2 arguments"));
	    	);
	    return as_value();		
	}

	boost::intrusive_ptr<Sound> so = ensureType<Sound>(fn.this_ptr);
	so->loadSound(fn.arg(0).to_string(), fn.arg(1).to_bool());

	return as_value();
}

as_value
sound_setpan(const fn_call& /*fn*/)
{
	static bool warned = false;
	if ( ! warned )
	{
		log_unimpl (__FUNCTION__);
		warned = true;
	}
	return as_value();
}

as_value
sound_settransform(const fn_call& /*fn*/)
{
	static bool warned = false;
	if ( ! warned )
	{
		log_unimpl (__FUNCTION__);
		warned = true;
	}
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
sound_ID3(const fn_call& /*fn*/)
{
	static bool warned = false;
	if ( ! warned )
	{
		log_unimpl (__FUNCTION__);
		warned = true;
	}
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

	o.init_member("attachSound", new builtin_function(sound_attachsound));
	o.init_member("getBytesLoaded", new builtin_function(sound_getbytesloaded));
	o.init_member("getBytesTotal", new builtin_function(sound_getbytestotal));
	o.init_member("getPan", new builtin_function(sound_getpan));
	o.init_member("getTransform", new builtin_function(sound_gettransform));
	o.init_member("getVolume", new builtin_function(sound_getvolume));
	o.init_member("loadSound", new builtin_function(sound_loadsound));
	o.init_member("setPan", new builtin_function(sound_setpan));
	o.init_member("setTransform", new builtin_function(sound_settransform));
	o.init_member("setVolume", new builtin_function(sound_setvolume));
	o.init_member("start", new builtin_function(sound_start));
	o.init_member("stop", new builtin_function(sound_stop));

	// Properties

	boost::intrusive_ptr<builtin_function> gettersetter;

	gettersetter = new builtin_function(&sound_duration, NULL);
	o.init_readonly_property("duration", *gettersetter);

	gettersetter = new builtin_function(&sound_ID3, NULL);
	o.init_property("ID3", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&sound_position, NULL);
	o.init_readonly_property("position", *gettersetter);

}

static as_object*
getSoundInterface()
{

	static boost::intrusive_ptr<as_object> o;
	if ( o == NULL )
	{
		o = new as_object(getObjectInterface());
		attachSoundInterface(*o);
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
		cl=new builtin_function(&sound_new, getSoundInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachSoundInterface(*cl);
		     
	}

	// Register _global.String
	global.init_member("Sound", cl.get());

}

} // end of gnash namespace
