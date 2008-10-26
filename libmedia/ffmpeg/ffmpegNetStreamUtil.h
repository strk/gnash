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
#include "dsodefs.h" //For DSOEXPORT

#ifdef USE_FFMPEG // does this make any sense ?
# include "ffmpegHeaders.h"
#endif

#include <queue>
#include <deque>

#include <iconv.h>
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
	DSOEXPORT raw_mediadata_t();

	DSOEXPORT ~raw_mediadata_t();

	int m_stream_index;
	boost::uint32_t m_size;
	boost::uint8_t* m_data;
	boost::uint8_t* m_ptr;
	boost::uint32_t m_pts;	// presentation timestamp in millisec
};


/// Elements-owning queue
//
/// This class is a queue owning its elements and optionally
/// limited in size.
///
template<class T>
class ElementsOwningQueue
{
public:

	/// Construct a queue, limited by given amount of elements (20 by default)
	ElementsOwningQueue(size_t limit=20) 
		:
		_limit(limit)
	{
	}

	// Destroy all elements of the queue. 
	~ElementsOwningQueue()
	{
		clear();
	}

	// Destroy all elements of the queue.
	void clear()
	{
		for (typename container::iterator i=_queue.begin(),
				e=_queue.end();
			i!=e;
			++i)
		{
			delete *i;
		}
		_queue.clear();
	}

	/// Returns the size if the queue. 
	//
	/// @return the size of the queue
	///
	size_t size()
	{
		return _queue.size();
	}

	/// Return true if the queue is empty
	bool empty()
	{
		return _queue.empty();
	}

	/// Return true if the queue is full
	//
	/// The function would never return true
	/// if the queue has no limit..
	///
	bool full()
	{
		if ( ! _limit ) return false;
		return _queue.size() >= _limit;
	}

	/// Set queue limit
	void setLimit(unsigned int limit)
	{
		_limit = limit;
	}

	/// Pushes an element to the queue. 
	//
	/// @param member
	/// The element to be pushed unto the queue.
	///
	/// @return false if the queue has reached its limit, true otherwise.
	///
	bool push(T member)
	{
		if ( _limit && _queue.size() >= _limit ) return false;
		_queue.push_back( member );
		return true;
	}

	/// Returns a pointer to the first element on the queue. 
	//
	/// If no elements are available this function returns NULL.
	///
	/// @return a pointer to the first element on the queue, NULL if queue is empty.
	///
	T front()
	{
		if ( _queue.empty() ) return 0;
		return _queue.front();
	}

	/// Pops the first element from the queue. 
	//
	/// If no elements are available this function is
	/// a noop. 
	///
	void pop()
	{
		if ( ! _queue.empty() ) _queue.pop_front();
	}

private:

	typedef std::deque<T> container;

	/// Limit queue to this number of elements
	unsigned int _limit;

	// Mutex used for locking
	boost::mutex _mutex;

	// The actual queue.
	container _queue;
};


/// This class is used to provide an easy interface to libavcodecs audio resampler.
///
class AudioResampler
{
public:
	DSOEXPORT AudioResampler();

	DSOEXPORT ~AudioResampler();
	
	/// Initializes the resampler
	//
	/// @param ctx
	/// The audio format container.
	///
	/// @return true if resampling is needed, if not false
	///
	DSOEXPORT bool init(AVCodecContext* ctx);
	
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
	DSOEXPORT int resample(
		boost::int16_t* input, boost::int16_t* output, int samples
	);

private:
	// The container of the resample format information.
	ReSampleContext* _context;
};

} // gnash.media namespace 
} // namespace gnash


#endif // FFMPEGNETSTREAMUTIL_H
