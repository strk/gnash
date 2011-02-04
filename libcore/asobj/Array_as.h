// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#ifndef GNASH_ARRAY_H
#define GNASH_ARRAY_H

#include "as_object.h" 

// Forward declarations
namespace gnash {
	class as_value;
}

namespace gnash {

/// Get the length of an object as though it were an array
//
/// It may well be an array, but this also works on normal objects with a 
/// length property
//
/// @param array    The object whose array length is needed.
/// @return         The array length of the object or 0 if no length is
///                 found.
size_t arrayLength(as_object& array);

/// Convert an integral value into an ObjectURI
//
/// NB this function adds a string value to the VM for each separate
/// integral value. It's the way the VM works.
//
/// @param i        The integral value to find
/// @return         The ObjectURI to look up.
ObjectURI arrayKey(VM& vm, size_t i);

/// A visitor to check whether an array is strict or not.
//
/// Strict arrays have no non-hidden non-numeric properties. Only real arrays
/// are strict arrays; any users of this functor should check that first.
class IsStrictArray : public PropertyVisitor
{
public:
    IsStrictArray(VM& st) : _strict(true), _st(st) {}
    virtual bool accept(const ObjectURI& uri, const as_value& val);

    bool strict() const {
        return _strict;
    }
private:
    bool _strict;
    VM& _st;
};


/// Genuine arrays handle the length property in a special way.
//
/// The only distinction between Arrays and Objects is that the length
/// property is changed when an element is added, and that changing the length
/// can result in deleted properties.
void checkArrayLength(as_object& array, const ObjectURI& uri,
        const as_value& val);

template<typename T>
void foreachArray(as_object& array, T& pred)
{
    size_t size = arrayLength(array);
    if (!size) return;

    VM& vm = getVM(array);

    for (size_t i = 0; i < static_cast<size_t>(size); ++i) {
        pred(getOwnProperty(array, arrayKey(vm, i)));
    }
}

/// Initialize the global.Array object
void array_class_init(as_object& global, const ObjectURI& uri);

void registerArrayNative(as_object& global);

}

#endif
