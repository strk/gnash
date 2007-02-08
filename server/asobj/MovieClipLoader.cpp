// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

// Implementation of ActionScript MovieClipLoader class.

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

#include <typeinfo> 
#include <string>
#include <set>

namespace gnash {

// Forward declarations
static void moviecliploader_loadclip(const fn_call& fn);
static void moviecliploader_unloadclip(const fn_call& fn);
static void moviecliploader_getprogress(const fn_call& fn);
static void moviecliploader_new(const fn_call& fn);
static void moviecliploader_addlistener(const fn_call& fn);
static void moviecliploader_removelistener(const fn_call& fn);

static void
attachMovieClipLoaderInterface(as_object& o)
{
  	o.init_member("loadClip", &moviecliploader_loadclip);
	o.init_member("unloadClip", &moviecliploader_unloadclip);
	o.init_member("getProgress", &moviecliploader_getprogress);

	o.init_member("addListener", &moviecliploader_addlistener);
	o.init_member("removeListener", &moviecliploader_removelistener);

#if 0
	// Load the default event handlers. These should really never
	// be called directly, as to be useful they are redefined
	// within the SWF script. These get called if there is a problem
	// Setup the event handlers
	o.set_event_handler(event_id::LOAD_INIT, &event_test);
	o.set_event_handler(event_id::LOAD_START, &event_test);
	o.set_event_handler(event_id::LOAD_PROGRESS, &event_test);
	o.set_event_handler(event_id::LOAD_ERROR, &event_test);
#endif
  
}

static as_object*
getMovieClipLoaderInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( o == NULL )
	{
		o = new as_object();
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
	void addListener(as_object* listener);

	void removeListener(as_object* listener);

	/// Invoke any listener for the specified event
	void dispatchEvent(const std::string& eventName, fn_call& fn);

	/// @ }

private:

	std::set<as_object*> _listeners;
	bool          _started;
	bool          _completed;
	tu_string     _filespec;
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
  //log_msg("%s: \n", __FUNCTION__);

  return &_mcl;
}


bool
MovieClipLoader::loadClip(const std::string& url_str, sprite_instance& target)
{
	// Prepare function call for events...
	as_environment env;
	env.push(as_value(&target));
	fn_call events_call(NULL, this, &env, 1, 0);

	URL url(url_str.c_str(), get_base_url());
	
#if GNASH_DEBUG
	log_msg(" resolved url: %s\n", url.str().c_str());
#endif
			 
	// Call the callback since we've started loading the file
	// TODO: probably we should move this below, after 
	//       the loading thread actually started
	dispatchEvent("onLoadStart", events_call);

	bool ret = target.loadMovie(url);
	if ( ! ret ) return false;


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

	log_warning("FIXME: MovieClipLoader calling onLoadComplete *before* movie has actually been fully loaded (cheating)");
	dispatchEvent("onLoadComplete", events_call);

	return true;
}

void
MovieClipLoader::unloadClip(void *)
{
  log_msg("%s: \n", __FUNCTION__);
}


void
MovieClipLoader::addListener(as_object* listener)
{
	assert(listener); // caller should check
	if ( _listeners.insert(listener).second )
	{
		// listener inserted
		listener->add_ref();
	}
	else
	{
		// listener already present, no need to
		// increment ref count
	}

}


void
MovieClipLoader::removeListener(as_object* listener)
{
	assert(listener); // caller should check
	std::set<as_object*>::iterator it = _listeners.find(listener);
	if ( it != _listeners.end() )
	{
		(*it)->drop_ref();
		_listeners.erase(it);
	}
}

  
// Callbacks
void
MovieClipLoader::dispatchEvent(const std::string& event, fn_call& fn)
{
	typedef std::set<as_object*>::iterator iterator;

#if GNASH_DEBUG
	log_msg("Dispatching %s event to " SIZET_FMT " listeners",
		event.c_str(), _listeners.size());
#endif

	for (iterator it=_listeners.begin(), itEnd=_listeners.end();
			it != itEnd;
			++it)
	{
		as_object* listener = *it;
		as_value method;
		if ( ! listener->get_member(event.c_str(), &method) )
		{
#if GNASH_DEBUG
log_msg(" Listener %p doesn't have an %s event to listen for, skipped",
	(void*)listener, event.c_str());
#endif
			// this listener doesn't care about this event
			// event
			continue;
		}

#if GNASH_DEBUG
		log_msg("Testing call to listener's "
			" %s function", event.c_str());
#endif

		call_method(method, fn.env, fn.this_ptr, fn.nargs, fn.first_arg_bottom_index);
	}

}

static void
moviecliploader_loadclip(const fn_call& fn)
{
	as_value	val, method;

	//log_msg("%s: nargs = %d\n", __FUNCTION__, fn.nargs);

	MovieClipLoader* ptr = \
		dynamic_cast<MovieClipLoader*>(fn.this_ptr);

	assert(ptr);
  
	as_value& url_arg = fn.arg(0);
#if 0 // whatever it is, we'll need a string, the check below would only be worth
      // IF_VERBOSE_MALFORMED_SWF, but I'm not sure it's worth the trouble of
      // checking it, and chances are that the reference player will be trying
      // to convert to string anyway...
	if ( ! url_arg.is_string() )
	{
		log_error("Malformed SWF, MovieClipLoader.loadClip() first argument is not a string (%s)", url_arg.to_string());
		fn.result->set_bool(false);
		return;
	}
#endif
	std::string str_url = url_arg.to_string(); 

	character* target = fn.env->find_target(fn.arg(1));
	if ( ! target )
	{
		log_error("Could not find target %s", fn.arg(1).to_string());
		fn.result->set_bool(false);
		return;
	}
	sprite_instance* sprite = dynamic_cast<sprite_instance*>(target);
	if ( ! sprite )
	{
		log_error("Target is not a sprite instance (%s)",
			typeid(*target).name());
		fn.result->set_bool(false);
		return;
	}

#if GNASH_DEBUG
	log_msg("load clip: %s, target is: %p\n",
		str_url.c_str(), (void*)sprite);
#endif

	bool ret = ptr->loadClip(str_url, *sprite);

	fn.result->set_bool(ret);

}

static void
moviecliploader_unloadclip(const fn_call& fn)
{
  const std::string filespec = fn.arg(0).to_string();
  log_msg("%s: FIXME: Load Movie Clip: %s\n", __FUNCTION__, filespec.c_str());
  
}

static void
moviecliploader_new(const fn_call& fn)
{

  as_object*	mov_obj = new MovieClipLoader;

  fn.result->set_as_object(mov_obj); // will store in a boost::intrusive_ptr
}

// Invoked every time the loading content is written to disk during
// the loading process.
static void
moviecliploader_getprogress(const fn_call& fn)
{
  //log_msg("%s: nargs = %d\n", __FUNCTION__, nargs);
  
  MovieClipLoader* ptr = dynamic_cast<MovieClipLoader*>(fn.this_ptr);
  assert(ptr); // or warn if bogus call ?
  
  as_object *target = fn.arg(0).to_object();
  
  struct mcl *mcl_data = ptr->getProgress(target);

  mcl_as_object *mcl_obj = new mcl_as_object;

  mcl_obj->init_member("bytesLoaded", mcl_data->bytes_loaded);
  mcl_obj->init_member("bytesTotal",  mcl_data->bytes_total);
  
  fn.result->set_as_object(mcl_obj); // will store in a boost::intrusive_ptr
}

static void
moviecliploader_addlistener(const fn_call& fn)
{
	assert(dynamic_cast<MovieClipLoader*>(fn.this_ptr));
	MovieClipLoader* mcl = static_cast<MovieClipLoader*>(fn.this_ptr);
  
	as_object *listener = fn.arg(0).to_object();
	if ( ! listener )
	{
		log_error("ActionScript bug: Listener given to MovieClipLoader.addListener() is not an object");
		return;
	}

	mcl->addListener(listener);
}

static void
moviecliploader_removelistener(const fn_call& fn)
{
	assert(dynamic_cast<MovieClipLoader*>(fn.this_ptr));
	MovieClipLoader* mcl = static_cast<MovieClipLoader*>(fn.this_ptr);
  
	as_object *listener = fn.arg(0).to_object();
	if ( ! listener )
	{
		log_error("ActionScript bug: Listener given to MovieClipLoader.removeListener() is not an object");
		return;
	}

	mcl->removeListener(listener);
}


void
moviecliploader_class_init(as_object& global)
{
	global.init_member("MovieClipLoader", as_value(moviecliploader_new));
}

} // end of gnash namespace
