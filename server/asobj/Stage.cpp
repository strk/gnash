// Stage.cpp:  All the world is one, for Gnash.
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

#include "Stage.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "VM.h"
#include "Object.h" // for getObjectInterface()
#include "AsBroadcaster.h" // for initializing self as a broadcaster
#include "namedStrings.h"

#include <string>

namespace gnash {

as_value stage_scalemode_getset(const fn_call& fn);
as_value stage_align_getset(const fn_call& fn);
as_value stage_showMenu_getset(const fn_call& fn);
as_value stage_width_getset(const fn_call& fn);
as_value stage_height_getset(const fn_call& fn);

static void
attachStageInterface(as_object& o)
{
	VM& vm = o.getVM();

	if ( vm.getSWFVersion() < 6 ) return;

	as_c_function_ptr getset;

	// Stage.scaleMode getter-setter
	getset = stage_scalemode_getset;
	vm.registerNative(getset, 666, 1);
	vm.registerNative(getset, 666, 2);
	o.init_property("scaleMode", getset, getset);

	// Stage.align getter-setter
	getset = stage_align_getset;
	vm.registerNative(getset, 666, 3);
	vm.registerNative(getset, 666, 4);
	o.init_property("align", getset, getset);

	// Stage.width getter-setter
	getset = stage_width_getset;
	vm.registerNative(getset, 666, 5);
	vm.registerNative(getset, 666, 6);
	o.init_property("width", getset, getset);

	// Stage.height getter-setter
	getset = stage_height_getset;
	vm.registerNative(getset, 666, 7);
	vm.registerNative(getset, 666, 8);
	o.init_property("height", getset, getset);

	// Stage.showMenu getter-setter
	getset = stage_showMenu_getset;
	vm.registerNative(getset, 666, 9);
	vm.registerNative(getset, 666, 10);
	o.init_property("showMenu", getset, getset);

}

Stage::Stage()
	:
	as_object(getObjectInterface()),
	_scaleMode(showAll),
	_alignMode(ALIGN_MODE_NONE)
{
	attachStageInterface(*this);

	int swfversion = _vm.getSWFVersion();
	if ( swfversion > 5 )
	{
		AsBroadcaster::initialize(*this);
	}
}

void
Stage::onResize()
{
	as_value v;
	if (get_member(NSV::PROP_SCALE_MODE, &v) && v.to_string() == "noScale" )
	{
		notifyResize();
	}
}

void
Stage::notifyResize()
{
	log_debug("notifying Stage listeners about a resize");
	callMethod(NSV::PROP_BROADCAST_MESSAGE, "onResize");
}

/// Expected behaviour is that the original movie size is aways returned
/// as long as scaling is allowed.
unsigned int
Stage::getWidth() const
{

    movie_root& m = VM::get().getRoot();

    if (_scaleMode == noScale)
    {
        return m.getWidth();    
    }
    return static_cast<unsigned int>(m.get_movie_definition()->get_width_pixels());
	
}

unsigned int
Stage::getHeight() const
{

    movie_root& m = VM::get().getRoot();

    if (_scaleMode == noScale)
    {
        return m.getHeight();    
    }
    return static_cast<unsigned int>(m.get_movie_definition()->get_height_pixels());

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

const char*
Stage::getAlignModeString()
{
	static const char* alignName[] = {
		"T",
		"B",
		"L",
		"R",
		"LT",
		"TR",
		"LB",
		"RB",
		"" };

	return alignName[_alignMode];
}

void
Stage::setScaleMode(ScaleMode mode)
{
	if ( _scaleMode == mode ) return; // nothing to do

	_scaleMode = mode;

	if ( _scaleMode == noScale )
	{
		VM::get().getRoot().allowRescaling(false);
	}
	else
	{
		VM::get().getRoot().allowRescaling(true);
	}
}

void
Stage::setAlignMode(AlignMode mode)
{
	if ( _alignMode == mode ) return; // nothing to do

	_alignMode = mode;

	static bool warned = false;
	if ( ! warned ) {
		log_unimpl("Stage.align goes through "
		            "the motions but is not implemented");
		warned = true;
	}

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

		const std::string& str = fn.arg(0).to_string();
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

as_value
stage_align_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Stage> stage = ensureType<Stage>(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
	    return as_value (stage->getAlignModeString());
	}
	else // setter
	{
		Stage::AlignMode mode;

		const std::string& str = fn.arg(0).to_string();

		if ( str == "T" ) mode = Stage::T;
		else if ( str == "B" ) mode = Stage::B;
		else if ( str == "L" ) mode = Stage::L;
		else if ( str == "R" ) mode = Stage::R;
		else if ( str == "LT" || str == "TL" ) mode = Stage::LT;
		else if ( str == "TR" || str == "RT" ) mode = Stage::TR;
		else if ( str == "LB" || str == "BL" ) mode = Stage::LB;
		else if ( str == "RB" || str == "BR" ) mode = Stage::RB;
        else mode = Stage::ALIGN_MODE_NONE;
        
		stage->setAlignMode(mode);

		return as_value();
	}
}

as_value
stage_showMenu_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Stage> stage = ensureType<Stage>(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
		static bool warned = false;
		if ( ! warned ) {
			log_unimpl("Stage.showMenu getter");
			warned=true;
		}
		return as_value();
	}
	else // setter
	{
		static bool warned = false;
		if ( ! warned ) {
			log_unimpl("Stage.showMenu setter");
			warned=true;
		}
		return as_value();
	}
}

// extern (used by Global.cpp)
void stage_class_init(as_object& global)
{
	static boost::intrusive_ptr<as_object> obj = new Stage();
	global.init_member("Stage", obj.get());
}

} // end of gnash namespace
