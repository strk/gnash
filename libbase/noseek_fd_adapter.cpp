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

#include <unistd.h> // for ::read
#include <cstring>

#include <boost/scoped_array.hpp>

//#define GNASH_NOSEEK_FD_VERBOSE 1

// define this if you want seeks back to be reported (on stderr)
//#define GNASH_NOSEEK_FD_WARN_SEEKSBACK 1


#include <stdexcept>
#include <cstdio>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>

#if !defined(_WIN32) && !defined(WIN32)
# include <unistd.h>
#endif

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
	fprintf(stderr, "cache(%p, " SIZET_FMT ") called\n", from, sz);
#endif
	// take note of current position
	long curr_pos = ftell(_cache);

#ifdef GNASH_NOSEEK_FD_VERBOSE
	fprintf(stderr, " current position: %ld\n", curr_pos);
#endif

	// seek to the end
	fseek(_cache, 0, SEEK_END);

#ifdef GNASH_NOSEEK_FD_VERBOSE
	fprintf(stderr, " after SEEK_END, position: %ld\n", ftell(_cache));
#endif

	size_t wrote = fwrite(from, 1, sz, _cache);
#ifdef GNASH_NOSEEK_FD_VERBOSE
	fprintf(stderr, " write " SIZET_FMT " bytes\n", wrote);
#endif
	if ( wrote < 1 )
	{
		char errmsg[256];
	
		snprintf(errmsg, 255,
			"writing to cache file: requested " SIZET_FMT ", wrote " SIZET_FMT " (%s)",
			sz, wrote, strerror(errno));
		fprintf(stderr, "%s\n", errmsg);
		throw IOException(errmsg);
	}

	_cached += sz;

#ifdef GNASH_NOSEEK_FD_VERBOSE
	fprintf(stderr, " after write, position: %ld\n", ftell(_cache));
#endif

	// reset position for next read
	fseek(_cache, curr_pos, SEEK_SET);

#ifdef GNASH_NOSEEK_FD_VERBOSE
	fprintf(stderr, " after seek-back, position: %ld\n", ftell(_cache));
#endif

	clearerr(_cache);

	return wrote;
}


/*private*/
void
NoSeekFile::fill_cache(size_t size)
{
#ifdef GNASH_NOSEEK_FD_VERBOSE
	fprintf(stderr, "fill_cache(%d) called\n", size);
#endif

	// See how big is the cache
	while ( _cached < size ) 
	{

		// Let's see how many bytes are left to read
		unsigned int bytesNeeded = size-_cached;
		if ( bytesNeeded > chunkSize ) bytesNeeded = chunkSize;

		bytesNeeded = chunkSize; // why read less ?

#ifdef GNASH_NOSEEK_FD_VERBOSE
		fprintf(stderr, " bytes needed = " SIZET_FMT "\n", bytesNeeded);
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
				fprintf(stderr, "EOF reached\n");
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
	fprintf(stderr, "_cache.tell = " SIZET_FMT "\n", tell());
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
	fprintf(stderr, "read_cache(%d) called\n", bytes);
#endif

	if ( eof() )
	{
#ifdef GNASH_NOSEEK_FD_VERBOSE
		fprintf(stderr, "read_cache: at eof!\n");
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
	fprintf(stderr, "an error occurred while reading from cache\n");
		}
#if GNASH_NOSEEK_FD_VERBOSE
		if ( feof(_cache) )
		{
	fprintf(stderr, "EOF reached while reading from cache\n");
		}
#endif
	}

#ifdef GNASH_NOSEEK_FD_VERBOSE
	fprintf(stderr, "fread from _cache returned " SIZET_FMT "\n", ret);
#endif

	return ret;

}

/*public*/
bool
NoSeekFile::eof() const
{
	bool ret = ( ! _running && feof(_cache) );
	
#ifdef GNASH_NOSEEK_FD_VERBOSE
	fprintf(stderr, "eof() returning %d\n", ret);
#endif
	return ret;

}

/*public*/
int
NoSeekFile::tell() const
{
	int ret =  ftell(_cache);

#ifdef GNASH_NOSEEK_FD_VERBOSE
	fprintf(stderr, "tell() returning %ld\n", ret);
#endif

	return ret;

}

/*public*/
int
NoSeekFile::seek(int pos)
{
#ifdef GNASH_NOSEEK_FD_WARN_SEEKSBACK
	if ( pos < tell() ) {
		fprintf(stderr,
			"Warning: seek backward requested (%ld from %ld)\n",
			pos, tell());
	}
#endif

	fill_cache(pos);

	if ( fseek(_cache, pos, SEEK_SET) == -1 ) {
		fprintf(stderr, "Warning: fseek failed\n");
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
	fprintf(stderr, "making NoSeekFile stream for fd %d\n", fd);
#endif

	NoSeekFile* stream = NULL;

	try {
		stream = new NoSeekFile(fd, cachefilename);
	} catch (const std::exception& ex) {
		fprintf(stderr, "NoSeekFile stream: %s\n", ex.what());
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
