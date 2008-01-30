// xmlnode.cpp:  ActionScript "XMLNode" class, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

/* $Id: xmlnode.cpp,v 1.45 2008/01/30 10:09:36 bwy Exp $ */

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "log.h"
#include "tu_config.h"
#include "fn_call.h"
#include "builtin_function.h"
#include "array.h" // for childNodes
#include "xmlnode.h"
#include "log.h"
#include "Object.h" // for getObjectInterface
#include "VM.h" // for getting the string_table..
#include "string_table.h" // ..for using the string_table

#include <boost/algorithm/string/case_conv.hpp>

#include <string>
#include <sstream>
#include <vector>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>

//#define DEBUG_MEMORY_ALLOCATION 1

namespace gnash {

static as_value xmlnode_new(const fn_call& fn);
static as_value xmlnode_nodename(const fn_call& fn);
static as_value xmlnode_nodevalue(const fn_call& fn);
static as_value xmlnode_nodetype(const fn_call& fn);
static as_value xmlnode_attributes(const fn_call& fn);
static as_value xmlnode_appendchild(const fn_call& fn);
static as_value xmlnode_clonenode(const fn_call& fn);
static as_value xmlnode_haschildnodes(const fn_call& fn);
static as_value xmlnode_insertbefore(const fn_call& fn);
static as_value xmlnode_removenode(const fn_call& fn);
static as_value xmlnode_tostring(const fn_call& fn);
static as_value xmlnode_firstchild(const fn_call& fn);
static as_value xmlnode_lastchild(const fn_call& fn);
static as_value xmlnode_nextsibling(const fn_call& fn);
static as_value xmlnode_previoussibling(const fn_call& fn);
static as_value xmlnode_childNodes(const fn_call& fn);
static as_value xmlnode_parentNode(const fn_call& fn);
as_object* getXMLNodeInterface();

//static LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();


//std::vector<as_object *> _xmlobjs;    // FIXME: hack alert

XMLNode::XMLNode()
    :
    as_object(getXMLNodeInterface()),
    _parent(0),
    _type(tElement)
{
    //log_msg("%s: %p", __PRETTY_FUNCTION__, this);
#ifdef DEBUG_MEMORY_ALLOCATION
    log_msg(_("\tCreating XMLNode data at %p"), this);
#endif
}

XMLNode::XMLNode(as_object* overridden_interface)
    :
    as_object(overridden_interface),
    _parent(0),
    _type(tElement)
{
    //log_msg("%s: %p", __PRETTY_FUNCTION__, this);
#ifdef DEBUG_MEMORY_ALLOCATION
    log_msg(_("\tCreating XMLNode data at %p"), this);
#endif
}

XMLNode::XMLNode(const XMLNode& tpl, bool deep)
    :
    as_object(getXMLNodeInterface()),
    _parent(0), // _parent is never implicitly copied
    _name(tpl._name),
    _value(tpl._value),
    _type(tpl._type)
{
    // only clone children if in deep mode
    if ( deep ) 
    {
        const ChildList& from=tpl._children;
        for (ChildList::const_iterator it=from.begin(), itEnd=from.end();
                        it != itEnd; ++it)
        {
                _children.push_back(new XMLNode(*(*it), deep));
        }
    }
}

XMLNode::~XMLNode()
{
    //log_msg("%s: %p", __PRETTY_FUNCTION__, this);
#ifdef DEBUG_MEMORY_ALLOCATION
    log_msg(_("\tDeleting XMLNode data %s with value %s at %p"), this->_name.c_str(), this->_value.c_str(), this);
#endif
}

bool
XMLNode::hasChildNodes()
{
    //GNASH_REPORT_FUNCTION;
    if (_children.size()) {
        return true;
    }
    return false;
}

boost::intrusive_ptr<XMLNode>
XMLNode::firstChild()
{
    //GNASH_REPORT_FUNCTION;
    if ( _children.empty() ) return NULL;
    return _children.front();
}

boost::intrusive_ptr<XMLNode>
XMLNode::lastChild()
{
	//GNASH_REPORT_FUNCTION;
	if ( _children.empty() )
	{
			log_msg(_("XMLNode %p has no children"), (void*)this);
			return NULL;
	}
	return _children.back();
}


void
XMLNode::appendChild(boost::intrusive_ptr<XMLNode> node)
{
    assert (node);

    boost::intrusive_ptr<XMLNode> oldparent = node->getParent();
    node->setParent(this);
    _children.push_back(node);
    if ( oldparent ) {
        oldparent->_children.remove(node);
    }

//  log_unimpl("%s: partially unimplemented", __PRETTY_FUNCTION__);
}

boost::intrusive_ptr<XMLNode> 
XMLNode::cloneNode(bool deep)
{
    //GNASH_REPORT_FUNCTION;
    //log_msg(_("%s: deep is %d"), __PRETTY_FUNCTION__, deep);

    boost::intrusive_ptr<XMLNode> newnode = new XMLNode(*this, deep);

    return newnode;
}

void
XMLNode::insertBefore(boost::intrusive_ptr<XMLNode> newnode, boost::intrusive_ptr<XMLNode> pos)
{
    // find iterator for positional parameter
    ChildList::iterator it = find(_children.begin(), _children.end(), pos);
    if ( it == _children.end() )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("XMLNode.insertBefore(): positional parameter is not a child of this node"));
        );
        return;
    }

    _children.insert(it, newnode);
    boost::intrusive_ptr<XMLNode> oldparent = newnode->getParent();
    newnode->setParent(this);
    if ( oldparent )
    {
        oldparent->_children.remove(newnode);
    }
}

/// \brief removes the specified XML object from its parent. Also
/// deletes all descendants of the node.
void
XMLNode::removeNode()
{
#ifndef GNASH_USE_GC
    assert(get_ref_count() > 1);
#endif
    boost::intrusive_ptr<XMLNode> oldparent = getParent();
    if ( oldparent )
    {
        oldparent->_children.remove(this);
    }
    _parent = NULL;
#ifndef GNASH_USE_GC
    assert(get_ref_count() > 0);
#endif
}

XMLNode *
XMLNode::previousSibling()
{
    //GNASH_REPORT_FUNCTION;

    if ( ! _parent) return NULL;
 	if (_parent->_children.size() <= 1) return NULL;

    XMLNode *previous_node = NULL;
    ChildList::iterator itx;
    for (itx = _parent->_children.begin(); itx != _parent->_children.end(); itx++)
    {
        if (itx->get() == this)
        {
            // log_msg("Found the previous XMLNode child !!!! %s <%p>", (*itx)->nodeName(), (void*)*itx);
		    return previous_node;
		}
		previous_node = itx->get();
    }

    return NULL;
}

XMLNode *
XMLNode::nextSibling()
{
    //GNASH_REPORT_FUNCTION;

    if ( ! _parent)
    {
            //log_msg("Node %p has no parent, returning NULL", this);
            return NULL;
    }
    if (_parent->_children.size() <= 1)
    {
            //log_msg("Node %p parent has only this node, returning NULL", this);
            return NULL;
    }

    XMLNode *previous_node = NULL;
    ChildList::reverse_iterator itx;
    for (itx = _parent->_children.rbegin(); itx != _parent->_children.rend(); itx++)
    {
        if (itx->get() == this)
        {
            //log_msg("Found the next XMLNode child !!!! %s <%p>", (*itx)->nodeName().c_str(), (void*)itx->get());
		    return previous_node;
		}
		previous_node = itx->get();
    }

    return NULL;
}

void
XMLNode::toString(std::ostream& xmlout) const
{
//    GNASH_REPORT_FUNCTION;
    stringify(*this, xmlout);
}

/* static private */
void
XMLNode::stringify(const XMLNode& xml, std::ostream& xmlout) 
{
//    GNASH_REPORT_FUNCTION;
    const std::string& nodevalue = xml.nodeValue();
    const std::string& nodename = xml.nodeName();
    NodeType type = xml.nodeType();


//    log_msg("%s: processing for object %s <%p>", __PRETTY_FUNCTION__, nodename, xml);

#ifdef GNASH_DEBUG
    log_msg(_("Stringifying node %p with name %s, value %s, %u attributes and %u children"),
                    (void*)&xml, nodename, nodevalue, xml._attributes.size(), xml._children.size());
#endif

    // Create the beginning of the tag
    if ( nodename.size() )
    {
        xmlout << "<" << nodename;
    
        // Process the attributes, if any
        AttribList::const_iterator ita;
        for (ita = xml._attributes.begin(); ita != xml._attributes.end(); ita++)
        {
            const XMLAttr& xa = *ita;
            // TODO: replace with XMLAttr::operator<<
            xmlout << " " << xa.name() << "=\"" << xa.value() << "\"";
        }

    	// If the node has no content, just close the tag now
    	if ( nodevalue.empty() && xml._children.empty() )
    	{
    		xmlout << " />";
		return;
    	}
	else
	{
    		// Will use a closing tag later
    		xmlout << ">";
	}

    }

    // Node value first, then children
    if ( type == tText )
    {
	    xmlout << nodevalue;
    }

    // Childs, after node value.
    ChildList::const_iterator itx;
    for (itx = xml._children.begin(); itx != xml._children.end(); itx++)
    {
//      log_msg(_("Found One XMLNode child.  %s <%p>"), (*itx)->nodeName().c_str(), (void*)*itx);
//      cerr << "<" << (*it)->nodeName() << ">" << endl;
        (*itx)->toString(xmlout);
    }

    if ( nodename.size() )
    {
	    xmlout << "</" << nodename << ">";
    }
}

void
attachXMLNodeInterface(as_object& o)
{
    o.init_member("appendChild", new builtin_function(xmlnode_appendchild));
    o.init_member("cloneNode", new builtin_function(xmlnode_clonenode));
    o.init_member("hasChildNodes", new builtin_function(xmlnode_haschildnodes));
    o.init_member("insertBefore", new builtin_function(xmlnode_insertbefore));
    o.init_member("removeNode", new builtin_function(xmlnode_removenode));
    o.init_member("toString", new builtin_function(xmlnode_tostring));

    // Properties - FIXME: use addProperty !

    boost::intrusive_ptr<builtin_function> gettersetter;

    gettersetter = new builtin_function(&xmlnode_nodevalue, NULL);
    o.init_property("nodeValue", *gettersetter, *gettersetter);

    gettersetter = new builtin_function(&xmlnode_nodename, NULL);
    o.init_property("nodeName", *gettersetter, *gettersetter);

    gettersetter = new builtin_function(&xmlnode_nodetype, NULL);
    o.init_readonly_property("nodeType", *gettersetter);

    gettersetter = new builtin_function(&xmlnode_attributes, NULL);
    o.init_readonly_property("attributes", *gettersetter);

    // These two return an array of objects
    gettersetter = new builtin_function(xmlnode_childNodes, NULL);
    o.init_readonly_property("childNodes", *gettersetter);

    /// \fn MLNode::firstChild
    /// \brief XMLNode::firstChild property
    ///
    /// Read-only property; evaluates the specified XML object and
    /// references the first child in the parent node's child
    /// list. This property is null if the node does not have
    /// children. This property is undefined if the node is a text
    /// node. This is a read-only property and cannot be used to
    /// manipulate child nodes; use the appendChild(), insertBefore(),
    /// and removeNode() methods to manipulate child nodes. 
    gettersetter = new builtin_function(&xmlnode_firstchild, NULL);
    o.init_readonly_property("firstChild", *gettersetter);

    /// \fn MLNode::lastChild
    /// \brief XMLNode::lastChild property 
    ///
    /// Read-only property; an XMLNode value that references the last
    /// child in the node's child list. The XML.lastChild property
    /// is null if the node does not have children. This property cannot
    /// be used to manipulate child nodes; use the appendChild(),
    /// insertBefore(), and removeNode() methods to manipulate child
    /// nodes.
    gettersetter = new builtin_function(&xmlnode_lastchild, NULL);
    o.init_readonly_property("lastChild", *gettersetter);

    gettersetter = new builtin_function(&xmlnode_nextsibling, NULL);
    o.init_readonly_property("nextSibling", *gettersetter);

    gettersetter = new builtin_function(&xmlnode_previoussibling, NULL);
    o.init_readonly_property("previousSibling", *gettersetter);

    gettersetter = new builtin_function(&xmlnode_parentNode, NULL);
    o.init_readonly_property("parentNode", *gettersetter);

}

// External, used by getXMLInterface() !
as_object*
getXMLNodeInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( o == NULL ) {
        o = new as_object(getObjectInterface());
        attachXMLNodeInterface(*o);
    }
    return o.get();
}

static as_value
xmlnode_new(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    
    XMLNode *xml_obj = new XMLNode;
    if ( fn.nargs > 0 )
    {
        xml_obj->nodeTypeSet(XMLNode::NodeType(int(fn.arg(0).to_number())));
        if (fn.nargs > 1)
        {
            xml_obj->nodeValueSet(fn.arg(1).to_string());
        }
    }
    
    return as_value(xml_obj);
}

static as_value
xmlnode_appendchild(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;

	boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);

	if ( ! fn.nargs )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("XMLNode::appendChild() needs at least one argument"));
		);
		return as_value();
	}

	boost::intrusive_ptr<XMLNode> xml_obj = boost::dynamic_pointer_cast<XMLNode>(fn.arg(0).to_object());	
	if ( ! xml_obj )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("First argument to XMLNode::appendChild() is not an XMLNode"));
		);
		return as_value();
	}

	ptr->appendChild(xml_obj);
	return as_value(); // undefined

}

static as_value
xmlnode_clonenode(const fn_call& fn)
{
    //GNASH_REPORT_FUNCTION;
//    log_msg("%s: %d args", __PRETTY_FUNCTION__, fn.nargs);
    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);

    bool deep = false;
    if (fn.nargs > 0) deep = fn.arg(0).to_bool();

    boost::intrusive_ptr<XMLNode> newnode = ptr->cloneNode(deep);
    return as_value(newnode.get());
}

static as_value
xmlnode_insertbefore(const fn_call& fn)
{
	//GNASH_REPORT_FUNCTION;
	boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);

	if ( fn.nargs < 2 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_aserror(_("XMLNode.insertBefore(%s) needs at least two argument"), ss.str().c_str());
		);
		return as_value();
	}

	boost::intrusive_ptr<XMLNode> newnode = boost::dynamic_pointer_cast<XMLNode>(fn.arg(0).to_object());
	if ( ! newnode )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_aserror(_("First argument to XMLNode.insertBefore(%s) is not an XMLNode"),
                ss.str().c_str());
		);
		return as_value();
	}

	boost::intrusive_ptr<XMLNode> pos = boost::dynamic_pointer_cast<XMLNode>(fn.arg(1).to_object());
	if ( ! pos )
	{
		IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
		log_aserror(_("Second argument to XMLNode.insertBefore(%s) is not an XMLNode"),
                ss.str().c_str());
		);
		return as_value();
	}

    ptr->insertBefore(newnode, pos);
    return as_value();
    
//    return as_value(ptr->obj.getAllocated());
//    ptr->obj.insertBefore();
    log_unimpl (__PRETTY_FUNCTION__);
    return as_value();
}

static as_value
xmlnode_removenode(const fn_call& fn)
{
    //GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);
    
    ptr->removeNode();
    return as_value();
}

// TODO: shouldn't overriding get_text_value() be fine ?
static as_value
xmlnode_tostring(const fn_call& fn)
{
    //GNASH_REPORT_FUNCTION;
    
    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);
    
    std::stringstream ss;
    ptr->toString(ss);
    //log_msg("Stringstream: %s", ss.str().c_str());

    return as_value(ss.str());
}

static as_value
xmlnode_haschildnodes(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);
    return as_value(ptr->hasChildNodes());
}

// Both a getter and a setter for nodeValue
static as_value
xmlnode_nodevalue(const fn_call& fn)
{
    //GNASH_REPORT_FUNCTION;

    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);
    as_value rv;
    rv.set_null();
    
    //log_msg("xmlnode_nodevalue called with %d args against 'this' = %p", fn.nargs, ptr);
    if ( fn.nargs == 0 )
    {
	    //log_msg("  nodeValue() returns '%s'", ptr->nodeValue().c_str());
        const std::string& val = ptr->nodeValue();
        if ( ! val.empty() ) rv = val;
    }
    else
    {
        ptr->nodeValueSet(fn.arg(0).to_string());
    }
    return rv;
}

// Both a getter and a setter for nodeName
static as_value
xmlnode_nodename(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);
    as_value rv;
    rv.set_null();

    if ( fn.nargs == 0 ) {
        const std::string& val = ptr->nodeName();
        if ( ! val.empty() ) rv = val;
    }
    else
    {
	ptr->nodeNameSet(fn.arg(0).to_string());
    }
    return rv;
}

// Both a getter and a (do-nothing) setter for nodeType
static as_value
xmlnode_nodetype(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    
    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);

    return as_value(ptr->nodeType());
}

// Both a getter and a (do-nothing) setter for attributes
static as_value
xmlnode_attributes(const fn_call& fn)
{
    //GNASH_REPORT_FUNCTION;
    
    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);

    VM& vm = ptr->getVM();
    string_table& st = vm.getStringTable();

    XMLNode::AttribList& attrs = ptr->attributes();
    //log_debug("Node %p has %d attributes", (void*)ptr.get(), attrs.size());

    boost::intrusive_ptr<as_object> ret = new as_object(); // attributes are not Object types (getObjectInterface());
    for (XMLNode::AttribList::const_iterator it=attrs.begin(),
        itEnd=attrs.end(); it != itEnd; ++it)
    {

        const XMLAttr& at = *it;
        const std::string& name = at.name();
        const std::string& val = at.value();
        //log_debug("%s: %s", name.c_str(), val.c_str());
        // These must be enumerable !
        ret->set_member(st.find(name), val);
    }

    return as_value(ret); 
}

// Both a getter and a (do-nothing) setter for firstChild
static as_value
xmlnode_firstchild(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);
    as_value rv;
    rv.set_null();

    boost::intrusive_ptr<XMLNode> node = ptr->firstChild();
    if (node) {
       rv = node.get();
    }

    return rv;
}

// Both a getter and a (do-nothing) setter for lastChild
static as_value
xmlnode_lastchild(const fn_call& fn)
{
    //GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);
    as_value rv;
    rv.set_null();

    boost::intrusive_ptr<XMLNode> node = ptr->lastChild();
    if (node) rv = node.get();

    return rv;
}

// Both a getter and a (do-nothing) setter for nextSibling
static as_value
xmlnode_nextsibling(const fn_call& fn)
{
    //GNASH_REPORT_FUNCTION;
    as_value rv;
    rv.set_null();

    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);
    XMLNode *node = ptr->nextSibling();
    if (node) {
	rv = node;
    }
    return rv;
}

// Both a getter and a (do-nothing) setter for previousSibling
static as_value
xmlnode_previoussibling(const fn_call& fn)
{
    //GNASH_REPORT_FUNCTION;
    as_value rv;
    rv.set_null();

    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);
    XMLNode *node = ptr->previousSibling();
    if (node) {
	rv = node;
    }
    return rv;
}

// Both a getter and a (do-nothing) setter for parentNode
static as_value
xmlnode_parentNode(const fn_call& fn)
{
    //GNASH_REPORT_FUNCTION;
    as_value rv;
    rv.set_null();

    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);
    XMLNode *node = ptr->getParent();
    if (node) {
	rv = node;
    }
    return rv;
}

// Both a getter and a (do-nothing) setter for childNodes
static as_value
xmlnode_childNodes(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);
    boost::intrusive_ptr<as_array_object> ary = new as_array_object();

    typedef XMLNode::ChildList ChildList;

    ChildList& child = ptr->childNodes();
    for ( ChildList::const_iterator it=child.begin(), itEnd=child.end();
                    it != itEnd; ++it )
    {
            boost::intrusive_ptr<XMLNode> node = *it;
            ary->push(as_value(node.get()));
    }

    return as_value(ary.get());
}

// extern (used by Global.cpp)
void xmlnode_class_init(as_object& global)
{
//    GNASH_REPORT_FUNCTION;
    // This is going to be the global XMLNode "class"/"function"
    static boost::intrusive_ptr<builtin_function> cl;

    if ( cl == NULL )
    {
        cl=new builtin_function(&xmlnode_new, getXMLNodeInterface());
    }

    // Register _global.String
    global.init_member("XMLNode", cl.get());

}

#ifdef GNASH_USE_GC
void
XMLNode::markReachableResources() const
{
	// Mark childs
	for (ChildList::const_iterator i=_children.begin(), e=_children.end(); i!=e; ++i)
	{
		(*i)->setReachable();
	}

	// Mark parent
	if ( _parent ) _parent->setReachable();

	markAsObjectReachable();
}
#endif // GNASH_USE_GC

} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
