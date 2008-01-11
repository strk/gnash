// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
#include "config.h"
#endif

#include "LoadVariablesThread.h"
#include "tu_file.h"
#include "log.h"
#include "GnashException.h"

#include <string>

namespace gnash {

void
LoadVariablesThread::completeLoad()
{
	//log_msg("completeLoad called");

	using std::string;

	// TODO: how to set _bytesTotal ?

	// this is going to override any previous setting,
	// better do this inside a subclass (in a separate thread)
	_bytesLoaded = 0;
	_bytesTotal = _stream->get_size();

	string toparse;

	size_t CHUNK_SIZE = 1024;
	char *buf = new char[CHUNK_SIZE];
	unsigned int parsedLines = 0;
	while ( size_t read = _stream->read_bytes(buf, CHUNK_SIZE) )
	{
		//log_msg("Read %u bytes", read);

		// TODO: use read_string ?
		string chunk(buf, read);
		toparse += chunk;

		//log_msg("toparse: %s", toparse.c_str());

		// parse remainder
		size_t lastamp = toparse.rfind('&');
		if ( lastamp != string::npos )
		{
			string parseable = toparse.substr(0, lastamp);
			//log_msg("parseable: %s", parseable.c_str());
			parse(parseable);
			toparse = toparse.substr(lastamp+1);
			//log_msg("toparse nextline: %s", toparse.c_str());
			++parsedLines;
		}

		_bytesLoaded += read;
		//dispatchDataEvent();

		// eof, get out !
		if ( _stream->get_eof() ) break;
	}

	if ( ! toparse.empty() )
	{
		parse(toparse);
	}

	_stream->go_to_end();
	_bytesLoaded = _stream->get_position();
	if ( _bytesTotal !=  _bytesLoaded )
	{
		log_error("Size of stream variables were loaded from advertised to be %d bytes long, while turned out to be only %d bytes long",
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
	_completed(false)
{
	if ( ! _stream.get() )
	{
		throw NetworkException();
	}
}

LoadVariablesThread::LoadVariablesThread(const URL& url)
	:
	_stream(StreamProvider::getDefaultInstance().getStream(url)),
	_completed(false)
{
	if ( ! _stream.get() )
	{
		throw NetworkException();
	}
}

} // namespace gnash 
