// tu_file.cpp	-- Ignacio Castaño, Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A file class that can be customized with callbacks.

#include "tu_file.h"

#include <boost/format.hpp>
#include <cerrno>
#include <cstdio>

#include "GnashFileUtilities.h"
#include "log.h"

namespace gnash {

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
    if (static_cast<size_t>(pos) > size()) return false;

    std::clearerr(_data); // make sure EOF flag is cleared.
    const int result = std::fseek(_data, pos, SEEK_SET);
    if (result == EOF) {
        return false;
    }

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
	    log_error("Could not fstat file");
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

} // end namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
