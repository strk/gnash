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

#ifndef __XML_H__
#define __XML_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"

#include "event_id.h"
#include "action.h"

//#define DEBUG_MEMORY_ALLOCATION 1
#include <vector>

#include "xmlattrs.h"
#include "xmlnode.h"

#ifdef DEBUG_MEMORY_ALLOCATION
	#include "log.h"
#endif

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>

// Forward declarations
namespace gnash {
	class fn_call;
}

namespace gnash {
  
/// XML class and ActionScript object
class DSOLOCAL XML : public as_object
{

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
    
    virtual bool on_event(const gnash::event_id& id);
    virtual void	on_event_load();
    bool loaded()    { return _loaded; }
    
    XMLNode *firstChild() {
        return _nodes;
        //return _node_data[0];
    }
    
    void clear() {
        delete _nodes;
    }
  
  std::vector<XMLNode *> childNodes();
  
    const char *stringify(XMLNode *xml);
    //  Returns true if the specified node has child nodes; otherwise, returns false.
  bool hasChildNodes() {
    if (_nodes) {
      if (_nodes->_children.size()) {
        return true;
      } 
    }
    return false;
  }
    
    XMLNode *extractNode(xmlNodePtr node, bool mem);
    XMLNode *processNode(xmlTextReaderPtr reader, XMLNode *node);

    void  change_stack_frame(int frame, gnash::as_object *xml, gnash::as_environment *env);
//    void  setupStackFrames(gnash::as_object *xml, gnash::as_environment *env);
    void  cleanupStackFrames( XMLNode *data);
    as_object *setupFrame(gnash::as_object *xml, XMLNode *data, bool src);
  
    const char *nodeNameGet()    { return _nodename; }
    const char *nodeName();
    const char *nodeValue();
    void nodeNameSet(char *name);
    void nodeValueSet(char *value);
    int length()                 { return _nodes->length(); }
  
    // These 6 have to 
    void addRequestHeader(const char *name, const char *value);
    XMLNode &cloneNode(XMLNode &newnode, bool deep);
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

    virtual void	on_xml_event(const gnash::event_id& id)
    {
    	on_event(id);
    }
  
    // Special event handler; 
    void	on_event_close()
    {
    	on_event(gnash::event_id::SOCK_CLOSE);
    }
  
    XMLNode *operator [] (int x);
#if 0
    XMLNode *operator = (XMLNode &node) {
        gnash::log_msg("%s: copy element %s\n", __PRETTY_FUNCTION__, node._name);
	//        _nodes = node.;
    }

#endif
    XML *operator = (XMLNode *node) {
        _nodes = node;    
        return this;
    }

    // FIXME: drop this override when firstChild and childNodes are properties
    virtual bool get_member(const tu_stringi& name, as_value* val) {
        //printf("GET XML MEMBER: %s at %p for object %p\n", name.c_str(), val, this);
        
        if ((name == "firstChild") || (name == "childNodes")) {
//             printf("Returning a self reference for %s for object at %p\n",
//                    name.c_str(), this);
            val->set_as_object(this);
            return true;
        }

	return get_member_default(name, val);
        
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


DSOEXPORT void xml_class_init(as_object& global);

// Exporting this is a temporary hack for not changing xmlsocket.cpp now
// (xmlsocket_xml_new calls xml_new)
DSOEXPORT void xml_new(const fn_call& fn);

DSOEXPORT int memadjust(int x);


}	// end namespace gnash

#endif	// __XML_H__


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
