// XMLNode_as.h:  ActionScript 3 "XMLNode" class, for Gnash.
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

#ifndef GNASH_ASOBJ3_XMLNODE_H
#define GNASH_ASOBJ3_XMLNODE_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "as_object.h"
#include "smart_ptr.h" // GNASH_USE_GC
#include "log.h"

#include <list>
#include <vector>
#include <string>
#include <sstream>

namespace gnash {


/// A node in an XML tree.
//
/// This class has various complications to reduce memory usage when parsing
/// very large XML documents.
//
/// 1. It is a Relay class that can be attached to an as_object.
/// 2. It does not have to have an associated object. This is only created
///    once the XMLNode is accessed in ActionScript.
/// 3. The top node of an XML tree is always accessible in ActionScript, either
///    as an XMLDocument_as or a user-created XMLNode_as.
/// 4. XMLNodes consequently mark their children as reachable, but not their
///    parent.
/// 5. When an XMLNode is destroyed, any children without an associated object
///    are also deleted. Children with an associated object will be destroyed
///    when the GC destroys the object.
class XMLNode_as : public Relay
{
public:

    enum NodeType {
        Element = 1,
        Attribute = 2,
        Text = 3,
        Cdata = 4,
        EntityRef = 5,
        Entity = 6,
        ProcInstr = 7,
        Comment = 8,
        Document = 9,
        DocType = 10,
        DocFragment = 11,
        Notation = 12
    };

    XMLNode_as(Global_as& gl);

    virtual ~XMLNode_as();

    // Initialize the global XMLNode class
    static void init(as_object& where, const ObjectURI& uri);

    /// Register ASnative methods
    static void registerNative(as_object& where);

    size_t length() const { return _children.size(); }

    const std::string& nodeName() const { return _name; }

    const std::string& nodeValue() const { return _value; }

    /// Get the type of an XML Node.
    NodeType nodeType() const { return _type; }

    /// Set the type of an XML Node.
    void nodeTypeSet(NodeType type) {
	    _type = type;
    }

    /// Set name of this node, but only if it doesn't have a name yet
    //
    /// TODO: check if this is the correct behaviour
    ///
    void nodeNameSet(const std::string& name) { _name = name; }

    bool extractPrefix(std::string& prefix);

    /// Set value of this node, overriding any previous value
    void nodeValueSet(const std::string& value) { _value = value; }

    /// Performs a recursive search of node attributes to find a match
    void getNamespaceForPrefix(const std::string& prefix, std::string& ns);

    /// Performs a recursive search of node attributes to find a match
    //
    /// @return false if no match found.
    bool getPrefixForNamespace(const std::string& ns, std::string& prefix);

    void setNamespaceURI(const std::string value) {
        _namespaceURI = value;
    }

    const std::string& getNamespaceURI() const {
        return _namespaceURI;
    }

    ///  Returns true if the specified node has child nodes; otherwise,
    ///  returns false.
    bool hasChildNodes();

    XMLNode_as* firstChild();
    XMLNode_as* lastChild();
    
    // Use a list for quick erasing
    typedef std::list<XMLNode_as*> Children;

    Children& childNodes() { return _children; }

    XMLNode_as* previousSibling();
    XMLNode_as* nextSibling();

    /// Copy a node
    //  UNIMPLEMENTED
    /// Method; constructs and returns a new XML node of the same type,
    /// name, value, and attributes as the specified XML object. If deep
    /// is set to true, all child nodes are recursively cloned, resulting
    /// in an exact copy of the original object's document tree. 
    XMLNode_as* cloneNode(bool deep);

    /// Append a child node the the XML object
    //
    /// Appends the specified node to the XML object's child
    /// list. This method operates directly on the node referenced by the
    /// childNode parameter; it does not append a copy of the node. If the
    /// node to be appended already exists in another tree structure,
    /// appending the node to the new location will remove it from its
    /// current location. If the childNode parameter refers to a node that
    /// already exists in another XML tree structure, the appended child
    /// node is placed in the new tree structure after it is removed from
    /// its existing parent node. 
    ///
    /// @param childNode    The XMLNode_as object to append as a child.

	void appendChild(XMLNode_as* childNode);

    /// Set the parent XMLNode_as of this node.
    //
    /// @param node     The new parent of this node. May be 0.
    void setParent(XMLNode_as* node) { _parent = node; }

    /// Get the parent XMLNode_as of this node. Can be 0.
    XMLNode_as* getParent() const {
        return _parent;
    }

    /// Insert a node before a node
    //
    /// Method; inserts a new child node into the XML object's child
    /// list, before the beforeNode node. If the beforeNode parameter is
    /// undefined or null, the node is added using the appendChild()
    /// method. If beforeNode is not a child of my_xml, the insertion
    /// fails.
    ///
    /// @param newnode
    ///     The node to insert, moving from its current tree
    ///
    /// @param beforeWhich
    ///     The node before which to insert the new one.
    ///     Must be a child of this XMLNode or the operation will fail.
    ///
    void insertBefore(XMLNode_as* newnode, XMLNode_as* pos);

    /// Removes the specified XML object from its parent.
    //
    /// Also deletes all descendants of the node.
    void removeNode();

    /// Convert the XMLNode to a string
    //
    /// @param o        The ostream to write the string to.
    /// @param encode   Whether to URL encode the node values. This
    ///                 is false by default, as it is only necessary
    ///                 for XML.sendAndLoad.
    virtual void toString(std::ostream& str, bool encode = false) const;

    /// Return the attributes object associated with this node.
    as_object* getAttributes() { return _attributes; }

    /// Return a read-only version of this node's attributes object.
    const as_object* getAttributes() const { return _attributes; }

    /// Set a named attribute to a value.
    //
    /// @param name     The name of the attribute to set. If already present,
    ///                 the value is changed. If not present, the attribute is
    ///                 added.
    /// @param value    The value to set the named attribute to.
    void setAttribute(const std::string& name, const std::string& value);

    /// Associate an as_object with this XMLNode_as.
    //
    /// Ownership is immediately be passed to the as_object!
    void setObject(as_object* o) {
        assert(!_object);
        assert(o);
        _object = o;
    }

    /// Return the object associated with this XMLNode_as.
    //
    /// The object will be created if it does not already exist.
    as_object* object();

protected:

    /// Mark reachable elements
    //
    /// These are: children, attributes object, associated as_object.
	virtual void setReachable();

    Global_as& _global;

    /// Clear all children, making sure unreferenced children are deleted.
    //
    /// AS-referenced child nodes will no longer be marked as reachable, so
    /// the GC will remove them on the next run.
    void clearChildren();

private:

    Children _children;

    /// A non-trivial copy-constructor for cloning nodes.
    XMLNode_as(const XMLNode_as &node, bool deep);

    as_object* _object;

    XMLNode_as* _parent;

    as_object* _attributes;

    std::string _name;

    std::string _value;

    NodeType _type;

    std::string _namespaceURI;

    static void stringify(const XMLNode_as& xml, std::ostream& xmlout,
            bool encode);

};

} // gnash namespace

// GNASH_ASOBJ3_XMLNODE_H
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:


