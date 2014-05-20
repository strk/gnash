// 
//   Copyright (C) 2008, 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <cstdint>
#include <iostream>
#include <boost/random/uniform_int.hpp>
#include <boost/random/mersenne_twister.hpp>

#include "buffer.h"
#include "amf.h"
#include "log.h"
//#include "network.h"
#include "GnashException.h"


/// \namespace cygnal
///
/// This namespace is for all the AMF specific classes in libamf.
namespace cygnal
{

/// \brief Convert a Hex digit into it's decimal value.
///
/// @param digit The digit as a hex value
///
/// @return The byte as a decimal value.
std::uint8_t
Buffer::hex2digit (std::uint8_t digit)
{  
    if (digit == 0)
        return 0;
    
    if (digit >= '0' && digit <= '9')
        return digit - '0';
    if (digit >= 'a' && digit <= 'f')
        return digit - 'a' + 10;
    if (digit >= 'A' && digit <= 'F')
        return digit - 'A' + 10;
    
    // shouldn't ever get this far
    return -1;
}

/// \brief Encode a Buffer from a hex string.
///
/// @param str A hex string, ex... "00 03 05 0a"
///
/// @return A reference to a Buffer in host endian format. This is
///		primary used only for testing to create binary data
///		from an easy to read and edit format.
Buffer &
Buffer::hex2mem(const std::string &str)
{
//    GNASH_REPORT_FUNCTION;
    size_t count = str.size();
    size_t size = (count/3) + 4;
    std::uint8_t ch = 0;
    
    std::uint8_t *ptr = const_cast<std::uint8_t *>(reinterpret_cast<const std::uint8_t *>(str.c_str()));
    std::uint8_t *end = ptr + count;

    init(size);
    
    for (size_t i=0; ptr<end; i++) {
        if (*ptr == ' ') {      // skip spaces.
            ptr++;
            continue;
        }
        ch = hex2digit(*ptr++) << 4;
        ch |= hex2digit(*ptr++);
        *this += ch;
	    i++;
    }
    resize(size);
    
    return *this;
}

// Convert each byte into its hex representation
std::string
Buffer::hexify ()
{
    return gnash::hexify(_data.get(), allocated(), false);
}

std::string
Buffer::hexify (bool ascii)
{
    return gnash::hexify(_data.get(), allocated(), ascii);
}

std::string
Buffer::hexify (cygnal::Buffer &buf, bool ascii)
{
    return gnash::hexify(buf.reference(), buf.allocated(), ascii);
}

/// \brief Initialize a block of memory for this buffer.
///		This should only be used internally by the Buffer
///		class.
///
/// @param nbytes The total size to allocate memory for.
///
/// @return A reference to the initialized Buffer.
Buffer &
Buffer::init(size_t size)
{
//    GNASH_REPORT_FUNCTION;
    if (!_data) {
	_data.reset(new std::uint8_t[size]);
	_seekptr = _data.get();
    }
    _seekptr = _data.get();
    _nbytes = size;

    clear();// FIXME; this is a perforance hit, but aids in debugging
#ifdef USE_STATS_BUFFERS
    clock_gettime (CLOCK_REALTIME, &_stamp);
#endif
    
    return *this;
}

/// \brief Create a new Buffer with the default size
Buffer::Buffer() 
    : _seekptr(0)
{
//    GNASH_REPORT_FUNCTION;
    _nbytes = cygnal::NETBUFSIZE;
    init(cygnal::NETBUFSIZE);
}
    
/// \brief Create a new Buffer with a size other than the default
Buffer::Buffer(size_t nbytes)
    : _seekptr(0)
{
//    GNASH_REPORT_FUNCTION;
    _nbytes = nbytes;
    init(_nbytes);
}

/// \brief Create a new Buffer with a hex string.
///		This is primary used only for testing to create binary
///		data from an easy to read and edit format.
/// @param str A hex string, ex... "00 03 05 0a"
Buffer::Buffer(const std::string &str)
{
//    GNASH_REPORT_FUNCTION;
    hex2mem(str);
}

/// Delete the memory allocated for this Buffer
Buffer::~Buffer()
{
//    GNASH_REPORT_FUNCTION;
    if (_data) {
#ifdef USE_STATS_BUFFERS
	struct timespec now;
	clock_gettime (CLOCK_REALTIME, &now);
	log_debug(_("Buffer %x (%d) stayed in queue for %f seconds"),
		  (void *)_data.get(), _nbytes,
		  (float)((now.tv_sec - _stamp.tv_sec) + ((now.tv_nsec - _stamp.tv_nsec)/1e9)));
#endif
        _seekptr = 0;
        _nbytes = 0;
    }
}

/// \brief Copy data into the buffer.
///		This overwrites all data, and resets the seek ptr.
///
/// @param data A pointer to the raw bytes to copy into the
///		buffer.
/// 
/// @param nbytes The number of bytes to copy.
///		
/// @return A reference to a Buffer.
Buffer &
Buffer::copy(std::uint8_t *data, size_t nbytes)
{    
//    GNASH_REPORT_FUNCTION;
    if (_data) {
	if (_nbytes >= nbytes) {
	    std::copy(data, data + nbytes, _data.get());
	    _seekptr = _data.get() + nbytes;
	} else {
	    boost::format msg("Not enough storage was allocated to hold the "
			      "copied data! Needs %1%, only has %2% bytes");
	    msg % nbytes % _nbytes;
	    throw gnash::GnashException(msg.str());
	}
    }
    return *this;
}

/// \brief Append data to existing data in the buffer.
///
/// @param data A pointer to the raw bytes to append to the
///		buffer.
/// 
/// @param nbytes The number of bytes to append.
///		
/// @return A reference to a Buffer.
Buffer &
Buffer::append(std::uint8_t *data, size_t nbytes)
{
//    GNASH_REPORT_FUNCTION;
    if (_data) {
	if (spaceLeft() >= nbytes) {
	    std::copy(data, data + nbytes, _seekptr);
	    _seekptr += nbytes;
	} else {
	    boost::format msg("Not enough storage was allocated to hold the "
			      "appended data! Needs %1%, only has %2% bytes");
	    msg % nbytes % spaceLeft();
	    throw gnash::GnashException(msg.str());
	}
    }

    return *this;
}

/// \brief Append a Buffer class to existing data in the buffer.
///
/// @param buf A Buffer class containing the data to append.
/// 
/// @return A reference to a Buffer.
Buffer &
Buffer::operator+=(Buffer &buf)
{
// //    GNASH_REPORT_FUNCTION;
    append(buf.reference(), buf.allocated());
    return *this;
}

/// \brief Copy a AMF0 type into the buffer.
///		This overwrites all data, and resets the seek ptr.
///
/// @param type An AMF0 type. 
/// 
/// @return A reference to a Buffer.
Buffer &
Buffer::operator+=(cygnal::Element::amf0_type_e type)
{
//    GNASH_REPORT_FUNCTION;
    std::uint8_t nb = static_cast<std::uint8_t>(type);
    
    return operator+=(nb);
}

/// \brief Append a byte to existing data in the buffer.
///
/// @param byte A single byte.
/// 
/// @return A reference to a Buffer.
Buffer &
Buffer::operator+=(char byte)
{
//    GNASH_REPORT_FUNCTION;
    std::uint8_t nb = static_cast<std::uint8_t>(byte);
    return operator+=(nb);
}

/// \brief Append a boolean to existing data in the buffer.
///
/// @param type A boolean. 
/// 
/// @return A reference to a Buffer.
Buffer &
Buffer::operator+=(bool flag)
{
//    GNASH_REPORT_FUNCTION;
    std::uint8_t nb = static_cast<std::uint8_t>(flag);
    return operator+=(nb);
}

/// \brief Append a byte to existing data in the buffer.
///
/// @param byte A single byte.
/// 
/// @return A reference to a Buffer.
Buffer &
Buffer::operator+=(std::uint8_t byte)
{
//    GNASH_REPORT_FUNCTION;
    if ((_seekptr + 1) <= (_data.get() + _nbytes)) {
	*_seekptr = byte;
	_seekptr += sizeof(std::uint8_t);
    }
    return *this;
}

/// \brief Append a string to existing data in the buffer.
///
/// @param str A string containing ASCII data to copy into the
///		buffer.
/// 
/// @return A reference to a Buffer.
Buffer &
Buffer::operator+=(const char *str)
{
//    GNASH_REPORT_FUNCTION;
    std::uint8_t *ptr = const_cast<std::uint8_t *>(reinterpret_cast<const std::uint8_t *>(str));
    return append(ptr, strlen(str));
    
}

/// \brief Append a string to existing data in the buffer.
///
/// @param str A string containing ASCII data to copy into the
///		buffer.
/// 
/// @return A reference to a Buffer.
Buffer &
Buffer::operator+=(const std::string &str)
{
//    GNASH_REPORT_FUNCTION;
    std::uint8_t *ptr = const_cast<std::uint8_t *>(reinterpret_cast<const std::uint8_t *>(str.c_str()));
    return append(ptr, str.size());
    
}

/// \brief Append a double to existing data in the buffer.
///
/// @param num A numeric double value.
/// 
/// @return A reference to a Buffer.
Buffer &
Buffer::operator+=(double num)
{
//    GNASH_REPORT_FUNCTION;
    std::uint8_t *ptr = reinterpret_cast<std::uint8_t *>(&num);
    return append(ptr, AMF0_NUMBER_SIZE);
}

/// \brief Append a short to existing data in the buffer.
/// 
/// @param num A numeric short value.
/// 
/// @return A reference to a Buffer.
Buffer &
Buffer::operator+=(std::uint16_t length)
{
//    GNASH_REPORT_FUNCTION;
    std::uint8_t *ptr = reinterpret_cast<std::uint8_t *>(&length);
    return append(ptr, sizeof(std::uint16_t));
}

/// \brief Append an integer to existing data in the buffer.
/// 
/// @param num A numeric integer value.
/// 
/// @return A reference to a Buffer.
Buffer &
Buffer::operator+=(std::uint32_t length)
{
//    GNASH_REPORT_FUNCTION;
    std::uint8_t *ptr = reinterpret_cast<std::uint8_t *>(&length);
    return append(ptr, sizeof(std::uint32_t));
}

/// \brief Append a Buffer class to existing data in the buffer.
///
/// @param buf A Buffer class containing the data to append.
/// 
/// @return A reference to a Buffer.
Buffer &
Buffer::operator+=(std::shared_ptr<Buffer> buf)
{
//    GNASH_REPORT_FUNCTION;
    append(buf->reference(), buf->allocated());
    return *this;
}

/// \brief Append a Buffer class to existing data in the buffer.
///
/// @param buf A Buffer class containing the data to append.
/// 
/// @return A reference to a Buffer.
Buffer &
Buffer::operator=(Buffer &buf)
{
//    GNASH_REPORT_FUNCTION;
    if (buf.size() != _nbytes) {
	resize(buf.size());
    }
    copy(buf.reference(), buf.size());

    return *this;
}

/// \brief Copy a string into the buffer.
///		This overwrites all data, and resets the seek ptr.
///
/// @param str A string containing ASCII data to copy into the
///		buffer.
/// 
/// @return A reference to a Buffer.
Buffer &
Buffer::operator=(const std::string &str)
{
//    GNASH_REPORT_FUNCTION;
    std::uint8_t *ptr = const_cast<std::uint8_t *>(reinterpret_cast<const std::uint8_t *>(str.c_str()));
    return copy(ptr, str.size());
}

Buffer &
Buffer::operator=(const char *str)
{
//    GNASH_REPORT_FUNCTION;
    std::uint8_t *ptr = const_cast<std::uint8_t *>(reinterpret_cast<const std::uint8_t *>(str));
    return copy(ptr, strlen(str));
}

/// \brief Copy a double into the buffer.
///		This overwrites all data, and resets the seek ptr.
///
/// @param num A numeric double value.
/// 
/// @return A reference to a Buffer.
Buffer &
Buffer::operator=(double num)
{
//    GNASH_REPORT_FUNCTION;
    std::uint8_t *ptr = reinterpret_cast<std::uint8_t *>(&num);
    return copy(ptr, AMF0_NUMBER_SIZE);
}

/// \brief Copy a short into the buffer.
///		This overwrites all data, and resets the seek ptr.
///
/// @param num A numeric short value.
/// 
/// @return A reference to a Buffer.
Buffer &
Buffer::operator=(std::uint16_t length)
{
//    GNASH_REPORT_FUNCTION;
    std::uint8_t *ptr = reinterpret_cast<std::uint8_t *>(&length);
    return copy(ptr, sizeof(std::uint16_t));
}

/// \brief Copy a AMF0 type into the buffer.
///		This overwrites all data, and resets the seek ptr.
///
/// @param type An AMF0 type. 
/// 
/// @return A reference to a Buffer.
Buffer &
Buffer::operator=(cygnal::Element::amf0_type_e type)
{
    std::uint8_t nb = static_cast<std::uint8_t>(type);
    return operator=(nb);
}

/// Copy a boolean into the buffer. This overwrites all data, and
///		resets the seek ptr.
///
/// @param flag A boolean. 
/// 
/// @return A reference to a Buffer.
Buffer &
Buffer::operator=(bool flag)
{
    std::uint8_t nb = static_cast<std::uint8_t>(flag);
    return operator=(nb);
}

/// \brief Copy a byte into the buffer.
///		This overwrites all data, and resets the seek ptr.
///
/// @param byte A single byte.
/// 
/// @return A reference to a Buffer.
Buffer &
Buffer::operator=(std::uint8_t byte)
{
//    GNASH__FUNCTION;
   
    return copy(&byte, 1);
}

/// \brief Copy a byte into the buffer.
///		This overwrites all data, and resets the seek ptr.
///
/// @param byte A pointer to a single byte.
/// 
/// @return A reference to a Buffer.
Buffer &
Buffer::operator=(std::uint8_t *data)
{
//    GNASH_REPORT_FUNCTION;
    if (data) {
	_data.reset(data);
    } else {
	throw gnash::ParserException("Passing invalid pointer!");
    }
    return *this;
}

/// \brief Copy a Buffer class into the buffer.
///		This overwrites all data, and resets the seek ptr.
///
/// @param buf A Buffer class containing the data to copy.
/// 
/// @return A reference to a Buffer.
Buffer &
Buffer::operator=(std::shared_ptr<Buffer> buf)
{
//    GNASH_REPORT_FUNCTION;
    copy(buf->reference(), buf->size());
    return *this;
}

/// \brief Test equivalance against another Buffer.
///		This compares all the data on the current Buffer with
///		the supplied one, so it can be a performance hit. This
///		is primarily only used for testing purposes.
///
/// @param buf A reference to a Buffer.
///
/// @return A boolean true if the Buffers are indentical.
bool
Buffer::operator==(Buffer &buf)
{
//    GNASH_REPORT_FUNCTION;
     if (buf.size() == _nbytes){
         if (memcmp(buf.reference(), _data.get(), _nbytes) == 0) {
             return true;
         }
     }
     return false;
}

/// \brief Drop a byte without resizing.
///		This will remove the byte from the Buffer, and then
///		move the remaining data to be in the correct
///		location. This resets the seek pointer.
///
/// @param byte The byte to remove from the buffer.
///
/// @return A real pointer to the base address of the buffer.
std::uint8_t *
Buffer::remove(std::uint8_t c)
{
//    GNASH_REPORT_FUNCTION;
    std::uint8_t *start = std::find(begin(), end(), c);

//    log_debug("Byte is at %x", (void *)start);
    
    if (start == 0) {
	return 0;
    }
    
    std::copy(start + 1, end(), start);
    *(end() - 1) = 0;
    _seekptr--;

    return _data.get();
}

/// \brief Drop a byte without resizing.
///		This will remove the byte from the Buffer, and then
///		move the remaining data to be in the correct
///		location. This resets the seek pointer.
///
/// @param start The location of the byte to remove from the
///		Buffer
///
/// @return A real pointer to the base address of the Buffer.
std::uint8_t *
Buffer::remove(int start)
{
//    GNASH_REPORT_FUNCTION;
    std::copy((_data.get() + start + 1), end(), (_data.get() + start)),
//    *end() = 0;
    _seekptr--;
    return _data.get();
}

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
/// @param range The amount of bytes to remove from the Buffer.
///
/// @return A real pointer to the base address of the Buffer.
std::uint8_t *
Buffer::remove(int start, int range)
{
//    GNASH_REPORT_FUNCTION;
    std::copy((_data.get() + range + 1), end(), (_data.get() + start)),
//    *end() = 0;
	_seekptr -= range;
    
    return _data.get();
}

/// \brief Clear the contents of the buffer by setting all the bytes to
///		zeros.
///
/// @return nothing
void
Buffer::clear()
{
//    GNASH_REPORT_FUNCTION;
    if (_data) {
        memset(_data.get(), 0, _nbytes);
    }
    _seekptr = _data.get();
}

/// \brief Resize the buffer that holds the data.
///		The new size of the current data is based on the
///		current amount of data within the allocated memory.
///		This is used to make a Buffer the same size as
///		the existing data, and to truncate the unsed portion
///		of the Buffer when copying to the new memory
///		location.
/// 
/// @return A reference to a Buffer.
Buffer &
Buffer::resize()
{
//    GNASH_REPORT_FUNCTION;
    return resize(_seekptr - _data.get());
}

/// \brief Resize the buffer that holds the data.
///		If the size is larger than the existing data, the data
///		is copied into the new region. If the size is smaller
///		than the existing data, the remaining data is
///		truncated when copying to the new memory location.
///
/// @param nbyte The size to resize the Buffer to.
///
/// @return A reference to a Buffer.
Buffer &
Buffer::resize(size_t size)
{
//    GNASH_REPORT_FUNCTION;
    std::unique_ptr<std::uint8_t[]> tmp;

    // If there is no size, don't do anything
    if (size == 0) {
	return *this;
    }
    
    // If we don't have any data yet in this buffer, resizing is cheap, as
    // we don't havce to copy any data.
    if (_seekptr == _data.get()) {
	_data.reset(new std::uint8_t[size]);
	_nbytes= size;
	return *this;
    }
    
    if (_nbytes == 0) {
	return init(size);
    } else {
	// Don't bother to resize without really changing anything
	if (size == _nbytes) {
	    return *this;
	}

	// check the sizes. If we had data read using ->reference(), the seekptr isn't
	// increased, so in these cases we just copy al lthe data blindly, as it's
	// better than loosing data.
	size_t used = 0;
	if (_seekptr != _data.get()) {
	    used = _seekptr - _data.get();
	} else {
	    if (size < _nbytes) {
		used = size;
	    } else {
		used = _nbytes;
	    }
	}
	
	
	// Copy the existing data into the new block of memory. The data
	// held currently is moved to the temporary array, and then gets
	// deleted when this method returns.
	// We loose data if we resize smaller than the data currently held.
	if (size < used) {
	    gnash::log_error(_("cygnal::Buffer::resize(%d): Truncating data (%d bytes) while resizing!"), size, used - size);
	    used = size;
	}
	std::uint8_t *newptr = new std::uint8_t[size];
	std::copy(_data.get(), _data.get() + used, newptr);
	_data.reset(newptr);
	
	// Make the seekptr point into the new space with the correct offset
	_seekptr = _data.get() + used;

	// Adjust the size
	_nbytes = size;
    }
    
    return *this;
}

///  \brief Dump the internal data of this class in a human readable form.
///		This should only be used for debugging purposes.
void
Buffer::dump(std::ostream& os) const
{
    os << "Buffer is " << _seekptr-_data.get() << "/" << _nbytes << " bytes: ";
     // Skip in-memory address " at " << (void *)_data.get() << endl;
    if (_nbytes > 0) {
	const size_t bytes = _seekptr - _data.get();
	os << gnash::hexify((unsigned char *)_data.get(), bytes, false)
       << std::endl;
	os << gnash::hexify((unsigned char *)_data.get(), bytes, true)
	   << std::endl;
    } else {
	os << "ERROR: Buffer size out of range!" << std::endl;
    }
}

/// \brief Corrupt a buffer with random errors.
///		This is used only for testing to make sure we can cleanly
///		handle corruption of the packets.
///
/// @param factor A divisor to adjust how many errors are created.
///
/// @return nothing
int
Buffer::corrupt()
{
    return corrupt(10);
}

int
Buffer::corrupt(int factor)
{
    boost::mt19937 seed;
    // Pick the number of errors to create based on the Buffer's data size
    boost::uniform_int<> errs(1, (_nbytes/factor));
    int errors = errs(seed);
    gnash::log_debug(_("Creating %d errors in the buffer"), errors);
    
    for (int i=0; i<errors; i++) {
	// find a location someplace within the file.
	boost::uniform_int<> location(0, _nbytes);
	int pos = location(seed);
	
//	log_debug("Creating error at %d in the buffer", pos);
	// Create a random new value for the byte
	boost::uniform_int<> shift(1, 256);
	int newval = shift(seed);
	// stomp the old value for our new one.
	_data[pos] = newval;
    }

    return errors;
}

} // end of amf namespace

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
