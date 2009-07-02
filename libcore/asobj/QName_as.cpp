// QName_as.cpp:  ActionScript 3 QName class, for Gnash.
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

#include "QName_as.h"
#include "as_object.h" 
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" 
#include "Object.h" 
#include "VM.h" 

#include <sstream>

namespace gnash {

namespace {
    as_value qname_ctor(const fn_call& fn);
    as_value qname_uri(const fn_call& fn);
    as_value qname_localName(const fn_call& fn);
    
    as_object* getQNameInterface();
    void attachQNameInterface(as_object& o);
}


class QName_as : public as_object
{

public:

    QName_as()
        :
        as_object(getQNameInterface())
    {}

};


// extern 
void
qname_class_init(as_object& where)
{
    boost::intrusive_ptr<builtin_function> cl;
    cl = new builtin_function(&qname_ctor, getQNameInterface());

    where.init_member("QName", cl.get());
}


namespace {

as_object*
getQNameInterface()
{
    static boost::intrusive_ptr<as_object> o;

    if (!o) {
        o = new as_object(getObjectInterface());
        VM::get().addStatic(o.get());
        attachQNameInterface(*o);
    }

    return o.get();
}


void
attachQNameInterface(as_object& o)
{
    // TODO: prop flags
    o.init_property("prefix", qname_localName, qname_localName);
    o.init_property("uri", qname_uri, qname_uri);
}

as_value
qname_localName(const fn_call& /*fn*/)
{
    log_unimpl("QName.localName");
    return as_value();
}

as_value
qname_uri(const fn_call& /*fn*/)
{
    log_unimpl("QName.uri");
    return as_value();
}

as_value
qname_ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> ns = new QName_as;
    return as_value(ns.get()); 
}

} // anonymous namespace
} // gnash namespace
