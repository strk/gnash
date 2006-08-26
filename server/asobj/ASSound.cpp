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

namespace gnash {

Sound::Sound() {
}

Sound::~Sound() {
}


void
Sound::attachSound()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
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

void
Sound::getVolume()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Sound::loadSound()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
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
Sound::setVolume()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Sound::start()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Sound::stop()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
sound_new(const fn_call& fn)
{
    sound_as_object *sound_obj = new sound_as_object;

    sound_obj->set_member("attachsound", &sound_attachsound);
    sound_obj->set_member("getbytesloaded", &sound_getbytesloaded);
    sound_obj->set_member("getbytestotal", &sound_getbytestotal);
    sound_obj->set_member("getpan", &sound_getpan);
    sound_obj->set_member("gettransform", &sound_gettransform);
    sound_obj->set_member("getvolume", &sound_getvolume);
    sound_obj->set_member("loadsound", &sound_loadsound);
    sound_obj->set_member("setpan", &sound_setpan);
    sound_obj->set_member("settransform", &sound_settransform);
    sound_obj->set_member("setvolume", &sound_setvolume);
    sound_obj->set_member("start", &sound_start);
    sound_obj->set_member("stop", &sound_stop);

    fn.result->set_as_object(sound_obj);
}

void
sound_start(const fn_call& fn)
{
    log_action("-- start sound \n");
    sound_handler* s = get_sound_handler();
		if (s != NULL)
		{
			int loop = 0;
			int secondOffset = 0;
			if (fn.nargs > 0)
			{
				int secondOffset = (int) fn.arg(0).to_number();

				// sanity check
				secondOffset = secondOffset <= 0 ? 0 : secondOffset;

				if (fn.nargs > 1)
				{
					loop = (int) fn.arg(1).to_number() - 1;

					// -1 means infinite playing of sound
					// sanity check
					loop = loop < 0 ? -1 : loop;
				}
			}

			sound_as_object*	so = (sound_as_object*) (as_object*) fn.this_ptr;
			assert(so);
			s->play_sound(so->sound_id, loop, secondOffset, 0);
		}

}

void
sound_stop(const fn_call& fn)
{
    log_action("-- stop sound \n");
    sound_handler* s = get_sound_handler();
    if (s != NULL)
	{
	    sound_as_object*	so = (sound_as_object*) (as_object*) fn.this_ptr;
	    assert(so);
	    s->stop_sound(so->sound_id);
	}
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

    sound_as_object*	so = (sound_as_object*) (as_object*) fn.this_ptr;
    assert(so);

    so->sound = fn.arg(0).to_tu_string();

    // check the import.
    movie_definition* def = fn.env->get_target()->get_root_movie()->get_movie_definition();
    assert(def);
    smart_ptr<resource> res = def->get_exported_resource(so->sound);
    if (res == NULL)
	{
	    log_error("import error: resource '%s' is not exported\n", so->sound.c_str());
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
    so->sound_id = si;
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
	sound_handler* s = get_sound_handler();
	if (s != NULL)
	{
		sound_as_object*	so = (sound_as_object*) (as_object*) fn.this_ptr;
		assert(so);
		int volume = s->get_volume(so->sound_id);
    fn.result->set_int(volume);
	}
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
		
	int volume = (int) fn.arg(0).to_number();

	// sanity check
	if (volume >= 0 && volume <=100)
	{
		sound_handler* s = get_sound_handler();
		if (s != NULL)
		{
			sound_as_object*	so = (sound_as_object*) (as_object*) fn.this_ptr;
			assert(so);
			s->set_volume(so->sound_id, volume);
		}
	}
}

} // end of gnash namespace

