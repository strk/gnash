// XML_as.cpp:  ActionScript "XMLDocument" class, for Gnash.
//
//   Copyright (C) 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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

#include "XMLNode_as.h"

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string/compare.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "log.h"
#include "as_function.h" 
#include "fn_call.h"
#include "Global_as.h"
#include "LoadableObject.h"
#include "XML_as.h"
#include "NativeFunction.h"
#include "VM.h"
#include "namedStrings.h"
#include "StringPredicates.h"
#include "Object.h"

namespace gnash {

// Forward declarations
namespace {

    as_value xml_new(const fn_call& fn);
    as_value xml_createElement(const fn_call& fn);
    as_value xml_createTextNode(const fn_call& fn);
    as_value xml_parseXML(const fn_call& fn);
    as_value xml_onData(const fn_call& fn);
    as_value xml_xmlDecl(const fn_call& fn);
    as_value xml_docTypeDecl(const fn_call& fn);
    as_value xml_contentType(const fn_call& fn);
    as_value xml_escape(const fn_call& fn);
    as_value xml_loaded(const fn_call& fn);
    as_value xml_status(const fn_call& fn);
    as_value xml_ignoreWhite(const fn_call& fn);

    typedef XML_as::xml_iterator xml_iterator;

    bool textAfterWhitespace(xml_iterator& it, xml_iterator end);
    bool textMatch(xml_iterator& it, xml_iterator end,
            const std::string& match, bool advance = true);
    bool parseNodeWithTerminator( xml_iterator& it, xml_iterator end,
            const std::string& terminator, std::string& content);

    void setIdMap(as_object& xml, XMLNode_as& childNode,
            const std::string& val);
	
	
    typedef std::map<std::string, std::string> Entities;
    const Entities& getEntities();

    void attachXMLProperties(as_object& o);
	void attachXMLInterface(as_object& o);
}


XML_as::XML_as(as_object& object) 
    :
    XMLNode_as(getGlobal(object)),
    _loaded(XML_LOADED_UNDEFINED), 
    _status(XML_OK),
    _contentType("application/x-www-form-urlencoded"),
    _ignoreWhite(false)
{
    setObject(&object);
}

// Parse the ASCII XML string into an XMLNode tree
XML_as::XML_as(as_object& object, const std::string& xml)
    :
    XMLNode_as(getGlobal(object)),
    _loaded(XML_LOADED_UNDEFINED), 
    _status(XML_OK),
    _contentType("application/x-www-form-urlencoded"),
    _ignoreWhite(false)
{
    setObject(&object);
    parseXML(xml);
}

void
escapeXML(std::string& text)
{
    const Entities& ent = getEntities();

    for (Entities::const_iterator i = ent.begin(), e = ent.end();
            i != e; ++i)
    {
        boost::replace_all(text, i->second, i->first);
    }
}

void
unescapeXML(std::string& text)
{
    const Entities& ent = getEntities();

    for (Entities::const_iterator i = ent.begin(), e = ent.end();
            i != e; ++i) {
        boost::replace_all(text, i->first, i->second);
    }

    // Additionally, the &nbsp; entity is unescaped (but never escaped).
    // Note we do this as UTF-8, which is most likely wrong for SWF5.
    boost::replace_all(text, "&nbsp;", "\xc2\xa0");
}

void
XML_as::toString(std::ostream& o, bool encode) const
{
    if (!_xmlDecl.empty()) o << _xmlDecl;
    if (!_docTypeDecl.empty()) o << _docTypeDecl;

    XMLNode_as* i = firstChild();
    while (i) {
        i->XMLNode_as::toString(o, encode);
        i = i->nextSibling();
    }
}

void
XML_as::parseAttribute(XMLNode_as* node, xml_iterator& it,
        const xml_iterator end, Attributes& attributes)
{
    const std::string terminators("\r\t\n >=");

    xml_iterator ourend = std::find_first_of(it, end,
            terminators.begin(), terminators.end());

    if (ourend == end) {
        _status = XML_UNTERMINATED_ELEMENT;
        return;
    }
    std::string name(it, ourend);
    
    if (name.empty()) {
        _status = XML_UNTERMINATED_ELEMENT;
        return;
    }

    // Point iterator to the DisplayObject after the name.
    it = ourend;

    // Skip any whitespace before the '='. If we reach the end of the string
    // or don't find an '=', it's a parser error.
    if (!textAfterWhitespace(it, end) || *it != '=') {
        _status = XML_UNTERMINATED_ELEMENT;
        return;
    }

    // Point to the DisplayObject after the '='
    ++it;

    // Skip any whitespace. If we reach the end of the string, or don't find
    // a " or ', it's a parser error.
    if (!textAfterWhitespace(it, end) || (*it != '"' && *it != '\'')) {
        _status = XML_UNTERMINATED_ELEMENT;
        return;
    }

    // Find the end of the attribute, looking for the opening DisplayObject,
    // as long as it's not escaped. We begin one after the present position,
    // which should be the opening DisplayObject. We want to remember what the
    // iterator is pointing to for a while, so don't advance it.
    ourend = it;
    do {
        ++ourend;
        ourend = std::find(ourend, end, *it);
    } while (ourend != end && *(ourend - 1) == '\\');

    if (ourend == end) {
        _status = XML_UNTERMINATED_ATTRIBUTE;
        return;
    }
    ++it;

    std::string value(it, ourend);

    // Replace entities in the value.
    unescapeXML(value);

    // We've already checked that ourend != end, so we can advance at 
    // least once.
    it = ourend;
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
XML_as::parseDocTypeDecl(xml_iterator& it, const xml_iterator end)
{

    xml_iterator ourend;
    xml_iterator current = it; 

    std::string::size_type count = 1;

    // Look for angle brackets in the doctype declaration.
    while (count) {

        // Find the next closing bracket after the current position.
        ourend = std::find(current, end, '>');
        if (ourend == end) {
            _status = XML_UNTERMINATED_DOCTYPE_DECL;
            return;
        }
        --count;

        // Count any opening brackets in between.
        count += std::count(current, ourend, '<');
        current = ourend;
        ++current;
    }

    const std::string content(it, ourend);
    std::ostringstream os;
    os << '<' << content << '>';
    _docTypeDecl = os.str();
    it = ourend + 1;
}

void
XML_as::parseXMLDecl(xml_iterator& it, const xml_iterator end)
{
    std::string content;
    if (!parseNodeWithTerminator(it, end, "?>", content)) {
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
XML_as::parseTag(XMLNode_as*& node, xml_iterator& it,
        const xml_iterator end)
{
    bool closing = (*it == '/');
    if (closing) ++it;

    // These are for terminating the tag name, not (necessarily) the tag.
    const std::string terminators("\r\n\t >");

    xml_iterator endName = std::find_first_of(it, end, terminators.begin(),
            terminators.end());

    // Check that one of the terminators was found; otherwise it's malformed.
    if (endName == end) {
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

        XMLNode_as* childNode = new XMLNode_as(_global);
        childNode->nodeNameSet(tagName);
        childNode->nodeTypeSet(Element);

        // Skip to the end of any whitespace after the tag name
        it = endName;

        if (!textAfterWhitespace(it, end)) {
            _status = XML_UNTERMINATED_ELEMENT;
           return;
        }

        // Parse any attributes in an opening tag only, stopping at "/>" or
        // '>'
        // Attributes are added in reverse order and without any duplicates.
        Attributes attributes;
        while (it != end && *it != '>' && _status == XML_OK)
        {
            if (end - it > 1 && std::equal(it, it + 2, "/>")) break;

            // This advances the iterator
            parseAttribute(childNode, it, end, attributes);

            // Skip any whitespace. If we reach the end of the string,
            // it's malformed.
            if (!textAfterWhitespace(it, end)) {
                _status = XML_UNTERMINATED_ELEMENT;
                return;
            }
        }
        
        // Do nothing more if there was an error in attributes parsing.
        if (_status != XML_OK) return;

        // testsuite/swfdec/xml-id-map.as tests that the node is appended
        // first.
        node->appendChild(childNode);

        for (Attributes::const_reverse_iterator i = attributes.rbegin(),
                e = attributes.rend(); i != e; ++i) {
            childNode->setAttribute(i->first, i->second);
            if (i->first == "id") setIdMap(*object(), *childNode, i->second);
        }

        if (*it == '/') ++it;
        else node = childNode;

        if (*it == '>') ++it;

        return;
    }

    // If we reach here, this is a closing tag.

    it = std::find(endName, end, '>');

    if (it == end) {
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
XML_as::parseText(XMLNode_as* node, xml_iterator& it,
        const xml_iterator end, bool iw)
{
    xml_iterator ourend = std::find(it, end, '<');
    std::string content(it, ourend);
    
    it = ourend;

    if (iw && 
        content.find_first_not_of("\t\r\n ") == std::string::npos) return;

    XMLNode_as* childNode = new XMLNode_as(_global);

    childNode->nodeTypeSet(XMLNode_as::Text);

    // Replace any entitites.
    unescapeXML(content);

    childNode->nodeValueSet(content);
    node->appendChild(childNode);

}

void
XML_as::parseComment(XMLNode_as* /*node*/, xml_iterator& it,
        const xml_iterator end)
{

    std::string content;

    if (!parseNodeWithTerminator(it, end, "-->", content)) {
        _status = XML_UNTERMINATED_COMMENT;
        return;
    }
    // Comments are discarded at least up to SWF8
}

void
XML_as::parseCData(XMLNode_as* node, xml_iterator& it,
        const xml_iterator end)
{
    std::string content;

    if (!parseNodeWithTerminator(it, end, "]]>", content)) {
        _status = XML_UNTERMINATED_CDATA;
        return;
    }

    XMLNode_as* childNode = new XMLNode_as(_global);
    childNode->nodeValueSet(content);
    childNode->nodeTypeSet(Text);
    node->appendChild(childNode);
}


// This parses an XML string into a tree of XMLNodes.
void
XML_as::parseXML(const std::string& xml)
{
    // Clear current data
    clear(); 

    if (xml.empty()) {
        log_error(_("XML data is empty"));
        return;
    }

    xml_iterator it = xml.begin();
    const xml_iterator end = xml.end();
    XMLNode_as* node = this;

    const bool iw = ignoreWhite();

    while (it != end && _status == XML_OK) {
        if (*it == '<') {
            ++it;
            if (textMatch(it, end, "!DOCTYPE", false)) {
                // We should not advance past the DOCTYPE label, as
                // the case is preserved.
                parseDocTypeDecl(it, end);
            }
            else if (textMatch(it, end, "?xml", false)) {
                // We should not advance past the xml label, as
                // the case is preserved.
                parseXMLDecl(it, end);
            }
            else if (textMatch(it, end, "!--")) {
                parseComment(node, it, end);
            }
            else if (textMatch(it, end, "![CDATA[")) {
                parseCData(node, it, end);
            }
            else parseTag(node, it, end);
        }
        else parseText(node, it, end, iw);
    }

    // If everything parsed correctly, check that we've got back to the
    // parent node. If not, there is a missing closing tag.
    if (_status == XML_OK && node != this) {
        _status = XML_MISSING_CLOSE_TAG;
    }

}

void
XML_as::clear()
{
    // TODO: should set childs's parent to NULL ?
    clearChildren();
    _docTypeDecl.clear();
    _xmlDecl.clear();
    _status = XML_OK;
}

// XML.prototype is assigned after the class has been constructed, so it
// replaces the original prototype and does not have a 'constructor'
// property.
void
xml_class_init(as_object& where, const ObjectURI& uri)
{
    Global_as& gl = getGlobal(where);
    as_object* cl = gl.createClass(&xml_new, 0);

    as_function* ctor = getMember(gl, NSV::CLASS_XMLNODE).to_function();

    if (ctor) {
        // XML.prototype is an XMLNode(1, "");
        fn_call::Args args;
        args += 1, "";
        as_object* proto =
            constructInstance(*ctor, as_environment(getVM(where)), args);
        attachXMLInterface(*proto);
        cl->init_member(NSV::PROP_PROTOTYPE, proto);
    }
    
    where.init_member(uri, cl, as_object::DefaultFlags);
}

void
registerXMLNative(as_object& where)
{
    VM& vm = getVM(where);
    vm.registerNative(xml_escape, 100, 5);
    vm.registerNative(xml_createElement, 253, 10);
    vm.registerNative(xml_createTextNode, 253, 11);
    vm.registerNative(xml_parseXML, 253, 12);
}

namespace {

void
attachXMLProperties(as_object& o)
{
    as_object* proto = o.get_prototype();
    if (!proto) return;
    const int flags = 0;
    proto->init_property("docTypeDecl", &xml_docTypeDecl, &xml_docTypeDecl,
            flags);
    proto->init_property("contentType", &xml_contentType, &xml_contentType,
            flags);
    proto->init_property("ignoreWhite", xml_ignoreWhite, xml_ignoreWhite, flags);
    proto->init_property("loaded", xml_loaded, xml_loaded);
    proto->init_property("status", xml_status, xml_status, flags);
    proto->init_property("xmlDecl", &xml_xmlDecl, &xml_xmlDecl, flags);
}


void
attachXMLInterface(as_object& o)
{
    VM& vm = getVM(o);
    Global_as& gl = getGlobal(o);

    const int flags = 0;

    // No flags:
    o.init_member("createElement", vm.getNative(253, 10), flags);
    o.init_member("createTextNode", vm.getNative(253, 11), flags);
    o.init_member("load", vm.getNative(301, 0), flags);
    
    /// This handles getBytesLoaded, getBytesTotal, and addRequestHeader
    attachLoadableInterface(o, flags);
    
    o.init_member("parseXML", vm.getNative(253, 12), flags); 
    o.init_member("send", vm.getNative(301, 1), flags);
    o.init_member("sendAndLoad", vm.getNative(301, 2), flags);
    o.init_member("onData", gl.createFunction(xml_onData), flags);
    o.init_member("onLoad", gl.createFunction(emptyFunction), flags);
}

as_value
xml_new(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);

    if (fn.nargs && !fn.arg(0).is_undefined()) {

        // Copy constructor clones nodes.
        if (fn.arg(0).is_object()) {
            as_object* other = toObject(fn.arg(0), getVM(fn));
            XML_as* xml;
            if (isNativeType(other, xml)) {
                as_object* clone = xml->cloneNode(true)->object();
                attachXMLProperties(*clone);
                return as_value(clone);
            }
        }

        const int version = getSWFVersion(fn);
        const std::string& xml_in = fn.arg(0).to_string(version);
        // It doesn't matter if the string is empty.
        obj->setRelay(new XML_as(*obj, xml_in));
        attachXMLProperties(*obj);
        return as_value();
    }

    obj->setRelay(new XML_as(*obj));
    attachXMLProperties(*obj);

    return as_value();
}

/// This is attached to the prototype (an XMLNode) on construction of XML
//
/// It has the curious effect of giving the XML object an inherited 'loaded'
/// property that fails when called on the prototype, because the prototype
/// is of type XMLNode.
as_value
xml_loaded(const fn_call& fn)
{
    XML_as* ptr = ensure<ThisIsNative<XML_as> >(fn);

    if (!fn.nargs) {
        XML_as::LoadStatus ls = ptr->loaded();
        if (ls == XML_as::XML_LOADED_UNDEFINED) return as_value();
        return as_value(static_cast<bool>(ls));
    }
    ptr->setLoaded(
            static_cast<XML_as::LoadStatus>(toBool(fn.arg(0), getVM(fn))));
    return as_value();
}

as_value
xml_status(const fn_call& fn)
{
    XML_as* ptr = ensure<ThisIsNative<XML_as> >(fn);
    
    if (!fn.nargs) {
        return as_value(ptr->status());
    }

    if (fn.arg(0).is_undefined()) {
        return as_value();
    }

    const double status = toNumber(fn.arg(0), getVM(fn));
    if (isNaN(status) ||
            status > std::numeric_limits<boost::int32_t>::max() ||
            status < std::numeric_limits<boost::int32_t>::min()) {

        ptr->setStatus(static_cast<XML_as::ParseStatus>(
                    std::numeric_limits<boost::int32_t>::min()));
    }
    else ptr->setStatus(static_cast<XML_as::ParseStatus>(int(status)));
    return as_value();
}
    
as_value
xml_ignoreWhite(const fn_call& fn)
{
    XML_as* ptr = ensure<ThisIsNative<XML_as> >(fn);
    if (!fn.nargs) {
        // Getter
        return as_value(ptr->ignoreWhite());
    }

    // Setter
    if (fn.arg(0).is_undefined()) return as_value();
    ptr->ignoreWhite(toBool(fn.arg(0), getVM(fn)));
    return as_value();
}

/// Only available as ASnative.
as_value
xml_escape(const fn_call& fn)
{
    if (!fn.nargs) return as_value();

    std::string escaped = fn.arg(0).to_string();
    escapeXML(escaped);
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
    if (!fn.nargs || fn.arg(0).is_undefined()) {
        return as_value();
    }

    const as_value& arg = fn.arg(0);

    const std::string& text = arg.to_string(getSWFVersion(fn));
    XMLNode_as *xml_obj = new XMLNode_as(getGlobal(fn));
    xml_obj->nodeNameSet(text);
    if (!text.empty()) xml_obj->nodeTypeSet(XMLNode_as::Text);

    return as_value(xml_obj->object());
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
        XMLNode_as* xml_obj = new XMLNode_as(getGlobal(fn));
        xml_obj->nodeValueSet(text);
        xml_obj->nodeTypeSet(XMLNode_as::Text);
        return as_value(xml_obj->object());
    }
    else {
        log_error(_("no text for text node creation"));
    }
    return as_value();
}


as_value
xml_parseXML(const fn_call& fn)
{
    XML_as* ptr = ensure<ThisIsNative<XML_as> >(fn);

    if (fn.nargs < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("XML.parseXML() needs one argument"));
        );
        return as_value();
    }

    const as_value arg = fn.arg(0);
    if (arg.is_undefined()) return as_value();

    const std::string& text = arg.to_string(getSWFVersion(fn));
    ptr->parseXML(text);
    
    return as_value();
}

as_value
xml_xmlDecl(const fn_call& fn)
{
    XML_as* ptr = ensure<ThisIsNative<XML_as> >(fn);

    if (!fn.nargs) {
        // Getter
        const std::string& xml = ptr->getXMLDecl();
        if (xml.empty()) return as_value();
        return as_value(xml);
    }

    // Setter

    const std::string& xml = fn.arg(0).to_string();
    ptr->setXMLDecl(xml);
    
    return as_value();
}

as_value
xml_contentType(const fn_call& fn)
{
    XML_as* ptr = ensure<ThisIsNative<XML_as> >(fn);

    if (!fn.nargs) {
        // Getter
        return as_value(ptr->getContentType());
    }

    // Setter
    const std::string& contentType = fn.arg(0).to_string();
    ptr->setContentType(contentType);
    
    return as_value();
}

as_value
xml_docTypeDecl(const fn_call& fn)
{
    XML_as* ptr = ensure<ThisIsNative<XML_as> >(fn);

    if (!fn.nargs) {
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
xml_onData(const fn_call& fn)
{
    as_object* thisPtr = fn.this_ptr;
    assert(thisPtr);

    // See http://gitweb.freedesktop.org/?p=swfdec/swfdec.git;
    // a=blob;f=libswfdec/swfdec_initialize.as

    as_value src;
    if (fn.nargs) src = fn.arg(0);

    if (!src.is_undefined()) {
        thisPtr->set_member(NSV::PROP_LOADED, true);
        callMethod(thisPtr, NSV::PROP_PARSE_XML, src);
        callMethod(thisPtr, NSV::PROP_ON_LOAD, true);
    }
    else {
        thisPtr->set_member(NSV::PROP_LOADED, false);
        callMethod(thisPtr, NSV::PROP_ON_LOAD, false);
    }

    return as_value();
}

/// Case insensitive match of a string, returning false if there too few
/// DisplayObjects left or if there is no match. If there is a match, and advance
/// is not false, the iterator points to the DisplayObject after the match.
bool
textMatch(xml_iterator& it, const xml_iterator end,
        const std::string& match, bool advance)
{
    const std::string::size_type len = match.length();

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
textAfterWhitespace(xml_iterator& it, const xml_iterator end)
{
    const std::string whitespace("\r\t\n ");
    while (it != end && whitespace.find(*it) != std::string::npos) ++it;
    return (it != end);
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
parseNodeWithTerminator(xml_iterator& it, const xml_iterator end,
        const std::string& terminator, std::string& content)
{
    xml_iterator ourend = std::search(it, end, terminator.begin(),
            terminator.end());

    if (ourend == end) {
        return false;
    }

    content = std::string(it, ourend);
    it = ourend + terminator.length();

    return true;
}


void
setIdMap(as_object& xml, XMLNode_as& childNode, const std::string& val)
{
    VM& vm = getVM(xml);

    const ObjectURI& id = getURI(vm, "idMap");

    if (getSWFVersion(xml) < 8) {
        // In version 7 or below, properties are added to the XML object.
        xml.set_member(getURI(vm, val), childNode.object());
        return;
    }

    // In version 8 or above, properties are added to an idMap member.
    as_value im;
    as_object* idMap;
    if (xml.get_member(id, &im)) {
        // If it's present but not an object just ignore it
        // and carry on.
        if (!im.is_object()) return;

        idMap = toObject(im, vm);
        assert(idMap);
    }
    else {
        // If it's not there at all create it.
        idMap = new as_object(getGlobal(xml));
        xml.set_member(id, idMap);
    }
    idMap->set_member(getURI(vm, val), childNode.object());
}

const Entities&
getEntities()
{
    static const Entities entities = boost::assign::map_list_of
        ("&amp;", "&")
        ("&quot;", "\"")
        ("&lt;", "<")
        ("&gt;", ">")
        ("&apos;", "'");

    return entities;
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

