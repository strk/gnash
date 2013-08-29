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

#include "LoadVariablesThread.h"
#include "IOChannel.h"
#include "log.h"
#include "GnashException.h"
#include "utf8.h"
#include "StreamProvider.h"

#include <string>
#include <boost/scoped_array.hpp>

//#define DEBUG_LOAD_VARIABLES 1

namespace gnash {

void
LoadVariablesThread::completeLoad()
{
#ifdef DEBUG_LOAD_VARIABLES
    log_debug("completeLoad called");
#endif


	// TODO: how to set _bytesTotal ?

	// this is going to override any previous setting,
	// better do this inside a subclass (in a separate thread)
	_bytesLoaded = 0;
	_bytesTotal = _stream->size();

	std::string toparse;

	const size_t chunkSize = 1024;
	boost::scoped_array<char> buf(new char[chunkSize]);
	unsigned int parsedLines = 0;
	// TODO: use read_string ?
	while ( size_t bytesRead = _stream->read(buf.get(), chunkSize) )
	{
#ifdef DEBUG_LOAD_VARIABLES
            log_debug("Read %u bytes", bytesRead);
#endif

		if ( _bytesLoaded )
		{
			std::string chunk(buf.get(), bytesRead);
			toparse += chunk;
		}
		else
		{
			size_t dataSize = bytesRead;
			utf8::TextEncoding encoding;
			char* ptr = utf8::stripBOM(buf.get(), dataSize,
					encoding);
			if ( encoding != utf8::encUTF8 &&
			     encoding != utf8::encUNSPECIFIED )
			{
                  log_unimpl(_("%s to UTF8 conversion in "
					    "MovieClip.loadVariables "
                                         "input parsing"),
					    utf8::textEncodingName(encoding));
			}
			std::string chunk(ptr, dataSize);
			toparse += chunk;
		}

#ifdef DEBUG_LOAD_VARIABLES
		log_debug("toparse: %s", toparse);
#endif

		// parse remainder
		size_t lastamp = toparse.rfind('&');
		if ( lastamp != std::string::npos )
		{
			std::string parseable = toparse.substr(0, lastamp);
#ifdef DEBUG_LOAD_VARIABLES
			log_debug("parseable: %s", parseable);
#endif
			parse(parseable);
			toparse = toparse.substr(lastamp+1);
#ifdef DEBUG_LOAD_VARIABLES
			log_debug("toparse nextline: %s", toparse);
#endif
			++parsedLines;
		}

		_bytesLoaded += bytesRead;

		// eof, get out !
		if ( _stream->eof() ) break;

		if ( cancelRequested() ) {
                    log_debug("Cancelling LoadVariables download thread...");
			_stream.reset();
			return;
		}
	}

	if ( ! toparse.empty() ) {
		parse(toparse);
	}

	try {
		_stream->go_to_end();
	}
        catch (IOException& ex) {
        log_error(_("Stream couldn't seek to end: %s"), ex.what());
	}
	
    _bytesLoaded = _stream->tell();
	if ( _bytesTotal !=  _bytesLoaded ) {
            log_error(_("Size of 'variables' stream advertised to be %d bytes,"
                          " but turned out to be %d bytes."),
			_bytesTotal, _bytesLoaded);
		_bytesTotal = _bytesLoaded;
	}

	_stream.reset(); // we don't need the IOChannel anymore

	//dispatchLoadEvent();
	setCompleted();
}

LoadVariablesThread::LoadVariablesThread(const StreamProvider& sp,
        const URL& url, const std::string& postdata)
	:
	_bytesLoaded(0),
	_bytesTotal(0),
	_stream(sp.getStream(url, postdata)),
	_completed(false),
	_canceled(false)
{
	if ( ! _stream.get() )
	{
		throw NetworkException();
	}
}

LoadVariablesThread::LoadVariablesThread(const StreamProvider& sp,
        const URL& url)
	:
	_bytesLoaded(0),
	_bytesTotal(0),
	_stream(sp.getStream(url)),
	_completed(false),
	_canceled(false)
{
	if ( ! _stream.get() )
	{
		throw NetworkException();
	}
}

void
LoadVariablesThread::cancel()
{
	boost::mutex::scoped_lock lock(_mutex);
	_canceled = true;
}

bool
LoadVariablesThread::cancelRequested()
{
	boost::mutex::scoped_lock lock(_mutex);
	return _canceled;
}

LoadVariablesThread::~LoadVariablesThread()
{
	if ( _thread.get() )
	{
		cancel();
		_thread->join();
		_thread.reset();
	}
}


} // namespace gnash 
