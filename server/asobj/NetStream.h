// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
//
//

/*  $Id: NetStream.h,v 1.28 2007/05/04 15:21:00 strk Exp $ */

#ifndef __NETSTREAM_H__
#define __NETSTREAM_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#include "impl.h"
#include "video_stream_instance.h"
#include "NetConnection.h"

// Forward declarations
namespace gnash {
	//class NetConnection;
}

namespace gnash {
  

class NetStream : public as_object {

protected:
	
	/// Status codes used for notifications
	enum StatusCode {
	
		// Internal status, not a valid ActionScript value
		invalidStatus,

		/// NetStream.Buffer.Empty (level: status)
		bufferEmpty,

		/// NetStream.Buffer.Full (level: status)
		bufferFull,

		/// NetStream.Buffer.Flush (level: status)
		bufferFlush,

		/// NetStream.Play.Start (level: status)
		playStart,

		/// NetStream.Play.Stop  (level: status)
		playStop,

		/// NetStream.Seek.Notify  (level: status)
		seekNotify,

		/// NetStream.Play.StreamNotFound (level: error)
		streamNotFound,

		/// NetStream.Seek.InvalidTime (level: error)
		invalidTime
	};

	boost::intrusive_ptr<NetConnection> _netCon;

	/// Set stream status.
	//
	/// Valid statuses are:
	///
	/// Status level:
	///  - NetStream.Buffer.Empty
	///  - NetStream.Buffer.Full
	///  - NetStream.Buffer.Flush
	///  - NetStream.Play.Start
	///  - NetStream.Play.Stop 
	///  - NetStream.Seek.Notify 
	///
	/// Error level:
	///  - NetStream.Play.StreamNotFound
	///  - NetStream.Seek.InvalidTime
	///
	void setStatus(StatusCode code);

	/// \brief
	/// Call any onStatus event handler passing it
	/// any queued status change, see _statusQueue
	//
	void processStatusNotifications();

	// The actionscript enviroment for the AS callbacks
	as_environment* m_env;

public:

	NetStream();

	virtual ~NetStream(){}

	virtual void close(){}

	virtual void pause(int /*mode*/){}

	virtual int play(const char* /*source*/){ log_error("FFMPEG or Gstreamer is needed to play video"); return 0; }

	virtual void seek(double /*pos*/){}

	virtual void setBufferTime(double /*time*/){}

	virtual void setNetCon(boost::intrusive_ptr<NetConnection> nc)
	{
		_netCon = nc;
	}

	virtual image::image_base* get_video(){ return NULL; }

	virtual int64_t time() { return 0; }

	virtual long bytesLoaded() { return 0; }

	virtual long bytesTotal() { return 0; }

	virtual bool playing()
	{
		return false;
	}

	virtual void advance(){}

	virtual bool newFrameReady() { return false; }

	void setEnvironment(as_environment* env)
	{
		assert(env);
		m_env = env;
	}

private:

	typedef std::vector<StatusCode> StatusQueue;

	/// List of status messages to be processed
	StatusQueue _statusQueue;

	/// Mutex protecting _statusQueue
	boost::mutex statusMutex;

	/// Last status code (to avoid consecutively notifying the same event)
	StatusCode _lastStatus;

	/// Get 'status' (first) and 'level' (second) strings for given status code
	//
	/// The two members of the pair are ensured to be not-NULL
	/// Any invalid code, out of bound or explicitly invalid (invalidCode) 
	/// returns two empty C strings.
	///
	std::pair<const char*, const char*> getStatusCodeInfo(StatusCode code);

	/// Return a newly allocated information object for the given status
	boost::intrusive_ptr<as_object> getStatusObject(StatusCode code);

};


// Initialize the global NetStream class
void netstream_class_init(as_object& global);

} // end of gnash namespace

// __NETSTREAM_H__
#endif

