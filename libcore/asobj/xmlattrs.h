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

#ifndef GNASH_XML_ATTRS_H
#define GNASH_XML_ATTRS_H

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
class XMLAttr
{
public:

    XMLAttr()
    {}

    XMLAttr(const std::string& name, const std::string& value)
            :
            _name(name),
            _value(value)
    {}

    ~XMLAttr()
    {}
  
    const std::string& name() const { return _name; }

    const std::string& value() const { return _value; }

private:

    std::string _name;
    std::string _value;
    xmlAttributeType _type;
    
};

/// XML Attribute ActionScript Object
class xmlattr_as_object : public as_object
{
public:
    //XMLAttr obj;
    int   padding;
#ifdef DEBUG_MEMORY_ALLOCATION
    xmlattr_as_object() {
        log_debug("\t\tCreating xmlattr_as_object at %p\n", this);
    };
    ~xmlattr_as_object() {
        log_debug("\tDeleting xmlattr_as_object at %p \n", this);
    };
#endif
};
 
} // end of gnash namespace

#endif	// __XML_ATTRS_H__


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
