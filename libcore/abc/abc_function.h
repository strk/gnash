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

#ifndef GNASH_ABC_FUNCTION_H
#define GNASH_ABC_FUNCTION_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h" 
#endif

#include "as_function.h"
#include "Method.h"

namespace gnash {
    class as_value;
    class CodeStream;
}

namespace gnash {
namespace abc {

class Machine;

/// ABC-defined Function 
class abc_function : public as_function
{

public:
	abc_function(Method* methodInfo, Machine* machine);

	as_value call(const fn_call& fn);

	CodeStream* getCodeStream() const {
        return _methodInfo->getBody();
    }

	boost::uint32_t getMaxRegisters() const {
        return _methodInfo->getMaxRegisters();
    }

    bool needsActivation() const {
        return _methodInfo->needsActivation();
    }

private:

    Method* _methodInfo;

    Machine* _machine;

};

} // namespace abc
} // gnash namespace

#endif
