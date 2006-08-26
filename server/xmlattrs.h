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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

#ifndef __XML_ATTRS_H__
#define __XML_ATTRS_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#define DEBUG_MEMORY_ALLOCATION 1
#include <vector>
#include "log.h"
#include "action.h"
#include "impl.h"

#ifdef HAVE_LIBXML

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>

namespace gnash {
  
/// XML Attribute class
class XMLAttr {
public:
    XMLAttr();
    ~XMLAttr();
  
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
class xmlattr_as_object : public as_object
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

#endif // HAVE_LIBXML

#endif	// __XML_ATTRS_H__


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
