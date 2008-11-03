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

/// \namespace cygnal
///	This namespace is for all the Cygnal specific classes.
namespace cygnal {

/// \class DiskStream
///	This class handles the loading of files into memory. Instead
///	of using read() from the standard library, this uses mmap() to
///	map the file into memory in chunks of the memory pagesize,
///	which is much faster and less resource intensive.
class DiskStream {
public:
    /// \enum DiskStream::state_e
    ///		This represents the state of the current stream.
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

    /// \brief Close the open disk file and it's associated stream.
    void close();

    /// \brief Open a file to be streamed.
    ///
    /// @param filespec The full path and file name for the data to be
    ///		read.
    ///
    /// @param netfd An optional file descriptor to read data from
    ///
    /// @param statistics The optional data structure to use for
    ///		collecting statistics on this stream.
    ///
    /// @return True if the file was opened sucessfully, false if not.
    bool open(const std::string &filespec);
    bool open(const std::string &filespec, int netfd);
    bool open(const std::string &filespec, int netfd, gnash::Statistics  &statistics);
    
    /// \brief Stream the file that has been loaded,
    ///
    /// @param netfd An optional file descriptor to read data from
    ///
    /// @return True if the data was streamed sucessfully, false if not.
    bool play();
    bool play(int netfd);
    
    /// \brief Stream a preview of the file.
    ///		A preview is a series of video frames from
    ///		the video file. Each video frame is taken by sampling
    ///		the file at a set interval.
    ///
    /// @param filespec The full path and file name for the data to be
    ///		read.
    ///
    /// @param quantity The number of frames to stream..
    ///
    /// @return True if the thumbnails were streamed sucessfully, false if not.
    bool preview(const std::string &filespec, int frames);
    
    /// \brief Stream a series of thumbnails.
    ///		A thumbnail is a series of jpg images of frames from
    ///		the video file instead of video frames. Each thumbnail
    ///		is taken by sampling the file at a set interval.
    ///
    /// @param filespec The full path and file name for the data to be
    ///		read.
    ///
    /// @param quantity The number of thumbnails to stream..
    ///
    /// @return True if the thumbnails were streamed sucessfully, false if not.
    bool thumbnail(const std::string &filespec, int quantity);
    
    /// \brief Pause the stream currently being played.
    ///
    /// @return True if the stream was paused sucessfully, false if not.
    bool pause();
    
    /// \brief Seek within the stream.
    ///
    /// @param the offset in bytes to the location within the file to
    ///		seek to.
    ///
    /// @return True if the stream was paused sucessfully, false if not.
    bool seek(off_t offset);
    
    /// \bried Upload a file into a sandbox.
    ///		The sandbox is an area where uploaded files can get
    ///		written to safely. For SWF content, the file name also
    ///		includes a few optional paths used to seperate
    ///		applications from each other.
    ///
    /// @param filespec The file name for the data to be written.
    ///
    /// @return True if the file was uploaded sucessfully, false if not.
    bool upload(const std::string &filespec);
    
    // Stream a single "real-time" source.
    bool multicast(const std::string &filespec);

    /// \brief Load a chunk of the file into memory
    ///		This offset must be a multipe of the pagesize.
    ///
    /// @param size The location in bytes in the file of the desired data.
    ///
    /// @return A real pointer to the location of the data at the
    ///		location pointed to by the offset.
    boost::uint8_t *loadChunk(off_t size);
    boost::uint8_t *loadChunk() { return loadChunk(_offset); };

    /// \brief Get the memory page size
    ///		This is a cached value of the system configuration
    ///		value for the default size in bytes of a memory page.
    ///
    /// @return the currently cached memory page size.
    size_t getPagesize() { return _pagesize; };
    
    /// \brief Set the memory page size
    ///		This is a cached value of the system configuration
    ///		value for the default size in bytes of a memory page.
    ///
    /// @param size The size of a page of memory to cache.
    ///
    /// @return nothing.
    void setPagesize(size_t size) { _pagesize = size; };
    
    ///  \brief Dump the internal data of this class in a human readable form.
    ///
    /// @remarks This should only be used for debugging purposes.
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
