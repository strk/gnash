// XMLNode_as.cpp:  ActionScript "XMLNode" class, for Gnash.
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

#include "xml/XMLNode_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value xmlnode_cloneNode(const fn_call& fn);
    as_value xmlnode_getNamespaceForPrefix(const fn_call& fn);
    as_value xmlnode_getPrefixForNamespace(const fn_call& fn);
    as_value xmlnode_hasChildNodes(const fn_call& fn);
    as_value xmlnode_insertBefore(const fn_call& fn);
    as_value xmlnode_removeNode(const fn_call& fn);
    as_value xmlnode_toString(const fn_call& fn);
    as_value xmlnode_ctor(const fn_call& fn);
    void attachXMLNodeInterface(as_object& o);
    void attachXMLNodeStaticInterface(as_object& o);
    as_object* getXMLNodeInterface();

}

class XMLNode_as : public as_object
{

public:

    XMLNode_as()
        :
        as_object(getXMLNodeInterface())
    {}
};

// extern (used by Global.cpp)
void xmlnode_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&xmlnode_ctor, getXMLNodeInterface());
        attachXMLNodeStaticInterface(*cl);
    }

    // Register _global.XMLNode
    global.init_member("XMLNode", cl.get());
}

namespace {

void
attachXMLNodeInterface(as_object& o)
{
    o.init_member("cloneNode", new builtin_function(xmlnode_cloneNode));
    o.init_member("getNamespaceForPrefix", new builtin_function(xmlnode_getNamespaceForPrefix));
    o.init_member("getPrefixForNamespace", new builtin_function(xmlnode_getPrefixForNamespace));
    o.init_member("hasChildNodes", new builtin_function(xmlnode_hasChildNodes));
    o.init_member("insertBefore", new builtin_function(xmlnode_insertBefore));
    o.init_member("removeNode", new builtin_function(xmlnode_removeNode));
    o.init_member("toString", new builtin_function(xmlnode_toString));
}

void
attachXMLNodeStaticInterface(as_object& o)
{

}

as_object*
getXMLNodeInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachXMLNodeInterface(*o);
    }
    return o.get();
}

as_value
xmlnode_cloneNode(const fn_call& fn)
{
    boost::intrusive_ptr<XMLNode_as> ptr =
        ensureType<XMLNode_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
xmlnode_getNamespaceForPrefix(const fn_call& fn)
{
    boost::intrusive_ptr<XMLNode_as> ptr =
        ensureType<XMLNode_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
xmlnode_getPrefixForNamespace(const fn_call& fn)
{
    boost::intrusive_ptr<XMLNode_as> ptr =
        ensureType<XMLNode_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
xmlnode_hasChildNodes(const fn_call& fn)
{
    boost::intrusive_ptr<XMLNode_as> ptr =
        ensureType<XMLNode_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
xmlnode_insertBefore(const fn_call& fn)
{
    boost::intrusive_ptr<XMLNode_as> ptr =
        ensureType<XMLNode_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
xmlnode_removeNode(const fn_call& fn)
{
    boost::intrusive_ptr<XMLNode_as> ptr =
        ensureType<XMLNode_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
xmlnode_toString(const fn_call& fn)
{
    boost::intrusive_ptr<XMLNode_as> ptr =
        ensureType<XMLNode_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
xmlnode_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new XMLNode_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

