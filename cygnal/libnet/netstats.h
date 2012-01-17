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

#ifndef __NETSTATS_H__
#define __NETSTATS_H__

//include all types plus i/o
#include <boost/date_time/posix_time/posix_time.hpp>

// This is what the ActionScript 'Client' class returns:
//
// bytes_in		Total number of bytes received.
// bytes_out		Total number of bytes sent.
// msg_in		Total number of RTMP messages received.
// msg_out		Total number of RTMP messages sent.
// msg_dropped		Total number of dropped RTMP messages.
// ping_rtt		Length of time the client takes to respond to a ping message.
// audio_queue_msgs	Current number of audio messages in the queue waiting to be delivered to the client.
// video_queue_msgs	Current number of video messages in the queue waiting to be delivered to the client.
// so_queue_msgs	Current number of shared object messages in the queue waiting to be delivered to the client.
// data_queue_msgs	Current number of data messages in the queue waiting to be delivered to the client.
// dropped_audio_msgs	Number of audio messages that were dropped.
// dropped_video_msgs	Number of video messages that were dropped.
// audio_queue_bytes	Total size of all audio messages (in bytes) in the queue waiting to be delivered to the client.
// video_queue_bytes	Total size of all video messages (in bytes) in the queue waiting to be delivered to the client.
// so_queue_bytes	Total size of all shared object messages (in bytes) in the queue waiting to be delivered to the client.
// data_queue_bytes	Total size of all data messages (in bytes) in the queue waiting to be delivered to the client.
// dropped_audio_bytes	Total size of all audio messages (in bytes) that were dropped.
// dropped_video_bytes	Total size of all video messages (in bytes) that were dropped.
// bw_out		Current upstream (client to server) bandwidth for this client.
// bw_in		Current downstream (server to client) bandwidth for this client.
// client_id		A unique ID issued by the server for this client.
//
// samples are taken every 3 seconds, or the interval supplied in Client::setInterval()

namespace gnash 
{

class NetStats {
public:
    NetStats();
    virtual ~NetStats();
    typedef enum {
        NO_CODEC,
        OGG,
        THEORA,
	DIRAC,
	SNOW,
        MP3,
        MPEG4,
	H264,
	H263,
        FLV,
        VP6,
        VP7
    } codec_e;
    typedef enum {
        NO_FILETYPE,
        HTTP,
        RTMP,
        RTMPT,
        RTMPTS,
        SWF,
        SWF6,
        SWF7,
        SWF8,
        SWF9,
        AUDIO,
        VIDEO
    } filetypes_e;
    // This is what the ActionScript 'Client' class returns:
    typedef struct {
        int bytes_in;		// Total number of bytes received. 
        int bytes_out;		// Total number of bytes sent.
        int msg_in;		// Total number of RTMP messages received.
        int msg_out;		// Total number of RTMP messages sent.
        int msg_dropped;	// Total number of dropped RTMP messages.
        int ping_rtt;		// Length of time the client takes to respond to a ping message.
        int audio_queue_msgs;	// Current number of audio messages in the queue waiting to be delivered to the client.
        int video_queue_msgs;	// Current number of video messages in the queue waiting to be delivered to the client.
        int so_queue_msgs;	// Current number of shared object messages in the queue waiting to be delivered to the client.
        int data_queue_msgs;	// Current number of data messages in the queue waiting to be delivered to the client.
        int dropped_audio_msgs;	// Number of audio messages that were dropped.
        int dropped_video_msgs;	// Number of video messages that were dropped.
        int audio_queue_bytes;	// Total size of all audio messages (in bytes) in the queue waiting to be delivered to the client.
        int video_queue_bytes;	// Total size of all video messages (in bytes) in the queue waiting to be delivered to the client.
        int so_queue_bytes;	// Total size of all shared object messages (in bytes) in the queue waiting to be delivered to the client.
        int data_queue_bytes;	// Total size of all data messages (in bytes) in the queue waiting to be delivered to the client.
        int dropped_audio_bytes;// Total size of all audio messages (in bytes) that were dropped.
        int dropped_video_bytes;// Total size of all video messages (in bytes) that were dropped.
        int bw_out;		// Current upstream (client to server) bandwidth for this client.
        int bw_in;		// Current downstream (server to client) bandwidth for this client.
        int client_id;		// A unique ID issued by the server for this client.
    } netstats_t;
    // start the clock counting down
    boost::posix_time::ptime startClock();
    // stop the clock from counting down
    boost::posix_time::ptime stopClock();
    
    // Accessors to set to the private data
    void setStartTime(boost::posix_time::ptime x) { _starttime = x; };
    void setStopTime(boost::posix_time::ptime x) { _stoptime = x; };
    void setBytes(int x) { _bytes = x; };
    void setFileType(filetypes_e x) { _type = x; };
    // Accumulate the byts transferred
    int addBytes(int x) { _bytes += x; return _bytes; };
    
    // Accessors to get to the private data
    int getBytes() { return _bytes; };
    filetypes_e getFileType() { return _type; };
    boost::posix_time::ptime getStartTime() { return _starttime; };
    boost::posix_time::ptime getStopTime() { return _stoptime; };
    boost::posix_time::time_duration getTimeSpan() { return _stoptime - _starttime; };
    NetStats &operator = (NetStats &stats);
private:
    boost::posix_time::ptime _starttime;
    boost::posix_time::ptime _stoptime;
    int                      _bytes;
    filetypes_e              _type;
};
 
} // end of gnash namespace

#endif // __NETSTATS_H__

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
