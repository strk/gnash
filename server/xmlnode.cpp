// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

/* $Id: xmlnode.cpp,v 1.18 2006/09/29 10:00:01 nihilus Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
#include "smart_ptr.h"
#include "tu_config.h"
#include "fn_call.h"

#ifdef HAVE_LIBXML

//#define DEBUG_MEMORY_ALLOCATION 1

#include "xmlnode.h"

#ifdef DEBUG_MEMORY_ALLOCATION
	#include "log.h"
#endif

#include <unistd.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>

using namespace std;

namespace gnash {
    
//std::vector<as_object *> _xmlobjs;    // FIXME: hack alert

XMLNode::XMLNode() :_name(0), _value(0), _type(XML_ELEMENT_NODE)
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
       delete _children[i]->_name;
     }
     if (_children[i]->_value) {
       delete _children[i]->_value;
     }
    }

    for (i=0; i<_attributes.size(); i++) {
	if (_attributes[i]->_name) {
	    delete _attributes[i]->_name;
	}
	if (_attributes[i]->_value) {
	    delete _attributes[i]->_value;
	}
    }

    _children.clear();
    _attributes.clear();

    if (_name) {
        delete _name;
    }
    if (_value) {
        delete _value;
    }
    //  _value.set_undefined();
}

void
XMLNode::nodeNameSet(char *name)
{
  int len = strlen(name) + 1;
 
  if (!_name) {
    _name = (char *)new char[len];
    memset(_name, 0, len);
    strcpy(_name, reinterpret_cast<const char *>(name));
  }
}

void
XMLNode::nodeValueSet(char *value)
{
  int len = strlen(value) + 1;
 
  if (!_value) {
    _value = (char *)new char[len];
    memset(_value, 0, len);
    strcpy(_value, reinterpret_cast<const char *>(value));
  }
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
  return "unknown";
}

const char *
XMLNode::nodeValue() 
{  
  if (_value) {
    return _value;
  }
  return "unknown";
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
void
XMLNode::appendChild(as_object *as, XMLNode *node)
{
//     log_msg("%s: %p, as is %d, node is %d\n",
// 	    __PRETTY_FUNCTION__, this, _objects.size(), _children.size());

    if (node) {
	_children.push_back(node);
    }

    if (as) {
	_objects.push_back(as);
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
    log_msg("%s: deep is %d\n", __PRETTY_FUNCTION__, deep);

    if (deep) {
//	newnode = _nodes;
    } else {
	newnode.nodeNameSet(_name);
	newnode.nodeValueSet(_value);
    }

    return newnode;
  
    log_msg("%s: partially unimplemented \n", __PRETTY_FUNCTION__);
}

/// \brief insert a node before a node
///
/// Method; inserts a new child node into the XML object's child
/// list, before the beforeNode node. If the beforeNode parameter is
/// undefined or null, the node is added using the appendChild()
/// method. If beforeNode is not a child of my_xml, the insertion
/// fails.
void
XMLNode::insertBefore(XMLNode */* newnode */, XMLNode */* node */)
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
const char *
XMLNode::toString()
{
    XMLNode *node;
    
    vector<XMLNode *>::const_iterator it;
    for (it = _children.begin(); it != _children.end(); it++) {
	node = *it;
//	log_msg("Got something\n");
	if (node->_name) {
	    log_msg("Node name is %s", node->_name);
	}
	if (node->_value) {
	    log_msg("Node value is %s", node->_name);
	}	
    }
    
    log_msg("%s: unimplemented \n", __PRETTY_FUNCTION__);
    return "Hello World!";
}

as_object *
XMLNode::previousSibling(int x)
{
    log_msg("%s: partially implemented. %lu objects\n",
	    __PRETTY_FUNCTION__,  static_cast<unsigned long>(_objects.size()));
    if (_objects.size() > 0) {
	return _objects[x-1];
    }

    return NULL;
}

as_object *
XMLNode::nextSibling(int x)
{
    log_msg("%s: unimplemented \n", __PRETTY_FUNCTION__);
    if (x < (int) _objects.size()) {
	return _objects[x];
    }
    return NULL;
}

void
xmlnode_new(const fn_call& fn)
{
    xmlnode_as_object *xml_obj;
    //const char    *data;
  
//    log_msg("%s\n", __PRETTY_FUNCTION__);
  
    xml_obj = new xmlnode_as_object;
    // Methods
    xml_obj->set_member("appendChild", &xmlnode_appendchild);
    xml_obj->set_member("cloneNode", &xmlnode_clonenode);
    xml_obj->set_member("hasChildNodes", &xmlnode_haschildnodes);
    xml_obj->set_member("insertBefore", &xmlnode_insertbefore);
    xml_obj->set_member("removeNode", &xmlnode_removenode);
    xml_obj->set_member("toString", &xmlnode_tostring);

    // Properties
    xml_obj->set_member("nodeName",  as_value(""));
    xml_obj->set_member("nodeValue", as_value(""));
    xml_obj->set_member("nodeType", as_value(""));

    // FIXME: these need real values
    // These two return an array of objects
    xml_obj->set_member("attributes", as_value(""));
    xml_obj->set_member("childNodes", as_value(""));

    //These return a reference to an object

    /// \fn MLNode::firstChild
    /// \brief XMLNode::firstChild property
    ///
    /// Read-only property; evaluates the specified XML object and
    /// references the first child in the parent node\ufffds child
    /// list. This property is null if the node does not have
    /// children. This property is undefined if the node is a text
    /// node. This is a read-only property and cannot be used to
    /// manipulate child nodes; use the appendChild(), insertBefore(),
    /// and removeNode() methods to manipulate child nodes. 

    xml_obj->set_member("firstChild", as_value(""));
    /// \fn MLNode::lastChild
    /// \brief XMLNode::lastChild property 
    ///
    /// Read-only property; an XMLNode value that references the last
    /// child in the node's child list. The XML.lastChild property
    /// is null if the node does not have children. This property cannot
    /// be used to manipulate child nodes; use the appendChild(),
    /// insertBefore(), and removeNode() methods to manipulate child
    /// nodes.
    xml_obj->set_member("lastChild",   as_value(""));
    xml_obj->set_member("nextSibling", as_value(""));
    xml_obj->set_member("parentNode",  as_value(""));
    xml_obj->set_member("previousSibling", as_value(""));

    fn.result->set_as_object(xml_obj);
}

void xmlnode_appendchild(const fn_call& fn)
{
    xmlnode_as_object *ptr = (xmlnode_as_object*)fn.this_ptr;
    assert(ptr);
//    log_msg("%s: %p, %d args\n", __PRETTY_FUNCTION__, ptr, fn.nargs);
    
    xmlnode_as_object *xml_obj = (xmlnode_as_object*)fn.env->bottom(fn.first_arg_bottom_index).to_object();
    
//    log_msg("%s: %p \n", __PRETTY_FUNCTION__, xml_obj);
    XMLNode *node = &(xml_obj->obj);
    if (ptr->obj.hasChildNodes() == false) {
	ptr->set_member("firstChild", xml_obj);
    }
    int length = ptr->obj.length();
    if (length > 0) {
	as_object *ass = xml_obj->obj.previousSibling(length);
// FIXME: This shouldn't always be NULL
// 	log_msg("%s: ASS is %p, length is %d\n", __PRETTY_FUNCTION__,
// 		ass, length);
  	ptr->set_member("previousSibling", ass);
//  	ptr->set_member("nextSibling", xml_obj->obj.nextSibling(ptr->obj.length()));
    }
    ptr->obj.appendChild((as_object *)xml_obj, node);
    // The last child in the list is always the one we just appended
    ptr->set_member("lastChild", xml_obj);
   
//    ptr->obj.appendChild(&(xml_obj->obj));
//    ptr->obj.nodeValueSet((char *)xmlnode_obj->obj.nodeValue());
}

void xmlnode_clonenode(const fn_call& fn)
{
    log_msg("%s: %d args\n", __PRETTY_FUNCTION__, fn.nargs);
    xmlnode_as_object	*ptr = (xmlnode_as_object*)fn.this_ptr;
    xmlnode_as_object   *xmlnode_obj;
    assert(ptr);

    if (fn.nargs > 0) {
      bool deep = fn.env->bottom(fn.first_arg_bottom_index).to_bool();
      xmlnode_obj = new xmlnode_as_object;
      ptr->obj.cloneNode(xmlnode_obj->obj, deep);
      fn.result->set_as_object(xmlnode_obj);
   } else {
        log_msg("ERROR: no Depth paramater!\n");
    }

}

void xmlnode_insertbefore(const fn_call& fn)
{
    xmlnode_as_object *ptr = (xmlnode_as_object*)fn.this_ptr;
    assert(ptr);
    
//    fn.result->set_int(ptr->obj.getAllocated());
//    ptr->obj.insertBefore();
    log_msg("%s:unimplemented \n", __PRETTY_FUNCTION__);
}
void xmlnode_removenode(const fn_call& fn)
{
    xmlnode_as_object *ptr = (xmlnode_as_object*)fn.this_ptr;
    assert(ptr);
    
//    fn.result->set_int(ptr->obj.getAllocated());
    ptr->obj.removeNode();
}
void xmlnode_tostring(const fn_call& fn)
{
    xmlnode_as_object *ptr = (xmlnode_as_object*)fn.this_ptr;
    assert(ptr);
    
    fn.result->set_string(ptr->obj.toString());
}

void xmlnode_haschildnodes(const fn_call& fn)
{
    xmlnode_as_object *ptr = (xmlnode_as_object*)fn.this_ptr;
    assert(ptr);
    fn.result->set_bool(ptr->obj.hasChildNodes());
}

#ifdef ENABLE_TESTING
void xmlnode_nodevalue(const fn_call& fn)
{
    log_msg("%s: \n", __PRETTY_FUNCTION__);
    xmlnode_as_object *ptr = (xmlnode_as_object*)fn.this_ptr;
    assert(ptr);
    
    fn.result->set_string(ptr->obj.nodeValue());
}
void xmlnode_nodename(const fn_call& fn)
{
    log_msg("%s: \n", __PRETTY_FUNCTION__);
    xmlnode_as_object *ptr = (xmlnode_as_object*)fn.this_ptr;
    assert(ptr);
    
    fn.result->set_string(ptr->obj.nodeName());
}
#endif

} // end of gnash namespace

// HAVE_LIBXML
#endif


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
