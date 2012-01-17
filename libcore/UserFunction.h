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
#ifndef GNASH_USER_FUNCTION_H
#define GNASH_USER_FUNCTION_H

#include "as_function.h"

namespace gnash {
    class Global_as;
}

namespace gnash {

/// A UserFunction is a callable function defined in ActionScript
//
/// Gnash has two types of UserFunction:
//
/// 1. Function: functions parsed from a SWF
/// 2. builtin_function: functions implemented in C++ as though they were
///    These are used to implement the API functions that the proprietary
///    player implements in a startup script.
class UserFunction : public as_function
{
public:

    /// Return the number of local registers needed
    //
    /// Only Function2 functions require local registers; for all others
    /// the value should be 0.
    virtual boost::uint8_t registers() const = 0;

protected:

    UserFunction(Global_as& gl) : as_function(gl) {}

    /// This is an abstract base class!
    virtual ~UserFunction() = 0;

};

inline UserFunction::~UserFunction() {}

}

#endif
