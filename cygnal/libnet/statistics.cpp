// statistics.cpp:  Network performance stats for Cygnal, for Gnash.
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

#include <boost/thread/mutex.hpp>
#include <string>
#include <list>
#include <iostream>

#include "log.h"
#include "netstats.h"
#include "statistics.h"

static boost::mutex io_mutex;

// The string versions of the codec, used for debugging. If you add
// another enum type to codec_e, you have to add the string
// representation here or you'll get the wrong output.
const char *codec_names[] = {
    "NO_CODEC",
    "Ogg",
    "Theora",
    "Dirac",
    "Snow",
    "MP3",
    "MPEG4",
    "H264",
    "H263",
    "FLV",
    "VP6",
    "VP7"
};

// The string versions of the file type, used for debugging. If you add
// another enum type to filetypes_e, you have to add the string
// representation here or you'll get the wrong output.
const char *filetype_names[] = {
        "NO_FILETYPE",
	"HTTP",
        "RTMP",
        "RTMPT",
        "RTMPTS",
        "SWF",
        "SWF5",
        "SWF6",
        "SWF7",
        "SWF8",
        "SWF9",
        "AUDIO",
        "VIDEO"
};

namespace gnash 
{

Statistics::Statistics() {
}

Statistics::~Statistics() {
    dump();
}

float
Statistics::getFPS() {
	return 0.0; // TODO: FIXME !
}

int
Statistics::getBitRate() {

    return (getStartTime() - getStopTime()).seconds() / getBytes();
}

int
Statistics::addStats() {
    NetStats *st = new NetStats;

    st->setStartTime(getStartTime());
    st->setStopTime(getStopTime());
    st->setBytes(getBytes());
    st->setFileType(getFileType());
    
    boost::mutex::scoped_lock lock(io_mutex);
    _netstats.push_back(st);
    
    return _netstats.size();
}

void
Statistics::dump() {   
    boost::mutex::scoped_lock lock(io_mutex);
    std::list<NetStats *>::iterator it;

    for (it = _netstats.begin(); it != _netstats.end(); it++) {
        NetStats *stats = (*it);
        if (stats->getFileType() <= VIDEO) {
            log_debug (_("Stream type is: %s"), filetype_names[stats->getFileType()]);
        }
//         if (((stats->getFileType() == VIDEO) || (stats->getFileType() == AUDIO)) &&
//             stats->getCodec() <= VP7) {
//             log_debug (_("Stream codec is: %s"), codec_names[stats->getCodec()]);
//         }
        log_debug (_("%d bytes were transfered in %s seconds"),
		 stats->getBytes(),
                 to_simple_string(stats->getTimeSpan()).c_str());
    }
}

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
