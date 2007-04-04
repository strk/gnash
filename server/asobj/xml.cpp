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

/* $Id: xml.cpp,v 1.32 2007/04/04 10:32:42 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "tu_config.h"
#include "as_function.h" // for as_function
#include "fn_call.h"
#include "action.h" // for call_method

#include "xmlattrs.h"
#include "xmlnode.h"
#include "xml.h"
#include "builtin_function.h"
#include "debugger.h"
#include "StreamProvider.h"
#include "URLAccessManager.h"
#include "tu_file.h"
#include "URL.h"
#include "VM.h"

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <sstream>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <memory>

using namespace std;

namespace gnash {
  
//#define DEBUG_MEMORY_ALLOCATION 1

static as_object* getXMLInterface();

// Callback functions for xmlReadIO
static int closeTuFile (void * context);
static int readFromTuFile (void * context, char * buffer, int len);

DSOEXPORT as_value xml_new(const fn_call& fn);
static as_value xml_load(const fn_call& fn);
static as_value xml_addrequestheader(const fn_call& fn);
static as_value xml_createelement(const fn_call& fn);
static as_value xml_createtextnode(const fn_call& fn);
static as_value xml_getbytesloaded(const fn_call& fn);
static as_value xml_getbytestotal(const fn_call& fn);
static as_value xml_parsexml(const fn_call& fn);
static as_value xml_send(const fn_call& fn);
static as_value xml_sendandload(const fn_call& fn);

// These are the event handlers called for this object
static as_value xml_loaded(const fn_call& fn);

static LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
#ifdef USE_DEBUGGER
static Debugger& debugger = Debugger::getDefaultInstance();
#endif

XML::XML() 
    :
    XMLNode(getXMLInterface()),
    _loaded(false), 
    _bytes_loaded(0),
    _bytes_total(0)
{
    //GNASH_REPORT_FUNCTION;
#ifdef DEBUG_MEMORY_ALLOCATION
    log_msg("Creating XML data at %p \n", this);
#endif
    //log_msg("%s: %p \n", __FUNCTION__, this);
}


// Parse the ASCII XML string into memory
XML::XML(const std::string& xml_in)
    :
    XMLNode(getXMLInterface()),
    _loaded(false), 
    _bytes_loaded(0),
    _bytes_total(0)
{
    //GNASH_REPORT_FUNCTION;
#ifdef DEBUG_MEMORY_ALLOCATION
    log_msg("Creating XML data at %p \n", this);
#endif
    parseXML(xml_in);
}

XML::XML(struct node * /* childNode */)
    :
    XMLNode(getXMLInterface()),
    _loaded(false), 
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
    log_msg("\tDeleting XML top level node at %p", this);
#endif
  
}

void
XML::onLoadEvent(bool success)
{
    // Do the events that (appear to) happen as the movie
    // loads.  frame1 tags and actions are executed (even
    // before advance() is called).  Then the onLoad event
    // is triggered.

    // In ActionScript 2.0, event method names are CASE SENSITIVE.
    // In ActionScript 1.0, event method names are CASE INSENSITIVE.
    // TODO: move to get_function_name directly ?
    std::string method_name = "onLoad";
    if ( _vm.getSWFVersion() < 7 )
        boost::to_lower(method_name, _vm.getLocale());

    if ( method_name.empty() ) return;

    as_value	method;
    if ( ! get_member(method_name, &method) ) return;
    if ( method.is_undefined() ) return;
    if ( ! method.is_function() ) return;

    as_environment env; // how to set target here ??
    env.push(as_value(success));
    call_method(method, &env, this, 1, env.stack_size()-1);
}

void
XML::onCloseEvent()
{
    // Do the events that (appear to) happen as the movie
    // loads.  frame1 tags and actions are executed (even
    // before advance() is called).  Then the onLoad event
    // is triggered.

    // In ActionScript 2.0, event method names are CASE SENSITIVE.
    // In ActionScript 1.0, event method names are CASE INSENSITIVE.
    // TODO: move to get_function_name directly ?
    std::string method_name = "onClose";
    if ( _vm.getSWFVersion() < 7 )
        boost::to_lower(method_name, _vm.getLocale());

    if ( method_name.empty() ) return;

    as_value	method;
    if ( ! get_member(method_name, &method) ) return;
    if ( method.is_undefined() ) return;
    if ( ! method.is_function() ) return;

    as_environment env; // how to set target here ??
    call_method(method, &env, this, 0, 0);
}

void
XML::extractNode(XMLNode& element, xmlNodePtr node, bool mem)
{
    xmlAttrPtr attr;
    xmlNodePtr childnode;
    xmlChar *ptr = NULL;
    boost::intrusive_ptr<XMLNode> child;
    int len;

//    log_msg("Created new element for %s at %p\n", node->name, element);

//    log_msg("%s: extracting node %s\n", __FUNCTION__, node->name);

    // See if we have any Attributes (properties)
    attr = node->properties;
    while (attr != NULL)
    {
        //log_msg("extractNode %s has property %s, value is %s\n",
        //          node->name, attr->name, attr->children->content);
        XMLAttr attrib(reinterpret_cast<const char*>(attr->name),
			reinterpret_cast<const char*>(attr->children->content));

#if 0
        len = memadjust(strlen(reinterpret_cast<const char *>(attr->name))+1);
        attrib->_name = (char *)new char[len];
        memset(attrib->_name, 0, len);
        strcpy(attrib->_name, reinterpret_cast<const char *>(attr->name));

        len = memadjust(strlen(reinterpret_cast<const char *>(attr->children->content))+1);
        attrib->_value = (char *)new char[len];
        memset(attrib->_value, 0, len);
        strcpy(attrib->_value, reinterpret_cast<const char *>(attr->children->content));
#endif

        //log_msg("\tPushing attribute %s for element %s has value %s\n",
        //        attr->name, node->name, attr->children->content);
        element._attributes.push_back(attrib);
        attr = attr->next;
    }

    len = memadjust(strlen(reinterpret_cast<const char *>(node->name))+1);
    element._name = (char *)new char[len];
    memset(element._name, 0, len);
    strcpy(element._name, reinterpret_cast<const char *>(node->name));
    //element._name = reinterpret_cast<const char *>(node->name);
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
                    element._value = (char *)new char[len];
                    memset(element._value, 0, len);
                    strcpy(element._value, reinterpret_cast<const char *>(ptr));
                    //element->_value = reinterpret_cast<const char *>(ptr);
                }
            }
            xmlFree(ptr);
        }
    }
    
    // See if we have any data (content)
    childnode = node->children;

    while (childnode != NULL)
    {
        if (childnode->type == XML_ELEMENT_NODE)
        {
            child = new XMLNode();
            extractNode(*child, childnode, mem);
            element._children.push_back(child);
        }
        childnode = childnode->next;
    }

}

// Read in an XML document from the specified source
bool
XML::parseDoc(xmlDocPtr document, bool mem)
{
    //GNASH_REPORT_FUNCTION;  

    xmlNodePtr cur;

    if (document == 0) {
        log_error("Can't load XML file!");
        return false;
    }

    cur = xmlDocGetRootElement(document);
  
    if (cur != NULL)
    {
        boost::intrusive_ptr<XMLNode> child = new XMLNode();
        extractNode(*child, cur, mem);
        _children.push_back(child);
    }  

    return true;
}

// This reads in an XML file from disk and parses into into a memory resident
// tree which can be walked through later.
bool
XML::parseXML(const std::string& xml_in)
{
    //GNASH_REPORT_FUNCTION;

    //log_msg("Parse XML from memory: %s", xml_in.c_str());

    if (xml_in.empty()) {
        log_error("XML data is empty!");
        return false;
    }

#ifndef USE_DMALLOC
    //dump_memory_stats(__FUNCTION__, __LINE__, "before xmlParseMemory");
#endif

    //_bytes_total = _bytes_loaded = xml_in.size();
    
    bool ret=true;

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

#ifndef USE_DMALLOC
    //dump_memory_stats(__FUNCTION__, __LINE__, "after xmlParseMemory");
#endif
    return ret;
  
}


// This reads in an XML file from disk and parses into into a memory resident
// tree which can be walked through later.
bool
XML::load(const URL& url)
{
//    GNASH_REPORT_FUNCTION;
  
    //log_msg("%s: mem is %d\n", __FUNCTION__, mem);

    std::auto_ptr<tu_file> str ( StreamProvider::getDefaultInstance().getStream(url) );
    if ( ! str.get() ) 
    {
        log_error("Can't load XML file: %s (security?)", url.str().c_str());
        onLoadEvent(false);
        return false;
    }

    log_msg("Load XML file from url: %s", url.str().c_str());

    xmlInitParser();

    _doc = xmlReadIO(readFromTuFile, closeTuFile, str.get(), url.str().c_str(), NULL, 0);
    _bytes_total = str->get_size();

    if (_doc == 0) {
        log_error("Can't read XML file (IO): %s!", url.str().c_str());
        onLoadEvent(false);
        return false;
    }

    _bytes_loaded = _bytes_total;

    parseDoc(_doc, false);
    xmlCleanupParser();
    xmlFreeDoc(_doc);
    xmlMemoryDump();
    _loaded = true;

    onLoadEvent(true);

    return true;
}


bool
XML::onLoad()
{
    log_msg("%s: FIXME: onLoad Default event handler\n", __FUNCTION__);

    return(_loaded);
}

void
XML::cleanupStackFrames(XMLNode * /* xml */)
{
    GNASH_REPORT_FUNCTION;
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
XML::addRequestHeader(const char * /* name */, const char * /* value */)
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


//
// Callbacks. These are the wrappers for the C++ functions so they'll work as
// callbacks from within gnash.
//
as_value
xml_load(const fn_call& fn)
{
    as_value	method;
    as_value	val;
    as_value	rv = false;
    bool          ret;

    //GNASH_REPORT_FUNCTION;
  
    boost::intrusive_ptr<XML> xml_obj = ensureType<XML>(fn.this_ptr);
  
    std::string filespec = fn.arg(0).to_string(); 

    URL url(filespec, get_base_url());

    // Set the argument to the function event handler based on whether the load
    // was successful or failed.
    ret = xml_obj->load(url);
    rv = ret;

    if (ret == false) {
        return rv;
    }
    
    rv = true;
    return rv;
}

void
attachXMLInterface(as_object& o)
{
    // FIXME: this doesn't appear to exist in the MM player, should it ?
    o.init_member("loaded", new builtin_function(xml_loaded));
	
    o.init_member("addRequestHeader", new builtin_function(xml_addrequestheader));
    o.init_member("createElement", new builtin_function(xml_createelement));
    o.init_member("createTextNode", new builtin_function(xml_createtextnode));
    o.init_member("getBytesLoaded", new builtin_function(xml_getbytesloaded));
    o.init_member("getBytesTotal", new builtin_function(xml_getbytestotal));
    o.init_member("load", new builtin_function(xml_load));
    o.init_member("parseXML", new builtin_function(xml_parsexml));
    o.init_member("send", new builtin_function(xml_send));
    o.init_member("sendAndLoad", new builtin_function(xml_sendandload));

}

static as_object*
getXMLInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( o == NULL )
    {
        o = new as_object(getXMLNodeInterface());
        attachXMLInterface(*o);
    }
    return o.get();
}

as_value
xml_new(const fn_call& fn)
{
    as_value      inum;
    boost::intrusive_ptr<XML> xml_obj;
    //const char    *data;
  
    // log_msg("%s: nargs=%d\n", __FUNCTION__, fn.nargs);
  
    if ( fn.nargs > 0 )
    {
        if ( fn.arg(0).is_object() )
        {
            boost::intrusive_ptr<as_object> obj = fn.env().top(0).to_object();
            xml_obj = boost::dynamic_pointer_cast<XML>(obj);
            if ( xml_obj )
            {
                log_msg("\tCloned the XML object at %p\n", xml_obj.get());
                return as_value(xml_obj->cloneNode(true).get());
            }
        }

        std::string xml_in = fn.arg(0).to_std_string();
        if ( xml_in.empty() )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("First arg given to XML constructor (%s) evaluates to the empty string",
                    fn.arg(0).to_debug_string().c_str());
            );
        }
        else
        {
            xml_obj = new XML(xml_in);
            return as_value(xml_obj.get());
        }
    }

    xml_obj = new XML;
    //log_msg("\tCreated New XML object at %p\n", xml_obj);

    return as_value(xml_obj.get());
}

//
// SWF Property of this class. These are "accessors" into the private data
// of the class.
//

// determines whether the document-loading process initiated by the XML.load()
// call has completed. If the process completes successfully, the method
// returns true; otherwise, it returns false.
as_value
xml_loaded(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    as_value	method;
    as_value	val;

    log_msg("%s:\n", __FUNCTION__);
    
    boost::intrusive_ptr<XML> ptr = ensureType<XML>(fn.this_ptr);
    std::string filespec = fn.arg(0).to_string();
    //fn.result->set(ptr->loaded());
    return as_value(ptr->loaded());
}


as_value xml_addrequestheader(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    log_msg("%s: %d args\n", __PRETTY_FUNCTION__, fn.nargs);
    
//    return as_value(ptr->getAllocated());
//    ptr->addRequestHeader();
    log_msg("%s:unimplemented \n", __FUNCTION__);
    return as_value();
}

/// \brief create a new XML element
///
/// Method; creates a new XML element with the name specified in the
/// parameter. The new element initially has no parent, no children,
/// and no siblings. The method returns a reference to the newly
/// created XML object that represents the element. This method and
/// the XML.createTextNode() method are the constructor methods for
/// creating nodes for an XML object. 

as_value xml_createelement(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    
    if (fn.nargs > 0) {
        const char *text = fn.arg(0).to_string();
	XMLNode *xml_obj = new XMLNode();
//	cerr << "create new child XMLNode is at " << (void *)xml_obj << endl;
	xml_obj->nodeNameSet(text);
	xml_obj->nodeTypeSet(XML_ELEMENT_NODE);
//	ptr->set_member(text, xml_obj); // FIXME: use a getter/setter !
	// no return code from this method
	return as_value(xml_obj);
	
    } else {
        log_msg("ERROR: no text for element creation!\n");
    }
    return as_value();
}

/// \brief Create a new XML node
/// 
/// Method; creates a new XML text node with the specified text. The
/// new node initially has no parent, and text nodes cannot have
/// children or siblings. This method returns a reference to the XML
/// object that represents the new text node. This method and the
/// XML.createElement() method are the constructor methods for
/// creating nodes for an XML object.

as_value xml_createtextnode(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;

    XMLNode *xml_obj;
    const char *text;

    if (fn.nargs > 0) {
	text = fn.arg(0).to_string(); 
	xml_obj = new XMLNode;
	xml_obj->nodeValueSet(text);
	xml_obj->nodeTypeSet(XML_TEXT_NODE);
	return as_value(xml_obj);
//	log_msg("%s: xml obj is %p\n", __PRETTY_FUNCTION__, xml_obj);
    } else {
	log_msg("ERROR: no text for text node creation!\n");
    }
    return as_value();
}

as_value xml_getbytesloaded(const fn_call& fn)
{
    boost::intrusive_ptr<XML> ptr = ensureType<XML>(fn.this_ptr);
    if ( ptr->loaded() )
	{
		return as_value(ptr->getBytesLoaded());
    }
	else
	{
		return as_value();
    }
}

as_value xml_getbytestotal(const fn_call& fn)
{
    boost::intrusive_ptr<XML> ptr = ensureType<XML>(fn.this_ptr);
    if ( ptr->loaded() )
	{
		return as_value(ptr->getBytesTotal());
	}
	else
	{
		return as_value();
	}
}

as_value xml_parsexml(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    as_value	method;
    as_value	val;    
    boost::intrusive_ptr<XML> ptr = ensureType<XML>(fn.this_ptr);

    if (fn.nargs > 0)
    {
        std::string text = fn.arg(0).to_std_string(&(fn.env()));
        ptr->parseXML(text);
    }
    
    return as_value();
}

/// \brief removes the specified XML object from its parent. Also
/// deletes all descendants of the node.
    
as_value xml_send(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<XML> ptr = ensureType<XML>(fn.this_ptr);
    
//    return as_value(ptr->getAllocated());
    ptr->send();
    return as_value();
}

static as_value
xml_sendandload(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<XML> ptr = ensureType<XML>(fn.this_ptr);
    
//    return as_value(ptr->getAllocated());
    ptr->sendAndLoad();
    return as_value();
}

int
memadjust(int x)
{
    return (x + (4 - x % 4));
}

// extern (used by Global.cpp)
void xml_class_init(as_object& global)
{
//    GNASH_REPORT_FUNCTION;
    // This is going to be the global XML "class"/"function"
    static boost::intrusive_ptr<builtin_function> cl;

    if ( cl == NULL ) {
	cl=new builtin_function(&xml_new, getXMLInterface());
	// replicate all interface to class, to be able to access
	// all methods as static functions
	attachXMLInterface(*cl);
    }
    
    // Register _global.String
    global.init_member("XML", cl.get());

}

// Callback function for xmlReadIO
static int
readFromTuFile (void * context, char * buffer, int len)
{
        tu_file* str = static_cast<tu_file*>(context);
        size_t read = str->read_bytes(buffer, len);
        if ( str->get_error() ) return -1;
        else return read;
}

// Callback function for xmlReadIO
static int
closeTuFile (void * /*context*/)
{
        // nothing to do, the tu_file destructor will close
        //tu_file* str = static_cast<tu_file*>(context);
        //str->close();
        return 0; // no error
}

} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
