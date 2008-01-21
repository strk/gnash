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

#ifndef __NETSTATS_H__
#define __NETSTATS_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

//include all types plus i/o
#include <boost/date_time/posix_time/posix_time.hpp>

namespace cygnal 
{

class NetStats {
public:
    NetStats();
    ~NetStats();
    typedef enum {
        NO_CODEC,
        OGG,
        THEORA,
        MP3,
        MPEG4,
        FLV,
        VP6,
        VP7
    } codec_e;
    typedef enum {
        NO_FILETYPE,
        RTMP,
        RTMPT,
        RTMPTS,
        FLASH6,
        FLASH7,
        FLASH8,
        FLASH9,
        AUDIO,
        VIDEO
    } filetypes_e;
    // start the clock counting down
    boost::posix_time::ptime startClock();
    // stop the clock from counting down
    boost::posix_time::ptime stopClock();
    
    // Accessors to set to the private data
    void setStartTime(boost::posix_time::ptime x) { _starttime = x; };
    void setStopTime(boost::posix_time::ptime x) { _stoptime = x; };
    void setBytes(int x) { _bytes = x; };
    void setCodec(codec_e x) { _codec = x; };
    void setFileType(filetypes_e x) { _type = x; };
    // Accumulate the byts transferred
    int addBytes(int x) { _bytes += x; return _bytes; };
    
    // Accessors to get to the private data
    int getBytes() { return _bytes; };
    codec_e getCodec() { return _codec; };
    filetypes_e getFileType() { return _type; };
    boost::posix_time::ptime getStartTime() { return _starttime; };
    boost::posix_time::ptime getStopTime() { return _stoptime; };
    boost::posix_time::time_duration getTimeSpan() { return _stoptime - _starttime; };
    NetStats &operator = (NetStats &stats);
private:
    boost::posix_time::ptime _starttime;
    boost::posix_time::ptime _stoptime;
    int                      _bytes;
    codec_e                  _codec;
    filetypes_e              _type;
};
 
} // end of cygnal namespace

#endif // __NETSTATS_H__

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
