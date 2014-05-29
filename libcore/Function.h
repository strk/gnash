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

#ifndef GNASH_SWF_FUNCTION_H
#define GNASH_SWF_FUNCTION_H

#include <vector>
#include <cassert>
#include <string>

#include "ConstantPool.h"
#include "UserFunction.h"
#include "ObjectURI.h"

// Forward declarations
namespace gnash {
    class action_buffer;
    class as_object;
    class VM;
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

/// A simple SWF-defined Function
//
/// This represents a callable Function defined in a SWF. The basic version
/// creates a scope in which 'arguments' array, 'this', 'super', and the
/// expected argument names are defined.
//
/// For a more advanced function, see Function2.
class Function : public UserFunction
{

public:

	typedef std::vector<as_object*> ScopeStack;

	/// \brief
	/// Create an ActionScript function as defined in an
	/// action_buffer starting at offset 'start'
	//
	Function(const action_buffer& ab, as_environment& env, size_t start,
		const ScopeStack& with_stack);

	virtual ~Function() {}

	const ScopeStack& getScopeStack() const {
		return _scopeStack;
	}

	const action_buffer& getActionBuffer() const {
		return _action_buffer;
	}

	size_t getStartPC() const {
		return _startPC;
	}

	size_t getLength() const {
		return _length;
	}

    /// Get the number of registers required for function execution.
    //
    /// For ordinary Functions this is always 0.
    virtual std::uint8_t registers() const {
        return 0;
    }

    /// Add an expected argument for the function.
    //
    /// For ordinary Functions the register is disregarded. This is only
    /// relevant for Function2s.
    //
    /// All argument names are declared as variables in the function scope,
    /// whether the argument is passed or not.
    //
    /// @param reg      The register for the argument.
    /// @param name     The name of the argument.
	void add_arg(std::uint8_t reg, const ObjectURI& name) {
            _args.emplace_back(reg, name);
	}

    /// Set the length in bytes of the function code.
	void setLength(size_t len);

	/// Dispatch.
	virtual as_value call(const fn_call& fn);

	/// Mark reachable resources. Override from as_object
	//
	/// Reachable resources from this object are its scope stack
	/// and the prototype.
	///
	virtual void markReachableResources() const;

protected:
	
    struct Argument
	{
        Argument(std::uint8_t r, const ObjectURI& n) : reg(r), name(n) {}
        std::uint8_t reg;
        ObjectURI name;
	};

	std::vector<Argument> _args;

	/// @@ might need some kind of ref count here, but beware cycles
	as_environment& _env;

    /// The ConstantPool in effect at time of function definition
    const ConstantPool* _pool;

private:

	/// Action buffer containing the function definition
	const action_buffer& _action_buffer;

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

};

/// Add properties to an 'arguments' object.
//
/// The 'arguments' variable is an array with an additional
/// 'callee' member, set to the function being called.
as_object* getArguments(Function& callee, as_object& args, 
        const fn_call& fn, as_object* caller);


} // end of gnash namespace

#endif

