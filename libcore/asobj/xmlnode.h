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

#include "xmlattrs.h"

#ifdef DEBUG_MEMORY_ALLOCATION
	#include "log.h"
#endif

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>

#include <list>
#include <vector>
#include <string>
#include <sstream>

namespace gnash {  
 
/// XMLNode  ActionScript class
//
/// This is the base class for the XML ActionScript class
///
class DSOLOCAL XMLNode : public gnash::as_object
{
public:


    typedef enum {

        /// Element
        tElement = 1,

        /// Attribute
        tAttribute = 2,

        /// Text
        tText = 3,

        /// CDATA section 
        tCdata = 4,

        /// Entity reference
        tEntityRef = 5,
        
        /// Entity
        tEntity = 6,
        
        /// Processing instruction
        tProcInstr = 7,
                
        /// Comment
        tComment = 8,

        /// Document
        tDocument = 9,

        /// Document type
        tDocType = 10,

        /// Document fragment
        tDocFgarment = 11,

        /// Notation
        tNotation = 12

    } NodeType;

    XMLNode();

    // This constructor is used by the XML class
    XMLNode(as_object* overridden_interface);

    XMLNode(const XMLNode &node, bool deep);
    ~XMLNode();

    size_t length() const { return _children.size(); }

    const std::string& nodeName() const { return _name; }

    const std::string& nodeValue() const { return _value; }

    /// Get the type of an XML Node.
    NodeType nodeType() const { return _type; }

    /// Set the type of an XML Node.
    void nodeTypeSet(NodeType type)
    {
	    _type = type;
    }

    /// Set name of this node, but only if it doesn't have a name yet
    //
    /// TODO: check if this is the correct behaviour
    ///
    void nodeNameSet(const std::string& name) { _name = name; }

    /// Set value of this node, overriding any previous value
    void nodeValueSet(const std::string& value) { _value = value; }
    //  nodeType 	XML.nodeType

    ///  Returns true if the specified node has child nodes; otherwise, returns false.
    bool hasChildNodes();

    boost::intrusive_ptr<XMLNode> firstChild();
    boost::intrusive_ptr<XMLNode> lastChild();
    
    // Use a list for quick erasing
    typedef std::list< boost::intrusive_ptr<XMLNode> > ChildList;

    typedef std::vector< XMLAttr > AttribList;

    ChildList& childNodes() { return _children; }

    AttribList& attributes() { return _attributes; }
    
    XMLNode& operator = (XMLNode &node) {
        gnash::log_debug("%s: \n", __PRETTY_FUNCTION__);
        if (this == &node) return *this;
        _name = node._name;
        _value = node._value;
        _children = node._children;
        _attributes = node._attributes;
        return *this;
    }
    
    XMLNode& operator = (XMLNode *node)
    {
	    assert(node);
	    return (*this = *node);
    }

    XMLNode* previousSibling();
    XMLNode* nextSibling();

    /// Copy a node
    //
    /// Method; constructs and returns a new XML node of the same type,
    /// name, value, and attributes as the specified XML object. If deep
    /// is set to true, all child nodes are recursively cloned, resulting
    /// in an exact copy of the original object's document tree. 
    ///
    boost::intrusive_ptr<XMLNode> cloneNode(bool deep);

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
    /// @param as
    ///	   The XMLNode ?
    ///
    /// @param node
    ///	   same as XMLNode::obj ?
    ///
    void appendChild(boost::intrusive_ptr<XMLNode> childNode);

    void setParent(XMLNode *node) { _parent = node; };
    XMLNode *getParent() { return _parent.get(); };

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
    void insertBefore(boost::intrusive_ptr<XMLNode> newnode, boost::intrusive_ptr<XMLNode> pos);

    /// Removes the specified XML object from its parent.
    //
    /// Also deletes all descendants of the node.
    /// Make sure to keep an intrusive_ptr against
    /// this instance during operation or the ref-counting
    /// management might destroy it.
    ///
    void removeNode();

    void toString(std::ostream& str) const;

    //void  change_stack_frame(int frame, gnash::as_object *xml, gnash::as_environment *env);

    // We might turn this back to a dumb pointer, as long
    // as we'll make sure in the XMLNode destructor and
    // any child cleaning interface to set child parent
    // to NULL
    boost::intrusive_ptr<XMLNode> _parent;

    ChildList       _children;
    AttribList      _attributes;

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

private:

    // TODO: make a lot more things private !

    std::string _name;

    std::string _value;

    NodeType     _type;

    static void stringify(const XMLNode& xml, std::ostream& xmlout);

};

// Initialize the global XMLNode class
void xmlnode_class_init(as_object& global);

// External, used by getXMLInterface() !
as_object* getXMLNodeInterface();

} // end of gnash namespace


#endif	// __XMLNODE_NODE_H__


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
