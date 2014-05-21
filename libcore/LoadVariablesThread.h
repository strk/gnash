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


#ifndef GNASH_LOADVARIABLESTHREAD_H
#define GNASH_LOADVARIABLESTHREAD_H

#include <string>
#include <map>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <functional> 

#include "URL.h" // for inlines

namespace gnash {
    class StreamProvider;
    class IOChannel;
}

namespace gnash {

// Exception thrown by LoadVariablesThread constructor if unable to connect
// to the stream input.
class NetworkException {};

/// A manager for loadVariable requests
//
/// Provides services for starting a "load and parse" thread, checking
/// its status and getting a parsed variables structure back when done.
///
class LoadVariablesThread
{
public:
	typedef std::map<std::string, std::string> ValuesMap;

	/// Construct a LoadVariablesThread opening a stream for the given URL
	//
	/// Throws a NetworkException if unable.
	///
	/// @param url
	///	URL to post to and fetch from
	///
	LoadVariablesThread(const StreamProvider& sp, const URL& url);

	/// \brief
	/// Construct a LoadVariablesThread opening a stream for the given URL,
	/// posting the given url-encoded data if using HTTP.
	//
	/// Throws a NetworkException if unable.
	///
	/// @param url
	///	URL to post to and fetch from
	///
	/// @param postdata
	///	Url-encoded post data.
	///
	LoadVariablesThread(const StreamProvider& sp, const URL& url,
            const std::string& postdata);

	/// Destroy the LoadVariablesThread, joining the thread if spawned
	~LoadVariablesThread();

	/// Return the name,value map parsed out of the loaded stream
	ValuesMap& getValues()
	{
		return _vals;
	}

	/// Start the load and parse thread
	void process()
	{
		assert(!_thread.get());
		assert(_stream.get());
		_thread.reset(new boost::thread(
                std::bind(LoadVariablesThread::execLoadingThread, this)));
	}

	/// Cancel a download in progress
	//
	/// Locks _mutex
	///
	void cancel();

	/// Return true if loading/parsing is in progress
	bool inProgress()
	{
		// TODO: should we mutex-protect this ?
		return ( _thread.get() != nullptr );
	}

	/// Mutex-protected inspector for thread completion
	//
	/// Only call this method from the same thread that
	/// also called process(), as the thread will be joined
	/// if it completed.
	///
	bool completed()
	{
		boost::mutex::scoped_lock lock(_mutex);
		if (  _completed && _thread.get() )
		{
			_thread->join();
			_thread.reset();
		}
		return _completed;
	}

	size_t getBytesLoaded() const
	{
		// TODO: should we mutex-protect this ?
		return _bytesLoaded;
	}

	size_t getBytesTotal() const
	{
		// TODO: should we mutex-protect this ?
		return _bytesTotal;
	}


private:

	/// Prevent copy
	LoadVariablesThread& operator==(const LoadVariablesThread&); 
	LoadVariablesThread(const LoadVariablesThread&); 

	/// Since I haven't found a way to pass boost::thread 
	/// constructor a non-static function, this is here to
	/// workaround that limitation (in either boost or more
	/// likely my own knowledge of it)
	static void execLoadingThread(LoadVariablesThread* ptr)
	{
		//log_debug("LoadVars loading thread started");
		ptr->completeLoad();
		//log_debug("LoadVars loading thread completed");
	}


	/// Mutex-protected mutator for thread completion
	void setCompleted()
	{
		boost::mutex::scoped_lock lock(_mutex);
		_completed = true;
		//log_debug("Completed");
	}


	/// Load all data from the _stream input.
	//
	/// This function should be run by a separate thread.
	///
	void completeLoad();

	/// Parse an url-encoded query string
	//
	/// Variables in the string will be added as properties
	/// of this object.
	///
	/// @param querystring
	///	An url-encoded query string.
	///	The string will be parsed using URL::parse_querystring
	///
	/// @return the number of variables found in the string
	///
	size_t parse(const std::string& str)
	{
		URL::parse_querystring(str, _vals);
		return _vals.size();
	}

	/// Check if download cancel was requested
	//
	/// Locks _mutex
	///
	bool cancelRequested();

	size_t _bytesLoaded;

	size_t _bytesTotal;

        std::unique_ptr<IOChannel> _stream;

        std::unique_ptr<boost::thread> _thread;

	ValuesMap _vals;

	bool _completed;

	bool _canceled;

	boost::mutex _mutex;
};

} // namespace gnash

#endif // GNASH_LOADVARIABLESTHREAD_H
