// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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


#include <boost/cstdint.hpp> // for boost::uint8_t
#include <algorithm> // for std::copy
#include <memory>
#include <cassert>


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
		_size(0),
		_capacity(capacity)
	{
		if ( _capacity )
		{
			_data.reset(new boost::uint8_t[_capacity]);
		}
	}

	/// Copy constructor
	//
	/// The copy ctor will set capacity to be
	/// as small as required to hold the size of the
	/// model buffer.
	///
	SimpleBuffer(const SimpleBuffer& b)
		:
		_size(b._size),
		_capacity(b._size)
	{
		if ( _size )
		{
			_data.reset(new boost::uint8_t[_size]);
			std::copy(b.data(), b.data()+b.size(), _data.get());
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
	boost::uint8_t* data() { return _data.get(); }

	/// Get a pointer to start of data. May be NULL if size==0.
	const boost::uint8_t* data() const { return _data.get(); }

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

		std::unique_ptr<boost::uint8_t[]> tmp;
		tmp.swap(_data);
		
		_data.reset(new boost::uint8_t[_capacity]);

		if ( tmp.get() )
		{
			if ( _size ) std::copy(tmp.get(), tmp.get()+_size, _data.get());
		}
	}

	/// Append data to the buffer
	//
	/// The buffer will be appropriately resized to have space for
	/// the incoming data. The data will be copied.
	///
	/// @param inData
	///	Data to append. Will be copied.
	///
	/// @param size
	///	Size of data to append
	///
	void append(const void* inData, size_t size)
	{
		const boost::uint8_t* newData = 
            reinterpret_cast<const boost::uint8_t*>(inData);
		size_t curSize = _size;
		resize(curSize+size);
		std::copy(newData, newData+size, _data.get()+curSize);
		assert(_size == curSize+size);
	}

	/// Append a byte to the buffer
	//
	/// The buffer will be appropriately resized to have space.
	///
	/// @param b
	///	Byte to append.
	///
	void appendByte(const boost::uint8_t b)
	{
		resize(_size + 1);
		_data[_size - 1] = b;
	}

	/// Append 2 bytes to the buffer
	//
	/// The buffer will be appropriately resized to have space.
	///
	/// @param s
	///	Short to append. Will be appended in network order. ie
	///  with high order byte first.
	///
	void appendNetworkShort(const boost::uint16_t s)
	{
		resize(_size + 2);
		_data[_size - 2] = s >> 8;
		_data[_size - 1] = s & 0xff;
	}

	/// Append 4 bytes to the buffer
	//
	/// The buffer will be appropriately resized to have space.
	///
	/// @param l
	///	Long to append. Will be appended in network order. ie
	///  with high order bytes first.
	///
	void appendNetworkLong(const boost::uint32_t l)
	{
		resize(_size + 4);
		_data[_size - 4] = l >> 24;
		_data[_size - 3] = (l >> 16) & 0xff;
		_data[_size - 2] = (l >> 8) & 0xff;
		_data[_size - 1] = l & 0xff;
	}

	/// Append data to the buffer
	//
	/// The buffer will be appropriately resized to have space for
	/// the incoming data. The data will be copied.
	///
	/// @param buf
	///	SimpleBuffer containing data to append
	///
	void append(const SimpleBuffer& buf)
	{
		size_t incomingDataSize = buf.size();
		const boost::uint8_t* incomingData = buf.data();
		append(incomingData, incomingDataSize);
	}

private:
	size_t _size;
	size_t _capacity;

	std::unique_ptr<boost::uint8_t[]> _data;
};


}	// namespace gnash

#endif // GNASH_SIMPLEBUFFER_H
