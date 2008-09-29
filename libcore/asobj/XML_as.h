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

#include "LoadableObject.h"
#include "xmlnode.h"
#include "log.h"
#include "dsodefs.h"

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
class XML_as : public XMLNode, public LoadableObject
{
public:

    enum ParseStatus {

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

    };


    XML_as();
    XML_as(const std::string& xml_in);
    ~XML_as() {};

    /// Convert the XML object to a string
    //
    /// This calls XMLNode::toString.
    /// @param o        The ostream to write the string to.
    /// @param encode   Whether to URL encode the node values.
    void toString(std::ostream& o, bool encode) const
    {
        XMLNode::toString(o, encode);
    }

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

    // An event handler that returns a
    bool onLoad();

    XMLNode *processNode(xmlTextReaderPtr reader, XMLNode *node);

    void change_stack_frame(int frame, gnash::as_object *xml, gnash::as_environment *env);

    XMLNode *createElement(const char *name);

    XMLNode *createTextNode(const char *name);

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

    ParseStatus _status;	

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
    
};


DSOEXPORT void xml_class_init(as_object& global);

}	// end namespace gnash

#endif


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
