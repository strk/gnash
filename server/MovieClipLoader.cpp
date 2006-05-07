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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

// Implementation of ActionScript MovieClipLoader class.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"
#include "Function.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <typeinfo> 

#ifdef HAVE_LIBXML
// TODO: http and sockets and such ought to be factored out into an
// abstract driver, like we do for file access.
#include <libxml/nanohttp.h>
#ifdef HAVE_WINSOCK
# include <windows.h>
# include <sys/stat.h>
# include <io.h>
#else
# include <unistd.h>
# include <fcntl.h>
#endif
#endif
#include "MovieClipLoader.h"
//#include "Movie.h" // for movie_definition::create_instance, to be renamed
#include "movie_definition.h"
#include "log.h"
#include "tu_file.h"
#include "image.h"
#include "render.h"
#include "impl.h"

namespace gnash {

  
  MovieClipLoader::MovieClipLoader()
      // :     character(0, 0)
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
MovieClipLoader::load(const tu_string& filespec)
{
  log_msg("%s: \n", __FUNCTION__);
}

// progress of the downloaded file(s).
struct mcl *
MovieClipLoader::getProgress(as_object *ao)
{
  //log_msg("%s: \n", __FUNCTION__);

  return &_mcl;
}


bool
MovieClipLoader::loadClip(const tu_string& str, void *)
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
MovieClipLoader::addListener(void *)
{
  log_msg("%s: \n", __FUNCTION__);
}


void
MovieClipLoader::removeListener(void *)
{
  log_msg("%s: \n", __FUNCTION__);
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

void
MovieClipLoader::on_button_event(event_id event)
{
  log_msg("%s: \n", __FUNCTION__);
  
  // Set our mouse state (so we know how to render).
  switch (event.m_id)
    {
    case event_id::ROLL_OUT:
    case event_id::RELEASE_OUTSIDE:
      _mouse_state = MOUSE_UP;
      break;
      
    case event_id::RELEASE:
    case event_id::ROLL_OVER:
    case event_id::DRAG_OUT:
      _mouse_state = MOUSE_OVER;
      break;
      
    case event_id::PRESS:
    case event_id::DRAG_OVER:
      _mouse_state = MOUSE_DOWN;
      break;
      
    default:
      assert(0);	// missed a case?
      break;
    };
  
  // @@ eh, should just be a lookup table.
#if 0
  // Add appropriate actions to the movie's execute list...
  for (int i = 0; i < m_def->m_button_actions.size(); i++) {
    if (m_def->m_button_actions[i].m_conditions & c) {
      // Matching action.
      for (int j = 0; j < m_def->m_button_actions[i].m_actions.size(); j++) {
        get_parent()->add_action_buffer(m_def->m_button_actions[i].m_actions[j]);
      }
    }
  }
#endif
  // Call conventional attached method.
  // @@ TODO
}

void moviecliploader_loadclip(const fn_call& fn)
{
#ifdef HAVE_LIBXML
	as_value	val, method;
	struct stat   stats;
	int           fd;

	log_msg("%s: nargs = %d\n", __FUNCTION__, fn.nargs);

	moviecliploader_as_object* ptr = \
		dynamic_cast<moviecliploader_as_object*>(fn.this_ptr);

	assert(ptr);
  
	tu_string url = fn.arg(0).to_string(); 
	as_object *target = (as_object *)fn.arg(1).to_object();

	log_msg("load clip: %s, target is: %p (%s)\n", url.c_str(),
		target, typeid(*target).name());

	//
	// Extract root movie URL 
	// @@ could be cached somewhere...
	//
	as_value target_url;
	if ( ! target->get_member("_url", &target_url) )
	{
		log_msg("FIXME: no _url member in target!");
	}

	log_msg(" target._url: %s\n", target_url.to_string());

	xmlNanoHTTPInit();      // This doesn't do much for now, but in the
                                // future it might, so here it is...

	if (target == NULL)
	{
		//log_error("target doesn't exist:\n");
		fn.result->set_bool(false);
		return;    
	}

	//
	// Resolve relative urls
	// @@ todo
	

	// local file path
	// this is either fetched from http or local in origin
	tu_string filespec;
	bool filespec_copied = false;

	if (url.utf8_substring(0, 7) == "http://")
	{
		// Grab the filename off the end of the URL, and use the same name
		// as the disk file when something is fetched. Store files in /tmp/.
		// If the file exists, libxml properly replaces it.
		char *filename = strrchr(url.c_str(), '/');
		filespec = "/tmp";
		filespec += filename; 
				
		// fetch resource from URL
		xmlNanoHTTPFetch(url.c_str(), filespec.c_str(), NULL);
		xmlNanoHTTPCleanup();

		// FIXME: check for success or failure
		filespec_copied = true;

	}
	else if (url.utf8_substring(0, 7) == "file://")
	{
		filespec = url.utf8_substring(7, url.length());
	}
	else
	{
		// @@ should never happen if we resolve relative urls
		log_msg("FIXME: unresolved relative url\n");
		filespec = url;
	}

	// If the file doesn't exist, don't try to do anything.
	if (stat(filespec.c_str(), &stats) < 0)
	{
		log_error("MovieClipLoader.loadClip(%s): doesn't exist\n",
			filespec.c_str());
		fn.result->set_bool(false);
		return;
	}

	log_msg(" local filename: %s\n", filespec.c_str());
			 
	// Call the callback since we've started loading the file
	if (fn.this_ptr->get_member("onLoadStart", &method))
	{
	//log_msg("FIXME: Found onLoadStart!\n");
		as_c_function_ptr	func = method.to_c_function();
		fn.env->set_variable("success", true, std::vector<with_stack_entry>());
		if (func)
		{
			// It's a C function.  Call it.
			//log_msg("Calling C function for onLoadStart\n");
			(*func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
		}
		else if (function_as_object* as_func = method.to_as_function())
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
#if 0 // why would this be an error ?
	else
	{
		log_error("Couldn't find onLoadStart!\n");
	}
#endif 

	// Call the callback since we've started loading the file
	if (fn.this_ptr->get_member("onLoadStart", &method))
	{
	//log_msg("FIXME: Found onLoadStart!\n");
		as_c_function_ptr	func = method.to_c_function();
		fn.env->set_variable("success", true, std::vector<with_stack_entry>());
		if (func)
		{
			// It's a C function.  Call it.
			//log_msg("Calling C function for onLoadStart\n");
			(*func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
		}
		else if (function_as_object* as_func = method.to_as_function())
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
#if 0 // why would this be an error ?
	else
	{
		log_error("Couldn't find onLoadStart!\n");
	}
#endif


	tu_string suffix = filespec.utf8_substring(filespec.length() - 4,
			filespec.length());
	log_msg("File suffix to load is: %s\n", suffix.c_str());

	if (suffix == ".swf")
	{
		movie_definition* md = create_library_movie(filespec.c_str());
		if (md == NULL) {
			log_error("can't create movie_definition for %s\n",
				filespec.c_str());
			return;
		}
		gnash::movie_interface* extern_movie;
		extern_movie = md->create_instance();
		if (extern_movie == NULL) {
			log_error("can't create extern movie_interface "
				"for %s\n", filespec.c_str());
			return;
		}
  
		save_extern_movie(extern_movie);
    
		character* tar = (character*)target;
		const char* name = tar->get_name().c_str();
		Uint16 depth = tar->get_depth();
		bool use_cxform = false;
		cxform color_transform =  tar->get_cxform();
		bool use_matrix = false;
		matrix mat = tar->get_matrix();
		float ratio = tar->get_ratio();
		Uint16 clip_depth = tar->get_clip_depth();

		movie* parent = tar->get_parent();
		movie* new_movie = static_cast<movie*>(extern_movie)->get_root_movie();

		assert(parent != NULL);

		((character*)new_movie)->set_parent(parent);
    
		parent->replace_display_object(
				(character*) new_movie,
                                   name,
                                   depth,
                                   use_cxform,
                                   color_transform,
                                   use_matrix,
                                   mat,
                                   ratio,
                                   clip_depth);
	}

	else if (suffix == ".jpg") 
	{

	// FIXME: temporarly disabled
	log_msg("Loading of jpegs unsupported");
	fn.result->set_bool(false);
	return;


		// Just case the filespec suffix claims it's a jpeg,
		// we have to check, since when grabbing an image from a
		// web server that doesn't exist, we don't get an error,
		// we get a short HTML page containing a 404.
		if ((fd=open(filespec.c_str(), O_RDONLY)) < 0)
		{
			log_error("can't open image!\n");
			if ( filespec_copied ) unlink(filespec.c_str());
			fn.result->set_bool(false);
			return;
		}

		unsigned char buf[5];
		memset(buf, 0, 5);
		if (!read(fd, buf, 4))
		{
			log_error("Can't read image header!\n");
			if ( filespec_copied ) unlink(filespec.c_str());
			fn.result->set_bool(false);
			return;
		}
		
		close(fd); // we don't need this anymore

		// This is the magic number for any JPEG format file
		if ((buf[0] == 0xff) && (buf[1] == 0xd8) && (buf[2] != 0xff))
		{
			log_error("File is not a JPEG!\n");
			if ( filespec_copied ) unlink(filespec.c_str());
			fn.result->set_bool(false);
			return;
		}
		
		//log_msg("File is a JPEG!\n");
    

		bitmap_info* bi = NULL;
		image::rgb* im = image::read_jpeg(filespec.c_str());
		if (im != NULL) {
			bi = render::create_bitmap_info_rgb(im);
			delete im;
		} else {
			log_error("Can't read jpeg: %s\n", filespec.c_str());
		}

		//bitmap_character*	 ch = new bitmap_character(bi);

		movie *mov = target->to_movie();
		//movie_definition *def = mov->get_movie_definition();
		//movie_definition *m = (movie_definition *)mov;
		//target->add_bitmap_info(bi);

		character* tar = (character*)mov;
		const char* name = tar->get_name().c_str();
		Uint16 id = tar->get_id();
		//log_msg("Target name is: %s, ID: %d\n", name, id);

		// FIXME: none of this works yet

		//movie_definition *md = create_library_movie(filespec.c_str());
		// add image to movie, under character id.
		//m->add_bitmap_character(666, ch);

		tu_string swfm = filespec.utf8_substring(0,
			filespec.length() - 3);
		swfm += "swf";

		movie_definition *ms = create_movie(swfm.c_str());
		// The file may not exist.
		if (ms) { 
			//movie_interface* extern_movie =
			//	create_library_movie_inst(ms);
			//character * newchar =
			//	ms->create_character_instance(tar->get_parent(),
			//			id);
		}
     
		//save_extern_movie(extern_movie);
		//movie* new_movie = static_cast<movie*>(extern_movie)->get_root_movie();
     
// #else
//     movie_definition*ms;
//     ms->add_bitmap_info(bi);
// #endif
		//movie* m = mov->get_root_movie();
		//set_current_root(extern_movie);
		//movie* m = static_cast<movie*>(extern_movie)->get_root_movie();
		mov->on_event(event_id::LOAD);
		//add_display_object();

		//Uint16 depth = tar->get_depth();
		bool use_cxform = false;
		//cxform color_transform =  tar->get_cxform();
		bool use_matrix = false;
		matrix mat = tar->get_matrix();
		//float ratio = tar->get_ratio();
		//Uint16 clip_depth = tar->get_clip_depth();
		std::vector<swf_event*>	dummy_event_handlers;
		movie* parent = tar->get_parent();
    
		character *newch = new character(parent, id);
    
#if 0
    parent->clone_display_object(name, "album_image", depth);
    parent->add_display_object((Uint16)id,
                                name,
                                dummy_event_handlers,
                                tar->get_depth(),
                                true,
                                tar->get_cxform(),
                                tar->get_matrix(),
                                tar->get_ratio(),
                                tar->get_clip_depth());
#endif // def HAVE_LIBXML 

		parent->replace_display_object(newch,
                                name,
                                tar->get_depth(),
                                use_cxform,
                                tar->get_cxform(),
                                use_matrix,
                                tar->get_matrix(),
                                tar->get_ratio(),
                                tar->get_clip_depth());
	}
  
	struct mcl *mcl_data = ptr->mov_obj.getProgress(target);

	// the callback since we're done loading the file
	// FIXME: these both probably shouldn't be set to the same value
	mcl_data->bytes_loaded = stats.st_size;
	mcl_data->bytes_total = stats.st_size;

	fn.env->set_member("target_mc", target);
	//env->push(as_value(target));
	//moviecliploader_onload_complete(result, this_ptr, env, 0, 0);
	moviecliploader_onload_complete(fn);
	//env->pop();
  
	fn.result->set_bool(true);

	//unlink(filespec.c_str());
  
	//xmlNanoHTTPCleanup();

#endif // HAVE_LIBXML
}

void
moviecliploader_unloadclip(const fn_call& fn)
{
  const tu_string filespec = fn.arg(0).to_string();
  log_msg("%s: FIXME: Load Movie Clip: %s\n", __FUNCTION__, filespec.c_str());
  
}

void
moviecliploader_new(const fn_call& fn)
{

  log_msg("%s: args=%d\n", __FUNCTION__, fn.nargs);
  
  //const tu_string filespec = fn.arg(0).to_string();
  
  as_object*	mov_obj = new moviecliploader_as_object;
  //log_msg("\tCreated New MovieClipLoader object at %p\n", mov_obj);

  mov_obj->set_member("loadClip",
                      &moviecliploader_loadclip);
  mov_obj->set_member("unloadClip",
                      &moviecliploader_unloadclip);
  mov_obj->set_member("getProgress",
                      &moviecliploader_getprogress);

#if 0
  // Load the default event handlers. These should really never
  // be called directly, as to be useful they are redefined
  // within the SWF script. These get called if there is a problem
  // Setup the event handlers
  mov_obj->set_event_handler(event_id::LOAD_INIT,
                             (as_c_function_ptr)&event_test);
  mov_obj->set_event_handler(event_id::LOAD_START,
                             (as_c_function_ptr)&event_test);
  mov_obj->set_event_handler(event_id::LOAD_PROGRESS,
                             (as_c_function_ptr)&event_test);
  mov_obj->set_event_handler(event_id::LOAD_ERROR,
                             (as_c_function_ptr)&event_test);
#endif
  
  fn.result->set_as_object(mov_obj);
}

void
moviecliploader_onload_init(const fn_call& fn)
{
  log_msg("%s: FIXME: Default event handler, you shouldn't be here!\n", __FUNCTION__);
}

// Invoked when a call to MovieClipLoader.loadClip() has successfully
// begun to download a file.
void
moviecliploader_onload_start(const fn_call& fn)
{
  log_msg("%s: FIXME: Default event handler, you shouldn't be here!\n", __FUNCTION__);
}

// Invoked every time the loading content is written to disk during
// the loading process.
void
moviecliploader_getprogress(const fn_call& fn)
{
  //log_msg("%s: nargs = %d\n", __FUNCTION__, nargs);
  
  moviecliploader_as_object*	ptr = (moviecliploader_as_object*) (as_object*) fn.this_ptr;
  assert(ptr);
  
  as_object *target = (as_object*) fn.arg(0).to_object();
  
  struct mcl *mcl_data = ptr->mov_obj.getProgress(target);

  mcl_as_object *mcl_obj = (mcl_as_object *)new mcl_as_object;

  mcl_obj->set_member("bytesLoaded", mcl_data->bytes_loaded);
  mcl_obj->set_member("bytesTotal",  mcl_data->bytes_total);
  
  fn.result->set_as_object(mcl_obj);
}

// Invoked when a file loaded with MovieClipLoader.loadClip() has
// completely downloaded.
void
moviecliploader_onload_complete(const fn_call& fn)
{
  as_value	val, method;
  //log_msg("%s: FIXME: nargs = %d\n", __FUNCTION__, nargs);
  //moviecliploader_as_object*	ptr = (moviecliploader_as_object*) (as_object*) this_ptr;
  
  tu_string url = fn.arg(0).to_string();  
  //as_object *target = (as_object *)env->bottom(first_arg-1).to_object();
  //log_msg("load clip: %s, target is: %p\n", url.c_str(), target);

  //log_msg("%s: FIXME: Default event handler, you shouldn't be here!\n", __FUNCTION__);
  if (fn.this_ptr->get_member("onLoadComplete", &method)) {
    //log_msg("FIXME: Found onLoadComplete!\n");
    as_c_function_ptr	func = method.to_c_function();
    fn.env->set_variable("success", true, std::vector<with_stack_entry>());
    if (func)
      {
        // It's a C function.  Call it.
        //log_msg("Calling C function for onLoadComplete\n");
        (*func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
      }
    else if (function_as_object* as_func = method.to_as_function())
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
void
moviecliploader_onload_error(const fn_call& fn)
{
  //log_msg("%s: FIXME: Default event handler, you shouldn't be here!\n", __FUNCTION__);
  as_value	val, method;
  log_msg("%s: FIXME: nargs = %d\n", __FUNCTION__, fn.nargs);
  //moviecliploader_as_object*	ptr = (moviecliploader_as_object*) (as_object*) this_ptr;
  
  tu_string url = fn.arg(0).to_string();  
  as_object *target = (as_object*) fn.arg(1).to_object();
  log_msg("load clip: %s, target is: %p\n", url.c_str(), target);

  //log_msg("%s: FIXME: Default event handler, you shouldn't be here!\n", __FUNCTION__);
  if (fn.this_ptr->get_member("onLoadError", &method)) {
    //log_msg("FIXME: Found onLoadError!\n");
    as_c_function_ptr	func = method.to_c_function();
    fn.env->set_variable("success", true, std::vector<with_stack_entry>());
    if (func)
      {
        // It's a C function.  Call it.
        log_msg("Calling C function for onLoadError\n");
        (*func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
      }
    else if (function_as_object* as_func = method.to_as_function())
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
void
moviecliploader_default(const fn_call& fn)
{
  log_msg("%s: FIXME: Default event handler, you shouldn't be here!\n", __FUNCTION__);
}

} // end of gnash namespace
