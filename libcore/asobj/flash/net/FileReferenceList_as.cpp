// filereferencelist_as.cpp:  ActionScript "FileReferenceList" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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


#include "FileReferenceList_as.h"

#include "as_function.h" 
#include "as_object.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "GnashException.h" // for ActionException
#include "VM.h"

#include <sstream>

namespace gnash {

static as_value filereferencelist_addListener(const fn_call& fn);
static as_value filereferencelist_browse(const fn_call& fn);
static as_value filereferencelist_removeListener(const fn_call& fn);
static as_value filereferencelist_fileList_getset(const fn_call& fn);
as_value filereferencelist_ctor(const fn_call& fn);

static void
attachFileReferenceListStaticInterface(as_object& /*o*/)
{

}

static void
attachFileReferenceListInterface(as_object& o)
{
    Global_as& gl = getGlobal(o);
    o.init_member("addListener", gl.createFunction(filereferencelist_addListener));
    o.init_member("browse", gl.createFunction(filereferencelist_browse));
    o.init_member("removeListener", gl.createFunction(filereferencelist_removeListener));
    o.init_property("fileList", filereferencelist_fileList_getset, filereferencelist_fileList_getset);
}


static as_value
filereferencelist_addListener(const fn_call& /*fn*/)
{
    return as_value();
}

static as_value
filereferencelist_browse(const fn_call& /*fn*/)
{
    return as_value();
}

static as_value
filereferencelist_removeListener(const fn_call& /*fn*/)
{
    return as_value();
}

static as_value
filereferencelist_fileList_getset(const fn_call& /*fn*/)
{
    return as_value();
}



as_value
filereferencelist_ctor(const fn_call& fn)
{
    if (fn.nargs) {
        std::stringstream ss;
        fn.dump_args(ss);
        LOG_ONCE(log_unimpl(_("FileReferenceList(%s): %s"), ss.str(),
                              _("arguments discarded")));
    }

    return as_value(); 
}

// extern 
void
filereferencelist_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinClass(where, filereferencelist_ctor,
            attachFileReferenceListInterface, 
            attachFileReferenceListStaticInterface, uri);
}

} // end of gnash namespace
