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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <vector>

#include "log.h"
#include "action.h"
#include "impl.h"
#include "smart_ptr.h"
#include "tu_config.h"
#include "Function.h"

#ifdef HAVE_LIBXML

#include "xmlnode.h"

#include <unistd.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>

using namespace std;

namespace gnash {
  
//#define DEBUG_MEMORY_ALLOCATION 1
  
//std::vector<as_object *> _xmlobjs;    // FIXME: hack alert

XMLNode::XMLNode() :_name(0), _value(0), _type(XML_ELEMENT_NODE)
{
    //log_msg("%s: %p \n", __FUNCTION__, this);
#ifdef DEBUG_MEMORY_ALLOCATION
    log_msg("\tCreating XMLNode data at %p \n", this);
#endif
    _name = 0;
    _value = 0;
}

XMLNode::~XMLNode()
{
    unsigned int i;
    //log_msg("%s: %p \n", __FUNCTION__, this);
#ifdef DEBUG_MEMORY_ALLOCATION
    log_msg("\tDeleting XMLNode data %s at %p\n", this->_name, this);
#endif
  
    for (i=0; i<_children.size(); i++) {
//     if (_children[i]->_name) {
//       delete _children[i]->_name;
//     }
//     if (_children[i]->_value) {
//       delete _children[i]->_value;
//     }
        delete _children[i];
    }

    for (i=0; i<_attributes.size(); i++) {
        //     if (_attributes[i]->_name) {
//       delete _attributes[i]->_name;
//     }
//     if (_attributes[i]->_value) {
//       delete _attributes[i]->_value;
//     }
        delete _attributes[i];
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

#ifdef ENABLE_TESTING
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
#endif

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

void
XMLNode::appendChild(XMLNode *node)
{
    log_msg("%s: %p\n", __PRETTY_FUNCTION__, this);
    XMLNode *cur;

     switch (node->_type) {
       case XML_TEXT_NODE:
// 	  log_msg("%s: Text Node\n", __PRETTY_FUNCTION__);
	  nodeValueSet(node->_value);
// 	  cur = _children.back();
// 	  cur->nodeValueSet(node->_value);
 	  break;
       case XML_ELEMENT_NODE:
// 	  log_msg("%s: Element Node\n", __PRETTY_FUNCTION__);
//	   _children.push_back(node);
 	  break;
       default:
 	  break;
     }
}    

void
xmlnode_new(const fn_call& fn)
{
    as_value      inum;
    xmlnode_as_object *xml_obj;
    //const char    *data;
  
    log_msg("%s: nargs=%d\n", __PRETTY_FUNCTION__, fn.nargs);
  
    xml_obj = new xmlnode_as_object;
#ifdef ENABLE_TESTING
    xml_obj->set_member("nodeName", &xmlnode_nodename);
    xml_obj->set_member("nodeValue", &xmlnode_nodevalue);
    xml_obj->set_member("appendChild", &xmlnode_appendchild);
#endif

    fn.result->set_as_object_interface(xml_obj);
}

void xmlnode_appendchild(const fn_call& fn)
{
    xmlnode_as_object *ptr = (xmlnode_as_object*)fn.this_ptr;
    assert(ptr);
//    log_msg("%s: %p, %d args\n", __PRETTY_FUNCTION__, ptr, fn.nargs);
    
    xmlnode_as_object *xml_obj = (xmlnode_as_object*)fn.env->bottom(fn.first_arg_bottom_index).to_object();
    
//    log_msg("%s: %p \n", __PRETTY_FUNCTION__, xml_obj);
    ptr->obj.appendChild(&(xml_obj->obj));
//    ptr->obj.nodeValueSet((char *)xml_obj->obj.nodeValue());
}

#ifdef ENABLE_TESTING
void xmlnode_nodevalue(const fn_call& fn)
{
  //    log_msg("%s: \n", __PRETTY_FUNCTION__);
    xmlnode_as_object *ptr = (xmlnode_as_object*)fn.this_ptr;
    assert(ptr);
    
    fn.result->set_string(ptr->obj.nodeValue());
}
void xmlnode_nodename(const fn_call& fn)
{
  //    log_msg("%s: \n", __PRETTY_FUNCTION__);
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
