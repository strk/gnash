// stream.cpp:  Network streaming server for Cygnal, for Gnash.
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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <cerrno>
#include <sys/mman.h>
#include <unistd.h>

#include "network.h"
#include "buffer.h"
#include "log.h"
#include "cque.h"
#include "diskstream.h"

#include <boost/thread/mutex.hpp>
static boost::mutex io_mutex;

using namespace gnash;
using namespace std;

// namespace {
// gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
// }

namespace cygnal {

DiskStream::DiskStream()
    : _bytes(0),
      _filefd(0),
      _netfd(0),
      _filesize(0),
      _chunksize(0)
{
//    GNASH_REPORT_FUNCTION;
}

DiskStream::DiskStream(const string &str)
    : _bytes(0),
      _filefd(0),
      _netfd(0),
      _filesize(0),
      _chunksize(0)
{
//    GNASH_REPORT_FUNCTION;
    _filespec = str;
}

DiskStream::DiskStream(const string &str, int netfd)
    : _bytes(0),
      _filefd(0),
      _filespec(0),
      _filesize(0),
      _chunksize(0)
{
//    GNASH_REPORT_FUNCTION;
    _netfd = netfd;
    _filespec = str;
}

#if 0

// Load a chunk of the file into memory
size_t
DiskStream::loadChunk(size_t size)
{
//    GNASH_REPORT_FUNCTION;

#ifdef HAVE_SYSCONF
    long pageSize = sysconf(_SC_PAGESIZE);
    if (size % pageSize) {
        size += pageSize - size % pageSize;
//      log_debug("Adjusting segment size to %d to be page aligned.\n", _size);
    }
#endif

#if 1
    
    if (_filefd) {
	_dataptr = static_cast<unsigned char *>(mmap(0, _chunksize,
						     PROT_READ, MAP_SHARED, _filefd, 0));
    } else {
	log_error (_("Couldn't load file %s"), filespec);
	return false;
    }
    
    if (_seekptr == MAP_FAILED) {
	log_error (_("Couldn't map file %s into memory: %s"),
		   filespec, strerror(errno));
	return false;
    } else {	    
	log_debug (_("File %s mapped to: %p"), filespec,
		   (void *)_dataptr);
	_seekptr = _dataptr;
	_state = OPEN;
	return true;
    }
#else
    do {
	boost::shared_ptr<amf::Buffer> buf(new amf::Buffer);
	ret = read(filefd, buf->reference(), buf->size());
	if (ret == 0) { // the file is done
	    break;
	}
	if (ret != buf->size()) {
	    buf->resize(ret);
	    log_debug("Got last data block from disk file, size %d", buf->size());
	}
	log_debug("Read %d bytes from %s.", ret, filespec);
	hand->pushout(buf);
	hand->notifyout();
    } while(ret > 0);
    log_debug("Done transferring %s to net fd #%d",
	      filespec, args->netfd);
    ::close(filefd); // close the disk file
    
#endif
}

DiskStream::~DiskStream() {
//    GNASH_REPORT_FUNCTION;
    if (_filefd) {
        close(_filefd);
    }
    if (_netfd) {
	::close(_netfd);
    }
}

bool
DiskStream::open(const string &filespec) {
//    GNASH_REPORT_FUNCTION;
    
    return open(filespec, _netfd);
}

bool
DiskStream::open(const string &filespec, int /*netfd*/)
{
//    GNASH_REPORT_FUNCTION;

    //struct stat stats;
    // TODO: should we use the 'netfd' passed as parameter instead ?
    return open(filespec, _netfd, _statistics);
}

bool
DiskStream::open(const string &filespec, int netfd, Statistics &statistics) {
    GNASH_REPORT_FUNCTION;

    struct stat st;
    
    _netfd = netfd;
    _statistics = statistics;

    log_debug("Trying to open %s", filespec);

    if (stat(filespec.c_str(), &st) == 0) {
	_filesize = st.st_size;
	boost::mutex::scoped_lock lock(io_mutex);
	_filefd = ::open(filespec.c_str(), O_RDONLY);
	log_debug (_("File %s is %lld bytes in size."), filespec,
		 (long long int) _filesize);
    } else {
	log_error (_("File %s doesn't exist"), filespec);
    }
    
    return true;
}

// Stream the file
bool
DiskStream::play() {
//    GNASH_REPORT_FUNCTION;

    return play(_netfd);
}

bool
DiskStream::play(int netfd) {
//    GNASH_REPORT_FUNCTION;

    _netfd = netfd;
    _state = PLAY;

    while (_state != DONE) {
        switch (_state) {
          case PLAY:
              _state = DONE;
              break;
          case PREVIEW:
              break;
          case THUMBNAIL:
              break;
          case PAUSE:
              break;
          case SEEK:
              break;
          case UPLOAD:
              break;
          case MULTICAST:
              break;
          case DONE:
              break;
          default:
              break;
        }
    }

    //int blocksize = 8192;
    int nbytes = 0;
    Network net;
//    while ((_seekptr - _dataptr) >= 0) {
////    nbytes = net.writeNet(_netfd, (char *)_seekptr, _filesize);
//    if (nbytes <= 0) {
//        break;
//    }
#ifdef USE_STATS_FILE
    _statistics->addBytes(nbytes);
    _bytes += nbytes;
    _seekptr += nbytes;
#endif
    
    log_debug("Done...");
	   
    munmap(_dataptr, _filesize);
    _seekptr = 0;

    return true;
}

// Stream a preview, instead of the full movie.
bool
DiskStream::preview(const string & /*filespec*/, int /*frames*/) {
//    GNASH_REPORT_FUNCTION;

    _state = PREVIEW;
    return true; // Default to true    
}

// Stream a series of thumbnails
bool
DiskStream::thumbnail(const string & /*filespec*/, int /*quantity*/) {
//    GNASH_REPORT_FUNCTION;
    
    _state = THUMBNAIL;
    return true; // Default to true
}

// Pause the stream
bool
DiskStream::pause(int /*frame*/) {
//    GNASH_REPORT_FUNCTION;
    
    _state = PAUSE;
    return true; // Default to true
}

// Seek within the stream
bool
DiskStream::seek(int /*frame*/) {
//    GNASH_REPORT_FUNCTION;
    
    _state = SEEK;
    return true; // Default to true    
}

// Upload a stream into a sandbox
bool
DiskStream::upload(const string & /*filespec*/) {
//    GNASH_REPORT_FUNCTION;
    
    _state = UPLOAD;
    return true; // Default to true
}

// Stream a single "real-time" source.
bool DiskStream::multicast(const string & /*filespec*/) {
//    GNASH_REPORT_FUNCTION;
    
    _state = MULTICAST;
    return true; // Default to true    
}

#endif

} // end of cygnal namespace


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
