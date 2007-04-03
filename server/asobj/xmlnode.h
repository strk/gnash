// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// 
//
//

#ifndef __XML_NODE_H__
#define __XML_NODE_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"

//#define DEBUG_MEMORY_ALLOCATION 1
#include <vector>
#include <string>
#include <sstream>

#include "action.h"
#include "impl.h"

#include "xmlattrs.h"

#ifdef DEBUG_MEMORY_ALLOCATION
	#include "log.h"
#endif

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>

namespace gnash {  
 
/// XML Node 
class DSOLOCAL XMLNode : public gnash::as_object
{
public:
    XMLNode();

    // This constructor is used by the XML class
    XMLNode(as_object* overridden_interface);

    XMLNode(const XMLNode &node, bool deep);
    ~XMLNode();

    size_t length() const { return _children.size(); }

    const char* nodeName() const { return _name; }

    const char* nodeValue() const { return _value; }

    int nodeType();
    void nodeTypeSet(xmlElementType type) {
	    _type = type;
    }

    //    char *valueGet();
  
    /// Set name of this node, but only if it doesn't have a name yet
    //
    /// TODO: check if this is the correct behaviour
    ///
    void nodeNameSet(const char *name);

    /// Set value of this node, overriding any previous value
    void nodeValueSet(const char *value);
    //  nodeType 	XML.nodeType

    ///  Returns true if the specified node has child nodes; otherwise, returns false.
    bool hasChildNodes();

    boost::intrusive_ptr<XMLNode> firstChild();
    boost::intrusive_ptr<XMLNode> lastChild();
    
    typedef std::vector< boost::intrusive_ptr<XMLNode> > ChildList;

    typedef std::vector< XMLAttr > AttribList;

    ChildList& childNodes() { return _children; }

    AttribList& attributes() { return _attributes; }
    
    boost::intrusive_ptr<XMLNode> operator [] (int x)
    {
        log_msg("%s: get element %d", __PRETTY_FUNCTION__, x);
        
        return _children[x];
    }
    
    XMLNode& operator = (XMLNode &node) {
        gnash::log_msg("%s: \n", __PRETTY_FUNCTION__);
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
    XMLNode *getParent() { return _parent; };

    /// Insert a node before a node
    //
    /// Method; inserts a new child node into the XML object's child
    /// list, before the beforeNode node. If the beforeNode parameter is
    /// undefined or null, the node is added using the appendChild()
    /// method. If beforeNode is not a child of my_xml, the insertion
    /// fails.
    void insertBefore(XMLNode *newnode, XMLNode *node);

    /// Removes the specified XML object from its parent.
    //
    /// Also deletes all descendants of the node.
    void removeNode();

    void toString(std::ostream& str) const;

    void  change_stack_frame(int frame, gnash::as_object *xml, gnash::as_environment *env);


    // why don't we use std::strings here ?
    // code would be much simpler and safer!
    char                *_name;
    char                *_value;

    xmlElementType      _type;
    XMLNode		*_parent;
    ChildList		_children;
    AttribList      _attributes;

private:

    // TODO: make a lot more things private !

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
