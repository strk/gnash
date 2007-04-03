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

// $Id: LoadThread.cpp,v 1.5 2007/04/03 16:45:02 strk Exp $

#include "LoadThread.h"

LoadThread::LoadThread()
	:
	_completed(false),
	_loadPosition(0),
	_userPosition(0),
	_actualPosition(0),
	_cache(NULL),
	_cacheStart(0),
	_cachedData(0),
	_cacheSize(0),
	_chunkSize(56),
	_streamSize(0)
{
}

LoadThread::~LoadThread()
{
	// stop the download thread if it's still runnning
	completed();
	if (_cache) delete[] _cache;
}

bool LoadThread::setStream(std::auto_ptr<tu_file> stream)
{
	_stream = stream;
	if (_stream.get() != NULL) {
		// Start the downloading.
		setupCache();
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

	if (_loadPosition >= static_cast<long>(pos)) {
		_userPosition = pos;
		return true;
	} else {
		_userPosition = _loadPosition;
		return false;
	}
}

size_t LoadThread::read(void *dst, size_t bytes)
{

	// If the data is in the cache we used it
	if (_cacheStart <= _userPosition && static_cast<long>(bytes) + _userPosition <= _cacheStart + _cachedData) {
		memcpy(dst, _cache + (_userPosition - _cacheStart), bytes);
		_userPosition += bytes;
		return bytes;

	// If the data is not in cache, but the file is completely loaded
	// we just get the data directly from the stream 
	} else if (_completed) {
		
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

	// The wanted data wasen't in the cache, and the file isn't loaded
	// so we now either load more data into the cache, or completely
	// replace the content.

	boost::mutex::scoped_lock lock(_mutex);

	// If the new data can fit in the cache we just load it into it
	if (_cacheStart <= _userPosition && static_cast<long>(bytes) + _userPosition < _cacheStart + _cacheSize) {

		// If the actual position is different from the position
		// last used by the user/owner, seek to the position
		if (_actualPosition != _userPosition) {
			_stream->set_position(_userPosition);
			_actualPosition = _userPosition;
		}

		// Try to read a wanted amount of bytes into the given 
		// buffer, note the new position and return the actual amount read
		int ret = _stream->read_bytes(dst, bytes);

		memcpy(_cache +(_userPosition - _cacheStart), dst, ret);
		_cachedData = _userPosition - _cacheStart + ret;
		_userPosition += ret;
		_actualPosition = _userPosition;
		return ret;

	}

	// We need to replace the cache...

	// check if the cache is big enough to contain the wanted data
	if (static_cast<long>(bytes) > _cacheSize-20000) {
		delete[] _cache;
		_cacheSize = bytes+20000;
		_cache = new uint8_t[_cacheSize];
	}

	// To avoid recaching all the time, we cache some data from before
	// the _userPosition
	long newcachestart = _userPosition;
	if (_userPosition > 20000) {
		newcachestart = _userPosition - 20000;
	}

	// Amount to read into the cache
	long readdata = 0; 
	if (_loadPosition >= newcachestart + _cacheSize) readdata = _cacheSize;
	else if (_loadPosition < newcachestart + _cacheSize && _loadPosition > _userPosition + static_cast<long>(bytes)) readdata = _loadPosition - newcachestart;
	else readdata = bytes + (_userPosition - newcachestart);

	// If the actual position is different from the position
	// last used by the user/owner, seek to the position
	if (_actualPosition != _userPosition) {
		_stream->set_position(newcachestart);
		_actualPosition = newcachestart;
	}


	// Try to read a wanted amount of bytes into the given 
	// buffer, note the new position and return the actual amount read
	int ret = _stream->read_bytes(_cache, readdata);

	_cachedData = ret;
	_cacheStart = newcachestart;

	if (ret < _userPosition - newcachestart) return 0;

	int newret = bytes;
	if (static_cast<int>(bytes) > ret) newret = ret - (_userPosition - newcachestart);

	memcpy(dst, _cache + (_userPosition - newcachestart), newret);
	_userPosition += newret;
	_actualPosition = newcachestart + _cachedData;
	if (newcachestart + _cachedData > _loadPosition) _loadPosition = _actualPosition;
	return newret;
}

bool LoadThread::eof()
{
	// Check if we're at the EOF
	if (_completed && _userPosition >= _loadPosition) return true;
	else return false;
}

size_t LoadThread::tell()
{
	return _userPosition;
}

long LoadThread::getBytesLoaded()
{
	// The load position is equal to the bytesloaded
	return _loadPosition;
}

long LoadThread::getBytesTotal()
{
	return _streamSize;
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

void LoadThread::setupCache()
{
	boost::mutex::scoped_lock lock(_mutex);

	_cache = new uint8_t[1024*500];
	_cacheSize = 1024*500;

	int ret = _stream->read_bytes(_cache, 1024);
	_cacheStart = 0;
	_cachedData = ret;
	_loadPosition = 1024;
	_streamSize = _stream->get_size();
}

void LoadThread::downloadThread(LoadThread* lt)
{
	// Until the download is completed keep downloading
	while (!lt->_completed) {
		if (lt->_chunkSize + lt->_loadPosition > lt->_cacheStart + lt->_cacheSize) lt->download();
		else lt->fillCache();
	}

}

void LoadThread::fillCache()
{
	if (_loadPosition >= _streamSize) {
		_completed = true;
		return;
	}

	boost::mutex::scoped_lock lock(_mutex);

	if (_loadPosition != _actualPosition) _stream->set_position(_loadPosition);

	int ret = _stream->read_bytes(_cache+_cachedData, _chunkSize);

	if (ret != _chunkSize) {
		_completed = true;
	}
	_cachedData += ret;
	_loadPosition = _loadPosition + ret;
	_actualPosition = _loadPosition;
}

void LoadThread::download()
{
	if (_loadPosition >= _streamSize) {
		_completed = true;
		return;
	}

	boost::mutex::scoped_lock lock(_mutex);

	_stream->set_position(_loadPosition + _chunkSize);

	long pos = _stream->get_position();
	if (pos != _loadPosition + _chunkSize) {
		_completed = true;
	}
	_loadPosition = pos;
	_actualPosition = pos;
}

bool LoadThread::isPositionConfirmed(size_t pos)
{
	return (static_cast<int32_t>(pos) <= _loadPosition);
}
