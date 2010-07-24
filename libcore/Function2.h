// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#ifndef GNASH_FUNCTION2_H
#define GNASH_FUNCTION2_H

#include <vector>
#include <cassert>
#include <string>

#include "swf_function.h"
#include "smart_ptr.h"

// Forward declarations
namespace gnash {
    class action_buffer;
    class as_object;
}

namespace gnash {

/// Function2 adds extra sauce to ordinary Functions.
//
/// The Function2 was introduced in version 6 of the player. Differences from
/// ordinary functions are:
/// 1. Up to 256 local registers.
/// 2. Ability to suppress super, this, arguments
/// 3. Ability to store super, this, arguments, _global, _root, and _parent
///    in registers.
class Function2 : public swf_function
{

public:

	enum SWFDefineFunction2Flags
	{
		/// Bind one register to "this" 
		PRELOAD_THIS = 0x01, // 1

		/// No "this" variable accessible by-name 
		SUPPRESS_THIS = 0x02, // 2

		/// Bind one register to "arguments" 
		PRELOAD_ARGUMENTS = 0x04, // 4

		/// No "argument" variable accessible by-name 
		SUPPRESS_ARGUMENTS = 0x08, // 8

		/// Bind one register to "super" 
		PRELOAD_SUPER = 0x10, // 16

		/// No "super" variable accessible by-name 
		SUPPRESS_SUPER = 0x20, // 32

		/// Bind one register to "_root" 
		PRELOAD_ROOT = 0x40, // 64

		/// Bind one register to "_parent" 
		PRELOAD_PARENT = 0x80, // 128

		/// Bind one register to "_global" 
		//
		/// TODO: check this.
        /// See:
        /// http://www.m2osw.com/swf_alexref.html#action_declare_function2
		///       Looks like flags would look swapped
		PRELOAD_GLOBAL = 256 // 0x100

	};

	/// \brief
	/// Create an ActionScript function as defined in an
	/// action_buffer starting at offset 'start'
	//
	Function2(const action_buffer& ab, as_environment& env, size_t start,
		const ScopeStack& with_stack);

	virtual ~Function2() {}

    virtual size_t registers() const {
        return _registerCount;
    }

	void set_local_register_count(boost::uint8_t ct) {
        _registerCount = ct;
    }

	void set_function2_flags(boost::uint16_t flags) {
        _function2Flags = flags;
    }

	/// Dispatch.
	virtual as_value call(const fn_call& fn);

private:

	boost::uint8_t _registerCount;

	/// used by function2 to control implicit arg register assignments
	// 
	/// See
    /// http://sswf.sourceforge.net/SWFalexref.html#action_declare_function2
	boost::uint16_t	_function2Flags;

};


} // end of gnash namespace

#endif

