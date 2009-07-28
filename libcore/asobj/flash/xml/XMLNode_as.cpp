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
#include "xml/XMLDocument_as.h"
#include "Array_as.h"
#include "Object.h"
#include "VM.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "string_table.h"
#include "PropertyList.h"
#include "Global_as.h"

#include <boost/bind.hpp>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

namespace gnash {

// Function Prototypes
namespace {
    void enumerateAttributes(const XMLNode_as& node, PropertyList::SortedPropertyList&attributes);
    bool prefixMatches(const PropertyList::SortedPropertyList::value_type& val,
            const std::string& prefix);
    bool namespaceMatches(
            const PropertyList::SortedPropertyList::value_type& val,
            const std::string& ns);    
    as_value xmlnode_new(const fn_call& fn);
    as_value xmlnode_nodeName(const fn_call& fn);
    as_value xmlnode_nodeValue(const fn_call& fn);
    as_value xmlnode_nodeType(const fn_call& fn);
    as_value xmlnode_attributes(const fn_call& fn);
    as_value xmlnode_appendChild(const fn_call& fn);
    as_value xmlnode_cloneNode(const fn_call& fn);
    as_value xmlnode_lastChild(const fn_call& fn);
    as_value xmlnode_firstChild(const fn_call& fn);
    as_value xmlnode_nextSibling(const fn_call& fn);
    as_value xmlnode_childNodes(const fn_call& fn);
    as_value xmlnode_previousSibling(const fn_call& fn);
    as_value xmlnode_parentNode(const fn_call& fn);
    as_value xmlnode_getNamespaceForPrefix(const fn_call& fn);
    as_value xmlnode_getPrefixForNamespace(const fn_call& fn);
    as_value xmlnode_namespaceURI(const fn_call& fn);
    as_value xmlnode_hasChildNodes(const fn_call& fn);
    as_value xmlnode_insertBefore(const fn_call& fn);
    as_value xmlnode_removeNode(const fn_call& fn);
    as_value xmlnode_toString(const fn_call& fn);
    as_value xmlnode_localName(const fn_call& fn);
    as_value xmlnode_prefix(const fn_call& fn);
    as_value xmlnode_ctor(const fn_call& fn);
    void attachXMLNodeInterface(as_object& o);
    void attachXMLNodeStaticInterface(as_object& o);
    as_object* getXMLNodeInterface();
}

XMLNode_as::XMLNode_as()
	: as_object(getXMLNodeInterface()),
    	  _parent(0),
    	  _attributes(new as_object),
    	  _type(Element)
{
    //log_debug("%s: %p", __PRETTY_FUNCTION__, this);
#ifdef DEBUG_MEMORY_ALLOCATION
    log_debug(_("\tCreating XMLNode data at %p"), this);
#endif
}

XMLNode_as::XMLNode_as(const XMLNode_as& tpl, bool deep)
    :
    as_object(getXMLNodeInterface()),
    _parent(0), // _parent is never implicitly copied
    _attributes(0),
    _name(tpl._name),
    _value(tpl._value),
    _type(tpl._type)
{
    // only clone children if in deep mode
    if (deep) {
        const Children& from=tpl._children;
        for (Children::const_iterator it=from.begin(), itEnd=from.end();
                        it != itEnd; ++it) {

            _children.push_back(new XMLNode_as(*(*it), deep));
        }
    }
}

XMLNode_as::~XMLNode_as()
{
#ifdef DEBUG_MEMORY_ALLOCATION
    log_debug(_("\tDeleting XMLNode data %s with as_value %s at %p"),
            this->_name, this->as_value, this);
#endif
}
	
bool
XMLNode_as::hasChildNodes()
{
    if (_children.size()) return true;
    return false;
}

boost::intrusive_ptr<XMLNode_as>
XMLNode_as::firstChild()
{
    if ( _children.empty() ) return NULL;
    return _children.front();
}

boost::intrusive_ptr<XMLNode_as>
XMLNode_as::cloneNode(bool deep)
{
    boost::intrusive_ptr<XMLNode_as> newnode = new XMLNode_as(*this, deep);
    return newnode;
}

boost::intrusive_ptr<XMLNode_as>
XMLNode_as::lastChild()
{
	if (_children.empty()) {
			log_debug(_("XMLNode_as %p has no children"), (void*)this);
			return 0;
	}
	return _children.back();
}

void
XMLNode_as::appendChild(boost::intrusive_ptr<XMLNode_as> node)
{
    assert (node);

    boost::intrusive_ptr<XMLNode_as> oldparent = node->getParent();
    node->setParent(this);
    _children.push_back(node);
    if ( oldparent ) {
        oldparent->_children.remove(node);
    }

}

void
XMLNode_as::insertBefore(boost::intrusive_ptr<XMLNode_as> newnode,
        boost::intrusive_ptr<XMLNode_as> pos)
{
	// find iterator for positional parameter
    Children::iterator it = std::find(_children.begin(), _children.end(), pos);
    if (it == _children.end()) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("XMLNode.insertBefore(): positional parameter "
                "is not a child of this node"));
        );
        return;
    }

    _children.insert(it, newnode);
    boost::intrusive_ptr<XMLNode_as> oldparent = newnode->getParent();
    newnode->setParent(this);
    if (oldparent) {
        oldparent->_children.remove(newnode);
    }
}

// \brief removes the specified XML object from its parent. Also
// deletes all descendants of the node.
void
XMLNode_as::removeNode()
{
    boost::intrusive_ptr<XMLNode_as> oldparent = getParent();
    if (oldparent) {
        oldparent->_children.remove(this);
    }
    _parent = 0;
}

XMLNode_as *
XMLNode_as::previousSibling()
{
    if (!_parent) return 0;
 	if (_parent->_children.size() <= 1) return 0;

    XMLNode_as *previous_node = 0;
    for (Children::iterator itx = _parent->_children.begin();
            itx != _parent->_children.end(); itx++) {

        if (itx->get() == this) return previous_node;
		
        previous_node = itx->get();
    }

    return 0;
}

XMLNode_as *
XMLNode_as::nextSibling()
{

    if (!_parent) return 0;

    if (_parent->_children.size() <= 1) return 0;

    XMLNode_as *previous_node = 0;
    for (Children::reverse_iterator itx = _parent->_children.rbegin();
            itx != _parent->_children.rend(); ++itx) {

        if (itx->get() == this) return previous_node;
		previous_node = itx->get();
    }

    return 0;
}

void
XMLNode_as::toString(std::ostream& xmlout, bool encode) const
{
    stringify(*this, xmlout, encode);
}

void
XMLNode_as::setAttribute(const std::string& name, const std::string& value)
{
    if (_attributes) {
        string_table& st = getStringTable(*this);
        _attributes->set_member(st.find(name), value);
    }
}

bool
XMLNode_as::getPrefixForNamespace(const std::string& ns, std::string& prefix)
{
    XMLNode_as* node = this;
    PropertyList::SortedPropertyList::const_iterator it; 
    PropertyList::SortedPropertyList attrs;
    
    while (node) {
        enumerateAttributes(*node, attrs);
        if (!attrs.empty())
        {
            it = std::find_if(attrs.begin(), attrs.end(), 
                        boost::bind(namespaceMatches, _1, ns));
            if (it != attrs.end()) break;
        }
        node = node->getParent();
    }

    // None found.
    if (!node) return false;

    // Return the matching prefix
    const std::string& name = it->first;

    if (name.length() == 5) {
        return true;
    }

    assert (name.length() >= 6);
    
    if (name[5] != ':') return false;

    // Can also be empty.
    prefix = name.substr(6);
    return true;    
}

void
XMLNode_as::getNamespaceForPrefix(const std::string& prefix, std::string& ns)
{
    XMLNode_as* node = this;
    PropertyList::SortedPropertyList::const_iterator it; 
    PropertyList::SortedPropertyList attrs;
    
    while (node) {

        enumerateAttributes(*node, attrs);

        if (!attrs.empty()) {

            it = std::find_if(attrs.begin(), attrs.end(), 
                        boost::bind(prefixMatches, _1, prefix));
            if (it != attrs.end()) break;
        }
        node = node->getParent();
    }

    // None found; return undefined
    if (!node) return;

    // Return the matching namespace
    ns = it->second;

}

bool
XMLNode_as::extractPrefix(std::string& prefix)
{
    prefix.clear();
    if (_name.empty()) return false;

    std::string::size_type pos = _name.find(':');
    if (pos == std::string::npos || pos == _name.size() - 1) {
        return false;
    }

    prefix = _name.substr(0, pos);
    return true;
}

void
XMLNode_as::stringify(const XMLNode_as& xml, std::ostream& xmlout, bool encode) 
{

    const std::string& nodeValue = xml.nodeValue();
    const std::string& nodeName = xml.nodeName();
    NodeType type = xml.nodeType();

#ifdef GNASH_DEBUG
    log_debug(_("Stringifying node %p with name %s, as_value %s, %u "
                "attributes and %u children"), (void*)&xml, nodeName,
            nodeValue, xml._attributes.size(), xml._children.size());
#endif

    // Create the beginning of the tag
    if (!nodeName.empty()) {

        xmlout << "<" << nodeName;
    
        // Process the attributes, if any
        PropertyList::SortedPropertyList attrs;
        enumerateAttributes(xml, attrs);
        if (!attrs.empty()) {

            for (PropertyList::SortedPropertyList::iterator i = 
                    attrs.begin(), e = attrs.end(); i != e; ++i) { 
                XMLDocument_as::escape(i->second);
                xmlout << " " << i->first << "=\"" << i->second << "\"";
            }
        }

    	// If the node has no content, just close the tag now
    	if ( nodeValue.empty() && xml._children.empty() ) {
    		xmlout << " />";
            return;
    	}
        else {
            // Will use a closing tag later
            xmlout << ">";
        }

    }

    // Node as_value first, then children
    if (type == Text)
    {
        as_object* global = getVM(xml).getGlobal();
        assert(global);

        // Insert entities.
        std::string escaped(nodeValue);
        XMLDocument_as::escape(escaped);
        const std::string& val = encode ? 
            global->callMethod(NSV::PROP_ESCAPE, escaped).to_string() :
            escaped;

	    xmlout << val;
    }

    // Childs, after node as_value.
    for (Children::const_iterator itx = xml._children.begin(); 
            itx != xml._children.end(); ++itx) {

        (*itx)->toString(xmlout, encode);
    }

    if (!nodeName.empty()) {
	    xmlout << "</" << nodeName << ">";
    }
}

#ifdef GNASH_USE_GC
void
XMLNode_as::markReachableResources() const
{
	// Mark children
    std::for_each(_children.begin(), _children.end(),
            boost::mem_fn(&as_object::setReachable));

	// Mark parent
	if (_parent) _parent->setReachable();

	// Mark attributes object
	if (_attributes) _attributes->setReachable();

	markAsObjectReachable();
}
#endif // GNASH_USE_GC

void
XMLNode_as::registerNative(as_object& where)
{
    VM& vm = getVM(where);
    vm.registerNative(xmlnode_cloneNode, 253, 1);
    vm.registerNative(xmlnode_removeNode, 253, 2);
    vm.registerNative(xmlnode_insertBefore, 253, 3);
    vm.registerNative(xmlnode_appendChild, 253, 4);
    vm.registerNative(xmlnode_hasChildNodes, 253, 5);
    vm.registerNative(xmlnode_toString, 253, 6);
}

as_object*
XMLNode_as::getXMLNodeInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( o == NULL ) {
        o = new as_object(getObjectInterface());
        attachXMLNodeInterface(*o);
    }
    return o.get();
}

void
XMLNode_as::init(as_object& where, const ObjectURI& uri)
{
    // This is the global XMLNode_as "class"
    static boost::intrusive_ptr<as_object> cl;

    if ( cl == NULL )
    {
        Global_as* gl = getGlobal(where);
        cl = gl->createClass(&xmlnode_new, getXMLNodeInterface());
    }

    where.init_member(getName(uri), cl.get(), as_object::DefaultFlags,
            getNamespace(uri));

}

namespace {

void
attachXMLNodeInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    // These need to be full-featured AS functions (builtin_function)
    
    VM& vm = getVM(o);

    const int noFlags = 0;
    
    // No prop flags:
    o.init_member("cloneNode", vm.getNative(253, 1), noFlags);
    o.init_member("removeNode", vm.getNative(253, 2), noFlags);
    o.init_member("insertBefore", vm.getNative(253, 3), noFlags);
    o.init_member("appendChild", vm.getNative(253, 4), noFlags);
    o.init_member("hasChildNodes", vm.getNative(253, 5), noFlags);
    o.init_member("toString", vm.getNative(253, 6), noFlags);
    o.init_member("getPrefixForNamespace", gl->createFunction(
                xmlnode_getPrefixForNamespace), noFlags);
    o.init_member("getNamespaceForPrefix", gl->createFunction(
                xmlnode_getNamespaceForPrefix), noFlags);


    const int protectedFlags = PropFlags::isProtected;

    // Just the protected flag:
    o.init_property("nodeValue", &xmlnode_nodeValue, 
            &xmlnode_nodeValue, protectedFlags);
    o.init_property("nodeName", &xmlnode_nodeName, 
            &xmlnode_nodeName, protectedFlags);

    o.init_readonly_property("firstChild", &xmlnode_firstChild, protectedFlags);
    o.init_readonly_property("lastChild", &xmlnode_lastChild, protectedFlags);
    o.init_readonly_property("localName", &xmlnode_localName, protectedFlags);
    o.init_readonly_property("namespaceURI", 
            &xmlnode_namespaceURI, protectedFlags);
    o.init_readonly_property("nextSibling", 
            &xmlnode_nextSibling, protectedFlags);
    o.init_readonly_property("prefix", &xmlnode_prefix, protectedFlags);
    o.init_readonly_property("previousSibling", 
            &xmlnode_previousSibling, protectedFlags);
    o.init_readonly_property("nodeType", &xmlnode_nodeType, protectedFlags);
    o.init_readonly_property("attributes", &xmlnode_attributes, protectedFlags);
    o.init_readonly_property("childNodes", &xmlnode_childNodes, protectedFlags);
    o.init_readonly_property("parentNode", &xmlnode_parentNode, protectedFlags);

}


as_value
xmlnode_new(const fn_call& fn)
{
    
    XMLNode_as *xml_obj = new XMLNode_as;
    if ( fn.nargs > 0 )
    {
        xml_obj->nodeTypeSet(XMLNode_as::NodeType(fn.arg(0).to_int()));
        if (fn.nargs > 1)
        {
            xml_obj->nodeValueSet(fn.arg(1).to_string());
        }
    }
    
    return as_value(xml_obj);
}


as_value
xmlnode_appendChild(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;

	boost::intrusive_ptr<XMLNode_as> ptr = ensureType<XMLNode_as>(fn.this_ptr);

	if ( ! fn.nargs )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("XMLNode::appendChild() needs at least one argument"));
		);
		return as_value();
	}

	boost::intrusive_ptr<XMLNode_as> xml_obj = 
        boost::dynamic_pointer_cast<XMLNode_as>(fn.arg(0).to_object(*getGlobal(fn)));	
	if ( ! xml_obj )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("First argument to XMLNode::appendChild() is not "
                "an XMLNode"));
		);
		return as_value();
	}

	ptr->appendChild(xml_obj);
	return as_value(); // undefined

}

as_value
xmlnode_cloneNode(const fn_call& fn)
{
    boost::intrusive_ptr<XMLNode_as> ptr = ensureType<XMLNode_as>(fn.this_ptr);

    bool deep = false;
    if (fn.nargs > 0) deep = fn.arg(0).to_bool();

    boost::intrusive_ptr<XMLNode_as> newnode = ptr->cloneNode(deep);
    return as_value(newnode.get());
}


as_value
xmlnode_insertBefore(const fn_call& fn)
{
	boost::intrusive_ptr<XMLNode_as> ptr = ensureType<XMLNode_as>(fn.this_ptr);

	if ( fn.nargs < 2 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_aserror(_("XMLNode.insertBefore(%s) needs at least two "
                "arguments"), ss.str());
		);
		return as_value();
	}

	boost::intrusive_ptr<XMLNode_as> newnode = 
        boost::dynamic_pointer_cast<XMLNode_as>(fn.arg(0).to_object(*getGlobal(fn)));

	if (!newnode) {
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_aserror(_("First argument to XMLNode.insertBefore(%s) is not "
                "an XMLNode"), ss.str());
		);
		return as_value();
	}

	boost::intrusive_ptr<XMLNode_as> pos = 
        boost::dynamic_pointer_cast<XMLNode_as>(fn.arg(1).to_object(*getGlobal(fn)));

	if (!pos) {
		IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
		log_aserror(_("Second argument to XMLNode.insertBefore(%s) is not "
                "an XMLNode"), ss.str());
		);
		return as_value();
	}

    ptr->insertBefore(newnode, pos);
    return as_value();
    
}


as_value
xmlnode_getNamespaceForPrefix(const fn_call& fn)
{
    boost::intrusive_ptr<XMLNode_as> ptr = ensureType<XMLNode_as>(fn.this_ptr);
    if (!fn.nargs) {
        return as_value();
    }

    std::string ns;

    ptr->getNamespaceForPrefix(fn.arg(0).to_string(), ns);
    if (ns.empty()) return as_value();
    return as_value(ns);
}


as_value
xmlnode_getPrefixForNamespace(const fn_call& fn)
{
    boost::intrusive_ptr<XMLNode_as> ptr = ensureType<XMLNode_as>(fn.this_ptr);
    if (!fn.nargs) {
        return as_value();
    }

    std::string prefix;

    // Return undefined if none found; otherwise the prefix string found.
    // This can be empty if it is a standard namespace.
    if (!ptr->getPrefixForNamespace(fn.arg(0).to_string(), prefix)) {
        return as_value();
    }
    return as_value(prefix);
}

/// If the node has a prefix, return the matching namespace. Otherwise, 
/// returns a namespaceURI set with the xmlns attribute, searching upwards
/// through parent nodes if necessary.
//
/// This standard namespace can only  be set during XML parsing and cannot
/// be changed or set using attributes.
//
/// Conversely, the similar getNamespaceForPrefix("") can be set and changed
/// through attributes.
as_value
xmlnode_namespaceURI(const fn_call& fn)
{
    boost::intrusive_ptr<XMLNode_as> ptr = ensureType<XMLNode_as>(fn.this_ptr);

    // Read-only property
    
    const std::string& name = ptr->nodeName();

    if (name.empty()) {
        as_value null;
        null.set_null();
        return null;
    }

    std::string prefix;
    if (ptr->extractPrefix(prefix)) {
        std::string ns;
        ptr->getNamespaceForPrefix(prefix, ns);
        return as_value(ns);
    }

    // Search recursively for a namespace. Return an empty string
    // if none found.
    XMLNode_as* node = ptr.get();
    while (node && node->getNamespaceURI().empty()) {
        node = node->getParent();
    }
    if (!node) return as_value("");

    return as_value(node->getNamespaceURI());
}


// Return the prefix part of the node name. If there is no colon, or one
// colon at the end of the string, this is empty. Otherwise it is the part
// up to the first colon.
as_value
xmlnode_prefix(const fn_call& fn)
{
    boost::intrusive_ptr<XMLNode_as> ptr = ensureType<XMLNode_as>(fn.this_ptr);

    // Read-only property
    
    if (ptr->nodeName().empty()) {
        as_value null;
        null.set_null();
        return null;
    }
    
    std::string prefix;
    if (!ptr->extractPrefix(prefix)) return as_value("");
    return as_value(prefix);
}


// The local part of a node name. If there is no colon or a single colon
// at the end of the string, this is the whole string. Otherwise all of the
// string after the first colon.
as_value
xmlnode_localName(const fn_call& fn)
{
    boost::intrusive_ptr<XMLNode_as> ptr = ensureType<XMLNode_as>(fn.this_ptr);

    // Read-only property
    
    if (ptr->nodeName().empty()) {
        as_value null;
        null.set_null();
        return null;
    }

    const std::string& nodeName = ptr->nodeName();
    if (nodeName.empty()) return as_value("");

    std::string::size_type pos = nodeName.find(':');
    if (pos == std::string::npos || pos == nodeName.size() - 1) {
        return as_value(nodeName);
    }

    return as_value(nodeName.substr(pos + 1));
}


as_value
xmlnode_removeNode(const fn_call& fn)
{
    boost::intrusive_ptr<XMLNode_as> ptr = ensureType<XMLNode_as>(fn.this_ptr);
    
    ptr->removeNode();
    return as_value();
}


// TODO: shouldn't overriding get_text_value() be fine ?
as_value
xmlnode_toString(const fn_call& fn)
{
    //GNASH_REPORT_FUNCTION;
    
    boost::intrusive_ptr<XMLNode_as> ptr = ensureType<XMLNode_as>(fn.this_ptr);
    
    std::stringstream ss;
    ptr->toString(ss);

    return as_value(ss.str());
}


as_value
xmlnode_hasChildNodes(const fn_call& fn)
{
    boost::intrusive_ptr<XMLNode_as> ptr = ensureType<XMLNode_as>(fn.this_ptr);
    return as_value(ptr->hasChildNodes());
}


as_value
xmlnode_nodeValue(const fn_call& fn)
{
    boost::intrusive_ptr<XMLNode_as> ptr = ensureType<XMLNode_as>(fn.this_ptr);
    as_value rv;
    rv.set_null();
    
    if ( fn.nargs == 0 )
    {
        const std::string& val = ptr->nodeValue();
        if ( ! val.empty() ) rv = val;
    }
    else
    {
        ptr->nodeValueSet(fn.arg(0).to_string());
    }
    return rv;
}


as_value
xmlnode_nodeName(const fn_call& fn)
{
    boost::intrusive_ptr<XMLNode_as> ptr = ensureType<XMLNode_as>(fn.this_ptr);
    as_value rv;
    rv.set_null();

    if (!fn.nargs) {
        const std::string& val = ptr->nodeName();
        if ( ! val.empty() ) rv = val;
    }
    else
    {
        ptr->nodeNameSet(fn.arg(0).to_string());
    }
    return rv;
}


as_value
xmlnode_nodeType(const fn_call& fn)
{
    boost::intrusive_ptr<XMLNode_as> ptr = ensureType<XMLNode_as>(fn.this_ptr);
    return as_value(ptr->nodeType());
}


as_value
xmlnode_attributes(const fn_call& fn)
{
    boost::intrusive_ptr<XMLNode_as> ptr = ensureType<XMLNode_as>(fn.this_ptr);

    as_object* attrs = ptr->getAttributes();
    if (attrs) return as_value(attrs);
    return as_value(); 
}


/// Read-only property; evaluates the specified XML object and
/// references the first child in the parent node's child
/// list. This property is null if the node does not have
/// children. This property is undefined if the node is a text
/// node. This is a read-only property and cannot be used to
/// manipulate child nodes; use the appendChild(), insertBefore(),
/// and removeNode() methods to manipulate child nodes. 
///
as_value
xmlnode_firstChild(const fn_call& fn)
{
    boost::intrusive_ptr<XMLNode_as> ptr = ensureType<XMLNode_as>(fn.this_ptr);
    as_value rv;
    rv.set_null();

    boost::intrusive_ptr<XMLNode_as> node = ptr->firstChild();
    if (node) {
       rv = node.get();
    }

    return rv;
}


/// Read-only property; an XMLNode as_value that references the last
/// child in the node's child list. The XML.lastChild property
/// is null if the node does not have children. This property cannot
/// be used to manipulate child nodes; use the appendChild(),
/// insertBefore(), and removeNode() methods to manipulate child
/// nodes.
as_value
xmlnode_lastChild(const fn_call& fn)
{
    boost::intrusive_ptr<XMLNode_as> ptr = ensureType<XMLNode_as>(fn.this_ptr);
    as_value rv;
    rv.set_null();

    boost::intrusive_ptr<XMLNode_as> node = ptr->lastChild();
    if (node) rv = node.get();

    return rv;
}


as_value
xmlnode_nextSibling(const fn_call& fn)
{
    as_value rv;
    rv.set_null();

    boost::intrusive_ptr<XMLNode_as> ptr = ensureType<XMLNode_as>(fn.this_ptr);
    XMLNode_as *node = ptr->nextSibling();
    if (node) {
	rv = node;
    }
    return rv;
}


as_value
xmlnode_previousSibling(const fn_call& fn)
{
    as_value rv;
    rv.set_null();

    boost::intrusive_ptr<XMLNode_as> ptr = ensureType<XMLNode_as>(fn.this_ptr);
    XMLNode_as *node = ptr->previousSibling();
    if (node) {
	rv = node;
    }
    return rv;
}


as_value
xmlnode_parentNode(const fn_call& fn)
{
    as_value rv;
    rv.set_null();

    boost::intrusive_ptr<XMLNode_as> ptr = ensureType<XMLNode_as>(fn.this_ptr);
    XMLNode_as *node = ptr->getParent();
    if (node) {
	rv = node;
    }
    return rv;
}


as_value
xmlnode_childNodes(const fn_call& fn)
{
    boost::intrusive_ptr<XMLNode_as> ptr = ensureType<XMLNode_as>(fn.this_ptr);
    boost::intrusive_ptr<Array_as> ary = new Array_as();

    typedef XMLNode_as::Children Children;

    Children& child = ptr->childNodes();
    for ( Children::const_iterator it=child.begin(), itEnd=child.end();
                    it != itEnd; ++it )
    {
        boost::intrusive_ptr<XMLNode_as> node = *it;
        ary->push(as_value(node.get()));
    }

    return as_value(ary.get());
}


void
enumerateAttributes(const XMLNode_as& node,
            PropertyList::SortedPropertyList& attrs)
{
    attrs.clear();
    const as_object* obj = node.getAttributes();
    if (obj) {
        obj->enumerateProperties(attrs);
    }

}

/// Return true if this attribute is a namespace specifier and the
/// namespace matches.
bool
namespaceMatches(const PropertyList::SortedPropertyList::value_type& val,
        const std::string& ns)
{
    StringNoCaseEqual noCaseCompare;
    return (noCaseCompare(val.first.substr(0, 5), "xmlns") && 
                noCaseCompare(val.second, ns));
}


bool
prefixMatches(const PropertyList::SortedPropertyList::value_type& val,
        const std::string& prefix)
{
    const std::string& name = val.first;
    StringNoCaseEqual noCaseCompare;

    // An empty prefix searches for a standard namespace specifier.
    // Attributes are stored with no trailing or leading whitespace,
    // so a simple comparison should do. TODO: what about "xmlns:"?
    if (prefix.empty()) {
        return noCaseCompare(name, "xmlns") || noCaseCompare(name, "xmlns:");
    }

    if (!noCaseCompare(name.substr(0, 6), "xmlns:")) return false;

    return noCaseCompare(prefix, name.substr(6));
}

as_value
xmlnode_ctor(const fn_call& /*fn*/)
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

