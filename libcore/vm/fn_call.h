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

#ifndef GNASH_FN_CALL_H
#define GNASH_FN_CALL_H

#include <string>
#include <vector>
#include <cassert> 
#include <ostream>
#include <algorithm>

#include "as_object.h"
#include "as_value.h"
#include "VM.h"
#include "GnashException.h"
#include "as_environment.h"


// Forward declarations
namespace gnash {
    class movie_definition;
}

namespace gnash {

/// A class to contain transferable arguments for a fn_call.
//
/// The operators += and , are implemented for intuitive syntax:
//
/// FunctionArgs<as_value> args; args += 0.0, "string", NaN.
//
/// This may have unexpected side effects if it is used in unexpected ways,
/// so stick to using such lists, or use operator += repeatedly.
//
/// The arguments can be moved to another container, and this happens when
/// the FunctionArgs object is passed to fn_call. It will still be valid
/// afterwards, but will contain no arguments.
template<typename T>
class FunctionArgs
{
public:

    typedef typename std::vector<T>::size_type size_type;
    typedef std::vector<T> container_type;
    typedef T value_type;

    FunctionArgs() {}

    /// The copy constructor copies all the arguments.
    FunctionArgs(const FunctionArgs& other)
        :
        _v(other._v)
    {}

    FunctionArgs& operator+=(const T& t) {
        _v.push_back(t);
        return *this;
    }

    FunctionArgs& operator,(const T& t) {
        _v.push_back(t);
        return *this;
    }

    /// Mark any reachable resources
    //
    /// This is only for cases where the lifetime of a FunctionArgs object
    /// extends beyond a function call.
    void setReachable() const {
        std::for_each(_v.begin(), _v.end(),
                      std::mem_fun_ref(&as_value::setReachable));
    }

    void swap(std::vector<T>& to) {
        std::swap(_v, to);
    }

    size_type size() const {
        return _v.size();
    }

private:
    std::vector<T> _v;
};


/// \brief
/// Parameters/environment for builtin or user-defined functions
/// callable from ActionScript.
class fn_call
{
public:

    typedef FunctionArgs<as_value> Args;

    /// Construct a fn_call
    //
    /// @param isNew        Pass true if this is a constructing fn_call,
    ///                     i.e. if it is called as a result of 'new'.
    /// @param super        Pass an overridden super value to the function
    ///                     call. If this is 0, the super reference will be
    ///                     calculated from the this pointer (if that is not
    ///                     null) whenever a function requires it.
    fn_call(as_object* this_in, const as_environment& env_in,
            Args& args, as_object* sup = 0, bool isNew = false)
        :
        this_ptr(this_in),
        super(sup),
        nargs(args.size()),
        callerDef(0),
        _env(env_in),
        _new(isNew)
    {
        args.swap(_args);
    }
    
    fn_call(as_object* this_in, const as_environment& env_in)
        :
        this_ptr(this_in),
        super(0),
        nargs(0),
        callerDef(0),
        _env(env_in),
        _new(false)
	{
	}
    
    /// Copy constructor
    fn_call(const fn_call& fn)
        :
        this_ptr(fn.this_ptr),
        super(fn.super),
        nargs(fn.nargs),
        callerDef(fn.callerDef),
        _env(fn._env),
        _args(fn._args),
        _new(false)
	{
	}
    
    /// The as_object (or a pointer derived thereof) on which this call
    /// is taking place.
    as_object* this_ptr;
    
    /// The "super" object in this function call context
    //
    /// If this is 0, the super may be constructed from the this pointer.
    as_object* super;
    
    /// Number of arguments to this ActionScript function call.
    Args::size_type nargs;
    
    /// Definition containing caller code. 0 if spontaneous (system event).
    const movie_definition* callerDef;
    
    /// Return the VM this fn_call is running from
    VM& getVM() const {
        return _env.getVM();
    }
    
    /// Return true if this call is an object instantiation
    bool isInstantiation() const {
        return _new;
	}
    
    /// Access a particular argument.
    const Args::value_type& arg(unsigned int n) const {
        assert(n < nargs);
        return _args[n]; 
	}
    
    const Args::container_type& getArgs() const {
        return _args;
    }
    
    void drop_bottom() {
        assert(!_args.empty());
        _args.erase(_args.begin());
        --nargs;
	}
    
    const as_environment& env() const {
        return _env;
	}
    
    /// Dump arguments to given output stream
    void dump_args(std::ostream& os) const {
        for (size_t i = 0; i < nargs; ++i) {
            if (i) os << ", ";
            os << arg(i);
        }
	}
    
    void resetArgs() {
        nargs = 0;
        _args.clear();
	}
    
    void pushArg(const Args::value_type& arg) {
        ++nargs;
        _args.push_back(arg);
	}
    
private:

    /// The ActionScript environment in which the function call is taking
    /// place. This contains, among other things, the function arguments.
    const as_environment& _env;
    
    /// The actual arguments
    Args::container_type _args;
    
    bool _new;
    
};


/// Check that the 'this' pointer has a particular native type ('Relay').
//
/// This is the most likely of the cases to reflect AS behaviour.
template<typename T>
struct ThisIsNative
{
    typedef T value_type;
    value_type* operator()(const as_object* o) const {
        return dynamic_cast<value_type*>(o->relay());
    }
};

/// Check that the 'this' pointer is a DisplayObject
//
/// By default this just checks for any DisplayObject type.
template<typename T = DisplayObject>
struct IsDisplayObject
{
    typedef T value_type;
    value_type* operator()(const as_object* o) const {
        if (!o) return 0;
        return dynamic_cast<T*>(o->displayObject());
    }
};

/// Check that the 'this' pointer is not null.
struct ValidThis
{
    typedef as_object value_type;
    value_type* operator()(as_object* o) const {
        return o;
    }
};

/// Templated function to check the validity of a function call.
//
/// It throws an exception if the condition is not fulfilled, it throws
/// an ActionTypeError, resulting in the function call being aborted and
/// an undefined as_value returned.
//
/// Note that not carrying out a function because the this pointer is
/// undefined is not ActionScript behaviour in most cases. To avoid
/// spreading its usage outside AS function implementations, this function
/// now takes a fn_call as an argument.
//
/// @tparam T       A struct defining a value_type and an operator() that
///                 checks the as_object's validity. A pointer to the
///                 value_type is returned on success, an exception thrown
///                 on failure.
/// @param fn       The function whose 'this' pointer should be checked.
/// @return         If the cast succeeds, the pointer cast to the
///                 requested type.
template<typename T>
typename T::value_type*
ensure(const fn_call& fn)
{
    as_object* obj = fn.this_ptr;
    if (!obj) throw ActionTypeError();
    
    typename T::value_type* ret = T()(obj);
    
    if (!ret) {
        std::string target = typeName(ret);
        std::string source = typeName(obj);

        std::string msg = "Function requiring " + target + " as 'this' "
            "called from " + source + " instance.";

        throw ActionTypeError(msg);
    }
    return ret;
}

inline string_table&
getStringTable(const fn_call& fn)
{
    return fn.getVM().getStringTable();
}

inline movie_root&
getRoot(const fn_call& fn)
{
    return fn.getVM().getRoot();
}

inline int
getSWFVersion(const fn_call& fn)
{
    return fn.getVM().getSWFVersion();
}

inline VM&
getVM(const fn_call& fn)
{
    return fn.getVM();
}

inline Global_as&
getGlobal(const fn_call& fn)
{
    return *fn.getVM().getGlobal();
}

} // namespace gnash


#endif 


// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
