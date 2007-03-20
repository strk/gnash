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

/* $Id: xmlnode.cpp,v 1.19 2007/03/20 15:01:20 strk Exp $ */

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
static as_value xmlnode_appendchild(const fn_call& fn);
static as_value xmlnode_clonenode(const fn_call& fn);
static as_value xmlnode_haschildnodes(const fn_call& fn);
static as_value xmlnode_insertbefore(const fn_call& fn);
static as_value xmlnode_removenode(const fn_call& fn);
static as_value xmlnode_tostring(const fn_call& fn);
static as_value xmlnode_nodename(const fn_call& fn);
static as_value xmlnode_firstchild(const fn_call& fn);
static as_value xmlnode_lastchild(const fn_call& fn);
static as_value xmlnode_nextsibling(const fn_call& fn);
static as_value xmlnode_previoussibling(const fn_call& fn);
static as_object* getXMLNodeInterface();

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
    _name = 0;
    _value = 0;
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

    for (i=0; i<_attributes.size(); i++) {
	if (_attributes[i]->_name) {
	    delete [] _attributes[i]->_name;
	}
	if (_attributes[i]->_value) {
	    delete [] _attributes[i]->_value;
	}
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

XMLNode *
XMLNode::firstChild()
{
    GNASH_REPORT_FUNCTION;
    if (_children.size() > 0) {
	return _children.front();
    }
    return NULL;
}

XMLNode *
XMLNode::lastChild()
{
    GNASH_REPORT_FUNCTION;
    
    if (_children.size() > 0) {
	return _children.back();
    }
    return NULL;
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

const char *
XMLNode::nodeName() 
{
    if (_name) {
	return _name;
    }
    return NULL;
}

const char *
XMLNode::nodeValue() 
{  
    if (_value) {
	return _value;
    }
    return NULL;
}

/// \brief append a node the the XMLNode object
///
/// Method; appends the specified node to the XMLNode object's child
/// list. This method operates directly on the node referenced by the
/// childNode parameter; it does not append a copy of the node. If the
/// node to be appended already exists in another tree structure,
/// appending the node to the new location will remove it from its
/// current location. If the childNode parameter refers to a node that
/// already exists in another XMLNode tree structure, the appended child
/// node is placed in the new tree structure after it is removed from
/// its existing parent node.
///
/// @param as
///	The XMLNode ?
///
/// @param node
///	same as XMLNode::obj ?
///
void
XMLNode::appendChild(XMLNode *node)
{
//    GNASH_REPORT_FUNCTION;
//     log_msg("%s: %p, as is %d, node is %d\n",
// 	    __PRETTY_FUNCTION__, this, _children.size(), _children.size());
//

    if (node) {
	node->setParent(this);
	_children.push_back(node);
    }

//    log_msg("%s: partially unimplemented\n", __PRETTY_FUNCTION__);
}

/// \brief copy a node
///
/// Method; constructs and returns a new XML node of the same type,
/// name, value, and attributes as the specified XML object. If deep
/// is set to true, all child nodes are recursively cloned, resulting
/// in an exact copy of the original object's document tree. 
XMLNode &
XMLNode::cloneNode(XMLNode &newnode, bool deep)
{
    GNASH_REPORT_FUNCTION;
    log_msg("%s: deep is %d\n", __PRETTY_FUNCTION__, deep);

    if (deep) {
//	newnode = _nodes;
    } else {
	newnode.nodeNameSet(_name);
	newnode.nodeValueSet(_value);
    }

    log_msg("%s: partially unimplemented \n", __PRETTY_FUNCTION__);
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

    vector<XMLNode *>::iterator itx;
    XMLNode *previous_node = NULL;
    if (_parent) {
 	if (_parent->_children.size() > 1) {
	    for (itx = _parent->_children.begin(); itx != _parent->_children.end(); itx++) {
		if ((*itx) == this) {
		    // log_msg("Found the previous XMLNode child !!!! %s <%p>\n", (*itx)->nodeName(), (void*)*itx);
		    return previous_node;
		}
		previous_node = *itx;
	    }
 	}
    }

    return NULL;
}

XMLNode *
XMLNode::nextSibling()
{
    GNASH_REPORT_FUNCTION;
    vector<XMLNode *>::iterator itx;
    if (_parent) {
 	if (_parent->_children.size() > 1) {
	    for (itx = _parent->_children.begin(); itx != _parent->_children.end(); itx++) {
		if ((*itx) == this) {
		    // We've found ourselves; now find next sibling if any
		    XMLNode *sibling = *++itx;
		    if (itx == _parent->_children.end()) return NULL;
		    else {
		        // log_msg("Found the next XMLNode child !!!! %s <%p>\n", (*itx)->nodeName(), (void*)*itx);
		        return sibling;
		    }
		}
	    }
 	}
    }
    return NULL;
}

const char *
XMLNode::toString()
{
//    GNASH_REPORT_FUNCTION;
    stringstream xmlout;
    return stringify(this, &xmlout);
}

const char *
XMLNode::stringify(XMLNode *xml, stringstream *xmlout)
{
//    GNASH_REPORT_FUNCTION;
    const char    *nodevalue = xml->nodeValue();
    const char    *nodename = xml->nodeName();
    
//    log_msg("%s: processing for object %s <%p>\n", __PRETTY_FUNCTION__, nodename, xml);

    // Create the beginning of the tag
    *xmlout << "<" << nodename;
    
    // Process the attributes, if any
    vector<XMLAttr *>::iterator ita;
    for (ita = xml->_attributes.begin(); ita != xml->_attributes.end(); ita++) {
	XMLAttr *xa = *ita;
// 	log_msg("\t\tAdding attribute as member %s, value is %s to node %s",
// 		nodename, xa->_name, xa->_value);
	*xmlout << " " << xa->_name << "=\"" << xa->_value << "\"";
    }

    *xmlout << ">";		// closing symbol for this tag
    
    if (nodevalue) {
	*xmlout << nodevalue;
	*xmlout << "</" << nodename << ">";
    }

//    int length = xml->_children.size();
//    log_msg("\tProcessing %d children nodes for %s", length, nodename);
    
    vector<XMLNode *>::iterator itx;
    for (itx = xml->_children.begin(); itx != xml->_children.end(); itx++) {
//	log_msg("Found One XMLNode child !!!! %s <%p>\n", (*itx)->nodeName(), (void*)*itx);
//	cerr << "<" << (*it)->nodeName() << ">" << endl;
	XMLNode *x = *itx;
	*xmlout << x->toString();
    }

    if (!nodevalue) {
	*xmlout << "</" << nodename << ">";
    }

    return xmlout->str().c_str();;
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

    // These two return an array of objects
    o.init_member("attributes", as_value(""));
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

static as_object*
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
    if (fn.nargs > 0) {
      boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);
//    log_msg("%s: %p, %d args\n", __PRETTY_FUNCTION__, ptr, fn.nargs);
	
	boost::intrusive_ptr<XMLNode> xml_obj = boost::dynamic_pointer_cast<XMLNode>(fn.arg(0).to_object());	
	if ( ! xml_obj )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("First argument to XMLNode::appendChild() is not an XMLNode");
		);
		return as_value();
	}

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
}

static as_value
xmlnode_clonenode(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
//    log_msg("%s: %d args\n", __PRETTY_FUNCTION__, fn.nargs);
    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);
    XMLNode   *xmlnode_obj;

    if (fn.nargs > 0) {
	bool deep = fn.arg(0).to_bool();
	xmlnode_obj = new XMLNode;
	ptr->cloneNode(*xmlnode_obj, deep);
	return as_value(xmlnode_obj);
    } else {
        log_msg("ERROR: no Depth paramater!\n");
    }
    return as_value();

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
//    GNASH_REPORT_FUNCTION;
    
    boost::intrusive_ptr<XMLNode> ptr = ensureType<XMLNode>(fn.this_ptr);
    
    return as_value(ptr->toString());
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
    if ( fn.nargs == 0 ) {
	//log_msg("  nodeValue() returns '%s'", ptr->nodeValue());
	const char* val = ptr->nodeValue();
	if ( val ) {
	    rv = val;
	}
    } else {
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
	if ( val ) {
	    rv = val;
	}
    } else {
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
	    XMLNode *node = ptr->firstChild();
	    if (node) {
		rv = node;
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
//    GNASH_REPORT_FUNCTION;
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

    XMLNode *node = ptr->lastChild();
    if (node) {
	rv = node;
    }
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
