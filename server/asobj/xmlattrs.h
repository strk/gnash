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

#ifndef __XML_ATTRS_H__
#define __XML_ATTRS_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"

//#define DEBUG_MEMORY_ALLOCATION 1
#include "as_object.h" // for inheritance

#include "log.h"

#include <vector>

#ifdef DEBUG_MEMORY_ALLOCATION
	#include "log.h"
#endif

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>

namespace gnash {
  
/// XML Attribute class
class DSOLOCAL XMLAttr {
public:
    XMLAttr();
    ~XMLAttr();
  
    // why don't we use std::strings here ?
    // code would be much simpler and safer!
    char                *_name;
    char                *_value;
    xmlAttributeType    _type;
    
    XMLAttr *operator = (XMLAttr node) {
        log_msg("\t\tCopying XMLAttr object at %p\n", (void*)this);
    
        _name = new char[strlen(node._name)+2];
        memset(_name, 0, strlen(node._name)+2);
        strcpy(_name, node._name);

        _value = new char[strlen(node._value)+2];
        memset(_value, 0, strlen(node._value)+2);
        strcpy(_value, node._value);

        return this;
    }
};

/// XML Attribute ActionScript Object
class DSOLOCAL xmlattr_as_object : public as_object
{
public:
    //XMLAttr obj;
    int   padding;
#ifdef DEBUG_MEMORY_ALLOCATION
    xmlattr_as_object() {
        log_msg("\t\tCreating xmlattr_as_object at %p\n", this);
    };
    ~xmlattr_as_object() {
        log_msg("\tDeleting xmlattr_as_object at %p \n", this);
    };
#endif
};
 
} // end of gnash namespace

#endif	// __XML_ATTRS_H__


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
