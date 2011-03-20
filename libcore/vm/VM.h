// VM.h: the Virtual Machine class, for Gnash
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

#ifndef GNASH_VM_H
#define GNASH_VM_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <map>
#include <vector>
#include <memory> 
#include <locale>
#include <boost/cstdint.hpp> 
#include <boost/random.hpp>
#include <boost/noncopyable.hpp>
#include <boost/array.hpp>

#include "GC.h"
#include "string_table.h"
#include "SafeStack.h"
#include "CallStack.h"
#include "as_value.h"
#include "namedStrings.h"
#include "ObjectURI.h"

// Forward declarations
namespace gnash {
	class Global_as;
	class VM;
	class fn_call;
	class movie_root;
	class NativeFunction;
    class SharedObjectLibrary;
	class as_value;
	class as_object;
	class VirtualClock;
    class UserFunction;
}

namespace gnash {

/// The AVM1 virtual machine
//
/// The VM class has no code for execution, but rather stores the resources
/// needed for execution:
//
/// 1. The stack
/// 2. Global registers
/// 3. The call stack.
//
/// Actual execution is done by ActionExec.
//
/// This header also contains a few utility functions for ActionScript
/// operations.
class DSOEXPORT VM : boost::noncopyable
{
public:

	typedef as_value (*as_c_function_ptr)(const fn_call& fn);
	
	/// Initializes the VM
    //
    /// @param version      The initial version of the VM
    /// @param root         The movie_root that owns this VM
    /// @param clock        The clock to use for advances.
	VM(int version, movie_root& root, VirtualClock& clock);

	~VM();

    /// Accessor for the VM's stack
    //
    /// TODO: drop
	SafeStack<as_value>& getStack() {
		return _stack;
	}

    /// Get the VM clock
    //
    /// NOTE: this clock should drive all internal operations
    /// but maybe accessing it trough VM isn't the best idea.
    /// TODO: consider making this accessible trough RunResources
    /// instead.
    VirtualClock& getClock() {
        return _clock;
    }

	/// Get SWF version context for the currently running actions.
	//
	/// This information will drive operations of the virtual machine
	///
	int getSWFVersion() const {
        return _swfversion;
    }

	/// Set SWF version of the currently executing code
	void setSWFVersion(int v);

	/// Get the number of milliseconds since VM was started
	unsigned long int getTime() const;

	/// Get a reference to the string table used by the VM.
	string_table& getStringTable() const { return _stringTable; }

	/// Get version of the player, in a compatible representation
	//
	/// This information will be used for the System.capabilities.version
	/// and $version ActionScript variables.
	///
	const std::string& getPlayerVersion() const;
	
	/// Get current OS name. This is used for System.capabilites.os. If
	/// defined in gnashrc, that takes precedence. For Linux, the string
	/// includes the kernel version (unname -sr). Only works for systems
	/// with sys/utsname.h (POSIX 4.4).
	std::string getOSName() const;
	
	/// Return the current language of the system. This is used for
	/// System.capabilities.language. Only works for systems with 
	/// a language environment variable.
	std::string getSystemLanguage() const;
	
	// The boost Random Number Generator to use.
	//
	// http://www.boost.org/libs/random/random-generators.html
	//
	// TODO: boost/nondet_random.hpp provides access to a random device,
	// which can be used in preference to a pseudo-RNG. It is only
	// presently available on some platforms.
	// http://www.boost.org/libs/random/nondet_random.html
	//
	// Generators have different limits on the size of the seed. Please
	// check if replacing the generator.
	//
	// The mt11213b provides a pseudo-random number cycle
	// of length 2^11213-1 and requires approx 352*sizeof(uint32_t) memory
	// once initialized. It is more than adequate for most purposes.
	typedef boost::mt11213b RNG;	

	// Get a pointer to the random number generator for
	// use by Math.random() and random().
	RNG& randomNumberGenerator();

	/// Get a pointer to this VM's Root movie (stage)
	movie_root& getRoot() const;

    /// Return the Shared Object Library
    SharedObjectLibrary& getSharedObjectLibrary() const {
        assert(_shLib.get());
        return *_shLib;
    }

	/// Get a pointer to this VM's _global Object
	Global_as* getGlobal() const;

	/// Mark all reachable resources (for GC)
	//
	/// - root movie / stage (_rootMovie)
	/// - Global object (_global)
	/// - Class Hierarchy object
	void markReachableResources() const;

	void registerNative(as_c_function_ptr fun, unsigned int x, unsigned int y);

	/// Return a native function or null
	NativeFunction* getNative(unsigned int x, unsigned int y) const;

    /// Get value of a register (local or global).
    //
    /// When not in a function context the selected register will be
    /// global or not at all (if index is not in the valid range
    /// of global registers).
    ///
    /// When in a function context defining no registers, 
    /// we'll behave the same as for a non-function context.
    ///
    /// When in a function context defining non-zero number
    /// of local registers, the register set will be either local
    /// or not at all (if index is not in the valid range of local
    /// registers).
    //
    /// @param index    The index of the register to retrieve.
    /// @return         A pointer to the as_value at the specified position, or
    ///                 0 if the index is invalid
    const as_value* getRegister(size_t index);

    /// Set value of a register (local or global).
    //
    /// When not in a function context the set register will be
    /// global or not at all (if index is not in the valid range
    /// of global registers).
    ///
    /// When in a function context defining no registers, 
    /// we'll behave the same as for a non-function context.
    ///
    /// When in a function context defining non-zero number
    /// of local registers, the register set will be either local
    /// or not at all (if index is not in the valid range of local
    /// registers).
    ///
    /// @param index    The index of the register to set. If the index
    ///                 is invalid, this is a no-op.
    /// @param val      The value to set the register to.
    void setRegister(size_t index, const as_value& val);

    /// Add a function call to the call frame.
    //
    /// This should be called for all user-defined functions before the
    /// function is executed
    //
    /// @return     The pushed CallFrame. This is identical to currentCall().
    CallFrame& pushCallFrame(UserFunction& f);

    /// Remove a function call from the call frame.
    //
    /// This should be called on return from the function.
    void popCallFrame();

    /// Return the CallFrame of the currently executing function.
    //
    /// Callers must ensure that there is a current function before calling
    /// this!
    CallFrame& currentCall();

    /// Whether a function call is in progress.
    bool calling() const {
        return !_callStack.empty();
    }

    /// Print stack, call stack, and registers to the specified ostream
    void dumpState(std::ostream& o, size_t limit = 0);

private:

	/// Stage associated with this VM
	movie_root& _rootMovie;

	/// The _global ActionScript object for AVM1
	Global_as* _global;

	/// Target SWF version
	int _swfversion;

	typedef std::map<unsigned int, as_c_function_ptr> FuncMap;
	typedef std::map<unsigned int, FuncMap> AsNativeTable;
	AsNativeTable _asNativeTable;

	/// Mutable since it should not affect how the VM runs.
	mutable string_table _stringTable;

	VirtualClock& _clock;

	SafeStack<as_value>	_stack;

    typedef boost::array<as_value, 4> GlobalRegisters;
    GlobalRegisters _globalRegisters;

	CallStack _callStack;

	/// Library of SharedObjects. Owned by the VM.
    std::auto_ptr<SharedObjectLibrary> _shLib;

    RNG _rng;

};

// @param lowerCaseHint if true the caller guarantees
//        that the lowercase equivalent of `str' is `str' again
//
inline ObjectURI
getURI(const VM& vm, const std::string& str, bool lowerCaseHint=false)
{
    lowerCaseHint=lowerCaseHint; // TODO pass hint to ObjectURI ctor
    // Possible optimization here is to directly compute
    // noCase value if VM version is < 7
    return ObjectURI((NSV::NamedStrings)vm.getStringTable().find(str));
}

inline ObjectURI
getURI(const VM&, NSV::NamedStrings s)
{
    // Possible optimization here is to directly
    // compute noCase values if VM version is < 7
    // (using the known information in NSV)
    return ObjectURI(s);
}

inline const std::string&
toString(VM& vm, const ObjectURI& uri)
{
	return uri.toString(vm.getStringTable());
}


/// A class to wrap frame access.  Stack allocating a frame guard
/// will ensure that all CallFrame pushes have a corresponding
/// CallFrame pop, even in the presence of extraordinary returns.
class FrameGuard
{
public:

    FrameGuard(VM& vm, UserFunction& func)
        :
        _vm(vm),
        _callFrame(_vm.pushCallFrame(func))
    {
    }

    /// Get the CallFrame we've just pushed.
    CallFrame& callFrame() {
        return _callFrame;
    }

    ~FrameGuard() {
        _vm.popCallFrame();
    }

private:
    VM& _vm;
    CallFrame& _callFrame;
};

/////////////////////////////////////////////////////////////////////////////
///
/// VM ops on as_value.
///
/// These are currently used in:
/// 1. VM (AVM1)
/// 2. Gnash's C++ implementation of ActionScript.
///
/// Usage 2 replicates VM behaviour, but does not use the VM's stack or other
/// resources. This can lead to different behaviour, for instance when there
/// are limits on the stack size. Such cases are probably rare.
///
/////////////////////////////////////////////////////////////////////////////

/// Carry out ActionNewAdd
//
/// @param op1      The as_value to add to.
/// @param op2      The as_value to add.
/// @param vm       The VM executing the operation.
//
/// TODO:           Consider whether it would be better to pass something
///                 other than the VM. But it is a VM operation, so it
///                 is logically sound.
void newAdd(as_value& op1, const as_value& op2, const VM& vm);

/// Carry out ActionSubtract
//
/// @param op1      The as_value to subtract from.
/// @param op2      The as_value to subtract.
/// @param vm       The VM executing the operation.
void subtract(as_value& op1, const as_value& op2, const VM& vm);

/// Carry out ActionSubtract
//
/// @param op1      The first comparand.
/// @param op2      The second comparand.
/// @param vm       The VM executing the operation.
as_value newLessThan(const as_value& op1, const as_value& op2, const VM& vm);

/// Check if two values are equal
//
/// Note that conversions are performed as necessary, which can result in
/// function calls, which can have any conceivable side effect. The order of
/// the values affects the order the conversions are performed in, so can
/// under some circumstances change the result of the comparison.
//
/// Equality comparisons depend strongly on the SWF version.
//
/// @param a    The first value to compare
/// @param b    The second value to compare
/// @param vm   The VM to use for the comparison.
/// @return     Whether the values are considered equal.
bool equals(const as_value& a, const as_value& b, const VM& vm);

/// Convert an as_value to boolean type
//
/// @param val  The value to return as a boolean
/// @param vm   The VM to use for the conversion.
/// @return     The boolean value of the passed as_value.
bool toBool(const as_value& v, const VM& vm);

/// Convert an as_value to a double
//
/// @param val  The value to return as a double
/// @param vm   The VM to use for the conversion.
/// @return     The double value of the passed as_value.
double toNumber(const as_value& v, const VM& vm);

/// Convert an as_value to an object
//
/// @param val  The value to return as an object
/// @param vm   The VM to use for the conversion.
/// @return     The Object representation value of the passed as_value.
as_object* toObject(const as_value& v, VM& vm);

/// AS2-compatible conversion to 32bit integer
//
/// This truncates large numbers to fit in the 32-bit space. It is not a 
/// proper function of as_value because it is simply a further operation on
/// the stored number type.
//
/// This function calls to_number(), so performs a conversion if necessary.
//
/// @param val  The value to return as an int.
/// @param vm   The VM to use for the conversion.
/// @return     The integer value of the passed as_value.
boost::int32_t toInt(const as_value& val, const VM& vm);

/// Force type to number.
//
/// @param v    The value to change to a number type.
/// @param vm   The VM to use for the conversion.
/// @return     The value passed as v.
as_value& convertToNumber(as_value& v, const VM& vm);

/// Force type to string.
//
/// @param v    The value to change to a string type.
/// @param vm   The VM to use for the conversion.
/// @return     The value passed as v.
as_value& convertToString(as_value& v, const VM& vm);

/// Force type to bool.
//
/// @param v    The value to change to a bool type.
/// @param vm   The VM to use for the conversion.
/// @return     The value passed as v.
as_value& convertToBoolean(as_value& v, const VM& vm);

/// Convert to the appropriate primitive type
//
/// @param v    The value to change to a primitive type.
/// @param vm   The VM to use for the conversion.
/// @return     The value passed as v.
as_value& convertToPrimitive(as_value& v, const VM& vm);

} // namespace gnash

#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
