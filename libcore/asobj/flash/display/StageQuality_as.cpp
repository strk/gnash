// StageQuality_as.cpp:  ActionScript "StageQuality" class, for Gnash.
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

#include "display/StageQuality_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "Object.h"

namespace gnash {

// Forward declarations
namespace {
    void attachStageQualityStaticInterface(as_object& o);
}

// extern (used by Global.cpp)
void
stagequality_class_init(as_object& where)
{
    static boost::intrusive_ptr<as_object> obj =
        new as_object(getObjectInterface());

    attachStageQualityStaticInterface(*obj);
    where.init_member("StageQuality", obj.get());
}

namespace {

void
attachStageQualityStaticInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    // TODO: attach constants.
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

