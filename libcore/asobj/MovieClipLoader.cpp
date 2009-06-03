// MovieClipLoader.cpp:  Implementation of ActionScript MovieClipLoader class.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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
#include "gnashconfig.h"
#endif

#include "smart_ptr.h" // GNASH_USE_GC
#include "action.h" // for call_method
#include "as_value.h"
#include "as_object.h" // for inheritance
#include "movie_root.h"
#include "fn_call.h"
#include "as_function.h"
#include "MovieClip.h"
#include "DisplayObject.h" // for loadClip (get_parent)
#include "log.h"
#include "URL.h" // for url parsing
#include "VM.h" // for the string table.
#include "string_table.h" // for the string table.
#include "builtin_function.h"
#include "Object.h" // for getObjectInterface
#include "AsBroadcaster.h" // for initializing self as a broadcaster
#include "namedStrings.h"
#include "Array_as.h" // for _listeners construction
#include "ExecutableCode.h"

#include <string>

//#define GNASH_DEBUG 1

namespace gnash {


// Forward declarations
namespace {
    as_value moviecliploader_loadclip(const fn_call& fn);
    as_value moviecliploader_unloadclip(const fn_call& fn);
    as_value moviecliploader_getprogress(const fn_call& fn);
    as_value moviecliploader_new(const fn_call& fn);
    void attachMovieClipLoaderInterface(as_object& o);
    as_object* getMovieClipLoaderInterface();
}

/// This class is used to queue a function call action
//
/// Exact use is to queue onLoadInit, which should be invoked
/// after actions of in first frame of a loaded movie are executed.
/// Since those actions are queued the only way to execute something
/// after them is to queue the function call as well.
///
/// The class might be made more general and accessible outside
/// of the MovieClipLoader class. For now it only works for
/// calling a function with a two argument.
///
class DelayedFunctionCall : public ExecutableCode
{

public:

    DelayedFunctionCall(as_object* target, string_table::key name,
            const as_value& arg1, const as_value& arg2)
        :
        _target(target),
        _name(name),
        _arg1(arg1),
        _arg2(arg2)
    {}


    ExecutableCode* clone() const
    {
        return new DelayedFunctionCall(*this);
    }

    virtual void execute()
    {
        _target->callMethod(_name, _arg1, _arg2);
    }

#ifdef GNASH_USE_GC
    /// Mark reachable resources (for the GC)
    //
    /// Reachable resources are:
    ///  - the action target (_target)
    ///
    virtual void markReachableResources() const
    {
      _target->setReachable();
      _arg1.setReachable();
      _arg2.setReachable();
    }
#endif // GNASH_USE_GC

private:

    as_object* _target;
    string_table::key _name;
    as_value _arg1, _arg2;

};

class MovieClipLoader: public as_object
{
public:

	MovieClipLoader();

	~MovieClipLoader() {}

	/// MovieClip
	bool loadClip(const std::string& url, MovieClip* target);

	void unloadClip();

protected:

    void markReachableResources() const {
        markAsObjectReachable();
    }

private:
};

MovieClipLoader::MovieClipLoader()
	:
	as_object(getMovieClipLoaderInterface())
{

	Array_as* ar = new Array_as();
	ar->push(this);
	set_member(NSV::PROP_uLISTENERS, ar);
}

bool
MovieClipLoader::loadClip(const std::string& url_str, MovieClip* target)
{
    
    movie_root& mr = _vm.getRoot();

	URL url(url_str, mr.runInfo().baseURL());
	
#if GNASH_DEBUG
	log_debug(_(" resolved url: %s"), url.str());
#endif
			 
	as_value targetVal(target);
	log_debug("Target is %s", targetVal);

	bool ret = target->loadMovie(url);
	if ( ! ret ) 
	{

        // FIXME: docs suggest the string can be either "URLNotFound" or
        // "LoadNeverCompleted". This is neither of them:
		as_value arg1("Failed to load movie or jpeg");

		// FIXME: The last argument is HTTP status, or 0 if no connection
        // was attempted (sandbox) or no status information is available
        // (supposedly the Adobe mozilla plugin).
		as_value arg2(0.0);
		callMethod(NSV::PROP_BROADCAST_MESSAGE, "onLoadError", targetVal,
                arg1, arg2);

		return false;
	}

    // this is to resolve the soft ref
	MovieClip* newChar = targetVal.to_sprite(); 
	if ( ! newChar )
	{
		// We could assert, but let's try to be nicer...
		log_error("MovieClip::loadMovie destroyed self without replacing?");
		return false;
	}

	// Dispatch onLoadStart
	callMethod(NSV::PROP_BROADCAST_MESSAGE, "onLoadStart", targetVal);

	// Dispatch onLoadProgress
	size_t bytesLoaded = newChar->get_bytes_loaded();
	size_t bytesTotal = newChar->get_bytes_total();
	callMethod(NSV::PROP_BROADCAST_MESSAGE, "onLoadProgress", targetVal,
		bytesLoaded, bytesTotal);

	// Dispatch onLoadComplete
	callMethod(NSV::PROP_BROADCAST_MESSAGE, "onLoadComplete", targetVal,
		as_value(0.0)); // TODO: find semantic of last arg

	/// This event must be dispatched when actions
	/// in first frame of loaded clip have been executed.
	///
	/// Since MovieClip::loadMovie above will invoke stagePlacementCallback
	/// and thus queue all actions in first frame, we'll queue the
	/// onLoadInit call next, so it happens after the former.
	std::auto_ptr<ExecutableCode> code(
            new DelayedFunctionCall(this, NSV::PROP_BROADCAST_MESSAGE, 
                "onLoadInit", targetVal));

	_vm.getRoot().pushAction(code, movie_root::apDOACTION);

	return true;
}

void
MovieClipLoader::unloadClip()
{
    GNASH_REPORT_FUNCTION;
}

/// Extern.
void
moviecliploader_class_init(as_object& global)
{
	// This is going to be the global Number "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl = NULL;

	if (cl == NULL)
	{
		cl=new builtin_function(&moviecliploader_new,
                getMovieClipLoaderInterface());
	}
	global.init_member("MovieClipLoader", cl.get()); 
}


namespace {

void
attachMovieClipLoaderInterface(as_object& o)
{
  	o.init_member("loadClip", new builtin_function(moviecliploader_loadclip));
	o.init_member("unloadClip",
            new builtin_function(moviecliploader_unloadclip));
	o.init_member("getProgress",
            new builtin_function(moviecliploader_getprogress));

	// NOTE: we want addListener/removeListener/broadcastMessage
	//       but don't what the _listeners property here...
	// TODO: add an argument to AsBroadcaster::initialize skip listeners ?
	AsBroadcaster::initialize(o);
	o.delProperty(NSV::PROP_uLISTENERS);
  
}

as_object*
getMovieClipLoaderInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( o == NULL )
	{
		o = new as_object(getObjectInterface());
		attachMovieClipLoaderInterface(*o);
	}
	return o.get();
}

as_value
moviecliploader_loadclip(const fn_call& fn)
{

	boost::intrusive_ptr<MovieClipLoader> ptr =
        ensureType<MovieClipLoader>(fn.this_ptr);
  
	if ( fn.nargs < 2 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_aserror(_("MovieClipLoader.loadClip(%s): missing arguments"),
            ss.str());
		);
		return as_value(false);
	}

	as_value url_arg = fn.arg(0);
	std::string str_url = url_arg.to_string(); 

	as_value tgt_arg = fn.arg(1);
	std::string tgt_str = tgt_arg.to_string();
	DisplayObject* target = fn.env().find_target(tgt_str);
	if ( ! target )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Could not find target %s (evaluated from %s)"),
			tgt_str, tgt_arg);
		);
		return as_value(false);
	}

	MovieClip* sprite = target->to_movie();
	if ( ! sprite )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Target %s is not a sprite instance (%s)"),
			target->getTarget(), typeName(*target));
		);
		return as_value(false);
	}

#if GNASH_DEBUG
	log_debug(_("load clip: %s, target is: %p\n"),
		str_url, (void*)sprite);
#endif

	ptr->loadClip(str_url, sprite);

	// We always want to return true unless something went wrong
	return as_value(true);

}

as_value
moviecliploader_unloadclip(const fn_call& fn)
{
  const std::string filespec = fn.arg(0).to_string();
  log_unimpl (_("%s: %s"), __PRETTY_FUNCTION__, filespec);
  return as_value();
}

as_value
moviecliploader_new(const fn_call& /* fn */)
{

  as_object* mov_obj = new MovieClipLoader;

  return as_value(mov_obj);
}

// Invoked every time the loading content is written to disk during
// the loading process.
as_value
moviecliploader_getprogress(const fn_call& fn)
{

	boost::intrusive_ptr<MovieClipLoader> ptr =
        ensureType<MovieClipLoader>(fn.this_ptr);
  
	if ( ! fn.nargs )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("MovieClipLoader.getProgress(): missing argument"));
		);
		return as_value();
	}

	boost::intrusive_ptr<as_object> target = fn.arg(0).to_object();
  
	if ( ! target.get() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("MovieClipLoader.getProgress(%s): first argument is "
                "not an object"), fn.arg(0));
		);
		return as_value();
	}

	MovieClip* sp = target->to_movie();
	if ( ! sp )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("MovieClipLoader.getProgress(%s): first argument is "
                "not an sprite"), fn.arg(0));
		);
		return as_value();
	}


	boost::intrusive_ptr<as_object> mcl_obj = new as_object;

	size_t bytesLoaded = sp->get_bytes_loaded();
	size_t bytesTotal = sp->get_bytes_total();

	string_table& st = ptr->getVM().getStringTable();

	// We want these to be enumerable
	mcl_obj->set_member(st.find("bytesLoaded"), bytesLoaded);
	mcl_obj->set_member(st.find("bytesTotal"),  bytesTotal);
  
	return as_value(mcl_obj.get()); // will keep alive
}

} // anonymous namespace
} // end of gnash namespace
