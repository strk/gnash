// Namespace_as.cpp:  ActionScript 3 Namespace class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "Namespace_as.h"
#include "as_object.h" 
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" 
#include "GnashException.h"
#include "VM.h" 

#include <sstream>

namespace gnash {

namespace {
    as_value namespace_ctor(const fn_call& fn);
    as_value namespace_uri(const fn_call& fn);
    as_value namespace_prefix(const fn_call& fn);
    
    as_object* getNamespaceInterface();
    void attachNamespaceInterface(as_object& o);
}

// extern 
void
namespace_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinClass(where, namespace_ctor, attachNamespaceInterface,
            0, uri);
}


namespace {

void
attachNamespaceInterface(as_object& o)
{
    // TODO: prop flags
    o.init_property("prefix", namespace_prefix, namespace_prefix);
    o.init_property("uri", namespace_uri, namespace_uri);
}

as_value
namespace_prefix(const fn_call& /*fn*/)
{
    log_unimpl("Namespace.prefix");
    return as_value();
}

as_value
namespace_uri(const fn_call& /*fn*/)
{
    log_unimpl("Namespace.uri");
    return as_value();
}

as_value
namespace_ctor(const fn_call& /*fn*/)
{
    // TODO: probably needs Relay.
    log_unimpl("Namespace");
    return as_value(); 
}

} // anonymous namespace
} // gnash namespace
