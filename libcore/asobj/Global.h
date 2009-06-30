// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

/// This file provides implementations of two different ActionScript global
/// objects: one for AVM1, one for AVM2.
//
/// The AVM1 global object has more (known) global functions. All AS2 classes
/// are initialized as object prototypes and functions attached to the 
/// global object. From SWF8, the 'flash' package is attached as follows:
///
///                             _global
///                                 |
///                               flash
///                                 |
///            ---------------------------------------------------
///            |         |          |          |         |       |
///         display     net     external    filters     geom    text
///
/// where each item is an object.
///
/// The AVM2 global object has functions such as trace(), escape(),
/// parseFloat(), parseInt() in common with AVM1. Some classes, such as
/// Array, Boolean, Date, String, and Object, are also directly attached.
/// Other classes, however, are different. The flash package in AVM2 is a 
/// namespace, not an object, and all members of the flash package are
/// attached with a namespace to the global object. As we do this on
/// demand, the AVM2 global object is much emptier than the AVM1 equivalent
/// to start with.
#ifndef GNASH_GLOBAL_H
#define GNASH_GLOBAL_H

#include "as_object.h" // for inheritance
#include "extension.h" // for composition

// Forward declarations
namespace gnash {
	class Machine;
	class VM;
	class fn_call;
	class ClassHierarchy;
}

namespace gnash {


class AVM1Global: public as_object
{
public:

	AVM1Global(VM& vm, ClassHierarchy *ch);
	~AVM1Global() {}

private:

    void loadExtensions();
	Extension _et;

};

class AVM2Global: public as_object
{
public:

	AVM2Global(Machine& m, ClassHierarchy *ch);
	~AVM2Global() {}

};


} // namespace gnash

#endif // ndef GNASH_GLOBAL_H
