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

#ifndef GNASH_SWF_FUNCTION_H
#define GNASH_SWF_FUNCTION_H

#include <vector>
#include <cassert>
#include <string>

#include "UserFunction.h"
#include "smart_ptr.h"

// Forward declarations
namespace gnash {
    class action_buffer;
    class as_object;
}

namespace gnash {

class TargetGuard
{
public:

	// @param ch : target to set temporarely
	// @param och : original target to set temporarily
	TargetGuard(as_environment& e, DisplayObject* ch, DisplayObject* och);
	~TargetGuard();

private:

    as_environment& env;
    DisplayObject* from;
    DisplayObject* from_orig;

};

/// SWF-defined Function 
class swf_function : public UserFunction
{

public:

	typedef std::vector<as_object*> ScopeStack;

	/// \brief
	/// Create an ActionScript function as defined in an
	/// action_buffer starting at offset 'start'
	//
	swf_function(const action_buffer& ab, as_environment& env, size_t start,
		const ScopeStack& with_stack);

	virtual ~swf_function() {}

	const ScopeStack& getScopeStack() const {
		return _scopeStack;
	}

	const action_buffer& getActionBuffer() const {
		return m_action_buffer;
	}

	size_t getStartPC() const {
		return _startPC;
	}

	size_t getLength() const {
		return _length;
	}

    virtual size_t registers() const {
        return 0;
    }

	void add_arg(boost::uint8_t arg_register, string_table::key name)
	{
        _args.push_back(Argument(arg_register, name));
	}

	void set_length(int len);

	/// Dispatch.
	virtual as_value call(const fn_call& fn);

#ifdef GNASH_USE_GC
	/// Mark reachable resources. Override from as_object
	//
	/// Reachable resources from this object are its scope stack
	/// and the prototype.
	///
	virtual void markReachableResources() const;
#endif // GNASH_USE_GC

protected:

	/// Action buffer containing the function definition
	const action_buffer& m_action_buffer;

	/// @@ might need some kind of ref count here, but beware cycles
	as_environment& _env;

	/// Scope stack on function definition.
	ScopeStack _scopeStack;

	/// \brief
	/// Offset within the action_buffer where
	/// start of the function is found.
	size_t	_startPC;

	/// Length of the function within the action_buffer
	//
	/// This is currently expressed in bytes as the
	/// action_buffer is just a blocḱ of memory corresponding
	/// to a DoAction block
	size_t _length;

	struct Argument
	{
        Argument(boost::uint8_t r, string_table::key n) : reg(r), name(n) {}
        boost::uint8_t reg;
        string_table::key name;
	};

	std::vector<Argument> _args;

};

/// Add properties to an 'arguments' object.
//
/// The 'arguments' variable is an array with an additional
/// 'callee' member, set to the function being called.
as_object* getArguments(swf_function& callee, as_object& args, 
        const fn_call& fn, as_object* caller);


} // end of gnash namespace

#endif

