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

#ifndef GNASH_ASOBJ_XML_H
#define GNASH_ASOBJ_XML_H

#include "action.h"
#include "LoadThread.h"
#include "xmlattrs.h"
#include "xmlnode.h"
#include "log.h"
#include "dsodefs.h"
#include "curl_adapter.h"

#include <vector>
#include <sstream>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>

//#define DEBUG_MEMORY_ALLOCATION 1

namespace gnash {

// Forward declarations
class fn_call;
class URL;
class LoaderThread;

/// XML class and ActionScript object
class XML : public XMLNode
{
public:

    typedef enum {

            /// Parsing was successful
            sOK = 0,

            /// Unterminated CDATA section
            sECDATA = -2,

            /// Unterminated XML declaration
            sEXMLDECL = -3,

            /// Unterminated DOCTYPE declaration
            sEDOCTYPEDECL = -4,

            /// Unterminated comment
            sECOMM = -5,

            /// Malformed XML element structure
            sESTRUCT = -6,

            /// Out of memory
            sEMEM = -7,

            /// Unterminated attribute value
            sEATTR = -8,

            /// Missing close tag (orphaned open tag)
            sEOPENTAG = -9,

            /// Missing start tag (orphaned close tag)
            sECLOSETAG = -10

    } Status;


    XML();
    XML(const std::string& xml_in);
    ~XML();
  
    /// This is overridden to provide the 'status' and 'loaded' members,
    /// which are NOT proper properties !
    /// See actionscript.all/XML.as
    ///
    bool get_member(string_table::key name, as_value *val,
		string_table::key nsname = 0);

    /// This is overridden to provide the 'status' and 'loaded' members,
    /// which are NOT proper properties !
    /// See actionscript.all/XML.as
    ///
    bool set_member(string_table::key name, const as_value& val,
                string_table::key nsname = 0, bool ifFound=false);

    // Methods

    /// Parses an XML document into the specified XML object tree.
    //
    /// This reads in an XML file from disk and parses into into a memory resident
    /// tree which can be walked through later.
    ///
    /// Calls to this function clear any precedently parsed data.
    ///
    bool parseXML(const std::string& xml_in);

    // Loads a document (specified by
    // the XML object) from a URL.
    //
    // @param url
    // 
    // @param env
    // 	The environment to use for calling event hadlers
    //	TODO: what about 'this' pointer?
    //
    bool load(const URL& url);

    // An event handler that returns a
    bool onLoad();

    // Boolean value indicating whether
    // the XML object was successfully
    // loaded with XML.load() or
    // XML.sendAndLoad().
    bool loaded()    { return _loaded; }
    
    XMLNode *processNode(xmlTextReaderPtr reader, XMLNode *node);

    void change_stack_frame(int frame, gnash::as_object *xml, gnash::as_environment *env);

    void cleanupStackFrames( XMLNode *data);

    void addRequestHeader(const curl_adapter::RequestHeader::value_type&);

    XMLNode *createElement(const char *name);

    XMLNode *createTextNode(const char *name);

    void send();

    /// ActionScript doesn't care about the success of the connection.
    void sendAndLoad(const URL& url, as_object& target);

    /// @return -1 if no loaded was started yet
    long int getBytesLoaded() const;

    /// @return -1 if no loaded was started yet
    long int getBytesTotal() const;

private:

    /// Remove all childs
    void clear();
  
    /// \brief
    /// Return true if ignoreWhite property was set to anythign evaluating
    /// to true.
    bool ignoreWhite() const;

    /// Return the libxml2 options to use during parsing.
    //
    /// The options might depend on current XML object state, like
    /// the 'ignoreWhite' parameter.
    ///
    /// See http://xmlsoft.org/html/libxml-parser.html#xmlParserOption
    ///
    int getXMLOptions() const;

    /// Read in an XML document from the specified source
    //
    /// This is the base method used by both parseXML() and load().
    ///
    bool parseDoc(xmlNodePtr startNode, bool mem);

    // -1 if never asked to load anything
    //  0 if asked to load but not yet loaded (or failure)
    //  1 if successfully loaded
    int _loaded;

    Status      _status;	

    /// Initialize an XMLNode from an xmlNodePtr
    //
    /// @param element
    ///     The XMLNode to initialize.
    ///
    /// @return false if the xmlNodePtr shouldn't exist
    /// (ie: an all-blanks node with ignoreWhite set to true).
    ///
    bool extractNode(XMLNode& element, xmlNodePtr node, bool mem);

    void setupFrame(gnash::as_object *xml, XMLNode *data, bool src);
  
    /// Initialize the libxml2 parser
    void initParser();

    /// Queue a load request from the given stream
    void queueLoad(std::auto_ptr<IOChannel> str);

    //static void _xmlErrorHandler(void *ctx, const char* fmt, ...);

    typedef std::list<LoadThread*> LoadThreadList;

    /// Queue of load requests
    LoadThreadList _loadThreads; 

    /// The load checker interval timer used to make loads async
    unsigned int _loadCheckerTimer;

    /// Wrapper around checkLoads for use as a Timer
    /// interval for checking loads
    static as_value checkLoads_wrapper(const fn_call& fn);

    /// Scan the LoadThread queue (_loadThreads) to see if any of
    /// them completed. If any did, invoke the onData event
    void checkLoads();

    long int _bytesTotal;
    long int _bytesLoaded;
    
    curl_adapter::RequestHeader _headers;
    
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
