//   Copyright (C) 2007 Free Software Foundation, Inc.
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

#include "SafeStack.h"
#include "as_value.h"

namespace gnash {

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
	int completeName(asBoundName& name, int initial = 0);

	/// Given a value v, find the class object of the superclass of v.
	///
	/// @param obj
	/// The object whose superclass is desired.
	///
	/// @param find_primitive
	/// If true, the ActionScript prototype will be find for primitive values.
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
	void getMember(asClass* pDefinition, asBoundName& name, as_value& source);

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
	void setMember(asClass*, asBoundName&, as_value& target, as_value& val);

	std::string& pool_string(uint32_t);
	int pool_int(uint32_t);
	unsigned int pool_uint(uint32_t);
	double pool_double(uint32_t);
	asNamespace* pool_namespace(uint32_t);
	asMethod* pool_method(uint32_t);

	as_value findProperty(asBoundName&);

	void pushScope(asScope*);
	asScope* popScope();

	void execute_as3();

	void execute_as2();

	/// push a function call to be executed next.
	///
	/// Any asBinding can be pushed, and it will appropriate value
	/// into return_slot.  This ensures that getter/setter properties
	/// can be accessed in the same way as other properties, and hides
	/// the difference between ActionScript methods and native C++ methods.
	///
	/// @param stack_in
	/// The initial stack size when the function is entered. This can be used
	/// to pass 'this' and other parameters to the call.
	///
	/// @param stack_out
	/// The maximum number of values to leave on the stack when the function
	/// returns.
	///
	/// @param return_slot
	/// A space for the return value. An assignment will always be made here,
	/// but mVoidSlot can be used for values that will be discarded.
	///
	/// @param pBind
	/// The non-null binding.  If this is only a partial binding, then
	/// the 'this' value will be used to complete it, when possible.
	void pushCall(unsigned int stack_in, unsigned int stack_out,
		as_value &return_slot, asBinding *pBind);

private:
	/// The state of the machine.
	class State
	{
	public:
		unsigned int mStackDepth;
		unsigned int mStackTotalSize;
		unsigned int mScopeStackDepth;
		unsigned int mScopeTotalSize;
		CodeStream *mStream;
		asNamespace *mDefaultXMLNamespace;
		asScope *mCurrentScope;
		asValue *mGlobalReturn;
	};

	void saveState();
	void restoreState();

	Stack<as_value> mStack;
	Stack<State> mStateStack;
	Stack<asScope> mScopeStack;
	CodeStream *mStream;

	ClassHierarchy *mCH;
	string_table& mST;

	asNamespace* mDefaultXMLNamespace;
	asScope *mGlobalScope;
	asScope *mCurrentScope;

	asValue *mGlobalReturn;
	asValue mIgnoreReturn; // Throw away returns go here.
};

} // namespace gnash
#endif /* GNASH_MACHINE_H */
