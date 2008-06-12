// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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


#ifndef GNASH_SIMPLEBUFFER_H
#define GNASH_SIMPLEBUFFER_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

//#include "dsodefs.h" // for DSOEXPORT (not used)

#include <cassert> // for assert
#include <boost/cstdint.hpp> // for boost::uint8_t
#include <algorithm> // for std::copy

namespace gnash {

/// A simple buffer of bytes
//
/// This class is fully inlined and just aiming to provide RIIA
/// and unified view of memory buffers.
/// It is a kind of a std::vector with a reduced interface
/// in the intentions of the author.
///
class SimpleBuffer {

public:

	/// Construct a SimpleBuffer with an optional initial capacity
	//
	/// @param capacity
	///	The initial buffer capacity. This is the amount of
	///	bytes you can append to the buffer before a new reallocation
	///	will occur.
	///
	SimpleBuffer(size_t capacity=0)
		:
		_data(0),
		_size(0),
		_capacity(capacity)
	{
		if ( _capacity ) _data = new boost::uint8_t[_capacity];
	}

	~SimpleBuffer()
	{
		delete [] _data;
	}

	/// Copy constructor
	//
	/// The copy ctor will set capacity to be
	/// as small as required to hold the size of the
	/// model buffer.
	///
	SimpleBuffer(const SimpleBuffer& b)
		:
		_data(0),
		_size(b._size),
		_capacity(b._size)
	{
		if ( _size )
		{
			_data = new boost::uint8_t[_size];
			std::copy(b._data, b._data+b._size, _data);
		}
	}

	/// Assignment operator
	//
	/// The assignment op will not reset capacity
	///
	SimpleBuffer& operator= (const SimpleBuffer& b)
	{
		if ( this != &b )  // don't waste time on self-assignment
		{
			resize(0); // shouldn't deallocate memory
			append(b);
		}
		return *this;
	}

	/// Return true if buffer is empty
	bool empty() const { return _size==0; }

	/// Return size of the buffer
	size_t size() const { return _size; }

	/// Return capacity of the buffer
	size_t capacity() const { return _capacity; }

	/// Get a pointer to start of data. May be NULL if size==0.
	boost::uint8_t* data() { return _data; }

	/// Get a pointer to start of data. May be NULL if size==0.
	const boost::uint8_t* data() const { return _data; }

	/// Resize the buffer
	void resize(size_t newSize)
	{
		reserve(newSize); // will set capacity
		_size = newSize;
	}

	/// Ensure at least 'newCapacity' bytes are allocated for this buffer
	void reserve(size_t newCapacity)
	{
		if ( _capacity >= newCapacity ) return;

		// TODO: use smalles power of 2 bigger then newCapacity
		_capacity = std::max(newCapacity, _capacity*2);

		boost::uint8_t* tmp = _data;
		_data = new boost::uint8_t[_capacity];
		if ( tmp )
		{
			if ( _size ) std::copy(tmp, tmp+_size, _data);
			delete [] tmp;
		}
	}

	/// Append data to the buffer
	//
	/// The buffer will be appropriately resized to have space for
	/// the incoming data. The data will be copied.
	///
	/// @param newData
	///	Data to append. Will be copied.
	///
	/// @param size
	///	Size of data to append
	///
	void append(const boost::uint8_t* newData, size_t size)
	{
		size_t curSize = _size;
		resize(curSize+size);
		std::copy(newData, newData+size, _data+curSize);
		assert(_size == curSize+size);
	}

	/// Append data to the buffer
	//
	/// The buffer will be appropriately resized to have space for
	/// the incoming data. The data will be copied.
	///
	/// @param newData
	///	SimpleBuffer containing data to append
	///
	void append(const SimpleBuffer& buf)
	{
		size_t incomingDataSize = buf.size();
		const boost::uint8_t* incomingData = buf.data();
		append(incomingData, incomingDataSize);
	}

private:

	boost::uint8_t* _data;

	size_t _size;
	size_t _capacity;

};


}	// namespace gnash

#endif // GNASH_SIMPLEBUFFER_H
