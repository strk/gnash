// VM.cpp: the Virtual Machine class, for Gnash
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

/* $Id: VM.cpp,v 1.19 2007/09/19 14:20:51 cmusick Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "VM.h"
#include "movie_definition.h"
#include "movie_instance.h"
#include "movie_root.h"
#include "Global.h"
#include "tu_timer.h" // for tu_timer::get_ticks()
#include "rc.h" //for overriding default version string with rcfile

#include <memory>

namespace {
gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
}

namespace gnash {

// Pointer to our singleton
std::auto_ptr<VM> VM::_singleton;

VM&
VM::init(movie_definition& movie)
{
	// Don't call more then once !
	assert(!_singleton.get());

	_singleton.reset(new VM(movie));

	assert(_singleton.get());

// Load up our pre-known properties.
string_table::svt preload_properties[] =
{
	{ "addListener", as_object::PROP_ADD_LISTENER },
	{ "align", as_object::PROP_ALIGN },
	{ "_alpha", as_object::PROP_uALPHA },
	{ "blockIndent", as_object::PROP_BLOCK_INDENT },
	{ "bold", as_object::PROP_BOLD },
	{ "broadcastMessage", as_object::PROP_BROADCAST_MESSAGE },
	{ "bullet", as_object::PROP_BULLET },
	{ "callee", as_object::PROP_CALLEE },
	{ "color", as_object::PROP_COLOR },
	{ "constructor", as_object::PROP_CONSTRUCTOR },
	{ "__constructor__", as_object::PROP_uuCONSTRUCTORuu },
	{ "_currentframe", as_object::PROP_uCURRENTFRAME },
	{ "_droptarget", as_object::PROP_uDROPTARGET },
	{ "enabled", as_object::PROP_ENABLED },
	{ "_focusrect", as_object::PROP_uFOCUSRECT },
	{ "_framesloaded", as_object::PROP_uFRAMESLOADED },
	{ "_height", as_object::PROP_uHEIGHT },
	{ "_highquality", as_object::PROP_uHIGHQUALITY },
	{ "htmlText", as_object::PROP_HTML_TEXT },
	{ "indent", as_object::PROP_INDENT },
	{ "italic", as_object::PROP_ITALIC },
	{ "leading", as_object::PROP_LEADING },
	{ "left_margin", as_object::PROP_LEFT_MARGIN },
	{ "length", as_object::PROP_LENGTH },
	{ "_listeners", as_object::PROP_uLISTENERS },
	{ "loaded", as_object::PROP_LOADED },
	{ "_name", as_object::PROP_uNAME },
	{ "onLoad", as_object::PROP_ON_LOAD },
	{ "onResize", as_object::PROP_ON_RESIZE },
	{ "onRollOut", as_object::PROP_ON_ROLL_OUT },
	{ "onRollOver", as_object::PROP_ON_ROLL_OVER },
	{ "onSelect", as_object::PROP_ON_SELECT },
	{ "onStatus", as_object::PROP_ON_STATUS },
	{ "_parent", as_object::PROP_uPARENT },
	{ "__proto__", as_object::PROP_uuPROTOuu },
	{ "prototype", as_object::PROP_PROTOTYPE },
	{ "push", as_object::PROP_PUSH },
	{ "removeListener", as_object::PROP_REMOVE_LISTENER },
	{ "rightMargin", as_object::PROP_RIGHT_MARGIN },
	{ "_rotation", as_object::PROP_uROTATION },
	{ "scaleMode", as_object::PROP_SCALE_MODE },
	{ "size", as_object::PROP_SIZE },
	{ "_soundbuftime", as_object::PROP_uSOUNDBUFTIME },
	{ "splice", as_object::PROP_SPLICE },
	{ "Stage", as_object::PROP_iSTAGE },
	{ "status", as_object::PROP_STATUS },
	{ "_target", as_object::PROP_uTARGET },
	{ "text", as_object::PROP_TEXT },
	{ "textColor", as_object::PROP_TEXT_COLOR },
	{ "textWidth", as_object::PROP_TEXT_WIDTH },
	{ "toString", as_object::PROP_TO_STRING },
	{ "_totalframes", as_object::PROP_uTOTALFRAMES },
	{ "underline", as_object::PROP_UNDERLINE },
	{ "_url", as_object::PROP_uURL },
	{ "valueOf", as_object::PROP_VALUE_OF },
	{ "_visible", as_object::PROP_uVISIBLE },
	{ "_width", as_object::PROP_uWIDTH },
	{ "x", as_object::PROP_X },
	{ "_x", as_object::PROP_uX },
	{ "_xmouse", as_object::PROP_uXMOUSE },
	{ "_xscale", as_object::PROP_uXSCALE },
	{ "y", as_object::PROP_Y },
	{ "_y", as_object::PROP_uY },
	{ "_ymouse", as_object::PROP_uYMOUSE },
	{ "_yscale", as_object::PROP_uYSCALE }
};
	if (_singleton->getSWFVersion() < 7)
	{
		_singleton->mStringTable.lower_next_group();
	}
	_singleton->mStringTable.insert_group(preload_properties,
		sizeof (preload_properties) / sizeof (string_table::svt));

	_singleton->setGlobal(new Global(*_singleton));
	assert(_singleton->getGlobal());

	return *_singleton;
}

VM&
VM::get()
{
	// Did you call VM::init ?
	assert(_singleton.get());
	return *_singleton;
}

bool
VM::isInitialized()
{
	return _singleton.get();
}

VM::VM(movie_definition& topmovie)
	:
	_root_movie(new movie_root()),
	_swfversion(topmovie.get_version()),
	_start_time(tu_timer::get_ticks())
{
}

VM::~VM()
{
	// nothing to do atm, but we'll likely
	// have to deregister lots of stuff when
	// things are setup
}

std::locale&
VM::getLocale() const
{
	// TODO: some research about what we should be using.
	//       IIRC older SWF contained some tags for this,
	//       while new SWF uses UNICODE...
	static std::locale loc("C");
	return loc;
}

int
VM::getSWFVersion() const
{
	return _swfversion;
}

const std::string&
VM::getPlayerVersion() const
{
	//From rcfile
	static const std::string version(rcfile.getFlashVersionString());
	return version;
}

movie_root&
VM::getRoot() const
{
	return *_root_movie;
}

/*public*/
as_object*
VM::getGlobal() const
{
	return _global.get();
}

/*private*/
void
VM::setGlobal(as_object* o)
{
	assert(!_global);
	_global = o;
}

uint64_t
VM::getTime()
{
  return  (tu_timer::get_ticks() -  _start_time);
}

void
VM::markReachableResources() const
{
#ifdef GNASH_USE_GC

	_root_movie->markReachableResources();

	_global->setReachable();

	/// Mark all static GcResources
	for (ResVect::const_iterator i=_statics.begin(), e=_statics.end(); i!=e; ++i)
	{
		(*i)->setReachable();
	}
#endif

}

void
VmGcRoot::markReachableResources() const
{
	_vm.markReachableResources();
}

} // end of namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
