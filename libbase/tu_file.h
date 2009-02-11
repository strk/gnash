// tu_file.h	-- Ignacio Castaño, Thatcher Ulrich 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A very generic file class that can be customized with callbacks.


#ifndef TU_FILE_H
#define TU_FILE_H

#include "dsodefs.h" // DSOEXPORT
#include "utility.h"
#include "IOChannel.h" // for inheritance

#include <cstdio>

namespace gnash {

// a file abstraction that can be customized with callbacks.
// Designed to be easy to hook up to FILE*, SDL_RWops*, or
// whatever stream type(s) you might use in your game or
// libraries.
class DSOEXPORT tu_file : public gnash::IOChannel
{
public:

    // Make a file from an ordinary FILE*.
    tu_file(FILE* fp, bool autoclose);
    
    ~tu_file();
    
    /// \brief Read a 32-bit word from a little-endian stream.
    ///	returning it as a native-endian word.
    //
    /// TODO: define what happens when the stream
    ///       is in error condition.
    ///
    boost::uint32_t read_le32() 
    {
	    // read8() is boost::uint8_t, so no masks with 0xff are required.
	    boost::uint32_t result = static_cast<boost::uint32_t>(read8());
	    result |= static_cast<boost::uint32_t>(read8()) << 8;
	    result |= static_cast<boost::uint32_t>(read8()) << 16;
	    result |= static_cast<boost::uint32_t>(read8()) << 24;
	    return(result);
    }
	
	/// \brief Read a 64-bit word from a little-ending stream,
	/// returning it as a native-endian word.
	//
	/// TODO: define what happens when the stream is in
	///       error condition, see bad().
	/// TODO: define a platform-neutral type for 64 bits.
	long double read_le_double64()
	{
		return static_cast<long double> (
			static_cast<boost::int64_t> (read_le32()) |
			static_cast<boost::int64_t> (read_le32()) << 32
		);
	}

    /// \brief Read a 16-bit word from a little-endian stream.
    //
    /// TODO: define what happens when the stream
    ///       is in error condition, see bad().
    ///
    boost::uint16_t read_le16()
    {
	    boost::uint16_t result = static_cast<boost::uint16_t>(read8());
	    result |= static_cast<boost::uint16_t>(read8()) << 8;
	    return(result);
    }

    /// \brief Write a 32-bit word to a little-endian stream.
    //
    /// TODO: define what happens when the stream
    ///       is in error condition, see bad().
    ///
    void 	write_le32(boost::uint32_t u)
    {
        write8(static_cast<boost::int8_t>(u));
        write8(static_cast<boost::int8_t>(u>>8));
        write8(static_cast<boost::int8_t>(u>>16));
        write8(static_cast<boost::int8_t>(u>>24));
    }

    /// \brief Write a 16-bit word to a little-endian stream.
    //
    /// TODO: define what happens when the stream
    ///       is in error condition, see bad().
    ///
    void write_le16(boost::uint16_t u)
    {
        write8(static_cast<boost::int8_t>(u));
        write8(static_cast<boost::int8_t>(u>>8));
    }
    
    /// \brief Read a single byte from the stream
    //
    /// TODO: define what happens when the stream
    ///       is in error condition, see bad().
    ///
    boost::uint8_t read_byte() { return read8(); }

    /// \brief write a single byte to the stream
    //
    /// TODO: define what happens when the stream
    ///       is in error condition, see bad().
    ///
    void write_byte(boost::uint8_t u) { write8(u); }
    
    /// \brief Read the given number of bytes from the stream
    //
    /// TODO: define what happens when the stream
    ///       is in error condition, see bad().
    ///
    std::streamsize read(void* dst, std::streamsize num);

    /// \brief Write the given number of bytes to the stream
    //
    /// TODO: define what happens when the stream
    ///       is in error condition, see bad().
    ///
    std::streamsize write(const void* src, std::streamsize num);

    /// \brief Return current stream position
    //
    /// TODO: define what to return when the stream
    ///       is in error condition, see bad().
    ///
    std::streampos tell() const;

    /// \brief Seek to the specified position
    //
    /// 
    /// TODO: define what happens when an error occurs, or
    ///       when we're already in an error condition
    ///
    /// @return true on success, or false on failure.
    ///
    bool seek(std::streampos p);

    /// \brief Seek to the end of the stream
    //
    /// TODO: define what happens when an error occurs
    ///
    void go_to_end();

    /// \brief Return true if the end of the stream has been reached.
    //
    /// TODO: define what to return when in error condition
    /// see bad().
    ///
    bool eof() const;
    
    /// \brief Return non-zero if the stream is in an error state
    //
    /// When the stream is in an error state there's nothing
    /// you can do about it, just delete it and log the error.
    ///
    /// There are some rough meaning for possible returned values
    /// but I don't think they make much sense currently.
    ///
    bool bad() const;
    
    /// Get the size of the stream
    size_t size() const;
    
private:

    boost::uint64_t	read64()
    {
        boost::uint64_t u;
        read(&u, 8);
        return u;
    }

    boost::uint32_t	read32()
    {
        boost::uint32_t u;
        read(&u, 4);
        return u;
    }

    boost::uint16_t read16()
    {
        boost::uint16_t u;
        read(&u, 2);
        return u;
    }
    
    boost::uint8_t	read8()
    {
        boost::uint8_t u;
        read(&u, 1);
        return u;
    }
    
    void write64(boost::uint64_t u)
    {
        write(&u, 8);
    }

    void write32(boost::uint32_t u)
    {
        write(&u, 4);
    }

    void write16(boost::uint16_t u)
    {
        write(&u, 2);
    }

    void write8(boost::uint8_t u)
    {
        write(&u, 1);
    }
    
    void close();
    
    void *	m_data;

    bool _autoclose;

};


} // namespace gnash
#endif // TU_FILE_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
