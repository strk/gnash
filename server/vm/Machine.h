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
#include "asClass.h"
#include "swf.h"

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
	/// The binding.  If this is only a partial binding, then
	/// the 'this' value will be used to complete it, when possible.
	/// Sending a null binding will result in a no-op, not an error.
	void pushCall(unsigned int stack_in, as_value *return_slot,
		Property *pBind);

	void immediateFunction(as_function *to_call, as_value& storage,
		as_object *pThis);
	void immediateProcedure(as_function *to_call, as_object *pthis,
		const as_value *stackAdditions, unsigned int stackAdditionsCount);

	Machine(string_table &ST, ClassHierarchy *CH);

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
		as_object *mCurrentScope;
		as_value *mGlobalReturn;
		as_object *mThis;
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

	SafeStack<as_value> mStack;
	SafeStack<State> mStateStack;
	SafeStack<Scope> mScopeStack;
	SafeStack<as_value> mFrame;
	CodeStream *mStream;

	ClassHierarchy *mCH;
	string_table& mST;

	asNamespace* mDefaultXMLNamespace;
	as_object* mCurrentScope;
	as_object* mGlobalScope;
	as_object* mDefaultThis;
	as_object* mThis;

	as_value *mGlobalReturn;
	as_value mIgnoreReturn; // Throw away returns go here.

	bool mIsAS3; // Is the stream an AS3 stream.
	abc_block* mPoolObject; // Where all of the pools are stored.
};

} // namespace gnash
#endif /* GNASH_MACHINE_H */
