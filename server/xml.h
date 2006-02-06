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

#ifndef __XML_H__
#define __XML_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#define DEBUG_MEMORY_ALLOCATION 1

#include "tu_config.h"
#include "log.h"
#include "action.h"
#include "impl.h"
#include "log.h"

#ifdef HAVE_LIBXML

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>

namespace gnash {
  
/// XML Attribute class
class XMLAttr {
public:
    XMLAttr();
    ~XMLAttr();
  
    char                *_name;
    char                *_value;
    xmlAttributeType    _type;
    
    XMLAttr *operator = (XMLAttr node) {
        log_msg("\t\tCopying XMLAttr object at %p\n", this);
    
        _name = new char[strlen(node._name)+2];
        memset(_name, 0, strlen(node._name)+2);
        strcpy(_name, node._name);

        _value = new char[strlen(node._value)+2];
        memset(_value, 0, strlen(node._value)+2);
        strcpy(_value, node._value);

        return this;
    }
};

/// XML Attribute ActionScript Object
struct xmlattr_as_object : public as_object
{
    //XMLAttr obj;
    int   padding;
#ifdef DEBUG_MEMORY_ALLOCATION
    xmlattr_as_object() {
        log_msg("\t\tCreating xmlattr_as_object at %p\n", this);
    };
    ~xmlattr_as_object() {
        log_msg("\tDeleting xmlattr_as_object at %p \n", this);
    };
#endif
};
 
/// XML Node 
class XMLNode
{
public:
    XMLNode();
    ~XMLNode();

    int length()                 { return _children.size(); }
    const char *nodeName()       { return _name; }
    
    void nodeNameSet(char *name) { _name = name; }
    char *valueGet()             { return _value; }
    void valueSet(char *value)   { _value = value; }

    const char *nodeValue()    { return _value; }
  
    //  nodeType 	XML.nodeType

    bool hasChildNodes() {
        if (_children.size() > 0) {
            return true;
        }
        return false;
    }
  
    XMLNode *firstChild() {
        return _children[0];
    }
  
    array<XMLNode *>childNodes() {
        return _children;
    }  
    
    XMLNode *operator [] (int x) {
        gnash::log_msg("%s: get element %d\n", __FUNCTION__, x);
        
        return _children[x];
    }
    
    XMLNode *operator = (XMLNode &node) {
        _name = node._name;
        _value = node._value;
        _children = node._children;
        _attributes = node._attributes;
        return this;
    }
    
    XMLNode *operator = (XMLNode *node) {
        _name = node->_name;
        _value = node->_value;
        _children = node->_children;
        _attributes = node->_attributes;
        return this;
    }
    
    void appendChild(XMLNode *node) {
        node->_children.push_back(node);
    }
    
    void  change_stack_frame(int frame, gnash::as_object *xml, gnash::as_environment *env);

    char                *_name;
    char                *_value;
    xmlElementType      _type;
    array<XMLNode *>    _children;
    array<XMLAttr *>    _attributes;
};

/// XML Node ActionScript object
struct xmlnode_as_object : public gnash::as_object
{
    XMLNode obj;
    int                _padding;

#ifdef DEBUG_MEMORY_ALLOCATION
    xmlnode_as_object() {
        log_msg("\tCreating xmlnode_as_object at %p \n", this);
    };
    ~xmlnode_as_object() {
        log_msg("\tDeleting xmlnode_as_object at %p \n", this);
    };
#endif
    virtual bool get_member(const tu_stringi& name, as_value* val) {
        //printf("GET XMLNode MEMBER: %s at %p for object %p\n", name.c_str(), val, this);
        
        if ((name == "firstChild") || (name == "childNodes")) {
            //printf("Returning a self reference for %s for object at %p\n", name.c_str(), this);
            val->set_as_object_interface(this);
            return true;
        }
        
#if 1
        printf("%s(%d): ERROR: as_member::get() unimplemented!", __PRETTY_FUNCTION__, __LINE__);
#else
        if (m_members.get(name, val) == false) {
            if (m_prototype != NULL) {
                return m_prototype->get_member(name, val);
            }
            return false;
        }
#endif
        return true;
    }
};

/// XML class
class XML {
public:
    XML();
    XML(tu_string xml_in);
    XML(struct node * childNode);
    virtual ~XML();

    // Methods
    // This is the base method used by both parseXML() and load().
    bool parseDoc(xmlDocPtr document, bool mem);
    // Parses an XML document into the specified XML object tree.
    bool parseXML(tu_string xml_in);
    // Loads a document (specified by
    bool load(const char *filespec);
    // the XML object) from a URL.

    // An event handler that returns a
    bool onLoad();
    // Boolean value indicating whether
    // the XML object was successfully
    // loaded with XML.load() or
    // XML.sendAndLoad().

    // Appends a node to the end of the specified object's child list.
    void appendChild(XMLNode *node);
    
    virtual bool on_event(gnash::event_id id);
    virtual void	on_event_load();
    bool loaded()    { return _loaded; }
    
    XMLNode *firstChild() {
        return _nodes;
        //return _node_data[0];
    }
    
    void clear() {
        delete _nodes;
    }
    
    array<XMLNode *> childNodes() {
        return _nodes->_children;
    }

    const char *stringify(XMLNode *xml);
    //  Returns true if the specified node has child nodes; otherwise, returns false.
    bool hasChildNodes() {
        return _nodes->_children.size();
    }
    
    XMLNode *extractNode(xmlNodePtr node, bool mem);
    XMLNode *processNode(xmlTextReaderPtr reader, XMLNode *node);

    void  change_stack_frame(int frame, gnash::as_object *xml, gnash::as_environment *env);
//    void  setupStackFrames(gnash::as_object *xml, gnash::as_environment *env);
    void  cleanupStackFrames( XMLNode *data);
    as_object *setupFrame(gnash::as_object *xml, XMLNode *data, bool src);
  
    const char *nodeNameGet()    { return _nodename; }
    void nodeNameSet(const char *name)    { _nodename = name; }
  
    int length()                 { return _nodes->length(); }
  
    // These 6 have to 
    void addRequestHeader(const char *name, const char *value);
    XMLNode *cloneNode(bool deep);
    XMLNode *createElement(const char *name);
    XMLNode *createTextNode(const char *name);
    void insertBefore(XMLNode *newnode, XMLNode *node);

    void load();
    void parseXML();
    void removeNode();
    void send();
    void sendAndLoad();
    const char *toString();

    int getBytesLoaded()         { return _bytes_loaded; };
    int getBytesTotal()          { return _bytes_total; };

    virtual void	on_xml_event(gnash::event_id id) { on_event(id); }
  
    // Special event handler; 
    void	on_event_close() { on_event(gnash::event_id::SOCK_CLOSE); }
  
    XMLNode *operator [] (int x);
#if 0
    XMLNode *operator = (XMLNode &node) {
        gnash::log_msg("%s: copy element %s\n", __FUNCTION__, node._name.c_str());
        _nodes = node;
    }

#endif
    XML *operator = (XMLNode *node) {
        _nodes = node;    
        return this;
    }
    
private:
    xmlDocPtr _doc;
    xmlNodePtr _firstChild;
    
    // Properties
    bool _loaded;
    const char  *_nodename;
    XMLNode     *_nodes;

    int         _bytes_loaded;
    int         _bytes_total;
    
    bool        _contentType;
    bool        _attributes;
    bool        _childNodes;
    bool        _xmlDecl;
    bool        _docTypeDecl;
    bool        _ignoreWhite;
    bool        _lastChild;
    bool        _nextSibling;
    bool        _nodeType;
    bool        _nodeValue;
    bool        _parentNode;
    bool        _status;
    bool        _previousSibling;

};

/// XML ActionScript object
struct xml_as_object : public gnash::as_object
{
    XML obj;
#ifdef DEBUG_MEMORY_ALLOCATION
    xml_as_object() {
        log_msg("\tCreating xml_as_object at %p\n", this);
    };
    ~xml_as_object() {
        log_msg("\tDeleting xml_as_object at %p\n", this);
    };
#endif

    virtual bool get_member(const tu_stringi& name, as_value* val) {
        //printf("GET XML MEMBER: %s at %p for object %p\n", name.c_str(), val, this);
        
        if ((name == "firstChild") || (name == "childNodes")) {
//             printf("Returning a self reference for %s for object at %p\n",
//                    name.c_str(), this);
            val->set_as_object_interface(this);
            return true;
        }
        
#if 0
        printf("%s(%s:%d): ERROR: as_member::get() unimplemented!",
               __PRETTY_FUNCTION__, __FILE__, __LINE__);
#else
        as_member m;
        if (m_members.get(name, &m) == false) {
            if (m_prototype != NULL) {
                return m_prototype->get_member(name, val);
            }
            return false;
        } else {
            *val=m.get_member_value();
            return true;
        }
#endif
        return true;
    }
};

void xml_load(const fn_call& fn);
void xml_set_current(const fn_call& fn);
void xml_new(const fn_call& fn);

void xml_addrequestheader(const fn_call& fn);
void xml_appendchild(const fn_call& fn);
void xml_clonenode(const fn_call& fn);
void xml_createelement(const fn_call& fn);
void xml_createtextnode(const fn_call& fn);
void xml_getbytesloaded(const fn_call& fn);
void xml_getbytestotal(const fn_call& fn);
void xml_haschildnodes(const fn_call& fn);
void xml_insertbefore(const fn_call& fn);
void xml_parsexml(const fn_call& fn);
void xml_removenode(const fn_call& fn);
void xml_send(const fn_call& fn);
void xml_sendandload(const fn_call& fn);
void xml_tostring(const fn_call& fn);

// These are the event handlers called for this object
void xml_onload(const fn_call& fn);
void xml_ondata(const fn_call& fn);
void xml_loaded(const fn_call& fn);


int memadjust(int x);


}	// end namespace gnash


#endif // HAVE_LIBXML

#endif	// __XML_H__


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
