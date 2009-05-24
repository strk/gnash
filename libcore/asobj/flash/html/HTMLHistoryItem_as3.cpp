// HTMLHistoryItem_as3.cpp:  ActionScript "HTMLHistoryItem" class, for Gnash.
//
//   Copyright (C) 2009 Free Software Foundation, Inc.
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

#include "html/HTMLHistoryItem_as3.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value htmlhistoryitem_ctor(const fn_call& fn);
    void attachHTMLHistoryItemInterface(as_object& o);
    void attachHTMLHistoryItemStaticInterface(as_object& o);
    as_object* getHTMLHistoryItemInterface();

}

// extern (used by Global.cpp)
void htmlhistoryitem_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&htmlhistoryitem_ctor, getHTMLHistoryItemInterface());
        attachHTMLHistoryItemStaticInterface(*cl);
    }

    // Register _global.HTMLHistoryItem
    global.init_member("HTMLHistoryItem", cl.get());
}

namespace {

void
attachHTMLHistoryItemInterface(as_object& o)
{
}

void
attachHTMLHistoryItemStaticInterface(as_object& o)
{

}

as_object*
getHTMLHistoryItemInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachHTMLHistoryItemInterface(*o);
    }
    return o.get();
}

as_value
htmlhistoryitem_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new HTMLHistoryItem_as3;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

