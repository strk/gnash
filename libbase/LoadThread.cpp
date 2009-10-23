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


#include "LoadThread.h"
#include "log.h"
#include "GnashSleep.h"

namespace gnash {

LoadThread::LoadThread(std::auto_ptr<IOChannel> stream)
	:
	_stream(stream),
	_completed(false),
	_loadPosition(0),
	_userPosition(0),
	_actualPosition(0),
	_cancelRequested(false),
	_cache(new boost::uint8_t[65535]),
	_cacheStart(0),
	_cachedData(0),
	_cacheSize(0),
	_chunkSize(56),
	_streamSize(0),
	_needAccess(false),
    _failed(!_stream.get())
{
    if (_failed) return;
}


void
LoadThread::download()
{
    _loadPosition += _stream->read(_cache.get() + _loadPosition, 65535);
    log_debug("loadPosition: %s", _loadPosition);
}

size_t
LoadThread::read(void *dst, size_t bytes)
{
    size_t b = std::min<size_t>(bytes, _loadPosition - _userPosition);

    log_debug("B: %s, _userPos: %s", b, _userPosition);
    std::copy(_cache.get() + _userPosition, _cache.get() + _userPosition + b,
            (char*)dst);
    _userPosition += b;
    return b;
}

bool LoadThread::eof() const
{
    return _failed || _stream->eof();
}

size_t LoadThread::tell() const
{
    return _loadPosition;
}

long LoadThread::getBytesLoaded() const
{
    return tell();
}

long LoadThread::getBytesTotal() const
{
    return _stream->size();
}

bool LoadThread::completed() const
{
    log_debug("Stream completed() ? %s, %s, %s", _failed, _stream->eof(),
            _stream->bad());
	return _failed || _stream->eof();
}

} // namespace gnash
