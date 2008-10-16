// xml.cpp:  XML markup language support, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "log.h"
#include "as_function.h" // for as_function
#include "fn_call.h"

#include "LoadableObject.h"
#include "xmlattrs.h"
#include "xmlnode.h"
#include "XML_as.h"
#include "builtin_function.h"
#include "VM.h"
#include "namedStrings.h"
#include "array.h"

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include <string>
#include <sstream>
#include <vector>
#include <boost/algorithm/string/case_conv.hpp>


namespace gnash {
  
//#define DEBUG_MEMORY_ALLOCATION 1

// Define this to enable verbosity of XML loads
//#define DEBUG_XML_LOADS 1

// Define this to enable verbosity of XML parsing
//#define DEBUG_XML_PARSE 1

static as_object* getXMLInterface();
static void attachXMLInterface(as_object& o);
static void attachXMLProperties(as_object& o);

static as_value xml_new(const fn_call& fn);
static as_value xml_createelement(const fn_call& fn);
static as_value xml_createtextnode(const fn_call& fn);
static as_value xml_getbytesloaded(const fn_call& fn);
static as_value xml_getbytestotal(const fn_call& fn);
static as_value xml_parsexml(const fn_call& fn);
static as_value xml_send(const fn_call& fn);
static as_value xml_ondata(const fn_call& fn);


XML_as::XML_as() 
    :
    as_object(getXMLInterface()),
    _loaded(-1), 
    _status(sOK)
{
#ifdef DEBUG_MEMORY_ALLOCATION
    log_debug(_("Creating XML data at %p"), this);
#endif

    attachXMLProperties(*this);
}


// Parse the ASCII XML string into memory
XML_as::XML_as(const std::string& xml_in)
    :
    as_object(getXMLInterface()),
    _loaded(-1), 
    _status(sOK)
{
#ifdef DEBUG_MEMORY_ALLOCATION
    log_debug(_("Creating XML data at %p"), this);
#endif

    parseXML(xml_in);
}

bool
XML_as::get_member(string_table::key name, as_value *val, string_table::key nsname)
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

        return as_object::get_member(name, val, nsname);
}

bool
XML_as::set_member(string_table::key name, const as_value& val, 
    string_table::key nsname, bool ifFound)
{
    if (name == NSV::PROP_STATUS)
    {
        // TODO: this should really be a proper property (see XML.as)
        if ( ! val.is_number() )
        {
            _status = static_cast<ParseStatus>(std::numeric_limits<boost::int32_t>::min());
        }
        else
        {
            unsigned int statusNumber = static_cast<int>(val.to_number());
            _status = static_cast<ParseStatus>(statusNumber);
        }
        return true;
    }
    else if (name == NSV::PROP_LOADED)
    {
        // TODO: this should really be a proper property
        bool b = val.to_bool();
        //log_debug(_("set_member 'loaded' (%s) became boolean %d"), val, b);
        if ( b ) _loaded = 1;
        else _loaded = 0;
        return true;
    }

    return as_object::set_member(name, val, nsname, ifFound);
}

bool
XML_as::extractNode(XMLNode& element, xmlNodePtr node, bool mem)
{
    xmlAttrPtr attr;
    xmlChar *ptr = NULL;
    boost::intrusive_ptr<XMLNode> child;

#ifdef DEBUG_XML_PARSE
    log_debug(_("%s: extracting node %s"), __FUNCTION__, node->name);
#endif

    // See if we have any Attributes (properties)
    attr = node->properties;
    while (attr != NULL)
    {
#ifdef DEBUG_XML_PARSE
        log_debug(_("extractNode %s has property %s, value is %s"),
                  node->name, attr->name, attr->children->content);
#endif
        
        std::ostringstream name, content;

        name << attr->name;
        content << attr->children->content;
        
        XMLAttr attrib(name.str(), content.str());

#ifdef DEBUG_XML_PARSE
        log_debug(_("\tPushing attribute %s for element %s has value %s, next attribute is %p"),
                attr->name, node->name, attr->children->content, attr->next);
#endif

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

            std::ostringstream name;
            name << node->name;
            element.nodeNameSet(name.str());
    }
    else if ( node->type == XML_TEXT_NODE )
    {
            element.nodeTypeSet(tText);

            ptr = xmlNodeGetContent(node);
            if (ptr == NULL) return false;
        if (node->content)
        {
        std::ostringstream in;
        in << ptr;
        // XML_PARSE_NOBLANKS seems not to be working, so here's
        // a custom implementation of it.
        if ( ignoreWhite() )
        {
            if ( in.str().find_first_not_of(" \n\t\r") == std::string::npos )
            {
#ifdef DEBUG_XML_PARSE
                log_debug("Text node value consists in blanks only, discarding");
#endif
                xmlFree(ptr);
                return false;
            }
        }
        element.nodeValueSet(in.str());
        }
            xmlFree(ptr);
    }

    // See if we have any data (content)
    xmlNodePtr childnode = node->children;

    while (childnode)
    {
        child = new XMLNode();
        child->setParent(&element);
        if ( extractNode(*child, childnode, mem) )
        {
            element._children.push_back(child);
        }
        childnode = childnode->next;
    }

    return true;
}

/*private*/
bool
XML_as::parseDoc(xmlNodePtr cur, bool mem)
{
    GNASH_REPORT_FUNCTION;  

    while (cur)
    {
        boost::intrusive_ptr<XMLNode> child = new XMLNode();
        child->setParent(this);
#ifdef DEBUG_XML_PARSE
        log_debug("\tParsing top-level node %s", cur->name);
#endif
        if ( extractNode(*child, cur, mem) ) 
        {
                _children.push_back(child);
        }
        cur = cur->next;
    }  

    return true;
}

// This parses an XML string into a
// tree which can be walked through later.
bool
XML_as::parseXML(const std::string& xml_in)
{

    if (xml_in.empty())
    {
        log_error(_("XML data is empty"));
        return false;
    }

    // Clear current data
    clear(); 
    
    initParser();

    xmlNodePtr firstNode; 

    xmlDocPtr doc = xmlReadMemory(xml_in.c_str(), xml_in.size(), NULL, NULL, getXMLOptions()); // do NOT recover here !
    if ( doc )
    {
        firstNode = doc->children; // xmlDocGetRootElement(doc);
    }
    else
    {
        log_debug(_("malformed XML, trying to recover"));
        int ret = xmlParseBalancedChunkMemoryRecover(NULL, NULL, NULL, 
                0, (const xmlChar*)xml_in.c_str(), &firstNode, 1);
        log_debug("xmlParseBalancedChunkMemoryRecover returned %d", ret);
        if ( ! firstNode )
        {
            log_error(_("unrecoverable malformed XML "
                        "(xmlParseBalancedChunkMemoryRecover returned "
                        "%d)."), ret);
            return false;
        }
        else
        {
            log_error(_("recovered malformed XML."));
        }
    }



    bool ret = parseDoc(firstNode, true);

    xmlCleanupParser();
    if ( doc ) xmlFreeDoc(doc); // TOCHECK: can it be freed before ?
    else if ( firstNode ) xmlFreeNodeList(firstNode);
    xmlMemoryDump();

    return ret;
  
}


bool
XML_as::onLoad()
{
    log_debug(_("%s: FIXME: onLoad Default event handler"), __FUNCTION__);

    return(_loaded);
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
    const int flags = 0;

    // No flags:
    o.init_member("addRequestHeader", new builtin_function(
                LoadableObject::loadableobject_addRequestHeader), flags);
    o.init_member("createElement", new builtin_function(xml_createelement), flags);
    o.init_member("createTextNode", new builtin_function(xml_createtextnode), flags);
    o.init_member("getBytesLoaded", new builtin_function(xml_getbytesloaded), flags);
    o.init_member("getBytesTotal", new builtin_function(xml_getbytestotal), flags);
    o.init_member("load", new builtin_function(
                LoadableObject::loadableobject_load), flags);
    o.init_member("parseXML", new builtin_function(xml_parsexml), flags);
    o.init_member("send", new builtin_function(xml_send), flags);
    o.init_member("sendAndLoad", new builtin_function(
                LoadableObject::loadableobject_sendAndLoad), flags);
    o.init_member("onData", new builtin_function(xml_ondata), flags);

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
    as_value inum;
    boost::intrusive_ptr<XML_as> xml_obj;
  
    if ( fn.nargs > 0 )
    {
        if ( fn.arg(0).is_object() )
        {
            boost::intrusive_ptr<as_object> obj = fn.arg(0).to_object();
            xml_obj = dynamic_cast<XML_as*>(obj.get());
            if ( xml_obj )
            {
                log_debug(_("Cloned the XML object at %p"),
                       (void *)xml_obj.get());
                return as_value(xml_obj->cloneNode(true).get());
            }
        }

        const std::string& xml_in = fn.arg(0).to_string();
        if ( xml_in.empty() )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("First arg given to XML constructor (%s) "
                    "evaluates to the empty string"), fn.arg(0));
            );
        }
        else
        {
            xml_obj = new XML_as(xml_in);
            return as_value(xml_obj.get());
        }
    }

    xml_obj = new XML_as;

    return as_value(xml_obj.get());
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
    
    if (fn.nargs > 0)
    {
        const std::string& text = fn.arg(0).to_string();
        XMLNode *xml_obj = new XMLNode;
        xml_obj->nodeNameSet(text);
        xml_obj->nodeTypeSet(XMLNode::tText);

        return as_value(xml_obj);
        
    }
    else {
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

    if (fn.nargs > 0) {
        const std::string& text = fn.arg(0).to_string();
        XMLNode* xml_obj = new XMLNode;
        xml_obj->nodeValueSet(text);
        xml_obj->nodeTypeSet(XMLNode::tText);
        return as_value(xml_obj);
    }
    else {
        log_error(_("no text for text node creation"));
    }
    return as_value();
}


as_value xml_getbytesloaded(const fn_call& fn)
{
    boost::intrusive_ptr<XML_as> ptr = ensureType<XML_as>(fn.this_ptr);
    long int ret = ptr->getBytesLoaded();
    if ( ret < 0 ) return as_value();
    else return as_value(ret);
}


as_value xml_getbytestotal(const fn_call& fn)
{
    boost::intrusive_ptr<XML_as> ptr = ensureType<XML_as>(fn.this_ptr);
    long int ret = ptr->getBytesTotal();
    if ( ret < 0 ) return as_value();
    else return as_value(ret);
}


as_value xml_parsexml(const fn_call& fn)
{

    boost::intrusive_ptr<XML_as> ptr = ensureType<XML_as>(fn.this_ptr);

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("XML.parseXML() needs one argument");
        );
        return as_value();
    }

    const std::string& text = fn.arg(0).to_string();
    ptr->parseXML(text);
    
    return as_value();
}

/// \brief removes the specified XML object from its parent. Also
/// deletes all descendants of the node.
as_value xml_send(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<XML_as> ptr = ensureType<XML_as>(fn.this_ptr);
    
    ptr->send("");
    return as_value();
}


static as_value
xml_ondata(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;

    as_object* thisPtr = fn.this_ptr.get();
    assert(thisPtr);

    // See http://gitweb.freedesktop.org/?p=swfdec/swfdec.git;a=blob;f=libswfdec/swfdec_initialize.as

    as_value src; src.set_null();
    if ( fn.nargs ) src = fn.arg(0);

    if ( ! src.is_null() )
    {
        thisPtr->set_member(NSV::PROP_LOADED, true);
        thisPtr->callMethod(NSV::PROP_PARSE_XML, src);
        thisPtr->callMethod(NSV::PROP_ON_LOAD, true);
    }
    else
    {
        thisPtr->set_member(NSV::PROP_LOADED, true);
        thisPtr->callMethod(NSV::PROP_ON_LOAD, true);
    }

    return as_value();
}

// extern (used by Global.cpp)
void xml_class_init(as_object& global)
{

    static boost::intrusive_ptr<builtin_function> cl;

    if ( cl == NULL )
    {
        cl=new builtin_function(&xml_new, getXMLInterface());
    }
    
    global.init_member("XML", cl.get());

}


void
XML_as::initParser()
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
XML_as::clear()
{
    // TODO: should set childs's parent to NULL ?
    _children.clear();

    _attributes.clear();
}

/*private*/
bool
XML_as::ignoreWhite() const
{

    string_table::key propnamekey = _vm.getStringTable().find("ignoreWhite");
    as_value val;
    if (!const_cast<XML_as*>(this)->get_member(propnamekey, &val) ) return false;
    return val.to_bool();
}

/*private*/
int
XML_as::getXMLOptions() const
{
    int options = XML_PARSE_NOENT
        //| XML_PARSE_RECOVER -- don't recover now, we'll call xmlParseBalancedChunkRecover later
        //| XML_PARSE_NOWARNING
            //| XML_PARSE_NOERROR
        | XML_PARSE_NOCDATA;
    // Using libxml2 to convert CDATA nodes to text seems to be what is
    // required.
    
    if ( ignoreWhite() )
    {
        // This doesn't seem to work, so the blanks skipping
        // is actually implemented in XML::extractNode instead.
            //log_debug("Adding XML_PARSE_NOBLANKS to options");
            options |= XML_PARSE_NOBLANKS;
    }

    return options;
}

} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
