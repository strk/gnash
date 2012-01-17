// Machine.h A VM to run AS3 code, and AS2 code in the future.
//
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#ifndef GNASH_MACHINE_H
#define GNASH_MACHINE_H

#include <string>
#include <vector>
#include "SafeStack.h"
#include "as_value.h"
#include "log.h"

namespace gnash {
    namespace abc {
        class AbcBlock;
        class MultiName;
        class Class;
        class abc_function;
        class Method;
        class Namespace;
    }
    class Global_as;
    class DisplayObject;
    class as_object;
    class Property;
    class CodeStream;
    class VM;
    template <typename T> class FunctionArgs;
    class string_table;
}


namespace gnash {

namespace abc {

/// The virtual machine for executing ABC (ActionScript Bytecode).
//
/// This machine is intended to work without relying on the C++ call stack,
/// by resetting its Stream and Stack members (actually, by limiting the stack)
/// to make function calls, rather than calling them directly in C++.
/// As a result, many of the internal calls are void functions even though they
/// will be returning some value.
/// The exception to this is that C++ native functions defined in ActionScript
/// objects will be called in the typical way.
///
/// The C++ exceptions mechanism is used for exception handling, since this
/// allows both ActionScript code and C++ code to use the exception handling
/// with a minimum of hassle, and it helps with correctness.
///
/// It was intended that this Machine should run both AS2 and AS3 code.
/// However, as the two codestreams must be strictly separated - different
/// global objects, no sharing of resources, and no ability to communicate
/// directly, there is no sense in using a single instance of this machine
/// for both AS2 and AS3 interpretation. It's questionable whether there is
/// any advantage in using this Machine for AS2; it is not a near-term goal.
class Machine
{
public:

    /// Create an AS3 interpreter.
	Machine(VM& vm);

    /// Initialize the AS resources
    //
    /// This extra step is necessary because the Machine is initialized
    /// and owned by the AVM1 machine(VM). All access to Machine currently
    /// occurs through VM, but since the VM's Machine pointer is null until
    /// the Machine ctor has completed, we cannot construct AS resources
    /// until after the ctor is complete, because AS resource creation
    /// accesses the Machine through the VM's pointer...
    //
    /// TODO: fix the mess.
    void init();


	// Flash specific members.
	/// The DisplayObject which initiated these actions.
	DisplayObject *getTarget();

	/// Set the DisplayObject which initiated the actions.
	/// Not null.
	void setTarget(DisplayObject* target);

	/// This will complete a name in AS3, where a part of the name
	/// is stored in the stream and another part may be stored on
	/// the stack.
	///
	/// @param name
	/// A partially filled MultiName, this should be the id
	/// from the stream.
	///
	/// @param initial
	/// The depth in the stack where the stack objects may be found.
	///
	/// @return
	/// The number of stack elements used by the name.
	/// At present, always 0, 1, or 2. These are not dropped.
	int completeName(MultiName& name, int initial = 0);

	/// Given a value v, find the class object of the superclass of v.
	///
	/// @param obj
	/// The object whose superclass is desired.
	///
	/// @param find_primitive
	/// If true, the ActionScript prototype will be found for primitive values.
	///
	/// @return
	/// Null if the superclass was not found, or the superclass.
	Class* findSuper(as_value& obj, bool find_primitive);

	/// Get a member from an object.
	///
	/// @param pDefinition
	/// The definition of the class which is to be used. This should be the
	/// one which has the property.
	///
	/// @param name
	/// The bound name of the member
	///
	/// @param source
	/// The source object -- the specific instance of the pDefinition class.
	///
	/// @return
	/// This returns the value, but on the stack.
	/// (Since the return value is not known until after control has left
	/// the caller of this, it's impossible to return a meaningful value.
	void getMember(Class* pDefinition, MultiName& name, as_value& source);

	/// Set a member in an object.
	///
	/// @param pDefinition
	/// The definition of the class which is to be used.
	///
	/// @param name
	/// The bound name of the member
	///
	/// @param source
	/// The source object -- where the instance should be set
	///
	/// @param newvalue
	/// The new value
	///
	/// @return
	/// Nothing.
	void setMember(Class*, MultiName&, as_value& target, as_value& val);

	Property* findProperty(MultiName&) { return NULL; }

	void execute();

	/// push a get call to be executed next.
	///
	/// Any Property can be pushed, and it will put an appropriate value
	/// into return_slot.  This ensures that getter properties
	/// can be accessed in the same way as other properties, and hides
	/// the difference between ActionScript methods and native C++ methods.
	///
	/// @param this_obj
	/// The 'this' to use for a getter/setter if it exists.
	///
	/// @param return_slot
	/// A space for the return value. An assignment will always be made here,
	/// but mVoidSlot can be used for values that will be discarded.
	///
	/// @param prop
	/// The property. If this is a value, it simply returns that value in
	/// the return_slot immediately. Otherwise, it may immediately call
	/// the gettter or it may push that onto the call stack and transfer
	/// control. Callers can be agnostic as to which happens.
	void pushGet(as_object *this_obj, as_value& return_slot, Property *prop);

	/// push a set call to be executed next.
	///
	/// Any Property can be pushed, and it will set the property, if possible.
	/// setter properties and simple properties alike will be handled by this.
	///
	/// @param this_obj
	/// The 'this' to use for a getter/setter if it exists.
	///
	/// @param value
	/// The value which should be set
	///
	/// @param prop
	/// The property desired to be set.
	///
	void pushSet(as_object *this_obj, as_value& value, Property *prop);

	/// push a call to be executed next
	///
	/// Push a call to be executed as soon as execution of the current opcode
	/// finishes. At the end, transfer will return to the previous context.
	///
	/// @param func
	/// The function to call
	///
	/// @param pThis
	/// The object to act as the 'this' pointer.
	///
	/// @param return_slot
	/// The slot to use for returns. Use mIgnoreReturn if you don't care
	/// what happens here.
	///
	/// @param stack_in
	/// How many of the values on the stack are for the new context
	///
	/// @param stack_out
	/// How much of the stack should be left behind when the function exits.
	/// For example: 0 will leave a stack which is stack_in shorter than it
	/// was on call. 1 will leave a stack which is 1 taller than it was on
	/// call.
	///
	/// RESTRICTION: stack_in - stack_out must not be negative
	void pushCall(as_function *func, as_object *pThis, as_value& return_slot,
		unsigned char stack_in, short stack_out);

	void immediateFunction(const as_function *to_call, as_object* pThis,
		as_value& storage, unsigned char stack_in, short stack_out);

	void immediateProcedure(const as_function *to_call, as_object *pthis,
		unsigned char stack_in, short stack_out) {
        immediateFunction(to_call, pthis, mIgnoreReturn, stack_in, stack_out);
    }

	void initMachine(AbcBlock* pool_block);

	as_value executeFunction(Method* function, const fn_call& fn);

	void instantiateClass(std::string className, as_object* global);
    /// Return the Global object for this Machine.
    //
    /// This should be different from the AVM1 global object because the VMs
    /// do not share any ActionScript resources. It should be the same
    /// for a complete run of the Machine so that modifications carried out
    /// by scripts are preserved for subsequent scripts.
    Global_as* global();

    void markReachableResources() const;

private:
	/// The state of the machine.
	class State
	{
	public:
		unsigned int _stackDepth;
		unsigned int _stackTotalSize;
		unsigned int _scopeStackDepth;
		unsigned int mScopeTotalSize;
		bool mReturn;
		CodeStream *mStream;
		Namespace *mDefaultXMLNamespace;
		as_object *mCurrentScope;
		as_value *mGlobalReturn;
		as_object *mThis;
		std::vector<as_value> _registers;
		abc_function* mFunction;
	void to_debug_string(){
		log_abc("StackDepth=%u StackTotalSize=%u ScopeStackDepth=%u ScopeTotalSize=%u",_stackDepth,_stackTotalSize,_scopeStackDepth,mScopeTotalSize);

	}
	};

	class Scope
	{
	public:
		unsigned int mHeightAfterPop;
		as_object *mScope;

		Scope() : mHeightAfterPop(0), mScope(NULL) {/**/}
		Scope(unsigned int i, as_object *o) : mHeightAfterPop(i),
			mScope(o)
		{/**/}
	};

	void saveState();
	void restoreState();

	as_value find_prop_strict(MultiName multiname);

	void print_stack();

	void print_scope_stack();

	void get_args(size_t argc, FunctionArgs<as_value>& args);
	
	void load_function(CodeStream* stream, boost::uint32_t maxRegisters);

	void executeCodeblock(CodeStream* stream);

	void clearRegisters(boost::uint32_t maxRegsiters);

	const as_value& getRegister(int index){
		log_abc("Getting value at a register %d ", index);
		return _registers[index];
	}

    void setRegister(size_t index, const as_value& val) {
        log_abc("Putting %s in register %s", val, index);
        if (_registers.size() <= index) {
            log_abc("Register doesn't exist! Adding new registers!");
            _registers.resize(index + 1);
        }
        _registers[index] = val;
    }

	void push_stack(as_value object){
		log_abc("Pushing value %s onto stack.", object);
		_stack.push(object);
	}

	as_value pop_stack(){
		as_value value = _stack.pop();
		log_abc("Popping value %s off the stack.", value);
		return value;
	}

	void push_scope_stack(as_value object);

	as_object* pop_scope_stack() {
		log_abc("Popping value %s off the scope stack.  There will be "
                "%u items left.", as_value(_scopeStack.top(0)),
                _scopeStack.size()-1);
		return _scopeStack.pop();
	}

	as_object* get_scope_stack(boost::uint8_t depth) const {
		log_abc("Getting value from scope stack %u from the bottom.",
                depth | 0x0);
		return _scopeStack.value(depth);
	}

	SafeStack<as_value> _stack;
	SafeStack<State> mStateStack;
	std::vector<as_value> _registers;

    /// The scope stack is used to look for objects as properties
    //
    /// This stack is not cleared before a function call, class instantiation
    /// etc, but anything on the stack cannot be altered by the function call.
    /// On return from the function, the stack should be the same as it was
    /// before.
    /// Most importantly, the complete stack is used for lookups, including
    /// the section that is not changeable.
	SafeStack<as_object*> _scopeStack;

    CodeStream *mStream;

	string_table& mST;

	Namespace* mDefaultXMLNamespace;
	as_object* mCurrentScope;
	as_object* mGlobalScope;
	as_object* mDefaultThis;
	as_object* mThis;

    /// The global object for this machine.
	Global_as* _global;

	as_value mGlobalReturn;
	as_value mIgnoreReturn; // Throw away returns go here.

	bool mExitWithReturn;
    AbcBlock* mPoolObject; // Where all of the pools are stored.

	abc_function* mCurrentFunction;

	VM& _vm;
};
} // namespace abc
} // namespace gnash
#endif 
