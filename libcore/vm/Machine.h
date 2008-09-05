// Machine.h A VM to run AS3 code, and AS2 code in the future.
//
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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

#include <vector>
#include <sstream>
#include "SafeStack.h"
#include "as_value.h"
#include "asClass.h"
#include "swf.h"
#include "as_environment.h"
#include "VM.h"

#define LOG_DEBUG_AVM(fmt,...) IF_VERBOSE_ACTION(log_action("AVM2: " fmt, ## __VA_ARGS__));

namespace gnash {

class character;
class as_object;
class abc_block;
class asName;
class Property;

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
/// The intent is that the machine will run both AS2 and AS3 code. Despite the
/// difference in presentation between the two, they should be compatible (or
/// able to become so), so that extensions written for AS2 will work in AS3
/// (and vice versa).
class Machine
{
public:
	// Flash specific members.
	/// The character which initiated these actions.
	character *getTarget();

	/// Set the character which initiated the actions.
	/// Not null.
	void setTarget(character* target);

	/// This will complete a name in AS3, where a part of the name
	/// is stored in the stream and another part may be stored on
	/// the stack.
	///
	/// @param name
	/// A partially filled asBoundName, this should be the id
	/// from the stream.
	///
	/// @param initial
	/// The depth in the stack where the stack objects may be found.
	///
	/// @return
	/// The number of stack elements used by the name.
	/// At present, always 0, 1, or 2. These are not dropped.
	int completeName(asName& name, int initial = 0);

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
	asClass* findSuper(as_value& obj, bool find_primitive);

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
	void getMember(asClass* pDefinition, asName& name, as_value& source);

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
	void setMember(asClass*, asName&, as_value& target, as_value& val);

	asBinding* findProperty(asName&) { return NULL; }

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
		unsigned char stack_in, short stack_out)
	{ immediateFunction(to_call, pthis, mIgnoreReturn, stack_in, stack_out); }

	void initMachine(abc_block* pool_block,as_object* global);

	as_value executeFunction(CodeStream* stream,const fn_call& fn);

	void instantiateClass(std::string className);

	Machine(VM& vm);

private:
	/// The state of the machine.
	class State
	{
	public:
		unsigned int mStackDepth;
		unsigned int mStackTotalSize;
		unsigned int mScopeStackDepth;
		unsigned int mScopeTotalSize;
		bool mReturn;
		CodeStream *mStream;
		asNamespace *mDefaultXMLNamespace;
		as_object *mCurrentScope;
		as_value *mGlobalReturn;
		as_object *mThis;
		std::vector<as_value> mRegisters;

	void to_debug_string(){
		LOG_DEBUG_AVM("StackDepth=%u StackTotalSize=%u ScopeStackDepth=%u ScopeTotalSize=%u",mStackDepth,mStackTotalSize,mScopeStackDepth,mScopeTotalSize);

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

	Property* find_prop_strict(asName multiname);

	void get_property(string_table::key name,string_table::key ns);
	
	void get_property(Property* p);

	as_value get_property_value(asName multiname);

	as_value get_property_value(boost::intrusive_ptr<as_object> obj, asName multiname);

	void print_stack();

	void print_scope_stack();

	std::auto_ptr< std::vector<as_value> > get_args(unsigned int argc);
	
	void load_function(CodeStream* stream);

	as_environment::ScopeStack* getScopeStack();

	void executeCodeblock(CodeStream* stream);

	as_value get_register(int index){
		LOG_DEBUG_AVM("Getting value at a register %d ",index);
		return mRegisters[index];
	}

	void push_stack(as_value object){
		LOG_DEBUG_AVM("Pushing value %s onto stack.",object.toDebugString());
		mStack.push(object);
		print_stack();
	}

	as_value pop_stack(){
		as_value value = mStack.pop();
		LOG_DEBUG_AVM("Poping value %s off the stack.",value.toDebugString());
		print_stack();
		return value;
	}

	void push_scope_stack(as_value object){
		LOG_DEBUG_AVM("Pushing value onto the scope stack.");
		mAsValueScopeStack.push(object);
		LOG_DEBUG_AVM("There are now %u items on the scope stack.",mAsValueScopeStack.size());
	}

	as_value pop_scope_stack(){
		LOG_DEBUG_AVM("Poping value off the scope stack.  There will be %u items left.",mAsValueScopeStack.size()-1);
		return mAsValueScopeStack.pop();
	}
	as_value get_scope_stack(boost::uint8_t depth){
		LOG_DEBUG_AVM("Geting value from scope stack %u from the top.",depth | 0x0);
		return as_value(mAsValueScopeStack.top(depth));
	}

	SafeStack<as_value> mStack;
	SafeStack<State> mStateStack;
	SafeStack<Scope> mScopeStack;
	std::vector<as_value> mRegisters;
	SafeStack<as_value> mAsValueScopeStack;
	CodeStream *mStream;

	ClassHierarchy *mCH;
	string_table& mST;

	asNamespace* mDefaultXMLNamespace;
	as_object* mCurrentScope;
	as_object* mGlobalScope;
	as_object* mDefaultThis;
	as_object* mThis;

	as_value mGlobalReturn;
	as_value mIgnoreReturn; // Throw away returns go here.

	bool mIsAS3; // Is the stream an AS3 stream.
	bool mExitWithReturn;
	abc_block* mPoolObject; // Where all of the pools are stored.

	VM& _vm;
};

} // namespace gnash
#endif /* GNASH_MACHINE_H */
