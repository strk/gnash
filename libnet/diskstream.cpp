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
#include "amf.h"
#include "log.h"
#include "cque.h"
#include "diskstream.h"
#include "cache.h"
#include "getclocktime.hpp"

#include <boost/thread/mutex.hpp>
static boost::mutex io_mutex;

using namespace gnash;
using namespace std;
using namespace amf;

/// \namespace gnash
///	This is the main namespace for Gnash and it's libraries.
namespace gnash {

static Cache& cache = Cache::getDefaultInstance();

/// \def _SC_PAGESIZE
///	This isn't set on all systems, but is used to get the page
///	size used for memory allocations.
#ifndef _SC_PAGESIZE
#define _SC_PAGESIZE 8
#endif

DiskStream::DiskStream()
    : _filefd(0),
      _netfd(0),
      _dataptr(0),
      _filesize(0),
      _pagesize(0),
      _offset(0)
{
//    GNASH_REPORT_FUNCTION;
#ifdef USE_STATS_CACHE
    clock_gettime (CLOCK_REALTIME, &_last_access);
    _accesses = 1;
#endif
}

DiskStream::DiskStream(const string &str)
    : _filefd(0),
      _netfd(0),
      _dataptr(0),
      _filesize(0),
      _pagesize(0),
      _offset(0)
{
//    GNASH_REPORT_FUNCTION;
    _filespec = str;
#ifdef USE_STATS_CACHE
    clock_gettime (CLOCK_REALTIME, &_last_access);
    _accesses = 1;
#endif
}

DiskStream::DiskStream(const string &str, int netfd)
    : _filefd(0),
      _filespec(0),
      _dataptr(0),
      _filesize(0),
      _pagesize(0),
      _offset(0)
{
//    GNASH_REPORT_FUNCTION;
    _netfd = netfd;
    _filespec = str;
#ifdef USE_STATS_CACHE
    clock_gettime (CLOCK_REALTIME, &_last_access);
    _accesses = 1;
#endif
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

/// \brief Load a chunk (pagesize) of the file into memory.
///	This loads a pagesize of the disk file into memory. We read
///	the file this way as it is faster and takes less resources
///	than read(), which add buffering we don't need.
///	This offset must be a multipe of the pagesize.
///
/// @param size The amount of bytes to read, often the filesize
///		for smaller files below CACHE_LIMIT.
///
/// @param offset The location in bytes in the file of the desired data.
///
/// @return A real pointer to the location of the data at the
///	location pointed to by the offset.
boost::uint8_t *
DiskStream::loadChunk(off_t offset)
{
    return loadChunk(_pagesize, offset);
}

boost::uint8_t *
DiskStream::loadChunk(size_t size, off_t offset)
{
//    GNASH_REPORT_FUNCTION;

    log_debug("%s: offset is: %d", __FUNCTION__, offset);
    /// If the data pointer is left from a failed mmap, don't do
    /// anything.
    if (_dataptr ==  MAP_FAILED) {
	log_error("Bad pointer to memory for file %s!", _filespec);
	return 0;
    }

#if 0
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
 	    log_debug("Adjusting offset from %d to %d so it's page aligned.",
 		      offset, _offset);
	}
	log_debug("Offset is page aligned already");
    }
#endif
    
    if (_filefd) {
	/// If the data pointer is legit, then we need to unmap that page
	/// to mmap() a new one. If we're still in the current mapped
	/// page, then just return the existing data pointer.
	if (_dataptr != 0) {
	    munmap(_dataptr, _pagesize);
	    _dataptr = 0;
	}

#if 0
	// See if the page has alady been mapped in;
	unsigned char vec[_pagesize];
	mincore(offset, _pagesize, vec);
	if (vec[i] & 0x1) {
	    // cached page is in memory
	}
#endif
	if (size <= _pagesize) {
	    size = _pagesize;
	}
	_dataptr = static_cast<unsigned char *>(mmap(0, size,
						     PROT_READ, MAP_SHARED, _filefd, offset));
    } else {
	log_error (_("Couldn't load file %s"), _filespec);
	return 0;
    }
    
    if (_dataptr == MAP_FAILED) {
	log_error (_("Couldn't map file %s into memory: %s"),
		   _filespec, strerror(errno));
	return 0;
    } else {
	clock_gettime (CLOCK_REALTIME, &_last_access);
	log_debug (_("File %s a offset %d mapped to: %p"), _filespec, _offset, (void *)_dataptr);
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

    // the file is already open
    if (_state == OPEN) {
#ifdef USE_STATS_CACHE
	_accesses++;
#endif
	return true;
    }
    
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
    
#ifdef USE_STATS_CACHE
    clock_gettime (CLOCK_REALTIME, &_first_access);
#endif
    
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
bool
DiskStream::multicast(const string & /*filespec*/)
{
//    GNASH_REPORT_FUNCTION;
    
    _state = MULTICAST;
    log_unimpl("%s", __PRETTY_FUNCTION__);
    return true; // Default to true    
}

DiskStream::filetype_e
DiskStream::determineFileType()
{
//    GNASH_REPORT_FUNCTION;
    
  return(determineFileType(_filespec));
}

DiskStream::filetype_e 
DiskStream::determineFileType(const string &filespec)
{
//    GNASH_REPORT_FUNCTION;
    
  if (filespec.empty()) {
    return FILETYPE_NONE;
  }

  string::size_type pos;
  pos = filespec.rfind(".");
  if (pos != string::npos) {
    string suffix = filespec.substr(pos, filespec.size());
    if (suffix == "html") {
      _filetype = FILETYPE_HTML;
    }
    if (suffix == "ogg") {
      _filetype = FILETYPE_OGG;
    }
    if (suffix == "swf") {
      _filetype = FILETYPE_SWF;
    }
    if (suffix == "flv") {
      _filetype = FILETYPE_FLV;
    }
    if (suffix == "mp3") {
      _filetype = FILETYPE_MP3;
    }
    if (suffix == "flac") {
      _filetype = FILETYPE_FLAC;
    }
  }

  return _filetype;
}

DiskStream::filetype_e 
DiskStream::determineFileType( boost::uint8_t *data)
{
//    GNASH_REPORT_FUNCTION;

  if (data == 0) {
    return FILETYPE_NONE;
  }
 
  // JPEG, offset 6 bytes, read the string JFIF
  if (memcpy(data + 6, "JFIF", 4) == 0) {
    return FILETYPE_NONE;
  }
  // SWF, offset 0, read the string FWS
  if (memcpy(data, "SWF", 3) == 0) {
    return FILETYPE_SWF;
  }
  // compressed SWF, offset 0, read the string CWS
  // FLV, offset 0, read the string FLV
  // PNG, offset 0, read the string PNG
  if (memcpy(data, "PNG", 3) == 0) {
    return FILETYPE_PNG;
  }
  // Ogg, offset 0, read the string OggS
  if (memcpy(data, "OggS", 4) == 0) {
    return FILETYPE_OGG;
  }
  // Theora, offset 28, read string theora
  if (memcpy(data + 28, "theora", 6) == 0) {
    return FILETYPE_THEORA;
  }
  // FLAC, offset 28, read string FLAC
  if (memcpy(data + 28, "FLAC", 4) == 0) {
    return FILETYPE_FLAC;
  }
  // Vorbis, offset 28, read string vorbis
  if (memcpy(data + 28, "vorbis", 6) == 0) {
    return FILETYPE_VORBIS;
  }
  // MP3, offset 0, read string ID3
  if (memcpy(data, "ID3", 3) == 0) {
    return FILETYPE_MP3;
  }

  // HTML
  //   offset 0, read string "\<!doctype\ html"
  //   offset 0, read string "\<head"
  //   offset 0, read string "\<title"
  //   offset 0, read string "\<html"
  if (memcpy(data, "ID3", 3) == 0) {
    return FILETYPE_HTML;
  }

  // XML, offset 0, read string "\<?xml"
  if (memcpy(data, "<?xml", 5) == 0) {
    return FILETYPE_XML;
  }
  
  return FILETYPE_NONE;
}

///  \brief Dump the internal data of this class in a human readable form.
/// @remarks This should only be used for debugging purposes.
void
DiskStream::dump()
{
//    GNASH_REPORT_FUNCTION;
	//state_e     _state;
    const char *state_str[] = {
	"NO_STATE",
	"PLAY",
	"PREVIEW",
	"THUMBNAIL",
	"PAUSE",
	"SEEK",
	"UPLOAD",
	"MULTICAST",
	"DONE"
    };
    
    cerr << "State is \"" << state_str[_state] << "\"" << endl;
    cerr << "Filespec is \"" << _filespec << "\"" << endl;
    cerr << "Disk file descriptor is fd #" << _filefd << endl;
    cerr << "Network file descritor is fd #" << _netfd << endl;
    cerr << "File size is " <<  _filesize << endl;
    cerr << "Memory Page size is " << _pagesize << endl;
    cerr << "Memory Offset is " << _offset << endl;
    
    // dump timing related data
    struct timespec now;
    clock_gettime (CLOCK_REALTIME, &now);    
    double time = ((now.tv_sec - _last_access.tv_sec) + ((now.tv_nsec - _last_access.tv_nsec)/1e9));
    
    cerr << "Time since last access:  " << fixed << ((now.tv_sec - _last_access.tv_sec) + ((now.tv_nsec - _last_access.tv_nsec)/1e9)) << " seconds ago." << endl;
    
#ifdef USE_STATS_CACHE
    cerr << "Time since first access: " << fixed <<
	((_last_access.tv_sec - _first_access.tv_sec) + ((_last_access.tv_nsec - _first_access.tv_nsec)/1e9))
	 << " seconds lifespan."
	 << endl;
#endif
    
}


} // end of cygnal namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
