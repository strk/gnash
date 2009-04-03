// AsBroadcaster.cpp - AsBroadcaster AS interface
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "Array_as.h" // for _listeners construction
#include "log.h"
#include "AsBroadcaster.h"
#include "fn_call.h"
#include "builtin_function.h"
#include "VM.h" // for getPlayerVersion() 
#include "Object.h" // for getObjectInterface
#include "namedStrings.h"

namespace gnash {

// Forward declarations.
namespace {
	as_value asbroadcaster_addListener(const fn_call& fn);
	as_value asbroadcaster_removeListener(const fn_call& fn);
	as_value asbroadcaster_broadcastMessage(const fn_call& fn);
	as_value asbroadcaster_initialize(const fn_call& fn);
    as_value asbroadcaster_ctor(const fn_call& fn);

    as_object* getAsBroadcasterInterface();
}


/// Helper for notifying listeners
namespace {

class BroadcasterVisitor
{
	
	/// Name of the event being broadcasted
	/// appropriately cased based on SWF version
	/// of the current VM
	std::string _eventName;
	string_table::key _eventKey;

	// These two will be needed for consistency checking
	//size_t _origEnvStackSize;
	//size_t _origEnvCallStackSize;

	/// Number of event dispatches
	unsigned int _dispatched;

	fn_call _fn;

public:

	/// @param eName name of event, will be converted to lowercase if needed
	///
	/// @param env Environment to use for functions invocation.
	///
	BroadcasterVisitor(const fn_call& fn)
		:
		_eventName(),
		_eventKey(0),
		_dispatched(0),
		_fn(fn)
	{
		_eventName = fn.arg(0).to_string();
		_eventKey = fn.getVM().getStringTable().find(_eventName);
		_fn.drop_bottom();
	}

	/// Call a method on the given value
	void visit(as_value& v)
	{
		boost::intrusive_ptr<as_object> o = v.to_object();
		if ( ! o ) return;

		as_value method;
		o->get_member(_eventKey, &method);
        _fn.super = o->get_super(_eventName.c_str());

		if (method.is_function()) {
			_fn.this_ptr = o.get();
			method.to_as_function()->call(_fn);
		}

		++_dispatched;
	}

	/// Return number of events dispached since last reset()
	unsigned int eventsDispatched() const { return _dispatched; }

	/// Reset count od dispatched events
	void reset() { _dispatched=0; }
};

}

/// AsBroadcaster class


void 
AsBroadcaster::initialize(as_object& o)
{
	as_object* asb = getAsBroadcaster();

	as_value tmp;

	if (asb->get_member(NSV::PROP_ADD_LISTENER, &tmp)) {
        o.set_member(NSV::PROP_ADD_LISTENER, tmp);
	}

	if (asb->get_member(NSV::PROP_REMOVE_LISTENER, &tmp)) {
        o.set_member(NSV::PROP_REMOVE_LISTENER, tmp);
	}
	
    o.set_member(NSV::PROP_BROADCAST_MESSAGE,
            new builtin_function(asbroadcaster_broadcastMessage));

    o.set_member(NSV::PROP_uLISTENERS, new Array_as());

#ifndef NDEBUG
	assert(o.get_member(NSV::PROP_uLISTENERS, &tmp));
	assert(tmp.is_object());
	assert(o.get_member(NSV::PROP_BROADCAST_MESSAGE, &tmp));
	assert(tmp.is_function());

    // The following properties may be overridden in the AsBroadcaster object
    // and thus unavailable: addListener, removeListener
#endif
}

as_object*
AsBroadcaster::getAsBroadcaster()
{
	VM& vm = VM::get();

	static boost::intrusive_ptr<as_object> obj = NULL;
	if ( ! obj )
	{
		obj = new builtin_function(asbroadcaster_ctor,
                getAsBroadcasterInterface()); 
		vm.addStatic(obj.get()); // correct ?

        const int flags = as_prop_flags::dontEnum |
                          as_prop_flags::dontDelete |
                          as_prop_flags::onlySWF6Up;

        // NOTE: we may add NSV::PROP_INITIALIZE, unavailable at
        // time of writing. Anyway, since AsBroadcaster is the only
        // class we know using an 'initialize' method we might as
        // well save the string_table size in case we'll not load
        // the class.
        obj->init_member("initialize",
                new builtin_function(asbroadcaster_initialize), flags);
        obj->init_member(NSV::PROP_ADD_LISTENER,
                new builtin_function(asbroadcaster_addListener), flags);
        obj->init_member(NSV::PROP_REMOVE_LISTENER,
                new builtin_function(asbroadcaster_removeListener), flags);
        obj->init_member(NSV::PROP_BROADCAST_MESSAGE, vm.getNative(101, 12),
                flags);
	}

	return obj.get();
}


void
AsBroadcaster::registerNative(as_object& global)
{
    VM& vm = global.getVM();
    vm.registerNative(asbroadcaster_broadcastMessage, 101, 12);
}


void
AsBroadcaster::init(as_object& global)
{
	// _global.AsBroadcaster is NOT a class, but a simple object
	global.init_member("AsBroadcaster", AsBroadcaster::getAsBroadcaster());
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
	if ( ! tgtval.is_object() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("AsBroadcaster.initialize(%s): first arg is "
                "not an object"), tgtval); 
		);
		return as_value();
	}

	boost::intrusive_ptr<as_object> tgt = tgtval.to_object();
	if ( ! tgt )
	{
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
	boost::intrusive_ptr<as_object> obj = fn.this_ptr;

	as_value newListener; assert(newListener.is_undefined());
	if ( fn.nargs ) newListener = fn.arg(0);

	obj->callMethod(NSV::PROP_REMOVE_LISTENER, newListener);

	as_value listenersValue;

	// TODO: test if we're supposed to crawl the target object's 
	//       inheritance chain in case it's own property _listeners 
	//       has been deleted while another one is found in any base
	//       class.
	if ( ! obj->get_member(NSV::PROP_uLISTENERS, &listenersValue) )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%p.addListener(%s): this object has no _listeners member"),
			(void*)fn.this_ptr.get(),
			fn.dump_args());
		);
		return as_value(true); // odd, but seems the case..
	}

	// assuming no automatic primitive-to-object cast will return an array...
	if (!listenersValue.is_object())
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%p.addListener(%s): this object's _listener isn't "
                "an object: %s"),
			(void*)fn.this_ptr.get(),
			fn.dump_args(), listenersValue);
		);
		return as_value(false); // TODO: check this
	}

	boost::intrusive_ptr<as_object> listenersObj = listenersValue.to_object();
	assert(listenersObj);

	boost::intrusive_ptr<Array_as> listeners = boost::dynamic_pointer_cast<Array_as>(listenersObj);
	if ( ! listeners )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%p.addListener(%s): this object's _listener isn't an array: %s -- will call 'push' on it anyway"),
			(void*)fn.this_ptr.get(),
			fn.dump_args(), listenersValue);
		);

		listenersObj->callMethod(NSV::PROP_PUSH, newListener);

	}
	else
	{
		listeners->push(newListener);
	}

	return as_value(true);

}


as_value
asbroadcaster_removeListener(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = fn.this_ptr;

	as_value listenersValue;

	// TODO: test if we're supposed to crawl the target object's 
	//       inheritance chain in case it's own property _listeners 
	//       has been deleted while another one is found in any base
	//       class.
	if (!obj->get_member(NSV::PROP_uLISTENERS, &listenersValue) )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%p.addListener(%s): this object has no _listeners "
                "member"), (void*)fn.this_ptr.get(), fn.dump_args());
		);
		return as_value(false); // TODO: check this
	}

	// assuming no automatic primitive-to-object cast will return an array...
	if ( ! listenersValue.is_object() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%p.addListener(%s): this object's _listener isn't "
                "an object: %s"), (void*)fn.this_ptr.get(), fn.dump_args(),
                listenersValue);
		);
		return as_value(false); // TODO: check this
	}

	boost::intrusive_ptr<as_object> listenersObj = listenersValue.to_object();
	assert(listenersObj);

	as_value listenerToRemove; assert(listenerToRemove.is_undefined());
	if ( fn.nargs ) listenerToRemove = fn.arg(0);

	boost::intrusive_ptr<Array_as> listeners = 
        boost::dynamic_pointer_cast<Array_as>(listenersObj);
	if ( ! listeners )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%p.addListener(%s): this object's _listener isn't an "
                "array: %s"), (void*)fn.this_ptr.get(), fn.dump_args(),
                listenersValue);
		);

		// TODO: implement brute force scan of pseudo-array
		unsigned int length = 
            listenersObj->getMember(NSV::PROP_LENGTH).to_int();

        string_table& st = obj->getVM().getStringTable();

		for (unsigned int i=0; i<length; ++i)
		{
			as_value iVal(i);
			std::string n = iVal.to_string();
			as_value v = listenersObj->getMember(st.find(n));
			if ( v.equals(listenerToRemove) )
			{
				listenersObj->callMethod(NSV::PROP_SPLICE, iVal, as_value(1));
				return as_value(true); 
			}
		}

		return as_value(false); // TODO: check this
	}
	else
	{
		// Remove the first listener matching the new value
		// See http://www.senocular.com/flash/tutorials/
        // listenersasbroadcaster/?page=2
		// TODO: make this call as a normal (don't want to
        // rely on _listeners type at all)
		bool removed = listeners->removeFirst(listenerToRemove);
		return as_value(removed);
	}

}


as_value
asbroadcaster_broadcastMessage(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = fn.this_ptr;

	as_value listenersValue;

	// TODO: test if we're supposed to crawl the target object's 
	//       inheritance chain in case its own property _listeners 
	//       has been deleted while another one is found in any base
	//       class.
	if ( ! obj->get_member(NSV::PROP_uLISTENERS, &listenersValue) )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror(_("%p.addListener(%s): this object has no "
                    "_listeners member"), (void*)fn.this_ptr.get(),
			        fn.dump_args());
		);
		return as_value(); // TODO: check this
	}

	// assuming no automatic primitive-to-object cast will return an array...
	if ( ! listenersValue.is_object() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%p.addListener(%s): this object's _listener "
                "isn't an object: %s"), (void*)fn.this_ptr.get(),
			    fn.dump_args(), listenersValue);
		);
		return as_value(); // TODO: check this
	}

	boost::intrusive_ptr<Array_as> listeners =
        boost::dynamic_pointer_cast<Array_as>(listenersValue.to_object());

    if ( ! listeners )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%p.addListener(%s): this object's _listener "
                "isn't an array: %s"), (void*)fn.this_ptr.get(),
			    fn.dump_args(), listenersValue);
		);
		return as_value(); // TODO: check this
	}

	if (!fn.nargs)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("%p.broadcastMessage() needs an argument", 
            (void*)fn.this_ptr.get());
		);
		return as_value();
	}

	BroadcasterVisitor visitor(fn); 
	listeners->visitAll(visitor);

	unsigned int dispatched = visitor.eventsDispatched();

	if ( dispatched ) return as_value(true);

	return as_value(); 

}

as_object*
getAsBroadcasterInterface()
{
	static boost::intrusive_ptr<as_object> o=NULL;
	if ( o == NULL )
	{
		o = new as_object(getObjectInterface());
		VM::get().addStatic(o.get());
	}
	return o.get();
}

as_value
asbroadcaster_ctor(const fn_call& /*fn*/)
{
	as_object* obj = new as_object(getAsBroadcasterInterface());
	return as_value(obj);
}

} // anonymous namespace

} // end of gnash namespace
