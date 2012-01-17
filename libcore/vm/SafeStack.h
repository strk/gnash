// SafeStack.h A stack which doesn't drop or free references until explicitly
// asked to do so, so that values outside of the stack are guaranteed good
// in an appropriate scope.
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
#ifndef GNASH_SAFESTACK_H
#define GNASH_SAFESTACK_H


#include <vector>

namespace gnash {

class StackException {};

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

	/// From the top of the stack, get the i'th value down.
    //
    /// 0 is the topmost value.
	const T& top(StackSize i) const
	{

		if (i >= size()) throw StackException();
		const StackSize offset = _end - i;
		return _data[offset >> _chunkShift][offset & _chunkMod];
	}

	/// From the top of the stack, get the i'th value down. 
    //
    /// This is a non-const version of top().
    /// 0 is the topmost value value.
	T& top(StackSize i)
	{

		if (i >= size()) throw StackException();
		const StackSize offset = _end - i;
		return _data[offset >> _chunkShift][offset & _chunkMod];
	}
    
    /// From the top of the stack, get the i'th value down. 
    //
    /// 0 is the topmost value value.
	const T& at(StackSize i) const
	{

		if (i >= totalSize()) throw StackException();
		const StackSize offset = _end - i;
		return _data[offset >> _chunkShift][offset & _chunkMod];
	}

	
    /// From the top of the stack, get the i'th value down. 
    //
    /// This is a non-const version of at().
    /// 0 is the topmost value value.
	T& at(StackSize i)
	{

		if (i >= totalSize()) throw StackException();
		const StackSize offset = _end - i;
		return _data[offset >> _chunkShift][offset & _chunkMod];
	}

	/// From the bottom of the stack, get the i'th value up. 0 is the
	/// bottommost value.
	T& value(StackSize i)
	{
		if (i >= size()) throw StackException();

		StackSize offset = _downstop + i + 2;
		return _data[offset >> _chunkShift][offset & _chunkMod];
	}

	const T& value(StackSize i) const
	{
		if (i >= size()) throw StackException();

		StackSize offset = _downstop + i + 2;
		return _data[offset >> _chunkShift][offset & _chunkMod];
	}

	/// Assign a value to given index counting from bottom.
	void assign(StackSize i, T val)
	{ 
		if (i >= size()) throw StackException();

		StackSize offset = _downstop + i + 2;
		_data[offset >> _chunkShift][offset & _chunkMod] = val;
	}

	/// Shrink the stack by i entries. Does not invalidate any entries
	/// previously given, it just sets the top for pop, push, and top
	/// operations.
	void drop(StackSize i) {
        if (i > size()) throw StackException();
        _end -= i;
    }

	/// Drop all stack elements reguardless of the "downstop"
	void clear() {
        _downstop = 0;
        _end = 1;
    }

	/// Put a new value onto the top of the stack.  The value will be
	/// copied.
	void push(const T& t) {
        grow(1);
        top(0) = t;
    }

	/// Pop the top of the stack.
	T& pop() {
        T& ret = top(0);
        drop(1);
        return ret;
    }

	/// Grow by i entries. Normally this is 1, but there might be sometime
	/// when you need more than that.
	void grow(StackSize i)
	{
		StackSize available = (1 << _chunkShift) * _data.size() - _end + 1;
		StackSize n = size()+i;
		while (available < n)
		{
            //log_debug("Increasing size of the real stack: %d.",_data.size());
			_data.push_back(new T[1 << _chunkShift]);
			available += 1 << _chunkShift;
		}
		_end += i;
	}

	/// Gives the size of the stack which is currently accessible.
	StackSize getDownstop() const 
	{
        return _downstop;
    }

	/// Alias for getDownstop()
	StackSize size() const { return _end - _downstop - 1; }

	/// Is the stack empty to us? (Check totalSize() != for actually empty)
	bool empty() const { return size() == 0; }

	/// Makes the stack appear empty to subsequent callers.  This can be used
	/// to simulate multiple stacks with a single stack, as in function
	/// calling. Returns the old downstop for restoring it using setDownstop.
	StackSize fixDownstop() 
	{
        StackSize ret = _downstop;
        _downstop = _end - 1;
        return ret;
    }

	/// Makes the stack read to a depth of 'i'. This cannot be more than
	/// totalSize()
	void setDownstop(StackSize i)
	{
        if (i > _end) throw StackException();
        _downstop = i;
    }

    /// Return the complete stack size, including non-accessible elements
    //
    /// This is required because AVM2 scope stacks are usable even when they
    /// appear inaccessible
	StackSize totalSize() const { return _end - 1; }

	/// Set the total size and local size of the stack, for restoring a
	/// stack through unknown changes.
	void setAllSizes(StackSize total, StackSize downstop)
	{
        _end = total + 1;
        _downstop = downstop;
    }

	/// Default constructor.
	SafeStack() : _data(), _downstop(0), _end(1) {}

	/// Delete the allocated data. 
	~SafeStack()
	{
		for (StackSize i = 0; i < _data.size(); ++i) delete [] _data[i];
	}

private:
	StackType _data;
	StackSize _downstop;
	StackSize _end;

	// If _chunkMod is not a power of 2 less 1, it will not work properly.
	static const StackSize _chunkShift = 6;
	static const StackSize _chunkMod = (1 << _chunkShift) - 1;
};

} // namespace gnash
#endif 
