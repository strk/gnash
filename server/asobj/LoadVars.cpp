// LoadVars.cpp:  ActionScript "LoadVars" class (HTTP variables), for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "LoadVars.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "as_function.h" // for calling event handlers
#include "as_environment.h" // for setting up a fn_call
#include "as_value.h" // for setting up a fn_call
#include "StreamProvider.h"
#include "URL.h"
#include "gnash.h" // for get_base_url
#include "tu_file.h"
#include "timers.h"
#include "VM.h"
#include "LoadVariablesThread.h"
#include "Object.h" // for getObjectInterface

#include <list>

namespace gnash {

static as_value loadvars_addrequestheader(const fn_call& fn);
static as_value loadvars_decode(const fn_call& fn);
static as_value loadvars_load(const fn_call& fn);
static as_value loadvars_send(const fn_call& fn);
static as_value loadvars_sendandload(const fn_call& fn);
static as_value loadvars_tostring(const fn_call& fn);
static as_value loadvars_ctor(const fn_call& fn);
//static as_object* getLoadVarsInterface();
//static void attachLoadVarsInterface(as_object& o);

//--------------------------------------------

/// LoadVars ActionScript class
//
class LoadVars: public as_object
{

public:

	/// @param env
	/// 	Environment to use for event handlers calls
	///
	LoadVars();

	~LoadVars();

	/// Load data from given URL
	//
	/// Actually adds a arequest for the load.
	/// The loader thread will only be started
	/// later.
	///
	void load(const std::string& url);

	/// \brief
	/// Load data from given URL into the given target, sending
	/// enumerable properties of this object using either POST or
	/// GET method.
	//
	/// Actually adds a request for the load.
	/// The loader thread will only be started later.
	///
	/// @param urlstr
	///	The base url string to post to (and load from).
	///
	/// @param target
	///	The LoadVars that will process a completed load, thus
	///	getting the members from the response attached.
	///
	/// @param post
	///	If false, variables will be sent using the GET method.
	///	If true (the default), variables will be sent using POST.
	///
	void sendAndLoad(const std::string& urlstr, LoadVars& target, bool post=true);

	static as_object* getLoadVarsInterface();

	static void attachLoadVarsInterface(as_object& o);

	// override from as_object ?
	//std::string get_text_value() const { return "LoadVars"; }

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

protected:

#ifdef GNASH_USE_GC
	/// Mark all reachable resources, for the GC
	//
	/// Reachable resources are:
	///	- onLoad event handler (_onLoad)
	///	- onData event handler (_onData)
	///	- associated environment (_env)
	///
	virtual void markReachableResources() const
	{

		if ( _onLoad ) _onLoad->setReachable();

		if ( _onData ) _onData->setReachable();

		_env.markReachableResources();

		// Invoke generic as_object marker
		markAsObjectReachable();
	}

#endif // GNASH_USE_GC

private:

	/// Forbid copy
	LoadVars(const LoadVars&)
		:
		as_object()
	{ abort(); }

	/// Forbid assignment
	LoadVars& operator=(const LoadVars&) { abort(); return *this; }

	/// Return enumerable property pairs in url-encoded form
	//
	/// TODO: move up to as_object and make public,
	///       for use by loadVariables ?
	///
	std::string getURLEncodedProperties();

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
	void addLoadVariablesThread(const std::string& urlstr, const char* postdata=NULL);

	/// Process a completed load
	size_t processLoaded(LoadVariablesThread& lr);

	/// Dispatch load event, if any
	as_value dispatchLoadEvent();

	/// Dispatch data event, if any
	as_value dispatchDataEvent();

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

	static as_value checkLoads_wrapper(const fn_call& fn);

	static as_value loaded_get(const fn_call& fn);

	static as_value onData_getset(const fn_call& fn);

	static as_value onLoad_getset(const fn_call& fn);

	static as_value getBytesLoaded_method(const fn_call& fn);

	static as_value getBytesTotal_method(const fn_call& fn);

	boost::intrusive_ptr<as_function> _onLoad;

	boost::intrusive_ptr<as_function> _onData;

	as_environment _env;

	size_t _bytesTotal;

	size_t _bytesLoaded;

	/// List of load requests
	typedef std::list<LoadVariablesThread*> LoadVariablesThreads;
	//typedef boost::ptr_list<LoadVariablesThread> LoadVariablesThreads;

	/// Load requests queue
	//
	/// Scheduling loads are needed because LoadVars
	/// exposes a getBytesLoaded() and getBytesTotal()
	/// which prevent parallel loads to properly work
	/// (ie: values from *which* load should be reported?)
	///
	LoadVariablesThreads _loadRequests;

	/// The load currently in progress.
	//
	/// When _currentLoad == _loadRequests.end()
	/// no load is in progress.
	///
	LoadVariablesThreads::iterator _currentLoad;

	unsigned int _loadCheckerTimer;

	/// Number of clompleted loads
	unsigned int _loaded;
};

LoadVars::LoadVars()
		:
		as_object(getLoadVarsInterface()),
		_env(),
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
	for ( LoadVariablesThreads::iterator i=_loadRequests.begin(), e=_loadRequests.end();
			i!=e; ++i)
	{
		delete *i;
	}
}

void
LoadVars::checkLoads()
{
	/// Process a completed load if any
	if ( isLoading() && (*_currentLoad)->completed() )
	{
		processLoaded(*(*_currentLoad));
		_loadRequests.pop_front();
		_currentLoad = _loadRequests.end();
	}

	if ( ! isLoading() )
	{
		if ( ! _loadRequests.empty() )
		{
			_currentLoad = _loadRequests.begin();
			(*_currentLoad)->process();
		}
		else
		{
			getVM().getRoot().clear_interval_timer(_loadCheckerTimer);
		}
	}
}

void
LoadVars::attachLoadVarsInterface(as_object& o)
{
	o.init_member("addRequestHeader", new builtin_function(loadvars_addrequestheader));
	o.init_member("decode", new builtin_function(loadvars_decode));
	o.init_member("getBytesLoaded", new builtin_function(LoadVars::getBytesLoaded_method));
	o.init_member("getBytesTotal", new builtin_function(LoadVars::getBytesTotal_method));
	o.init_member("load", new builtin_function(loadvars_load));
	o.init_member("send", new builtin_function(loadvars_send));
	o.init_member("sendAndLoad", new builtin_function(loadvars_sendandload));
	o.init_member("toString", new builtin_function(loadvars_tostring));

	boost::intrusive_ptr<builtin_function> gettersetter;

	gettersetter = new builtin_function(&LoadVars::onLoad_getset, NULL);
	o.init_property("onLoad", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&LoadVars::onData_getset, NULL);
	o.init_property("onData", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&LoadVars::loaded_get, NULL);
	o.init_readonly_property("loaded", *gettersetter);
}

as_object*
LoadVars::getLoadVarsInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object(getObjectInterface());
		attachLoadVarsInterface(*o);
	}
	return o.get();
}

/*private*/
as_value
LoadVars::dispatchDataEvent()
{
	if ( ! _onData ) return as_value();
	
	//log_msg("Calling _onData func");
	// This would be the function calls "context"
	// will likely be the same to all events
	fn_call fn(this, &_env, 0, 0);

	return _onData->call(fn);
}

/* private */
as_value
LoadVars::dispatchLoadEvent()
{
	if ( ! _onLoad ) return as_value();
	
	//log_msg("Calling _onLoad func");
	// This would be the function calls "context"
	// will likely be the same to all events
	fn_call fn(this, &_env, 0, 0);

	return _onLoad->call(fn);
}

/* private */
size_t
LoadVars::processLoaded(LoadVariablesThread& lr)
{
	typedef LoadVariablesThread::ValuesMap ValuesMap;
	using std::string;

	string_table& st = getVM().getStringTable();

	ValuesMap& vals = lr.getValues();
	for  (ValuesMap::iterator it=vals.begin(), itEnd=vals.end();
			it != itEnd; ++it)
	{
		set_member(st.find(it->first), as_value(it->second.c_str()));
		//log_msg("Setting %s == %s", it->first.c_str(), it->second.c_str());
	}

	_bytesLoaded = lr.getBytesLoaded();
	_bytesTotal = lr.getBytesTotal();
	++_loaded;

	dispatchLoadEvent();

	return vals.size();
}

void
LoadVars::addLoadVariablesThread(const std::string& urlstr, const char* postdata)
{
	if ( _loadRequests.empty() )
	{
		//log_msg("addLoadVariablesThread(): new requests, starting timer");

		using boost::intrusive_ptr;
		intrusive_ptr<builtin_function> loadsChecker = new builtin_function(
			&LoadVars::checkLoads_wrapper, NULL);
		std::auto_ptr<Timer> timer(new Timer);
		timer->setInterval(*loadsChecker, 50, this);
		_loadCheckerTimer = getVM().getRoot().add_interval_timer(timer, true);
	}

	URL url(urlstr, get_base_url());

	std::auto_ptr<LoadVariablesThread> newThread;

	try
	{
		if ( postdata ) newThread.reset( new LoadVariablesThread(url, postdata) );
		else newThread.reset( new LoadVariablesThread(url) );

		_loadRequests.insert( _loadRequests.end(), newThread.release() );
	}
	catch (NetworkException&)
	{
		log_error(_("Could not load variables from %s"), url.str().c_str());
	}
}

void
LoadVars::load(const std::string& urlstr)
{
	addLoadVariablesThread(urlstr);
}

std::string
LoadVars::getURLEncodedProperties()
{
	// TODO: optimize this function... 

	using std::string;

	string qstring;

	typedef std::map<std::string, std::string> VarMap;
	VarMap vars;

	//return qstring;

	// TODO: it seems that calling enumerateProperties(vars) here
	//       somehow corrupts the stack !
	enumerateProperties(vars);

	for (VarMap::iterator it=vars.begin(), itEnd=vars.end();
			it != itEnd; ++it)
	{
		string var = it->first; URL::encode(var);
		string val = it->second; URL::encode(val);
		if ( it != vars.begin() ) qstring += string("&");
		qstring += var + string("=") + val;
	}

	return qstring;
}

void
LoadVars::sendAndLoad(const std::string& urlstr, LoadVars& target, bool post)
{
	std::string querystring = getURLEncodedProperties();
	if ( post ) {
		target.addLoadVariablesThread(urlstr, querystring.c_str());
	} else {
		std::string url = urlstr + "?" + querystring;
		target.addLoadVariablesThread(urlstr);
	}
}

/* private static */
as_value
LoadVars::onLoad_getset(const fn_call& fn)
{
	boost::intrusive_ptr<LoadVars> ptr = ensureType<LoadVars>(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
		as_function* h = ptr->getLoadHandler();
		if ( h ) return as_value(h);
		else return as_value();
	}
	else // setter
	{
		as_function* h = fn.arg(0).to_as_function();
		if ( h ) ptr->setLoadHandler(h);
	}
	return as_value();
}

/* private static */
as_value
LoadVars::checkLoads_wrapper(const fn_call& fn)
{
	boost::intrusive_ptr<LoadVars> ptr = ensureType<LoadVars>(fn.this_ptr);
	ptr->checkLoads();
	return as_value();
}

/* private static */
as_value
LoadVars::onData_getset(const fn_call& fn)
{

	boost::intrusive_ptr<LoadVars> ptr = ensureType<LoadVars>(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
		as_function* h = ptr->getDataHandler();
		if ( h ) return as_value(h);
		else return as_value();
	}
	else // setter
	{
		as_function* h = fn.arg(0).to_as_function();
		if ( h ) ptr->setDataHandler(h);
	}
	return as_value();
}

/* private static */
as_value
LoadVars::loaded_get(const fn_call& fn)
{

	boost::intrusive_ptr<LoadVars> ptr = ensureType<LoadVars>(fn.this_ptr);

	return as_value(ptr->loaded() > 0);
}

static as_value
loadvars_addrequestheader(const fn_call& fn)
{
	boost::intrusive_ptr<LoadVars> ptr = ensureType<LoadVars>(fn.this_ptr);
	UNUSED(ptr);
	log_unimpl (__FUNCTION__);
	return as_value(); 
}

static as_value
loadvars_decode(const fn_call& fn)
{
	boost::intrusive_ptr<LoadVars> ptr = ensureType<LoadVars>(fn.this_ptr);
	UNUSED(ptr);
	log_unimpl (__FUNCTION__);
	return as_value(); 
}

as_value
LoadVars::getBytesLoaded_method(const fn_call& fn)
{
	boost::intrusive_ptr<LoadVars> ptr = ensureType<LoadVars>(fn.this_ptr);
	return as_value(ptr->getBytesLoaded());
}

as_value
LoadVars::getBytesTotal_method(const fn_call& fn)
{
	boost::intrusive_ptr<LoadVars> ptr = ensureType<LoadVars>(fn.this_ptr);
	return as_value(ptr->getBytesTotal());
}

static as_value
loadvars_load(const fn_call& fn)
{
	boost::intrusive_ptr<LoadVars> obj = ensureType<LoadVars>(fn.this_ptr);

	if ( fn.nargs < 1 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("LoadVars.load() requires at least one argument"));
		);
		return as_value(false);
	}

	const std::string& urlstr = fn.arg(0).to_string();
	if ( urlstr.empty() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("LoadVars.load(): invalid empty url"));
		);
		return as_value(false);
	}

	obj->load(urlstr);
	return as_value(true);
	
}

static as_value
loadvars_send(const fn_call& fn)
{
	boost::intrusive_ptr<LoadVars> ptr = ensureType<LoadVars>(fn.this_ptr);
	UNUSED(ptr);
	log_unimpl (__FUNCTION__);
	return as_value(); 
}

static as_value
loadvars_sendandload(const fn_call& fn)
{
	boost::intrusive_ptr<LoadVars> ptr = ensureType<LoadVars>(fn.this_ptr);

	if ( fn.nargs < 2 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("LoadVars.sendAndLoad() requires at least two arguments"));
		);
		return as_value(false);
	}

	const std::string& urlstr = fn.arg(0).to_string();
	if ( urlstr.empty() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("LoadVars.sendAndLoad(): invalid empty url"));
		);
		return as_value(false);
	}

	boost::intrusive_ptr<LoadVars> target = boost::dynamic_pointer_cast<LoadVars>(fn.arg(1).to_object());
	if ( ! target )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("LoadVars.sendAndLoad(): invalid target (must be a LoadVars object)"));
		);
		return as_value(false);
	}

	// Post by default, override by ActionScript third argument
	bool post = true;
	if ( fn.nargs > 2 && fn.arg(2).to_string() == "GET" ) post = false;

	//log_msg("LoadVars.sendAndLoad(%s, %p) called, and returning TRUE", urlstr.c_str(), target.get());

	ptr->sendAndLoad(urlstr, *target, post);
	return as_value(true);
}

static as_value
loadvars_tostring(const fn_call& fn)
{
	boost::intrusive_ptr<LoadVars> ptr = ensureType<LoadVars>(fn.this_ptr);
	UNUSED(ptr);
	log_unimpl (__FUNCTION__);
	return as_value(); 
}

static as_value
loadvars_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new LoadVars();
	
	return as_value(obj.get()); // will keep alive
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
