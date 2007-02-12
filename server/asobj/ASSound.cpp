// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "ASSound.h"
#include "sound.h" // for sound_sample_impl
#include "movie_definition.h"
#include "sprite_instance.h"
#include "fn_call.h"
#include "GnashException.h"
#include "builtin_function.h"

#include <string>

namespace gnash {

static void sound_new(const fn_call& fn);
static void sound_attachsound(const fn_call& fn);
static void sound_getbytesloaded(const fn_call& fn);
static void sound_getbytestotal(const fn_call& fn);
static void sound_getpan(const fn_call& fn);
static void sound_gettransform(const fn_call& fn);
static void sound_getvolume(const fn_call& fn);
static void sound_loadsound(const fn_call& fn);
static void sound_setpan(const fn_call& fn);
static void sound_settransform(const fn_call& fn);
static void sound_setvolume(const fn_call& fn);
static void sound_start(const fn_call& fn);
static void sound_stop(const fn_call& fn);
static as_object* getSoundInterface();

Sound::Sound() 	:
	as_object(getSoundInterface()),
	soundName(""),
	soundId(-1)
{
}

Sound::~Sound() {
}


void
Sound::attachSound(int si, const char* name)
{
	soundId = si;
	soundName = name;
}

void
Sound::getBytesLoaded()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Sound::getBytesTotal()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Sound::getPan()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Sound::getTransform()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
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
Sound::loadSound(std::string /*file*/, bool /*streaming*/)
{
    log_msg("%s is still testing!! \n", __FUNCTION__);


}

void
Sound::setPan()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Sound::setTransform()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
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

void
sound_new(const fn_call& fn)
{
	Sound *sound_obj = new Sound;

	fn.result->set_as_object(sound_obj);
}

// Wrapper around dynamic_cast to implement user warning.
// To be used by builtin properties and methods.
static Sound*
ensure_sound(as_object* obj)
{
	Sound* ret = dynamic_cast<Sound*>(obj);
	if ( ! ret )
	{
		throw ActionException("builtin method or gettersetter for Sound objects called against non-Sound instance");
	}
	return ret;
}

void
sound_start(const fn_call& fn)
{
	log_action("-- start sound \n");
	Sound* so = ensure_sound(fn.this_ptr);
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

}

void
sound_stop(const fn_call& fn)
{
	log_action("-- stop sound \n");
	Sound* so = ensure_sound(fn.this_ptr);

	int si = -1;

	if (fn.nargs > 0) {
		const char* name = fn.arg(0).to_string();

		// check the import.
		movie_definition* def = fn.env->get_target()->get_root_movie()->get_movie_definition();
		assert(def);
		boost::intrusive_ptr<resource> res = def->get_exported_resource(name);
		if (res == NULL)
		{
		    log_error("import error: resource '%s' is not exported\n", name);
		    return;
		}

		sound_sample_impl* ss = (sound_sample_impl*) res->cast_to_sound_sample();

		if (ss != NULL)
		{
		    si = ss->m_sound_handler_id;
		}
		else
		{
		    log_error("sound sample is NULL\n");
		    return;
		}

	}
	so->stop(si);

}

void
sound_attachsound(const fn_call& fn)
{
    log_action("-- attach sound \n");
    if (fn.nargs < 1)
	{
	    log_error("attach sound needs one argument\n");
	    return;
	}

	Sound* so = ensure_sound(fn.this_ptr);

    const char* name = fn.arg(0).to_string();
    if (!name) {
		log_error("attachSound need a non-null argument\n");
		return;
	}

    // check the import.
    movie_definition* def = fn.env->get_target()->get_root_movie()->get_movie_definition();
    assert(def);
    boost::intrusive_ptr<resource> res = def->get_exported_resource(so->soundName.c_str());
    if (res == NULL)
	{
	    log_error("import error: resource '%s' is not exported\n", so->soundName.c_str());
	    return;
	}

    int si = 0;
    sound_sample_impl* ss = (sound_sample_impl*) res->cast_to_sound_sample();

    if (ss != NULL)
	{
	    si = ss->m_sound_handler_id;
	}
    else
	{
	    log_error("sound sample is NULL\n");
	    return;
	}

    // sanity check
    assert(si >= 0 && si < 1000);
    so->attachSound(si, name);
}

void
sound_getbytesloaded(const fn_call& /*fn*/)
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
sound_getbytestotal(const fn_call& /*fn*/)
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
sound_getpan(const fn_call& /*fn*/)
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
sound_gettransform(const fn_call& /*fn*/)
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
sound_getvolume(const fn_call& fn)
{

	Sound* so = ensure_sound(fn.this_ptr);

	int volume = so->getVolume();

	fn.result->set_int(volume);

	return;
}

void
sound_loadsound(const fn_call& /*fn*/)
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
sound_setpan(const fn_call& /*fn*/)
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
sound_settransform(const fn_call& /*fn*/)
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
sound_setvolume(const fn_call& fn)
{
	if (fn.nargs < 1)
	{
		log_error("set volume of sound needs one argument\n");
		return;
	}

	Sound* so = ensure_sound(fn.this_ptr);	
	int volume = (int) fn.arg(0).to_number();

	so->setVolume(volume);
}

void
sound_duration(const fn_call& /*fn*/)
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
sound_ID3(const fn_call& /*fn*/)
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
sound_position(const fn_call& /*fn*/)
{
	log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
attachSoundInterface(as_object& o)
{

	o.init_member("attachSound", &sound_attachsound);
	o.init_member("getBytesLoaded", &sound_getbytesloaded);
	o.init_member("getBytesTotal", &sound_getbytestotal);
	o.init_member("getPan", &sound_getpan);
	o.init_member("getTransform", &sound_gettransform);
	o.init_member("getVolume", &sound_getvolume);
	o.init_member("loadSound", &sound_loadsound);
	o.init_member("setPan", &sound_setpan);
	o.init_member("setTransform", &sound_settransform);
	o.init_member("setVolume", &sound_setvolume);
	o.init_member("start", &sound_start);
	o.init_member("stop", &sound_stop);

	// Properties

	boost::intrusive_ptr<builtin_function> gettersetter;

	gettersetter = new builtin_function(&sound_duration, NULL);
	o.init_property("duration", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&sound_ID3, NULL);
	o.init_property("ID3", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&sound_position, NULL);
	o.init_property("position", *gettersetter, *gettersetter);

}

static as_object*
getSoundInterface()
{

	static boost::intrusive_ptr<as_object> o;
	if ( o == NULL )
	{
		o = new as_object();
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

