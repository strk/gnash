// AsBroadcaster.cpp - AsBroadcaster AS interface
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "AsBroadcaster.h"

#include "Array_as.h" // for _listeners construction
#include "log.h"
#include "fn_call.h"
#include "NativeFunction.h" 
#include "Global_as.h"
#include "namedStrings.h"
#include "ObjectURI.h"

//#define GNASH_DEBUG_BROADCASTER 1

#ifdef GNASH_DEBUG_BROADCASTER
# include "Stats.h"
#endif

namespace gnash {

// Forward declarations.
namespace {
    as_value asbroadcaster_addListener(const fn_call& fn);
    as_value asbroadcaster_removeListener(const fn_call& fn);
    as_value asbroadcaster_broadcastMessage(const fn_call& fn);
    as_value asbroadcaster_initialize(const fn_call& fn);
}


/// Helper for notifying listeners
namespace {

#ifdef GNASH_DEBUG_BROADCASTER
struct BroadcasterStats {
    typedef std::map<ObjectURI&, unsigned long int> Stat;
    Stat stat;
    const VM& _st;
    BroadcasterStats(const VM& vm) : _st(st) {}
    void check(ObjectURI& k) {
        if ( ! (++stat[k] % 100) ) dump();
    }
    void dump() {
        using namespace std;
        typedef std::map<unsigned long int, ObjectURI&> Sorted;
        Sorted sorted;
        for (Stat::iterator i=stat.begin(), e=stat.end(); i!=e; ++i)
            sorted[i->second] = i->first;
        cerr << "Broadcaster stats follow:" << endl;
        for (Sorted::reverse_iterator i=sorted.rbegin(), e=sorted.rend();
                i!=e; ++i)
            std::cerr
                      << std::setw(10)
                      << i->first
                      << ":"
                      << _st.value(i->second) << "("
                      << i->second << ")"
                      << std::endl;
        
    }
};
#endif // GNASH_DEBUG_BROADCASTER

class BroadcasterVisitor
{
public:

    /// @param eName name of event, will be converted to lowercase if needed
    ///
    /// @param env Environment to use for functions invocation.
    ///
    BroadcasterVisitor(const fn_call& fn)
        :
        _eventURI(getURI(getVM(fn),fn.arg(0).to_string())),
        _dispatched(0),
        _fn(fn)
    {
        _fn.drop_bottom();
    }

    /// Call a method on the given value
    void operator()(const as_value& v)
    {

        as_object* o = toObject(v, getVM(_fn));
        if (!o) return;

#ifdef GNASH_DEBUG_BROADCASTER
        static stats::KeyLookup stats("BroadcasterVisitor call operator",
            getVM(_fn), 1);
        stats.check(_eventURI.name);
#endif
        as_value method;
        o->get_member(_eventURI, &method);

        if (method.is_function()) {
            _fn.super = o->get_super(_eventURI);
            _fn.this_ptr = o;
            method.to_function()->call(_fn);
        }

        ++_dispatched;
    }

    /// Return number of events dispatched.
    size_t eventsDispatched() const { return _dispatched; }

private:
    
    /// Name of the event being broadcasted
    /// appropriately cased based on SWF version
    /// of the current VM
    const ObjectURI& _eventURI;

    /// Number of event dispatches
    size_t _dispatched;

    fn_call _fn;

};

}

/// AsBroadcaster class


void 
AsBroadcaster::initialize(as_object& o)
{
    Global_as& gl = getGlobal(o);

    // Find _global.AsBroadcaster.
    as_object* asb = toObject(
            getMember(gl, NSV::CLASS_AS_BROADCASTER), getVM(o));

    // If it's not an object, these are left undefined, but they are
    // always attached to the initialized object.
    as_value al, rl;

    const int flags = as_object::DefaultFlags;

    if (asb) {
        al = getMember(*asb, NSV::PROP_ADD_LISTENER);
        rl = getMember(*asb, NSV::PROP_REMOVE_LISTENER);
    }
    
    o.set_member(NSV::PROP_ADD_LISTENER, al);
    o.set_member(NSV::PROP_REMOVE_LISTENER, rl);
    
    // The function returned by ASnative(101, 12) is attached, even though
    // this may not exist (e.g. if _global.ASnative is altered)
    const as_value& asn = callMethod(&gl, NSV::PROP_AS_NATIVE, 101, 12);
    o.set_member(NSV::PROP_BROADCAST_MESSAGE, asn);

    // This corresponds to  "_listeners = [];", which is different from
    // _listeners = new Array();
    o.set_member(NSV::PROP_uLISTENERS, gl.createArray());
 
    // This function should call ASSetPropFlags on these four properties.
    o.set_member_flags(NSV::PROP_BROADCAST_MESSAGE, flags);
    o.set_member_flags(NSV::PROP_ADD_LISTENER, flags);
    o.set_member_flags(NSV::PROP_REMOVE_LISTENER, flags);
    o.set_member_flags(NSV::PROP_uLISTENERS, flags);

}

void
attachAsBroadcasterStaticInterface(as_object& o)
{
    const int flags = PropFlags::dontEnum |
                      PropFlags::dontDelete |
                      PropFlags::onlySWF6Up;

    Global_as& gl = getGlobal(o);

    o.init_member("initialize",
            gl.createFunction(asbroadcaster_initialize), flags);
    o.init_member(NSV::PROP_ADD_LISTENER,
            gl.createFunction(asbroadcaster_addListener), flags);
    o.init_member(NSV::PROP_REMOVE_LISTENER,
            gl.createFunction(asbroadcaster_removeListener), flags);

    VM& vm = getVM(o);
    o.init_member(NSV::PROP_BROADCAST_MESSAGE, vm.getNative(101, 12),
            flags);

}


void
AsBroadcaster::registerNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(asbroadcaster_broadcastMessage, 101, 12);
}


void
AsBroadcaster::init(as_object& where, const ObjectURI& uri)
{
    // AsBroadcaster is a class, even though it doesn't look much like one.
    // Its prototype has no properties.
    registerBuiltinClass(where, emptyFunction, 0,
            attachAsBroadcasterStaticInterface, uri);
}


namespace {

as_value
asbroadcaster_initialize(const fn_call& fn)
{
    if ( fn.nargs < 1 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("AsBroadcaster.initialize() requires one argument, "
                "none given"));
        );
        return as_value();
    }

    // TODO: check if automatic primitive to object conversion apply here
    const as_value& tgtval = fn.arg(0);
    if (!tgtval.is_object()) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("AsBroadcaster.initialize(%s): first arg is "
                "not an object"), tgtval); 
        );
        return as_value();
    }

    as_object* tgt = toObject(tgtval, getVM(fn));
    if (!tgt) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("AsBroadcaster.initialize(%s): first arg is an object"
            " but doesn't cast to one (dangling DisplayObject ref?)"), tgtval); 
        );
        return as_value();
    }

    AsBroadcaster::initialize(*tgt);

    return as_value();
}
as_value
asbroadcaster_addListener(const fn_call& fn)
{

    as_object* obj = ensure<ValidThis>(fn);

    as_value newListener; assert(newListener.is_undefined());
    if ( fn.nargs ) newListener = fn.arg(0);

    callMethod(obj, NSV::PROP_REMOVE_LISTENER, newListener);

    as_value listenersValue;

    // TODO: test if we're supposed to crawl the target object's 
    //       inheritance chain in case it's own property _listeners 
    //       has been deleted while another one is found in any base
    //       class.
    if (!obj->get_member(NSV::PROP_uLISTENERS, &listenersValue)) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss; fn.dump_args(ss);
            log_aserror(_("%p.addListener(%s): this object has no "
                    "_listeners member"), (void*)fn.this_ptr, ss.str());
        );
        return as_value(true); // odd, but seems the case..
    }

    // assuming no automatic primitive-to-object cast will return an array...
    if (!listenersValue.is_object())
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss; fn.dump_args(ss);
            log_aserror(_("%p.addListener(%s): this object's _listener isn't "
                "an object: %s"), (void*)fn.this_ptr, ss.str(),
                listenersValue);
        );
        // TODO: check this
        return as_value(false); 
    }

    as_object* listeners = toObject(listenersValue, getVM(fn));

    // We checked is_object() above.
    assert(listeners); 

    callMethod(listeners, NSV::PROP_PUSH, newListener);

    return as_value(true);

}


as_value
asbroadcaster_removeListener(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);

    as_value listenersValue;

    // TODO: test if we're supposed to crawl the target object's 
    //       inheritance chain in case it's own property _listeners 
    //       has been deleted while another one is found in any base
    //       class.
    if (!obj->get_member(NSV::PROP_uLISTENERS, &listenersValue)) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss; fn.dump_args(ss);
            log_aserror(_("%p.addListener(%s): this object has no _listeners "
                "member"), (void*)fn.this_ptr, ss.str());
        );
        return as_value(false); // TODO: check this
    }

    // assuming no automatic primitive-to-object cast will return an array...
    if ( ! listenersValue.is_object() )
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss; fn.dump_args(ss);
            log_aserror(_("%p.addListener(%s): this object's _listener isn't "
                "an object: %s"), (void*)fn.this_ptr, ss.str(),
                listenersValue);
        );
        return as_value(false); // TODO: check this
    }

    as_object* listeners = toObject(listenersValue, getVM(fn));
    assert(listeners);

    as_value listenerToRemove; 
    if (fn.nargs) listenerToRemove = fn.arg(0);

    // Remove the first listener matching the new value
    // See http://www.senocular.com/flash/tutorials/
    // listenersasbroadcaster/?page=2
    
    // This is an ActionScript-like implementation, which is why it looks
    // like poor C++.
    const int length = toInt(getMember(*listeners, NSV::PROP_LENGTH),
            getVM(fn));
    int i = 0;

    VM& vm = getVM(fn);
    const ObjectURI& propSplice = getURI(vm, NSV::PROP_SPLICE);

    while (i < length) {
        std::ostringstream s;
        s << i;
        as_value el = getMember(*listeners, getURI(vm, s.str()));
        if (equals(el, listenerToRemove, getVM(fn))) {
            callMethod(listeners, propSplice, s.str(), 1);
            return as_value(true);
        }
        ++i;
    }
    return as_value(false);

}


as_value
asbroadcaster_broadcastMessage(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);

    as_value listenersValue;

    // TODO: test if we're supposed to crawl the target object's 
    //       inheritance chain in case its own property _listeners 
    //       has been deleted while another one is found in any base
    //       class.
    if (!obj->get_member(NSV::PROP_uLISTENERS, &listenersValue)) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss; fn.dump_args(ss);
            log_aserror(_("%p.addListener(%s): this object has no "
                    "_listeners member"), obj, ss.str());
        );
        return as_value(); // TODO: check this
    }

    // assuming no automatic primitive-to-object cast will return an array...
    if ( ! listenersValue.is_object() )
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss; fn.dump_args(ss);
            log_aserror(_("%p.addListener(%s): this object's _listener "
                "isn't an object: %s"), (void*)fn.this_ptr,
                ss.str(), listenersValue);
        );
        return as_value(); // TODO: check this
    }

    as_object* listeners = toObject(listenersValue, getVM(fn));

    if (!fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror("%p.broadcastMessage() needs an argument", 
            (void*)fn.this_ptr);
        );
        return as_value();
    }

    BroadcasterVisitor visitor(fn); 
    foreachArray(*listeners, visitor);

    const size_t dispatched = visitor.eventsDispatched();

    if (dispatched) return as_value(true);

    return as_value(); 

}

} // anonymous namespace

} // end of gnash namespace
