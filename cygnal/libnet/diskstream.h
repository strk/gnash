// 
//   Copyright (C) 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#include <string>
#include <iostream> 

#include "amf.h"
#include "buffer.h"
#include "flv.h"
#include "cque.h"
#include "statistics.h"
#include "getclocktime.hpp"
#include "dsodefs.h"
#include <boost/scoped_ptr.hpp>

/// \namespace gnash
///	This is the main namespace for Gnash and it's libraries.
namespace gnash {

/// \class DiskStream
///	This class handles the loading of files into memory. Instead
///	of using read() from the standard library, this uses mmap() to
///	map the file into memory in chunks of the memory pagesize,
///	which is much faster and less resource intensive.
class DSOEXPORT DiskStream {
public:
    /// \enum DiskStream::state_e
    ///		This represents the state of the current stream.
    typedef enum {
        NO_STATE,
	CREATED,
	CLOSED,
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

  typedef enum {
    FILETYPE_NONE,
    FILETYPE_AMF,
    FILETYPE_SWF,
    FILETYPE_HTML,
    FILETYPE_PNG,
    FILETYPE_JPEG,
    FILETYPE_GIF,
    FILETYPE_MP3,
    FILETYPE_MP4,
    FILETYPE_OGG,
    FILETYPE_VORBIS,
    FILETYPE_THEORA,
    FILETYPE_DIRAC,
    FILETYPE_TEXT,
    FILETYPE_FLV,
    FILETYPE_VP6,
    FILETYPE_XML,
    FILETYPE_FLAC,
    FILETYPE_ENCODED,
    FILETYPE_PHP,
  } filetype_e;

    DSOEXPORT DiskStream();
    DSOEXPORT DiskStream(const std::string &filespec);
    DSOEXPORT DiskStream(const std::string &filespec, cygnal::Buffer &buf);
    DSOEXPORT DiskStream(const std::string &filespec, boost::uint8_t *data, size_t size);
    DSOEXPORT DiskStream(const std::string &filespec, int netfd);
    DSOEXPORT ~DiskStream();

    /// \brief Close the open disk file and it's associated stream.
    DSOEXPORT void close();

    /// \brief Open a file to be streamed.
    ///
    /// @param filespec The full path and file name for the data to be
    ///		read. The file must already exist.
    ///
    /// @param netfd An optional file descriptor to read data from
    ///
    /// @param statistics The optional data structure to use for
    ///		collecting statistics on this stream.
    ///
    /// @return True if the file was opened successfully, false if not.
    DSOEXPORT bool open(const std::string &filespec);
    DSOEXPORT bool open(const std::string &filespec, int netfd);
    DSOEXPORT bool open(const std::string &filespec, int netfd, gnash::Statistics  &statistics);

    /// \brief Stream the file that has been loaded,
    ///
    /// @param netfd An optional file descriptor to read data from
    ///
    /// @param flag True to play the entire file, false to play part.
    ///
    /// @return True if the data was streamed successfully, false if not.
    bool play();
    bool play(bool flag);
    bool play(int netfd, bool flag);
    
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
    /// @return True if the thumbnails were streamed successfully, false if not.
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
    /// @return True if the thumbnails were streamed successfully, false if not.
    bool thumbnail(const std::string &filespec, int quantity);
    
    /// \brief Pause the stream currently being played.
    ///
    /// @return True if the stream was paused successfully, false if not.
    bool pause();
    
    /// \brief Seek within the stream.
    ///
    /// @param the offset in bytes to the location within the file to
    ///		seek to.
    ///
    /// @return A real pointer to the location of the data seeked to.
    boost::uint8_t * seek(off_t offset);
    
    /// \brief Upload a file into a sandbox.
    ///		The sandbox is an area where uploaded files can get
    ///		written to safely. For SWF content, the file name also
    ///		includes a few optional paths used to seperate
    ///		applications from each other.
    ///
    /// @param filespec The file name for the data to be written.
    ///
    /// @return True if the file was uploaded successfully, false if not.
    bool upload(const std::string &filespec);
    
    // Stream a single "real-time" source.
    bool multicast(const std::string &filespec);

    /// \brief Load a chunk of the file into memory
    ///		This offset must be a multipe of the pagesize.
    ///
    /// @param size The amount of bytes to read, often the filesize
    ///		for smaller files below CACHE_LIMIT.
    ///
    /// @param offset The location in bytes in the file of the desired data.
    ///
    /// @return A real pointer to the location of the data at the
    ///		location pointed to by the offset.
    DSOEXPORT boost::uint8_t *loadToMem(size_t filesize, off_t offset);
    DSOEXPORT boost::uint8_t *loadToMem(off_t offset);
    DSOEXPORT boost::uint8_t *loadToMem() { return loadToMem(_offset); };

    /// \brief Write the data in memory to disk
    ///
    /// @param filespec The relative path to the file to write, which goes in
    ///		a safebox for storage.
    ///
    /// @param data The data to be written
    ///
    /// @param size The amount of data in bytes to be written
    ///
    /// @return true if the operation suceeded, false if it failed.
    DSOEXPORT bool writeToDisk(const std::string &filespec, boost::uint8_t *data, size_t size);
    DSOEXPORT bool writeToDisk(const std::string &filespec, cygnal::Buffer &data);
    DSOEXPORT bool writeToDisk(const std::string &filespec);
    DSOEXPORT bool writeToDisk();

    /// \brief Write the existing data to the Network.
    ///
    /// @return true is the write suceeded, false if it failed.
    bool writeToNet(int start, int bytes);

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

    /// \brief copy another DiskStream into ourselves, so they share data
    ///		in memory.
    DiskStream &operator=(DiskStream *stream);

    /// \brief Dump the internal data of this class in a human readable form.
    /// @remarks This should only be used for debugging purposes.
     void dump();
//    friend std::ostream& operator<< (std::ostream &os, const DiskStream &ds);

    /// \brief Get the base address for the memory page.
    ///
    /// @return A real pointer to the base address data in the file, but after
    /// the header bytes.
    boost::uint8_t *get() { return _dataptr; };
    bool fullyPopulated();
//    bool fullyPopulated() { return ((_seekptr - _filesize) == _dataptr); };
    
    /// \brief Get the size of the file.
    ///
    /// @return A value that is the size of the file in bytes.
    size_t getFileSize() { return _filesize; };

    DiskStream::filetype_e getFileType() { return _filetype; };

    std::string &getFilespec() { return _filespec; }
    void setFilespec(std::string filespec) { _filespec = filespec; }

    /// \brief Get the time of the last access.
    ///
    /// @return A real pointer to the struct timespec of the last access.
    struct timespec *getLastAccessTime() { return &_last_access; };

    state_e getState() { return _state; };
    void setState(state_e state) { _state = state; };

    int getFileFd() { return _filefd; };
    int getNetFd() { return _netfd; };
    
#ifdef USE_STATS_CACHE
    /// \brief Get the time of the first access.
    ///		This function should only be used for debugging and
    ///		performance testing.
    ///
    /// @return A real pointer to the struct timespec of the last access.
    struct timespec *getFirstAccessTime() { return &_first_access; };
    size_t getAccessCount() { return _accesses; };
#endif    

    /// \brief Dump the internal data of this class in a human readable form.
    /// @remarks This should only be used for debugging purposes.
    void dump() const { dump(std::cerr); }
    /// \overload dump(std::ostream& os) const
    void dump(std::ostream& os) const;

private:
    /// \var DiskStream::_state
    ///		The current status of the stream while streaming.
    state_e     _state;
    
    /// \var DiskStream::_filefd
    ///		The file descriptor of the disk file.
    int         _filefd;
    
    /// \var DiskStream::_netfd
    ///		The file descriptor of the network connection.
    int         _netfd;

    /// \var DiskStream::_filespec
    ///		The path and file name of the disk file to stream.
    std::string _filespec;

    /// \var DiskStream::_statistics
    ///		The data structure for collecting statistical
    ///		information.
    gnash::Statistics  _statistics;

    /// \var DiskStream::_dataptr
    ///		The base address of the memory page.
    boost::uint8_t *_dataptr;

    /// \var DiskStream::_max_memload
    ///		The maximum amount of data to load into memory
    size_t	_max_memload;
    
    /// \var DiskStream::_seekptr
    ///		The current location within the current memory page where
    ///		the data ends, which is in increments of the empry page size.
    boost::uint8_t *_seekptr;

    /// \var DiskStream::_filesize
    ///		The size of the disk file to stream.
    size_t	_filesize;

    /// \var DiskStream::_pagesize
    ///		The memory page size.
    size_t	_pagesize;

    /// \var DiskStream::_offset
    ///		The offset within the file of the current memory
    ///		page.
    off_t	_offset;

    /// \brief An internal routine used to extract the type of file.
    ///
    /// @param filespec An optional filename to extract the type from.
    ///
    /// @return an AMF filetype
    filetype_e determineFileType();
    filetype_e determineFileType( boost::uint8_t *data);
    filetype_e determineFileType(const std::string &filespec);

    // Get the file stats, so we know how to set the
    // Content-Length in the header.
    bool getFileStats(const std::string &filespec);
    DiskStream::filetype_e _filetype;

    struct timespec _last_access;
    
#ifdef USE_STATS_CACHE
    struct timespec _first_access;	// used for timing how long data stays in the queue.
    size_t	    _accesses;
#endif

#ifdef USE_STATS_FILE
    int         _bytes;
#endif

    // The header, tag, and onMetaData from the FLV file.
    boost::shared_ptr<cygnal::Flv>    _flv;
};

/// \brief Dump to the specified output stream.
inline std::ostream& operator << (std::ostream& os, const DiskStream& ds)
{
	ds.dump(os);
	return os;
}

} // end of cygnal namespace

#endif // __DISKSTREAM_H__

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
