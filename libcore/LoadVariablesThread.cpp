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

//#define DEBUG_LOAD_VARIABLES 1

namespace gnash {

// static
LoadVariablesThread::ValuesMap
LoadVariablesThread::completeLoad(IOChannel* varstream,
   std::atomic<bool>& canceled)
{
    std::unique_ptr<IOChannel> stream(varstream);
#ifdef DEBUG_LOAD_VARIABLES
    log_debug("completeLoad called");
#endif
        ValuesMap parseResult;

	int bytesLoaded = 0;
	int bytesTotal = stream->size();

	std::string toparse;

	const size_t chunkSize = 1024;
	std::unique_ptr<char[]> buf(new char[chunkSize]);
	unsigned int parsedLines = 0;
	// TODO: use read_string ?
	while ( size_t bytesRead = stream->read(buf.get(), chunkSize) )
	{
#ifdef DEBUG_LOAD_VARIABLES
            log_debug("Read %u bytes", bytesRead);
#endif

		if ( bytesLoaded )
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
                        URL::parse_querystring(toparse, parseResult);
			toparse = toparse.substr(lastamp+1);
#ifdef DEBUG_LOAD_VARIABLES
			log_debug("toparse nextline: %s", toparse);
#endif
			++parsedLines;
		}

		bytesLoaded += bytesRead;

		// eof, get out !
		if ( stream->eof() ) break;

		if ( canceled.load() ) {
                    log_debug("Cancelling LoadVariables download thread...");
			stream.reset();
			return parseResult;
		}
	}

	if ( ! toparse.empty() ) {
            URL::parse_querystring(toparse, parseResult);
	}

	try {
		stream->go_to_end();
	}
        catch (IOException& ex) {
        log_error(_("Stream couldn't seek to end: %s"), ex.what());
	}
	
        bytesLoaded = stream->tell();
	if ( bytesTotal !=  bytesLoaded ) {
            log_error(_("Size of 'variables' stream advertised to be %d bytes,"
                          " but turned out to be %d bytes."),
			bytesTotal, bytesLoaded);
	}

	//dispatchLoadEvent();

        return parseResult;
}

LoadVariablesThread::LoadVariablesThread(const StreamProvider& sp,
        const URL& url, const std::string& postdata)
	: _canceled(false)
{
    startThread(sp.getStream(url, postdata));
}

LoadVariablesThread::LoadVariablesThread(const StreamProvider& sp,
        const URL& url)
    : _canceled(false)
{
    startThread(sp.getStream(url));
}

void
LoadVariablesThread::startThread(std::unique_ptr<IOChannel> stream)
{
    if (!stream) {
        throw NetworkException();
    }

    _vals = std::async(std::launch::async, completeLoad, stream.release(), 
        std::ref(_canceled));
}

LoadVariablesThread::~LoadVariablesThread()
{
	if ( _vals.valid() )
	{
		_canceled = true;
		_vals.wait();
	}
}


} // namespace gnash 
