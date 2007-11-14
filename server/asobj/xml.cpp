// xml.cpp:  XML markup language support, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

/* $Id: xml.cpp,v 1.52 2007/11/14 09:16:49 bwy Exp $ */

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
#include "namedStrings.h"

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
//#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <memory>

using namespace std;

namespace gnash {
  
//#define DEBUG_MEMORY_ALLOCATION 1

static as_object* getXMLInterface();
static void attachXMLInterface(as_object& o);
static void attachXMLProperties(as_object& o);

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
static as_value xml_ondata(const fn_call& fn);

static LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
#ifdef USE_DEBUGGER
static Debugger& debugger = Debugger::getDefaultInstance();
#endif

XML::XML() 
    :
    XMLNode(getXMLInterface()),
    _loaded(-1), 
    _bytes_loaded(0),
    _bytes_total(0),
    _status(sOK)
{
    //GNASH_REPORT_FUNCTION;
#ifdef DEBUG_MEMORY_ALLOCATION
    log_msg(_("Creating XML data at %p"), this);
#endif
    //log_msg("%s: %p", __FUNCTION__, this);
    attachXMLProperties(*this);
}


// Parse the ASCII XML string into memory
XML::XML(const std::string& xml_in)
    :
    XMLNode(getXMLInterface()),
    _loaded(-1), 
    _bytes_loaded(0),
    _bytes_total(0),
    _status(sOK)
{
    //GNASH_REPORT_FUNCTION;
#ifdef DEBUG_MEMORY_ALLOCATION
    log_msg(_("Creating XML data at %p"), this);
#endif
    parseXML(xml_in);
}

XML::XML(struct node * /* childNode */)
    :
    XMLNode(getXMLInterface()),
    _loaded(-1), 
    _bytes_loaded(0),
    _bytes_total(0),
    _status(sOK)
{
    GNASH_REPORT_FUNCTION;
#ifdef DEBUG_MEMORY_ALLOCATION
    log_msg(_("\tCreating XML data at %p"), this);
#endif
    //log_msg("%s: %p", __FUNCTION__, this);
}

bool
XML::get_member(string_table::key name, as_value *val, string_table::key nsname)
{
        if (name == NSV::PROP_STATUS) 
        {
                val->set_int(_status);
                return true;
        }
        else if (name == NSV::PROP_LOADED)
        {
                if ( _loaded < 0 ) val->set_undefined();
                else val->set_bool(_loaded);
                return true;
        }

        return get_member_default(name, val, nsname);
}

void
XML::set_member(string_table::key name, const as_value& val, 
	string_table::key nsname)
{
        if (name == NSV::PROP_STATUS)
	{
		if ( ! val.is_number() )
		{
			_status = static_cast<XML::Status>(std::numeric_limits<int32_t>::min());
		}
		else
		{
			unsigned int statusNumber = static_cast<int>(val.to_number());
			_status = XML::Status( static_cast<XML::Status>(statusNumber) );
		}
		return;
	}
        else if (name == NSV::PROP_LOADED)
        {
                bool b = val.to_bool();
		log_msg(_("set_member 'loaded' (%s) became boolean %d"), val.to_debug_string().c_str(), b);
                if ( b ) _loaded = 1;
                else _loaded = 0;
                return;
        }

        set_member_default(name, val, nsname);
}

XML::~XML()
{
    //GNASH_REPORT_FUNCTION;
    
#ifdef DEBUG_MEMORY_ALLOCATION
    log_msg(_("\tDeleting XML top level node at %p"), this);
#endif
  
}

void
XML::onLoadEvent(bool success, as_environment& env)
{
    // Do the events that (appear to) happen as the movie
    // loads.  frame1 tags and actions are executed (even
    // before advance() is called).  Then the onLoad event
    // is triggered.

    // In ActionScript 2.0, event method names are CASE SENSITIVE.
    // In ActionScript 1.0, event method names are CASE INSENSITIVE.
    // TODO: move to get_function_name directly ?
    std::string method_name1 = "onLoad";
    if ( _vm.getSWFVersion() < 7 )
        boost::to_lower(method_name1, _vm.getLocale());

    if ( method_name1.empty() ) return;

	string_table::key method_name = _vm.getStringTable().find(method_name1);
    as_value	method;
    if (!get_member(method_name, &method) ) return;
    if ( method.is_undefined() ) return;
    if ( ! method.is_function() ) return;

#ifndef NDEBUG
    size_t prevStackSize = env.stack_size();
#endif
    env.push(as_value(success));
    call_method(method, &env, this, 1, env.stack_size()-1);
    env.drop(1);
#ifndef NDEBUG
    assert( prevStackSize == env.stack_size());
#endif
}

void
XML::onCloseEvent(as_environment& env)
{
    // Do the events that (appear to) happen as the movie
    // loads.  frame1 tags and actions are executed (even
    // before advance() is called).  Then the onLoad event
    // is triggered.

    // In ActionScript 2.0, event method names are CASE SENSITIVE.
    // In ActionScript 1.0, event method names are CASE INSENSITIVE.
    // TODO: move to get_function_name directly ?
    std::string method_name1 = "onClose";
    if ( _vm.getSWFVersion() < 7 )
        boost::to_lower(method_name1, _vm.getLocale());

    if ( method_name1.empty() ) return;

	string_table::key method_name = _vm.getStringTable().find(method_name1);
    as_value	method;
    if (! get_member(method_name, &method) ) return;
    if ( method.is_undefined() ) return;
    if ( ! method.is_function() ) return;

    call_method(method, &env, this, 0, 0);
}

bool
XML::extractNode(XMLNode& element, xmlNodePtr node, bool mem)
{
    xmlAttrPtr attr;
    xmlChar *ptr = NULL;
    boost::intrusive_ptr<XMLNode> child;

//    log_msg(_("Created new element for %s at %p"), node->name, element);

//    log_msg(_("%s: extracting node %s"), __FUNCTION__, node->name);

    // See if we have any Attributes (properties)
    attr = node->properties;
    while (attr != NULL)
    {
        //log_msg(_("extractNode %s has property %s, value is %s"),
        //          node->name, attr->name, attr->children->content);
        XMLAttr attrib(reinterpret_cast<const char*>(attr->name),
			reinterpret_cast<const char*>(attr->children->content));

        //log_msg(_("\tPushing attribute %s for element %s has value %s"),
        //        attr->name, node->name, attr->children->content);
        element._attributes.push_back(attrib);
        attr = attr->next;
    }
    if (node->type == XML_COMMENT_NODE)
    {
    	// Comments apparently not handled until AS3
    	// Comments in a text node are a *sibling* of the text node
    	// for libxml2.
    	return false;
    }
    else if (node->type == XML_ELEMENT_NODE)
    {
            element.nodeTypeSet(tElement);

            std::string name(reinterpret_cast<const char*>(node->name));
            element.nodeNameSet(name);
    }
    else if ( node->type == XML_TEXT_NODE )
    {
            element.nodeTypeSet(tText);

            ptr = xmlNodeGetContent(node);
            if (ptr == NULL) return false;
	    if (node->content)
	    {
		const char* in = reinterpret_cast<const char*>(ptr);
		// XML_PARSE_NOBLANKS seems not to be working, so here's
		// a custom implementation of it.
		if ( ignoreWhite() )
		{
			if ( strspn(in, " \n\t\r") == strlen(in) )
			{
				log_msg("Text node value consists in blanks only, discarding");
				return false;
			}
		}
		std::string val(in);
		element.nodeValueSet(val);
	    }
            xmlFree(ptr);
    }

    // See if we have any data (content)
    xmlNodePtr childnode = node->children;

    while (childnode)
    {
        child = new XMLNode();
        child->setParent(&element);
        if ( extractNode(*child, childnode, mem) ) element._children.push_back(child);
        childnode = childnode->next;
    }

    return true;
}

/*private*/
bool
XML::parseDoc(xmlDocPtr document, bool mem)
{
    //GNASH_REPORT_FUNCTION;  

    xmlNodePtr cur;

    if (document == 0) {
        log_error(_("Can't load XML file"));
        return false;
    }

    cur = xmlDocGetRootElement(document);
  
    if (cur != NULL)
    {
        boost::intrusive_ptr<XMLNode> child = new XMLNode();
        child->setParent(this);
        if ( extractNode(*child, cur, mem) ) 
	{
        	_children.push_back(child);
	}
    }  

    return true;
}

// This reads in an XML file from disk and parses into into a memory resident
// tree which can be walked through later.
bool
XML::parseXML(const std::string& xml_in)
{
    //GNASH_REPORT_FUNCTION;

    //log_msg(_("Parse XML from memory: %s"), xml_in.c_str());

    if (xml_in.empty()) {
        log_error(_("XML data is empty"));
        return false;
    }

#ifndef USE_DMALLOC
    //dump_memory_stats(__FUNCTION__, __LINE__, "before xmlParseMemory");
#endif

    //_bytes_total = _bytes_loaded = xml_in.size();

    // Clear current data
    clear(); 
    
    initParser();

    //_doc = xmlParseMemory(xml_in.c_str(), xml_in.size());
    _doc = xmlReadMemory(xml_in.c_str(), xml_in.size(), NULL, NULL, getXMLOptions());
    if (_doc == 0) {
        log_error(_("Can't parse XML data"));
        return false;
    }

    bool ret = parseDoc(_doc, true);
    xmlCleanupParser();
    xmlFreeDoc(_doc);
    xmlMemoryDump();

#ifndef USE_DMALLOC
    //dump_memory_stats(__FUNCTION__, __LINE__, "after xmlParseMemory");
#endif
    return ret;
  
}

void
XML::queueLoad(std::auto_ptr<tu_file> str,  as_environment& env)
{
	GNASH_REPORT_FUNCTION;

    // Set the "loaded" parameter to false
    VM& vm = _vm;
    string_table& st = vm.getStringTable();
    string_table::key loadedKey = st.find("loaded");
    string_table::key onDataKey = st.find(PROPNAME("onData"));
    set_member(loadedKey, as_value(false));

    // TODO:
    // 1. interrupt any pre-existing loading thread (send onLoad event in that case?)
    // 2. start new loading thread
    //
    // Using LoadThread should do. Most likely we should do something
    // similar for LoadVars or ::loadVariables, so might consider generalizing
    // a load-in-a-separate-thread-calling-onLoad-when-done class
    //
    // The class would use a separate thread, provide cancelling and
    // will call a specified function (or functor) when all data arrived,
    // passing it the full data buffer.
    //
    std::string src;
    char buf[256];
    while ( 1 )
    {
        size_t bytes = str->read_bytes(buf, 255);
        src.append(buf, bytes);
        if ( bytes < 255 ) break; // end of buffer
    }
    callMethod(onDataKey, env, as_value(src));
}

// This reads in an XML file from disk and parses into into a memory resident
// tree which can be walked through later.
bool
XML::load(const URL& url, as_environment& env)
{
    GNASH_REPORT_FUNCTION;
  
    //log_msg(_("%s: mem is %d"), __FUNCTION__, mem);

    std::auto_ptr<tu_file> str ( StreamProvider::getDefaultInstance().getStream(url) );
    if ( ! str.get() ) 
    {
        log_error(_("Can't load XML file: %s (security?)"), url.str().c_str());
        as_value nullValue; nullValue.set_null();
        callMethod(VM::get().getStringTable().find("onData"), env, nullValue);
        return false;
    }

    log_msg(_("Loading XML file from url: '%s'"), url.str().c_str());
    queueLoad(str, env);

    return true;
}


bool
XML::onLoad()
{
    log_msg(_("%s: FIXME: onLoad Default event handler"), __FUNCTION__);

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
    log_unimpl (__FUNCTION__);
}


void
XML::load()
{
    log_unimpl (__FUNCTION__);
}

void
XML::parseXML()
{
    log_unimpl (__FUNCTION__);
}

void
XML::send()
{
    log_unimpl (__FUNCTION__);
}

void
XML::sendAndLoad()
{
    log_unimpl (__FUNCTION__);
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
  
    const std::string& filespec = fn.arg(0).to_string(&(fn.env()));

    URL url(filespec, get_base_url());

    // Set the argument to the function event handler based on whether the load
    // was successful or failed.
    ret = xml_obj->load(url, fn.env());
    rv = ret;

    if (ret == false) {
        return rv;
    }
    
    rv = true;
    return rv;
}

static void
attachXMLProperties(as_object& /*o*/)
{
    // if we use a proper member here hasOwnProperty() would return true
    // but we want it to return false instead. See XML.as
    //o.init_member("status", as_value(XML::sOK));
}

static void
attachXMLInterface(as_object& o)
{
    o.init_member("addRequestHeader", new builtin_function(xml_addrequestheader));
    o.init_member("createElement", new builtin_function(xml_createelement));
    o.init_member("createTextNode", new builtin_function(xml_createtextnode));
    o.init_member("getBytesLoaded", new builtin_function(xml_getbytesloaded));
    o.init_member("getBytesTotal", new builtin_function(xml_getbytestotal));
    o.init_member("load", new builtin_function(xml_load));
    o.init_member("parseXML", new builtin_function(xml_parsexml));
    o.init_member("send", new builtin_function(xml_send));
    o.init_member("sendAndLoad", new builtin_function(xml_sendandload));
    o.init_member("onData", new builtin_function(xml_ondata));

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
  
    // log_msg(_("%s: nargs=%d"), __FUNCTION__, fn.nargs);
  
    if ( fn.nargs > 0 )
    {
        if ( fn.arg(0).is_object() )
        {
            boost::intrusive_ptr<as_object> obj = fn.env().top(0).to_object();
            xml_obj = boost::dynamic_pointer_cast<XML>(obj);
            if ( xml_obj )
            {
                log_msg(_("\tCloned the XML object at %p"), (void *)xml_obj.get());
                return as_value(xml_obj->cloneNode(true).get());
            }
        }

        const std::string& xml_in = fn.arg(0).to_string(&(fn.env()));
        if ( xml_in.empty() )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("First arg given to XML constructor (%s) evaluates to the empty string"),
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
    //log_msg(_("\tCreated New XML object at %p"), xml_obj);

    return as_value(xml_obj.get());
}

//
// SWF Property of this class. These are "accessors" into the private data
// of the class.
//

as_value xml_addrequestheader(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    log_msg(_("%s: %d args"), __PRETTY_FUNCTION__, fn.nargs);
    
//    return as_value(ptr->getAllocated());
//    ptr->addRequestHeader();
    log_unimpl (__FUNCTION__);
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

static as_value
xml_createelement(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    
    if (fn.nargs > 0) {
        const std::string& text = fn.arg(0).to_string(&(fn.env()));
	XMLNode *xml_obj = new XMLNode();
	xml_obj->nodeNameSet(text);
	xml_obj->nodeTypeSet(XMLNode::tText);
	// no return code from this method
	return as_value(xml_obj);
	
    } else {
        log_error(_("no text for element creation"));
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

as_value
xml_createtextnode(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;

    XMLNode *xml_obj;

    if (fn.nargs > 0) {
	const std::string& text = fn.arg(0).to_string(&(fn.env()));
	xml_obj = new XMLNode;
	xml_obj->nodeValueSet(text);
	xml_obj->nodeTypeSet(XMLNode::tText);
	return as_value(xml_obj);
//	log_msg(_("%s: xml obj is %p"), __PRETTY_FUNCTION__, xml_obj);
    } else {
	log_error(_("no text for text node creation"));
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
    //GNASH_REPORT_FUNCTION;
    as_value	method;
    as_value	val;    
    boost::intrusive_ptr<XML> ptr = ensureType<XML>(fn.this_ptr);

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror("XML.parseXML() needs one argument");
        );
        return as_value();
    }

    const std::string& text = fn.arg(0).to_string(&(fn.env()));
    ptr->parseXML(text);
    
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

static as_value
xml_ondata(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;

    VM& vm = VM::get();
    string_table& st = vm.getStringTable();
    string_table::key onLoadKey = st.find(PROPNAME("onLoad"));
    string_table::key loadedKey = st.find("loaded");
    as_environment& env = fn.env();

    as_object* thisPtr = fn.this_ptr.get();
    assert(thisPtr);

    // See http://gitweb.freedesktop.org/?p=swfdec/swfdec.git;a=blob;f=libswfdec/swfdec_initialize.as

    as_value src; src.set_null();
    if ( fn.nargs ) src = fn.arg(0);

    if ( ! src.is_null() )
    {
        string_table::key parseXMLKey = st.find(PROPNAME("parseXML"));
        as_value tmp(true);
        thisPtr->set_member(loadedKey, tmp);
        thisPtr->callMethod(parseXMLKey, env, src);
        thisPtr->callMethod(onLoadKey, env, tmp);
    }
    else
    {
        as_value tmp(true);
        thisPtr->set_member(loadedKey, tmp);
        thisPtr->callMethod(onLoadKey, env, tmp);
    }

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

    if ( cl == NULL )
    {
        cl=new builtin_function(&xml_new, getXMLInterface());
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

#if 0 // not time for this (yet)
static
void _xmlErrorHandler(void* ctx, const char* fmt, ...)
{
    va_list ap;
    static const unsigned long BUFFER_SIZE = 128;
    char tmp[BUFFER_SIZE];

    va_start (ap, fmt);
    vsnprintf (tmp, BUFFER_SIZE, fmt, ap);
    tmp[BUFFER_SIZE-1] = '\0';

    log_error(_("XML parser: %s"), tmp);
    
    va_end (ap);    
}
#endif // disabled

void
XML::initParser()
{
    static bool initialized = false;
    if ( ! initialized )
    {
        xmlInitParser();
        //xmlGenericErrorFunc func = _xmlErrorHandler;
        //initGenericErrorDefaultFunc(&func);
        initialized = true;
    }
}

void
XML::clear()
{
	// TODO: should set childs's parent to NULL ?
	_children.clear();

	_attributes.clear();
}

/*private*/
bool
XML::ignoreWhite() const
{
    // TODO: initialize this thing once...
    std::string propname;
    if ( VM::get().getSWFVersion() < 7 ) propname = "ignorewhite";
    else propname = "ignoreWhite";

	string_table::key propnamekey = VM::get().getStringTable().find(propname);
    as_value val;
    if (!const_cast<XML*>(this)->get_member(propnamekey, &val) ) return false;
    return val.to_bool();
}

/*private*/
int
XML::getXMLOptions() const
{
    int options = XML_PARSE_RECOVER | XML_PARSE_NOWARNING
    		| XML_PARSE_NOERROR | XML_PARSE_NOCDATA;
    // Using libxml2 to convert CDATA nodes to text seems to be what is
    // required.
    
    if ( ignoreWhite() )
    {
	    // This doesn't seem to work, so the blanks skipping
	    // is actually implemented in XML::extractNode instead.
            //log_msg("Adding XML_PARSE_NOBLANKS to options");
            options |= XML_PARSE_NOBLANKS;
    }

    return options;
}

} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
