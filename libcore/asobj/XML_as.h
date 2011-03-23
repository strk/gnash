// XML_as.h:  ActionScript 3 "XMLDocument" class, for Gnash.
//
//   Copyright (C) 2009, 2010, 2011 Free Software Foundation, Inc.
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

#ifndef GNASH_ASOBJ3_XMLDOCUMENT_H
#define GNASH_ASOBJ3_XMLDOCUMENT_H

#include "XMLNode_as.h"
#include "dsodefs.h"
#include "StringPredicates.h"

#include <map>
#include <string>


namespace gnash {

// Forward declarations
class fn_call;
class URL;

/// Implements XML (AS2) and flash.xml.XMLDocument (AS3) class.
//
/// This class interface is identical in AS3 and AS2; it is probably 
/// included in AS3 for backward compatibility.
//
/// The class definition is necessary because XML is encoded differently
/// in AMF.
class XML_as : public XMLNode_as
{
public:

    typedef std::string::const_iterator xml_iterator;

    enum ParseStatus {
            XML_OK = 0,
            XML_UNTERMINATED_CDATA = -2,
            XML_UNTERMINATED_XML_DECL = -3,
            XML_UNTERMINATED_DOCTYPE_DECL = -4,
            XML_UNTERMINATED_COMMENT = -5,
            XML_UNTERMINATED_ELEMENT = -6,
            XML_OUT_OF_MEMORY = -7,
            XML_UNTERMINATED_ATTRIBUTE = -8,
            XML_MISSING_CLOSE_TAG = -9,
            XML_MISSING_OPEN_TAG = -10
    };

    enum LoadStatus {
        XML_LOADED_UNDEFINED = -1,
        XML_LOADED_FALSE = false,
        XML_LOADED_TRUE = true
    };

    /// Create an XML object.
    //
    /// An XMLDocument is always user-created, so always starts with an
    /// associated object.
    XML_as(as_object& object);

    XML_as(as_object& object, const std::string& xml);

    ~XML_as() {};
    
    /// Convert the XML object to a string
    //
    /// This calls XMLNode::toString after adding an xmlDecl and
    /// docTypeDecl
    //
    /// @param o        The ostream to write the string to.
    /// @param encode   Whether to URL encode the node values.
    void toString(std::ostream& o, bool encode) const;

    const std::string& getXMLDecl() const {
        return _xmlDecl;
    }

    void setXMLDecl(const std::string& xml) {
        _xmlDecl = xml;
    }

    const std::string& getDocTypeDecl() const {
        return _docTypeDecl;
    }

    void setDocTypeDecl(const std::string& docType) {
        _docTypeDecl = docType;
    }

    // Methods

    /// Parses an XML document into the specified XML object tree.
    //
    /// This reads in an XML file from disk and parses into into a memory
    /// resident tree which can be walked through later.
    ///
    /// Calls to this function clear any precedently parsed data.
    ///
    void parseXML(const std::string& xml);

    XMLNode_as* createElement(const std::string& name);

    XMLNode_as* createTextNode(const std::string& name);

    ParseStatus status() const {
        return _status;
    }

    void setStatus(ParseStatus st) {
        _status = st;
    }

    LoadStatus loaded() const {
        return _loaded;
    }

    void setLoaded(LoadStatus st) {
        _loaded = st;
    }

private:

    typedef std::map<std::string, std::string, StringNoCaseLessThan> Attributes;

    void parseTag(XMLNode_as*& node, xml_iterator& it, xml_iterator end);

    void parseAttribute(XMLNode_as* node, xml_iterator& it,
            xml_iterator end, Attributes& attributes);

    void parseDocTypeDecl( xml_iterator& it, xml_iterator end);

    void parseText(XMLNode_as* node, xml_iterator& it, xml_iterator end);

    void parseXMLDecl(xml_iterator& it, xml_iterator end);

    void parseComment(XMLNode_as* node, xml_iterator& it, xml_iterator end);

    void parseCData(XMLNode_as* node, xml_iterator& it, xml_iterator end);
 
    /// Clear all properties.
    //
    /// This removes all children, resets doctype and xml decls, and
    /// sets status to XML.
    void clear();
  
    /// \brief
    /// Return true if ignoreWhite property was set to anything evaluating
    /// to true.
    bool ignoreWhite();

    // -1 if never asked to load anything
    //  0 if asked to load but not yet loaded (or failure)
    //  1 if successfully loaded
    LoadStatus _loaded;

    ParseStatus _status;	
 
    std::string _docTypeDecl;

    std::string _xmlDecl;

};


/// Escape using XML entities.
//
/// Note this is not the same as a URL escape.
void escapeXML(std::string& text);
void unescapeXML(std::string& text);

/// Register the XML class.
void xml_class_init(as_object& where, const ObjectURI& uri);

/// Register XML native functions.
void registerXMLNative(as_object& where);

}	// namespace gnash
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

