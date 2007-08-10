// Timeline.h:  Holds immutable data for a sprite/movie timeline, for Gnash.
//
//   Copyright (C) 2007 Free Software Foundation, Inc.
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

#ifndef GNASH_TIMELINE_H
#define GNASH_TIMELINE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vector>
#include <set>
#include <boost/thread/mutex.hpp>

namespace gnash
{


/// \brief
/// Holds the immutable data for a sprite/move timeline, as read from
/// as SWF stream.
//
/// Timeline data consists in a set of depths supposed to contain an instance
/// for each frame in the sprite/movie.
///
/// This information is extracted while parsing the SWF so it might be
/// incomplete for a frame until parsing of that frame completes. 
///
/// Multiple threads can use instances of this class.
/// In particular, the loader/parser thread will use the mutators and the movie
/// advancement/actions executors thread will use the inspectors.
/// For this reason access to the data is mutex-protected. Locking info are
/// specified for each locking function.
///
class Timeline 
{

public:

	friend std::ostream& operator<< (std::ostream& os, const Timeline& t);

	/// Construct a Timeline instance 
	Timeline()
	{
		_frameDepths.push_back(DepthSet());
	}

	/// Destroy a Timeline instance
	~Timeline() {}

	/// \brief
	/// Reserve memory for up to the given number
	/// of frames.
	//
	/// You don't need to call this function except
	/// for improving performance when number of frames
	/// is known in advance. This is expected in well-formed
	/// SWF streams.
	///
	/// Locks the mutex
	/// (likely superfluos, as tipically called right after creation)
	///
	void reserve(size_t nframes)
	{
		boost::mutex::scoped_lock lock(_frameDepthsMutex);

		// reserve one more element for the end sentinel
		_frameDepths.reserve(nframes+1);
	}

	/// \brief
	/// Close data for current frame and move cursor to next one,
	/// allocating it if needed
	//
	/// Locks the mutex to avoid race conditions in which the parser
	/// is closing a frame and the VM is inspecting a frame (either the
	/// same or a different one). See getFrameDepths().
	///
	void closeFrame()
	{
		boost::mutex::scoped_lock lock(_frameDepthsMutex);

		// Copy depth set from previous frame
		_frameDepths.push_back(_frameDepths.back());
	}

	/// Add a timeline depth to the current frame 
	//
	/// @param depth
	///	Depth of an instance placed by PlaceObject* tag.
	///	Assumed to be in the static zone (an assertion would fail otherwise).
	///
	/// Does NOT lock the mutex as this function is intended to be 
	/// called by a single thread (the loader/parser).
	///
	/// @param depth
	///	Depth of an instance placed by PlaceObject* tag.
	///	Assumed to be in the static zone (an assertion would fail otherwise).
	///
	void addDepth(int depth)
	{
		assert(depth < 0 && depth >= -16384); // or should be > -16384 (not ==?)

		_frameDepths.back().insert(depth);
	}

	/// Remove a timeline depth from the current frame 
	//
	/// Does NOT lock the mutex as this function is intended to be 
	/// called by a single thread (the loader/parser).
	///
	/// @param depth
	///	Depth of an instance removed by RemoveObject* tag, or by PlaceObject* tag
	///	with replace/move semantic.
	///	Assumed to be in the static zone (an assertion would fail otherwise).
	///
	void removeDepth(int depth)
	{
		assert(depth < 0 && depth >= -16384); // or should be > -16384 (not ==?)

		_frameDepths.back().erase(depth);
	}

	/// \brief
	/// Get the set of timeline depths supposed to contain
	/// an instance in the given frame (0-based)
	//
	/// PRECONDITIONS:
	///	- Definition for target frame was completed.
	///	  See closeFrame().
	///
	/// Locks the mutex to avoid race conditions in which the parser
	/// is closing a frame and the VM is inspecting a frame (either the
	/// same or a different one). See closeFrame().
	///
	/// @param frameno
	///	0-based frame number we want to inspect.
	///	Assumed to be in the static zone (an assertion would fail otherwise).
	///
	/// @param depths
	///	A vector to copy all frame depths to.
	///	The depths are copied to both provide thread-safety
	///	and to allow future optimization of memory.
	///	In particular we might construct frame depths on demand, by
	///	scanning a list of insertion/deletion events, rather then
	///	keeping a set of depths for *each* frame.
	///	
	///	
	///
	void getFrameDepths(size_t frameno, std::vector<int>& depths)
	{
		boost::mutex::scoped_lock lock(_frameDepthsMutex);

		assert(frameno < _frameDepths.size());

		DepthSet& from = _frameDepths[frameno];
		depths.assign(from.begin(), from.end());
	}

private:

	/// A set of depths in the static zone
	typedef std::set<int> DepthSet;

	/// A vector of depth sets (one for each frame)
	typedef std::vector<DepthSet> FrameDepths;

	/// Return the number of frames completely defined
	//
	/// Doesn't lock the mutex.
	///
	size_t closedFrames() const
	{
		assert(!_frameDepths.empty());
		return _frameDepths.size()-1;
	}

	/// Return the 0-based index of the frame currently being defined
	//
	/// Doesn't lock the mutex.
	///
	size_t currentFrame() const
	{
		return _frameDepths.size()-1;
	}


	/// A map of depths for each frame
	//
	/// Access to this container is mutex-protected
	/// for thread safety. See _frameDepthsMutex.
	///
	FrameDepths _frameDepths;

	/// Mutex protecting access to the _frameDepths member
	boost::mutex _frameDepthsMutex;

};

inline std::ostream&
operator<< (std::ostream& os, const Timeline& t)
{
	for (Timeline::FrameDepths::const_iterator it=t._frameDepths.begin(), itEnd=t._frameDepths.end(); it!=itEnd; ++it)
	{
		os << "[";
		for (Timeline::DepthSet::const_iterator di=it->begin(), de=it->end(); di!=de; ++di)
		{
			if ( di != it->begin() ) os << ",";
			os << *di;
		}
		os << "]";
		os << "\n";
	}
	return os;
}

} // end of namespace gnash

#endif // GNASH_SPRITE_H
