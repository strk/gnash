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

/// \namespace gnash
///	This is the main namespace for Gnash and it's libraries.
namespace gnash {

/// \def _SC_PAGESIZE
///	This isn't set on all systems, but is used to get the page
///	size used for memory allocations.
#ifndef _SC_PAGESIZE
#define _SC_PAGESIZE 8
#endif

DiskStream::DiskStream()
    : _bytes(0),
      _filefd(0),
      _netfd(0),
      _dataptr(0),
      _filesize(0),
      _pagesize(0),
      _offset(0)
{
//    GNASH_REPORT_FUNCTION;
}

DiskStream::DiskStream(const string &str)
    : _bytes(0),
      _filefd(0),
      _netfd(0),
      _dataptr(0),
      _filesize(0),
      _pagesize(0),
      _offset(0)
{
//    GNASH_REPORT_FUNCTION;
    _filespec = str;
}

DiskStream::DiskStream(const string &str, int netfd)
    : _bytes(0),
      _filefd(0),
      _filespec(0),
      _dataptr(0),
      _filesize(0),
      _pagesize(0),
      _offset(0)
{
//    GNASH_REPORT_FUNCTION;
    _netfd = netfd;
    _filespec = str;
}

DiskStream::~DiskStream()
{
//    GNASH_REPORT_FUNCTION;
    if (_filefd) {
        ::close(_filefd);
    }
    if (_netfd) {
	::close(_netfd);
    }
}

/// \brief Close the open disk file and it's associated stream.
void
DiskStream::close()
{
//    GNASH_REPORT_FUNCTION;
    _filesize = 0;
    _offset = 0;
    if ((_dataptr != MAP_FAILED) && (_dataptr != 0)) {
	munmap(_dataptr, _pagesize);
    }
    _dataptr = 0;
    _filespec.clear();
}

///  \brief Dump the internal data of this class in a human readable form.
/// @remarks This should only be used for debugging purposes.
void
DiskStream::dump()
{
//    GNASH_REPORT_FUNCTION;
    //state_e     _state;
    cerr << "Bytes read is is " << _bytes << endl;
    cerr << "Disk file descriptor is " << _filefd << endl;
    cerr << "Network file descritor is " << _netfd << endl;
    cerr << "Filespec is " << _filespec << endl;
//    gnash::Statistics  _statistics;
//     unsigned char *_dataptr;
//     unsigned char *_seekptr;
    cerr << "File size is " <<  _filesize << endl;
    cerr << "Memory Page size is " << _pagesize << endl;

    _que.dump();
}

/// \brief Load a chunk (pagesize) of the file into memory.
///	This loads a pagesize of the disk file into memory. We read
///	the file this way as it is faster and takes less resources
///	than read(), which add buffering we don't need.
///	This offset must be a multipe of the pagesize.
///
/// @param size The location in bytes in the file of the desired data.
///
/// @return A real pointer to the location of the data at the
///	location pointed to by the offset.
boost::uint8_t *
DiskStream::loadChunk(off_t offset)
{
//    GNASH_REPORT_FUNCTION;

    /// If the data pointer is left from a failed mmap, don't do
    /// anything.
    if (_dataptr ==  MAP_FAILED) {
	log_error("Bad pointer to memory for file %s!", _filespec);
	return 0;
    }

    /// We only map pages of pagesize, so if the offset is smaller
    /// than that, don't use it.
    if (static_cast<size_t>(offset) < _pagesize) {
	_offset = 0;
//	log_debug("Loading first segment");
    } else {
	if (offset % _pagesize) {
	    // calculate the number of pages
	    int pages = ((offset - (offset % _pagesize)) / _pagesize);
	    _offset = pages * _pagesize;
// 	    log_debug("Adjusting offset from %d to %d so it's page aligned.",
// 		      offset, _offset);
	}
    }
    
    if (_filefd) {
    /// If the data pointer is legit, then we need to unmap that page
    /// to mmap() a new one. If we're still in the current mapped
    /// page, then just return the existing data pointer.
    if (_dataptr != 0) {
	// If the offset is less than what we already mmapped, we
	boost::uint32_t diff = *reinterpret_cast<boost::uint32_t *>(_dataptr + _offset);
	if (diff < _pagesize) {
	    return _dataptr + _offset;
	    // unmap the old data before allocating a new chunk
	} else {
	    munmap(_dataptr, _pagesize);
	    _dataptr = 0;
	}
    }

	_dataptr = static_cast<unsigned char *>(mmap(0, _pagesize,
					PROT_READ, MAP_SHARED, _filefd, _offset));
	offset = (offset % _pagesize);
    } else {
	log_error (_("Couldn't load file %s"), _filespec);
	return 0;
    }
    
    if (_dataptr == MAP_FAILED) {
	log_error (_("Couldn't map file %s into memory: %s"),
		   _filespec, strerror(errno));
	return 0;
    } else {
//	log_debug (_("File %s mapped to: %p"), _filespec, (void *)_dataptr);
	_seekptr = _dataptr + offset;
	_state = OPEN;
    }
    
    return _seekptr;
#if 0
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

/// \brief Open a file to be streamed.
///
/// @param filespec The full path and file name for the data to be
///	read.
///
/// @return True if the file was opened sucessfully, false if not.
bool
DiskStream::open(const string &filespec)
{
//    GNASH_REPORT_FUNCTION;
    
    return open(filespec, _netfd);
}

/// \brief Open a file to be streamed.
///
/// @param filespec The full path and file name for the data to be
///	read.
///
/// @param netfd An optional file descriptor to read data from
///
/// @return True if the file was opened sucessfully, false if not.
bool
DiskStream::open(const string &filespec, int /*netfd*/)
{
//    GNASH_REPORT_FUNCTION;

    //struct stat stats;
    // TODO: should we use the 'netfd' passed as parameter instead ?
    return open(filespec, _netfd, _statistics);
}

/// \brief Open a file to be streamed.
///
/// @param filespec The full path and file name for the data to be
///	read.
///
/// @param netfd An optional file descriptor to read data from
///
/// @param statistics The optional data structure to use for
///	collecting statistics on this stream.
///
/// @return True if the file was opened sucessfully, false if not.
bool
DiskStream::open(const string &filespec, int netfd, Statistics &statistics)
{
//    GNASH_REPORT_FUNCTION;

    struct stat st;
    
    _netfd = netfd;
    _statistics = statistics;
    _filespec = filespec;

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

    /// \brief get the pagesize and cache the value
#ifdef HAVE_SYSCONF
    _pagesize = sysconf(_SC_PAGESIZE);
#else
#error "Need to define the memory page size without sysconf()!"
#endif

//     // The pagesize is how much of the file to load. As all memory is
//     // only mapped in multiples of pages, we use that for the default size.
//     if (_pagesize == 0) {
// 	_pagesize = pageSize;
//     }
    
    return true;
}

/// \brief Stream the file that has been loaded,
///
/// @return True if the data was streamed sucessfully, false if not.
bool
DiskStream::play()
{
//    GNASH_REPORT_FUNCTION;

    return play(_netfd);
}

/// \brief Stream the file that has been loaded,
///
/// @param netfd An optional file descriptor to read data from
///
/// @return True if the data was streamed sucessfully, false if not.
bool
DiskStream::play(int netfd)
{
//    GNASH_REPORT_FUNCTION;

    _netfd = netfd;
    _state = PLAY;

    log_unimpl("%s", __PRETTY_FUNCTION__);
    
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
//    Network net;
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

/// \brief Stream a preview of the file.
///	A preview is a series of video frames from
///	the video file. Each video frame is taken by sampling
///	the file at a set interval.
///
/// @param filespec The full path and file name for the data to be
///	read.
///
/// @param quantity The number of frames to stream..
///
/// @return True if the thumbnails were streamed sucessfully, false if not.
bool
DiskStream::preview(const string & /*filespec*/, int /*frames*/)
{
//    GNASH_REPORT_FUNCTION;

    _state = PREVIEW;
    log_unimpl("%s", __PRETTY_FUNCTION__);
    return true; // Default to true    
}

/// \brief Stream a series of thumbnails.
///	A thumbnail is a series of jpg images of frames from
///	the video file instead of video frames. Each thumbnail
///	is taken by sampling the file at a set interval.
///
/// @param filespec The full path and file name for the data to be
///	read.
///
/// @param quantity The number of thumbnails to stream..
///
/// @return True if the thumbnails were streamed sucessfully, false if not.
bool
DiskStream::thumbnail(const string & /*filespec*/, int /*quantity*/)
{
//    GNASH_REPORT_FUNCTION;
    
    _state = THUMBNAIL;
    log_unimpl("%s", __PRETTY_FUNCTION__);
    return true; // Default to true
}

/// \brief Pause the stream currently being played.
///
/// @return True if the stream was paused sucessfully, false if not.
bool
DiskStream::pause()
{
//    GNASH_REPORT_FUNCTION;
    
    _state = PAUSE;
    log_unimpl("%s", __PRETTY_FUNCTION__);
    return true; // Default to true
}

/// \brief Seek within the stream.
///
/// @param the offset in bytes to the location within the file to
///	seek to.
///
/// @return A real pointer to the location of the data seeked to.
boost::uint8_t *
DiskStream::seek(off_t offset)
{
//    GNASH_REPORT_FUNCTION;
    
    _state = SEEK;
    return loadChunk(offset);    
}

/// \brief Upload a file into a sandbox.
///	The sandbox is an area where uploaded files can get
///	written to safely. For SWF content, the file name also
///	includes a few optional paths used to seperate
///	applications from each other.
///
/// @param filespec The file name for the data to be written.
///
/// @return True if the file was uploaded sucessfully, false if not.
bool
DiskStream::upload(const string & /*filespec*/)
{
//    GNASH_REPORT_FUNCTION;
    
    _state = UPLOAD;
    log_unimpl("%s", __PRETTY_FUNCTION__);
    return true; // Default to true
}

// Stream a single "real-time" source.
bool DiskStream::multicast(const string & /*filespec*/)
{
//    GNASH_REPORT_FUNCTION;
    
    _state = MULTICAST;
    log_unimpl("%s", __PRETTY_FUNCTION__);
    return true; // Default to true    
}
void
DiskStream::dump(std::ostream& os) const
{
    
//    GNASH_REPORT_FUNCTION;
}


} // end of cygnal namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
