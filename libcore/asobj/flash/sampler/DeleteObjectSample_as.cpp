// DeleteObjectSample_as.cpp:  ActionScript "DeleteObjectSample" class, for Gnash.
//
//   Copyright (C) 2009, 2010 Free Software Foundation, Inc.
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


#include "sampler/DeleteObjectSample_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value deleteobjectsample_ctor(const fn_call& fn);
    void attachDeleteObjectSampleInterface(as_object& o);
    void attachDeleteObjectSampleStaticInterface(as_object& o);
}

// extern (used by Global.cpp)
void
deleteobjectsample_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinClass(where, deleteobjectsample_ctor, attachDeleteObjectSampleInterface, 
        attachDeleteObjectSampleStaticInterface, uri);
}

namespace {

void
attachDeleteObjectSampleInterface(as_object& /*o*/)
{
}

void
attachDeleteObjectSampleStaticInterface(as_object& /*o*/)
{

}

as_value
deleteobjectsample_ctor(const fn_call& /*fn*/)
{
    return as_value();
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

