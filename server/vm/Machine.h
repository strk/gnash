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

class Machine
{
public:
	// Flash specific members.
	/// The character which initiated these actions.
	character *getTarget();

	/// Set the character which initiated the actions.
	/// Not null.
	void setTarget(character* target);

	void completeName(asBindingName&);

	asClass* findSuper(as_value&, bool);

	as_value getMember(asClass*, asBindingName&, as_value&);
	bool setMember(asClass*, asBindingName&, as_value& target, as_value& val);

	std::string pool_string(uint32_t);
	int pool_int(uint32_t);
	unsigned int pool_uint(uint32_t);
	double pool_double(uint32_t);
	asNamespace* pool_namespace(uint32_t);
	asMethod* pool_method(uint32_t);

	as_value(v) findProperty(asBindingName&);

	void pushScope(asScope*);
	asScope* popScope();

	void execute_as3();

	void execute_as2();

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
