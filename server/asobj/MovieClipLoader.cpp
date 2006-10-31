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
static void moviecliploader_onload_init(const fn_call& fn);
static void moviecliploader_onload_start(const fn_call& fn);
static void moviecliploader_onload_progress(const fn_call& fn);
static void moviecliploader_onload_complete(const fn_call& fn);
static void moviecliploader_onload_error(const fn_call& fn);
static void moviecliploader_default(const fn_call& fn);
static void moviecliploader_addlistener(const fn_call& fn);
static void moviecliploader_removelistener(const fn_call& fn);

static void
attachMovieClipLoaderInterface(as_object& o)
{
  	o.set_member("loadClip", &moviecliploader_loadclip);
	o.set_member_flags("loadClip", 1); // hidden
	o.set_member("unloadClip", &moviecliploader_unloadclip);
	o.set_member_flags("unloadClip", 1); // hidden
	o.set_member("getProgress", &moviecliploader_getprogress);
	o.set_member_flags("getProgress", 1); // hidden

	o.set_member("addListener", &moviecliploader_addlistener);
	o.set_member_flags("addListener", 1); // hidden
	o.set_member("removeListener", &moviecliploader_removelistener);
	o.set_member_flags("removeListener", 1); // hidden

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
	static as_object* o=NULL;
	if ( o == NULL )
	{
		o = new as_object();
		attachMovieClipLoaderInterface(*o);
	}
	return o;
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

	void load(const tu_string& filespec);
  
	struct mcl *getProgress(as_object *ao);

	/// MovieClip
	bool loadClip(const std::string& url, void *);

	void unloadClip(void *);

	/// Add an object to the list of event listeners
	//
	/// This function will call add_ref() on the
	/// given object.
	///
	void addListener(as_object* listener);

	void removeListener(as_object* listener);

	// Callbacks
	void onLoadStart(void *);
	void onLoadProgress(void *);
	void onLoadInit(void *);
	void onLoadComplete(void *);
	void onLoadError(void *);

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
  log_msg("%s: \n", __FUNCTION__);
  _mcl.bytes_loaded = 0;
  _mcl.bytes_total = 0;  
}

MovieClipLoader::~MovieClipLoader()
{
  log_msg("%s: \n", __FUNCTION__);
}

void
MovieClipLoader::load(const tu_string& /*filespec*/)
{
  log_msg("%s: \n", __FUNCTION__);
}

// progress of the downloaded file(s).
struct mcl *
MovieClipLoader::getProgress(as_object* /*ao*/)
{
  //log_msg("%s: \n", __FUNCTION__);

  return &_mcl;
}


bool
MovieClipLoader::loadClip(const std::string&, void *)
{
  log_msg("%s: \n", __FUNCTION__);

  return false;
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
MovieClipLoader::onLoadStart(void *)
{
  log_msg("%s: \n", __FUNCTION__);
}

void
MovieClipLoader::onLoadProgress(void *)
{
  log_msg("%s: \n", __FUNCTION__);
}

void
MovieClipLoader::onLoadInit(void *)
{
  log_msg("%s: \n", __FUNCTION__);
}

void
MovieClipLoader::onLoadComplete(void *)
{
  log_msg("%s: \n", __FUNCTION__);
}

void
MovieClipLoader::onLoadError(void *)
{
  log_msg("%s: \n", __FUNCTION__);
}

static void
moviecliploader_loadclip(const fn_call& fn)
{
	as_value	val, method;

	log_msg("%s: nargs = %d\n", __FUNCTION__, fn.nargs);

	MovieClipLoader* ptr = \
		dynamic_cast<MovieClipLoader*>(fn.this_ptr);

	assert(ptr);
  
	as_value& url_arg = fn.arg(0);
	if ( url_arg.get_type() != as_value::STRING )
	{
		log_error("Malformed SWF, MovieClipLoader.loadClip() first argument is not a string (%s)", url_arg.to_string());
		fn.result->set_bool(false);
		return;
	}

	std::string str_url = fn.arg(0).to_string(); 
	character* target = fn.env->find_target(fn.arg(1));
	if ( ! target )
	{
		log_error("Could not find target %s", fn.arg(1).to_string());
		fn.result->set_bool(false);
		return;
	}

	log_msg("load clip: %s, target is: %p\n",
		str_url.c_str(), (void*)target);

	// Get a pointer to target's sprite parent 
	character* parent = target->get_parent();
	assert(parent);

	URL url(str_url.c_str(), get_base_url());
	
	log_msg(" resolved url: %s\n", url.str().c_str());
			 
	// Call the callback since we've started loading the file
	if (fn.this_ptr->get_member("onLoadStart", &method))
	{
	//log_msg("FIXME: Found onLoadStart!\n");
		as_c_function_ptr	func = method.to_c_function();
		fn.env->set_variable("success", true);
		if (func)
		{
			// It's a C function.  Call it.
			//log_msg("Calling C function for onLoadStart\n");
			(*func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
		}
		else if (as_function* as_func = method.to_as_function())
		{
		// It's an ActionScript function.  Call it.
			//log_msg("Calling ActionScript function for onLoadStart\n");
			(*as_func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
		}
		else
		{
			log_error("error in call_method(): method is not a function\n");
		}    
	}

	// Call the callback since we've started loading the file
	if (fn.this_ptr->get_member("onLoadStart", &method))
	{
	//log_msg("FIXME: Found onLoadStart!\n");
		as_c_function_ptr	func = method.to_c_function();
		fn.env->set_variable("success", true);
		if (func)
		{
			// It's a C function.  Call it.
			//log_msg("Calling C function for onLoadStart\n");
			(*func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
		}
		else if (as_function* as_func = method.to_as_function())
		{
		// It's an ActionScript function.  Call it.
			//log_msg("Calling ActionScript function for onLoadStart\n");
			(*as_func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
		}
		else
		{
			log_error("error in call_method(): method is not a function\n");
		}    
	}

	std::string path = url.path();
	std::string suffix = path.substr(path.size() - 4);
	log_msg("File suffix to load is: %s\n", suffix.c_str());

	movie_definition* md = create_library_movie(url);
	if (md == NULL) {
		log_error("can't create movie_definition for %s\n",
			url.str().c_str());
		fn.result->set_bool(false);
		return;
	}

	log_msg("movie definition created\n");

	gnash::movie_interface* extern_movie;
	extern_movie = md->create_instance();
	if (extern_movie == NULL) {
		log_error("can't create extern movie_interface "
			"for %s\n", url.str().c_str());
		fn.result->set_bool(false);
		return;
	}

	log_msg("movie instance created\n");

	save_extern_movie(extern_movie);

	character* tar = target;
	const char* name = tar->get_name().c_str();
	uint16_t depth = tar->get_depth();
	bool use_cxform = false;
	cxform color_transform =  tar->get_cxform();
	bool use_matrix = false;
	matrix mat = tar->get_matrix();
	float ratio = tar->get_ratio();
	uint16_t clip_depth = tar->get_clip_depth();

	character* new_movie = extern_movie->get_root_movie();

	new_movie->set_parent(parent);

	parent->replace_display_object(
			   new_movie,
			   name,
			   depth,
			   use_cxform,
			   color_transform,
			   use_matrix,
			   mat,
			   ratio,
			   clip_depth);
  
	struct mcl *mcl_data = ptr->getProgress(target);

	// the callback since we're done loading the file
	// FIXME: these both probably shouldn't be set to the same value
	//mcl_data->bytes_loaded = stats.st_size;
	//mcl_data->bytes_total = stats.st_size;
	mcl_data->bytes_loaded = 666; // fake values for now
	mcl_data->bytes_total = 666;

	fn.env->set_member("target_mc", target);
	moviecliploader_onload_complete(fn);
	//env->pop();
  
	fn.result->set_bool(true);

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

  fn.result->set_as_object(mov_obj);
}

static void
moviecliploader_onload_init(const fn_call& /*fn*/)
{
  log_msg("%s: FIXME: Default event handler, you shouldn't be here!\n", __FUNCTION__);
}

// Invoked when a call to MovieClipLoader.loadClip() has successfully
// begun to download a file.
static void
moviecliploader_onload_start(const fn_call& /*fn*/)
{
  log_msg("%s: FIXME: Default event handler, you shouldn't be here!\n", __FUNCTION__);
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

  mcl_obj->set_member("bytesLoaded", mcl_data->bytes_loaded);
  mcl_obj->set_member("bytesTotal",  mcl_data->bytes_total);
  
  fn.result->set_as_object(mcl_obj);
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

// Invoked when a file loaded with MovieClipLoader.loadClip() has
// completely downloaded.
static void
moviecliploader_onload_complete(const fn_call& fn)
{
  as_value	val, method;
  //log_msg("%s: FIXME: nargs = %d\n", __FUNCTION__, nargs);
  //MovieClipLoader*	ptr = (MovieClipLoader*) (as_object*) this_ptr;
  
  std::string url = fn.arg(0).to_string();  
  //as_object *target = (as_object *)env->bottom(first_arg-1).to_object();
  //log_msg("load clip: %s, target is: %p\n", url.c_str(), target);

  //log_msg("%s: FIXME: Default event handler, you shouldn't be here!\n", __FUNCTION__);
  if (fn.this_ptr->get_member("onLoadComplete", &method)) {
    //log_msg("FIXME: Found onLoadComplete!\n");
    as_c_function_ptr	func = method.to_c_function();
    fn.env->set_variable("success", true);
    if (func)
      {
        // It's a C function.  Call it.
        //log_msg("Calling C function for onLoadComplete\n");
        (*func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
      }
    else if (as_function* as_func = method.to_as_function())
      {
        // It's an ActionScript function.  Call it.
        //log_msg("Calling ActionScript function for onLoadComplete\n");
        (*as_func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
      }
    else
      {
        log_error("error in call_method(): method is not a function\n");
      }    
  } else {
    log_error("Couldn't find onLoadComplete!\n");
  }
}

// Invoked when a file loaded with MovieClipLoader.loadClip() has failed to load.
static void
moviecliploader_onload_error(const fn_call& fn)
{
  //log_msg("%s: FIXME: Default event handler, you shouldn't be here!\n", __FUNCTION__);
  as_value	val, method;
  log_msg("%s: FIXME: nargs = %d\n", __FUNCTION__, fn.nargs);
  //MovieClipLoader*	ptr = (MovieClipLoader*) (as_object*) this_ptr;
  
  std::string url = fn.arg(0).to_string();  
  as_object *target = (as_object*) fn.arg(1).to_object();
  log_msg("load clip: %s, target is: %p\n", url.c_str(), (void *)target);

  //log_msg("%s: FIXME: Default event handler, you shouldn't be here!\n", __FUNCTION__);
  if (fn.this_ptr->get_member("onLoadError", &method)) {
    //log_msg("FIXME: Found onLoadError!\n");
    as_c_function_ptr	func = method.to_c_function();
    fn.env->set_variable("success", true);
    if (func)
      {
        // It's a C function.  Call it.
        log_msg("Calling C function for onLoadError\n");
        (*func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
      }
    else if (as_function* as_func = method.to_as_function())
      {
        // It's an ActionScript function.  Call it.
        log_msg("Calling ActionScript function for onLoadError\n");
        (*as_func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
      }
    else
      {
        log_error("error in call_method(): method is not a function\n");
      }    
  } else {
    log_error("Couldn't find onLoadError!\n");
  }
}

// This is the default event handler. To wind up here is an error.
static void
moviecliploader_default(const fn_call& /*fn*/)
{
  log_msg("%s: FIXME: Default event handler, you shouldn't be here!\n", __FUNCTION__);
}

void
moviecliploader_class_init(as_object& global)
{
	global.set_member("MovieClipLoader", as_value(moviecliploader_new));
}

} // end of gnash namespace
