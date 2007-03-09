// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

// $Id: LoadThread.h,v 1.2 2007/03/09 23:15:57 tgc Exp $
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>

#include "tu_file.h"
#include <memory>

/// \brief
/// The LoadThread class can be used to download from a file
/// or stream using a thread, without having to block.
/// When the object is created it starts a thread which downloads
/// from the tu_file given as an argument to the constructor, and
/// keeps downloading until the download is complete. It is possible
/// for the object owner to query for data, position, loaded data,
/// total data while downloading, without blocking. Though if there has 
/// not been downloaded enough data to accomendate a request (seek/read)
/// it will block until the data is present (curl_adaptor behavoir).
///
/// When using the LoadThread, all access to the tu_file should be
/// done through LoadThread, or it will likely break.
///
/// @todo When we read from a real movie stream (rtmp) we might
/// want to use a cirkular-buffer.

class LoadThread
{

public:
	/// Creating the object starts the threaded loading 
	/// of the stream/file passed as an argument.
	LoadThread(tu_file* stream);
	~LoadThread();

	/// Put read pointer at given position
	/// Will block if there is not enough data buffered,
	/// and wait until enough data is present.
	bool seek(size_t pos);

	/// Read 'bytes' bytes into the given buffer.
	/// Return number of actually read bytes.
	/// Will block if there is not enough data buffered,
	/// and wait until enough data is present.
	size_t read(void *dst, size_t bytes);

	/// Return true if EOF has been reached
	bool eof();

	/// Report global position within the file
	size_t tell();

	///	Returns the number of bytes cached
	long getBytesLoaded();

	///	Returns the total size of the file
	long getBytesTotal();

	/// Check if the load is completed
	bool completed();

private:

	/// The thread function used to download from the stream
	static void downloadThread(LoadThread* lt);

	/// The function that does the actual downloading
	void download();

	/// The stream/file we want to access
	std::auto_ptr<tu_file> _stream;

	long _bytesLoaded;

	bool _completed;

	boost::mutex _mutex;

	std::auto_ptr<boost::thread> _thread;
	
	long _loadPosition;
	long _userPosition;
	long _actualPosition;
};

