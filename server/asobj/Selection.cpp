// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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

#include "Selection.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function

namespace gnash {

void selection_addlistener(const fn_call& fn);
void selection_getbeginindex(const fn_call& fn);
void selection_getcaretindex(const fn_call& fn);
void selection_getendindex(const fn_call& fn);
void selection_getfocus(const fn_call& fn);
void selection_removelistener(const fn_call& fn);
void selection_setfocus(const fn_call& fn);
void selection_setselection(const fn_call& fn);
void selection_ctor(const fn_call& fn);

static void
attachSelectionInterface(as_object& o)
{
	o.set_member("addListener", &selection_addlistener);
	o.set_member("getBeginIndex", &selection_getbeginindex);
	o.set_member("getCaretIndex", &selection_getcaretindex);
	o.set_member("getEndIndex", &selection_getendindex);
	o.set_member("getFocus", &selection_getfocus);
	o.set_member("removeListener", &selection_removelistener);
	o.set_member("setFocus", &selection_setfocus);
	o.set_member("setSelection", &selection_setselection);
}

static as_object*
getSelectionInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object();
		attachSelectionInterface(*o);
	}
	return o.get();
}

class selection_as_object: public as_object
{

public:

	selection_as_object()
		:
		as_object(getSelectionInterface())
	{}

	// override from as_object ?
	//const char* get_text_value() const { return "Selection"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};

void selection_addlistener(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}
void selection_getbeginindex(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}
void selection_getcaretindex(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}
void selection_getendindex(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}
void selection_getfocus(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}
void selection_removelistener(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}
void selection_setfocus(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}
void selection_setselection(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}

void
selection_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new selection_as_object;
	
	fn.result->set_as_object(obj.get()); // will keep alive
}

// extern (used by Global.cpp)
void
selection_class_init(as_object& global)
{
	// Selection is NOT a class, but a simple object, see Selection.as

	static boost::intrusive_ptr<as_object> obj = new as_object();
	attachSelectionInterface(*obj);
	global.set_member("Selection", obj.get());

}


} // end of gnash namespace

