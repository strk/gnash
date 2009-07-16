// ID3Info_as.cpp:  ActionScript "ID3Info" class, for Gnash.
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

#include "media/ID3Info_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value id3info_ctor(const fn_call& fn);
    void attachID3InfoInterface(as_object& o);
    void attachID3InfoStaticInterface(as_object& o);
    as_object* getID3InfoInterface();

}

class ID3Info_as : public as_object
{

public:

    ID3Info_as()
        :
        as_object(getID3InfoInterface())
    {}
};

// extern (used by Global.cpp)
void id3info_class_init(as_object& global, const ObjectURI& uri)
{
    static boost::intrusive_ptr<as_object> cl;

    if (!cl) {
	Global_as* gl = getGlobal(global);
        cl = gl->createClass(&id3info_ctor, getID3InfoInterface());
        attachID3InfoStaticInterface(*cl);
    }

    // Register _global.ID3Info
    global.init_member("ID3Info", cl.get());
}

namespace {

void
attachID3InfoInterface(as_object& /*o*/)
{
}

void
attachID3InfoStaticInterface(as_object& /*o*/)
{

}

as_object*
getID3InfoInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachID3InfoInterface(*o);
    }
    return o.get();
}

as_value
id3info_ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new ID3Info_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

