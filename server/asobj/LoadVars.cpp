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
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "LoadVars.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException in ensureLoadVars
#include "as_function.h" // for calling event handlers
#include "as_environment.h" // for setting up a fn_call
#include "as_value.h" // for setting up a fn_call
#include "StreamProvider.h"
#include "URL.h"
#include "gnash.h" // for get_base_url
#include "tu_file.h"
#include "timers.h"
#include "VM.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp> 
#include <memory>
#include <list>

namespace gnash {

static void loadvars_addrequestheader(const fn_call& fn);
static void loadvars_decode(const fn_call& fn);
static void loadvars_load(const fn_call& fn);
static void loadvars_send(const fn_call& fn);
static void loadvars_sendandload(const fn_call& fn);
static void loadvars_tostring(const fn_call& fn);
static void loadvars_ctor(const fn_call& fn);
//static as_object* getLoadVarsInterface();
//static void attachLoadVarsInterface(as_object& o);

class LoadRequest
{
public:
	typedef map<string, string> ValuesMap;

	LoadRequest(const URL& url)
		:
		_stream(StreamProvider::getDefaultInstance().getStream(url)),
		_completed(false)
	{
	}

	LoadRequest(const URL& url, const std::string& postdata)
		:
		_stream(StreamProvider::getDefaultInstance().getStream(url, postdata)),
		_completed(false)
	{
	}

	LoadRequest(const LoadRequest& lr)
		:
		_stream(const_cast<LoadRequest&>(lr)._stream),
		_completed(false)
	{
	}

	ValuesMap& getValues()
	{
		return _vals;
	}

	void process()
	{
		assert(!_thread.get());
		_thread.reset( new boost::thread(boost::bind(LoadRequest::execLoadingThread, this)) );
	}

	bool inProgress()
	{
		return ( _thread.get() != NULL );
	}

	/// Mutex-protected inspector for thread completion
	bool completed()
	{
		boost::mutex::scoped_lock lock(_mutex);
		if (  _completed && _thread.get() )
		{
			_thread->join();
		}
		return _completed;
	}

	/// Mutex-protected mutator for thread completion
	void setCompleted()
	{
		boost::mutex::scoped_lock lock(_mutex);
		_completed = true;
		_thread.reset();
	}

	/// Since I haven't found a way to pass boost::thread 
	/// constructor a non-static function, this is here to
	/// workaround that limitation (in either boost or more
	/// likely my own knowledge of it)
	static void execLoadingThread(LoadRequest* ptr)
	{
		//log_msg("LoadVars loading thread started");
		ptr->completeLoad();
		//log_msg("LoadVars loading thread completed");
	}

	size_t getBytesLoaded() const {
		return _bytesLoaded;
	}

	size_t getBytesTotal() const {
		return _bytesTotal;
	}


private:

	/// Load all data from the _stream input.
	//
	/// This function should be run by a separate thread.
	///
	void completeLoad()
	{
		using std::string;

		// TODO: how to set _bytesTotal ?

		// this is going to override any previous setting,
		// better do this inside a subclass (in a separate thread)
		_bytesLoaded = 0;

		//log_msg("completeLoad called");

		string toparse;

		size_t CHUNK_SIZE = 1024;
		char *buf = new char[CHUNK_SIZE];
		unsigned int parsedLines = 0;
		while ( size_t read = _stream->read_bytes(buf, CHUNK_SIZE) )
		{
			// TODO: use read_string ?
			string chunk(buf, read);
			toparse += chunk;

			//log_msg("toparse: %s", toparse.c_str());

			// parse remainder
			size_t lastamp = toparse.rfind('&');
			if ( lastamp != string::npos )
			{
				string parseable = toparse.substr(0, lastamp);
				//log_msg("parseable: %s", parseable.c_str());
				parse(parseable);
				//log_msg("Parsed %d vals", parsed);
				toparse = toparse.substr(lastamp+1);
				++parsedLines;
			}

			_bytesLoaded += read;
			//dispatchDataEvent();

			// found newline, discard anything before that
			if ( strchr(buf, '\n') )
			{
				if ( parsedLines ) break;
				else toparse.clear();
			}

			// eof, get out !
			if ( _stream->get_eof() ) break;
		}

		if ( ! toparse.empty() )
		{
			parse(toparse);
		}

		_stream->go_to_end();
		_bytesLoaded = _stream->get_position();
		_bytesTotal = _bytesLoaded;

		//dispatchLoadEvent();
		delete[] buf;
		setCompleted();
	}


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
		using std::map;
		using std::string;

		URL::parse_querystring(str, _vals);

		return _vals.size();
	}

	size_t _bytesLoaded;

	size_t _bytesTotal;

	std::auto_ptr<tu_file> _stream;

	std::auto_ptr<boost::thread> _thread;

	ValuesMap _vals;

	bool _completed;

	boost::mutex _mutex;
};

/// LoadVars ActionScript class
//
class LoadVars: public as_object
{

public:

	/// @param env
	/// 	Environment to use for event handlers calls
	///
	LoadVars(as_environment* env);

	~LoadVars();

	/// Load data from given URL
	//
	/// Actually adds a arequest for the load.
	/// The loader thread will only be started
	/// later.
	///
	void load(const std::string& url);

	static as_object* getLoadVarsInterface();

	static void attachLoadVarsInterface(as_object& o);

	// override from as_object ?
	//const char* get_text_value() const { return "LoadVars"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
	
	size_t getBytesLoaded() const {
		return _bytesLoaded;
	}

	size_t getBytesTotal() const {
		return _bytesTotal;
	}

	// Retur number of completed loads
	unsigned int loaded() const {
		return _loaded;
	}

private:

	/// Return true if a load is currently in progress.
	//
	/// NOTE: doesn't lock the _loadRequestsMutex !
	///
	bool isLoading() const
	{
		return _currentLoad != _loadRequests.end();
	}

	/// Check for completed loading threads, fire
	/// new threads if needed.
	void checkLoads();

	/// \brief
	/// Add a load request to the queue, processing it
	/// if no other loads are in progress.
	///
	/// @param postdata
	///	URL-encoded post data. NULL for no post.
	///
	void addLoadRequest(const std::string& urlstr, const char* postdata=NULL);

	/// Process a completed load
	size_t processLoaded(LoadRequest& lr);

	/// Dispatch load event, if any
	void dispatchLoadEvent();

	/// Dispatch data event, if any
	void dispatchDataEvent();

	void setLoadHandler(as_function* fn) {
		_onLoad = fn;
	}

	as_function* getLoadHandler() {
		return _onLoad.get();
	}

	as_function* getDataHandler() {
		return _onData.get();
	}

	void setDataHandler(as_function* fn) {
		_onData = fn;
	}

	static void checkLoads_wrapper(const fn_call& fn);

	static void loaded_getset(const fn_call& fn);

	static void onData_getset(const fn_call& fn);

	static void onLoad_getset(const fn_call& fn);

	static void getBytesLoaded_method(const fn_call& fn);

	static void getBytesTotal_method(const fn_call& fn);

	boost::intrusive_ptr<as_function> _onLoad;

	boost::intrusive_ptr<as_function> _onData;

	as_environment* _env;

	size_t _bytesTotal;

	size_t _bytesLoaded;

	/// List of load requests
	typedef std::list<LoadRequest> LoadRequests;

	/// Load requests queue
	//
	/// Scheduling loads are needed because LoadVars
	/// exposes a getBytesLoaded() and getBytesTotal()
	/// which prevent parallel loads to properly work
	/// (ie: values from *which* load should be reported?)
	///
	LoadRequests _loadRequests;

	/// The load currently in progress.
	//
	/// When _currentLoad == _loadRequests.end()
	/// no load is in progress.
	///
	LoadRequests::iterator _currentLoad;

	std::auto_ptr<tu_file> _stream;

	unsigned int _loadCheckerTimer;

	/// Number of clompleted loads
	unsigned int _loaded;
};

LoadVars::LoadVars(as_environment* env)
		:
		as_object(getLoadVarsInterface()),
		_env(env),
		_bytesTotal(0),
		_bytesLoaded(0),
		_loadRequests(),
		_currentLoad(_loadRequests.end()),
		_loaded(0)
{
	//log_msg("LoadVars %p created", this);
}

LoadVars::~LoadVars()
{
	//log_msg("Deleting LoadVars %p", this);
}

void
LoadVars::checkLoads()
{
	/// Process a completed load if any
	if ( isLoading() && _currentLoad->completed() )
	{
		processLoaded(*_currentLoad);
		_loadRequests.pop_front();
		_currentLoad = _loadRequests.end();
	}

	if ( ! isLoading() )
	{
		if ( ! _loadRequests.empty() )
		{
			_currentLoad = _loadRequests.begin();
			_currentLoad->process();
		}
		else
		{
			VM::get().getRoot().clear_interval_timer(_loadCheckerTimer);
		}
	}
}

void
LoadVars::attachLoadVarsInterface(as_object& o)
{
	o.init_member("addRequestHeader", &loadvars_addrequestheader);
	o.init_member("decode", &loadvars_decode);
	o.init_member("getBytesLoaded", &LoadVars::getBytesLoaded_method);
	o.init_member("getBytesTotal", &LoadVars::getBytesTotal_method);
	o.init_member("load", &loadvars_load);
	o.init_member("send", &loadvars_send);
	o.init_member("sendAndLoad", &loadvars_sendandload);
	o.init_member("toString", &loadvars_tostring);

	boost::intrusive_ptr<builtin_function> gettersetter;

	gettersetter = new builtin_function(&LoadVars::onLoad_getset, NULL);
	o.init_property("onLoad", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&LoadVars::onData_getset, NULL);
	o.init_property("onData", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&LoadVars::loaded_getset, NULL);
	o.init_property("loaded", *gettersetter, *gettersetter);
}

as_object*
LoadVars::getLoadVarsInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object();
		attachLoadVarsInterface(*o);
	}
	return o.get();
}

/*private*/
void
LoadVars::dispatchDataEvent()
{
	if ( ! _onData ) return;
	
	//log_msg("Calling _onData func");
	// This would be the function calls "context"
	// will likely be the same to all events
	as_value ret;
	fn_call fn(&ret, this, _env, 0, 0);

	_onData->call(fn);
}

/* private */
void
LoadVars::dispatchLoadEvent()
{
	if ( ! _onLoad ) return;
	
	//log_msg("Calling _onLoad func");
	// This would be the function calls "context"
	// will likely be the same to all events
	as_value ret;
	fn_call fn(&ret, this, _env, 0, 0);

	_onLoad->call(fn);
}

/* private */
size_t
LoadVars::processLoaded(LoadRequest& lr)
{
	typedef LoadRequest::ValuesMap ValuesMap;
	using std::string;

	ValuesMap& vals = lr.getValues();
	for  (ValuesMap::iterator it=vals.begin(), itEnd=vals.end();
			it != itEnd; ++it)
	{
		set_member(it->first, as_value(it->second.c_str()));
		//log_msg("Setting %s == %s", it->first.c_str(), it->second.c_str());
	}

	_bytesLoaded = lr.getBytesLoaded();
	_bytesTotal = lr.getBytesTotal();
	++_loaded;

	dispatchLoadEvent();

	return vals.size();
}

void
LoadVars::addLoadRequest(const std::string& urlstr, const char* postdata)
{
	if ( _loadRequests.empty() )
	{
		//log_msg("addLoadRequest(): new requests, starting timer");

		using boost::intrusive_ptr;
		intrusive_ptr<builtin_function> loadsChecker = new builtin_function(
			&LoadVars::checkLoads_wrapper, NULL);
		Timer timer; timer.setInterval(*loadsChecker, 50, this, _env);
		_loadCheckerTimer = VM::get().getRoot().add_interval_timer(timer);
	}

	URL url(urlstr, get_base_url());
	if ( postdata ) {
		_loadRequests.insert( _loadRequests.end(), LoadRequest(url, postdata) );
	} else {
		_loadRequests.insert( _loadRequests.end(), LoadRequest(url) );
	}
}

void
LoadVars::load(const std::string& urlstr)
{
	addLoadRequest(urlstr);
}

static LoadVars*
ensureLoadVars(as_object* obj)
{
	LoadVars* ret = dynamic_cast<LoadVars*>(obj);
	if ( ! ret )
	{
		throw ActionException("builtin method or gettersetter for LoadVars objects called against non-LoadVars instance");
	}
	return ret;
}

/* private static */
void
LoadVars::onLoad_getset(const fn_call& fn)
{
	LoadVars* ptr = ensureLoadVars(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
		as_function* h = ptr->getLoadHandler();
		if ( h ) fn.result->set_as_function(h);
		else fn.result->set_undefined();
	}
	else // setter
	{
		as_function* h = fn.arg(0).to_as_function();
		if ( h ) ptr->setLoadHandler(h);
	}
}

/* private static */
void
LoadVars::checkLoads_wrapper(const fn_call& fn)
{

	LoadVars* ptr = ensureLoadVars(fn.this_ptr);
	ptr->checkLoads();

}

/* private static */
void
LoadVars::onData_getset(const fn_call& fn)
{

	LoadVars* ptr = ensureLoadVars(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
		as_function* h = ptr->getDataHandler();
		if ( h ) fn.result->set_as_function(h);
		else fn.result->set_undefined();
	}
	else // setter
	{
		as_function* h = fn.arg(0).to_as_function();
		if ( h ) ptr->setDataHandler(h);
	}
}

/* private static */
void
LoadVars::loaded_getset(const fn_call& fn)
{

	LoadVars* ptr = ensureLoadVars(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
		fn.result->set_bool(ptr->loaded() > 0);
	}
	else // setter
	{
		IF_VERBOSE_ASCODING_ERRORS(
			log_msg("Tried to set LoadVars.loaded, which is a read-only property");
		);
		return;
	}
}


static void
loadvars_addrequestheader(const fn_call& fn)
{
	LoadVars* ptr = ensureLoadVars(fn.this_ptr);
	UNUSED(ptr);
	log_error("%s: unimplemented", __FUNCTION__);
}

static void
loadvars_decode(const fn_call& fn)
{
	LoadVars* ptr = ensureLoadVars(fn.this_ptr);
	UNUSED(ptr);
	log_error("%s: unimplemented", __FUNCTION__);
}

void
LoadVars::getBytesLoaded_method(const fn_call& fn)
{
	LoadVars* ptr = ensureLoadVars(fn.this_ptr);
	fn.result->set_int(ptr->getBytesLoaded());
}

void
LoadVars::getBytesTotal_method(const fn_call& fn)
{
	LoadVars* ptr = ensureLoadVars(fn.this_ptr);
	fn.result->set_int(ptr->getBytesTotal());
}

static void
loadvars_load(const fn_call& fn)
{
	LoadVars* obj = ensureLoadVars(fn.this_ptr);

	if ( fn.nargs < 1 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("LoadVars.load() requires at least one argument");
		);
		fn.result->set_bool(false);
		return;
	}

	std::string urlstr = fn.arg(0).to_std_string();
	if ( urlstr.empty() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("LoadVars.load(): invalid empty url ");
		);
		fn.result->set_bool(false);
		return;
	}

	obj->load(urlstr);
	fn.result->set_bool(true);
	
}

static void
loadvars_send(const fn_call& fn)
{
	LoadVars* ptr = ensureLoadVars(fn.this_ptr);
	UNUSED(ptr);
	log_error("%s: unimplemented", __FUNCTION__);
}

static void
loadvars_sendandload(const fn_call& fn)
{
	LoadVars* ptr = ensureLoadVars(fn.this_ptr);
	UNUSED(ptr);
	log_error("%s: unimplemented", __FUNCTION__);
}

static void
loadvars_tostring(const fn_call& fn)
{
	LoadVars* ptr = ensureLoadVars(fn.this_ptr);
	UNUSED(ptr);
	log_error("%s: unimplemented", __FUNCTION__);
}

static void
loadvars_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new LoadVars(fn.env);
	
	fn.result->set_as_object(obj.get()); // will keep alive
}

// extern (used by Global.cpp)
void
loadvars_class_init(as_object& global)
{
	// This is going to be the global LoadVars "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&loadvars_ctor, LoadVars::getLoadVarsInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		LoadVars::attachLoadVarsInterface(*cl);
		     
	}

	// Register _global.LoadVars
	global.init_member("LoadVars", cl.get());

}


} // end of gnash namespace

