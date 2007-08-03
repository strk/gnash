// Stage.cpp:  All the world is one, for Gnash.
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

#include "Stage.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "VM.h"

#include <string>
#include <boost/algorithm/string/case_conv.hpp> // for using PROPNAME

namespace gnash {

as_value stage_addlistener(const fn_call& fn);
as_value stage_removelistener(const fn_call& fn);
as_value stage_scalemode_getset(const fn_call& fn);
as_value stage_width_getset(const fn_call& fn);
as_value stage_height_getset(const fn_call& fn);

static void
attachStageInterface(as_object& o)
{
	if ( VM::get().getSWFVersion() < 6 ) return;

	o.init_member("addListener", new builtin_function(stage_addlistener));
	o.init_member("removeListener", new builtin_function(stage_removelistener));

	boost::intrusive_ptr<builtin_function> getset(new builtin_function(stage_scalemode_getset));
	o.init_property("scaleMode", *getset, *getset);

	getset = new builtin_function(stage_width_getset);
	o.init_property("width", *getset, *getset);

	getset = new builtin_function(stage_height_getset);
	o.init_property("height", *getset, *getset);
}

Stage::Stage()
	:
	_scaleMode(showAll)
{
	attachStageInterface(*this);
}

void
Stage::onResize(as_environment* env)
{
	as_value v;
	if ( get_member(PROPNAME("scaleMode"), &v) && v.to_string(env) == "noScale" )
	{
		notifyResize(env);
	}
}

void
Stage::notifyResize(as_environment* env)
{
	for (ListenersList::iterator it=_listeners.begin(),
			itEnd=_listeners.end();
			it != itEnd; ++it)
	{
		notifyResize(*it, env);
	}
}

/// Notify an object about an resize event
void
Stage::notifyResize(boost::intrusive_ptr<as_object> obj, as_environment* env)
{
	std::string eventname = PROPNAME("onResize");

	as_value method;
	if ( ! obj->get_member(eventname, &method) ) {
		// nothing to do
		return;
	}

	boost::intrusive_ptr<as_function> func = method.to_as_function();
	if ( ! func ) return; // method is not a function

	func->call(fn_call(obj.get(), env, 0, 0));
}

unsigned
Stage::getWidth() const
{
	return VM::get().getRoot().getWidth();
}

unsigned
Stage::getHeight() const
{
	return VM::get().getRoot().getHeight();
}

void
Stage::addListener(boost::intrusive_ptr<as_object> obj)
{
	//log_msg("Adding listener %p to Stage %p", obj.get(), this);
	_listeners.push_back(obj);
}

void
Stage::removeListener(boost::intrusive_ptr<as_object> obj)
{
	//log_msg("Removing listener %p from Stage %p", obj.get(), this);
	_listeners.remove(obj);
}

const char*
Stage::getScaleModeString()
{
	static const char* modeName[] = {
		"showAll",
		"noScale",
		"exactFill",
		"noBorder" };

	return modeName[_scaleMode];
}

void
Stage::setScaleMode(ScaleMode mode)
{
	_scaleMode = mode;

	//log_msg("Scale mode set to %s", getScaleModeString());
	if ( _scaleMode == noScale )
	{
		//log_msg("Setting rescaling allowance to false");
		VM::get().getRoot().allowRescaling(false);
	}
}

as_value stage_addlistener(const fn_call& fn)
{
	boost::intrusive_ptr<Stage> stage = ensureType<Stage>(fn.this_ptr);

	if ( fn.nargs < 1 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Stage.addListener() needs one argument"));
		);
		return as_value();
	}

	boost::intrusive_ptr<as_object> obj = fn.arg(0).to_object();
	if ( ! obj )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss;
		fn.dump_args(ss);
		log_aserror(_("Invalid call to Stage.addListener(%s) : first arg doesn't cast to an object"),
			ss.str().c_str());
		);
		return as_value();
	}

	stage->addListener(obj);
	return as_value();
}

as_value stage_removelistener(const fn_call& fn)
{
	boost::intrusive_ptr<Stage> stage = ensureType<Stage>(fn.this_ptr);

	if ( fn.nargs < 1 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Stage.removeListener() needs one argument"));
		);
		return as_value();
	}

	boost::intrusive_ptr<as_object> obj = fn.arg(0).to_object();
	if ( ! obj )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss;
		fn.dump_args(ss);
		log_aserror(_("Invalid call to Stage.removeListener(%s) : first arg doesn't cast to an object"),
			ss.str().c_str());
		);
		return as_value();
	}

	stage->removeListener(obj);
	return as_value();
}

as_value stage_scalemode_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Stage> stage = ensureType<Stage>(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
		return as_value(stage->getScaleModeString());
	}
	else // setter
	{
		Stage::ScaleMode mode = Stage::showAll;

		const std::string& str = fn.arg(0).to_string(&(fn.env()));
		if ( str == "noScale" ) mode = Stage::noScale;
		else if ( str == "exactFill" ) mode = Stage::exactFill;
		else if ( str == "noBorder" ) mode = Stage::noBorder;

		stage->setScaleMode(mode);
		return as_value();
	}
}

as_value
stage_width_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Stage> stage = ensureType<Stage>(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
		return as_value(stage->getWidth());
	}
	else // setter
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Stage.width is a read-only property!"));
		);
		return as_value();
	}
}

as_value
stage_height_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Stage> stage = ensureType<Stage>(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
		return as_value(stage->getHeight());
	}
	else // setter
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Stage.height is a read-only property!"));
		);
		return as_value();
	}
}

// extern (used by Global.cpp)
void stage_class_init(as_object& global)
{
	static boost::intrusive_ptr<as_object> obj = new Stage();
	global.init_member("Stage", obj.get());
}

#ifdef GNASH_USE_GC
void
Stage::markReachableResources() const
{
        for (ListenersList::const_iterator i=_listeners.begin(), e=_listeners.end(); i!=e; ++i)
        {
                (*i)->setReachable();
        }

	// Invoke generic as_object marker
	markAsObjectReachable();
}
#endif // def GNASH_USE_GC

} // end of gnash namespace
