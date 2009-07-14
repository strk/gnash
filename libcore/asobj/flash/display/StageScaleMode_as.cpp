// StageScaleMode_as.cpp:  ActionScript "StageScaleMode" class, for Gnash.
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

#include "display/StageScaleMode_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h"

namespace gnash {

// Forward declarations
namespace {
    void attachStageScaleModeStaticInterface(as_object& o);
    as_object* getStageScaleModeInterface();
}

void
stagescalemode_class_init(as_object& where)
{
    boost::intrusive_ptr<as_object> obj = new as_object(getObjectInterface());
    attachStageScaleModeStaticInterface(*obj);

    where.init_member("StageScaleMode", obj.get());
}


namespace {

void
attachStageScaleModeStaticInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("EXACT_FIT", "exactFit");
    o.init_member("NO_BORDER", "noBorder");
    o.init_member("NO_SCALE", "noScale");
    o.init_member("SHOW_ALL", "showAll");
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

