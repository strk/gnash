// AsBroadcaster.cpp - AsBroadcaster AS interface
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

#include "array.h" // for _listeners construction
#include "log.h"
#include "AsBroadcaster.h"
#include "fn_call.h"
#include "builtin_function.h"
#include "VM.h" // for getPlayerVersion() 
#include "Object.h" // for getObjectInterface

#include <boost/algorithm/string/case_conv.hpp> // for PROPNAME

namespace gnash {

class BroadcasterVisitor
{
	
	/// Name of the event being broadcasted
	/// appropriately cased based on SWF version
	/// of the current VM
	std::string _eventName;

	/// Environment to use for marhalling and functions invokation
	as_environment& _env;

	// These two will be needed for consistency checking
	//size_t _origEnvStackSize;
	//size_t _origEnvCallStackSize;

public:

	/// @param eName name of event, will be converted to lowercase if needed
	///
	/// @param env Environment to use for marhalling and functions invocation.
	///	   Note that visit() will push values on it !
	///
	BroadcasterVisitor(const std::string& eName, as_environment& env)
		:
		_eventName(PROPNAME(eName)),
		_env(env)
	{
	}

	/// Call a method on the given value
	void visit(as_value& v)
	{
		boost::intrusive_ptr<as_object> o = v.to_object();
		if ( ! o ) return;

#ifndef NDEBUG
		size_t oldStackSize = _env.stack_size();
#endif
		/*as_value ret =*/ o->callMethod(_eventName, _env);
		assert ( _env.stack_size() == oldStackSize );
	}
};

void 
AsBroadcaster::initialize(as_object& o)
{
	log_debug("Initializing object %p as an AsBroadcaster", (void*)&o);
	// TODO: reserch on protection flags for these methods
	o.set_member(PROPNAME("addListener"), new builtin_function(AsBroadcaster::addListener_method));
	o.set_member(PROPNAME("removeListener"), new builtin_function(AsBroadcaster::removeListener_method));
	o.set_member(PROPNAME("broadcastMessage"), new builtin_function(AsBroadcaster::broadcastMessage_method));
	o.set_member("_listeners", new as_array_object());

#ifndef NDEBUG
	as_value tmp;
	assert(o.get_member("_listeners", &tmp));
	assert(tmp.is_object());
	assert(o.get_member(PROPNAME("addListener"), &tmp));
	assert(tmp.is_function());
	assert(o.get_member(PROPNAME("removeListener"), &tmp));
	assert(tmp.is_function());
	assert(o.get_member(PROPNAME("broadcastMessage"), &tmp));
	assert(tmp.is_function());
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
	as_value& tgtval = fn.arg(0);
	if ( ! tgtval.is_object() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("AsBroadcaster.initialize(%s): first arg is not an object"), tgtval.to_debug_string().c_str()); 
		);
		return as_value();
	}

	boost::intrusive_ptr<as_object> tgt = tgtval.to_object();

	AsBroadcaster::initialize(*tgt);

	log_debug("AsBroadcaster.initialize(%s): TESTING", tgtval.to_debug_string().c_str());

	return as_value();
}

as_value
AsBroadcaster::addListener_method(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = fn.this_ptr;

	as_value newListener; assert(newListener.is_undefined());
	if ( fn.nargs ) newListener = fn.arg(0);

	obj->callMethod(PROPNAME("removeListener"), fn.env(), newListener);

	as_value listenersValue;

	// TODO: test if we're supposed to crawl the target object's 
	//       inheritance chain in case it's own property _listeners 
	//       has been deleted while another one is found in any base
	//       class.
	if ( ! obj->get_member("_listeners", &listenersValue) )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%p.addListener(%s): this object has no _listeners member"),
			(void*)fn.this_ptr.get(),
			fn.dump_args().c_str());
		);
		return as_value(true); // odd, but seems the case..
	}

	// assuming no automatic primitive-to-object cast will return an array...
	if ( ! listenersValue.is_object() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%p.addListener(%s): this object's _listener isn't an object: %s"),
			(void*)fn.this_ptr.get(),
			fn.dump_args().c_str(), listenersValue.to_debug_string().c_str());
		);
		return as_value(false); // TODO: check this
	}

	boost::intrusive_ptr<as_object> listenersObj = listenersValue.to_object();
	assert(listenersObj);

	boost::intrusive_ptr<as_array_object> listeners = boost::dynamic_pointer_cast<as_array_object>(listenersObj);
	if ( ! listeners )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%p.addListener(%s): this object's _listener isn't an array: %s -- will call 'push' on it anyway"),
			(void*)fn.this_ptr.get(),
			fn.dump_args().c_str(), listenersValue.to_debug_string().c_str());
		);

		listenersObj->callMethod(PROPNAME("push"), fn.env(), newListener);

	}
	else
	{
		listeners->push(newListener);
	}

	log_debug("%p.addListener(%s) TESTING", (void*)fn.this_ptr.get(), fn.dump_args().c_str());
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
	if ( ! obj->get_member("_listeners", &listenersValue) )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%p.addListener(%s): this object has no _listeners member"),
			(void*)fn.this_ptr.get(),
			fn.dump_args().c_str());
		);
		return as_value(false); // TODO: check this
	}

	// assuming no automatic primitive-to-object cast will return an array...
	if ( ! listenersValue.is_object() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%p.addListener(%s): this object's _listener isn't an object: %s"),
			(void*)fn.this_ptr.get(),
			fn.dump_args().c_str(), listenersValue.to_debug_string().c_str());
		);
		return as_value(false); // TODO: check this
	}

	boost::intrusive_ptr<as_object> listenersObj = listenersValue.to_object();
	assert(listenersObj);

	as_value listenerToRemove; assert(listenerToRemove.is_undefined());
	if ( fn.nargs ) listenerToRemove = fn.arg(0);

	boost::intrusive_ptr<as_array_object> listeners = boost::dynamic_pointer_cast<as_array_object>(listenersObj);
	if ( ! listeners )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%p.addListener(%s): this object's _listener isn't an array: %s"),
			(void*)fn.this_ptr.get(),
			fn.dump_args().c_str(), listenersValue.to_debug_string().c_str());
		);

		// TODO: implement brute force scan of pseudo-array
		unsigned int length = listenersObj->getMember("length").to_int(fn.env());
		for (unsigned int i=0; i<length; ++i)
		{
			as_value iVal(i);
			std::string n = iVal.to_string(&(fn.env()));
			as_value v = listenersObj->getMember(n);
			if ( v.equals(listenerToRemove, fn.env()) )
			{
				listenersObj->callMethod("splice", fn.env(), iVal, as_value(1));
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
		bool removed = listeners->removeFirst(listenerToRemove, fn.env());
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
	if ( ! obj->get_member("_listeners", &listenersValue) )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%p.addListener(%s): this object has no _listeners member"),
			(void*)fn.this_ptr.get(),
			fn.dump_args().c_str());
		);
		return as_value(); // TODO: check this
	}

	// assuming no automatic primitive-to-object cast will return an array...
	if ( ! listenersValue.is_object() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%p.addListener(%s): this object's _listener isn't an object: %s"),
			(void*)fn.this_ptr.get(),
			fn.dump_args().c_str(), listenersValue.to_debug_string().c_str());
		);
		return as_value(); // TODO: check this
	}

	boost::intrusive_ptr<as_array_object> listeners = boost::dynamic_pointer_cast<as_array_object>(listenersValue.to_object());
	if ( ! listeners )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%p.addListener(%s): this object's _listener isn't an array: %s"),
			(void*)fn.this_ptr.get(),
			fn.dump_args().c_str(), listenersValue.to_debug_string().c_str());
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

	BroadcasterVisitor visitor(fn.arg(0).to_string(), fn.env());
	listeners->visitAll(visitor);

	log_debug("AsBroadcaster.broadcastMessage TESTING");

	return as_value(true);
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

void
AsBroadcaster_init(as_object& global)
{
	// _global.AsBroadcaster is NOT a class, but a simple object

	VM& vm = VM::get();
	int swfVersion = vm.getSWFVersion();

	static boost::intrusive_ptr<as_object> obj = NULL;
	if ( ! obj )
	{
		obj = new builtin_function(AsBroadcaster_ctor, getAsBroadcasterInterface()); 
		VM::get().addStatic(obj.get()); // correct ?
		if ( swfVersion >= 6 )
		{
			obj->init_member("initialize", new builtin_function(AsBroadcaster::initialize_method));
		}
	}
	global.init_member("AsBroadcaster", obj.get());
}

} // end of gnash namespace
