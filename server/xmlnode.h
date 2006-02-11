// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifndef __XML_NODE_H__
#define __XML_NODE_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#define DEBUG_MEMORY_ALLOCATION 1
#include <vector>
#include "log.h"
#include "action.h"
#include "impl.h"

#ifdef HAVE_LIBXML

#include "xmlattrs.h"

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>

namespace gnash {  
 
/// XML Node 
class XMLNode
{
public:
    XMLNode();
    ~XMLNode();


    
    int length()                 { return _children.size(); }
    const char *nodeName();
    const char *nodeValue();
  //    char *valueGet();
  
#ifdef ENABLE_TESTING
    void nodeNameSet(char *name);
    void nodeValueSet(char *value);
#endif
    //  nodeType 	XML.nodeType

    bool hasChildNodes() {
      if (_children.size()) {
        return true;
      }
      return false;
    }
  
    XMLNode *firstChild()		{ return _children[0]; }
  
    std::vector<XMLNode *>childNodes()  { return _children; }  
    
    XMLNode *operator [] (int x) {
        gnash::log_msg("%s: get element %d\n", __PRETTY_FUNCTION__, x);
        
        return _children[x];
    }
    
    XMLNode *operator = (XMLNode &node) {
        gnash::log_msg("%s: \n", __PRETTY_FUNCTION__);
        _name = node._name;
        _value = node._value;
	_children = node._children;
        _attributes = node._attributes;
        return this;
    }
    
    XMLNode *operator = (XMLNode *node) {
        gnash::log_msg("%s: \n", __PRETTY_FUNCTION__);
        _name = node->_name;
        _value = node->_value;
	_children = node->_children;
        _attributes = node->_attributes;
        return this;
    }

    as_object *previousSibling(int x);
    as_object *nextSibling(int x);
    XMLNode &cloneNode(XMLNode &newnode, bool deep);
    void appendChild(as_object *as,XMLNode *node);
    void insertBefore(XMLNode *newnode, XMLNode *node);
    void removeNode();
    const char *toString();

    void  change_stack_frame(int frame, gnash::as_object *xml, gnash::as_environment *env);


    char                *_name;
    char                *_value;
    xmlElementType      _type;
    std::vector<as_object *>  _objects;
    std::vector<XMLNode *>    _children;
    std::vector<XMLAttr *>    _attributes;
};

/// XML Node ActionScript object
struct xmlnode_as_object : public gnash::as_object
{
    XMLNode		obj;
//    int			_padding;
    
#ifdef DEBUG_MEMORY_ALLOCATION
    xmlnode_as_object() {
	//      obj  = new XMLNode;
        log_msg("\tCreating xmlnode_as_object at %p \n", this);
    };
    ~xmlnode_as_object() {
        log_msg("\tDeleting xmlnode_as_object at %p \n", this);
    };
#endif
};

void xmlnode_new(const fn_call& fn);

#ifdef ENABLE_TESTING
void xmlnode_haschildren(const fn_call& fn);
void xmlnode_nodename(const fn_call& fn);
void xmlnode_nodevalue(const fn_call& fn);
#endif

void xmlnode_appendchild(const fn_call& fn);
void xmlnode_clonenode(const fn_call& fn);
void xmlnode_haschildnodes(const fn_call& fn);
void xmlnode_insertbefore(const fn_call& fn);
void xmlnode_removenode(const fn_call& fn);
void xmlnode_tostring(const fn_call& fn);

} // end of gnash namespace


#endif // HAVE_LIBXML

#endif	// __XMLNODE_NODE_H__


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
