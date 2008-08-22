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

// a file abstraction that can be customized with callbacks.
// Designed to be easy to hook up to FILE*, SDL_RWops*, or
// whatever stream type(s) you might use in your game or
// libraries.
class DSOEXPORT tu_file : public gnash::IOChannel
{
public:
    typedef int (* read_func)(void* dst, int bytes, void* appdata);
    typedef int (* write_func)(const void* src, int bytes, void* appdata);
    typedef int (* seek_func)(int pos, void* appdata);
    typedef int (* seek_to_end_func)(void* appdata);
    typedef int (* tell_func)(void* appdata);
    typedef bool (* get_eof_func)(void* appdata);
    typedef int (* get_err_func)(void* appdata);
    typedef long (* get_stream_size_func)(void* appdata);
    typedef int (* close_func)(void* appdata);
    
    // Make a file from an ordinary FILE*.
    tu_file(FILE* fp, bool autoclose);
    
    // Open a file using ordinary fopen().  Automatically closes the
    // file when we are destroyed.
    tu_file(const char* name, const char* mode);
   
    ~tu_file();
    
    /// Copy remaining contents of *in into *this.
    //
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    void copy_from(tu_file* in);
    
    /// Copy a fixed number of bytes from *in to *this.
    //
    /// Returns number of bytes copied.
    ///
    /// TODO: define what happens when either one of the streams
    ///       is in error condition, see get_error().
    ///
    int	copy_bytes(tu_file* in, int bytes);
    

    /// \brief Read a 32-bit word from a little-endian stream.
    ///	returning it as a native-endian word.
    //
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
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
	///       error condition, see get_error().
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
    ///       is in error condition, see get_error().
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
    ///       is in error condition, see get_error().
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
    ///       is in error condition, see get_error().
    ///
    void write_le16(boost::uint16_t u)
    {
        write8(static_cast<boost::int8_t>(u));
        write8(static_cast<boost::int8_t>(u>>8));
    }
    
    /// \brief Read a single byte from the stream
    //
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    boost::uint8_t read_byte() { return read8(); }

    /// \brief write a single byte to the stream
    //
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    void write_byte(boost::uint8_t u) { write8(u); }
    
    /// \brief Read the given number of bytes from the stream
    //
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    int read(void* dst, int num)
    {
        return m_read(dst, num, m_data);
    }

    /// \brief Write the given number of bytes to the stream
    //
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    int write(const void* src, int num)
    {
        return m_write(src, num, m_data);
    }

    /// \brief Return current stream position
    //
    /// TODO: define what to return when the stream
    ///       is in error condition, see get_error().
    ///
    int	tell() const { return m_tell(m_data); }

    /// \brief Seek to the specified position
    //
    /// 
    /// TODO: define what happens when an error occurs, or
    ///       when we're already in an error condition
    ///
    /// @return 0 on success, or TU_FILE_SEEK_ERROR on failure.
    ///
    int	seek(int p) { return m_seek(p, m_data); }

    /// \brief Seek to the end of the stream
    //
    /// TODO: define what happens when an error occurs
    ///
    void	go_to_end() { m_seek_to_end(m_data); }

    /// \brief Return true if the end of the stream has been reached.
    //
    /// TODO: define what to return when in error condition
    /// see get_error().
    ///
    bool eof() const { return m_get_eof(m_data); }
    
    /// \brief Return non-zero if the stream is in an error state
    //
    /// When the stream is in an error state there's nothing
    /// you can do about it, just delete it and log the error.
    ///
    /// There are some rough meaning for possible returned values
    /// but I don't think they make much sense currently.
    ///
    int	get_error() const { return m_get_err(m_data); }
    

    /// \brief Get the size of the stream
    int size() const { return m_get_stream_size(m_data); }
    
    // \brief UNSAFE back door, for testing only.
    void* get_app_data_DEBUG() { return m_data; }
    
    
private:

    boost::uint64_t	read64()
    {
        boost::uint64_t u;
        m_read(&u, 8, m_data);
        return u;
    }

    boost::uint32_t	read32()
    {
        boost::uint32_t u;
        m_read(&u, 4, m_data);
        return u;
    }

    boost::uint16_t read16()
    {
        boost::uint16_t u;
        m_read(&u, 2, m_data);
        return u;
    }
    
    boost::uint8_t	read8()
    {
        boost::uint8_t u;
        m_read(&u, 1, m_data);
        return u;
    }
    
    void write64(boost::uint64_t u)
    {
        m_write(&u, 8, m_data);
    }

    void write32(boost::uint32_t u)
    {
        m_write(&u, 4, m_data);
    }

    void write16(boost::uint16_t u)
    {
        m_write(&u, 2, m_data);
    }

    void write8(boost::uint8_t u)
    {
        m_write(&u, 1, m_data);
    }
    
    void close();
    
    void *		m_data;
    read_func		m_read;
    write_func		m_write;
    seek_func		m_seek;
    seek_to_end_func 	m_seek_to_end;
    tell_func		m_tell;
    get_eof_func	m_get_eof;
    get_err_func	m_get_err;
    get_stream_size_func	m_get_stream_size;
    close_func		m_close;
};

#endif // TU_FILE_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
