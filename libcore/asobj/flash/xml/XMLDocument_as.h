// XMLDocument_as.h:  ActionScript 3 "XMLDocument" class, for Gnash.
//
//   Copyright (C) 2009 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "LoadableObject.h"
#include "xml/XMLNode_as.h"
#include "log.h"
#include "dsodefs.h"
#include "StringPredicates.h"

#include <map>
#include <string>


namespace gnash {

// Forward declarations
class fn_call;
class URL;
class LoaderThread;

/// Implements XML (AS2) and flash.xml.XMLDocument (AS3) class.
//
/// This class interface is identical in AS3 and AS2; it is probably 
/// included in AS3 for backward compatibility.
class XMLDocument_as : public XMLNode_as, public LoadableObject
{
public:

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

    XMLDocument_as();

    XMLDocument_as(const std::string& xml);

    ~XMLDocument_as() {};
    
    static void init(as_object& global, const ObjectURI& uri);
    static void registerNative(as_object& global);

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
    /// This reads in an XML file from disk and parses into into a memory
    /// resident tree which can be walked through later.
    ///
    /// Calls to this function clear any precedently parsed data.
    ///
    void parseXML(const std::string& xml);

    // An event handler that returns a what?
    bool onLoad();

    /// Escape using XML entities.
    //
    /// Note this is not the same as a URL escape.
    static void escape(std::string& text);

    /// Unescape XML entities.
    //
    /// Note this is not the same as a URL unescape.
    static void unescape(std::string& text);

    XMLNode_as* createElement(const std::string& name);

    XMLNode_as* createTextNode(const std::string& name);

private:

    typedef std::map<std::string, std::string> Entities;

    static const Entities& getEntities();

    typedef std::map<std::string, std::string, StringNoCaseLessThan> Attributes;

    void parseTag(XMLNode_as*& node, const std::string& xml, 
            std::string::const_iterator& it);

    void parseAttribute(XMLNode_as* node, const std::string& xml, 
            std::string::const_iterator& it, Attributes& attributes);

    void parseDocTypeDecl(const std::string& xml, 
            std::string::const_iterator& it);

    void parseText(XMLNode_as* node, const std::string& xml, 
            std::string::const_iterator& it);

    void parseXMLDecl(const std::string& xml, 
            std::string::const_iterator& it);

    void parseComment(XMLNode_as* node, const std::string& xml, 
            std::string::const_iterator& it);

    void parseCData(XMLNode_as* node, const std::string& xml, 
            std::string::const_iterator& it);
 
    /// Remove all children
    void clear();
  
    /// \brief
    /// Return true if ignoreWhite property was set to anything evaluating
    /// to true.
    bool ignoreWhite() const;

    // -1 if never asked to load anything
    //  0 if asked to load but not yet loaded (or failure)
    //  1 if successfully loaded
    int _loaded;

    ParseStatus _status;	
 
    std::string _docTypeDecl;

    std::string _xmlDecl;

};

}	// namespace gnash
// GNASH_ASOBJ3_XMLDOCUMENT_H
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

