// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_LOADTHREAD_H
#define GNASH_LOADTHREAD_H


#include "IOChannel.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include <boost/scoped_array.hpp>
#include <boost/noncopyable.hpp>

#include <memory>

// Undefine this to NOT use a thread for loading
// This is useful for debugging.
#define THREADED_LOADS 1

namespace gnash {

/// \brief
/// The LoadThread class can be used to download from a file
/// or stream using a thread, without having to block.
//
/// When the object is created it starts a thread which seeks forward
/// in the IOCHannel given as an argument to the constructor, which will
/// make cause the IOChannel backend to download the amount of data needed
/// to complete the seek. This is repeated until the complete file is
/// downloaded. It is possible
/// for the object owner to query for data, position, loaded data,
/// total data while downloading, without blocking. Though if there has 
/// not been downloaded enough data to accomendate a request (seek/read)
/// it will block until the data is present (curl_adaptor behavoir).
///
/// When using the LoadThread, all access to the IOChannel should be
/// done through LoadThread, or it will likely break.
///
class DSOEXPORT LoadThread : private boost::noncopyable
{

public:
	/// Create a LoadThread using to the given IOChannel as input
	LoadThread(std::auto_ptr<IOChannel> str);

	/// Stops the download if still running
	~LoadThread();

	/// Put read pointer at given position
	//
	/// Will block if there is not enough data buffered,
	/// and wait until enough data is present.
	///
	/// @return 0 on success, -1 on error (EOF).
	///
	int seek(size_t pos);

	/// Read 'bytes' bytes into the given buffer.
	/// Return number of actually read bytes.
	/// Will block if there is not enough data buffered,
	/// and wait until enough data is present.
	size_t read(void *dst, size_t bytes);

	/// Return true if EOF has been reached
	bool eof() const;

	/// Report global position within the file
	size_t tell() const;

	///	Returns the number of bytes known to be accessable
	long getBytesLoaded() const;

	///	Returns the total size of the file
	//
	/// NOTE:
	/// You can't rely on returned
	/// value to know the exact size, as network
	/// server (http/ftp) might return misleading
	/// information about the size of a resource.
	///
	/// TODO:
	/// Document current implementation when it
	/// comes to read past that advertised size
	/// or short of it... Reading the code is
	/// discouraging for me (--strk)
	///
	long getBytesTotal() const;

	// alias for getBytesTotal()
	long size() const { return getBytesTotal(); }

	/// Check if the load is completed
	bool completed() const;

	/// Check if given position is confirmed to be accessable
	bool isPositionConfirmed(size_t pos) const
	{
		return (static_cast<boost::int32_t>(pos) <= _loadPosition);
	}

	/// Request download cancel
	void requestCancel();

private:

	/// Return true if cancel was requested
	bool cancelRequested() const;

	/// The thread function used to download from the stream
	static void downloadThread(LoadThread* lt);

	/// The function that does the actual downloading
	void download();

	/// Fills the cache at the begining
	void setupCache();

	/// Fills the cache when needed
	void fillCache();

	/// The stream/file we want to access
	std::auto_ptr<IOChannel> _stream;

	volatile bool _completed;

#ifdef THREADED_LOADS
	mutable boost::mutex _mutex;

	std::auto_ptr<boost::thread> _thread;
#endif
	
	volatile long _loadPosition;
	volatile long _userPosition;
	volatile long _actualPosition;

	// If true, download request was canceled
	bool _cancelRequested;

	// Cache...
	boost::scoped_array<boost::uint8_t> _cache;

	// The fileposition where the cache start
	volatile long _cacheStart;

	// Data amount in the cache
	volatile long _cachedData;

	// Size of the cache
	volatile long _cacheSize;

	// The amount we load at one go
	long _chunkSize;

	// size of the stream
	long _streamSize;

	// Tell the download loop to be nice and take a break
	// This is needed since the loop in downloadThread() calls fillCache() and 
	// download() which locks _mutex, and sometimes the read() function can't
	// get a lock because fillCache() and download() just "keeps" it, which can
	// makes read() wait for for a really long time.
	volatile bool _needAccess;

	/// Reset all values to original state
	void reset();
};

} // namespace gnash

#endif // GNASH_LOADTHREAD_H
