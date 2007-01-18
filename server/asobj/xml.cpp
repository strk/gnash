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

/* $Id: xml.cpp,v 1.11 2007/01/18 19:38:05 bjacques Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "tu_config.h"
#include "as_function.h" // for as_function
#include "fn_call.h"

#include "xmlattrs.h"
#include "xmlnode.h"
#include "xml.h"
#include "builtin_function.h"

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <vector>

using namespace std;

namespace gnash {
  
//#define DEBUG_MEMORY_ALLOCATION 1

static as_object* getXMLInterface();

DSOEXPORT void xml_new(const fn_call& fn);
static void xml_load(const fn_call& fn);
static void xml_set_current(const fn_call& fn);

static void xml_addrequestheader(const fn_call& fn);
static void xml_appendchild(const fn_call& fn);
static void xml_clonenode(const fn_call& fn);
static void xml_createelement(const fn_call& fn);
static void xml_createtextnode(const fn_call& fn);
static void xml_getbytesloaded(const fn_call& fn);
static void xml_getbytestotal(const fn_call& fn);
static void xml_haschildnodes(const fn_call& fn);
static void xml_insertbefore(const fn_call& fn);
static void xml_parsexml(const fn_call& fn);
static void xml_removenode(const fn_call& fn);
static void xml_send(const fn_call& fn);
static void xml_sendandload(const fn_call& fn);
static void xml_tostring(const fn_call& fn);
static void xml_firstchild(const fn_call& fn);
static void xml_childnodes(const fn_call& fn);

// These are the event handlers called for this object
static void xml_onload(const fn_call& fn);
static void xml_ondata(const fn_call& fn);
static void xml_loaded(const fn_call& fn);

// Properties
static void xml_nodename(const fn_call& fn);
static void xml_nodevalue(const fn_call& fn);
  
XML::XML() 
	:
	as_object(getXMLInterface()),
	_loaded(false), 
	_nodename(0),
	_nodes(0),
	_bytes_loaded(0),
	_bytes_total(0)
{
    GNASH_REPORT_FUNCTION;
#ifdef DEBUG_MEMORY_ALLOCATION
    log_msg("Creating XML data at %p \n", this);
#endif
    //log_msg("%s: %p \n", __FUNCTION__, this);
}


// Parse the ASCII XML string into memory
XML::XML(tu_string xml_in)
	:
	as_object(getXMLInterface()),
	_loaded(false), 
	_nodename(0),
	_nodes(0),
	_bytes_loaded(0),
	_bytes_total(0)
{
    GNASH_REPORT_FUNCTION;
#ifdef DEBUG_MEMORY_ALLOCATION
    log_msg("Creating XML data at %p \n", this);
#endif
    //log_msg("%s: %p \n", __FUNCTION__, this);
    //memset(&_nodes, 0, sizeof(XMLNode));
    parseXML(xml_in);
}

XML::XML(struct node */* childNode */)
	:
	as_object(getXMLInterface()),
	_loaded(false), 
	_nodename(0),
	_nodes(0),
	_bytes_loaded(0),
	_bytes_total(0)
{
    GNASH_REPORT_FUNCTION;
#ifdef DEBUG_MEMORY_ALLOCATION
    log_msg("\tCreating XML data at %p \n", this);
#endif
    //log_msg("%s: %p \n", __FUNCTION__, this);
}


XML::~XML()
{
    GNASH_REPORT_FUNCTION;
    
#ifdef DEBUG_MEMORY_ALLOCATION
    if (this->_nodes) {
        log_msg("\tDeleting XML top level node %s at %p \n", this->_nodes->_name, this);
    } else {
        log_msg("\tDeleting XML top level node at %p \n", this);
    }
#endif
  
    //log_msg("%s: %p \n", __FUNCTION__, this);
    delete _nodes;
}

const char *
XML::nodeName()
{
  printf("%s: XML %p _nodes at %p\n", __PRETTY_FUNCTION__, (void*)this, (void*)_nodes);
  if (_nodes) {
    return _nodes->nodeName();
  }
  return "undefined";
}

const char *
XML::nodeValue()
{
  printf("%s: XML _nodes at %p\n", __PRETTY_FUNCTION__, (void*)_nodes);
  if (_nodes) {
    return _nodes->nodeValue();
  }
  return "undefined";
}

void
XML::nodeNameSet(const char */*name */)
{
  if (!_nodes) {
    _nodes = new XMLNode;
    printf("%s: New XML %p _nodes at %p\n", __PRETTY_FUNCTION__, (void*)this, (void*)_nodes);
  }
  //  _nodes->nodeNameSet(name);
  printf("%s: XML %p _name at %p, %s\n", __PRETTY_FUNCTION__, (void*)this,
	 _nodes->nodeName(), _nodes->nodeName() );
}

void
XML::nodeValueSet(const char */* value */)
{
  if (!_nodes) {
    _nodes = new XMLNode;
    printf("%s: New XML _nodes at %p\n", __PRETTY_FUNCTION__, (void*)_nodes);
  }
  
  //  _nodes->nodeValueSet(value);
  printf("%s: XML _nodes at %p\n", __PRETTY_FUNCTION__, (void*)_nodes);
}

// Dispatch event handler(s), if any.
bool
XML::on_event(const event_id& /* id */)
{
    GNASH_REPORT_FUNCTION;
    
    // Keep m_as_environment alive during any method calls!
    //  boost::intrusive_ptr<as_object_interface>	this_ptr(this);
  
#if 0
    // First, check for built-in event handler.
    as_value	method;
    if (get_event_handler(event_id(id), &method)) {
        call_method0(method, &m_as_environment, this);
        return true;
    }
  
    // Check for member function.
    // In ActionScript 2.0, event method names are CASE SENSITIVE.
    // In ActionScript 1.0, event method names are CASE INSENSITIVE.
    const tu_string&	method_name = id.get_function_name();
    if (method_name.length() > 0) {
        as_value	method;
        if (get_member(method_name, &method)) {
            call_method0(method, &m_as_environment, this);
            return true;
        }
    }
#endif
    return false;
}

void
XML::on_event_load()
{
    // Do the events that (appear to) happen as the movie
    // loads.  frame1 tags and actions are executed (even
    // before advance() is called).  Then the onLoad event
    // is triggered.
    on_event(event_id::LOAD);
}

XMLNode*
XML::extractNode(xmlNodePtr node, bool mem)
{
    xmlAttrPtr attr;
    xmlNodePtr childnode;
    xmlChar *ptr = NULL;
    XMLNode *element, *child;
    int len;

    element = new XMLNode;
            
    //log_msg("Created new element for %s at %p\n", node->name, element);
    memset(element, 0, sizeof (XMLNode));

    log_msg("%s: extracting node %s\n", __FUNCTION__, node->name);

    // See if we have any Attributes (properties)
    attr = node->properties;
    while (attr != NULL) {
        //log_msg("extractNode %s has property %s, value is %s\n",
        //          node->name, attr->name, attr->children->content);
        XMLAttr *attrib = new XMLAttr;
        len = memadjust(strlen(reinterpret_cast<const char *>(attr->name))+1);
        attrib->_name = (char *)new char[len];
        memset(attrib->_name, 0, len);
        strcpy(attrib->_name, reinterpret_cast<const char *>(attr->name));
        len = memadjust(strlen(reinterpret_cast<const char *>(attr->children->content))+1);
        attrib->_value = (char *)new char[len];
        memset(attrib->_value, 0, len);
        strcpy(attrib->_value, reinterpret_cast<const char *>(attr->children->content));
        //log_msg("\tPushing attribute %s for element %s has value %s\n",
        //        attr->name, node->name, attr->children->content);
        element->_attributes.push_back(attrib);
        attr = attr->next;
    }

    len = memadjust(strlen(reinterpret_cast<const char *>(node->name))+1);
    element->_name = (char *)new char[len];
    memset(element->_name, 0, len);
    strcpy(element->_name, reinterpret_cast<const char *>(node->name));
    //element->_name = reinterpret_cast<const char *>(node->name);
    if (node->children) {
        //ptr = node->children->content;
        ptr = xmlNodeGetContent(node->children);
        if (ptr != NULL) {
            if ((strchr((const char *)ptr, '\n') == 0) && (ptr[0] != 0)) {
                if (node->children->content == NULL) {
                    //log_msg("Node %s has no contents\n", node->name);
                } else {
                    //log_msg("extractChildNode from text for %s has contents %s\n", node->name, ptr);
                    len = memadjust(strlen(reinterpret_cast<const char *>(ptr))+1);
                    element->_value = (char *)new char[len];
                    memset(element->_value, 0, len);
                    strcpy(element->_value, reinterpret_cast<const char *>(ptr));
                    //element->_value = reinterpret_cast<const char *>(ptr);
                }
            }
            xmlFree(ptr);
        }
    }
    
    // See if we have any data (content)
    childnode = node->children;

    while (childnode != NULL) {
        if (childnode->type == XML_ELEMENT_NODE) {
            //log_msg("\t\t extracting node %s\n", childnode->name);
            child = extractNode(childnode, mem);
            //if (child->_value.get_type() != as_value::UNDEFINED) {
            if (child->_value != 0) {
                //log_msg("\tPushing childNode %s, value %s on element %p\n", child->_name.c_str(), child->_value.to_string(), element);
            } else {
                //log_msg("\tPushing childNode %s on element %p\n", child->_name.c_str(), element);
            }
            element->_children.push_back(child);
        }
        childnode = childnode->next;
    }

    return element;
}

// Read in an XML document from the specified source
bool
XML::parseDoc(xmlDocPtr document, bool mem)
{
    GNASH_REPORT_FUNCTION;
    
    XMLNode *top;
    xmlNodePtr cur;

    if (document == 0) {
        log_error("Can't load XML file!\n");
        return false;
    }

    cur = xmlDocGetRootElement(document);
  
    if (cur != NULL) {
        top = extractNode(cur, mem);
        //_nodes->_name = reinterpret_cast<const char *>(cur->name);
        _nodes = top;
        //_node_data.push_back(top);
        //cur = cur->next;
    }  

    _loaded = true;
    return true;
}

// This reads in an XML file from disk and parses into into a memory resident
// tree which can be walked through later.
bool
XML::parseXML(tu_string xml_in)
{
    GNASH_REPORT_FUNCTION;

    log_msg("Parse XML from memory: %s\n", xml_in.c_str());

    if (xml_in.size() == 0) {
        log_error("XML data is empty!\n");
        return false;
    }

#ifndef USE_DMALLOC
    //dump_memory_stats(__FUNCTION__, __LINE__, "before xmlParseMemory");
#endif

    _bytes_total = _bytes_loaded = xml_in.size();
    
#ifdef USE_XMLREADER
    XMLNode *node = 0;
    xmlTextReaderPtr reader;

    reader = xmlReaderForMemory(xml_in.c_str(), xml_in.size(), NULL, NULL, 0);
    if (reader != NULL) {
	bool ret = true;
        while (ret) {
            ret = xmlTextReaderRead(reader);
            node = processNode(reader, node);
        }
        xmlFreeTextReader(reader);
        if (ret != false) {
            log_error("%s : couldn't parse\n", xml_in.c_str());
            return false;
        }
    } else {
        log_error("Unable to open %s\n", xml_in.c_str());
        return false;
    }
    xmlCleanupParser();
    return true;
#else

bool ret = true;

#ifdef USE_DOM
    xmlInitParser();
  
    _doc = xmlParseMemory(xml_in.c_str(), xml_in.size());
    if (_doc == 0) {
        log_error("Can't parse XML data!\n");
        return false;
    }
    ret = parseDoc(_doc, true);
    xmlCleanupParser();
    xmlFreeDoc(_doc);
    xmlMemoryDump();
#endif
#ifndef USE_DMALLOC
    //dump_memory_stats(__FUNCTION__, __LINE__, "after xmlParseMemory");
#endif
    return ret;
#endif
  
}

//     XML_READER_TYPE_NONE = 0
//     XML_READER_TYPE_ELEMENT = 1,
//     XML_READER_TYPE_ATTRIBUTE = 2,
//     XML_READER_TYPE_TEXT = 3,
//     XML_READER_TYPE_COMMENT = 8,
//     XML_READER_TYPE_SIGNIFICANT_WHITESPACE = 14,
//     XML_READER_TYPE_END_ELEMENT = 15,
//
// processNode:
// 2 1 IP 0// processNode:
// 3 3 #text 0 192.168.2.50
// processNode:
// 2 15 IP 0
// processNode:
// 2 14 #text 0
const char *tabs[] = {
    "",
    "\t",
    "\t\t",
    "\t\t\t",
    "\t\t\t",
    "\t\t\t\t",
};

#ifdef USE_XMLREADER
// This is an xmlReader (SAX) based parser. For some reason it core dumps
// when compiled with GCC 3.x, but works just fine with GCC 4.x.
XMLNode*
XML::processNode(xmlTextReaderPtr reader, XMLNode *node)
{
    log_msg("%s: node is %p\n", __PRETTY_FUNCTION__, node);
    static XMLNode *parent[10];
    xmlChar *name, *value;
    int   depth;
    XMLNode *element;
    //static int previous_depth = 0;
    xmlReaderTypes type;

    if (node == 0) {
        memset(parent, 0, sizeof(XMLNode *));
    }
    type = (xmlReaderTypes)xmlTextReaderNodeType(reader);
    depth = xmlTextReaderDepth(reader);
    value = xmlTextReaderValue(reader);
    name = xmlTextReaderName(reader);
  
    if (name == NULL)
        name = xmlStrdup(BAD_CAST "--");

#if 0
    printf("%d %d %s %d\n",
           depth,
           (int)type,
           name,
           xmlTextReaderIsEmptyElement(reader));  
#endif

  
    //child = node->_children[0];
    switch(xmlTextReaderNodeType(reader)) {
      case XML_READER_TYPE_NONE:
          break;
      case XML_READER_TYPE_SIGNIFICANT_WHITESPACE: // This is an empty text node
          //log_msg("\tWhitespace at depth %d\n", depth);
          break;
      case XML_READER_TYPE_END_ELEMENT:
          if (depth == 0) {          // This is the last node in the file
              element = node;
              break;
          }
          parent[depth]->_children.push_back(element);
//       log_msg("Pushing element %s on node %s\n", node->_name, parent[depth]->_name);
//       log_msg("End element at depth %d is %s for parent %s %p\n", depth, name,
//               parent[depth]->_name, parent[depth]);
          element = parent[depth];
          break;
      case XML_READER_TYPE_ELEMENT:
          element = new XMLNode;
//      log_msg("%sElement at depth %d is %s for node at %p\n", tabs[depth], depth, name, element);
          element->_name = (char *)new char[strlen(reinterpret_cast<const char *>(name))+1];
          memset(element->_name, 0, strlen(reinterpret_cast<const char *>(name))+1);
          strcpy(element->_name, reinterpret_cast<const char *>(name));
          if (node == 0) {
              _nodes = element;
              parent[0] = element;
          } else {
              parent[depth] = node;
              parent[depth+1] = node;
          }
          //  xmlTextReaderAttributeCount(reader);
          if (xmlTextReaderHasAttributes(reader)) {
              // log_msg("Has Attributes!\n");
              xmlTextReaderMoveToFirstAttribute(reader);
              processNode(reader, element);
              while(xmlTextReaderMoveToNextAttribute(reader)) {
                  processNode(reader, element);
              }
          }
          break;
      case XML_READER_TYPE_TEXT:
          element = node;
//      log_msg("%sValue at depth %d is \"%s\" for node at %p\n", tabs[depth], depth, value, element);
          element->_value = (char *)new char[strlen(reinterpret_cast<const char *>(value))+1];
          memset(element->_value, 0, strlen(reinterpret_cast<const char *>(value))+1);
          strcpy(element->_value, reinterpret_cast<const char *>(value));
          break;
      case XML_READER_TYPE_ATTRIBUTE:
          element = node;
          XMLAttr *attrib = new XMLAttr;
          attrib->_name = (char *)new char[strlen(reinterpret_cast<const char *>(name))+1];
          memset(attrib->_name, 0, strlen(reinterpret_cast<const char *>(name))+1);
          strcpy(attrib->_name, reinterpret_cast<const char *>(name));
          attrib->_value = (char *)new char[strlen(reinterpret_cast<const char *>(value))+1];
          memset(attrib->_value, 0, strlen(reinterpret_cast<const char *>(value))+1);
          strcpy(attrib->_value, reinterpret_cast<const char *>(value));
//     log_msg("%sPushing attribute %s, value \"%s\" for node %s\n", tabs[depth], name, value, element->_name);
          element->_attributes.push_back(attrib);
          break;
      default:   // FIXME: why does this break GCC 3.3.3 but not 3.4.3 ?
          log_error("Unsupported XML type %d\n!", type);
          break;
    };

    xmlFree(name);
    if (value != NULL) {
        xmlFree(value);
    }
    //previous_depth = depth;
    return element;
}
#endif

// This reads in an XML file from disk and parses into into a memory resident
// tree which can be walked through later.
bool
XML::load(const char *filespec)
{
    GNASH_REPORT_FUNCTION;
    struct stat stats;
    log_msg("Load disk XML file: %s\n", filespec);
  
    //log_msg("%s: mem is %d\n", __FUNCTION__, mem);

    // See if the file exists
    if (stat(filespec, &stats) == 0) {
        _bytes_total = stats.st_size;
        _bytes_loaded = stats.st_size; // FIXME: this should probably
                                       // be set later on after the
                                       // file is loaded
    }
#ifdef USE_XMLREADER
    XMLNode *node = 0;
    xmlTextReaderPtr reader;  
  
    reader = xmlNewTextReaderFilename(filespec);
    if (reader != NULL) {
        bool ret = true;
        while (ret) {
            ret = xmlTextReaderRead(reader);
            node = processNode(reader, node);
        }
        xmlFreeTextReader(reader);
        if (ret != false) {
            log_error("%s : couldn't parse\n", filespec);
            return false;
        }
    } else {
        log_error("Unable to open %s\n", filespec);
        return false;
    }
    xmlCleanupParser();
    return true;
#else
#ifdef USE_DOM
    xmlInitParser();
    _doc = xmlParseFile(filespec);
    if (_doc == 0) {
        log_error("Can't load XML file: %s!\n", filespec);
        return false;
    }
    parseDoc(_doc, false);
    xmlCleanupParser();
    xmlFreeDoc(_doc);
    xmlMemoryDump();
    return true;
#else
#error "You have to enable either a DOM or an xmlReader XML parser"
#endif
#endif
}


vector<XMLNode *>
XML::childNodes() 
{
  if (_nodes) {
    return _nodes->_children;
  } else {
    return static_cast< vector<XMLNode*> >(0);
  }
}

bool
XML::onLoad()
{
    log_msg("%s: FIXME: onLoad Default event handler\n", __FUNCTION__);

    return(_loaded);
}

XMLNode *
XML::operator [] (int x) {
    log_msg("%s:\n", __FUNCTION__);

    return _nodes->_children[x];
}

void
XML::cleanupStackFrames(XMLNode */* xml */)
{
    GNASH_REPORT_FUNCTION;
}

as_object *
XML::setupFrame(as_object *obj, XMLNode *xml, bool mem)
{
//    GNASH_REPORT_FUNCTION;
    
    int		 child;
    unsigned int i;
    const char    *nodename;
    //const char    *nodevalue;
    //AS_value      nodevalue;
    int           length;
    as_value      inum;
    XMLNode       *childnode;
    XMLNode *xmlchildnode_obj;
    xmlattr_as_object* attr_obj;

    //log_msg("\t%s: processing node %s for object %p, mem is %d\n", __FUNCTION__, xml->_name, obj, mem);
  
    // Get the data for this node
    nodename   = xml->_name;
    //nodename   = xml->_name.c_str();
    //nodevalue  = xml->_value;
    length     = xml->length();

    // Set these members in the top level object passed in. This are used
    // primarily by the disk based XML parser, where at least in all my current
    // test cases this is referenced with firstChild first, then nodeName and
    // childNodes.
    //obj->set_member("nodeName", nodename); // use the getter/setter !
    obj->set_member("length", length); // FIXME: use a getter/setter !
    if (xml->_value != 0) {
        //obj->set_member("nodeValue", xml->_value); // use the getter/setter !
        log_msg("\tnodevalue for %s is: %s\n", nodename, xml->_value);
    } else {
        // obj->set_member("nodeValue", as_value::UNDEFINED); // use the getter/setter !
    }

    // Process the attributes, if any
    if (xml->_attributes.size() == 0) {
        //log_msg("\t\tNo attributes for node %s, created empty object at %p\n", nodename, attr_obj);
//     log_msg("\t\tNo attributes for node %s\n", nodename);
    } else {
        attr_obj = new xmlattr_as_object;
        for (i=0; i<xml->_attributes.size(); i++) {
            attr_obj->set_member(xml->_attributes[i]->_name, xml->_attributes[i]->_value);
	    log_msg("\t\tAdding attribute as member %s, value is %s to node %s (%p)\n",
		    xml->_attributes[i]->_name,
		    xml->_attributes[i]->_value, nodename, static_cast<void*>(obj) );
        }
        obj->set_member("attributes", attr_obj);
    }

    //xml->_attributes.resize(0);
    //obj->set_member("attributes", attr_obj);

    // Process the children, if there are any
    if (length) {
        //log_msg("\tProcessing %d children nodes for %s\n", length, nodename);
        inum = 0;
        for (child=0; child<length; child++) {
            // Create a new AS object for this node's children
            xmlchildnode_obj = new XMLNode;
            // When parsing XML from memory, the test movies I have expect the firstChild
            // to be the first element of the array instead.
            if (mem) {
                childnode = xml;
                //obj->set_member(inum.to_string(), obj);
                //inum += 1;
                //childnode = xml->_children[child];
            } else {
                childnode = xml->_children[child];
            }
            setupFrame(xmlchildnode_obj, childnode, false); // setup child node
            obj->set_member(inum.to_string(), xmlchildnode_obj);
            inum += 1;
        }
    } else {
        //log_msg("\tNode %s has no children\n", nodename);
    }  

    return obj;
}


/// \brief add or change the HTTP Request header
///
/// Method; adds or changes HTTP request headers (such as Content-Type
/// or SOAPAction) sent with POST actions. In the first usage, you pass
/// two strings to the method: headerName and headerValue. In the
/// second usage, you pass an array of strings, alternating header
/// names and header values.
///
/// If multiple calls are made to set the same header name, each
/// successive value replaces the value set in the previous call.
void
XML::addRequestHeader(const char */* name */, const char */* value */)
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

/// \brief append a node the the XML object
///
/// Method; appends the specified node to the XML object's child
/// list. This method operates directly on the node referenced by the
/// childNode parameter; it does not append a copy of the node. If the
/// node to be appended already exists in another tree structure,
/// appending the node to the new location will remove it from its
/// current location. If the childNode parameter refers to a node that
/// already exists in another XML tree structure, the appended child
/// node is placed in the new tree structure after it is removed from
/// its existing parent node. 
void
XML::appendChild(XMLNode *node)
{
    if (!_nodes) {
      _nodes = new XMLNode;
    }
    _nodes->_children.push_back(node);
    //    log_msg("%s: %p at _nodes at %p\n", __PRETTY_FUNCTION__, this, _nodes);
}

/// \brief copy a node
///
/// Method; constructs and returns a new XML node of the same type,
/// name, value, and attributes as the specified XML object. If deep
/// is set to true, all child nodes are recursively cloned, resulting
/// in an exact copy of the original object's document tree. 
XMLNode &
XML::cloneNode(XMLNode &newnode, bool deep)
{
    log_msg("%s: deep is %d\n", __PRETTY_FUNCTION__, deep);

    if (_nodes && deep) {
	newnode = _nodes;
//     } else {
// 	newnode.nodeNameSet((char *)_nodes->nodeName());
// 	newnode.nodeValueSet((char *)_nodes->nodeValue());    
    }

    return newnode;
  
    log_msg("%s:partially unimplemented \n", __PRETTY_FUNCTION__);
}

/// \brief create a new XML element
///
/// Method; creates a new XML element with the name specified in the
/// parameter. The new element initially has no parent, no children,
/// and no siblings. The method returns a reference to the newly
/// created XML object that represents the element. This method and
/// the XML.createTextNode() method are the constructor methods for
/// creating nodes for an XML object. 
XMLNode *
XML::createElement(const char */* name */)
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
    return (XMLNode*)0;
}

/// \brief Create a new XML node
/// 
/// Method; creates a new XML text node with the specified text. The
/// new node initially has no parent, and text nodes cannot have
/// children or siblings. This method returns a reference to the XML
/// object that represents the new text node. This method and the
/// XML.createElement() method are the constructor methods for
/// creating nodes for an XML object.
XMLNode *
XML::createTextNode(const char */* name */)
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
    return (XMLNode*)0;
}

/// \brief insert a node before a node
///
/// Method; inserts a new child node into the XML object's child
/// list, before the beforeNode node. If the beforeNode parameter is
/// undefined or null, the node is added using the appendChild()
/// method. If beforeNode is not a child of my_xml, the insertion
/// fails.
void
XML::insertBefore(XMLNode */* newnode */, XMLNode */* node */)
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
XML::load()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
XML::parseXML()
{
    log_msg("%s: unimplemented \n", __FUNCTION__);
}

/// \brief removes the specified XML object from its parent. Also
/// deletes all descendants of the node.
void
XML::removeNode()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
XML::send()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
XML::sendAndLoad()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

const char *
XML::toString()
{
    if (_nodes) {
	return stringify(_nodes);
    }
    return NULL;
}

// 
const char *
XML::stringify(XMLNode *xml)
{
    int           child;
    unsigned int  i;
    const char    *nodename = xml->nodeName();
    int           length;
    string	  str;
    
    log_msg("%s: processing for object %p\n", __PRETTY_FUNCTION__, (void*)this);
  
    // Process the attributes, if any
    if (_nodes->_attributes.size() == 0) {
        log_msg("\t\tNo attributes for node\n");
    } else {
        for (i=0; i<xml->_attributes.size(); i++) {
            log_msg("\t\tAdding attribute as member %s, value is %s to node %s\n",
                    xml->_attributes[i]->_name,
                    xml->_attributes[i]->_value, nodename);
        }
    }

    vector<XMLNode *>::iterator it;
    for (it = _nodes->_children.begin(); it != _nodes->_children.end(); ++it) {
	log_msg("Found One!!!! %p\n", (void*)*it);
    }
    
    // Process the children, if there are any
    length = xml->_children.size();

    if (length) {
        log_msg("\tProcessing %d children nodes\n", length);
        for (child=0; child<length; child++) {
	    log_msg("Name %p\n", (void*)(xml->_children[child]));
	    if (xml->_children[child]->_name) {
//		log_msg("Name %p", xml->_children[child]);
	    }
	    if (xml->_children[child]->_value) {
//		log_msg("Value %s", xml->_children[child]->_value);
	    }
	
//	    str += stringify(xml->_children[child]);
        }
    } else {
        log_msg("\tNode %s has no children\n", nodename);
    }
    return str.c_str();
}

//
// Callbacks. These are the wrappers for the C++ functions so they'll work as
// callbacks from within gnash.
//
void
xml_load(const fn_call& fn)
{
    as_value	method;
    as_value	val;
    bool          ret;
    struct stat   stats;

    //GNASH_REPORT_FUNCTION;
  
    XML *xml_obj = (XML*)fn.this_ptr;
  
    std::string filespec = fn.arg(0).to_string(); 

    // If the file doesn't exist, don't try to do anything.
    if (stat(filespec.c_str(), &stats) < 0) {
        fprintf(stderr, "ERROR: doesn't exist.%s\n", filespec.c_str());
        fn.result->set_bool(false);
        return;
    }
  
    // Set the argument to the function event handler based on whether the load
    // was successful or failed.
    ret = xml_obj->load(filespec.c_str());
    fn.result->set_bool(ret);

    if (ret == false) {
        return;
    }
    
    //env->bottom(first_arg) = ret;
    //  struct node *first_node = ptr->firstChildGet();
  
    //const char *name = ptr->nodeNameGet();

    if (xml_obj->hasChildNodes() == false) {
        log_error("%s: No child nodes!\n", __FUNCTION__);
    }  
    xml_obj->setupFrame(xml_obj, xml_obj->firstChild(), false);
  
#if 1
    if (fn.this_ptr->get_member("onLoad", &method)) {
        //    log_msg("FIXME: Found onLoad!\n");
        fn.env->set_variable("success", true);
        fn.arg(0) = true;
        as_c_function_ptr	func = method.to_c_function();
        if (func) {
	    // It's a C function.  Call it.
	    log_msg("Calling C function for onLoad\n");
	    (*func)(fn_call(&val, xml_obj, fn.env, fn.nargs, fn.first_arg_bottom_index)); // was this_ptr instead of node
	} else if (as_function* as_func = method.to_as_function()) {
	    // It's an ActionScript function.  Call it.
	    log_msg("Calling ActionScript function for onLoad\n");
	    (*as_func)(fn_call(&val, xml_obj, fn.env, fn.nargs, fn.first_arg_bottom_index)); // was this_ptr instead of node
	} else {
	    log_error("error in call_method(): method is not a function\n");
	}
    } else {
        log_msg("Couldn't find onLoad event handler, setting up callback\n");
        // ptr->set_event_handler(event_id::XML_LOAD, (as_c_function_ptr)&xml_onload);
    }
#else
    xml_obj->set_event_handler(event_id::XML_LOAD, &xml_onload);

#endif

    fn.result->set_bool(true);
}

// This executes the event handler for XML::XML_LOAD if it's been defined,
// and the XML file has loaded sucessfully.
void
xml_onload(const fn_call& fn)
{
    //log_msg("%s:\n", __FUNCTION__);
    
    as_value	method;
    as_value      val;
    static bool first = true;     // This event handler should only be executed once.
    XML*	ptr = (XML*) (as_object*) fn.this_ptr;
    assert(ptr);
  
    if ((ptr->loaded()) && (first)) {
        // env->set_variable("success", true, 0);
        //as_value bo(true);
        //env->push_val(bo);

        first = false;
        log_msg("The XML file has been loaded successfully!\n");
        // ptr->on_event(event_id::XML_LOAD);
        //env->set_variable("success", true, 0);
        //env->bottom(0) = true;
    
        if (fn.this_ptr->get_member("onLoad", &method)) {
            // log_msg("FIXME: Found onLoad!\n");
            as_c_function_ptr	func = method.to_c_function();
            if (func)
                {
                    // It's a C function.  Call it.
                    log_msg("Calling C function for onLoad\n");
                    (*func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
                }
            else if (as_function* as_func = method.to_as_function())
                {
                    // It's an ActionScript function.  Call it.
                    log_msg("Calling ActionScript function for onLoad\n");
                    (*as_func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
                }
            else
                {
                    log_error("error in call_method(): method is not a function\n");
                }    
        } else {
            log_msg("FIXME: Couldn't find onLoad!\n");
        }
    }
      
    fn.result->set_bool(val.to_bool());
}

// This is the default event handler, and is usually redefined in the SWF script
void
xml_ondata(const fn_call& fn)
{
    log_msg("%s:\n", __FUNCTION__);
    
    as_value	method;
    as_value	val;
    static bool first = true;     // FIXME: ugly hack!
  
    XML*	ptr = (XML*)fn.this_ptr;
    assert(ptr);
  
    if ((ptr->loaded()) && (first)) {
        if (fn.this_ptr->get_member("onData", &method)) {
            log_msg("FIXME: Found onData!\n");
            as_c_function_ptr	func = method.to_c_function();
            fn.env->set_variable("success", true);
            if (func)
                {
                    // It's a C function.  Call it.
                    log_msg("Calling C function for onData\n");
                    (*func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
                }
            else if (as_function* as_func = method.to_as_function())
                {
                    // It's an ActionScript function.  Call it.
                    log_msg("Calling ActionScript function for onData\n");
                    (*as_func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
                }
            else
                {
                    log_error("error in call_method(): method is not a function\n");
                }    
        } else {
            log_msg("FIXME: Couldn't find onData!\n");
        }
    }

    //fn.result->set(&val);
    fn.result->set_bool(val.to_bool());
}

void
attachXMLInterface(as_object& o)
{
	// FIXME: this doesn't appear to exist in the MM player, should it ?
	o.set_member("loaded", &xml_loaded);
	
        o.set_member("addRequestHeader", &xml_addrequestheader);
        o.set_member("appendChild", &xml_appendchild);
        o.set_member("cloneNode", &xml_clonenode);
        o.set_member("createElement", &xml_createelement);
        o.set_member("createTextNode", &xml_createtextnode);
        o.set_member("getBytesLoaded", &xml_getbytesloaded);
        o.set_member("getBytesTotal", &xml_getbytestotal);
        o.set_member("hasChildNodes", &xml_haschildnodes);
        o.set_member("insertBefore", &xml_insertbefore);
        o.set_member("load", &xml_load);
        o.set_member("parseXML", &xml_parsexml);
        o.set_member("removeNode", &xml_removenode);
        o.set_member("send", &xml_send);
        o.set_member("sendAndLoad", &xml_sendandload);
        o.set_member("toString", &xml_tostring);

	// Properties

	boost::intrusive_ptr<builtin_function> gettersetter;

	gettersetter = new builtin_function(&xml_nodename, NULL);
	o.add_property("nodeName", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&xml_nodevalue, NULL);
	o.add_property("nodeValue", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&xml_firstchild, NULL);
	o.add_property("firstChild", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&xml_childnodes, NULL);
	o.add_property("childNodes", *gettersetter, *gettersetter);
}

static as_object*
getXMLInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( o == NULL )
	{
		o = new as_object();
		attachXMLInterface(*o);
	}
	return o.get();
}

void
xml_new(const fn_call& fn)
{
    as_value      inum;
    XML *xml_obj;
    //const char    *data;
  
    // log_msg("%s: nargs=%d\n", __FUNCTION__, fn.nargs);
  
    if (fn.nargs > 0) {
	as_object* obj = fn.env->top(0).to_object();

        if (! obj ) {
            xml_obj = new XML;
            //log_msg("\tCreated New XML object at %p\n", xml_obj);
            tu_string datain = fn.env->top(0).to_tu_string();
            xml_obj->parseXML(datain);
            //log_msg("*** Start setting up the stack frames ***\n");
            xml_obj->setupFrame(xml_obj, xml_obj->firstChild(), true);
            //xml_obj->clear();
            //delete xml_obj->firstChild();
        } else {
	    assert(dynamic_cast<XML*>(obj));
            XML*	xml_obj = (XML*)obj;
            //log_msg("\tCloned the XML object at %p\n", xml_obj);
            //result->set(xml_obj);
            fn.result->set_as_object(xml_obj);
            return;
        }
    } else {
        xml_obj = new XML;
        //log_msg("\tCreated New XML object at %p\n", xml_obj);
    }

    fn.result->set_as_object(xml_obj);
}

//
// SWF Property of this class. These are "accessors" into the private data
// of the class.
//

// determines whether the document-loading process initiated by the XML.load()
// call has completed. If the process completes successfully, the method
// returns true; otherwise, it returns false.
void
xml_loaded(const fn_call& fn)
{
    as_value	method;
    as_value	val;

    log_msg("%s:\n", __FUNCTION__);
    
    XML*	ptr = (XML*) (as_object*) fn.this_ptr;
    assert(ptr);
    std::string filespec = fn.arg(0).to_string();
    //fn.result->set(ptr->loaded());
    fn.result->set_bool(ptr->loaded());
}


void xml_addrequestheader(const fn_call& fn)
{
    log_msg("%s: %d args\n", __PRETTY_FUNCTION__, fn.nargs);
    XML *ptr = (XML*)fn.this_ptr;
    assert(ptr);
    
//    fn.result->set_int(ptr->getAllocated());
//    ptr->addRequestHeader();
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void xml_appendchild(const fn_call& fn)
{
  //    log_msg("%s: %d args\n", __PRETTY_FUNCTION__, fn.nargs);
    XML *ptr = (XML*)fn.this_ptr;
    assert(ptr);
    XMLNode *xml_obj = (XMLNode*)fn.env->top(0).to_object();
    
    ptr->appendChild(xml_obj);
}

void xml_clonenode(const fn_call& fn)
{
    log_msg("%s: %d args\n", __PRETTY_FUNCTION__, fn.nargs);
    XML	*ptr = (XML*)fn.this_ptr;
    XMLNode   *xml_obj;
    assert(ptr);

    if (fn.nargs > 0) {
      bool deep = fn.arg(0).to_bool(); 
      xml_obj = new XMLNode;
      ptr->cloneNode(*xml_obj, deep);
      fn.result->set_as_object(xml_obj);
   } else {
        log_msg("ERROR: no Depth paramater!\n");
    }

}
void xml_createelement(const fn_call& fn)
{
  //    log_msg("%s: %d args\n", __PRETTY_FUNCTION__, fn.nargs);
    XML *ptr = (XML*)fn.this_ptr;
    assert(ptr);
    XMLNode *xml_obj;
    const char *text;

    if (fn.nargs > 0) {
        text = fn.arg(0).to_string(); 
	xml_obj = new XMLNode;
	xml_obj->nodeNameSet(text);
	xml_obj->nodeTypeSet(XML_ELEMENT_NODE); 
	fn.result->set_as_object(xml_obj);
   } else {
        log_msg("ERROR: no text for element creation!\n");
    }
}


void xml_createtextnode(const fn_call& fn)
{
	log_msg("%s: %d args\n", __PRETTY_FUNCTION__, fn.nargs);

	//assert(dynamic_cast<XML*>(fn.this_ptr));
	//XML *ptr = static_cast<XML*>(fn.this_ptr);

	XMLNode *xml_obj;
	const char *text;

	if (fn.nargs > 0)
	{
		text = fn.arg(0).to_string(); 
		xml_obj = new XMLNode;
		xml_obj->nodeValueSet(text);
		xml_obj->nodeTypeSet(XML_TEXT_NODE);
		fn.result->set_as_object(xml_obj);
//	log_msg("%s: xml obj is %p\n", __PRETTY_FUNCTION__, xml_obj);
    } else {
		log_msg("ERROR: no text for text node creation!\n");
    }
}

void xml_getbytesloaded(const fn_call& fn)
{
    XML *ptr = (XML*)fn.this_ptr;
    assert(ptr);
    fn.result->set_int(ptr->getBytesLoaded());
}

void xml_getbytestotal(const fn_call& fn)
{
    XML *ptr = (XML*)fn.this_ptr;
    assert(ptr);
    fn.result->set_int(ptr->getBytesTotal());
}

void xml_haschildnodes(const fn_call& fn)
{
    XML *ptr = (XML*)fn.this_ptr;
    assert(ptr);
    fn.result->set_bool(ptr->hasChildNodes());
}
void xml_insertbefore(const fn_call& fn)
{
    XML *ptr = (XML*)fn.this_ptr;
    assert(ptr);
    
//    fn.result->set_int(ptr->getAllocated());
//    ptr->insertBefore();
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void xml_parsexml(const fn_call& fn)
{
    const char *text;
    as_value	method;
    as_value	val;    
    XML *ptr = (XML*)fn.this_ptr;
    assert(ptr);

    if (fn.nargs > 0) {
        text = fn.arg(0).to_string(); 
	if (ptr->parseXML(text)) {
	    ptr->setupFrame(ptr, ptr->firstChild(), false);  
	}
    }
    
#if 1
    if (fn.this_ptr->get_member("onLoad", &method)) {
        log_msg("FIXME: Found onLoad!\n");
        fn.env->set_variable("success", true);
	fn.arg(0) = true;
        as_c_function_ptr	func = method.to_c_function();
        if (func) {
	    // It's a C function.  Call it.
	    log_msg("Calling C function for onLoad\n");
	    (*func)(fn_call(&val, ptr, fn.env, fn.nargs, fn.first_arg_bottom_index)); // was this_ptr instead of node
	} else if (as_function* as_func = method.to_as_function()) {
	    // It's an ActionScript function.  Call it.
	    log_msg("Calling ActionScript function for onLoad\n");
	    (*as_func)(fn_call(&val, ptr, fn.env, fn.nargs, fn.first_arg_bottom_index)); // was this_ptr instead of node
	} else {
	    log_error("error in call_method(): method is not a function\n");
	}
    } else {
        log_msg("Couldn't find onLoad event handler, setting up callback\n");
        // ptr->set_event_handler(event_id::XML_LOAD, (as_c_function_ptr)&xml_onload);
    }
#else
    
#endif
//    fn.result->set_int(ptr->getAllocated());
}
    
void xml_removenode(const fn_call& fn)
{
    XML *ptr = (XML*)fn.this_ptr;
    assert(ptr);
    
//    fn.result->set_int(ptr->getAllocated());
    ptr->removeNode();
}
void xml_send(const fn_call& fn)
{
    XML *ptr = (XML*)fn.this_ptr;
    assert(ptr);
    
//    fn.result->set_int(ptr->getAllocated());
    ptr->send();
}
void xml_sendandload(const fn_call& fn)
{
    XML *ptr = (XML*)fn.this_ptr;
    assert(ptr);
    
//    fn.result->set_int(ptr->getAllocated());
    ptr->sendAndLoad();
}
void xml_tostring(const fn_call& fn)
{
    XML *ptr = (XML*)fn.this_ptr;
    assert(ptr);
    
    fn.result->set_string(ptr->toString());
}

// Both a getter and a setter for nodeName
static void
xml_nodename(const fn_call& fn)
{
	assert(dynamic_cast<XML*>(fn.this_ptr));
	XML *ptr = static_cast<XML*>(fn.this_ptr);

	if ( fn.nargs == 0 ) {
		const char* val = ptr->nodeName();
		if ( val ) {
			fn.result->set_string(val);
		} else {
			fn.result->set_null();
		}
	} else {
		ptr->nodeNameSet(fn.arg(0).to_string());
	}
}

// Both a getter and a setter for nodeValue
static void
xml_nodevalue(const fn_call& fn)
{
	//GNASH_REPORT_FUNCTION;

	assert(dynamic_cast<XML*>(fn.this_ptr));
	XML *ptr = static_cast<XML*>(fn.this_ptr);
    
	//log_msg("xml_nodevalue called with %d args against 'this' = %p", fn.nargs, ptr);
	if ( fn.nargs == 0 ) {
		//log_msg("  nodeValue() returns '%s'", ptr->nodeValue());
		const char* val = ptr->nodeValue();
		if ( val ) {
			fn.result->set_string(val);
		} else {
			fn.result->set_null();
		}
	} else {
		//log_msg(" arg(0) == '%s'", fn.arg(0).to_string());
		ptr->nodeValueSet(fn.arg(0).to_string());
	}
}

// Both a getter and a (do-nothing) setter for firstChild
static void
xml_firstchild(const fn_call& fn)
{
	assert(dynamic_cast<XML*>(fn.this_ptr));
	XML *ptr = static_cast<XML*>(fn.this_ptr);

	if ( fn.nargs == 0 )
	{
		//fn.result->set_as_object(ptr->firstChild());
		fn.result->set_as_object(ptr);
	}
	else
	{
		IF_VERBOSE_ASCODING_ERRORS(
			log_aserror("Tried to set read-only property XML.firstChild");
		);
	}
}

// Both a getter and a (do-nothing) setter for childNodes
static void
xml_childnodes(const fn_call& fn)
{
	assert(dynamic_cast<XML*>(fn.this_ptr));
	XML *ptr = static_cast<XML*>(fn.this_ptr);

	if ( fn.nargs == 0 )
	{
		//fn.result->set_as_object(ptr->childNodes());
		fn.result->set_as_object(ptr);
	}
	else
	{
		IF_VERBOSE_ASCODING_ERRORS(
			log_aserror("Tried to set read-only property XML.childNodes");
		);
	}
}

int
memadjust(int x)
{
    return (x + (4 - x % 4));
}

// extern (used by Global.cpp)
void xml_class_init(as_object& global)
{
	// This is going to be the global XML "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&xml_new, getXMLInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachXMLInterface(*cl);
		     
	}

	// Register _global.String
	global.set_member("XML", cl.get());

}

} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
