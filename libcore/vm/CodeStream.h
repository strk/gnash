// CodeStream.h A class which allows bounds-checked reading from a char array
//
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_CODESTREAM_H
#define GNASH_CODESTREAM_H

#include <boost/utility.hpp>

namespace gnash {

/// The exception which will be thrown by the CodeStream for access
/// violations.
class CodeStreamException { };

/// A checked read DisplayObject array
///
/// CodeStream provides a safe interface to read various things from a
/// DisplayObject array of known size.  Any attempt to access memory outside
/// of the given array will throw an exception of type CodeStreamException
class CodeStream : private boost::noncopyable
{
public:
	/// Construct a CodeStream
	///
	/// @param pStart
	/// The beginning of the DisplayObject array
	///
	/// @param length
	/// The length of the array. Memory in [pStart, pStart + length) may
	/// be accessed. It is not an error to send a length of 0.
	///
	/// @param own
	/// If true, the given memory will be copied. Otherwise, the caller
	/// retains ownership and should not delete the memory before this
	/// is invalid.
	CodeStream(const char *pStart, std::size_t length, bool own = false) :
		mRaw(pStart), mRawEnd(pStart + length), mEnd(pStart + length),
		mOwn(false)
	{
		if (own && length > 0)
		{
			mCurrent = mRaw = new char[length];
			memcpy(const_cast<char*>(mRaw), pStart, length);
			mRawEnd = mEnd = mRaw + length;
			mOwn = true;
		}
	}

	/// Construct an empty CodeStream. Call reInitialize to fill it.
	CodeStream() : mRaw(NULL), mRawEnd(NULL), mEnd(NULL), mOwn(false)
	{/**/}

	/// Destruct a CodeStream
	///
	/// If the stream owns the memory, it will destroy it.
	~CodeStream()
	{ if (mOwn) delete [] mRaw; }

	/// Pseudo-construct a CodeStream
	///
	/// This has the same parameters as the non-default constructor,
	/// but it can be used to re-initialize the CodeStream object.
	void reInitialize(const char *pStart, std::size_t length,
		bool own = false)
	{
		if (own)
		{
			// Delete mRaw if it's not large enough and it's ours.
			if (mOwn && length > static_cast<unsigned int> (mRawEnd - mRaw))
			{
				mOwn = false;
				delete [] mRaw;
			}
			if (!mOwn)
				mRaw = new char [length];
			memcpy(const_cast<char *>(mRaw), pStart, length);
			mEnd = mRawEnd = pStart + length;
			mCurrent = mRaw;
			return;
		}

		if (mOwn)
		{
			// We own now, but don't want to.
			delete [] mRaw;
		}
		mCurrent = mRaw = pStart;
		mEnd = mRawEnd = pStart + length;
	}

	/// Read a variable length encoded 32 bit unsigned integer
	boost::uint32_t read_V32()
	{
		if (mCurrent == mEnd) throw CodeStreamException();

		// We can do an unchecked read in these cases.
		if (mEnd - mCurrent > 4 || !(*(mEnd - 1) & 0x80))
		{
			boost::uint32_t result = *mCurrent++;
			if (!(result & 0x00000080))	return result;
			result = (result & 0x0000007F) | *mCurrent++ << 7;
			if (!(result & 0x00004000)) return result;
			result = (result & 0x00003FFF) | *mCurrent++ << 14;
			if (!(result & 0x00200000)) return result;
			result = (result & 0x001FFFFF) | *mCurrent++ << 21;
			if (!(result & 0x10000000)) return result;
			return (result & 0x0FFFFFFF) | *mCurrent++ << 28;
		}	
		boost::uint32_t result = *mCurrent++;
		if (!(result & 0x00000080))	return result;
		if (mCurrent == mEnd) throw CodeStreamException();
		result = (result & 0x0000007F) | *mCurrent++ << 7;
		if (!(result & 0x00004000)) return result;
		if (mCurrent == mEnd) throw CodeStreamException();
		result = (result & 0x00003FFF) | *mCurrent++ << 14;
		if (!(result & 0x00200000)) return result;
		if (mCurrent == mEnd) throw CodeStreamException();
		result = (result & 0x001FFFFF) | *mCurrent++ << 21;
		if (!(result & 0x10000000)) return result;
		if (mCurrent == mEnd) throw CodeStreamException();
		return (result & 0x0FFFFFFF) | *mCurrent++ << 28;
	}

	/// Read an opcode for ActionScript 3
	boost::uint8_t read_as3op()
	{
		if (mCurrent == mEnd)
			return 0;
		return static_cast<boost::uint8_t> (*mCurrent++);
	}

	/// Provide the offset into the stream of the current position. Can be
	/// used for seeking.
	std::size_t tell()
	{ return mCurrent - mRaw; }

	/// Change the current position by a relative value.
	void seekBy(int change)
	{
		if ((change > 0 && change > (mEnd - mCurrent)) ||
			(change < 0 && -change > (mCurrent - mRaw)))
			throw CodeStreamException();
		mCurrent += change;
	}

	/// Set the current position to an absolute value (relative to the start)
	void seekTo(unsigned int set)
	{
		if (set > static_cast<unsigned int> (mEnd - mRaw))
			throw CodeStreamException();
		mCurrent = mRaw + set;
	}

	/// Read a signed integer encoded in 24 bits.
	boost::int32_t read_S24()
	{
		if (mEnd - mCurrent < 3)
			throw CodeStreamException();
		int result = *mCurrent++ + (*mCurrent++ << 8) + (*mCurrent ++ << 16);
		if (result & (1 << 23)) // Negative result, adjust appropriately.
			result = -(result & ~(1 << 23));
		return static_cast<boost::int32_t>(result);
	}

	/// Read a signed 8-bit DisplayObject.
    boost::int8_t read_s8()
	{
		if (mCurrent == mEnd)
			throw CodeStreamException();
		return static_cast<boost::int8_t> (*mCurrent++);
	}

	/// Read an unsigned 8-bit DisplayObject.
	boost::uint8_t read_u8()
	{
		if (mCurrent == mEnd)
			throw CodeStreamException();
		return static_cast<boost::uint8_t> (*mCurrent++);
	}

	/// Set a stop at position bytes from the start. This becomes the new
	/// effective end of the code stream, but the end may be unstopped or
	/// set to different length. In no case can the end be set to a point
	/// beyond the original end of the stream.
	void set_end(unsigned int position)
	{
		if (position > static_cast<unsigned int>(mRawEnd - mRaw))
			throw CodeStreamException();
		mEnd = mRaw + position;
		if (mCurrent > mEnd)
			mCurrent = mEnd;
	}

	/// Unset any stop placed on the stream.
	void unset_end()
	{ mEnd = mRawEnd; }

	/// Take ownership of mRaw.  mRaw should be a block which can be
	/// de-allocated by calling delete [] mRaw
	void takeMemoryOwnership()
	{ mOwn = true; }

	/// Same as read_V32(), but doesn't bother with the arithmetic for
	/// calculating the value.
	void skip_V32()
	{
		if (mCurrent == mEnd) throw CodeStreamException();

		// We can do an unchecked read in these cases.
		if (mEnd - mCurrent > 4 || !(*(mEnd - 1) & 0x80))
		{
			// shortcut evalution is mandated as standard.
			if ((*mCurrent++ & 0x80) && (*mCurrent++ & 0x80) && (*mCurrent++ &0x80)
				&& (*mCurrent++ & 0x80) && (*mCurrent++ & 0x80))
				return;
			return;
		}	
		if (!(*mCurrent++ & 0x80)) return;
		if (mCurrent == mEnd) throw CodeStreamException();
		if (!(*mCurrent++ & 0x80)) return;
		if (mCurrent == mEnd) throw CodeStreamException();
		if (!(*mCurrent++ & 0x80)) return;
		if (mCurrent == mEnd) throw CodeStreamException();
		if (!(*mCurrent++ & 0x80)) return;
		if (mCurrent == mEnd) throw CodeStreamException();
		++mCurrent;
	}

private:
	const char *mRaw; // Nobody may write into this, and we handle all access.
	const char *mRawEnd; // We own the memory in [mRaw, mRawEnd)
	const char *mEnd; // We may not read this or beyond.
	const char *mCurrent; // Our current read pointer.
	bool mOwn; // Do we own the memory?
};

} // namespace gnash
#endif /* GNASH_CODESTREAM_H */
