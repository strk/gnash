// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <cerrno>
#include <sys/mman.h>
#include <unistd.h>

#include "stream.h"
#include "network.h"
#include "amf.h"
#include "rtmp.h"
#include "log.h"

#include <boost/thread/mutex.hpp>
static boost::mutex io_mutex;

using namespace gnash;
using namespace std;

namespace cygnal {

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
}

#if 0
static void
sendfile_thread()
{
    GNASH_REPORT_FUNCTION;
    
    struct stat stats;
    struct filedes loadfile;
    int fd;
    char *fdptr;
    
    memcpy(&loadfile, arg, sizeof(struct filedes));
    
    // Get the file stats
    if (stat(loadfile.filespec, &stats) == 0) {

	fd = open(loadfile.filespec, O_RDONLY);
// 	dbglogfile << "File " << loadfile.filespec
// 		   << " is " << stats.st_size
// 		   << " bytes in size." << endl;
	if (fd) {
	    fdptr = static_cast<char *>(mmap(0, stats.st_size,
		   PROT_READ, MAP_SHARED, fd, 0));
	} else {
	    dbglogfile << "ERROR: Couldn't load "
		       << loadfile.filespec << endl;
	}
	
	if (fdptr == MAP_FAILED) {
	    dbglogfile << "ERROR: Couldn't map file "
		       << loadfile.filespec << " into memory! "
		       << strerror(errno) << endl;
	} else {	    
	    dbglogfile << "File " << loadfile.filespec
		       << " mapped to: " << (void *)fdptr << endl;
	}
	
// 	if (stats.st_size > 1024*8) {
// 	}
	
// 	if (stats.st_blksize > 10) {
// 	}

// 	if (stats.st_blocks > 10) {
// 	}
    } else {
	dbglogfile << "ERROR: File " << loadfile.filespec
		   << " doesn't exist!" << endl;
    }

    int filesize = stats.st_size;
    int blocksize = 8192;
    char *tmpptr = fdptr;
    int nbytes = 0;
    Network net;
    while ((_seekptr - _dataptr) <= RTMP_BODY_SIZE) {
	if (filesize < RTMP_BODY_SIZE) {
	    nbytes = net.writeNet(loadfile.fd, tmpptr, filesize);
	    filesize = 0;
	} else {
	    nbytes = net.writeNet(loadfile.fd, tmpptr, blocksize);
	    filesize -= blocksize;
	    tmpptr += blocksize;
	}
	
	if (nbytes <= 0) {
	    dbglogfile << "Done..." << endl;
	    return &nbytes;
	}
    }
	   
    close(fd);
    munmap(fdptr, stats.st_size);
}
#endif

Stream::Stream()
    : _bytes(0),
      _filefd(0),
      _netfd(0),
      _filespec(0),
      _statistics(0),
      _dataptr(0),
      _seekptr(0),
      _filesize(0)
{
    GNASH_REPORT_FUNCTION;
    
}

Stream::~Stream() {
    GNASH_REPORT_FUNCTION;
    if (_filefd) {
        close(_filefd);
    }
    if (_netfd) {
        close(_netfd);
    }
}

bool
Stream::open(const char *filespec) {
    GNASH_REPORT_FUNCTION;
    
    return open(filespec, _netfd);
}

bool
Stream::open(const char *filespec, int netfd) {
    GNASH_REPORT_FUNCTION;

    struct stat stats;
    return open(filespec, _netfd, _statistics);
}

bool
Stream::open(const char *filespec, int netfd, Statistics  *statistics) {
    GNASH_REPORT_FUNCTION;

    struct stat st;

    _netfd = netfd;
    _statistics = statistics;

    dbglogfile << "Trying to open " << filespec << endl;
    
    if (stat(filespec, &st) == 0) {
        _filesize = st.st_size;
        boost::mutex::scoped_lock lock(io_mutex);
	_filefd = ::open(filespec, O_RDONLY);
  	dbglogfile << "File " << filespec
  		   << " is " << _filesize
  		   << " bytes in size." << endl;
	if (_filefd) {
	    _dataptr = static_cast<unsigned char *>(mmap(0, _filesize,
		   PROT_READ, MAP_SHARED, _filefd, 0));
	} else {
	    dbglogfile << "ERROR: Couldn't load "
		       << filespec << endl;
            return false;
	}
	
	if (_seekptr == MAP_FAILED) {
	    dbglogfile << "ERROR: Couldn't map file "
		       << filespec << " into memory! "
		       << strerror(errno) << endl;
            return false;
	} else {	    
	    dbglogfile << "File " << filespec
		       << " mapped to: " << (void *)_dataptr << endl;
            _seekptr = _dataptr;
            _state = OPEN;
            return true;
	}
// 	if (stats.st_size > 1024*8) {
// 	}
	
// 	if (stats.st_blksize > 10) {
// 	}

// 	if (stats.st_blocks > 10) {
// 	}
    } else {
	dbglogfile << "ERROR: File " << filespec
		   << " doesn't exist!" << endl;
    }

    
    return true;
}

// Stream the movie
bool
Stream::play() {
    GNASH_REPORT_FUNCTION;

    return play(_netfd);
}

bool
Stream::play(int netfd) {
    GNASH_REPORT_FUNCTION;

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

    int blocksize = 8192;
    int nbytes = 0;
    Network net;
//    while ((_seekptr - _dataptr) >= 0) {
    nbytes = net.writeNet(_netfd, (char *)_seekptr, _filesize);
//    if (nbytes <= 0) {
//        break;
//    }
    _statistics->addBytes(nbytes);
    _bytes += nbytes;
    _seekptr += nbytes;
	
    dbglogfile << "Done..." << endl;
	   
    munmap(_dataptr, _filesize);
    _seekptr = 0;

    return true;
}

// Stream a preview, instead of the full movie.
bool
Stream::preview(const char *filespec, int frames) {
    GNASH_REPORT_FUNCTION;

    _state = PREVIEW;
    return true; // Default to true    
}

// Stream a series of thumbnails
bool
Stream::thumbnail(const char *filespec, int quantity) {
    GNASH_REPORT_FUNCTION;
    
    _state = THUMBNAIL;
    return true; // Default to true
}

// Pause the stream
bool
Stream::pause(int frame) {
    GNASH_REPORT_FUNCTION;
    
    _state = PAUSE;
    return true; // Default to true
}

// Seek within the stream
bool
Stream::seek(int frame) {
    GNASH_REPORT_FUNCTION;
    
    _state = SEEK;
    return true; // Default to true    
}

// Upload a stream into a sandbox
bool
Stream::upload(const char *filespec) {
    GNASH_REPORT_FUNCTION;
    
    _state = UPLOAD;
    return true; // Default to true
}

// Stream a single "real-time" source.
bool Stream::multicast(const char *filespec) {
    GNASH_REPORT_FUNCTION;
    
    _state = MULTICAST;
    return true; // Default to true    
}

} // end of cygnal namespace


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
