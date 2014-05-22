// stream.cpp:  Network streaming server for Cygnal, for Gnash.
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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <sys/types.h>
#include <cstdint>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <cerrno>
#include <algorithm>
#if !defined(_WIN32) && !defined(__amigaos4__)
#include <sys/mman.h>
#elif defined(__amigaos4__)
#include <proto/exec.h>
#include <cstdlib> //for malloc/free
#else
#include <windows.h>
#endif

#include "GnashSystemIOHeaders.h"
#include "network.h"
#include "buffer.h"
#include "amf.h"
#include "log.h"
#include "cque.h"
#include "diskstream.h"
#include "cache.h"
#include "getclocktime.hpp"

// This is Linux specific, but offers better I/O for sending
// files out a network connection.
#ifdef HAVE_SENDFILE
# include <sys/sendfile.h>
#endif

#include <mutex>
static std::mutex io_mutex;
static std::mutex mem_mutex;

using std::string;

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

/// \var MAX_PAGES
///	This is the maximum number of pages that we load into memory from a file.
const size_t MAX_PAGES = 2560;

#ifndef MAP_FAILED
#define MAP_FAILED 0
#endif

DiskStream::DiskStream()
    : _state(DiskStream::NO_STATE),
      _filefd(0),
      _netfd(0),
      _dataptr(nullptr),
      _max_memload(0),
      _filesize(0),
      _pagesize(0),
      _offset(0)
{
//    GNASH_REPORT_FUNCTION;
    /// \brief get the pagesize and cache the value
#ifdef HAVE_SYSCONF
    _pagesize = sysconf(_SC_PAGESIZE);
    _max_memload = _pagesize * MAX_PAGES;    
#else
#if _WIN32
    // The default page size for Win32 is 4k
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    _pagesize = si.dwPageSize;
#else
#ifdef __amigaos4__
	uint32 PageSize;

	IExec->GetCPUInfoTags(
               GCIT_ExecPageSize, &PageSize, 
               TAG_DONE);
    _pagesize = PageSize;
#else
#error "Need to define the memory page size without sysconf()!"
#endif
#endif
#endif
#ifdef USE_STATS_CACHE
    clock_gettime (CLOCK_REALTIME, &_last_access);
    _accesses = 1;
#endif
}

DiskStream::DiskStream(const string &str)
    : _state(DiskStream::NO_STATE),
      _filefd(0),
      _netfd(0),
      _dataptr(nullptr),
      _max_memload(0),
      _filesize(0),
      _pagesize(0),
      _offset(0)
{
//    GNASH_REPORT_FUNCTION;
    /// \brief get the pagesize and cache the value
#ifdef HAVE_SYSCONF
    _pagesize = sysconf(_SC_PAGESIZE);
    _max_memload = _pagesize * MAX_PAGES;    
#else
#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    _pagesize = si.dwPageSize;
#else
#ifdef __amigaos4__
	uint32 PageSize;

	IExec->GetCPUInfoTags(
               GCIT_ExecPageSize, &PageSize, 
               TAG_DONE);
    _pagesize = PageSize;
#else
#error "Need to define the memory page size without sysconf()!"
#endif
#endif
#endif

    _filespec = str;
#ifdef USE_STATS_CACHE
    clock_gettime (CLOCK_REALTIME, &_last_access);
    _accesses = 1;
#endif
}

DiskStream::DiskStream(const string &str, std::uint8_t *data, size_t size)
    : _state(DiskStream::NO_STATE),
      _filefd(0),
      _netfd(0),
      _dataptr(nullptr),
      _max_memload(0),
      _pagesize(0),
      _offset(0)
{
//    GNASH_REPORT_FUNCTION;
    
    /// \brief get the pagesize and cache the value
#ifdef HAVE_SYSCONF
    _pagesize = sysconf(_SC_PAGESIZE);
    _max_memload = _pagesize * MAX_PAGES;    
#else
#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    _pagesize = si.dwPageSize;
#else
#ifdef __amigaos4__
	uint32 PageSize;

	IExec->GetCPUInfoTags(
               GCIT_ExecPageSize, &PageSize, 
               TAG_DONE);
    _pagesize = PageSize;
#else
#error "Need to define the memory page size without sysconf()!"
#endif
#endif
#endif

    _dataptr = new std::uint8_t[size];
    // Note that this is a copy operation, which may effect performance. We do this for now
    // incase the top level pointer gets deleted. This should really be using
    // boost::scoped_array, but we don't want that complexity till this code stabalizes.
    std::copy(data, data + size, _dataptr);
    _filespec = str;
    _filesize = size;
    
#ifdef USE_STATS_CACHE
    clock_gettime (CLOCK_REALTIME, &_last_access);
    _accesses = 1;
#endif    
}

DiskStream::DiskStream(const string &str, cygnal::Buffer &buf)
    : _state(DiskStream::NO_STATE),
      _filefd(0),
      _netfd(0),
      _dataptr(nullptr),
      _max_memload(0),
      _pagesize(0),
      _offset(0)
{
//    GNASH_REPORT_FUNCTION;
    
    /// \brief get the pagesize and cache the value
#ifdef HAVE_SYSCONF
    _pagesize = sysconf(_SC_PAGESIZE);
    _max_memload = _pagesize * MAX_PAGES;    
#else
#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    _pagesize = si.dwPageSize;
#else
#ifdef __amigaos4__
    uint32 PageSize;
    
    IExec->GetCPUInfoTags(
	GCIT_ExecPageSize, &PageSize, 
	TAG_DONE);
    _pagesize = PageSize;
#else
#error "Need to define the memory page size without sysconf()!"
#endif
#endif
#endif

    _dataptr = new std::uint8_t[buf.size()];
    // Note that this is a copy operation, which may effect performance. We do this for now
    // incase the top level pointer gets deleted. This should really be using
    // boost::scoped_array, but we don't want that complexity till this code stabalizes.
    std::copy(buf.begin(), buf.end(), _dataptr);
    _filespec = str;
    _filesize = buf.size();
    
#ifdef USE_STATS_CACHE
    clock_gettime (CLOCK_REALTIME, &_last_access);
    _accesses = 1;
#endif
}

DiskStream::DiskStream(const string &str, int netfd)
    : _state(DiskStream::NO_STATE),
      _filefd(0),
      _filespec(nullptr),
      _dataptr(nullptr),
      _max_memload(0),
      _filesize(0),
      _pagesize(0),
      _offset(0)
{
//    GNASH_REPORT_FUNCTION;
    /// \brief get the pagesize and cache the value
#ifdef HAVE_SYSCONF
    _pagesize = sysconf(_SC_PAGESIZE);
    _max_memload = _pagesize * MAX_PAGES;    
#else
#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    _pagesize = si.dwPageSize;
#else
#ifdef __amigaos4__
	uint32 PageSize;

	IExec->GetCPUInfoTags(
               GCIT_ExecPageSize, &PageSize, 
               TAG_DONE);
    _pagesize = PageSize;
#else
#error "Need to define the memory page size without sysconf()!"
#endif
#endif
#endif

    _netfd = netfd;
    _filespec = str;
#ifdef USE_STATS_CACHE
    clock_gettime (CLOCK_REALTIME, &_last_access);
    _accesses = 1;
#endif
}

DiskStream::~DiskStream()
{
    GNASH_REPORT_FUNCTION;
    log_debug(_("Deleting %s on fd #%d"), _filespec, _filefd);

    if (_filefd) {
        ::close(_filefd);
    }
    if (_netfd) {
	::close(_netfd);
    }
}

/// \brief copy another DiskStream into ourselves, so they share data
///		in memory.
DiskStream &
DiskStream::operator=(DiskStream *stream)
{
    GNASH_REPORT_FUNCTION;
    
    _filespec = stream->getFilespec();
    _filetype = stream->getFileType();
    _filefd = stream->getFileFd();
    _netfd = stream->getNetFd();
    _dataptr = stream->get();
    _state = stream->getState();

    return *this;
}

bool
DiskStream::fullyPopulated()
{
    // GNASH_REPORT_FUNCTION;
    
    if ((_filesize < _max_memload) && (_dataptr != nullptr)) {
	return true;
    }
    return false;
}

/// \brief Close the open disk file, but stay resident in memory.
void
DiskStream::close()
{
    // GNASH_REPORT_FUNCTION;

    log_debug(_("Closing %s on fd #%d"), _filespec, _filefd);

    if (_filefd) {
        ::close(_filefd);
    }

    // reset everything in case we get reopened.
    _filefd = 0;
    _netfd = 0;
    _offset = 0;
    _seekptr = _dataptr + _pagesize;
    _state = CLOSED;

#if 0				// FIXME: don't unmap the memory for debugging
#ifdef _WIN32
    UnmapViewOfFile(_dataptr);
#elif defined(__amigaos4__)
	if (_dataptr) free(_dataptr);
#else
    if ((_dataptr != MAP_FAILED) && (_dataptr != 0)) {
	munmap(_dataptr, _pagesize);
    }
#endif
#endif
    
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
std::uint8_t *
DiskStream::loadToMem(off_t offset)
{
    // GNASH_REPORT_FUNCTION;

    return loadToMem(_filesize, offset);
}

std::uint8_t *
DiskStream::loadToMem(size_t filesize, off_t offset)
{
    GNASH_REPORT_FUNCTION;


    log_debug(_("%s: offset is: %d"), __FUNCTION__, offset);

    // store the offset we came in with so next time we know where to start
    _offset = offset;
	
    /// We only map memory in pages of pagesize, so if the offset is smaller
    /// than that, start at page 0.
    off_t page = 0;
    if (static_cast<size_t>(offset) < _pagesize) {
	page = 0;
    } else {
	if (offset % _pagesize) {
	    // calculate the number of pages
	    page = ((offset - (offset % _pagesize)) / _pagesize) * _pagesize;
 	    log_debug(_("Adjusting offset from %d to %d so it's page aligned."),
 		      offset, page);
	} else {
	    log_debug(_("Offset is page aligned already"));
	}
    }

    // Figure out the maximum number of bytes we can load into memory.
    size_t loadsize = 0;
    if (filesize < _max_memload) {
	log_debug(_("Loading entire file of %d bytes into memory segment"),
		    filesize);
	loadsize = filesize;
    } else {
	log_debug(_("Loading partial file of %d bytes into memory segment"),
		  filesize, _max_memload);
	loadsize = _max_memload;
    }
    
    // If we were initialized from a memory Buffer, data is being uploaded into
    // this DiskStream, so sufficient memory will already be allocated for this data.
    // If the data came from a disk based file, then we allocate enough memory to hold it.
    if (_dataptr) {
	log_debug(_("Using existing Buffer for file"));
	return _dataptr + offset;
    }
    
    std::uint8_t *dataptr = nullptr;
    
    if (_filefd) {
	/// If the data pointer is legit, then we need to unmap that page
	/// to mmap() a new one. If we're still in the current mapped
	/// page, then just return the existing data pointer.
	if (dataptr != nullptr) {
#ifdef _WIN32
	    UnmapViewOfFile(_dataptr);
#elif defined(__amigaos4__)
	if (_dataptr) free(_dataptr);
#else
	    munmap(_dataptr, _pagesize);
#endif
	}
#if 0
	// See if the page has already been mapped in;
	unsigned char vec[_pagesize];
	mincore(offset, _pagesize, vec);
	if (vec[i] & 0x1) {
	    // cached page is in memory
	}
#endif
// 	if (size <= _pagesize) {
// 	    size = _filesize;
// 	}

	// lock in case two threads try to load the same file at the
	// same time.
	std::lock_guard<std::mutex> lock(mem_mutex);
	
#ifdef _WIN32
	HANDLE handle = CreateFileMapping((HANDLE)_get_osfhandle(_filefd), NULL,
					  PAGE_WRITECOPY, 0, 0, NULL);
	if (handle != NULL) {
	    dataptr = static_cast<std::uint8_t *>(MapViewOfFile(handle, FILE_MAP_COPY, 0, offset, page));
	    CloseHandle(handle);

	}
#elif defined(__amigaos4__)
	dataptr = static_cast<std::uint8_t *>(malloc(loadsize));
#else
	dataptr = static_cast<std::uint8_t *>(mmap(nullptr, loadsize,
						     PROT_READ, MAP_SHARED,
						     _filefd, page));
#endif
    } else {
	log_error(_("Couldn't load file %s"), _filespec);
	return nullptr;
    }
    
    if (dataptr == MAP_FAILED) {
	log_error(_("Couldn't map file %s into memory: %s"),
		   _filespec, strerror(errno));
	return nullptr;
    } else {
	log_debug(_("File %s a offset %d mapped to: %p"), _filespec, offset, (void *)dataptr);
	clock_gettime (CLOCK_REALTIME, &_last_access);
	_dataptr = dataptr;
	// map the seekptr to the end of data
	_seekptr = _dataptr + _pagesize;
	_state = OPEN;
	_offset = 0;
    }

    std::uint8_t *ptr = dataptr;
    if (_filetype == FILETYPE_FLV) {
	// FIXME: for now, assume all media files are in FLV format
	_flv.reset(new cygnal::Flv);
	std::shared_ptr<cygnal::Flv::flv_header_t> head = _flv->decodeHeader(ptr);
	ptr += sizeof(cygnal::Flv::flv_header_t);
	ptr += sizeof(cygnal::Flv::previous_size_t);
	std::shared_ptr<cygnal::Flv::flv_tag_t> tag  = _flv->decodeTagHeader(ptr);
	ptr += sizeof(cygnal::Flv::flv_tag_t);
	size_t bodysize = _flv->convert24(tag->bodysize);	    
	if (tag->type == cygnal::Flv::TAG_METADATA) {
	    std::shared_ptr<cygnal::Element> metadata = _flv->decodeMetaData(ptr, bodysize);
	    if (metadata) {
		metadata->dump();
	    }
	}
    }

    if (filesize < _max_memload) {
	close();
    }
    
    // The data pointer points to the real data past all the header bytes.
//    _dataptr = ptr;

    return _seekptr;    
}

/// \brief Write the existing data to the Network.
///
/// @return true is the write suceeded, false if it failed.
bool
DiskStream::writeToNet(int /* start */, int /* bytes */)
{
    GNASH_REPORT_FUNCTION;

    return false;
}

/// \brief Write the data in memory to disk
///
/// @param filespec The relative path to the file to write, which goes in
///		a safebox for storage.
///
/// @return true if the operation suceeded, false if it failed.
bool
DiskStream::writeToDisk()
{
//    GNASH_REPORT_FUNCTION;
    return writeToDisk(_filespec, _dataptr, _filesize);
}

bool
DiskStream::writeToDisk(const std::string &filespec)
{
//    GNASH_REPORT_FUNCTION;
    return writeToDisk(filespec, _dataptr, _filesize);
}

bool
DiskStream::writeToDisk(const std::string &filespec, cygnal::Buffer &data)
{
//    GNASH_REPORT_FUNCTION;
    return writeToDisk(filespec, data.reference(), data.allocated());
}

bool
DiskStream::writeToDisk(const std::string &filespec, std::uint8_t *data, size_t size)
{
//    GNASH_REPORT_FUNCTION;

    int fd = ::open(filespec.c_str() ,O_WRONLY|O_CREAT, S_IRWXU);
    if (fd < 0) {
        log_error(strerror(errno));
    }
    log_debug(_("Writing data (%d bytes) to disk: \"%s\""), size, filespec);
    if(::write(fd, data, size) < 0) {
        log_error(strerror(errno));
    };
    ::close(fd);

    return true;
}

/// \brief Open a file to be streamed.
///
/// @param filespec The full path and file name for the data to be
///	read.
///
/// @return True if the file was opened successfully, false if not.
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
/// @return True if the file was opened successfully, false if not.
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
/// @return True if the file was opened successfully, false if not.
bool
DiskStream::open(const string &filespec, int netfd, Statistics &statistics)
{
    GNASH_REPORT_FUNCTION;

    // the file is already open
    if (_state == OPEN) {
#ifdef USE_STATS_CACHE
	_accesses++;
#endif
	return true;
    }

    // If DONE, then we were previously open, but not closed, so we
    // just reopen the same stream.
    if ((_state == DONE) || (_state == CLOSED)) {
	_state = OPEN;
	return true;
    }
    
    _netfd = netfd;
    _statistics = statistics;
    _filespec = filespec;

    log_debug(_("Trying to open %s"), filespec);
    
    if (getFileStats(filespec)) {
	std::lock_guard<std::mutex> lock(io_mutex);
	_filefd = ::open(_filespec.c_str(), O_RDONLY);
	log_debug (_("Opening file %s (fd #%d), %lld bytes in size."),
		   _filespec, _filefd,
		   (std::int64_t) _filesize);
	_state = OPEN;
	_filetype = determineFileType(filespec);
	loadToMem(0); // load the first page into memory
    } else {
	log_error (_("File %s doesn't exist"), _filespec);
	_state = DONE;
	return false;
    }
    
#ifdef USE_STATS_CACHE
    clock_gettime (CLOCK_REALTIME, &_first_access);
#endif
    
    return true;
}

/// \brief Stream the file that has been loaded,
///
/// @return True if the data was streamed successfully, false if not.
bool
DiskStream::play()
{
//    GNASH_REPORT_FUNCTION;

    return play(_netfd, true);
}

bool
DiskStream::play(bool flag)
{
//    GNASH_REPORT_FUNCTION;

    return play(_netfd, flag);
}

/// \brief Stream the file that has been loaded,
///
/// @param netfd The file descriptor to use for operations
///
/// @param flag True to only send the first packet, False plays entire file.
///
/// @return True if the data was streamed successfully, false if not.
bool
DiskStream::play(int netfd, bool flag)
{
    GNASH_REPORT_FUNCTION;

    bool done = false;
//     dump();
    
    _netfd = netfd;

    while (!done) {
	// If flag is false, only play the one page of the file.
	if (!flag) {
	    done = true;
	}
        switch (_state) {
	  case NO_STATE:
	      log_network(_("No Diskstream open %s for net fd #%d"),
			  _filespec, netfd);
	      break;
          case CREATED:
          case CLOSED:
	      if (_dataptr) {
		  log_network(_("Diskstream %s is closed on net fd #%d."),
			      _filespec, netfd);
	      }
	      done = true;
	      continue;
          case OPEN:
	      loadToMem(0);
	      _offset = 0;
	      _state = PLAY;
	      // continue;
          case PLAY:
	  {
	      size_t ret;
	      Network net;
	      if ((_filesize - _offset) < _pagesize) {
#ifdef HAVE_SENDFILE_XX
		  ret = sendfile(netfd, _filefd, &_offset, _filesize - _offset);
#else
		  ret = net.writeNet(netfd, (_dataptr + _offset), (_filesize - _offset));
		  if (ret != (_filesize - _offset)) {
		      log_error(_("In %s(%d): couldn't write %d bytes to net fd #%d! %s"),
				__FUNCTION__, __LINE__, (_filesize - _offset),
				netfd, strerror(errno));
		  }
#endif
		  log_network(_("Done playing file %s, size was: %d"),
			      _filespec, _filesize);
 		  close();
		  done = true;
		  // reset to the beginning of the file
		  _offset = 0;
	      } else {
		  //log_network("\tPlaying part of file %s, offset is: %d out of %d bytes.", _filespec, _offset, _filesize);
#ifdef HAVE_SENDFILE_XX
		  ret = sendfile(netfd, _filefd, &_offset, _pagesize);
#else
		  ret = net.writeNet(netfd, (_dataptr + _offset), _pagesize);
		  if (ret != _pagesize) {
		      log_error(_("In %s(%d): couldn't write %d of bytes of data to net fd #%d! Got %d, %s"),
				__FUNCTION__, __LINE__, _pagesize, netfd,
				ret, strerror(errno));
		      return false;
		  }
		  _offset += _pagesize;
#endif
	      }
	      switch (errno) {
		case EINVAL:
		case ENOSYS:
		case EFAULT:
		    log_error("%s", strerror(errno));
		    break;
		default:
		    break;
	      }
	      break;
	  }
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
	      log_debug(_("Restarting Disk Stream from the beginning"));
	      _offset = 0;
	      _filefd = 0;
	      _state = PLAY;
	      _seekptr = _dataptr + _pagesize;
	      _netfd = netfd;
              continue;
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
    // _seekptr += nbytes;
#endif

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
/// @return True if the thumbnails were streamed successfully, false if not.
bool
DiskStream::preview(const string & /*filespec*/, int /*frames*/)
{
//    GNASH_REPORT_FUNCTION;

    _state = PREVIEW;
    log_unimpl(__PRETTY_FUNCTION__);
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
/// @return True if the thumbnails were streamed successfully, false if not.
bool
DiskStream::thumbnail(const string & /*filespec*/, int /*quantity*/)
{
//    GNASH_REPORT_FUNCTION;
    
    _state = THUMBNAIL;
    log_unimpl(__PRETTY_FUNCTION__);
    return true; // Default to true
}

/// \brief Pause the stream currently being played.
///
/// @return True if the stream was paused successfully, false if not.
bool
DiskStream::pause()
{
//    GNASH_REPORT_FUNCTION;
    
    _state = PAUSE;
    log_unimpl(__PRETTY_FUNCTION__);
    return true; // Default to true
}

/// \brief Seek within the stream.
///
/// @param the offset in bytes to the location within the file to
///	seek to.
///
/// @return A real pointer to the location of the data seeked to.
std::uint8_t *
DiskStream::seek(off_t offset)
{
//    GNASH_REPORT_FUNCTION;
    
    _state = SEEK;
    return loadToMem(offset);    
}

/// \brief Upload a file into a sandbox.
///	The sandbox is an area where uploaded files can get
///	written to safely. For SWF content, the file name also
///	includes a few optional paths used to seperate
///	applications from each other.
///
/// @param filespec The file name for the data to be written.
///
/// @return True if the file was uploaded successfully, false if not.
bool
DiskStream::upload(const string & /*filespec*/)
{
//    GNASH_REPORT_FUNCTION;
    
    _state = UPLOAD;
    log_unimpl( __PRETTY_FUNCTION__);
    return true; // Default to true
}

// Stream a single "real-time" source.
bool
DiskStream::multicast(const string & /*filespec*/)
{
//    GNASH_REPORT_FUNCTION;
    
    _state = MULTICAST;
    log_unimpl(__PRETTY_FUNCTION__);
    return true; // Default to true    
}

DiskStream::filetype_e
DiskStream::determineFileType()
{
//    GNASH_REPORT_FUNCTION;
    
  return(determineFileType(_filespec));
}

// Get the file type, so we know how to set the
// Content-type in the header.
bool
DiskStream::getFileStats(const std::string &filespec)
{
  //    GNASH_REPORT_FUNCTION;    
    string actual_filespec = filespec;
    struct stat st;
    bool try_again = false;

    do {
      //	cerr << "Trying to open " << actual_filespec << "\r\n";
      if (stat(actual_filespec.c_str(), &st) == 0) {
	// If it's a directory, then we emulate what apache
	// does, which is to load the index.html file in that
	// directry if it exists.
	if (S_ISDIR(st.st_mode)) {
	    log_debug(_("%s is a directory, appending index.html"),
		    actual_filespec.c_str());
	  if (actual_filespec[actual_filespec.size()-1] != '/') {
	    actual_filespec += '/';
	  }
	  actual_filespec += "index.html";
	  try_again = true;
	  continue;
	} else { 		// not a directory
	  //	  log_debug("%s is a normal file\n", actual_filespec.c_str());
	  _filespec = actual_filespec;
	  _filetype = determineFileType(_filespec);
	  _filesize = st.st_size;
	  try_again = false;
	}
      } else {
	_filetype = FILETYPE_NONE;
	return false;
      } // end of stat()
      
      _filesize = st.st_size;
    } while (try_again);
    
    return true;
}

DiskStream::filetype_e 
DiskStream::determineFileType(const string &filespec)
{
//    GNASH_REPORT_FUNCTION;
    
  if (filespec.empty()) {
    return FILETYPE_NONE;
  }

  string::size_type pos;

  string name = filespec;

  // transform to lower case so we match filenames which may
  // have the suffix in upper case, or this test fails.
  std::transform(name.begin(), name.end(), name.begin(), 
		 (int(*)(int)) tolower);

  pos = name.rfind(".");
  if (pos != string::npos) {
    string suffix = name.substr(pos+1, name.size());
    _filetype = FILETYPE_NONE;
    if (suffix == "htm") {
      _filetype = FILETYPE_HTML;
    } else if (suffix == "html") {
      _filetype = FILETYPE_HTML;
    } else if (suffix == "ogg") {
      _filetype = FILETYPE_OGG;
    } else if (suffix == "ogv") {
      _filetype = FILETYPE_OGG;
    } else if (suffix == "swf") {
      _filetype = FILETYPE_SWF;
    } else if (suffix == "php") {
      _filetype = FILETYPE_PHP;
    } else if (suffix == "flv") {
      _filetype = FILETYPE_FLV;
    }else if (suffix == "mp3") {
      _filetype = FILETYPE_MP3;
    } else if (suffix == "flac") {
      _filetype = FILETYPE_FLAC;
    } else if (suffix == "jpg") {
      _filetype = FILETYPE_JPEG;
    } else if (suffix == "jpeg") {
      _filetype = FILETYPE_JPEG;
    } else if (suffix == "txt") {
      _filetype = FILETYPE_TEXT;
    } else if (suffix == "xml") {
      _filetype = FILETYPE_XML;
    } else if (suffix == "mp4") {
      _filetype = FILETYPE_MP4;
    } else if (suffix == "mpeg") {
      _filetype = FILETYPE_MP4;
    } else if (suffix == "png") {
      _filetype = FILETYPE_PNG;
    } else if (suffix == "gif") {
      _filetype = FILETYPE_GIF;
    }
  }

  return _filetype;
}

DiskStream::filetype_e 
DiskStream::determineFileType( std::uint8_t *data)
{
//    GNASH_REPORT_FUNCTION;

  if (data == nullptr) {
    return FILETYPE_NONE;
  }
 
  // JPEG, offset 6 bytes, read the string JFIF
  if (memcpy(data + 6, "JFIF", 4) == nullptr) {
    return FILETYPE_NONE;
  }
  // SWF, offset 0, read the string FWS
  if (memcpy(data, "SWF", 3) == nullptr) {
    return FILETYPE_SWF;
  }
  // compressed SWF, offset 0, read the string CWS
  // FLV, offset 0, read the string FLV
  // PNG, offset 0, read the string PNG
  if (memcpy(data, "PNG", 3) == nullptr) {
    return FILETYPE_PNG;
  }
  // Ogg, offset 0, read the string OggS
  if (memcpy(data, "OggS", 4) == nullptr) {
    return FILETYPE_OGG;
  }
  // Theora, offset 28, read string theora
  if (memcpy(data + 28, "theora", 6) == nullptr) {
    return FILETYPE_THEORA;
  }
  // FLAC, offset 28, read string FLAC
  if (memcpy(data + 28, "FLAC", 4) == nullptr) {
    return FILETYPE_FLAC;
  }
  // Vorbis, offset 28, read string vorbis
  if (memcpy(data + 28, "vorbis", 6) == nullptr) {
    return FILETYPE_VORBIS;
  }
  // MP3, offset 0, read string ID3
  if (memcpy(data, "ID3", 3) == nullptr) {
    return FILETYPE_MP3;
  }

  // HTML
  //   offset 0, read string "\<!doctype\ html"
  //   offset 0, read string "\<head"
  //   offset 0, read string "\<title"
  //   offset 0, read string "\<html"
  if (memcpy(data, "ID3", 3) == nullptr) {
    return FILETYPE_HTML;
  }

  // XML, offset 0, read string "\<?xml"
  if (memcpy(data, "<?xml", 5) == nullptr) {
    return FILETYPE_XML;
  }
  
  return FILETYPE_NONE;
}

///  \brief Dump the internal data of this class in a human readable form.
/// @remarks This should only be used for debugging purposes.
void
DiskStream::dump()
{
    using namespace std;
//    GNASH_REPORT_FUNCTION;
    const char *state_str[] = {
	"NO_STATE",
	"CREATED",
	"CLOSED",
        "OPEN",	
	"PLAY",
	"PREVIEW",
	"THUMBNAIL",
	"PAUSE",
	"SEEK",
	"UPLOAD",
	"MULTICAST",
	"DONE"
    };

    const char *type_str[] = {
        "NONE",
	"AMF",
	"SWF",
	"HTML",
	"PNG",
	"JPEG",
	"GIF",
	"MP3",
	"MP4",
	"OGG",
	"VORBIS",
	"THEORA",
	"DIRAC",
	"TEXT",
	"FLV",
	"VP6",
	"XML",
	"FLAC",
	"ENCODED"
    };	
    
    cerr << "State is \"" << state_str[_state] << "\"" << endl;
    cerr << "File type is \"" << type_str[_filetype] << "\"" << endl;
    cerr << "Filespec is \"" << _filespec << "\"" << endl;
    cerr << "Disk file descriptor is fd #" << _filefd << endl;
    cerr << "Network file descriptor is fd #" << _netfd << endl;
    cerr << "File size is " <<  _filesize << endl;
    cerr << "Memory Page size is " << _pagesize << endl;
    cerr << "Memory Offset is " << _offset << endl;
    cerr << "Base Memory Address is " << (void *)_dataptr << endl;
    cerr << "Seek Pointer Memory Address is " << (void *)_seekptr << endl;
    
    // dump timing related data
    struct timespec now;
    clock_gettime (CLOCK_REALTIME, &now);    
//    double time = ((now.tv_sec - _last_access.tv_sec) + ((now.tv_nsec - _last_access.tv_nsec)/1e9));
    
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
// indent-tabs-mode: nil
// End:
