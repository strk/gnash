// Microphone_as.cpp:  ActionScript "Microphone" class, for Gnash.
//
//   Copyright (C) 2009, 2010, 2011 Free Software Foundation, Inc.
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


#include "Microphone_as.h"

#include <algorithm>
#include <boost/scoped_ptr.hpp>
#include <memory>

#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "NativeFunction.h"
#include "Object.h"
#include "GnashNumeric.h"
#include "AudioInput.h"
#include "MediaHandler.h"
#include "Relay.h"
#include "RunResources.h"
#include "namedStrings.h"


namespace gnash {

namespace {
    as_value microphone_get(const fn_call& fn);
    as_value microphone_setgain(const fn_call& fn);
    as_value microphone_setrate(const fn_call& fn);
    as_value microphone_setsilencelevel(const fn_call& fn);
    as_value microphone_setuseechosuppression(const fn_call& fn);
    as_value microphone_activityLevel(const fn_call& fn);
    as_value microphone_gain(const fn_call& fn);
    as_value microphone_index(const fn_call& fn);
    as_value microphone_muted(const fn_call& fn);
    as_value microphone_name(const fn_call& fn);
    as_value microphone_names(const fn_call& fn);
    as_value microphone_rate(const fn_call& fn);
    as_value microphone_silenceLevel(const fn_call& fn);
    as_value microphone_silenceTimeout(const fn_call& fn);
    as_value microphone_useEchoSuppression(const fn_call& fn);

    void attachMicrophoneStaticInterface(as_object& o);
    void attachMicrophoneInterface(as_object& o);
}

// extern (used by Global.cpp)
void
microphone_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinClass(where, emptyFunction, attachMicrophoneInterface,
                         attachMicrophoneStaticInterface, uri);
}

void
registerMicrophoneNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(microphone_names, 2104, 201);
    vm.registerNative(microphone_setsilencelevel, 2104, 0);
    vm.registerNative(microphone_setrate, 2104, 1);
    vm.registerNative(microphone_setgain, 2104, 2);
    vm.registerNative(microphone_setuseechosuppression, 2104, 3);
}

namespace {

// get() and names are static properties in AS2.
void
attachMicrophoneStaticInterface(as_object& o)
{
    Global_as& gl = getGlobal(o);

    const int flags = 0;

	o.init_member("get", gl.createFunction(microphone_get), flags);
 
    VM& vm = getVM(o);   
    NativeFunction* getset = vm.getNative(2102, 201);
    o.init_property("names", *getset, *getset);
}

// These are added to the AS2 prototype when get() is called.
void
attachMicrophoneProperties(as_object& o)
{
    Global_as& gl = getGlobal(o);


    as_function* getset = gl.createFunction(microphone_activityLevel);
    o.init_readonly_property("activityLevel", microphone_activityLevel);
    getset = gl.createFunction(microphone_gain);
    o.init_readonly_property("gain", microphone_gain);
    getset = gl.createFunction(microphone_index);
    o.init_readonly_property("index", microphone_index);
    getset = gl.createFunction(microphone_muted);
    o.init_readonly_property("muted", microphone_muted);
    getset = gl.createFunction(microphone_name);
    o.init_readonly_property("name", *getset);
    getset = gl.createFunction(microphone_rate);
    o.init_readonly_property("rate", *getset);
    getset = gl.createFunction(microphone_silenceLevel);
    o.init_readonly_property("silenceLevel", *getset);
    getset = gl.createFunction(microphone_silenceTimeout);
    o.init_readonly_property("silenceTimeout", *getset);
    getset = gl.createFunction(microphone_useEchoSuppression);
    o.init_readonly_property("useEchoSuppression", *getset);
}

void
attachMicrophoneInterface(as_object& o)
{

    VM& vm = getVM(o);

    const int flags = as_object::DefaultFlags | PropFlags::onlySWF6Up;

	o.init_member("setSilenceLevel", vm.getNative(2104, 0), flags);
	o.init_member("setRate", vm.getNative(2104, 1), flags);
	o.init_member("setGain", vm.getNative(2104, 2), flags);
	o.init_member("setUseEchoSuppression", vm.getNative(2104, 3), flags);
    
}

class Microphone_as : public Relay
{

public:

    Microphone_as(std::auto_ptr<media::AudioInput> input)
        :
        _input(input.release())
    {
        assert(_input.get());
    }

    /// Takes a value from 0..100
    void setGain(int gain) {
        _input->setGain(gain);
    }

    /// Returns a value from 0..100
    int gain() const {
        return _input->gain();
    }

    /// The index of this AudioInput.
    //
    /// Should this be stored in the AudioInput, this class, or somewhere else?
    size_t index() const {
        return _input->index();
    }

    /// Whether Microphone access is allowedd
    //
    /// This is set in the rcfile; should we query that, or the AudioInput
    /// itself?
    bool muted() const {
        return _input->muted();
    }

    /// The name of the Microphone
    const std::string& name() const {
        return _input->name();
    }

    /// Takes any int, is then set to the nearest available by the AudioInput
    //
    /// Supported rates are: 5, 8, 11, 16, 22, 44
    void setRate(int rate) {
        _input->setRate(rate);
    }

    /// Returns the actual value of the AudioInput rate
    //
    /// Values are in kHz.
    int rate() const {
        return _input->rate();
    }

    /// Range 0..100
    int silenceLevel() const {
        return _input->silenceLevel();
    }

    /// Range?
    int activityLevel() const {
        return _input->activityLevel();
    }

    void setUseEchoSuppression(bool b) {
        _input->setUseEchoSuppression(b);
    }

    bool useEchoSuppression() const {
        return _input->useEchoSuppression();
    }

    int silenceTimeout() const {
        return _input->silenceTimeout();
    }

    void setSilenceTimeout(int i) const {
        _input->setSilenceTimeout(i);
    }

    void setSilenceLevel(int i) const {
        _input->setSilenceLevel(i);
    }

private:
    boost::scoped_ptr<media::AudioInput> _input;

};

// AS2 static accessor.
as_value
microphone_get(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    // Properties are attached to the prototype (not __proto__) when get() is
    // called. 
    as_object* proto = toObject(getMember(*ptr, NSV::PROP_PROTOTYPE), getVM(fn));
    attachMicrophoneProperties(*proto);
 
    // TODO: this should return the same object when the same device is
    // meant, not a new object each time. It will be necessary to query
    // the MediaHandler for this, and possibly to store the as_objects
    // somewhere.
    const RunResources& r = getRunResources(getGlobal(fn));
    media::MediaHandler* handler = r.mediaHandler();

    if (!handler) {
        log_error(_("No MediaHandler exists! Cannot create a Microphone "
                    "object"));
        return as_value();
    }
    std::auto_ptr<media::AudioInput> input(handler->getAudioInput(0));

    if (!input.get()) {
        // TODO: what should happen if the index is not available?
        return as_value();
    }

    // Normally the VM would furnish us with a newly instantiated object, if
    // a constructor were used. But we're in a factory, so we have to build
    // one for ourselves.
    as_object* mic_obj = createObject(getGlobal(fn));
    mic_obj->set_prototype(proto);
    attachMicrophoneInterface(*mic_obj);
    attachMicrophoneProperties(*mic_obj);

    mic_obj->setRelay(new Microphone_as(input));

    return as_value(mic_obj);
}


as_value 
microphone_setgain(const fn_call& fn)
{
    Microphone_as* ptr = ensure<ThisIsNative<Microphone_as> >(fn);
    
    // Really return if there are 2 args?
    if (fn.nargs != 1) {
        log_error("Microphone.gain(): wrong number of parameters passed");
        return as_value();
    } 

    const boost::int32_t gain = clamp<boost::int32_t>(toInt(fn.arg(0), getVM(fn)), 0, 100);
    ptr->setGain(gain);
    return as_value();
}


as_value
microphone_setrate(const fn_call& fn)
{
    Microphone_as* ptr = ensure<ThisIsNative<Microphone_as> >(fn);
    
    if (fn.nargs != 1) {
        log_error("Microphone.setRate: wrong number of parameters passed");
        return as_value();
    }
    ptr->setRate(toInt(fn.arg(0), getVM(fn)));
    return as_value();
}

as_value
microphone_activityLevel(const fn_call& fn)
{
    Microphone_as* ptr = ensure<ThisIsNative<Microphone_as> >(fn);
        
    if (!fn.nargs) {
        log_unimpl("Microphone::activityLevel only has default value (-1)");
        return as_value(ptr->activityLevel());
    }

    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set activity property of Microphone"));
    );

    return as_value();
}

as_value
microphone_gain(const fn_call& fn)
{
    Microphone_as* ptr = ensure<ThisIsNative<Microphone_as> >(fn);
        
    if (!fn.nargs) {
        return as_value(ptr->gain());
    }
    return as_value();
}

as_value
microphone_index(const fn_call& fn)
{
    Microphone_as* ptr = ensure<ThisIsNative<Microphone_as> >(fn);
    
    if (!fn.nargs) {
        return as_value(ptr->index());
    }

    return as_value();
}

as_value
microphone_muted(const fn_call& fn)
{
    Microphone_as* ptr = ensure<ThisIsNative<Microphone_as> >(fn);
    
    if (!fn.nargs) {
        log_unimpl("Microphone::muted is always false (always allows access)");
        return as_value(ptr->muted());
    }

    return as_value();
}

as_value
microphone_name(const fn_call& fn)
{
    Microphone_as* ptr = ensure<ThisIsNative<Microphone_as> >(fn);
        
    if (!fn.nargs) {
        return as_value(ptr->name());
    }

    return as_value();
}


as_value
microphone_names(const fn_call& fn)
{
    // TODO: populate from MediaHandler like Camera.names.
    std::vector<std::string> vect;
    
    size_t size = vect.size();
    
    Global_as& gl = getGlobal(fn);
    as_object* data = gl.createArray();
    
    for (size_t i = 0; i < size; ++i) {
        callMethod(data, NSV::PROP_PUSH, vect[i]);
    }
        
    return as_value();
} 


as_value
microphone_rate(const fn_call& fn)
{
    Microphone_as* ptr = ensure<ThisIsNative<Microphone_as> >(fn);
    
    return as_value(ptr->rate());
}

as_value
microphone_silenceLevel(const fn_call& fn)
{
    Microphone_as* ptr = ensure<ThisIsNative<Microphone_as> >(fn);

    return as_value(ptr->silenceLevel());
}

as_value
microphone_silenceTimeout(const fn_call& fn)
{
    Microphone_as* ptr = ensure<ThisIsNative<Microphone_as> >(fn);
        
    log_unimpl("Microphone::silenceTimeout can be set, but is unimplemented");
    return as_value(ptr->silenceTimeout());
}

as_value
microphone_useEchoSuppression(const fn_call& fn)
{
    Microphone_as* ptr = ensure<ThisIsNative<Microphone_as> >(fn);
 
    // Documented to be a bool (which would make sense), but is a number.
    const double d = ptr->useEchoSuppression();
    return as_value(d);
}


as_value
microphone_setsilencelevel(const fn_call& fn)
{

    Microphone_as* ptr = ensure<ThisIsNative<Microphone_as> >(fn);
    
    const size_t numargs = fn.nargs;
    if (numargs > 2) {
        log_error("%s: Too many arguments", __FUNCTION__);
        return as_value();
    }

    const double level = clamp<double>(toNumber(fn.arg(0), getVM(fn)), 0, 100);
    ptr->setSilenceLevel(level);
    
    if (numargs > 1) {
        // If it's less than 0, it's set to 0.
        const int timeout = std::max<boost::int32_t>(toInt(fn.arg(1), getVM(fn)), 0);
        ptr->setSilenceTimeout(timeout);
    }
    return as_value();
}

as_value 
microphone_setuseechosuppression(const fn_call& fn)
{
    Microphone_as* ptr = ensure<ThisIsNative<Microphone_as> >(fn);
    
    if (!fn.nargs) {
        return as_value();
    }
    ptr->setUseEchoSuppression(toBool(fn.arg(0), getVM(fn)));
    return as_value();
}

} // anonymous

} // end of gnash namespace
