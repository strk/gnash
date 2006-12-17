// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <boost/thread/mutex.hpp>
#include <string>
#include <list>
#include <iostream>

#include "log.h"
#include "netstats.h"
#include "statistics.h"

using namespace gnash;
using namespace std;

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
}

static boost::mutex io_mutex;

// The string versions of the codec, used for debugging. If you add
// another enum type to codec_e, you have to add the string
// representation here or you'll get the wrong output.
const char *codec_names[] = {
    "NO_CODEC",
    "Ogg",
    "Theora",
    "MP3",
    "MPEG4",
    "FLV",
    "VP6",
    "VP7"
};

// The string versions of the codec, used for debugging. If you add
// another enum type to filetypes_e, you have to add the string
// representation here or you'll get the wrong output.
const char *filetype_names[] = {
        "NO_FILETYPE",
        "RTMP",
        "RTMPT",
        "RTMPTS",
        "FLASH5",
        "FLASH6",
        "FLASH7",
        "FLASH8",
        "FLASH9",
        "AUDIO",
        "VIDEO"
};

namespace cygnal 
{

Statistics::Statistics() {
}

Statistics::~Statistics() {
    dump();
}

float
Statistics::getFPS() {
    
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
    st->setCodec(getCodec());
    st->setFileType(getFileType());
    
    boost::mutex::scoped_lock lock(io_mutex);
    _netstats.push_back(st);
    
    return _netstats.size();
}

void
Statistics::dump() {   
    boost::mutex::scoped_lock lock(io_mutex);
    list<NetStats *>::iterator it;

    for (it = _netstats.begin(); it != _netstats.end(); it++) {
        NetStats *stats = (*it);
        if (stats->getFileType() <= VIDEO) {
            dbglogfile << "Stream type is: " << filetype_names[stats->getFileType()] << endl;
        }
        if (((stats->getFileType() == VIDEO) || (stats->getFileType() == AUDIO)) &&
            stats->getCodec() <= VP7) {
            dbglogfile << "Stream codec is: " << codec_names[stats->getCodec()];
        }
        dbglogfile << stats->getBytes() << " bytes were transfered in "
                   << to_simple_string(stats->getTimeSpan()).c_str()
                   << " seconds." << endl;
    }
}

} // end of cygnal namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
