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

#include "noseek_fd_adapter.h"

#include <cerrno>
#include <cstdio>
#include <string>
#include <boost/format.hpp>
#include <iostream>

#include "IOChannel.h" // for inheritance
#include "GnashSystemIOHeaders.h" // for read
#include "utility.h"
#include "log.h"

//#define GNASH_NOSEEK_FD_VERBOSE 1

// define this if you want seeks back to be reported (on stderr)
//#define GNASH_NOSEEK_FD_WARN_SEEKSBACK 1


namespace gnash {
namespace noseek_fd_adapter {

/***********************************************************************
 *
 *  NoSeekFile definition
 *
 *  TODO: cleanup this class, it makes too many seeks
 * 
 **********************************************************************/

class NoSeekFile : public IOChannel
{

public:

    /// Open a stream for the specified filedes
    //
    /// @param fd
    ///    A filedescriptor for a stream opened for reading
    ///
    /// @param filename
    ///    An optional filename to use for storing the cache.
    ///    If NULL the cache would be an unnamed file and
    ///    would not be accessible after destruction of this 
    ///    instance.
    ///
    NoSeekFile(int fd, const char* filename=nullptr);

    ~NoSeekFile();

    // See IOChannel.h for description
    virtual std::streamsize read(void *dst, std::streamsize bytes);

    // See IOChannel for description
    virtual bool eof() const;

    // See IOChannel for description
    virtual bool bad() const { return false; }

    // See IOChannel for description
    virtual std::streampos tell() const;

    // See IOChannel for description
    virtual bool seek(std::streampos pos);

    // See IOChannel for description
    virtual void go_to_end() {
        throw IOException("noseek_fd_adapter doesn't support seek to end");
    }

private:

    /// Read buffer size
    static const std::streamsize chunkSize = 512;

    // Open either a temporary file or a named file
    // (depending on value of _cachefilename)
    void openCacheFile();

    // Use this file to cache data
    FILE* _cache;

    // the input file descriptor
    int _fd;

    // transfer in progress
    int _running;

    // cache filename
    const char* _cachefilename;

    // Current size of cached data
    size_t _cached;

    // Current read buffer
    char _buf[chunkSize];

    // Attempt at filling the cache up to the given size.
    void fill_cache(std::streamsize size);

    // Append sz bytes to the cache
    std::streamsize cache(void *from, std::streamsize sz);

    void printInfo();

};


const std::streamsize NoSeekFile::chunkSize;

NoSeekFile::NoSeekFile(int fd, const char* filename)
    :
    _fd(fd),
    _running(1),
    _cachefilename(filename),
    _cached(0)
{
    // might throw an exception
    openCacheFile();
}

NoSeekFile::~NoSeekFile()
{
    std::fclose(_cache);
}

std::streamsize
NoSeekFile::cache(void* from, std::streamsize sz)
{

#ifdef GNASH_NOSEEK_FD_VERBOSE
    std::cerr << boost::format("cache(%p, %d) called") % from % sz << std::endl;
#endif
    // take note of current position
    std::streampos curr_pos = std::ftell(_cache);

#ifdef GNASH_NOSEEK_FD_VERBOSE
    std::cerr << boost::format(" current position: %ld)") % curr_pos << std::endl;
#endif

    // seek to the end
    std::fseek(_cache, 0, SEEK_END);

#ifdef GNASH_NOSEEK_FD_VERBOSE
    std::cerr << boost::format(" after SEEK_END, position: %ld") %
        std::ftell(_cache) << std::endl;
#endif

    std::streamsize wrote = std::fwrite(from, 1, sz, _cache);
#ifdef GNASH_NOSEEK_FD_VERBOSE
    std::cerr << boost::format(" write %d bytes") % wrote;
#endif
    if (wrote < 1) {
        boost::format err = boost::format("writing to cache file: "
                "requested %d, wrote %d (%s)")
            % sz % wrote % std::strerror(errno);
            
        std::cerr << err << std::endl;
        throw IOException(err.str());
    }

    _cached += sz;

#ifdef GNASH_NOSEEK_FD_VERBOSE
    std::cerr << boost::format(" after write, position: %ld") % 
        std::ftell(_cache) << std::endl;
#endif

    // reset position for next read
    std::fseek(_cache, curr_pos, SEEK_SET);

#ifdef GNASH_NOSEEK_FD_VERBOSE
    std::cerr << boost::format(" after seek-back, position: %ld") % 
        std::ftell(_cache) << std::endl;
#endif

    std::clearerr(_cache);

    return wrote;
}


void
NoSeekFile::fill_cache(std::streamsize size)
{
#ifdef GNASH_NOSEEK_FD_VERBOSE
    std::cerr << boost::format(" fill_cache(%d) called") % size << std::endl;
#endif

    assert(size >= 0);

    // See how big is the cache
    while (_cached < static_cast<size_t>(size)) {

#ifdef GNASH_NOSEEK_FD_VERBOSE
        size_t bytesNeeded = size - _cached;
        bytesNeeded = std::min<size_t>(bytesNeeded, chunkSize);
        std::cerr << boost::format(" bytes needed = %d") % bytesNeeded <<
            std::endl;
#endif

        std::streamsize bytesRead = ::read(_fd, _buf, chunkSize);
        if (bytesRead < 0) {
            std::cerr << boost::format(_("Error reading %d bytes from "
                        "input stream")) % chunkSize << std::endl;
            _running = false;
            // this looks like a CRITICAL error (since we don't handle it..)
            throw IOException("Error reading from input stream");
        }

        if (bytesRead < chunkSize) {
            if (bytesRead == 0) {
#ifdef GNASH_NOSEEK_FD_VERBOSE
                std::cerr << "EOF reached" << std::endl;
#endif
                _running = false;
                return;
            }
        }
        cache(_buf, bytesRead);
    }
}

void
NoSeekFile::printInfo()
{
    std::cerr << "_cache.tell = " << tell() << std::endl;
}

void
NoSeekFile::openCacheFile()
{
    if (_cachefilename) {
        _cache = std::fopen(_cachefilename, "w+b");
        if (!_cache) {
            throw IOException("Could not create cache file " + 
                    std::string(_cachefilename));
        }
    }
    else {
        _cache = tmpfile();
        if (!_cache) {
            throw IOException("Could not create temporary cache file");
        }
    }

}

std::streamsize
NoSeekFile::read(void *dst, std::streamsize bytes)
{
#ifdef GNASH_NOSEEK_FD_VERBOSE
    std::cerr << boost::format("read_cache(%d) called") % bytes << std::endl;
#endif

    if (eof()) {
#ifdef GNASH_NOSEEK_FD_VERBOSE
        std::cerr << "read_cache: at eof!" << std::endl;
#endif
        return 0;
    }

    fill_cache(bytes + tell());

#ifdef GNASH_NOSEEK_FD_VERBOSE
    printInfo();
#endif

    std::streamsize ret = std::fread(dst, 1, bytes, _cache);

    if (ret == 0) {
        if (std::ferror(_cache)) {
            std::cerr << "an error occurred while reading from cache" <<
                std::endl;
        }
#if GNASH_NOSEEK_FD_VERBOSE
        if (std::feof(_cache)) {
            std::cerr << "EOF reached while reading from cache" << std::endl;
        }
#endif
    }

#ifdef GNASH_NOSEEK_FD_VERBOSE
    std::cerr << boost::format("fread from _cache returned %d") % ret <<
        std::endl;
#endif

    return ret;

}

bool
NoSeekFile::eof() const
{
    bool ret = (!_running && std::feof(_cache));
    
#ifdef GNASH_NOSEEK_FD_VERBOSE
    std::cerr << boost::format("eof() returning %d") % ret << std::endl;
#endif
    return ret;

}

std::streampos
NoSeekFile::tell() const
{
    std::streampos ret = std::ftell(_cache);

#ifdef GNASH_NOSEEK_FD_VERBOSE
    std::cerr << boost::format("tell() returning %ld") % ret << std::endl;
#endif

    return ret;

}

bool
NoSeekFile::seek(std::streampos pos)
{
#ifdef GNASH_NOSEEK_FD_WARN_SEEKSBACK
    if (pos < tell()) {
        std::cerr << boost::format("Warning: seek backward requested "
                        "(%ld from %ld)") % pos % tell() << std::endl;
    }
#endif

    fill_cache(pos);

    if (std::fseek(_cache, pos, SEEK_SET) == -1) {
        std::cerr << "Warning: fseek failed" << std::endl;
        return false;
    } 
    
    return true;

}

/***********************************************************************
 *
 * Adapter calls
 * 
 **********************************************************************/

// this is the only exported interface
IOChannel*
make_stream(int fd, const char* cachefilename)
{
#ifdef GNASH_NOSEEK_FD_VERBOSE
    std::cerr << boost::format("making NoSeekFile stream for fd %d") % fd << std::endl;
#endif

    NoSeekFile* stream = nullptr;

    try {
        stream = new NoSeekFile(fd, cachefilename);
    } 
    catch (const std::exception& ex) {
        std::cerr << boost::format("NoSeekFile stream: %s") % ex.what() << std::endl;
        delete stream;
        return nullptr;
    }

    return stream;
}

} // namespace gnash::noseek_fd_adapter
} // namespace gnash



// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
