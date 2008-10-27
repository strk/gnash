// 
//   Copyright (C) 2008 Free Software Foundation, Inc.
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
//

#ifndef __BUFFER_H__
#define __BUFFER_H__ 1

#include <boost/cstdint.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream> // for output operator
#include <string>

#include "getclocktime.hpp"
#include "amf.h"
#include "element.h"
#include "network.h"
#include "dsodefs.h"

// _definst_ is the default instance name

/// \namespace amf
///
/// This namespace is for all the AMF specific classes in libamf.
namespace amf
{

/// \class Buffer
///
/// This class is used to hold all data for libamf classes. It is a
/// simplified form of std::vector, but with more knowledge of data
/// types when copying or appending data to make higher level code
/// easier to read.
class DSOEXPORT Buffer 
{
public:
    /// \brief Create a new Buffer with the default size
    Buffer();
    /// \brief Create a new Buffer with a size other than the default
    Buffer(size_t nbytes);

    /// \brief Create a new Buffer with a hex string.
    ///		This is primary used only for testing to create binary
    ///		data from an easy to read and edit format.
    /// @param str A hex string
    /// @example "00 03 05 0a"
    Buffer(const std::string &str);

    /// Delete the memory allocated for this Buffer
    ~Buffer();

    /// \brief Encode a Buffer from a hex string.
    ///
    /// @param str A hex string.
    /// @example "00 03 05 0a"
    ///
    /// @return A reference to a Buffer in host endian format. This is
    ///		primary used only for testing to create binary data
    ///		from an easy to read and edit format.
    Buffer &hex2mem(const std::string &str);
    
    /// \brief Clear the contents of the buffer by setting all the bytes to
    ///		zeros.
    ///
    /// @return nothing
    void clear();
    
    /// \brief Test to see if the buffer has any data.
    ///
    /// @return true or false
    bool empty() { return (_seekptr)?true:false; };

    /// \brief Resize the buffer that holds the data.
    ///		The new size of the current data is based on the
    ///		current amount of data within the allocated memory.
    ///		This is used to make a Buffer the same size as
    ///		the existing data, and to truncate the unsed portion
    ///		of the Buffer when copying to the new memory
    ///		location.
    /// 
    /// @return A reference to a Buffer.
    Buffer &resize();
    /// \brief Resize the buffer that holds the data.
    ///		If the size is larger than the existing data, the data
    ///		is copied into the new region. If the size is smaller
    ///		than the existing data, the remaining data is
    ///		truncated when copying to the new memory location.
    ///
    /// @param nbyte The size to resize the Buffer to.
    ///
    /// @return A reference to a Buffer.
    Buffer &resize(size_t nbytes);

    /// \brief Copy data into the buffer.
    ///		This overwrites all data, and resets the seek ptr.
    ///
    /// @param data A pointer to the raw bytes to copy into the
    ///		buffer.
    /// 
    /// @param nbytes The number of bytes to copy.
    ///		
    /// @return A reference to a Buffer.
    Buffer &copy(gnash::Network::byte_t *data, size_t nbytes);
    
    /// \brief Copy a Buffer class into the buffer.
    ///		This overwrites all data, and resets the seek ptr.
    ///
    /// @param buf A Buffer class containing the data to copy.
    /// 
    /// @return A reference to a Buffer.
    Buffer &operator=(Buffer &buf);
    Buffer &operator=(boost::shared_ptr<Buffer>& buf);
    /// \brief Copy a string into the buffer.
    ///		This overwrites all data, and resets the seek ptr.
    ///
    /// @param str A string containing ASCII data to copy into the
    ///		buffer.
    /// 
    /// @return A reference to a Buffer.
    Buffer &operator=(const std::string &str);
    Buffer &operator=(const char *str);
    /// \brief Copy a double into the buffer.
    ///		This overwrites all data, and resets the seek ptr.
    ///
    /// @param num A numeric double value.
    /// 
    /// @return A reference to a Buffer.
    Buffer &operator=(double num);
    /// \brief Copy a short into the buffer.
    ///		This overwrites all data, and resets the seek ptr.
    ///
    /// @param num A numeric short value.
    /// 
    /// @return A reference to a Buffer.
    Buffer &operator=(boost::uint16_t length);
    /// \brief Copy a byte into the buffer.
    ///		This overwrites all data, and resets the seek ptr.
    ///
    /// @param byte A single byte.
    /// 
    /// @return A reference to a Buffer.
    Buffer &operator=(gnash::Network::byte_t byte);
    /// \brief Copy a byte into the buffer.
    ///		This overwrites all data, and resets the seek ptr.
    ///
    /// @param byte A pointer to a single byte.
    /// 
    /// @return A reference to a Buffer.
    Buffer &operator=(gnash::Network::byte_t *byte);
    /// \brief Copy a AMF0 type into the buffer.
    ///		This overwrites all data, and resets the seek ptr.
    ///
    /// @param type An AMF0 type. 
    /// 
    /// @return A reference to a Buffer.
    Buffer &operator=(amf::Element::amf0_type_e type);
    /// Copy a boolean into the buffer. This overwrites all data, and
    ///		resets the seek ptr.
    ///
    /// @param flag A boolean. 
    /// 
    /// @return A reference to a Buffer.
    Buffer &operator=(bool flag);
    
    /// \brief Append data to existing data in the buffer.
    ///
    /// @param data A pointer to the raw bytes to append to the
    ///		buffer.
    /// 
    /// @param nbytes The number of bytes to append.
    ///		
    /// @return A reference to a Buffer.
    Buffer &append(gnash::Network::byte_t *data, size_t nbytes);

    /// \brief Append a Buffer class to existing data in the buffer.
    ///
    /// @param buf A Buffer class containing the data to append.
    /// 
    /// @return A reference to a Buffer.
    Buffer &operator+=(Buffer &buf);
    Buffer &operator+=(boost::shared_ptr<Buffer> &buf);

    /// \brief Append a string to existing data in the buffer.
    ///
    /// @param str A string containing ASCII data to copy into the
    ///		buffer.
    /// 
    /// @return A reference to a Buffer.
    Buffer &operator+=(const std::string &str);
    Buffer &operator+=(const char *str);
    /// \brief Append a double to existing data in the buffer.
    ///
    /// @param num A numeric double value.
    /// 
    /// @return A reference to a Buffer.
    Buffer &operator+=(double num);
    /// \brief Append a short to existing data in the buffer.
    /// 
    /// @param num A numeric short value.
    /// 
    /// @return A reference to a Buffer.
    Buffer &operator+=(boost::uint16_t length);
    /// \brief Append a byte to existing data in the buffer.
    ///
    /// @param byte A single byte.
    /// 
    /// @return A reference to a Buffer.
    Buffer &operator+=(gnash::Network::byte_t byte);
    Buffer &operator+=(char byte);
    /// \brief Append an AMF0 type to existing data in the buffer.
    ///
    /// @param type An AMF0 type. 
    /// 
    /// @return A reference to a Buffer.
    Buffer &operator+=(amf::Element::amf0_type_e type);
    /// \brief Append a boolean to existing data in the buffer.
    ///
    /// @param type A boolean. 
    /// 
    /// @return A reference to a Buffer.
    Buffer &operator+=(bool);
    
    /// \brief Find a set of bytes in the buffer
    ///
    /// @param byte The bytes to find in the buffer.
    ///
    /// @param size The size of the bytes being searched for.
    ///
    /// @return A real pointer to the address of the byte in the buffer.
    gnash::Network::byte_t *find(gnash::Network::byte_t *byte, size_t size);
    
    /// \brief Drop a byte without resizing.
    ///		This will remove the byte from the Buffer, and then
    ///		move the remaining data to be in the correct
    ///		location. This resets the seek pointer.
    ///
    /// @param byte The byte to remove from the buffer.
    ///
    /// @return A real pointer to the base address of the buffer.
    gnash::Network::byte_t *remove(gnash::Network::byte_t c);
    /// \brief Drop a byte without resizing.
    ///		This will remove the byte from the Buffer, and then
    ///		move the remaining data to be in the correct
    ///		location. This resets the seek pointer.
    ///
    /// @param index The location of the byte to remove from the
    ///		Buffer
    ///
    /// @return A real pointer to the base address of the Buffer.
    gnash::Network::byte_t *remove(int index);
    /// \brief Drop bytes without resizing.
    ///		This will remove the bytes from the Buffer, and then
    ///		move the remaining data to be in the correct
    ///		location. This resets the seek pointer.
    ///
    /// @param index The location of the byte to start removing data
    ///		from the Buffer. This is an numerical value, not a
    ///		pointer.
    ///
    /// @param start The location of the byte to remove from the
    ///		Buffer
    /// @param range The amoiunt of bytes to remove from the Buffer.
    ///
    /// @return A real pointer to the base address of the Buffer.
    gnash::Network::byte_t *remove(int start, int range);
//    Network::byte_t *remove(char c);
    
    /// \brief Return the base address of the Buffer.
    ///
    /// @return A real pointer to the base address of the Buffer.
    gnash::Network::byte_t *begin() { return _data.get() ; };
    gnash::Network::byte_t *reference() { return _data.get(); }
    /// \brief Return the last address of the Buffer
    ///		Which is the base address plus the total size of the
    ///		Buffer.
    ///
    /// @return A real pointer to the last address of the Buffer.
    gnash::Network::byte_t *end() { return begin() + _nbytes; };

    /// \brief Get the size of the Buffer.
    ///
    /// @return The size of the Buffer.
    size_t size() { return _nbytes; }
    
    /// \brief Set the size of the Buffer.
    ///		Note that this does not resize the Buffer, it merely
    ///		is a convienient way to set the size field of the
    ///		Buffer class, and should only be used by low level
    ///		internal code and testing.
    ///
    /// @param nbytes 
    ///
    /// @return The size of the Buffer.
    void setSize(size_t nbytes) { _nbytes = nbytes; };
    
    /// \brief Test equivalance against another Buffer.
    ///		This compares all the data on the current Buffer with
    ///		the supplied one, so it can be a performance hit. This
    ///		is primarily only used for testing purposes.
    ///
    /// @param buf A reference to a Buffer.
    ///
    /// @return A boolean true if the Buffers are indentical.
    bool operator==(Buffer &buf);

    /// \brief Get the byte at a specified location.
    ///
    /// @param index The location as a numerical value of the byte to
    ///		get.
    ///
    /// @return The byte at the specified location.
    gnash::Network::byte_t operator[](int index) { return _data[index]; };

    /// \brief Get the byte at a specified location.
    ///
    /// @param index The location as a numerical value of the byte to
    ///		get.
    ///
    /// @return A real pointer to the byte at the specified location.
    gnash::Network::byte_t *at(int index) { return _data.get() + index; };

    /// \brief How much room is left in the buffer past the seek pointer.
    ///		This is primarily used to see if the buffer is fully
    ///		populated with data before appending more.
    ///
    /// @return The amoount of unused bytes in the Buffer.
    size_t spaceLeft() { return (_nbytes - (_seekptr - _data.get())); };
    
    ///  \brief Dump the internal data of this class in a human readable form.
    ///		This should only be used for debugging purposes.
    void dump() const { dump(std::cerr); }
    /// \overload dump(std::ostream& os) const
    void dump(std::ostream& os) const;
    
  protected:
    /// \var _seekptr
    ///	\brief This is a pointer to the address in the Buffer to
    ///		write data to then next time some is appended.
    gnash::Network::byte_t *_seekptr;
    
    /// \var _data
    ///	\brief This is the container of the actual data in this
    ///		Buffer.
    boost::scoped_array<gnash::Network::byte_t> _data;
    /// \var _nbytes
    ///	\brief This is the total allocated size of the Buffer.
    size_t         _nbytes;
    /// \var _stamp
    ///	\brief This is used when collecting performance statistics of
    ///		the low level functioning of the Buffer, and should
    ///		only be used for testing and debugging purposes.
#ifdef USE_STATS_BUFFERS
    struct timespec _stamp;	// used for timing how long data stays in the queue.
#endif
    
  private:
    /// \brief Initialize a block of memory for this buffer.
    ///		This should only be used internally by the Buffer
    ///		class.
    ///
    /// @param nbytes The total size to allocate memory for.
    ///
    /// @return A reference to the initialized Buffer.
    Buffer &init(size_t nbytes);
    
    /// \brief Convert a Hex digit into it's decimal value.
    ///
    /// @param digit The digit as a hex value
    ///
    /// @return The byte as a decimal value.
    gnash::Network::byte_t hex2digit (gnash::Network::byte_t digit);
};

/// \brief Dump to the specified output stream.
inline std::ostream& operator << (std::ostream& os, const Buffer& buf)
{
	buf.dump(os);
	return os;
}

} // end of amf namespace

#endif // end of __BUFFER_H__

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
