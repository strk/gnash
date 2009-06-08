// XMLDocument_as.cpp:  ActionScript "XMLDocument" class, for Gnash.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "log.h"
#include "as_function.h" //for as_function
#include "fn_call.h"

#include "LoadableObject.h"
#include "xml/XMLNode_as.h"
#include "xml/XMLDocument_as.h"
#include "builtin_function.h"
#include "VM.h"
#include "namedStrings.h"
#include "StringPredicates.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "GnashException.h" // for ActionException

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string/compare.hpp>
#include <boost/algorithm/string/replace.hpp>


namespace gnash {

// Forward declarations
namespace {
    as_object* getXMLInterface();
	void attachXMLInterface(as_object& o);
	void attachXMLProperties(as_object& o);
	//left as xml to not break everything else (these exist in AS2&3)
    as_value xml_new(const fn_call& fn);
    as_value xml_createElement(const fn_call& fn);
    as_value xml_createTextNode(const fn_call& fn);
    as_value xml_getBytesLoaded(const fn_call& fn);
    as_value xml_getBytesTotal(const fn_call& fn);
    as_value xml_parseXML(const fn_call& fn);
    as_value xml_ondata(const fn_call& fn);
    as_value xml_xmlDecl(const fn_call& fn);
    as_value xml_docTypeDecl(const fn_call& fn);
    as_value xml_escape(const fn_call& fn);

    bool textAfterWhitespace(const std::string& xml,
            std::string::const_iterator& it);
    bool textMatch(const std::string& xml, std::string::const_iterator& it,
            const std::string& match, bool advance = true);
    bool parseNodeWithTerminator(const std::string& xml,
            std::string::const_iterator& it, const std::string& terminator,
            std::string& content);
	
	
    as_value xmldocument_ctor(const fn_call& fn);
    void attachXMLProperties(as_object& /*o*/);
	void attachXMLInterface(as_object& o);
    as_object* getXMLInterface();

}

XMLDocument_as::XMLDocument_as() 
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
XMLDocument_as::XMLDocument_as(const std::string& xml)
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

const XMLDocument_as::Entities&
XMLDocument_as::getEntities()
{

    static Entities entities = boost::assign::map_list_of
        ("&amp;", "&")
        ("&quot;", "\"")
        ("&lt;", "<")
        ("&gt;", ">")
        ("&apos;", "'");

    return entities;

}

void
XMLDocument_as::escape(std::string& text)
{
    const Entities& ent = getEntities();

    for (Entities::const_iterator i = ent.begin(), e = ent.end();
            i != e; ++i)
    {
        boost::replace_all(text, i->second, i->first);
    }
}

void
XMLDocument_as::unescape(std::string& text)
{
    const Entities& ent = getEntities();

    for (Entities::const_iterator i = ent.begin(), e = ent.end();
            i != e; ++i)
    {
        boost::replace_all(text, i->first, i->second);
    }

}

void
XMLDocument_as::toString(std::ostream& o, bool encode) const
{
    if (!_xmlDecl.empty()) o << _xmlDecl;
    if (!_docTypeDecl.empty()) o << _docTypeDecl;

    XMLNode_as::toString(o, encode);
}

bool
XMLDocument_as::get_member(string_table::key name, as_value *val,
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
XMLDocument_as::set_member(string_table::key name, const as_value& val, 
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
XMLDocument_as::parseAttribute(XMLNode_as* node, const std::string& xml,
        std::string::const_iterator& it, Attributes& attributes)
{

    const std::string terminators("\r\t\n >=");

    std::string::const_iterator end = std::find_first_of(it, xml.end(),
            terminators.begin(), terminators.end());

    if (end == xml.end()) {
        _status = XML_UNTERMINATED_ELEMENT;
        return;
    }
    std::string name(it, end);
    
    if (name.empty()) {
        _status = XML_UNTERMINATED_ELEMENT;
        return;
    }

    // Point iterator to the DisplayObject after the name.
    it = end;

    // Skip any whitespace before the '='. If we reach the end of the string
    // or don't find an '=', it's a parser error.
    if (!textAfterWhitespace(xml, it) || *it != '=') {
        _status = XML_UNTERMINATED_ELEMENT;
        return;
    }

    // Point to the DisplayObject after the '='
    ++it;

    // Skip any whitespace. If we reach the end of the string, or don't find
    // a " or ', it's a parser error.
    if (!textAfterWhitespace(xml, it) || (*it != '"' && *it != '\'')) {
        _status = XML_UNTERMINATED_ELEMENT;
        return;
    }

    // Find the end of the attribute, looking for the opening DisplayObject,
    // as long as it's not escaped. We begin one after the present position,
    // which should be the opening DisplayObject. We want to remember what the
    // iterator is pointing to for a while, so don't advance it.
    end = it;
    do {
        ++end;
        end = std::find(end, xml.end(), *it);
    } while (end != xml.end() && *(end - 1) == '\\');

    if (end == xml.end()) {
        _status = XML_UNTERMINATED_ATTRIBUTE;
        return;
    }
    ++it;

    std::string value(it, end);

    // Replace entities in the value.
    unescape(value);

    //log_debug("adding attribute to node %s: %s, %s", node->nodeName(),
    //        name, value);

    // We've already checked that end != xml.end(), so we can advance at 
    // least once.
    it = end;
    // Advance past the last attribute DisplayObject
    ++it;

    // Handle namespace. This is set once only for each node, and is also
    // pushed to the attributes list once.
    StringNoCaseEqual noCaseCompare;
    if (noCaseCompare(name, "xmlns") || noCaseCompare(name, "xmlns:")) {
        if (!node->getNamespaceURI().empty()) return;
        node->setNamespaceURI(value);
    }

    // This ensures values are not inserted twice, which is expected
    // behaviour
    attributes.insert(std::make_pair(name, value));

}

/// Parse and set the docTypeDecl. This is stored without any validation and
/// with the same case as in the parsed XML.
void
XMLDocument_as::parseDocTypeDecl(const std::string& xml,
        std::string::const_iterator& it)
{

    std::string::const_iterator end;
    std::string::const_iterator current = it; 

    std::string::size_type count = 1;

    // Look for angle brackets in the doctype declaration.
    while (count) {

        // Find the next closing bracket after the current position.
        end = std::find(current, xml.end(), '>');
        if (end == xml.end()) {
            _status = XML_UNTERMINATED_DOCTYPE_DECL;
            return;
        }
        --count;

        // Count any opening brackets in between.
        count += std::count(current, end, '<');
        current = end;
        ++current;
    }

    const std::string content(it, end);
    std::ostringstream os;
    os << '<' << content << '>';
    _docTypeDecl = os.str();
    it = end + 1;
}


void
XMLDocument_as::parseXMLDecl(const std::string& xml, std::string::const_iterator& it)
{
    std::string content;
    if (!parseNodeWithTerminator(xml, it, "?>", content))
    {
        _status = XML_UNTERMINATED_XML_DECL;
        return;
    }

    std::ostringstream os;
    os << "<" << content << "?>";

    // This is appended to any xmlDecl already there.
    _xmlDecl += os.str();

}

// The iterator should be pointing to the first char after the '<'
void
XMLDocument_as::parseTag(XMLNode_as*& node, const std::string& xml,
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
        // This can leave endName before it, e.g when a self-closing tag is
        // empty ("</>"). This must be checked before trying to construct
        // a string!
        --endName;
    }
    
    // If the tag is empty, the XML counts as malformed. 
    if (it >= endName) {
        _status = XML_UNTERMINATED_ELEMENT;
        return;
    }

    std::string tagName(it, endName);

    if (!closing) {

        XMLNode_as* childNode = new XMLNode_as;
        childNode->nodeNameSet(tagName);
        childNode->nodeTypeSet(Element);

        //log_debug("created childNode with name %s", childNode->nodeName());
        // Skip to the end of any whitespace after the tag name
        it = endName;

        if (!textAfterWhitespace(xml, it)) {
            _status = XML_UNTERMINATED_ELEMENT;
           return;
        }

        // Parse any attributes in an opening tag only, stopping at "/>" or
        // '>'
        // Attributes are added in reverse order and without any duplicates.
        Attributes attributes;
        while (it != xml.end() && *it != '>' && _status == XML_OK)
        {
            if (xml.end() - it > 1 && std::equal(it, it + 2, "/>")) break;

            // This advances the iterator
            parseAttribute(childNode, xml, it, attributes);

            // Skip any whitespace. If we reach the end of the string,
            // it's malformed.
            if (!textAfterWhitespace(xml, it)) {
                _status = XML_UNTERMINATED_ELEMENT;
                return;
            }
        }
        
        // Do nothing more if there was an error in attributes parsing.
        if (_status != XML_OK) return;

        for (Attributes::const_reverse_iterator i = attributes.rbegin(),
                e = attributes.rend(); i != e; ++i) {
            childNode->setAttribute(i->first, i->second);
        }

        node->appendChild(childNode);
        if (*it == '/') ++it;
        else node = childNode;

        if (*it == '>') ++it;

        return;
    }

    // If we reach here, this is a closing tag.

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
        XMLNode_as* s = node;
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
XMLDocument_as::parseText(XMLNode_as* node, const std::string& xml, 
        std::string::const_iterator& it)
{
    std::string::const_iterator end = std::find(it, xml.end(), '<');
    std::string content(it, end);
    
    it = end;

    if (ignoreWhite() && 
        content.find_first_not_of("\t\r\n ") == std::string::npos) return;

    XMLNode_as* childNode = new XMLNode_as;

    childNode->nodeTypeSet(XMLNode_as::Text);

    // Replace any entitites.
    unescape(content);

    childNode->nodeValueSet(content);
    node->appendChild(childNode);

    //log_debug("appended text node: %s", content);
}



void
XMLDocument_as::parseComment(XMLNode_as* /*node*/, const std::string& xml, 
        std::string::const_iterator& it)
{
    //log_debug("discarding comment node");

    std::string content;

    if (!parseNodeWithTerminator(xml, it, "-->", content)) {
        _status = XML_UNTERMINATED_COMMENT;
        return;
    }
    // Comments are discarded at least up to SWF8
    
}

void
XMLDocument_as::parseCData(XMLNode_as* node, const std::string& xml, 
        std::string::const_iterator& it)
{
    std::string content;

    if (!parseNodeWithTerminator(xml, it, "]]>", content)) {
        _status = XML_UNTERMINATED_CDATA;
        return;
    }

    XMLNode_as* childNode = new XMLNode_as;
    childNode->nodeValueSet(content);
    childNode->nodeTypeSet(Text);
    node->appendChild(childNode);
    
}


// This parses an XML string into a tree of XMLNodes.
void
XMLDocument_as::parseXML(const std::string& xml)
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
    XMLNode_as* node = this;

    while (it != xml.end() && _status == XML_OK)
    {
        if (*it == '<')
        {
            ++it;
            if (textMatch(xml, it, "!DOCTYPE", false))
            {
                // We should not advance past the DOCTYPE label, as
                // the case is preserved.
                parseDocTypeDecl(xml, it);
            }
            else if (textMatch(xml, it, "?xml", false))
            {
                // We should not advance past the xml label, as
                // the case is preserved.
                parseXMLDecl(xml, it);
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

    // If everything parsed correctly, check that we've got back to the
    // parent node. If not, there is a missing closing tag.
    if (_status == XML_OK && node != this) {
        _status = XML_MISSING_CLOSE_TAG;
    }

}

bool
XMLDocument_as::onLoad()
{
    log_debug(_("%s: FIXME: onLoad Default event handler"), __FUNCTION__);

    return(_loaded);
}


void
XMLDocument_as::clear()
{
    // TODO: should set childs's parent to NULL ?
    _children.clear();
    _docTypeDecl.clear();
    _xmlDecl.clear();
}

bool
XMLDocument_as::ignoreWhite() const
{

    string_table::key propnamekey = _vm.getStringTable().find("ignoreWhite");
    as_value val;
    if (!const_cast<XMLDocument_as*>(this)->get_member(propnamekey, &val)) {
        return false;
    }
    return val.to_bool();
}


// extern (used by Global.cpp)
void
XMLDocument_as::init(as_object& global)
{

    static boost::intrusive_ptr<builtin_function> cl;

    if ( cl == NULL )
    {
        cl=new builtin_function(&xml_new, getXMLInterface());
    }
    
    global.init_member("XML", cl.get());

}

void
XMLDocument_as::registerNative(as_object& global)
{
    VM& vm = global.getVM();
    vm.registerNative(xml_escape, 100, 5);
    vm.registerNative(xml_createElement, 253, 8);
    vm.registerNative(xml_createTextNode, 253, 9);
    vm.registerNative(xml_parseXML, 253, 10);
}

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

    VM& vm = o.getVM();

    const int flags = 0;

    // No flags:
    o.init_member("addRequestHeader", new builtin_function(
                LoadableObject::loadableobject_addRequestHeader), flags);
    o.init_member("createElement", vm.getNative(253, 8), flags);
    o.init_member("createTextNode", vm.getNative(253, 9), flags);
    o.init_member("getBytesLoaded", 
            new builtin_function(xml_getBytesLoaded), flags);
    o.init_member("getBytesTotal", 
            new builtin_function(xml_getBytesTotal), flags);
    o.init_member("load", vm.getNative(301, 0), flags);
    o.init_member("parseXML", vm.getNative(253, 10), flags); 
    o.init_member("send", vm.getNative(301, 1), flags);
    o.init_member("sendAndLoad", vm.getNative(301, 2), flags);
    o.init_member("onData", new builtin_function(xml_ondata), flags);

    o.init_property("xmlDecl", &xml_xmlDecl, &xml_xmlDecl, flags);
    o.init_property("docTypeDecl", &xml_docTypeDecl, &xml_docTypeDecl, flags);
}

as_object*
getXMLInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( o == NULL )
    {
        o = new as_object(XMLNode_as::getXMLNodeInterface());
        VM::get().addStatic(o.get());
        attachXMLInterface(*o);
    }
    return o.get();
}

as_value
xml_new(const fn_call& fn)
{
    boost::intrusive_ptr<XMLDocument_as> xml_obj;
  
    if ( fn.nargs > 0 )
    {
        if ( fn.arg(0).is_object() )
        {
            boost::intrusive_ptr<as_object> obj = fn.arg(0).to_object();
            xml_obj = dynamic_cast<XMLDocument_as*>(obj.get());
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
            xml_obj = new XMLDocument_as(xml_in);
            return as_value(xml_obj.get());
        }
    }

    xml_obj = new XMLDocument_as;

    return as_value(xml_obj.get());
}


/// Only available as ASnative.
as_value
xml_escape(const fn_call& fn)
{
    if (!fn.nargs) return as_value();

    std::string escaped = fn.arg(0).to_string();
    XMLDocument_as::escape(escaped);
    return as_value(escaped);
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
xml_createElement(const fn_call& fn)
{
    
    if (fn.nargs > 0)
    {
        const std::string& text = fn.arg(0).to_string();
        XMLNode_as *xml_obj = new XMLNode_as;
        xml_obj->nodeNameSet(text);
        xml_obj->nodeTypeSet(XMLNode_as::Text);

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
xml_createTextNode(const fn_call& fn)
{

    if (fn.nargs > 0) {
        const std::string& text = fn.arg(0).to_string();
        XMLNode_as* xml_obj = new XMLNode_as;
        xml_obj->nodeValueSet(text);
        xml_obj->nodeTypeSet(XMLNode_as::Text);
        return as_value(xml_obj);
    }
    else {
        log_error(_("no text for text node creation"));
    }
    return as_value();
}


as_value
xml_getBytesLoaded(const fn_call& fn)
{
    boost::intrusive_ptr<XMLDocument_as> ptr = ensureType<XMLDocument_as>(fn.this_ptr);
    long int ret = ptr->getBytesLoaded();
    if ( ret < 0 ) return as_value();
    else return as_value(ret);
}


as_value
xml_getBytesTotal(const fn_call& fn)
{
    boost::intrusive_ptr<XMLDocument_as> ptr = ensureType<XMLDocument_as>(fn.this_ptr);
    long int ret = ptr->getBytesTotal();
    if ( ret < 0 ) return as_value();
    else return as_value(ret);
}


as_value
xml_parseXML(const fn_call& fn)
{

    boost::intrusive_ptr<XMLDocument_as> ptr = ensureType<XMLDocument_as>(fn.this_ptr);

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
xml_xmlDecl(const fn_call& fn)
{
    boost::intrusive_ptr<XMLDocument_as> ptr = ensureType<XMLDocument_as>(fn.this_ptr);

    if (!fn.nargs)
    {
        // Getter
        const std::string& xml = ptr->getXMLDecl();
        if (xml.empty()) return as_value();
        return as_value(xml);
    }

    // Setter

    const std::string& xml = fn.arg(0).to_string();
    ptr->setDocTypeDecl(xml);
    
    return as_value();

}

as_value
xml_docTypeDecl(const fn_call& fn)
{
    boost::intrusive_ptr<XMLDocument_as> ptr = ensureType<XMLDocument_as>(fn.this_ptr);

    if (!fn.nargs)
    {
        // Getter
        const std::string& docType = ptr->getDocTypeDecl();
        if (docType.empty()) return as_value();
        return as_value(docType);
    }

    // Setter

    const std::string& docType = fn.arg(0).to_string();
    ptr->setDocTypeDecl(docType);
    
    return as_value();

}

as_value
xml_ondata(const fn_call& fn)
{

    as_object* thisPtr = fn.this_ptr.get();
    assert(thisPtr);

    // See http://gitweb.freedesktop.org/?p=swfdec/swfdec.git;
    // a=blob;f=libswfdec/swfdec_initialize.as

    as_value src;
    if (fn.nargs) src = fn.arg(0);

    if (!src.is_undefined()) {
        thisPtr->set_member(NSV::PROP_LOADED, true);
        thisPtr->callMethod(NSV::PROP_PARSE_XML, src);
        thisPtr->callMethod(NSV::PROP_ON_LOAD, true);
    }
    else {
        thisPtr->set_member(NSV::PROP_LOADED, false);
        thisPtr->callMethod(NSV::PROP_ON_LOAD, false);
    }

    return as_value();
}

/// Case insensitive match of a string, returning false if there too few
/// DisplayObjects left or if there is no match. If there is a match, and advance
/// is not false, the iterator points to the DisplayObject after the match.
bool
textMatch(const std::string& xml, std::string::const_iterator& it,
        const std::string& match, bool advance)
{

    const std::string::size_type len = match.length();
    const std::string::const_iterator end = xml.end();

    if (static_cast<size_t>(end - it) < len) return false;

    if (!std::equal(it, it + len, match.begin(), boost::is_iequal())) {
        return false;
    }
    if (advance) it += len;
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
///             this points to the first DisplayObject after the terminator 
///             after return
/// @param content  If the return is true, this is filled with the content of
///                 the tag.
/// @param xml      The complete XML string.
bool
parseNodeWithTerminator(const std::string& xml,
        std::string::const_iterator& it, const std::string& terminator,
        std::string& content)
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

as_value
xmldocument_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new XMLDocument_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

