// AsBroadcaster.cpp - AsBroadcaster AS interface
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

#include "array.h" // for _listeners construction
#include "log.h"
#include "AsBroadcaster.h"
#include "fn_call.h"
#include "builtin_function.h"
#include "VM.h" // for getPlayerVersion() 
#include "Object.h" // for getObjectInterface
#include "namedStrings.h"

#include <boost/algorithm/string/case_conv.hpp> // for PROPNAME

namespace gnash {

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
		_eventKey = VM::get().getStringTable().find(_eventName);
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

		if ( method.is_function() )
		{

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

void 
AsBroadcaster::initialize(as_object& o)
{
	as_object* asb = getAsBroadcaster();

	//log_debug("Initializing object %p as an AsBroadcaster", (void*)&o);

	as_value tmp;

	if ( asb->get_member(NSV::PROP_ADD_LISTENER, &tmp) )
	{
		o.set_member(NSV::PROP_ADD_LISTENER, tmp);
	}

	if ( asb->get_member(NSV::PROP_REMOVE_LISTENER, &tmp) )
	{
		o.set_member(NSV::PROP_REMOVE_LISTENER, tmp);
	}
	
	o.set_member(NSV::PROP_BROADCAST_MESSAGE, new builtin_function(AsBroadcaster::broadcastMessage_method));
	o.set_member(NSV::PROP_uLISTENERS, new Array_as());

#ifndef NDEBUG
	assert(o.get_member(NSV::PROP_uLISTENERS, &tmp));
	assert(tmp.is_object());
	assert(o.get_member(NSV::PROP_BROADCAST_MESSAGE, &tmp));
	assert(tmp.is_function());

#if 0 // we can't rely on the following, due to possible override 
      // of the AsBroadcaster properties used to intialize this
      // object
	assert(o.get_member(NSV::PROP_ADD_LISTENER, &tmp));
	assert(tmp.is_function());
	assert(o.get_member(NSV::PROP_REMOVE_LISTENER, &tmp));
	assert(tmp.is_function());
#endif // 0

#endif
}

as_value
AsBroadcaster::initialize_method(const fn_call& fn)
{
	// TODO: initialize first arg object as an AsBroadcaster
	//       (call the AsBroadcaster::initialize(as_object*) static ?)
	if ( fn.nargs < 1 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("AsBroadcaster.initialize() requires one argument, none given"));
		);
		return as_value();
	}

	// TODO: check if automatic primitive to object conversion apply here
	const as_value& tgtval = fn.arg(0);
	if ( ! tgtval.is_object() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("AsBroadcaster.initialize(%s): first arg is not an object"), tgtval); 
		);
		return as_value();
	}

	boost::intrusive_ptr<as_object> tgt = tgtval.to_object();
	if ( ! tgt )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("AsBroadcaster.initialize(%s): first arg is an object"
            " but doesn't cast to one (dangling character ref?)"), tgtval); 
		);
		return as_value();
	}

	AsBroadcaster::initialize(*tgt);

	//log_debug("AsBroadcaster.initialize(%s): TESTING", tgtval);

	return as_value();
}

as_value
AsBroadcaster::addListener_method(const fn_call& fn)
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
	if ( ! listenersValue.is_object() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%p.addListener(%s): this object's _listener isn't an object: %s"),
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

	//log_debug("%p.addListener(%s) TESTING", (void*)fn.this_ptr.get(), fn.dump_args());
	return as_value(true);

}

as_value
AsBroadcaster::removeListener_method(const fn_call& fn)
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
		log_aserror(_("%p.addListener(%s): this object has no _listeners member"),
			(void*)fn.this_ptr.get(),
			fn.dump_args());
		);
		return as_value(false); // TODO: check this
	}

	// assuming no automatic primitive-to-object cast will return an array...
	if ( ! listenersValue.is_object() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%p.addListener(%s): this object's _listener isn't an object: %s"),
			(void*)fn.this_ptr.get(),
			fn.dump_args(), listenersValue);
		);
		return as_value(false); // TODO: check this
	}

	boost::intrusive_ptr<as_object> listenersObj = listenersValue.to_object();
	assert(listenersObj);

	as_value listenerToRemove; assert(listenerToRemove.is_undefined());
	if ( fn.nargs ) listenerToRemove = fn.arg(0);

	boost::intrusive_ptr<Array_as> listeners = boost::dynamic_pointer_cast<Array_as>(listenersObj);
	if ( ! listeners )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%p.addListener(%s): this object's _listener isn't an array: %s"),
			(void*)fn.this_ptr.get(),
			fn.dump_args(), listenersValue);
		);

		// TODO: implement brute force scan of pseudo-array
		unsigned int length = listenersObj->getMember(NSV::PROP_LENGTH).to_int();
		for (unsigned int i=0; i<length; ++i)
		{
			as_value iVal(i);
			std::string n = iVal.to_string();
			as_value v = listenersObj->getMember(VM::get().getStringTable().find(n));
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
		// See http://www.senocular.com/flash/tutorials/listenersasbroadcaster/?page=2
		// TODO: make this call as a normal (don't want to rely on _listeners type at all)
		bool removed = listeners->removeFirst(listenerToRemove);
		return as_value(removed);
	}

}

as_value
AsBroadcaster::broadcastMessage_method(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = fn.this_ptr;

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
		return as_value(); // TODO: check this
	}

	// assuming no automatic primitive-to-object cast will return an array...
	if ( ! listenersValue.is_object() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%p.addListener(%s): this object's _listener isn't an object: %s"),
			(void*)fn.this_ptr.get(),
			fn.dump_args(), listenersValue);
		);
		return as_value(); // TODO: check this
	}

	boost::intrusive_ptr<Array_as> listeners = boost::dynamic_pointer_cast<Array_as>(listenersValue.to_object());
	if ( ! listeners )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%p.addListener(%s): this object's _listener isn't an array: %s"),
			(void*)fn.this_ptr.get(),
			fn.dump_args(), listenersValue);
		);
		return as_value(); // TODO: check this
	}

	if ( ! fn.nargs )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("%p.broadcastMessage() needs an argument", (void*)fn.this_ptr.get());
		);
		return as_value();
	}

	BroadcasterVisitor visitor(fn); 
	listeners->visitAll(visitor);

	unsigned int dispatched = visitor.eventsDispatched();

	//log_debug("AsBroadcaster.broadcastMessage() dispatched %u events", dispatched);

	if ( dispatched ) return as_value(true);
	else return as_value(); // undefined

}

static as_object*
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

static as_value
AsBroadcaster_ctor(const fn_call& /*fn*/)
{
	as_object* obj = new as_object(getAsBroadcasterInterface());
	return as_value(obj); // will keep alive
}

as_object*
AsBroadcaster::getAsBroadcaster()
{
	VM& vm = VM::get();
	int swfVersion = vm.getSWFVersion();

	static boost::intrusive_ptr<as_object> obj = NULL;
	if ( ! obj )
	{
		obj = new builtin_function(AsBroadcaster_ctor, getAsBroadcasterInterface()); 
		VM::get().addStatic(obj.get()); // correct ?
		if ( swfVersion >= 6 )
		{
			// NOTE: we may add NSV::PROP_INITIALIZE, unavailable at time of writing.
			//       anyway, since AsBroadcaster is the only class we know using an 'initialize'
			//       method we might as well save the string_table size in case we'll not load
			//       the class.
			obj->init_member("initialize", new builtin_function(AsBroadcaster::initialize_method));

			obj->init_member(NSV::PROP_ADD_LISTENER, new builtin_function(AsBroadcaster::addListener_method));
			obj->init_member(NSV::PROP_REMOVE_LISTENER, new builtin_function(AsBroadcaster::removeListener_method));
			obj->init_member(NSV::PROP_BROADCAST_MESSAGE, new builtin_function(AsBroadcaster::broadcastMessage_method));
		}
	}

	return obj.get();
}

void
AsBroadcaster_init(as_object& global)
{
	// _global.AsBroadcaster is NOT a class, but a simple object
	global.init_member("AsBroadcaster", AsBroadcaster::getAsBroadcaster());
}

} // end of gnash namespace
