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

#ifndef __DISKSTREAM_H__
#define __DISKSTREAM_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

//#ifdef HAVE_AIO_H
//#include <aio.h>
//#endif

#include <string>
#include <iostream> 

#include "cque.h"
#include "statistics.h"

namespace cygnal {

class DiskStream {
public:
    typedef enum {
        NO_STATE,
        OPEN,
        PLAY,
        PREVIEW,
        THUMBNAIL,
        PAUSE,
        SEEK,
        UPLOAD,
        MULTICAST,
        DONE
    } state_e;  
    DiskStream();
    DiskStream(const std::string &filespec);
    DiskStream(const std::string &filespec, int netfd);
    ~DiskStream();

    /// \brief close the open disk file and stream.
    void close();
    
    bool open(const std::string &filespec);
    bool open(const std::string &filespec, int netfd);
    bool open(const std::string &filespec, int netfd, gnash::Statistics  &statistics);
    
    // Stream the movie
    bool play();
    bool play(int netfd);
    
    // Stream a preview, instead of the full movie.
    bool preview(const std::string &filespec, int frames);
    
    // Stream a series of thumbnails
    bool thumbnail(const std::string &filespec, int quantity);
    
    // Pause the stream
    bool pause(int frame);
    
    // Seek within the stream
    bool seek(int frame);
    
    // Upload a stream into a sandbox
    bool upload(const std::string &filespec);
    
    // Stream a single "real-time" source.
    bool multicast(const std::string &filespec);

    /// \brief Load a chunk of the file into memory
    ///		This offset must be a multipe of the pagesize.
    boost::uint8_t *loadChunk(off_t size);
    boost::uint8_t *loadChunk() { return loadChunk(_offset); };

    size_t getPagesize() { return _pagesize; };
    void setPagesize(size_t size) { _pagesize = size; };
    
    void dump();
//    friend std::ostream& operator<< (std::ostream &os, const DiskStream &ds);

    boost::uint8_t *get() { return _dataptr; };
private:
    state_e     _state;
    int         _bytes;
    int         _filefd;
    int         _netfd;
    std::string _filespec;
    gnash::Statistics  _statistics;
    boost::uint8_t *_dataptr;
    boost::uint8_t *_seekptr;
    size_t	_filesize;
    size_t	_pagesize;
    off_t	_offset;
//    struct aiocb _aio_control_block;
    gnash::CQue _que;
};
 
} // end of cygnal namespace

//#endif	// HAVE_AIO_H

#endif // __DISKSTREAM_H__

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
