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
#include "utf8.h" // for BOM stripping

#include "xmlattrs.h"
#include "xmlnode.h"
#include "XML_as.h"
#include "builtin_function.h"
#include "debugger.h"
#include "StreamProvider.h"
#include "URLAccessManager.h"
#include "IOChannel.h"
#include "URL.h"
#include "VM.h"
#include "namedStrings.h"
#include "timers.h" // for setting up timers to check loads
#include "array.h"

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include <string>
#include <sstream>
#include <vector>
#include <boost/algorithm/string/case_conv.hpp>
#include <memory>
#include <functional> // std::make_pair


namespace gnash {
  
//#define DEBUG_MEMORY_ALLOCATION 1

// Define this to enable verbosity of XML loads
//#define DEBUG_XML_LOADS 1

// Define this to enable verbosity of XML parsing
//#define DEBUG_XML_PARSE 1

static as_object* getXMLInterface();
static void attachXMLInterface(as_object& o);
static void attachXMLProperties(as_object& o);

DSOEXPORT as_value xml_new(const fn_call& fn);
static as_value xml_load(const fn_call& fn);
static as_value xml_addRequestHeader(const fn_call& fn);
static as_value xml_createelement(const fn_call& fn);
static as_value xml_createtextnode(const fn_call& fn);
static as_value xml_getbytesloaded(const fn_call& fn);
static as_value xml_getbytestotal(const fn_call& fn);
static as_value xml_parsexml(const fn_call& fn);
static as_value xml_send(const fn_call& fn);
static as_value xml_sendAndLoad(const fn_call& fn);
static as_value xml_ondata(const fn_call& fn);

#ifdef USE_DEBUGGER
static Debugger& debugger = Debugger::getDefaultInstance();
#endif

XML_as::XML_as() 
    :
    XMLNode(getXMLInterface()),
    //_doc(0),
    //_firstChild(0),
    _loaded(-1), 
    _status(sOK),
    _loadThreads(),
    _loadCheckerTimer(0),
    _bytesTotal(-1),
    _bytesLoaded(-1)
{
#ifdef DEBUG_MEMORY_ALLOCATION
    log_debug(_("Creating XML data at %p"), this);
#endif

    attachXMLProperties(*this);
}


// Parse the ASCII XML string into memory
XML_as::XML_as(const std::string& xml_in)
    :
    XMLNode(getXMLInterface()),
    //_doc(0),
    //_firstChild(0),
    _loaded(-1), 
    _status(sOK),
    _loadThreads(),
    _loadCheckerTimer(0),
    _bytesTotal(-1),
    _bytesLoaded(-1)
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

        return get_member_default(name, val, nsname);
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
			_status = static_cast<XML_as::Status>(std::numeric_limits<boost::int32_t>::min());
		}
		else
		{
			unsigned int statusNumber = static_cast<int>(val.to_number());
			_status = XML_as::Status( static_cast<XML_as::Status>(statusNumber) );
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

        return set_member_default(name, val, nsname, ifFound);
}

XML_as::~XML_as()
{
    //GNASH_REPORT_FUNCTION;

    for (LoadThreadList::iterator it = _loadThreads.begin(),
		    e = _loadThreads.end(); it != e; ++it)
    {
        delete *it; // supposedly joins the thread
    }

    if ( _loadCheckerTimer )
    {
        VM& vm = getVM();
        vm.getRoot().clear_interval_timer(_loadCheckerTimer);
    }
    
#ifdef DEBUG_MEMORY_ALLOCATION
    log_debug(_("\tDeleting XML top level node at %p"), this);
#endif
  
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
    //GNASH_REPORT_FUNCTION;

    if (xml_in.empty()) {
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
        int ret = xmlParseBalancedChunkMemoryRecover(NULL, NULL, NULL, 0, (const xmlChar*)xml_in.c_str(), &firstNode, 1);
        log_debug("xmlParseBalancedChunkMemoryRecover returned %d", ret);
        if ( ! firstNode )
        {
            log_error(_("unrecoverable malformed XML (xmlParseBalancedChunkMemoryRecover returned %d)."), ret);
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

void
XML_as::queueLoad(std::auto_ptr<IOChannel> str)
{

    bool startTimer = _loadThreads.empty();

    std::auto_ptr<LoadThread> lt ( new LoadThread(str) );

    // we push on the front to avoid invalidating
    // iterators when queueLoad is called as effect
    // of onData invocation.
    // Doing so also avoids processing queued load
    // request immediately
    // 
    _loadThreads.push_front(lt.get());
#ifdef DEBUG_XML_LOADS
    log_debug("Pushed thread %p to _loadThreads, number of XML load threads now: %d", (void*)lt.get(),  _loadThreads.size());
#endif
    lt.release();


    if ( startTimer )
    {
        boost::intrusive_ptr<builtin_function> loadsChecker = 
            new builtin_function(&XML_as::checkLoads_wrapper);

        std::auto_ptr<Timer> timer(new Timer);
        timer->setInterval(*loadsChecker, 50, this);
        _loadCheckerTimer = getVM().getRoot().add_interval_timer(timer, true);

#ifdef DEBUG_XML_LOADS
        log_debug("Registered XML loads interval %d", _loadCheckerTimer);
#endif
    }

    _bytesLoaded = 0;
    _bytesTotal = -1;

}

long int
XML_as::getBytesLoaded() const
{
    return _bytesLoaded;
}

long int
XML_as::getBytesTotal() const
{
    return _bytesTotal;
}

/* private */
void
XML_as::checkLoads()
{
#ifdef DEBUG_XML_LOADS
    static int call=0;
    log_debug("XML %p checkLoads call %d, _loadThreads: %d", (void *)this, _loadThreads.size(), ++call);
#endif

    if ( _loadThreads.empty() ) return; // nothing to do

    string_table::key onDataKey = NSV::PROP_ON_DATA;

    for (LoadThreadList::iterator it=_loadThreads.begin();
            it != _loadThreads.end(); )
    {
        LoadThread* lt = *it;

        // TODO: notify progress 

	_bytesLoaded = lt->getBytesLoaded();
        _bytesTotal = lt->getBytesTotal();

#ifdef DEBUG_XML_LOADS
        log_debug("XML loads thread %p got %ld/%ld bytes", (void*)lt, lt->getBytesLoaded(), lt->getBytesTotal() );
#endif
        if ( lt->completed() )
        {
            size_t xmlsize = lt->getBytesLoaded();
            boost::scoped_array<char> buf(new char[xmlsize+1]);
            size_t actuallyRead = lt->read(buf.get(), xmlsize);
            if ( actuallyRead != xmlsize )
			{
				// This would be either a bug of LoadThread or an expected
				// possibility which lacks documentation (thus a bug in documentation)
				//
#ifdef DEBUG_XML_LOADS
				log_debug("LoadThread::getBytesLoaded() returned %d but ::read(%d) returned %d",
					xmlsize, xmlsize, actuallyRead);
#endif
			}
            buf[actuallyRead] = '\0';
            // Strip BOM, if any.
            // See http://savannah.gnu.org/bugs/?19915
            utf8::TextEncoding encoding;
            // NOTE: the call below will possibly change 'xmlsize' parameter
            char* bufptr = utf8::stripBOM(buf.get(), xmlsize, encoding);
            if ( encoding != utf8::encUTF8 && encoding != utf8::encUNSPECIFIED )
            {
                log_unimpl("%s to utf8 conversion in XML input parsing", utf8::textEncodingName(encoding));
            }
            as_value dataVal(bufptr); // memory copy here (optimize?)

            it = _loadThreads.erase(it);
            delete lt; // supposedly joins the thread...

            // might push_front on the list..
            callMethod(onDataKey, dataVal);

#ifdef DEBUG_XML_LOADS
            log_debug("Completed load, _loadThreads have now %d elements", _loadThreads.size());
#endif
        }
        else
        {
            ++it;
        }
    }

    if ( _loadThreads.empty() ) 
    {
#ifdef DEBUG_XML_LOADS
        log_debug("Clearing XML load checker interval timer");
#endif
    	VM& vm = getVM();
        vm.getRoot().clear_interval_timer(_loadCheckerTimer);
        _loadCheckerTimer=0;
    }
}

/* private static */
as_value
XML_as::checkLoads_wrapper(const fn_call& fn)
{
#ifdef DEBUG_XML_LOADS
    log_debug("checkLoads_wrapper called");
#endif

	boost::intrusive_ptr<XML_as> ptr = ensureType<XML_as>(fn.this_ptr);
	ptr->checkLoads();
	return as_value();
}

// This reads in an XML file from disk and parses into into a memory resident
// tree which can be walked through later.
bool
XML_as::load(const URL& url)
{
    GNASH_REPORT_FUNCTION;

    // Set a loaded property to false before starting the load.
    set_member(NSV::PROP_LOADED, false);

    std::auto_ptr<IOChannel> str ( StreamProvider::getDefaultInstance().getStream(url) );
    if ( ! str.get() ) 
    {
        log_error(_("Can't load XML file: %s (security?)"), url.str());
        return false;
        // TODO: this is still not correct.. we should still send onData later...
        //as_value nullValue; nullValue.set_null();
        //callMethod(NSV::PROP_ON_DATA, nullValue);
    }

    log_security(_("Loading XML file from url: '%s'"), url.str());
    queueLoad(str);

    return true;
}


bool
XML_as::onLoad()
{
    log_debug(_("%s: FIXME: onLoad Default event handler"), __FUNCTION__);

    return(_loaded);
}

void
XML_as::cleanupStackFrames(XMLNode * /* xml */)
{
    GNASH_REPORT_FUNCTION;
}

void
XML_as::send()
{
    log_unimpl (__FUNCTION__);
}

void
XML_as::sendAndLoad(const URL& url, as_object& target)
{

    /// All objects get a loaded member, set to false.
    target.set_member(NSV::PROP_LOADED, false);

    std::stringstream ss;
    toString(ss);
    const std::string& data = ss.str();

    string_table& st = _vm.getStringTable();
    as_value customHeaders;

    NetworkAdapter::RequestHeaders headers;

    if ( get_member(st.find("_customHeaders"), &customHeaders) )
    {

        /// Read in our custom headers if they exist and are an
        /// array.
        Array_as* array = dynamic_cast<Array_as*>(
                        customHeaders.to_object().get());
                        
        if (array)
        {
            Array_as::const_iterator e = array->end();
            --e;

            for (Array_as::const_iterator i = array->begin(); i != e; ++i)
            {
                // Only even indices can be a header.
                if (i.index() % 2) continue;
                if (! (*i).is_string()) continue;
                
                // Only the immediately following odd number can be a value.
                if (array->at(i.index() + 1).is_string())
                {
                    const std::string& name = (*i).to_string();
                    const std::string& val =
                                array->at(i.index() + 1).to_string();

                    // Values should overwrite existing ones.
                    headers[name] = val;
                }
                
            }
        }
    }

    as_value contentType;
    if ( get_member(st.find("contentType"), &contentType) )
    {
        // This should not overwrite anything set in XML.addRequestHeader();
        headers.insert(std::make_pair("Content-Type", contentType.to_string()));
    }

    std::auto_ptr<IOChannel> stream;

    /// Doesn't matter if the headers are empty.
    stream = StreamProvider::getDefaultInstance().getStream(url, data, headers);

    if (!stream.get()) 
    {
        log_error(_("Can't load XML file: %s (security?)"), url.str());
        return;
        // TODO: this is still not correct.. we should still send onData later...
        //as_value nullValue; nullValue.set_null();
        //callMethod(NSV::PROP_ON_DATA, nullValue);
    }

    log_security(_("Loading XML file from url: '%s'"), url.str());
    target.queueLoad(stream);
    
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
  
    boost::intrusive_ptr<XML_as> xml_obj = ensureType<XML_as>(fn.this_ptr);
  
    if ( ! fn.nargs )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("XML.load(): missing argument"));
        );
        return rv;
    }

    const std::string& filespec = fn.arg(0).to_string();

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
    o.init_member("addRequestHeader", new builtin_function(xml_addRequestHeader));
    o.init_member("createElement", new builtin_function(xml_createelement));
    o.init_member("createTextNode", new builtin_function(xml_createtextnode));
    o.init_member("getBytesLoaded", new builtin_function(xml_getbytesloaded));
    o.init_member("getBytesTotal", new builtin_function(xml_getbytestotal));
    o.init_member("load", new builtin_function(xml_load));
    o.init_member("parseXML", new builtin_function(xml_parsexml));
    o.init_member("send", new builtin_function(xml_send));
    o.init_member("sendAndLoad", new builtin_function(xml_sendAndLoad));
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
    boost::intrusive_ptr<XML_as> xml_obj;
  
    if ( fn.nargs > 0 )
    {
        if ( fn.arg(0).is_object() )
        {
            boost::intrusive_ptr<as_object> obj = fn.arg(0).to_object();
            xml_obj = boost::dynamic_pointer_cast<XML_as>(obj);
            if ( xml_obj )
            {
                log_debug(_("Cloned the XML object at %p"), (void *)xml_obj.get());
                return as_value(xml_obj->cloneNode(true).get());
            }
        }

        const std::string& xml_in = fn.arg(0).to_string();
        if ( xml_in.empty() )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("First arg given to XML constructor (%s) evaluates to the empty string"),
                    fn.arg(0));
            );
        }
        else
        {
            xml_obj = new XML_as(xml_in);
            return as_value(xml_obj.get());
        }
    }

    xml_obj = new XML_as;
    //log_debug(_("\tCreated New XML object at %p"), xml_obj);

    return as_value(xml_obj.get());
}

/// Can take either a two strings as arguments or an array of strings,
/// alternately header and value.
as_value
xml_addRequestHeader(const fn_call& fn)
{
    
	boost::intrusive_ptr<XML_as> ptr = ensureType<XML_as>(fn.this_ptr);   

    string_table& st = ptr->getVM().getStringTable();
    as_value customHeaders;

    Array_as* array;

    if (ptr->get_member(st.find("_customHeaders"), &customHeaders))
    {
        array = dynamic_cast<Array_as*>(customHeaders.to_object().get());
        if (!array)
        {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("XML.addRequestHeader: XML._customHeaders "
                              "is not an array"));
            );
            return as_value();
        }
    }
    else
    {
        array = new Array_as;
        // This property is always initialized on the first call to
        // addRequestHeaders.
        ptr->set_member(st.find("_customHeaders"), array);
    }

    if (fn.nargs == 0)
    {
        // Return after having initialized the _customHeaders array.
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("XML.addRequestHeader requires at least "
                          "one argument"));
        );
        return as_value();
    }
    
    if (fn.nargs == 1)
    {
        boost::intrusive_ptr<as_object> obj = fn.arg(0).to_object();
        boost::intrusive_ptr<Array_as> headerArray =
                        dynamic_cast<Array_as*>(obj.get());
        if (!headerArray)
        {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("XML.addRequestHeader: single argument "
                                "is not an array"));
            );
            return as_value();
        }
        
        // An array with 1 or 0 elements is invalid
        if (headerArray->size() < 2) return as_value();
        
        // Add the new array to the existing one.
        array->concat(*headerArray);
        return as_value();
    }
        
    if (fn.nargs > 2)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("XML.addRequestHeader(%s): arguments after the"
                            "second will be discarded"), ss.str());
        );
    }
    
    // Both arguments must be strings.
    if (!fn.arg(0).is_string() || !fn.arg(1).is_string())
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("XML.addRequestHeader(%s): both arguments "
                        "must be a string"), ss.str());
        );
        return as_value(); 
    }

    // Push both to the _customHeaders array.
    const std::string& name = fn.arg(0).to_string();
    const std::string& val = fn.arg(1).to_string();

    array->callMethod(NSV::PROP_PUSH, fn.arg(0), fn.arg(1));
    
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
    
    if (fn.nargs > 0) {
        const std::string& text = fn.arg(0).to_string();
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

    XMLNode *xml_obj;

    if (fn.nargs > 0) {
        const std::string& text = fn.arg(0).to_string();
        xml_obj = new XMLNode;
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

    as_value	method;
    as_value	val;    
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
    
    ptr->send();
    return as_value();
}

/// Returns true if the arguments are valid, otherwise false. The
/// success of the connection is irrelevant.
/// The second argument must be an object, but does not have to 
/// be an XML object.
static as_value
xml_sendAndLoad(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<XML_as> ptr = ensureType<XML_as>(fn.this_ptr);
    
    if ( fn.nargs < 2 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss;
        fn.dump_args(ss);
        log_aserror(_("XML.sendAndLoad(%s): missing arguments"),
		ss.str());
        );
        return as_value(false);
    }

    const std::string& filespec = fn.arg(0).to_string();

    if (!fn.arg(1).is_object())
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::ostringstream ss;
        fn.dump_args(ss);
        log_aserror(_("XML.sendAndLoad(%s): second argument is not "
                "an object"), ss.str());
        );

        return as_value(false);
    }

    boost::intrusive_ptr<as_object> targetObj = fn.arg(1).to_object();
    assert(targetObj);

    URL url(filespec, get_base_url());

    ptr->sendAndLoad(url, *targetObj);

    return as_value(true);
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

	string_table::key propnamekey = VM::get().getStringTable().find("ignoreWhite");
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
