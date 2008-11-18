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

#include "log.h"

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
    
};

} // end of gnash namespace

#endif


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
