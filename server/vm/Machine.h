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

#include "as_value.h"

namespace gnash {

class ASException
{
};

class StackException : public ASException
{
};

template <class T>
class Stack
{
public:
	T& top(unsigned int i)
	{
		if (i >= mDownstop) 
			throw StackException();
		unsigned int offset = mEnd - i;
		return mData[offset >> mChunkShift][offset & mChunkMod];
	}

	T& value(unsigned int i)
	{
		if (i >= mDownstop)
			throw StackException();
		unsigned int offset = mEnd - mDownstop + i;
		return mData[offset >> mChunkShift][offset & mChunkMod];
	}

	void drop(unsigned int i)
	{ if (i >= mDownstop) throw StackException(); mDownstop -= i; mEnd -= i; }

	void push(const T& t)
	{ grow(1); top(0) = t; }

	void grow(unsigned int i)
	{
		if (((mEnd + i) >> mChunkShift) > mData.size()) 
		{
			if (i > (1 << mChunkShift))
				throw StackException();
			mData.push_back(new data_vector(1 << mChunkShift));
			mData.back()->resize(1 << mChunkShift);
		}
		mDownstop += i;
		mEnd += i;
	}

	unsigned int getDownstop() const 
	{ return mDownstop; }

	unsigned int fixDownstop() 
	{ unsigned int ret = mDownstop; mDownstop = 0; return ret; }

	void setDownstop(unsigned int i)
	{ if (mDownstop > mEnd) throw StackException(); mDownstop = i; }

	Stack() : mData(), mDownstop(1), mEnd(1)
	{ /**/ }

	~Stack()
	{
		stack_type::iterator i = mData.begin();
		for ( ; i != mData.end(); ++i)
		{
			delete (*i);
		}
	}

private:
	typedef std::vector<T> data_vector;
	typedef std::vector<data_vector *> stack_type;
	stack_type mData;
	unsigned int mDownstop;
	unsigned int mEnd;

	// If mChunkMod is not a power of 2 less 1, it will not work properly.
	static const unsigned int mChunkShift = 6;
	static const unsigned int mChunkMod = (1 << mChunkShift) - 1;
};

class CodeStream
{
public:
	uint32_t read_V32();
	uint8_t read_as3op();
	std::size_t tell();
	void seekBy(int change);
	void seekTo(std::size_t set);
	int32_t read_S24();
	int8_t read_s8();
	uint8_t read_u8();
	void skip_V32();
};

class FunctionEntry
{
public:
	FunctionEntry(Stack<as_value> *s) : mStack(s)
	{ mStackReset = s->fixDownstop(); }

	~FunctionEntry()
	{ if (mStack) mStack->setDownstop(mStackReset); }

private:
	unsigned int mStackReset;
	Stack<as_value> *mStack;
};

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

	void pushCall(int param_count, int return_count, asMethod *pMethod);

private:
	Stack<as_value> mStack;
	Stack<FunctionEntry> mCallStack;
	Stack<as_value> mFrame;
	Stack<asScope*> mScopeStack;
	CodeStream mStream;

	ClassHierarchy *mCH;

	asNamespace* mDefaultXMLNamespace;
	asScope *mGlobalScope;
	asScope *mCurrentScope;
	asScope *mBaseScope;
};

} // namespace gnash
#endif /* GNASH_MACHINE_H */
