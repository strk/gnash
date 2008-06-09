// MediaHandler.cpp:  Default MediaHandler implementation, for Gnash.
//
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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


#include "MediaHandler.h"
#include "FLVParser.h"
#include "IOChannel.h"
#include "log.h"

namespace gnash {
namespace media {

std::auto_ptr<MediaHandler> MediaHandler::_handler;

/* public static */
bool
MediaHandler::isFLV(IOChannel& stream)
{
	char head[4] = {0, 0, 0, 0};
	stream.seek(0);
	size_t actuallyRead = stream.read_bytes(head, 3);
	stream.seek(0);

	if (actuallyRead < 3)
	{
		log_error(_("MediaHandler::isFLV: Could not read 3 bytes from input stream"));
		return false;
	}

	if (std::string(head) != "FLV") return false;
	return true;
}

std::auto_ptr<MediaParser>
MediaHandler::createMediaParser(std::auto_ptr<IOChannel> stream)
{
	std::auto_ptr<MediaParser> parser;

	if ( ! isFLV(*stream) )
	{
		log_error(_("MediaHandler::createMediaParser: only FLV input is supported by this MediaHandler"));
		return parser;
	}

	parser.reset( new FLVParser(stream) );
	assert(! stream.get() ); // TODO: when ownership will be transferred...

	return parser;
}

} // end of gnash::media namespace
} // end of gnash namespace

