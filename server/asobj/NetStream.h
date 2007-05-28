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

/*  $Id: NetStream.h,v 1.35 2007/05/28 15:24:59 tgc Exp $ */

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
#include "FLVParser.h"

// Forward declarations
namespace gnash {
	//class NetConnection;
}

namespace gnash {
  

/// NetStream ActionScript class
//
/// This class is responsible for handlign external
/// media files. Provides interfaces for playback control.
///
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

	// The size of the buffer in milliseconds
	uint32_t m_bufferTime;

	// The video outputformat
	int m_videoFrameFormat;

	// Are a new frame ready to be returned?
	volatile bool m_newFrameReady;

	// Mutex to insure we don't corrupt the image
	boost::mutex image_mutex;

	// Are the playing loop running or not
	volatile bool m_go;

	// The image/videoframe which is given to the renderer
	image::image_base* m_imageframe;

	// paused or not
	volatile bool m_pause;

	// The video URL
	std::string url;

	// The homegrown parser we use for FLV
	FLVParser* m_parser;

	// Are we playing a FLV?
	bool m_isFLV;

	// The handler which is invoked on status change
	boost::intrusive_ptr<as_function> m_statusHandler;

	// should we start when the FLVParser has buffered/parsed enough frames,
	// so that the differens between the current frames timestamp (0 at the 
	// beginning) and the last parseable frames timestamp i bigger than 
	// m_bufferTime.
	bool m_start_onbuffer;

	// The position in the inputfile, only used when not playing a FLV
	long inputPos;

public:

	NetStream();

	virtual ~NetStream(){}

	/// Closes the video session and frees all ressources used for decoding
	/// except the FLV-parser (this might not be correct).
	virtual void close(){}

	/// Pauses/starts the playback of the media played by the current instance
	//
	/// @param mode
	///	Defines what mode to put the instance in. 
	/// -1 : toogle mode
	/// 0 : switch to pause
	/// 1 : switch to play
	///
	virtual void pause(int /*mode*/){}

	/// Starts the playback of the media
	//
	/// @param source
	///	Defines what file to play
	///
	/// @return 1 (true) on success, 0 (false) on failure
	///
	virtual int play(const std::string& /*source*/){ log_error(_("FFMPEG or Gstreamer is needed to play video")); return 0; }

	/// Seek in the media played by the current instance
	//
	/// @param position
	///	Defines in seconds where to seek to
	///
	virtual void seek(double /*pos*/){}

	/// Tells where the playhead currently is
	//
	/// @return The time in seconds of the current playhead position
	///
	virtual int64_t time() { return 0; }

	/// Prepare for the rendering of next SWF frame
	virtual void advance(){}

	/// Sets the NetConnection needed to access external files
	//
	/// @param netconnection
	///
	void setNetCon(boost::intrusive_ptr<NetConnection> nc)
	{
		_netCon = nc;
	}

	/// Sets the AS Enviroment needed for eventhandlers
	//
	/// @param enviroment
	///
	void setEnvironment(as_environment* env)
	{
		assert(env);
		m_env = env;
	}

	/// Tells whether we're playing or not
	//
	/// @return true if playing, false if not.
	///
	inline bool playing()
	{
		return m_go;
	}

	/// Specifies how long to buffer data before starting to display the stream.
	//
	/// @param time
	/// The time in seconds that should be buffered.
	///
	void setBufferTime(uint32_t time);

	/// Returns what the buffer has been set to. (100 miliseconds is default)
	//
	/// @return The size of the buffer in seconds.
	///
	uint32_t bufferTime();

	/// Returns the number of bytes loaded of the media file
	//
	/// @return the number of bytes loaded of the media file
	///
	long bytesLoaded();

	/// Returns the total number of bytes (size) of the media file
	//
	/// @return the total number of bytes (size) of the media file
	///
	long bytesTotal();

	/// Returns the number of second of the media file that is buffered and 
	/// yet to be played
	//
	/// @return Returns the number of second of the media file that is 
	/// buffered and yet to be played
	long bufferLength();

	/// Tells us if there is a new video frame ready
	//
	/// @return true if a frame is ready, false if not
	bool newFrameReady();

	/// Returns the video frame closest to current cursor. See time().
	//
	/// @return a image containing the video frame, NULL if none were ready
	image::image_base* get_video();

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

