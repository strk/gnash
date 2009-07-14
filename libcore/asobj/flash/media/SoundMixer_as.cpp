// SoundMixer_as.cpp:  ActionScript "SoundMixer" class, for Gnash.
//
//   Copyright (C) 2009 Free Software Foundation, Inc.
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

#include "media/SoundMixer_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value soundmixer_computeSpectrum(const fn_call& fn);
    as_value soundmixer_stopAll(const fn_call& fn);
    as_value soundmixer_ctor(const fn_call& fn);
    void attachSoundMixerInterface(as_object& o);
    void attachSoundMixerStaticInterface(as_object& o);
    as_object* getSoundMixerInterface();

}

class SoundMixer_as : public as_object
{

public:

    SoundMixer_as()
        :
        as_object(getSoundMixerInterface())
    {}
};

// extern (used by Global.cpp)
void soundmixer_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&soundmixer_ctor, getSoundMixerInterface());
        attachSoundMixerStaticInterface(*cl);
    }

    // Register _global.SoundMixer
    global.init_member("SoundMixer", cl.get());
}

namespace {

void
attachSoundMixerInterface(as_object& o)
{
    o.init_member("computeSpectrum", gl->createFunction(soundmixer_computeSpectrum));
    o.init_member("stopAll", gl->createFunction(soundmixer_stopAll));
}

void
attachSoundMixerStaticInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);

}

as_object*
getSoundMixerInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachSoundMixerInterface(*o);
    }
    return o.get();
}

as_value
soundmixer_computeSpectrum(const fn_call& fn)
{
    boost::intrusive_ptr<SoundMixer_as> ptr =
        ensureType<SoundMixer_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
soundmixer_stopAll(const fn_call& fn)
{
    boost::intrusive_ptr<SoundMixer_as> ptr =
        ensureType<SoundMixer_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
soundmixer_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new SoundMixer_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

