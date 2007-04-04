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
#include <sstream>

#include "xmlattrs.h"
#include "xmlnode.h"

#ifdef DEBUG_MEMORY_ALLOCATION
	#include "log.h"
#endif

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>

using namespace std;

namespace gnash {

// Forward declarations
class fn_call;
class URL;

/// XML class and ActionScript object
class DSOLOCAL XML : public XMLNode
{
public:

    XML();
    XML(const std::string& xml_in);
    XML(struct node * childNode);
    virtual ~XML();
  
    // Methods
    // This is the base method used by both parseXML() and load().
    bool parseDoc(xmlDocPtr document, bool mem);

    // Parses an XML document into the specified XML object tree.
    bool parseXML(const std::string& xml_in);

    // Loads a document (specified by
    // the XML object) from a URL.
    bool load(const URL& url);

    // An event handler that returns a
    bool onLoad();

    // Boolean value indicating whether
    // the XML object was successfully
    // loaded with XML.load() or
    // XML.sendAndLoad().

    bool loaded()    { return _loaded; }
    
    void clear() {}
  
    XMLNode *processNode(xmlTextReaderPtr reader, XMLNode *node);

    void  change_stack_frame(int frame, gnash::as_object *xml, gnash::as_environment *env);
//    void  setupStackFrames(gnash::as_object *xml, gnash::as_environment *env);

    void  cleanupStackFrames( XMLNode *data);

    // These 6 have to 
    void addRequestHeader(const char *name, const char *value);

    XMLNode *createElement(const char *name);

    XMLNode *createTextNode(const char *name);

    void load();

    void parseXML();

    void send();

    void sendAndLoad();

    int getBytesLoaded()         { return _bytes_loaded; };
    int getBytesTotal()          { return _bytes_total; };

private:
    xmlDocPtr _doc;
    xmlNodePtr _firstChild;
    
    // Properties
    bool _loaded;

    size_t      _bytes_loaded;
 
    size_t      _bytes_total;
    
    bool        _status;	// TODO Should be Number

    /// Trigger the onLoad event, if any
    void onLoadEvent(bool success);

    /// Trigger the onClose event, if any
    void onCloseEvent();
  
    /// Initialize an XMLNode from an xmlNodePtr
    //
    /// @param element
    ///     The XMLNode to initialize.
    ///
    void extractNode(XMLNode& element, xmlNodePtr node, bool mem);

    void setupFrame(gnash::as_object *xml, XMLNode *data, bool src);
  

};


DSOEXPORT void xml_class_init(as_object& global);

// Exporting this is a temporary hack for not changing xmlsocket.cpp now
// (xmlsocket_xml_new calls xml_new)
DSOEXPORT as_value xml_new(const fn_call& fn);

DSOEXPORT int memadjust(int x);


}	// end namespace gnash

#endif	// __XML_H__


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
