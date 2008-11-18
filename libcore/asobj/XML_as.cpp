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

// The XML parsing algorithms are based on swfdec.

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "log.h"
#include "as_function.h" // for as_function
#include "fn_call.h"

#include "LoadableObject.h"
#include "xmlattrs.h"
#include "XMLNode_as.h"
#include "XML_as.h"
#include "builtin_function.h"
#include "VM.h"
#include "namedStrings.h"
#include "array.h"
#include "StringPredicates.h"

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string/compare.hpp>


namespace gnash {
  
//#define DEBUG_MEMORY_ALLOCATION 1

// Define this to enable verbosity of XML loads
//#define DEBUG_XML_LOADS 1

// Define this to enable verbosity of XML parsing
//#define DEBUG_XML_PARSE 1
// Forward declarations.
namespace {
    as_object* getXMLInterface();
    void attachXMLInterface(as_object& o);
    void attachXMLProperties(as_object& o);

    as_value xml_new(const fn_call& fn);
    as_value xml_createelement(const fn_call& fn);
    as_value xml_createtextnode(const fn_call& fn);
    as_value xml_getbytesloaded(const fn_call& fn);
    as_value xml_getbytestotal(const fn_call& fn);
    as_value xml_parsexml(const fn_call& fn);
    as_value xml_ondata(const fn_call& fn);

    bool textAfterWhitespace(const std::string& xml,
            std::string::const_iterator& it);
    bool textMatch(const std::string& xml, std::string::const_iterator& it,
            const std::string& match);
    bool parseNodeWithTerminator(const std::string& xml,
            std::string::const_iterator& it, const std::string& terminator,
            std::string& content);
}

XML_as::XML_as() 
    :
    as_object(getXMLInterface()),
    _loaded(-1), 
    _status(XML_OK)
{
#ifdef DEBUG_MEMORY_ALLOCATION
    log_debug(_("Creating XML data at %p"), this);
#endif

    attachXMLProperties(*this);
}


// Parse the ASCII XML string into an XMLNode tree
XML_as::XML_as(const std::string& xml)
    :
    as_object(getXMLInterface()),
    _loaded(-1), 
    _status(XML_OK)
{
#ifdef DEBUG_MEMORY_ALLOCATION
    log_debug(_("Creating XML data at %p"), this);
#endif

    parseXML(xml);
}

const XML_as::Entities&
XML_as::getEntities()
{

    static Entities entities = boost::assign::map_list_of
        ("&amp;", "&")
        ("&apos;", "\"")
        ("&lt;", "<")
        ("&gt;", ">")
        ("&quot;", "'");

    return entities;

}

void
XML_as::escape(std::string& text)
{
    const Entities& ent = getEntities();

    for (Entities::const_iterator i = ent.begin(), e = ent.end();
            i != e; ++i)
    {
        boost::replace_all(text, i->second, i->first);
    }
}

void
XML_as::unescape(std::string& text)
{
    const Entities& ent = getEntities();

    for (Entities::const_iterator i = ent.begin(), e = ent.end();
            i != e; ++i)
    {
        boost::replace_all(text, i->first, i->second);
    }

}

bool
XML_as::get_member(string_table::key name, as_value *val,
        string_table::key nsname)
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
            _status = static_cast<ParseStatus>(
                    std::numeric_limits<boost::int32_t>::min());
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
        if ( b ) _loaded = 1;
        else _loaded = 0;
        return true;
    }

    return as_object::set_member(name, val, nsname, ifFound);
}

void
XML_as::parseAttribute(XMLNode* node, const std::string& xml,
        std::string::const_iterator& it)
{

    const std::string terminators("\r\t\n >=");

    std::string::const_iterator end = std::find_first_of(it, xml.end(),
            terminators.begin(), terminators.end());

    if (end == xml.end()) {
        _status = XML_UNTERMINATED_ELEMENT;
        return;
    }
    std::string name(it, end);

    // Point iterator to the character after the name.
    it = end;

    // Skip any whitespace before the '='. If we reach the end of the string
    // or don't find an '=', it's a parser error.
    if (!textAfterWhitespace(xml, it) || *it != '=') {
        _status = XML_UNTERMINATED_ELEMENT;
        return;
    }

    // Point to the character after the '='
    ++it;

    // Skip any whitespace. If we reach the end of the string, or don't find
    // a " or ', it's a parser error.
    if (!textAfterWhitespace(xml, it) || (*it != '"' && *it != '\'')) {
        _status = XML_UNTERMINATED_ELEMENT;
        return;
    }

    // Find the end of the attribute, looking for the opening character,
    // as long as it's not escaped. We begin one after the present position,
    // which should be the opening character. We want to remember what the
    // iterator is pointing to for a while, so don't advance it.
    end = it;
    do {
        ++end;
        end = std::find(end, xml.end(), *it);
    } while (end != xml.end() && *(end - 1) == '\'');

    if (end == xml.end()) {
        _status = XML_UNTERMINATED_ATTRIBUTE;
        return;
    }
    ++it;

    std::string value(it, end);
    //log_debug("adding attribute to node %s: %s, %s", node->nodeName(),
    //        name, value);

    // We've already checked that end != xml.end(), so we can advance at 
    // least once.
    it = end;
    // Advance past the last attribute character
    ++it;

    // Replace entities in the value.
    escape(value);

    XMLAttr attr(name, value);
    node->_attributes.push_back(attr);

}

void
XML_as::parseDocTypeDecl(XMLNode* node, const std::string& xml,
    std::string::const_iterator& it)
{
    std::string content;
    parseNodeWithTerminator(xml, it, ">", content);
}

void
XML_as::parseXMLDecl(XMLNode* node, const std::string& xml,
    std::string::const_iterator& it)
{
    std::string content;
    parseNodeWithTerminator(xml, it, "?>", content);

    // Handle content.
}

// The iterator should be pointing to the first char after the '<'
void
XML_as::parseTag(XMLNode*& node, const std::string& xml,
    std::string::const_iterator& it)
{
    //log_debug("Processing node: %s", node->nodeName());

    bool closing = (*it == '/');
    if (closing) ++it;

    // These are for terminating the tag name, not (necessarily) the tag.
    const std::string terminators("\r\n\t >");

    std::string::const_iterator endName = std::find_first_of(it, xml.end(),
            terminators.begin(), terminators.end());

    // Check that one of the terminators was found; otherwise it's malformed.
    if (endName == xml.end()) {
        _status = XML_UNTERMINATED_ELEMENT;
        return;
    }

    // Knock off the "/>" of a self-closing tag.
    if (std::equal(endName - 1, endName + 1, "/>")) {
        //log_debug("self-closing tag");
        --endName;
    }

    std::string tagName(it, endName);
    //log_debug("tagName : %s", tagName);

    if (!closing) {

        XMLNode* childNode = new XMLNode;
        childNode->nodeNameSet(tagName);
        childNode->nodeTypeSet(Element);

        log_debug("created childNode with name %s", childNode->nodeName());
        // Skip to the end of any whitespace after the tag name
        it = endName;

        if (!textAfterWhitespace(xml, it)) {
            _status = XML_UNTERMINATED_ELEMENT;
           return;
        }

        // Parse any attributes in an opening tag only, stopping at "/>" or
        // '>'
        while (it != xml.end() && *it != '>' && _status == XML_OK)
        {
            if (xml.end() - it > 1 && std::equal(it, it + 2, "/>")) break;

            // This advances the iterator
            parseAttribute(childNode, xml, it);

            // Skip any whitespace. If we reach the end of the string,
            // it's malformed.
            if (!textAfterWhitespace(xml, it)) {
                _status = XML_UNTERMINATED_ELEMENT;
                return;
            }
        }

        node->appendChild(childNode);
        if (*it == '/') ++it;
        else node = childNode;

        if (*it == '>') ++it;

        return;
    }

    // This may be xml.end(), which is okay.
    it = std::find(endName, xml.end(), '>');

    if (it == xml.end())
    {
       _status = XML_UNTERMINATED_ELEMENT;
       return;
    }
    ++it;

    StringNoCaseEqual noCaseCompare;

    if (node->getParent() && noCaseCompare(node->nodeName(), tagName)) {
        node = node->getParent();
    }
    else {
        // Malformed. Search for the parent node.
        XMLNode* s = node;
        while (s && !noCaseCompare(s->nodeName(), tagName)) {
            //log_debug("parent: %s, this: %s", s->nodeName(), tagName);
            s = s->getParent();
        }
        if (s) {
            // If there's a parent, the open tag is orphaned.
            _status = XML_MISSING_CLOSE_TAG;
        }
        else {
            // If no parent, the close tag is orphaned.
            _status = XML_MISSING_OPEN_TAG;
        }
    }

}

void
XML_as::parseText(XMLNode* node, const std::string& xml, 
        std::string::const_iterator& it)
{
    std::string::const_iterator end = std::find(it, xml.end(), '<');
    std::string content(it, end);
    
    it = end;

    if (ignoreWhite() && 
        content.find_first_not_of("\t\r\n ") == std::string::npos) return;

    XMLNode* childNode = new XMLNode;

    childNode->nodeTypeSet(XMLNode::Text);

    // Replace any entitites.
    unescape(content);

    childNode->nodeValueSet(content);
    node->appendChild(childNode);

    //log_debug("appended text node: %s", content);
}



void
XML_as::parseComment(XMLNode* /*node*/, const std::string& xml, 
        std::string::const_iterator& it)
{
    log_debug("discarding comment node");

    std::string content;

    if (!parseNodeWithTerminator(xml, it, "-->", content)) {
        _status = XML_UNTERMINATED_COMMENT;
        return;
    }
    // Comments are discarded at least up to SWF8
    
}

void
XML_as::parseCData(XMLNode* node, const std::string& xml, 
        std::string::const_iterator& it)
{
    std::string content;

    if (!parseNodeWithTerminator(xml, it, "]]>", content)) {
        _status = XML_UNTERMINATED_CDATA;
        return;
    }

    XMLNode* childNode = new XMLNode;
    childNode->nodeValueSet(content);
    childNode->nodeTypeSet(Text);
    node->appendChild(childNode);
    
}


// This parses an XML string into a tree of XMLNodes.
void
XML_as::parseXML(const std::string& xml)
{
    GNASH_REPORT_FUNCTION; 
    if (xml.empty())
    {
        log_error(_("XML data is empty"));
        return;
    }

    // Clear current data
    clear(); 
    _status = XML_OK;
    

    std::string::const_iterator it = xml.begin();
    XMLNode* node = this;

    while (it != xml.end() && _status == XML_OK)
    {
        if (*it == '<')
        {
            ++it;
            if (textMatch(xml, it, "!DOCTYPE"))
            {
                parseDocTypeDecl(node, xml, it);
            }
            else if (textMatch(xml, it, "?xml"))
            {
                parseXMLDecl(node, xml, it);
            }
            else if (textMatch(xml, it, "!--"))
            {
                parseComment(node, xml, it);
            }
            else if (textMatch(xml, it, "![CDATA["))
            {
                parseCData(node, xml, it);
            }
            else parseTag(node, xml, it);
        }
        else parseText(node, xml, it);
    }
  
    return;
}

bool
XML_as::onLoad()
{
    log_debug(_("%s: FIXME: onLoad Default event handler"), __FUNCTION__);

    return(_loaded);
}


void
XML_as::clear()
{
    // TODO: should set childs's parent to NULL ?
    _children.clear();

    _attributes.clear();
}

bool
XML_as::ignoreWhite() const
{

    string_table::key propnamekey = _vm.getStringTable().find("ignoreWhite");
    as_value val;
    if (!const_cast<XML_as*>(this)->get_member(propnamekey, &val)) return false;
    return val.to_bool();
}


// extern (used by Global.cpp)
void
xml_class_init(as_object& global)
{

    static boost::intrusive_ptr<builtin_function> cl;

    if ( cl == NULL )
    {
        cl=new builtin_function(&xml_new, getXMLInterface());
    }
    
    global.init_member("XML", cl.get());

}

///
/// XML object AS interface.
///

namespace {


void
attachXMLProperties(as_object& /*o*/)
{
    // if we use a proper member here hasOwnProperty() would return true
    // but we want it to return false instead. See XML.as
    //o.init_member("status", as_value(XML::sOK));
}

void
attachXMLInterface(as_object& o)
{
    const int flags = 0;

    // No flags:
    o.init_member("addRequestHeader", new builtin_function(
                LoadableObject::loadableobject_addRequestHeader), flags);
    o.init_member("createElement", 
            new builtin_function(xml_createelement), flags);
    o.init_member("createTextNode", 
            new builtin_function(xml_createtextnode), flags);
    o.init_member("getBytesLoaded", 
            new builtin_function(xml_getbytesloaded), flags);
    o.init_member("getBytesTotal", 
            new builtin_function(xml_getbytestotal), flags);
    o.init_member("load", new builtin_function(
                LoadableObject::loadableobject_load), flags);
    o.init_member("parseXML", new builtin_function(xml_parsexml), flags);
    o.init_member("send", new builtin_function(
                LoadableObject::loadableobject_send), flags);
    o.init_member("sendAndLoad", new builtin_function(
                LoadableObject::loadableobject_sendAndLoad), flags);
    o.init_member("onData", new builtin_function(xml_ondata), flags);

}

as_object*
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
as_value
xml_createelement(const fn_call& fn)
{
    
    if (fn.nargs > 0)
    {
        const std::string& text = fn.arg(0).to_string();
        XMLNode *xml_obj = new XMLNode;
        xml_obj->nodeNameSet(text);
        xml_obj->nodeTypeSet(XMLNode::Text);

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
        xml_obj->nodeTypeSet(XMLNode::Text);
        return as_value(xml_obj);
    }
    else {
        log_error(_("no text for text node creation"));
    }
    return as_value();
}


as_value
xml_getbytesloaded(const fn_call& fn)
{
    boost::intrusive_ptr<XML_as> ptr = ensureType<XML_as>(fn.this_ptr);
    long int ret = ptr->getBytesLoaded();
    if ( ret < 0 ) return as_value();
    else return as_value(ret);
}


as_value
xml_getbytestotal(const fn_call& fn)
{
    boost::intrusive_ptr<XML_as> ptr = ensureType<XML_as>(fn.this_ptr);
    long int ret = ptr->getBytesTotal();
    if ( ret < 0 ) return as_value();
    else return as_value(ret);
}


as_value
xml_parsexml(const fn_call& fn)
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


as_value
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

/// Case insenstive match of a string, returning false if there too few
/// characters left or if there is no match. If there is a match, the
/// iterator points to the character after the match.
bool
textMatch(const std::string& xml, std::string::const_iterator& it,
        const std::string& match)
{

    const std::string::size_type len = match.length();
    const std::string::const_iterator end = xml.end();

    if (static_cast<size_t>(end - it) < len) return false;

    if (!std::equal(it, it + len, match.begin(), boost::is_iequal())) {
        return false;
    }
    it += len;
    return true;
}

/// Advance past whitespace
//
/// @return true if there is text after the whitespace, false if we 
///         reach the end of the string.
bool
textAfterWhitespace(const std::string& xml, std::string::const_iterator& it)
{
    const std::string whitespace("\r\t\n ");
    while (it != xml.end() && whitespace.find(*it) != std::string::npos) ++it;
    return (it != xml.end());
}

/// Parse a complete node up to a specified terminator.
//
/// @return     false if we reach the end of the text before finding the
///             terminator.
/// @param it   The current position of the iterator. If the return is true,
///             this points to the first character after the terminator 
///             after return
/// @param content  If the return is true, this is filled with the content of
///                 the tag.
/// @param xml      The complete XML string.
bool
parseNodeWithTerminator(const std::string& xml, std::string::const_iterator& it,
        const std::string& terminator, std::string& content)
{
    std::string::const_iterator end = std::search(it, xml.end(),
            terminator.begin(), terminator.end());

    if (end == xml.end()) {
        return false;
    }

    content = std::string(it, end);
    it = end + terminator.length();

    return true;
}

} // anonymous namespace
} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
