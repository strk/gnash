// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_FN_CALL_H
#define GNASH_FN_CALL_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "as_environment.h" // for inlines (arg)
#include "as_object.h" // for dtor visibility by boost::intrusive_ptr
#include "smart_ptr.h"

#include <cassert> // for inlines (arg)
#include <ostream> // for inlines (dump_args)
#include <sstream> // for inlines (dump_args)

// Forward declarations
namespace gnash {
    class as_environment;
    class as_function;
    class as_object;
    class as_value;
    class movie_definition;
}

namespace gnash {


/// \brief
/// Parameters/environment for builtin or user-defined functions
/// callable from ActionScript.
class fn_call
{
public:
	/// The as_object (or a pointer derived thereof) on which this call
	/// is taking place.
	boost::intrusive_ptr<as_object> this_ptr;

	/// The "super" object in this function call context
	as_object* super;

	/// Number of arguments to this ActionScript function call.
	unsigned int nargs;

    /// Definition containing caller code. 0 if spontaneous (system event).
    const movie_definition* callerDef;

    fn_call(const fn_call& fn)
        :
        this_ptr(fn.this_ptr),
        super(fn.super),
		nargs(fn.nargs),
        callerDef(fn.callerDef),
        _env(fn._env)
	{
		if (fn._args.get()) {
			_args.reset(new std::vector<as_value>(*fn._args));
        }
	}

	fn_call(const fn_call& fn, as_object* this_in, as_object* sup = 0)
		:
        this_ptr(this_in),
        super(sup),
        nargs(fn.nargs),
        callerDef(fn.callerDef),
		_env(fn._env)
	{
		if (fn._args.get()) {
			_args.reset(new std::vector<as_value>(*fn._args));
        }
	}

	fn_call(as_object* this_in, as_environment& env_in,
			int nargs_in, size_t first_in, as_object* sup = 0)
		:
		this_ptr(this_in),
		super(sup),
		nargs(nargs_in),
        callerDef(0),
		_env(env_in)
	{
		assert(first_in + 1 == env_in.stack_size());
		readArgs(env_in, first_in, nargs);
	}

	fn_call(as_object* this_in, as_environment& env_in,
			std::auto_ptr<std::vector<as_value> > args, as_object* sup = 0)
		:
		this_ptr(this_in),
		super(sup),
		nargs(args->size()),
        callerDef(0),
		_env(env_in),
		_args(args)
	{
	}

	fn_call(as_object* this_in, as_environment& env_in)
		:
		this_ptr(this_in),
		super(0),
		nargs(0),
        callerDef(0),
		_env(env_in),
		_args(0)
	{
	}

    /// Return the VM this fn_call is running from
    VM& getVM() const
    {
        return _env.getVM();
    }

	/// Return true if this call is an object instantiation
	bool isInstantiation() const
	{
		// Currently the as_function::constructInstance
		// will set 'this_ptr' to NULL when calling a builtin
		// function, so we use this info to find out.
		// For the future, we might use an explicit flag instead
		// as I belive there are some cases in which 'this' is
		// undefined even in a normal function call.
		return (this_ptr == 0);
	}

	/// Access a particular argument.
	const as_value& arg(unsigned int n) const
	{
		assert(n < nargs);
		return (*_args)[n]; 
	}

    const std::vector<as_value>& getArgs() const {
        return *_args;
    }

	void drop_bottom()
	{
		assert(_args.get() && !(*_args).empty());
		for (size_t i=0; i<(*_args).size()-1; ++i)
		{
			(*_args)[i] = (*_args)[i+1];
		}
		_args->pop_back();
		--nargs;
	}

	as_environment& env() const
	{
		return _env;
	}

	/// Dump arguments to given output stream
	void dump_args(std::ostream& os) const
	{
		for (unsigned int i=0; i<nargs; ++i)
		{
			if ( i ) os << ", ";
			os << arg(i).toDebugString();
		}
	}

	/// Return arguments as a string (for debugging)
	std::string dump_args() const
	{
		std::stringstream ss;
		dump_args(ss);
		return ss.str();
	}

	void resetArgs()
	{
		nargs=0;
		_args->clear();
	}

	void pushArg(const as_value& arg)
	{
		nargs++;
		_args->push_back(arg);
	}

private:

	/// The ActionScript environment in which the function call is taking
	/// place. This contains, among other things, the function arguments.
	as_environment& _env;

	/// The actual arguments
	std::auto_ptr< std::vector<as_value> > _args;

	void readArgs(as_environment& env, int first_in, int nargs)
	{
		_args.reset(new std::vector<as_value>);
		for (int i=0; i<nargs; ++i)
			_args->push_back(env.bottom(first_in - i));
	}

};

/// Signature of a builtin function callable from ActionScript
typedef as_value (*as_c_function_ptr)(const fn_call& fn);


} // namespace gnash


#endif // _GNASH_FN_CALL_H_


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
