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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "noseek_fd_adapter.h"
#include "IOChannel.h" // for inheritance
#include "utility.h"
#include "log.h"

#include "GnashSystemIOHeaders.h" // for ::read

#include <boost/scoped_array.hpp>

//#define GNASH_NOSEEK_FD_VERBOSE 1

// define this if you want seeks back to be reported (on stderr)
//#define GNASH_NOSEEK_FD_WARN_SEEKSBACK 1

#include <cerrno>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>

#include <string>
#include <boost/format.hpp>

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
	///	A filedescriptor for a stream opened for reading
	///
	/// @param filename
	///	An optional filename to use for storing the cache.
	///	If NULL the cache would be an unnamed file and
	///	would not be accessible after destruction of this 
	///	instance.
	///
	NoSeekFile(int fd, const char* filename=NULL);

	~NoSeekFile();

	// See IOChannel.h for description
	virtual int read(void *dst, int bytes);

	// See IOChannel for description
	virtual bool eof() const;

	// See IOChannel for description
	virtual int get_error() const { return 0; }

	// See IOChannel for description
	virtual int tell() const;

	// See IOChannel for description
	virtual int seek(int pos);

	// See IOChannel for description
	virtual void go_to_end() {
		throw IOException("noseek_fd_adapter doesn't support seek to end");
	}

private:

	/// Read buffer size
	static const unsigned int chunkSize = 512;

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
	long unsigned _cached;

	// Current read buffer
	char _buf[chunkSize];

	// Attempt at filling the cache up to the given size.
	void fill_cache(size_t size);

	// Append sz bytes to the cache
	size_t cache(void *from, size_t sz);

	void printInfo();

};

/***********************************************************************
 *
 *  NoSeekFile implementation
 * 
 **********************************************************************/


/*private*/
size_t
NoSeekFile::cache(void *from, size_t sz)
{

#ifdef GNASH_NOSEEK_FD_VERBOSE
	std::cerr << boost::format("cache(%p, %d) called") % from % sz << std::endl;
#endif
	// take note of current position
	long curr_pos = ftell(_cache);

#ifdef GNASH_NOSEEK_FD_VERBOSE
	std::cerr << boost::format(" current position: %ld)") % curr_pos << std::endl;
#endif

	// seek to the end
	fseek(_cache, 0, SEEK_END);

#ifdef GNASH_NOSEEK_FD_VERBOSE
	std::cerr << boost::format(" after SEEK_END, position: %ld") % ftell(_cache) << std::endl;
#endif

	size_t wrote = fwrite(from, 1, sz, _cache);
#ifdef GNASH_NOSEEK_FD_VERBOSE
    std::cerr << boost::format(" write %d bytes") % wrote;
#endif
	if ( wrote < 1 )
	{
        boost::format err = boost::format("writing to cache file: requested %d, wrote %d (%s)")
            % sz % wrote % std::strerror(errno);
			
		std::cerr << err << std::endl;
		throw IOException(err.str());
	}

	_cached += sz;

#ifdef GNASH_NOSEEK_FD_VERBOSE
    std::cerr << boost::format(" after write, position: %ld") % std::ftell(_cache) << std::endl;
#endif

	// reset position for next read
	fseek(_cache, curr_pos, SEEK_SET);

#ifdef GNASH_NOSEEK_FD_VERBOSE
    std::cerr << boost::format(" after seek-back, position: %ld") % std::ftell(_cache) << std::endl;
#endif

	clearerr(_cache);

	return wrote;
}


/*private*/
void
NoSeekFile::fill_cache(size_t size)
{
#ifdef GNASH_NOSEEK_FD_VERBOSE
    std::cerr << boost::format(" fill_cache(%d) called") % size << std::endl;
#endif

	// See how big is the cache
	while ( _cached < size ) 
	{

		// Let's see how many bytes are left to read
		unsigned int bytesNeeded = size-_cached;
		if ( bytesNeeded > chunkSize ) bytesNeeded = chunkSize;

		bytesNeeded = chunkSize; // why read less ?

#ifdef GNASH_NOSEEK_FD_VERBOSE
	std::cerr << boost::format(" bytes needed = %d") % bytesNeeded << std::endl;
#endif

		ssize_t bytesRead = ::read(_fd, (void*)_buf, bytesNeeded);
		if ( bytesRead < 0 )
		{
			std::cerr << boost::format(_("Error reading %d bytes from input stream")) % bytesNeeded;
			_running = false;
			// this looks like a CRITICAL error (since we don't handle it..)
			throw IOException("Error reading from input stream");
			return;
		}

		if ( static_cast<size_t>(bytesRead) < bytesNeeded )
		{
			if ( bytesRead == 0 )
			{
#ifdef GNASH_NOSEEK_FD_VERBOSE
    std::cerr << "EOF reached" << std::endl;
#endif
				_running = false;
				return;
			}
		}

		cache(_buf, static_cast<size_t>(bytesRead));

	}
}

/*private*/
void
NoSeekFile::printInfo()
{
	std::cerr << "_cache.tell = " << tell() << std::endl;
}

/*private*/
void
NoSeekFile::openCacheFile()
{
	if ( _cachefilename )
	{
		_cache = fopen(_cachefilename, "w+b");
		if ( ! _cache )
		{
			throw IOException("Could not create cache file " + std::string(_cachefilename));
		}
	}
	else
	{
		_cache = tmpfile();
		if ( ! _cache ) {
			throw IOException("Could not create temporary cache file");
		}
	}

}

/*public*/
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

/*public*/
NoSeekFile::~NoSeekFile()
{
	fclose(_cache);
}

/*public*/
int
NoSeekFile::read(void *dst, int bytes)
{
#ifdef GNASH_NOSEEK_FD_VERBOSE
	std::cerr << boost::format("read_cache(%d) called") % bytes << std::endl;
#endif

	if ( eof() )
	{
#ifdef GNASH_NOSEEK_FD_VERBOSE
	    std::cerr << "read_cache: at eof!" << std::endl;
#endif
		return 0;
	}


	fill_cache(tell()+bytes);

#ifdef GNASH_NOSEEK_FD_VERBOSE
	printInfo();
#endif

	size_t ret = fread(dst, 1, bytes, _cache);

	if ( ret == 0 )
	{
		if ( ferror(_cache) )
		{
            std::cerr << "an error occurred while reading from cache" << std::endl;
		}
#if GNASH_NOSEEK_FD_VERBOSE
		if ( feof(_cache) )
		{
            std::cerr << "EOF reached while reading from cache" << std::endl;
		}
#endif
	}

#ifdef GNASH_NOSEEK_FD_VERBOSE
    std::cerr << boost::format("fread from _cache returned %d") % ret << std::endl;
#endif

	return ret;

}

/*public*/
bool
NoSeekFile::eof() const
{
	bool ret = ( ! _running && feof(_cache) );
	
#ifdef GNASH_NOSEEK_FD_VERBOSE
	std::cerr << boost::format("eof() returning %d") % ret << std::endl;
#endif
	return ret;

}

/*public*/
int
NoSeekFile::tell() const
{
	int ret = std::ftell(_cache);

#ifdef GNASH_NOSEEK_FD_VERBOSE
	std::cerr << boost::format("tell() returning %ld") % ret << std::endl;
#endif

	return ret;

}

/*public*/
int
NoSeekFile::seek(int pos)
{
#ifdef GNASH_NOSEEK_FD_WARN_SEEKSBACK
	if ( pos < tell() ) {
		std::cerr << boost::format("Warning: seek backward requested "
		                "(%ld from %ld)") % pos % tell() << std::endl;
	}
#endif

	fill_cache(pos);

	if ( std::fseek(_cache, pos, SEEK_SET) == -1 ) {
		std::cerr << "Warning: fseek failed" << std::endl;
		return -1;
	} else {
		return 0;
	}

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

	NoSeekFile* stream = NULL;

	try {
		stream = new NoSeekFile(fd, cachefilename);
	} catch (const std::exception& ex) {
		std::cerr << boost::format("NoSeekFile stream: %s") % ex.what() << std::endl;
		delete stream;
		return NULL;
	}

	return stream;
}

} // namespace gnash::noseek_fd_adapter
} // namespace gnash



// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
