// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/* $Id: xmlnode.cpp,v 1.23 2007/04/04 09:02:10 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
#include "log.h"
#include "tu_config.h"
#include "fn_call.h"
#include "builtin_function.h"

//#define DEBUG_MEMORY_ALLOCATION 1

#include "xmlnode.h"

#ifdef DEBUG_MEMORY_ALLOCATION
#include "log.h"
#endif

#include <unistd.h>
#include <string>
#include <sstream>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>

using namespace std;

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
as_object* getXMLNodeInterface();

static LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();


//std::vector<as_object *> _xmlobjs;    // FIXME: hack alert

XMLNode::XMLNode()
    :
    as_object(getXMLNodeInterface()),
    _name(0),
    _value(0),
    _type(XML_ELEMENT_NODE),
    _parent(0)
{
    //log_msg("%s: %p \n", __PRETTY_FUNCTION__, this);
#ifdef DEBUG_MEMORY_ALLOCATION
    log_msg("\tCreating XMLNode data at %p \n", this);
#endif
}

XMLNode::XMLNode(as_object* overridden_interface)
    :
    as_object(overridden_interface),
    _name(0),
    _value(0),
    _type(XML_ELEMENT_NODE),
    _parent(0)
{
    //log_msg("%s: %p \n", __PRETTY_FUNCTION__, this);
#ifdef DEBUG_MEMORY_ALLOCATION
    log_msg("\tCreating XMLNode data at %p \n", this);
#endif
}

XMLNode::XMLNode(const XMLNode& tpl, bool deep)
    :
    as_object(getXMLNodeInterface()),
    _name(tpl._name),
    _value(tpl._value),
    _type(tpl._type),
    _parent(tpl._parent)
{
    if ( ! deep )
    {
            _children = tpl._children;
    }
    else // deep copy
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
    unsigned int i;
    //log_msg("%s: %p \n", __PRETTY_FUNCTION__, this);
#ifdef DEBUG_MEMORY_ALLOCATION
    log_msg("\tDeleting XMLNode data %s at %p\n", this->_name, this);
#endif
  
    for (i=0; i<_children.size(); i++) {
	if (_children[i]->_name) {
	    delete [] _children[i]->_name;
	}
	if (_children[i]->_value) {
	    delete [] _children[i]->_value;
	}
    }

    for (i=0; i<_attributes.size(); i++)
    {
            // shouldn't we delete attributes here ??
            // TODO: plug this leak somehow !!
    }

    _children.clear();
    _attributes.clear();

    if (_name) {
        delete [] _name;
    }
    if (_value) {
        delete [] _value;
    }
    //  _value.set_undefined();
}

bool
XMLNode::hasChildNodes()
{
    GNASH_REPORT_FUNCTION;
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
	GNASH_REPORT_FUNCTION;
	if ( _children.empty() )
	{
			log_msg("XMLNode %p has no childrens", (void*)this);
			return NULL;
	}
	return _children.back();
}

void
XMLNode::nodeNameSet(const char *name)
{
    int len = strlen(name) + 1;
 
    if (!_name) {
	_name = (char *)new char[len];
	memset(_name, 0, len);
	strcpy(_name, name);
    }
}

void
XMLNode::nodeValueSet(const char *value)
{
    int len = strlen(value) + 1;
 
    // Should we use std::string here ?
    delete [] _value;
    _value = new char[len];
    memset(_value, 0, len);
    strcpy(_value, value);
}

/// \brief Get the type of an XML Node.
///

/// Read-only property; a nodeType value, either 1 for an XML element
/// or 3 for a text node. The nodeType is a numeric value from the
/// NodeType enumeration in the W3C DOM Level 1 recommendation:
/// www.w3.org/TR/1998/REC-DOM-Level-1-19981001/level-one-core.html.
/// The following table lists the values.
int
XMLNode::nodeType() 
{
    switch (_type) {
      case XML_TEXT_NODE:
	  return 3;
	  break;
      case XML_ELEMENT_NODE:
	  return 1;
	  break;
      default:
	  return 0;
    }
    // you should never get here
    return -1;
}

void
XMLNode::appendChild(boost::intrusive_ptr<XMLNode> node)
{
    if (node)
	{
		node->setParent(this);
		_children.push_back(node);
    }

//    log_msg("%s: partially unimplemented\n", __PRETTY_FUNCTION__);
}

boost::intrusive_ptr<XMLNode> 
XMLNode::cloneNode(bool deep)
{
    GNASH_REPORT_FUNCTION;
    log_msg("%s: deep is %d\n", __PRETTY_FUNCTION__, deep);

    boost::intrusive_ptr<XMLNode> newnode = new XMLNode(*this, deep);

    return newnode;
}

/// \brief insert a node before a node
///
/// Method; inserts a new child node into the XML object's child
/// list, before the beforeNode node. If the beforeNode parameter is
/// undefined or null, the node is added using the appendChild()
/// method. If beforeNode is not a child of my_xml, the insertion
/// fails.
void
XMLNode::insertBefore(XMLNode * /* newnode */, XMLNode * /* node */)
{
    log_msg("%s: unimplemented \n", __PRETTY_FUNCTION__);
}
/// \brief removes the specified XML object from its parent. Also
/// deletes all descendants of the node.
void
XMLNode::removeNode()
{
    log_msg("%s: unimplemented \n", __PRETTY_FUNCTION__);
}

XMLNode *
XMLNode::previousSibling()
{
    GNASH_REPORT_FUNCTION;

    if ( ! _parent) return NULL;
 	if (_parent->_children.size() <= 1) return NULL;

    XMLNode *previous_node = NULL;
    ChildList::iterator itx;
    for (itx = _parent->_children.begin(); itx != _parent->_children.end(); itx++)
    {
        if (itx->get() == this)
        {
            // log_msg("Found the previous XMLNode child !!!! %s <%p>\n", (*itx)->nodeName(), (void*)*itx);
		    return previous_node;
		}
		previous_node = itx->get();
    }

    return NULL;
}

XMLNode *
XMLNode::nextSibling()
{
    GNASH_REPORT_FUNCTION;

    if ( ! _parent) return NULL;
 	if (_parent->_children.size() <= 1) return NULL;

    XMLNode *previous_node = NULL;
    ChildList::reverse_iterator itx;
    for (itx = _parent->_children.rbegin(); itx != _parent->_children.rend(); itx++)
    {
        if (itx->get() == this)
        {
            // log_msg("Found the next XMLNode child !!!! %s <%p>\n", (*itx)->nodeName(), (void*)*itx);
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
    const char* nodevalue = xml.nodeValue();
    const char* nodename = xml.nodeName();


//    log_msg("%s: processing for object %s <%p>\n", __PRETTY_FUNCTION__, nodename, xml);

#ifdef GNASH_DEBUG
    log_msg("Stringifying node %p with name %s, value %s, %u attributes and %u childs",
                    (void*)&xml, nodename, nodevalue, xml._attributes.size(), xml._children.size());
#endif

    // Create the beginning of the tag
    if (nodename)
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

        xmlout << ">";
    }

    // Node value first, then childs
    if ( nodevalue )
    {
	    xmlout << nodevalue;
    }

    // Childs, after node value.
    ChildList::const_iterator itx;
    for (itx = xml._children.begin(); itx != xml._children.end(); itx++)
    {
//      log_msg("Found One XMLNode child !!!! %s <%p>\n", (*itx)->nodeName(), (void*)*itx);
//      cerr << "<" << (*it)->nodeName() << ">" << endl;
        (*itx)->toString(xmlout);
    }

    if (nodename)
    {
	    assert(nodename);
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
    o.init_property("nodeType", *gettersetter, *gettersetter);

    gettersetter = new builtin_function(&xmlnode_attributes, NULL);
    o.init_property("attributes", *gettersetter, *gettersetter);

    // These two return an array of objects
    o.init_member("childNodes", as_value(""));

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
    o.init_property("firstChild", *gettersetter, *gettersetter);

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
    o.init_property("lastChild", *gettersetter, *gettersetter);

    gettersetter = new builtin_function(&xmlnode_nextsibling, NULL);
    o.init_property("nextSibling", *gettersetter, *gettersetter);

    gettersetter = new builtin_function(&xmlnode_previoussibling, NULL);
    o.init_property("previousSibling", *gettersetter, *gettersetter);

    o.init_member("parentNode",  as_value().set_null());

}

// External, used by getXMLInterface() !
as_object*
getXMLNodeInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( o == NULL ) {
	o = new as_object();
	attachXMLNodeInterface(*o);
    }
    return o.get();
}

static as_value
xmlnode_new(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    
    XMLNode *xml_obj = new XMLNode;
    if ( fn.nargs > 0 )	{
	xml_obj->nodeTypeSet(static_cast<xmlElementType>(
				 static_cast<int>(fn.arg(0).to_number())));
	if (fn.nargs > 1)	{
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
		log_aserror("XMLNode::appendChild() needs at least one argument");
		);
		return as_value();
	}

	boost::intrusive_ptr<XMLNode> xml_obj = boost::dynamic_pointer_cast<XMLNode>(fn.arg(0).to_object());	
	if ( ! xml_obj )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("First argument to XMLNode::appendChild() is not an XMLNode");
		);
		return as_value();
	}

	ptr->appendChild(xml_obj);
	return as_value(); // undefined

#if 0
	if (xml_obj->nodeType() == XML_ELEMENT_NODE) {
	    ptr->appendChild(xml_obj.get());
	} else {
	    ptr->nodeValueSet(xml_obj->nodeValue());
	}
	
//    log_msg("%s: %p \n", __PRETTY_FUNCTION__, xml_obj);
	int length = ptr->length();
	if (length > 0) {
	    XMLNode *ass = xml_obj->previousSibling(); // or is it 'ptr' ??
// FIXME: This shouldn't always be NULL
// 	log_msg("%s: ASS is %p, length is %d\n", __PRETTY_FUNCTION__,
// 		ass, length);
	    ptr->set_member("previousSibling", ass); // FIXME: don't do this, rely on getter/setter
//  	ptr->set_member("nextSibling", xml_obj->obj.nextSibling(ptr->obj.length()));
	}
	// The last child in the list is always the one we just appended
	ptr->set_member("lastChild", xml_obj.get()); // FIXME: don't do this, rely on getter/setter
    } else {
        log_msg("ERROR: no child XMLNode paramaters!\\n");
    }
    return as_value();
#endif
}

static as_value
xmlnode_clonenode(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
//    log_msg("%s: %d args\n", __PRETTY_FUNCTION__, fn.nargs);
    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);

    bool deep = false;
    if (fn.nargs > 0) deep = fn.arg(0).to_bool();

    boost::intrusive_ptr<XMLNode> newnode = ptr->cloneNode(deep);
    return as_value(newnode.get());
}

static as_value
xmlnode_insertbefore(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);
    
//    return as_value(ptr->obj.getAllocated());
//    ptr->obj.insertBefore();
    log_msg("%s:unimplemented \n", __PRETTY_FUNCTION__);
    return as_value();
}

static as_value
xmlnode_removenode(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);
    
//    return as_value(ptr->obj.getAllocated());
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
	    //log_msg("  nodeValue() returns '%s'", ptr->nodeValue());
        const char* val = ptr->nodeValue();
        if ( val ) rv = val;
    }
    else
    {
        //log_msg(" arg(0) == '%s'", fn.arg(0).to_string());
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
        const char* val = ptr->nodeName();
        if ( val ) rv = val;
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

    if ( fn.nargs == 0 ) {
	return as_value(ptr->nodeType());
    } else {
	IF_VERBOSE_ASCODING_ERRORS(
	    log_aserror("Tried to set read-only property XMLNode.nodeType");
	    );
    }
    return as_value();
}

// Both a getter and a (do-nothing) setter for attributes
static as_value
xmlnode_attributes(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    
    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);

    if ( fn.nargs == 0 )
	{
        XMLNode::AttribList& attrs = ptr->attributes();
		boost::intrusive_ptr<as_object> ret = new as_object();
		for (XMLNode::AttribList::const_iterator it=attrs.begin(),
                        itEnd=attrs.end();
                it != itEnd; ++it)
		{
                const XMLAttr& at = *it;
                const std::string& name = at.name();
                const std::string& val = at.value();
                ret->init_member(name, val);
		}
		//log_error("FIXME: XMLNode.attributes not implemented yet");
		return as_value(ret); 
    }
	else
	{
		IF_VERBOSE_ASCODING_ERRORS(
	    log_aserror("Tried to set read-only property XMLNode.attributes");
	    );
    }
    return as_value();
}

// Both a getter and a (do-nothing) setter for firstChild
static as_value
xmlnode_firstchild(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);
    as_value rv;
    rv.set_null();

    if ( fn.nargs == 0 )
    {
        boost::intrusive_ptr<XMLNode> node = ptr->firstChild();
        if (node) {
		    rv = node.get();
	    }
    }
    else
    {
        IF_VERBOSE_ASCODING_ERRORS(
	    log_aserror("Tried to set read-only property XMLNode.firstChild");
	    );
    }

    return rv;
}

// Both a getter and a (do-nothing) setter for lastChild
static as_value
xmlnode_lastchild(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);
    as_value rv;
    rv.set_null();

    if ( fn.nargs != 0 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
	    log_aserror("Tried to set read-only property XMLNode.lastChild");
	    );
        return rv;
    } 

    boost::intrusive_ptr<XMLNode> node = ptr->lastChild();
    if (node) rv = node.get();

    return rv;
}

// Both a getter and a (do-nothing) setter for nextSibling
static as_value
xmlnode_nextsibling(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    as_value rv;
    rv.set_null();

    if ( fn.nargs != 0 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror("Tried to set read-only property XMLNode.nextSibling");
        );
	return rv;
    }
    
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
    GNASH_REPORT_FUNCTION;
    as_value rv;
    rv.set_null();

    if ( fn.nargs != 0 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror("Tried to set read-only property XMLNode.previousSibling");
        );
	return rv;
    }

    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);
    XMLNode *node = ptr->previousSibling();
    if (node) {
	rv = node;
    }
    return rv;
}

// extern (used by Global.cpp)
void xmlnode_class_init(as_object& global)
{
//    GNASH_REPORT_FUNCTION;
    // This is going to be the global XMLNode "class"/"function"
    static boost::intrusive_ptr<builtin_function> cl;

    if ( cl == NULL ) {
	cl=new builtin_function(&xmlnode_new, getXMLNodeInterface());
	// replicate all interface to class, to be able to access
	// all methods as static functions
	attachXMLNodeInterface(*cl);	     
    }

    // Register _global.String
    global.init_member("XMLNode", cl.get());

}

} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
