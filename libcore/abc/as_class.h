// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#ifndef GNASH_ABC_AS_CLASS_H
#define GNASH_ABC_AS_CLASS_H

#include <string>
#include "as_object.h"
namespace gnash {
    namespace abc {
        class Class;
    }
}

namespace gnash {
namespace abc {

/// The implementation of a 'Class' type in ActionScript 3.
//
/// A Class is a first-class type, i.e. it can be referenced itself in
/// ActionScript.
//
/// Although Classes are nominally 'dynamic' types, there seems to be no
/// way to alter them in ActionScript, or to create them dynamically. In
/// order to reference them, the Class must already be constructed and
/// known in the execution scope, then retrieved by name.
//
/// Accordingly, all as_class objects have an associated Class, which is its
/// static definition.
//
/// TODO: see how to implement "[class Class]", the prototype of all classes.
class as_class : public as_object
{
public:

    as_class(Global_as& gl, Class* c);
    virtual ~as_class() {}

    virtual const std::string& stringValue() const;

private:

    Class* _class;

    const std::string _name;
};

} // namespace abc
} // namespace gnash

#endif
