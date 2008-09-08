// Selection.cpp:  Selectable graphical things, for Gnash.
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

#include "Selection.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "Object.h" // for getObjectInterface

namespace gnash {

as_value selection_addlistener(const fn_call& fn);
as_value selection_getbeginindex(const fn_call& fn);
as_value selection_getcaretindex(const fn_call& fn);
as_value selection_getendindex(const fn_call& fn);
as_value selection_getfocus(const fn_call& fn);
as_value selection_removelistener(const fn_call& fn);
as_value selection_setfocus(const fn_call& fn);
as_value selection_setselection(const fn_call& fn);
as_value selection_ctor(const fn_call& fn);

static void
attachSelectionInterface(as_object& o)
{
	o.init_member("addListener", new builtin_function(selection_addlistener));
	o.init_member("getBeginIndex", new builtin_function(selection_getbeginindex));
	o.init_member("getCaretIndex", new builtin_function(selection_getcaretindex));
	o.init_member("getEndIndex", new builtin_function(selection_getendindex));
	o.init_member("getFocus", new builtin_function(selection_getfocus));
	o.init_member("removeListener", new builtin_function(selection_removelistener));
	o.init_member("setFocus", new builtin_function(selection_setfocus));
	o.init_member("setSelection", new builtin_function(selection_setselection));
}

static as_object*
getSelectionInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object(getObjectInterface());
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
	//std::string get_text_value() const { return "Selection"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};

as_value selection_addlistener(const fn_call& /*fn*/) {
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}
as_value selection_getbeginindex(const fn_call& /*fn*/) {
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}
as_value selection_getcaretindex(const fn_call& /*fn*/) {
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}
as_value selection_getendindex(const fn_call& /*fn*/) {
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}
as_value selection_getfocus(const fn_call& /*fn*/) {
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}
as_value selection_removelistener(const fn_call& /*fn*/) {
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}
as_value selection_setfocus(const fn_call& /*fn*/) {
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}
as_value selection_setselection(const fn_call& /*fn*/) {
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
selection_ctor(const fn_call& /* fn */)
{
	boost::intrusive_ptr<as_object> obj = new selection_as_object;
	
	return as_value(obj.get()); // will keep alive
}

// extern (used by Global.cpp)
void
selection_class_init(as_object& global)
{
	// Selection is NOT a class, but a simple object, see Selection.as

	static boost::intrusive_ptr<as_object> obj = new as_object(getObjectInterface());
	attachSelectionInterface(*obj);
	global.init_member("Selection", obj.get());

}


} // end of gnash namespace
