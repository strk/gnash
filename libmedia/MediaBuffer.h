// MediaBuffer.h: Buffer where decoded audio and video data is stored.
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


#ifndef __MEDIABUFFER_H__
#define __MEDIABUFFER_H__

#include "image.h"

#include <queue>
#include <boost/thread/mutex.hpp>
#include <boost/cstdint.hpp> // for boost::?int??_t

namespace gnash {
namespace media {


/// This class is used to store decoded audio data
/// while it is in the MediaBuffer.
class raw_audiodata_t
{
public:
	raw_audiodata_t():
	m_size(0),
	m_data(NULL),
	m_ptr(NULL),
	m_pts(0)
	{
	}

	~raw_audiodata_t()
	{
		if (m_data) delete [] m_data;
	}

//	int m_stream_index;

	/// Size of the data stored
	boost::uint32_t m_size;

	/// Pointer to the data. The data is owned by this class.
	boost::uint8_t* m_data;

	/// Pointer to where the data should be used from. Used with audio, since we
	/// don't always use all the data.
	boost::uint8_t* m_ptr;

	/// Timestamp in millisec
	boost::uint32_t m_pts;
};

/// This class is used to store decoded video data
/// while it is in the MediaBuffer.
class raw_videodata_t
{
public:
	raw_videodata_t():
	image(NULL),
	timestamp(0)
	{
	}

	~raw_videodata_t()
	{
	}

	/// Pointer to the data. The data is owned by this class.
	std::auto_ptr<image::ImageBase> image;

	/// Timestamp in millisec
	boost::uint32_t timestamp;
};

/// Threadsafe elements-owning queue
//
/// This class is a threadsafe queue, using std:queue and locking.
/// It is used to store decoded audio and video data which are waiting to be "played"
/// Elements of the queue are owned by instances of this class.
///
class MediaBuffer
{
public:
	MediaBuffer()
			:
	_bufferTime(100) // Deafault value is 100 milliseconds
	{
	}

	~MediaBuffer()
	{
		flush();
	}

	/// Returns the size if the audio queue. Locks.
	//
	/// @return the size of the audio queue
	///
	size_t audioSize() {
		boost::mutex::scoped_lock lock(_mutex);
		return audioQueue.size();
	}

	/// Returns the size if the video queue. Locks.
	//
	/// @return the size of the video queue
	///
	size_t videoSize() {
		boost::mutex::scoped_lock lock(_mutex);
		return videoQueue.size();
	}

	/// Pushes an element to the audio queue. Locks.
	//
	/// @param member
	/// The element to be pushed unto the audio queue.
	///
	void pushAudio(raw_audiodata_t* member) {
		boost::mutex::scoped_lock lock(_mutex);
		audioQueue.push(member);
	}

	/// Pushes an element to the video queue. Locks.
	//
	/// @param member
	/// The element to be pushed unto the video queue.
	///
	void pushVideo(raw_videodata_t* member) {
		boost::mutex::scoped_lock lock(_mutex);
		videoQueue.push(member);
	}

	/// Returns a pointer to the first element on the audio queue. Locks.
	//
	/// If no elements are available this function returns NULL.
	///
	/// @return a pointer to the first element on the audio queue, NULL if queue is empty.
	///
	raw_audiodata_t* audioFront() {
		boost::mutex::scoped_lock lock(_mutex);
		if (audioQueue.empty()) return NULL;
		return audioQueue.front();
	}

	/// Returns a pointer to the first element on the video queue. Locks.
	//
	/// If no elements are available this function returns NULL.
	///
	/// @return a pointer to the first element on the video queue, NULL if queue is empty.
	///
	raw_videodata_t* videoFront() {
		boost::mutex::scoped_lock lock(_mutex);
		if (videoQueue.empty()) return NULL;
		return videoQueue.front();
	}

	/// Pops the first element from the audio queue. Locks.
	//
	/// If no elements are available this function is a noop. 
	///
	void audioPop() {
		boost::mutex::scoped_lock lock(_mutex);
		if (!audioQueue.empty()) audioQueue.pop();
	}

	/// Pops the first element from the video queue. Locks.
	//
	/// If no elements are available this function is a noop. 
	///
	void videoPop() {
		boost::mutex::scoped_lock lock(_mutex);
		if (!videoQueue.empty()) videoQueue.pop();
	}

	/// Fluses the buffer/queues
	void flush() {
		boost::mutex::scoped_lock lock(_mutex);
		while (!videoQueue.empty() > 0)
		{
			delete videoQueue.front();
			videoQueue.pop();
		}

		while (!audioQueue.empty() > 0)
		{
			delete audioQueue.front();
			audioQueue.pop();
		}
	}
	
	/// Sets the size of the buffer in milliseconds. Locks.
	//
	/// @param size
	/// The size of the buffer.
	///
	void setBufferTime(boost::uint32_t size) {
		boost::mutex::scoped_lock lock(_mutex);
		_bufferTime = size;
	}

	/// Gets the requested size of the buffer in milliseconds. Locks.
	//
	/// @return the requested size of the buffer.
	///
	boost::uint32_t getReqBufferTime() {
		boost::mutex::scoped_lock lock(_mutex);
		return _bufferTime;
	}

	/// Gets the real size of the buffer in milliseconds. The size of
	/// the audio and video buffer is compared and the biggest is returned. Locks.
	//
	/// @return the current size of the buffer in milliseconds.
	///
	boost::uint32_t getBufferTime() {
		boost::mutex::scoped_lock lock(_mutex);
		return calcBufferTime();
	}

	/// Checks if the contents of the buffer span a timeframe larger than
	/// than the requested size. Locks.
	//
	/// @return the real size of the buffer in milliseconds.
	///
	bool isFull() {
		boost::mutex::scoped_lock lock(_mutex);
		bool ret = (calcBufferTime() >= _bufferTime);
		return ret;
	}

private:

	/// Calculates the real size of the buffer in milliseconds. The size of
	/// the audio and video buffer is compared and the biggest is returned.
	//
	/// @return the real size of the buffer in milliseconds.
	///
	boost::uint32_t calcBufferTime() {
		boost::uint32_t size = 0;

		// Get the size of audio buffer
		if (!audioQueue.empty()) {
			size = audioQueue.back()->m_pts - audioQueue.front()->m_pts;
		}

		// Get the size of video buffer, and use that if it is bigger than
		// the vaule from the audio buffer.
		if (!videoQueue.empty()) {
			boost::uint32_t vSize = videoQueue.back()->timestamp - videoQueue.front()->timestamp;
			if (vSize > size) size = vSize;
		}
		return size;
	}


	/// Mutex used for locking
	boost::mutex _mutex;

	/// The queues of audio and video data.
	std::queue <raw_audiodata_t*> audioQueue;
	std::queue <raw_videodata_t*> videoQueue;

	/// The requested size of the buffer in milliseconds
	boost::uint32_t _bufferTime;
};

} // gnash.media namespace 
} // gnash namespace

#endif // __MEDIABUFFER_H__
