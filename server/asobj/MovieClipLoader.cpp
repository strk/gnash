// MovieClipLoader.cpp:  Implementation of ActionScript MovieClipLoader class.
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

#include "tu_config.h"

#include "action.h" // for call_method
#include "as_value.h"
#include "as_object.h" // for inheritance
#include "fn_call.h"
#include "as_function.h"
#include "movie_definition.h"
#include "sprite_instance.h"
#include "character.h" // for loadClip (get_parent)
#include "log.h"
#include "URL.h" // for url parsing
#include "builtin_function.h"

#include <typeinfo> 
#include <string>
#include <set>

namespace gnash {

// Forward declarations
static as_value moviecliploader_loadclip(const fn_call& fn);
static as_value moviecliploader_unloadclip(const fn_call& fn);
static as_value moviecliploader_getprogress(const fn_call& fn);
static as_value moviecliploader_new(const fn_call& fn);
static as_value moviecliploader_addlistener(const fn_call& fn);
static as_value moviecliploader_removelistener(const fn_call& fn);

static void
attachMovieClipLoaderInterface(as_object& o)
{
  	o.init_member("loadClip", new builtin_function(moviecliploader_loadclip));
	o.init_member("unloadClip", new builtin_function(moviecliploader_unloadclip));
	o.init_member("getProgress", new builtin_function(moviecliploader_getprogress));

	o.init_member("addListener", new builtin_function(moviecliploader_addlistener));
	o.init_member("removeListener", new builtin_function(moviecliploader_removelistener));

#if 0
	// Load the default event handlers. These should really never
	// be called directly, as to be useful they are redefined
	// within the SWF script. These get called if there is a problem
	// Setup the event handlers
	o.set_event_handler(event_id::LOAD_INIT, new builtin_function(event_test));
	o.set_event_handler(event_id::LOAD_START, new builtin_function(event_test));
	o.set_event_handler(event_id::LOAD_PROGRESS, new builtin_function(event_test));
	o.set_event_handler(event_id::LOAD_ERROR, new builtin_function(event_test));
#endif
  
}

static as_object*
getMovieClipLoaderInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( o == NULL )
	{
		o = new as_object();
		//log_msg(_("MovieClipLoader interface @ %p"), o.get());
		attachMovieClipLoaderInterface(*o);
	}
	return o.get();
}


// progress info
struct mcl {
	int bytes_loaded;
	int bytes_total;
};


/// Progress object to use as return of MovieClipLoader.getProgress()
struct mcl_as_object : public as_object
{
	struct mcl data;
};

class MovieClipLoader: public as_object
{
public:

	MovieClipLoader();

	~MovieClipLoader();

	struct mcl *getProgress(as_object *ao);

	/// MovieClip
	bool loadClip(const std::string& url, sprite_instance& target);

	void unloadClip(void *);

	/// @todo make an EventDispatcher class for this
	/// @ {
	///

	/// Add an object to the list of event listeners
	//
	/// This function will call add_ref() on the
	/// given object.
	///
	void addListener(boost::intrusive_ptr<as_object> listener);

	void removeListener(boost::intrusive_ptr<as_object> listener);

	/// Invoke any listener for the specified event
	void dispatchEvent(const std::string& eventName, fn_call& fn);

	/// @ }

private:

	typedef std::set< boost::intrusive_ptr<as_object> > Listeners;

	Listeners _listeners;
	bool          _started;
	bool          _completed;
	std::string     _filespec;
	int           _progress;
	bool          _error;
	struct mcl    _mcl;
};

MovieClipLoader::MovieClipLoader()
	:
	as_object(getMovieClipLoaderInterface())
{
	_mcl.bytes_loaded = 0;
	_mcl.bytes_total = 0;  
}

MovieClipLoader::~MovieClipLoader()
{
	GNASH_REPORT_FUNCTION;
}

// progress of the downloaded file(s).
struct mcl *
MovieClipLoader::getProgress(as_object* /*ao*/)
{
  GNASH_REPORT_FUNCTION;

  return &_mcl;
}


bool
MovieClipLoader::loadClip(const std::string& url_str, sprite_instance& target)
{
	// Prepare function call for events...
	as_environment env;
	env.push(as_value(&target));
	fn_call events_call(this, &env, 1, 0);

	URL url(url_str.c_str(), get_base_url());
	
#if GNASH_DEBUG
	log_msg(_(" resolved url: %s"), url.str().c_str());
#endif
			 
	// Call the callback since we've started loading the file
	// TODO: probably we should move this below, after 
	//       the loading thread actually started
	dispatchEvent("onLoadStart", events_call);

	bool ret = target.loadMovie(url);
	if ( ! ret ) 
	{
		// TODO: dispatchEvent("onLoadError", ...)
		return false;
	}


	/// This event must be dispatched when actions
	/// in first frame of loaded clip have been executed.
	///
	/// Since movie_def_impl::create_instance takes
	/// care of this, this should be the correct place
	/// to invoke such an event.
	///
	/// TODO: check if we need to place it before calling
	///       this function though...
	///
	dispatchEvent("onLoadInit", events_call);

	struct mcl *mcl_data = getProgress(&target);

	// the callback since we're done loading the file
	// FIXME: these both probably shouldn't be set to the same value
	//mcl_data->bytes_loaded = stats.st_size;
	//mcl_data->bytes_total = stats.st_size;
	mcl_data->bytes_loaded = 666; // fake values for now
	mcl_data->bytes_total = 666;

	// TODO: dispatchEvent("onLoadProgress", ...)

	log_unimpl (_("FIXME: MovieClipLoader calling onLoadComplete *before* movie has actually been fully loaded (cheating)"));
	dispatchEvent("onLoadComplete", events_call);

	return true;
}

void
MovieClipLoader::unloadClip(void *)
{
  GNASH_REPORT_FUNCTION;
}


void
MovieClipLoader::addListener(boost::intrusive_ptr<as_object> listener)
{
	assert(listener); // caller should check
	_listeners.insert(listener);
}


void
MovieClipLoader::removeListener(boost::intrusive_ptr<as_object> listener)
{
	assert(listener); // caller should check
	Listeners::iterator it = _listeners.find(listener);
	if ( it != _listeners.end() )
	{
		_listeners.erase(it);
	}
}

  
// Callbacks
void
MovieClipLoader::dispatchEvent(const std::string& event, fn_call& fn)
{
	typedef Listeners::iterator iterator;

#if GNASH_DEBUG
	log_msg(_("Dispatching %s event to " SIZET_FMT " listeners"),
		event.c_str(), _listeners.size());
#endif

	for (iterator it=_listeners.begin(), itEnd=_listeners.end();
			it != itEnd;
			++it)
	{
		boost::intrusive_ptr<as_object> listener = *it;
		as_value method;
		if ( ! listener->get_member(event.c_str(), &method) )
		{
#if GNASH_DEBUG
log_msg(_("Listener %p doesn't have an %s event to listen for, skipped"),
	(void*)listener, event.c_str());
#endif
			// this listener doesn't care about this event
			// event
			continue;
		}

#if GNASH_DEBUG
		log_msg(_("Testing call to listener's "
			" %s function"), event.c_str());
#endif

		call_method(method, &fn.env(), fn.this_ptr.get(), fn.nargs, fn.offset());
	}

}

static as_value
moviecliploader_loadclip(const fn_call& fn)
{
	as_value	val, method;

	//log_msg(_("%s: nargs = %d"), __FUNCTION__, fn.nargs);

	boost::intrusive_ptr<MovieClipLoader> ptr = ensureType<MovieClipLoader>(fn.this_ptr);
  
	as_value& url_arg = fn.arg(0);
#if 0 // whatever it is, we'll need a string, the check below would only be worth
      // IF_VERBOSE_MALFORMED_SWF, but I'm not sure it's worth the trouble of
      // checking it, and chances are that the reference player will be trying
      // to convert to string anyway...
	if ( ! url_arg.is_string() )
	{
		log_swferror(_("MovieClipLoader.loadClip() first argument is not a string (%s)"), url_arg.to_string());
		return as_value(false);
		return;
	}
#endif
	std::string str_url = url_arg.to_string(); 

	character* target = fn.env().find_target(fn.arg(1));
	if ( ! target )
	{
		log_error(_("Could not find target %s"), fn.arg(1).to_string().c_str());
		return as_value(false);
	}
	sprite_instance* sprite = dynamic_cast<sprite_instance*>(target);
	if ( ! sprite )
	{
		log_error(_("Target is not a sprite instance (%s)"),
			typeid(*target).name());
		return as_value(false);
	}

#if GNASH_DEBUG
	log_msg(_("load clip: %s, target is: %p\n"),
		str_url.c_str(), (void*)sprite);
#endif

	bool ret = ptr->loadClip(str_url, *sprite);

	return as_value(ret);

}

static as_value
moviecliploader_unloadclip(const fn_call& fn)
{
  const std::string filespec = fn.arg(0).to_string();
  log_unimpl (_("%s: %s"), __PRETTY_FUNCTION__, filespec.c_str());
  return as_value();
}

static as_value
moviecliploader_new(const fn_call& /* fn */)
{

  as_object*	mov_obj = new MovieClipLoader;
  //log_msg(_("MovieClipLoader instance @ %p"), mov_obj);

  return as_value(mov_obj); // will store in a boost::intrusive_ptr
}

// Invoked every time the loading content is written to disk during
// the loading process.
static as_value
moviecliploader_getprogress(const fn_call& fn)
{
  //log_msg(_("%s: nargs = %d"), __FUNCTION__, nargs);
  
  boost::intrusive_ptr<MovieClipLoader> ptr = ensureType<MovieClipLoader>(fn.this_ptr);
  
  boost::intrusive_ptr<as_object> target = fn.arg(0).to_object();
  
  struct mcl *mcl_data = ptr->getProgress(target.get());

  boost::intrusive_ptr<mcl_as_object> mcl_obj ( new mcl_as_object );

  mcl_obj->init_member("bytesLoaded", mcl_data->bytes_loaded);
  mcl_obj->init_member("bytesTotal",  mcl_data->bytes_total);
  
  return as_value(mcl_obj.get()); // will store in a boost::intrusive_ptr
}

static as_value
moviecliploader_addlistener(const fn_call& fn)
{
	boost::intrusive_ptr<MovieClipLoader> mcl = ensureType<MovieClipLoader>(fn.this_ptr);
  
	boost::intrusive_ptr<as_object> listener = fn.arg(0).to_object();
	if ( ! listener )
	{
		IF_VERBOSE_ASCODING_ERRORS (
		log_aserror(_("Listener given to MovieClipLoader.addListener() is not an object"));
		)
		return as_value();
	}

	mcl->addListener(listener);
	return as_value();
}

static as_value
moviecliploader_removelistener(const fn_call& fn)
{
	boost::intrusive_ptr<MovieClipLoader> mcl = ensureType<MovieClipLoader>(fn.this_ptr);
  
	boost::intrusive_ptr<as_object> listener = fn.arg(0).to_object();
	if ( ! listener )
	{
		IF_VERBOSE_ASCODING_ERRORS (
		log_aserror(_("Listener given to MovieClipLoader.removeListener() is not an object"));
		)
		return as_value();
	}

	mcl->removeListener(listener);
	return as_value();
}


void
moviecliploader_class_init(as_object& global)
{
	// This is going to be the global Number "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl=NULL;

	if ( cl == NULL )
	{
		cl=new builtin_function(&moviecliploader_new, getMovieClipLoaderInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachMovieClipLoaderInterface(*cl);  // not sure we should be doing this..
	}
	global.init_member("MovieClipLoader", cl.get()); //as_value(moviecliploader_new));
	//log_msg(_("MovieClipLoader class @ %p"), cl.get());
}

} // end of gnash namespace
