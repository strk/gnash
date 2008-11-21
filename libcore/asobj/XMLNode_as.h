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

#ifndef GNASH_ASOBJ_XML_NODE_H
#define GNASH_ASOBJ_XML_NODE_H

//#define DEBUG_MEMORY_ALLOCATION 1

#include "smart_ptr.h" // GNASH_USE_GC
#include "action.h"
#include "impl.h"
#include "log.h"

#include <list>
#include <vector>
#include <string>
#include <sstream>

namespace gnash {  
 
/// XMLNode  ActionScript class
//
/// This is also the base class for the XML actionscript class (see
/// XML_as.cpp, XML_as.h). Because XML_as also inherits from LoadableObject,
/// this is a virtual base class.
class XMLNode_as : public virtual as_object
{
public:

    enum NodeType {

        /// Element
        Element = 1,

        /// Attribute
        Attribute = 2,

        /// Text
        Text = 3,

        /// CDATA section 
        Cdata = 4,

        /// Entity reference
        EntityRef = 5,
        
        /// Entity
        Entity = 6,
        
        /// Processing instruction
        ProcInstr = 7,
                
        /// Comment
        Comment = 8,

        /// Document
        Document = 9,

        /// Document type
        DocType = 10,

        /// Document fragment
        DocFragment = 11,

        /// Notation
        Notation = 12

    };

    XMLNode_as();

    XMLNode_as(const XMLNode_as &node, bool deep);
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

    /// Set name of this node, but only if it doesn't have a name yet
    //
    /// TODO: check if this is the correct behaviour
    ///
    void nodeNameSet(const std::string& name) { _name = name; }

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

    boost::intrusive_ptr<XMLNode_as> firstChild();
    boost::intrusive_ptr<XMLNode_as> lastChild();
    
    // Use a list for quick erasing
    typedef std::list< boost::intrusive_ptr<XMLNode_as> > Children;

    Children& childNodes() { return _children; }

    XMLNode_as& operator = (XMLNode_as &node) {
        log_debug("%s: \n", __PRETTY_FUNCTION__);
        if (this == &node) return *this;
        _name = node._name;
        _value = node._value;
        _children = node._children;
        _attributes = node._attributes;
        return *this;
    }
    
    XMLNode_as& operator = (XMLNode_as *node)
    {
	    assert(node);
	    return (*this = *node);
    }

    XMLNode_as* previousSibling();
    XMLNode_as* nextSibling();

    /// Copy a node
    //
    /// Method; constructs and returns a new XML node of the same type,
    /// name, value, and attributes as the specified XML object. If deep
    /// is set to true, all child nodes are recursively cloned, resulting
    /// in an exact copy of the original object's document tree. 
    boost::intrusive_ptr<XMLNode_as> cloneNode(bool deep);

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
    /// @param childNode
    ///	   same as XMLNode_as::obj ?
    ///
    void appendChild(boost::intrusive_ptr<XMLNode_as> childNode);

    void setParent(XMLNode_as *node) { _parent = node; };
    XMLNode_as *getParent() { return _parent.get(); };

    /// Insert a node before a node
    //
    /// Method; inserts a new child node into the XML object's child
    /// list, before the beforeNode node. If the beforeNode parameter is
    /// undefined or null, the node is added using the appendChild()
    /// method. If beforeNode is not a child of my_xml, the insertion
    /// fails.
    ///
    /// @param newnoe
    ///     The node to insert, moving from its current tree
    ///
    /// @param beforeWhich
    ///     The node before which to insert the new one.
    ///     Must be a child of this XMLNode or the operation will fail.
    ///
    void insertBefore(boost::intrusive_ptr<XMLNode_as> newnode, 
            boost::intrusive_ptr<XMLNode_as> pos);

    /// Removes the specified XML object from its parent.
    //
    /// Also deletes all descendants of the node.
    /// Make sure to keep an intrusive_ptr against
    /// this instance during operation or the ref-counting
    /// management might destroy it.
    ///
    void removeNode();

    /// Convert the XMLNode to a string
    //
    /// @param o        The ostream to write the string to.
    /// @param encode   Whether to URL encode the node values. This
    ///                 is false by default, as it is only necessary
    ///                 for XML.sendAndLoad.
    virtual void toString(std::ostream& str, bool encode = false) const;

    as_object* getAttributes() { return _attributes; }

    const as_object* getAttributes() const { return _attributes; }

    void setAttribute(const std::string& name, const std::string& value);

protected:

#ifdef GNASH_USE_GC
	/// Mark XMLNode-specific reachable resources and invoke
	/// the parent's class version (markAsObjectReachable)
	//
	/// XMLNode-specific reachable resources are:
	/// 	- The child elements (_children)
	/// 	- The parent elements (_parent)
	///
	virtual void markReachableResources() const;
#endif // GNASH_USE_GC

    Children _children;

private:

    boost::intrusive_ptr<XMLNode_as> _parent;

    as_object* _attributes;

    std::string _name;

    std::string _value;

    NodeType _type;

    std::string _namespaceURI;

    static void stringify(const XMLNode_as& xml, std::ostream& xmlout,
            bool encode);

};

// Initialize the global XMLNode class
void xmlnode_class_init(as_object& global);

// Used by XML_as
as_object* getXMLNodeInterface();

} // gnash namespace


#endif	


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
