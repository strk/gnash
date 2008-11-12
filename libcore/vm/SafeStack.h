// SafeStack.h A stack which doesn't drop or free references until explicitly
// asked to do so, so that values outside of the stack are guaranteed good
// in an appropriate scope.
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
#ifndef GNASH_SAFESTACK_H
#define GNASH_SAFESTACK_H

#include "log.h"

#include <vector>

namespace gnash {

class StackException {/**/};

/// A stack in which all references given remain valid while the stack lives.
///
/// Safe in SafeStack means that you can maintain a reference
/// given by the stack as long as the stack is alive.  Since it is a reference,
/// there is no guarantee that it will remain constant, but it is guaranteed
/// that it will remain valid.
///
/// Access outside of the bounds of the stack will result in a StackException
/// being thrown.
template <class T>
class SafeStack
{

	typedef std::vector<T*> StackType;

public:

    // May be useful for callers to know.
    typedef typename StackType::size_type StackSize;

	/// From the top of the stack, get the i'th value down. 0 is the topmost
	/// value.
	T& top(StackSize i)
	{

		if (i >= size()) 
			throw StackException();
		StackSize offset = mEnd - i;
		//log_debug("top(%d): mEnd:%d, mDownstop:%d, offset:%d", i, mEnd, mDownstop, offset);
		return mData[offset >> mChunkShift][offset & mChunkMod];
	}

	/// From the bottom of the stack, get the i'th value up. 0 is the
	/// bottommost value.
	T& value(StackSize i)
	{
		if (i >= size())
			throw StackException();

		StackSize offset = mDownstop + i + 2;
		//log_debug("value(%d): mEnd:%d, mDownstop:%d, offset:%d", i, mEnd, mDownstop, offset);
		return mData[offset >> mChunkShift][offset & mChunkMod];
	}

	const T& value(StackSize i) const
	{
		if (i >= size())
			throw StackException();

		StackSize offset = mDownstop + i + 2;
		//log_debug("value(%d): mEnd:%d, mDownstop:%d, offset:%d", i, mEnd, mDownstop, offset);

		return mData[offset >> mChunkShift][offset & mChunkMod];
	}

	/// Assign a value to given index counting from bottom.
	void assign(StackSize i, T val)
	{ 
		if (i >= size())
			throw StackException();

		StackSize offset = mDownstop + i + 2;
		//log_debug("value(%d): mEnd:%d, mDownstop:%d, offset:%d", i, mEnd, mDownstop, offset);
		mData[offset >> mChunkShift][offset & mChunkMod] = val;
	}

	/// Shrink the stack by i entries. Does not invalidate any entries
	/// previously given, it just sets the top for pop, push, and top
	/// operations.
	void drop(StackSize i)
	{ if (i > size()) throw StackException(); mEnd -= i; }

	/// Drop all stack elements reguardless of the "downstop"
	void clear()
	{ mDownstop=0; mEnd=1; }

	/// Put a new value onto the top of the stack.  The value will be
	/// copied.
	void push(const T t)
	{ grow(1); top(0) = t; }

	/// Pop the top of the stack.
	T& pop()
	{	T& ret = top(0); drop(1); return ret; }

	/// Grow by i entries. Normally this is 1, but there might be sometime
	/// when you need more than that.
	void grow(StackSize i)
	{
		StackSize available = (1 << mChunkShift) * mData.size() - mEnd + 1;
		StackSize n = size()+i;
		while (available < n)
		{
//			log_debug("Increasing size of the real stack: %d.",mData.size());
			mData.push_back(new T[1 << mChunkShift]);
			available += 1 << mChunkShift;
		}
		mEnd += i;
	}

	/// Gives the size of the stack which is currently accessible.
	StackSize getDownstop() const 
	{ return mDownstop; }

	/// Alias for getDownstop()
	StackSize size() const { return mEnd - mDownstop - 1; /*mEnd is one past end*/ }

	/// Is the stack empty to us? (Check totalSize() != for actually empty)
	bool empty() const { return size() == 0; }

	/// Makes the stack appear empty to subsequent callers.  This can be used
	/// to simulate multiple stacks with a single stack, as in function
	/// calling. Returns the old downstop for restoring it using setDownstop.
	StackSize fixDownstop() 
	{ StackSize ret = mDownstop; mDownstop = mEnd-1; return ret; }

	/// Makes the stack read to a depth of 'i'. This cannot be more than
	/// totalSize()
	void setDownstop(StackSize i)
	{ if (i > mEnd) throw StackException(); mDownstop = i; }

	/// The total size of the stack. This is not what can be read. That
	/// value is given by size()
	///
	/// This function is probably not what you need for anything except for
	/// setting downstops that weren't returned by either fixDownstop() or
	/// getDownstop()
	StackSize totalSize() const { return mEnd - 1; }

	/// Set the total size and local size of the stack, for restoring a
	/// stack through unknown changes.
	void setAllSizes(StackSize total, StackSize downstop)
	{ mEnd = total + 1; mDownstop = downstop; }

	/// Default constructor.
	SafeStack() : mData(), mDownstop(0), mEnd(1)
	{ /**/ }

	/// Delete the allocated data. 
	~SafeStack()
	{
		for (StackSize i = 0; i < mData.size(); ++i)
			delete [] mData[i];
	}

private:
	StackType mData;
	StackSize mDownstop;
	StackSize mEnd;

	// If mChunkMod is not a power of 2 less 1, it will not work properly.
	static const StackSize mChunkShift = 6;
	static const StackSize mChunkMod = (1 << mChunkShift) - 1;
};

} // namespace gnash
#endif /* GNASH_SAFESTACK_H */
