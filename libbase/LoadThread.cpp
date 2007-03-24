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

// $Id: LoadThread.cpp,v 1.3 2007/03/24 14:36:47 tgc Exp $

#include "LoadThread.h"

LoadThread::LoadThread()
	:
	_bytesLoaded(0),
	_completed(false),
	_loadPosition(0),
	_userPosition(0),
	_actualPosition(0)
{
}

LoadThread::~LoadThread()
{
	// stop the download thread if it's still runnning
	completed();
}

bool LoadThread::setStream(std::auto_ptr<tu_file> stream)
{
	_stream = stream;
	if (_stream.get() != NULL) {
		// Start the downloading.
		_thread.reset( new boost::thread(boost::bind(LoadThread::downloadThread, this)) );

		return true;
	} else {
		return false;
	}
}

bool LoadThread::seek(size_t pos)
{
	// Try to seek to the wanted position, and return
	// true is the new position is equal the wanted,
	// or else return false

	boost::mutex::scoped_lock lock(_mutex);
	_stream->set_position(pos);
	unsigned int ret = _stream->get_position();
	_userPosition = ret;
	_actualPosition = _userPosition;
	return (pos == ret);	
}

size_t LoadThread::read(void *dst, size_t bytes)
{
	
	boost::mutex::scoped_lock lock(_mutex);

	// If the actual position is different from the position
	// last used by the user/owner, seek to the position
	if (_actualPosition != _userPosition) {
		_stream->set_position(_userPosition);
		_actualPosition = _userPosition;
	}
	
	// Try to read a wanted amount of bytes into the given 
	// buffer, note the new position and return the actual amount read
	int ret = _stream->read_bytes(dst, bytes);
	_userPosition += ret;
	_actualPosition = _userPosition;
	return ret;
}

bool LoadThread::eof()
{
	boost::mutex::scoped_lock lock(_mutex);

	// If the actual position is different from the position
	// last used by the user/owner, seek to the position
	if (_actualPosition != _userPosition) {
		_stream->set_position(_userPosition);
		_actualPosition = _userPosition;
	}

	// Check if we're at the EOF
	return _stream->get_eof();

}

size_t LoadThread::tell()
{

	boost::mutex::scoped_lock lock(_mutex);
	return _userPosition;
}

long LoadThread::getBytesLoaded()
{
	boost::mutex::scoped_lock lock(_mutex);

	// The load position is equal to the bytesloaded
	return _loadPosition;
}

long LoadThread::getBytesTotal()
{
	boost::mutex::scoped_lock lock(_mutex);
	return _stream->get_size();
}

bool LoadThread::completed()
{
	boost::mutex::scoped_lock lock(_mutex);
	if (  _completed && _thread.get() )
	{
		_thread->join();
	}

	return _completed;
}

void LoadThread::downloadThread(LoadThread* lt)
{

	// Until the download is completed keep downloading
	while (!lt->_completed) {
		lt->download();
	}

}

void LoadThread::download()
{
	boost::mutex::scoped_lock lock(_mutex);
	size_t CHUNK_SIZE = 1024;
	_stream->set_position(_loadPosition + CHUNK_SIZE);

	unsigned int pos = _stream->get_position();
	if (pos != _loadPosition + CHUNK_SIZE) {
		_completed = true;
	}
	_loadPosition = pos;
	_actualPosition = pos;
}

bool LoadThread::isPositionConfirmed(size_t pos)
{
	boost::mutex::scoped_lock lock(_mutex);
	return (static_cast<int32_t>(pos) <= _loadPosition);
}
