// tu_file.cpp	-- Ignacio Castaño, Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A file class that can be customized with callbacks.

#include "tu_file.h"

#include <cstdio>
#include <boost/format.hpp>
#include <cerrno>

#include "GnashFileUtilities.h"
#include "utility.h"
#include "IOChannel.h" 
#include "log.h"

namespace gnash {

/// An IOChannel that works on a C stdio file.
class tu_file : public IOChannel
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
    std::uint32_t read_le32()
    {
	    // read_byte() is std::uint8_t, so no masks with 0xff are required.
	    std::uint32_t result = static_cast<std::uint32_t>(read_byte());
	    result |= static_cast<std::uint32_t>(read_byte()) << 8;
	    result |= static_cast<std::uint32_t>(read_byte()) << 16;
	    result |= static_cast<std::uint32_t>(read_byte()) << 24;
	    return(result);
    }
	
    /// \brief Read a 16-bit word from a little-endian stream.
    //
    /// TODO: define what happens when the stream
    ///       is in error condition, see bad().
    ///
    std::uint16_t read_le16()
    {
	    std::uint16_t result = static_cast<std::uint16_t>(read_byte());
	    result |= static_cast<std::uint16_t>(read_byte()) << 8;
	    return(result);
    }
    
    /// \brief Read a single byte from the stream
    //
    /// TODO: define what happens when the stream
    ///       is in error condition, see bad().
    ///
    std::uint8_t read_byte() {
        std::uint8_t u;
        read(&u, 1);
        return u;
    }
    
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
    
    void close();
    
    FILE* _data;

    bool _autoclose;

};


//// Create a file from a standard file pointer.
tu_file::tu_file(FILE* fp, bool autoclose = false)
    :
    _data(fp),
    _autoclose(autoclose)
{
}

tu_file::~tu_file()
{
    // Close this file when destroyed unless not requested.
    if (_autoclose) close();
}


// Return the number of bytes actually read.  EOF or an error would
// cause that to not be equal to "bytes".
std::streamsize
tu_file::read(void* dst, std::streamsize bytes) 
{
    assert(dst);
    return std::fread(dst, 1, bytes, _data);
}

// Return the number of bytes actually written.
std::streamsize
tu_file::write(const void* src, std::streamsize bytes)
{
    assert(src);
    return std::fwrite(src, 1, bytes, _data);
}

bool
tu_file::seek(std::streampos pos)
{
    // TODO: optimize this by caching total stream size ?
    if (pos > static_cast<std::streampos>(size())) return false;

    std::clearerr(_data); // make sure EOF flag is cleared.
    const int result = std::fseek(_data, pos, SEEK_SET);
    if (result == EOF) {
        return false;
    }
    assert (pos < std::numeric_limits<long>::max());
    assert (std::ftell(_data) == pos);

    return true;
}

void
tu_file::go_to_end()
{
    const int err = std::fseek(_data, 0, SEEK_END);
    if (err == -1) {
        boost::format fmt = boost::format(
                _("Error while seeking to end: %1%")) % strerror(errno);
        throw IOException(fmt.str());
    }
}

std::streampos
tu_file::tell() const
{
    std::streampos ret = std::ftell(_data);
    if (ret < 0) throw IOException("Error getting stream position");

    assert(static_cast<size_t>(ret) <= size());
    return ret;
}

bool
tu_file::eof() const
{
    return std::feof(_data);
}

bool
tu_file::bad() const
{
    if (!_data) return true;
    return std::ferror(_data);
}

size_t
tu_file::size() const
{
    assert(_data);

    struct stat statbuf;
    if (fstat(fileno(_data), &statbuf) < 0)
    {
	log_error(_("Could not fstat file"));
	return static_cast<size_t>(-1);
    }
    return statbuf.st_size;
}


void
tu_file::close()
{
    assert(_data);
    std::fclose(_data);
}

std::unique_ptr<IOChannel>
makeFileChannel(FILE* fp, bool close)
{
    std::unique_ptr<IOChannel> ret(new tu_file(fp, close));
    return ret;
}

std::unique_ptr<IOChannel>
makeFileChannel(const char* filepath, const char* mode)
{
	FILE* fp = fopen(filepath, mode);
	if ( fp == 0 ) { return std::unique_ptr<IOChannel>(); }

	return makeFileChannel(fp, true);
}


} // end namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
