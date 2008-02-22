// ffmpegNetStreamUtil.h: Utility classes for use in 
// server/asobj/NetStreamFfmpeg.*
//
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


#ifndef FFMPEGNETSTREAMUTIL_H
#define FFMPEGNETSTREAMUTIL_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "log.h"

#ifdef USE_FFMPEG
extern "C" {
#include <ffmpeg/avcodec.h>
}
#endif

#include <queue>

#include <SDL_audio.h>
#include <boost/cstdint.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

namespace gnash {
namespace media {

class raw_mediadata_t
{
public:
	raw_mediadata_t();

	~raw_mediadata_t();

	int m_stream_index;
	boost::uint32_t m_size;
	boost::uint8_t* m_data;
	boost::uint8_t* m_ptr;
	boost::uint32_t m_pts;	// presentation timestamp in millisec
};


/// Threadsafe elements-owning queue
//
/// This class is a threadsafe queue, using std:queue and locking.
/// It is used to store decoded audio and video data which are waiting to be "played"
/// Elements of the queue are owned by instances of this class.
///
template<class T>
class multithread_queue
{
public:

	multithread_queue() 
	{
	}

	// Destroy all elements of the queue. Locks.
	~multithread_queue()
	{
	  clear();
	}

	// Destroy all elements of the queue. Locks.
	void clear()
	{
	  boost::mutex::scoped_lock lock( _mutex );

	  while ( ! m_queue.empty() ) {
	    T x = m_queue.front();
	    m_queue.pop();
	    delete x;
	  }
	}

	/// Returns the size if the queue. Locks.
	//
	/// @return the size of the queue
	///
	size_t size()
	{
	  boost::mutex::scoped_lock lock( _mutex );

	  size_t n = m_queue.size();

	  return n;
	}

	/// Pushes an element to the queue. Locks.
	//
	/// @param member
	/// The element to be pushed unto the queue.
	///
	/// @return true if queue isn't full and the element was pushed to the queue,
	/// or false if the queue was full, and the element wasn't push unto it.
	///
	bool push(T member)
	{
	  bool rc = false;
	  boost::mutex::scoped_lock lock( _mutex );

	  // We only keep max 20 items in the queue.
	  // If it's "full" the item must wait, see calls to 
	  // this function in read_frame() to see how it is 
	  // done.
	  if ( m_queue.size() < 20 ) {
	    m_queue.push( member );
	    rc = true;
	  }

	  return rc;
	}

	/// Returns a pointer to the first element on the queue. Locks.
	//
	/// If no elements are available this function returns NULL.
	///
	/// @return a pointer to the first element on the queue, NULL if queue is empty.
	///
	T front()
	{
	  boost::mutex::scoped_lock lock( _mutex );
	
	  T member = NULL;

	  if ( ! m_queue.empty() ) {
	    member = m_queue.front();
	  }
  
	  return member;
	}

	/// Pops the first element from the queue. Locks.
	//
	/// If no elements are available this function is
	/// a noop. 
	///
	void pop()
	{
	  boost::mutex::scoped_lock lock( _mutex );

	  if ( ! m_queue.empty() ) {
	    m_queue.pop();
	  }
	}

private:

	// Mutex used for locking
	boost::mutex _mutex;

	// The actual queue.
	std::queue<T> m_queue;
};


/// This class is used to provide an easy interface to libavcodecs audio resampler.
///
class AudioResampler
{
public:
	AudioResampler();

	~AudioResampler();
	
	/// Initializes the resampler
	//
	/// @param ctx
	/// The audio format container.
	///
	/// @return true if resampling is needed, if not false
	///
	bool init(AVCodecContext* ctx);
	
	/// Resamples audio
	//
	/// @param input
	/// A pointer to the audio data that needs resampling
	///
	/// @param output
	/// A pointer to where the resampled output should be placed
	///
	/// @param samples
	/// Number of samples in the audio
	///
	/// @return the number of samples in the output data.
	///
	int resample(
		boost::int16_t* input, boost::int16_t* output, int samples
	);

private:
	// The container of the resample format information.
	ReSampleContext* _context;
};

} // gnash.media namespace 
} // namespace gnash


#endif // FFMPEGNETSTREAMUTIL_H
