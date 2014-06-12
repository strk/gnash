// XMLNode_as.h:  ActionScript 3 "XMLNode" class, for Gnash.
//
//   Copyright (C) 2009, 2010, 2011. 2012 Free Software Foundation, Inc.
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

#include <list>
#include <string>
#include <cassert>

#include "Relay.h"

namespace gnash {
    class as_object;
    class Global_as;
    struct ObjectURI;
}

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
///    as an XML_as or a user-created XMLNode_as.
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

    size_t length() const { return _children.size(); }

    const std::string& nodeName() const { return _name; }

    const std::string& nodeValue() const { return _value; }

    /// Get the type of an XML Node.
    NodeType nodeType() const { return _type; }

    /// Set the type of an XML Node.
    void nodeTypeSet(NodeType type) {
	    _type = type;
    }

    /// Set name of this node
    void nodeNameSet(const std::string& name) { _name = name; }

    bool extractPrefix(std::string& prefix) const;

    /// Set value of this node
    void nodeValueSet(const std::string& value) { _value = value; }

    /// Performs a recursive search of node attributes to find a match
    void getNamespaceForPrefix(const std::string& prefix, std::string& ns)
        const;

    /// Performs a recursive search of node attributes to find a match
    //
    /// @return false if no match found.
    bool getPrefixForNamespace(const std::string& ns, std::string& prefix)
        const;

    void setNamespaceURI(const std::string& value) {
        _namespaceURI = value;
    }

    const std::string& getNamespaceURI() const {
        return _namespaceURI;
    }

    /// Returns true if 'this' descends from the specified node.
    bool descendsFrom(XMLNode_as* node) const;

    ///  Returns true if the specified node has child nodes; otherwise,
    ///  returns false.
    bool hasChildNodes() const;

    XMLNode_as* firstChild() const;
    XMLNode_as* lastChild() const;
    
    // Use a list for quick erasing
    typedef std::list<XMLNode_as*> Children;

    as_object* childNodes();

    XMLNode_as* previousSibling() const;
    XMLNode_as* nextSibling() const;

    /// Copy a node
    //  
    /// Method; constructs and returns a new XML node of the same type,
    /// name, value, and attributes as the specified XML object. If deep
    /// is set to true, all child nodes are recursively cloned, resulting
    /// in an exact copy of the original object's document tree. 
    XMLNode_as* cloneNode(bool deep) const;

    /// Append a child node to this XML object
    //
    /// The child node's parent is set to this object, the node is added to
    /// this object's children.
    //
    /// The childNodes array will be updated if it exists.
    //
    /// @param node     The node to add as a child
	void appendChild(XMLNode_as* node);

    /// Remove a child node from this XML object
    //
    /// The child node's parent is set to 0, the node is removed from
    /// this object's children.
    //
    /// The childNodes array will be updated if it exists.
    //
    /// @param node     The node to remove.
    void removeChild(XMLNode_as* node);

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
    /// @param pos
    ///     The node before which to insert the new one.
    ///     Must be a child of this XMLNode or the operation will fail.
    ///
    void insertBefore(XMLNode_as* newnode, XMLNode_as* pos);

    /// Convert the XMLNode to a string
    //
    /// @param o        The ostream to write the string to.
    /// @param encode   Whether to URL encode the node values. This
    ///                 is false by default, as it is only necessary
    ///                 for XML.sendAndLoad.
    virtual void toString(std::ostream& str, bool encode = false) const;

    /// Return the attributes object associated with this node.
    as_object* getAttributes() const { return _attributes; }

    /// Set a named attribute to a value.
    //
    /// @param name     The name of the attribute to set. If already present,
    ///                 the value is changed. If not present, the attribute is
    ///                 added.
    /// @param value    The value to set the named attribute to.
    void setAttribute(const std::string& name, const std::string& value);

    /// Associate an as_object with this XMLNode_as.
    //
    /// An XMLNode_as with an associated object is regarded as being owned
    /// by that object, so make sure it is! Using as_object::setRelay will
    /// achieve that.
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

    /// Set the parent XMLNode_as of this node.
    //
    /// @param node     The new parent of this node. May be 0.
    void setParent(XMLNode_as* node) { _parent = node; }

    /// Reset the array of childNodes to match the actual children.
    //
    /// Only called when the XML structure changes, and only once the
    /// childNodes array has been created. Before this point it is not
    /// referenceable, so we don't need to do anything.
    void updateChildNodes();

    /// A non-trivial copy-constructor for cloning nodes.
    XMLNode_as(const XMLNode_as &node, bool deep);

    Children _children;

    as_object* _object;

    XMLNode_as* _parent;

    as_object* _attributes;

    as_object* _childNodes;

    std::string _name;

    std::string _value;

    NodeType _type;

    std::string _namespaceURI;

    static void stringify(const XMLNode_as& xml, std::ostream& xmlout,
            bool encode);

    /// Is GC mark scan in progress ? 
    //
    /// Used to guard against infinite loops
    ///
    bool _gcMarkInProgress;

    /// Class to prevent infinite loops
    //
    /// could probably be replaced with a templated class taking an
    /// object and two values to toggle between.
    /// See also FrameGuard, TargetGuard and PoolGuard
    class GCMarkGuard {
        XMLNode_as* _x;
    public:
        GCMarkGuard(XMLNode_as* x): _x(x) { _x->_gcMarkInProgress = true; }
        ~GCMarkGuard() { _x->_gcMarkInProgress = false; }
    };
    friend class GCMarkGuard;

};

// Initialize the global XMLNode class
void xmlnode_class_init(as_object& where, const ObjectURI& uri);

/// Register ASnative methods
void registerXMLNodeNative(as_object& where);

} // gnash namespace

// GNASH_ASOBJ3_XMLNODE_H
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:


