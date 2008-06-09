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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "LoadVariablesThread.h"
#include "IOChannel.h"
#include "log.h"
#include "GnashException.h"
#include "utf8.h"

#include <string>

//#define DEBUG_LOAD_VARIABLES 1

namespace gnash {

void
LoadVariablesThread::completeLoad()
{
#ifdef DEBUG_LOAD_VARIABLES
	log_debug("completeLoad called");
#endif

	using std::string;

	// TODO: how to set _bytesTotal ?

	// this is going to override any previous setting,
	// better do this inside a subclass (in a separate thread)
	_bytesLoaded = 0;
	_bytesTotal = _stream->get_size();

	string toparse;

	size_t CHUNK_SIZE = 1024;
	char* buf = new char[CHUNK_SIZE];
	unsigned int parsedLines = 0;
	// TODO: use read_string ?
	while ( size_t read = _stream->read_bytes(buf, CHUNK_SIZE) )
	{
#ifdef DEBUG_LOAD_VARIABLES
		log_debug("Read %u bytes", read);
#endif

		if ( _bytesLoaded )
		{
			string chunk(buf, read);
			toparse += chunk;
		}
		else
		{
			size_t dataSize = read;
			utf8::TextEncoding encoding;
			char* ptr = utf8::stripBOM(buf, dataSize, encoding);
			if ( encoding != utf8::encUTF8 && encoding != utf8::encUNSPECIFIED )
			{
				log_unimpl("%s to utf8 conversion in MovieClip.loadVariables input parsing", utf8::textEncodingName(encoding));
			}
			string chunk(ptr, dataSize);
			toparse += chunk;
		}

#ifdef DEBUG_LOAD_VARIABLES
		log_debug("toparse: %s", toparse.c_str());
#endif

		// parse remainder
		size_t lastamp = toparse.rfind('&');
		if ( lastamp != string::npos )
		{
			string parseable = toparse.substr(0, lastamp);
#ifdef DEBUG_LOAD_VARIABLES
			log_debug("parseable: %s", parseable.c_str());
#endif
			parse(parseable);
			toparse = toparse.substr(lastamp+1);
#ifdef DEBUG_LOAD_VARIABLES
			log_debug("toparse nextline: %s", toparse.c_str());
#endif
			++parsedLines;
		}

		_bytesLoaded += read;
		//dispatchDataEvent();

		// eof, get out !
		if ( _stream->get_eof() ) break;

		if ( cancelRequested() )
		{
			log_debug("Cancelling LoadVariables download thread...");
			break;
		}
	}

	if ( ! toparse.empty() )
	{
		parse(toparse);
	}

	_stream->go_to_end();
	_bytesLoaded = _stream->get_position();
	if ( _bytesTotal !=  _bytesLoaded )
	{
		log_error("Size of stream variables were loaded from advertised to be %d bytes long,"
		         " but turned out to be only %d bytes long",
			_bytesTotal, _bytesLoaded);
		_bytesTotal = _bytesLoaded;
	}

	//dispatchLoadEvent();
	delete[] buf;
	setCompleted();
}

LoadVariablesThread::LoadVariablesThread(const URL& url, const std::string& postdata)
	:
	_stream(StreamProvider::getDefaultInstance().getStream(url, postdata)),
	_completed(false),
	_canceled(false)
{
	if ( ! _stream.get() )
	{
		throw NetworkException();
	}
}

LoadVariablesThread::LoadVariablesThread(const URL& url)
	:
	_stream(StreamProvider::getDefaultInstance().getStream(url)),
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
