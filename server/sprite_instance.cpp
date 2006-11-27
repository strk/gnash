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

// Stateful live Sprite instance

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h" 
#include "action.h" // for call_method_parsed (call_method_args)
#include "gnash.h"
#include "render.h"  // for bounds_in_clipping_area()
#include "sprite_instance.h"
#include "movie_definition.h"
#include "MovieClipLoader.h" // @@ temp hack for loading tests
#include "as_value.h"
#include "as_function.h"
#include "text_character_def.h" // @@ temp hack for createTextField exp.
#include "execute_tag.h"
#include "fn_call.h"
#include "Key.h"
#include "movie_root.h"
#include "swf_event.h"
#include "sprite_definition.h"
#include "ActionExec.h"
#include "builtin_function.h"
#include "smart_ptr.h"
#include "VM.h"

#include <vector>
#include <string>
#include <cmath>

#include <functional> // for mem_fun, bind1st
#include <algorithm> // for for_each

// This needs to be included first for NetBSD systems or we get a weird
// problem with pthread_t being defined too many times if we use any
// STL containers.
#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

namespace gnash {

//------------------------------------------------
// Utility funx
//------------------------------------------------

// Execute the actions in the action list, in the given
// environment. The list of action will be consumed
// starting from the first element. When the function returns
// the list should be empty.
void
sprite_instance::execute_actions(sprite_instance::ActionList& action_list)
{
	// action_list may be changed due to actions (appended-to)
	// this loop might be optimized by using an iterator
	// and a final call to .clear() 
	while ( ! action_list.empty() )
	{
		action_buffer* ab = action_list.front();
		action_list.pop_front(); 

		execute_action(*ab);
	}
}

static void sprite_play(const fn_call& fn)
{
	assert(dynamic_cast<sprite_instance*>(fn.this_ptr));
	sprite_instance* sprite = static_cast<sprite_instance*>(fn.this_ptr);
	if (sprite == NULL)
	{
	    sprite = dynamic_cast<sprite_instance*>(fn.env->get_target());
	}
	assert(sprite);
	sprite->set_play_state(sprite_instance::PLAY);
}

static void sprite_stop(const fn_call& fn)
{
	assert(dynamic_cast<sprite_instance*>(fn.this_ptr));
	sprite_instance* sprite = static_cast<sprite_instance*>(fn.this_ptr);
	if (sprite == NULL)
	{
	    sprite = dynamic_cast<sprite_instance*>(fn.env->get_target());
	}
	assert(sprite);
	sprite->set_play_state(sprite_instance::STOP);

	// Stop sound stream as well, if such exist
	int stream_id = sprite->get_sound_stream_id();
	if (sprite->get_sound_stream_id() != -1) {
		sound_handler* sh = get_sound_handler();
		if (sh != NULL) sh->stop_sound(stream_id);
		sprite->set_sound_stream_id(-1);
	}

/*	sound_handler* sh = get_sound_handler();
	if (sh != NULL) sh->stop_all_sounds();*/
}

//removeMovieClip() : Void
static void sprite_remove_movieclip(const fn_call& fn)
{
	assert(dynamic_cast<sprite_instance*>(fn.this_ptr));
	sprite_instance* sprite = static_cast<sprite_instance*>(fn.this_ptr);
	if (sprite == NULL)
	{
	    sprite = dynamic_cast<sprite_instance*>(fn.env->get_target());
	}

	assert(sprite);

	sprite_instance* parent = (sprite_instance*) sprite->get_parent();
	if (parent)
	{
		parent->remove_display_object(sprite->get_depth(), 0);
	}
}

// attachMovie(idName:String, newName:String,
//             depth:Number [, initObject:Object]) : MovieClip
static void sprite_attach_movie(const fn_call& fn)
{
	sprite_instance* sprite = dynamic_cast<sprite_instance*>(fn.this_ptr);
	if (sprite == NULL)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_warning("attachMovie called against an object"
			" which is NOT a MovieClip (%s), "
			"returning undefined", typeid(fn.this_ptr).name());
		);
		fn.result->set_undefined();
		return;
	}

	if (fn.nargs < 3 || fn.nargs > 4)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_warning("attachMovie called with wrong number of arguments"
			" expected 3 to 4, got (%d) - returning undefined",
			fn.nargs);
		);
		fn.result->set_undefined();
		return;
	}

	// Get exported resource 
	std::string id_name = fn.arg(0).to_std_string();
	boost::intrusive_ptr<resource> exported = sprite->get_movie_definition()->get_exported_resource(id_name.c_str());
	if ( exported == NULL )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_warning("attachMovie: '%s': no such exported resource - "
			"returning undefined",
			id_name.c_str());
		);
		fn.result->set_undefined();
		return;
	}
	movie_definition* exported_movie = dynamic_cast<movie_definition*>(exported.get());
	if ( ! exported_movie )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_warning("attachMovie: exported resource '%s' "
			"is not a movie definition (%s) -- "
			"returning undefined",
			id_name.c_str(),
			typeid(*(exported.get())).name());
		);
		fn.result->set_undefined();
		return;
	}

	std::string newname = fn.arg(1).to_std_string();
	//int depth_val = int(fn.arg(2).to_number());

	if (fn.nargs > 3 )
	{
		//as_object* initObject = fn.arg(3).to_object();
		//if ( initObject ) newch->copyProperties(*initObject);
	}

	log_error("MovieClip.attachMovie() unimplemented -- "
		"returning undefined");
	fn.result->set_undefined();
	//fn.result->set_as_object(newch);
	return;
}

// attachAudio(id:Object) : Void
static void sprite_attach_audio(const fn_call& fn)
{
	log_error("MovieClip.attachAudio() unimplemented -- "
		"returning undefined");
	fn.result->set_undefined();
}

//createEmptyMovieClip(name:String, depth:Number) : MovieClip
static void sprite_create_empty_movieclip(const fn_call& fn)
{
	assert(dynamic_cast<sprite_instance*>(fn.this_ptr));
	sprite_instance* sprite = static_cast<sprite_instance*>(fn.this_ptr);
	if (sprite == NULL)
	{
	    sprite = dynamic_cast<sprite_instance*>(fn.env->get_target());
	}

	assert(sprite);

	if (fn.nargs != 2)
	{
	    log_error("createEmptyMovieClip needs 2 args\n");
	    return;
	}

	character* ch = sprite->add_empty_movieclip(fn.arg(0).to_string(), int(fn.arg(1).to_number()));
	fn.result->set_as_object(ch);
}

static void sprite_get_depth(const fn_call& fn)
{
	assert(dynamic_cast<sprite_instance*>(fn.this_ptr));
	sprite_instance* sprite = static_cast<sprite_instance*>(fn.this_ptr);
	if (sprite == NULL)
	{
	    sprite = dynamic_cast<sprite_instance*>(fn.env->get_target());
	}

	assert(sprite);
	int n = sprite->get_depth();

	// Macromedia Flash help says: depth starts at -16383 (0x3FFF)
	fn.result->set_int( - (n + 16383 - 1));
}

//swapDepths(target:Object) : Void
static void sprite_swap_depths(const fn_call& fn)
{
	assert(dynamic_cast<sprite_instance*>(fn.this_ptr));
	sprite_instance* sprite = static_cast<sprite_instance*>(fn.this_ptr);
	if (sprite == NULL)
	{
	    sprite = dynamic_cast<sprite_instance*>(fn.env->get_target());
	}
	assert(sprite);
	
	if (fn.nargs != 1)
	{
	    log_error("swapDepths needs one arg\n");
	    return;
	}

	sprite_instance* target;
	if (fn.arg(0).get_type() == as_value::OBJECT)
	{
		target = (sprite_instance*) fn.arg(0).to_object();
	}
	else
	if (fn.arg(0).get_type() == as_value::NUMBER)
	{
		// Macromedia Flash help says: depth starts at -16383 (0x3FFF)
		int target_depth = int(fn.arg(0).to_number()) + 16383 + 1;

		sprite_instance* parent = (sprite_instance*) sprite->get_parent();
		target = (sprite_instance*) parent->get_character_at_depth(target_depth);
	}
	else
	{
    log_error("swapDepths has received invalid arg\n");
		return;
	}

	if (sprite == NULL || target == NULL)
	{
    log_error("It is impossible to swap NULL character\n");
		return;
	}

	if (sprite->get_parent() == target->get_parent() && sprite->get_parent() != NULL)
	{
		int target_depth = target->get_depth();
		target->set_depth(sprite->get_depth());
		sprite->set_depth(target_depth);

		sprite_instance* parent = (sprite_instance*) sprite->get_parent();
		parent->swap_characters(sprite, target);
	}
	else
	{
    log_error("MovieClips should have the same parent\n");
	}
}

// TODO: wrap the functionality in a sprite_instance method
//       and invoke it from here, this should only be a wrapper
//
//duplicateMovieClip(name:String, depth:Number, [initObject:Object]) : MovieClip
static void sprite_duplicate_movieclip(const fn_call& fn)
{
	assert(dynamic_cast<sprite_instance*>(fn.this_ptr));
	sprite_instance* sprite = static_cast<sprite_instance*>(fn.this_ptr);
	if (sprite == NULL)
	{
	    sprite = dynamic_cast<sprite_instance*>(fn.env->get_target());
	}
	assert(sprite);
	
	if (fn.nargs < 2)
	{
	    log_error("duplicateMovieClip needs 2 or 3 args\n");
	    return;
	}

	// strk question: Would a call to 
	//   sprite->get_movie_defition()->create_instance()
	//   and an add_display_object taking a character_instance
	//   instead of a character *id* be more appropriate ?
	//   (sounds more general)

	// Copy event handlers from sprite
	// We should not copy 'm_action_buffer' since the 'm_method' already contains it
	std::vector<swf_event*>	event_handlers;
	const std::map<event_id, as_value>& sprite_events = sprite->get_event_handlers();
	typedef std::map<event_id, as_value>::const_iterator event_iterator;
	for (event_iterator it = sprite_events.begin(),
		itEnd = sprite_events.end();
		it != itEnd; ++it )
	{
		swf_event* e = new swf_event; // FIXME: who will delete this ?
		e->m_event = it->first;
		e->m_method = it->second;
		event_handlers.push_back(e);
	}

	character* parent = sprite->get_parent();
	sprite_instance* parent_sprite = parent ? parent->to_movie() : NULL;
	character* ch = NULL;
	if (parent_sprite)
	{
		ch = parent_sprite->add_display_object(
			sprite->get_id(),
			fn.arg(0).to_string(),
			event_handlers,
			int(fn.arg(1).to_number()),
			true, // replace if depth is occupied (to drop)
			sprite->get_cxform(),
			sprite->get_matrix(),
			sprite->get_ratio(),
			sprite->get_clip_depth());

		// Copy members from initObject
		if (fn.nargs == 3 && ch)
		{
			as_object* initObject = fn.arg(2).to_object();
			if ( initObject ) ch->copyProperties(*initObject);
		}

	}
	fn.result->set_as_object(ch);
}

static void sprite_goto_and_play(const fn_call& fn)
{
	assert(dynamic_cast<sprite_instance*>(fn.this_ptr));
	sprite_instance* sprite = static_cast<sprite_instance*>(fn.this_ptr);
	if (sprite == NULL)
	{
	    sprite = dynamic_cast<sprite_instance*>(fn.env->get_target());
	}
	assert(sprite);

	if (fn.nargs < 1)
	{
	    log_error("sprite_goto_and_play needs one arg\n");
	    return;
	}

	// Convert to 0-based
	size_t target_frame = size_t(fn.arg(0).to_number() - 1);

	sprite->goto_frame(target_frame);
	sprite->set_play_state(sprite_instance::PLAY);
}

static void sprite_goto_and_stop(const fn_call& fn)
{
	assert(dynamic_cast<sprite_instance*>(fn.this_ptr));
	sprite_instance* sprite = static_cast<sprite_instance*>(fn.this_ptr);
	if (sprite == NULL)
	{
	    sprite = dynamic_cast<sprite_instance*>(fn.env->get_target());
	}
	assert(sprite);

	if (fn.nargs < 1)
	{
	    log_error("sprite_goto_and_stop needs one arg\n");
	    return;
	}

	// Convert to 0-based
	size_t target_frame = size_t(fn.arg(0).to_number() - 1);

	sprite->goto_frame(target_frame);
	sprite->set_play_state(sprite_instance::STOP);
}

static void sprite_next_frame(const fn_call& fn)
{
	assert(dynamic_cast<sprite_instance*>(fn.this_ptr));
	sprite_instance* sprite = static_cast<sprite_instance*>(fn.this_ptr);
	if (sprite == NULL)
	{
	    sprite = dynamic_cast<sprite_instance*>(fn.env->get_target());
	}
	assert(sprite);

	size_t frame_count = sprite->get_frame_count();
	size_t current_frame = sprite->get_current_frame();
	if (current_frame < frame_count)
	{
	    sprite->goto_frame(current_frame + 1);
	}
	sprite->set_play_state(sprite_instance::STOP);
}

static void sprite_prev_frame(const fn_call& fn)
{
	assert(dynamic_cast<sprite_instance*>(fn.this_ptr));
	sprite_instance* sprite = static_cast<sprite_instance*>(fn.this_ptr);
	if (sprite == NULL)
	{
	    sprite = dynamic_cast<sprite_instance*>(fn.env->get_target());
	}
	assert(sprite);

	size_t current_frame = sprite->get_current_frame();
	if (current_frame > 0)
	{
	    sprite->goto_frame(current_frame - 1);
	}
	sprite->set_play_state(sprite_instance::STOP);
}

static void sprite_get_bytes_loaded(const fn_call& fn)
{
	assert(dynamic_cast<sprite_instance*>(fn.this_ptr));
	sprite_instance* sprite = static_cast<sprite_instance*>(fn.this_ptr);
	if (sprite == NULL)
	{
	    sprite = dynamic_cast<sprite_instance*>(fn.env->get_target());
	}
	assert(sprite);

	fn.result->set_int(sprite->get_bytes_loaded());
}

static void sprite_get_bytes_total(const fn_call& fn)
{
	assert(dynamic_cast<sprite_instance*>(fn.this_ptr));
	sprite_instance* sprite = static_cast<sprite_instance*>(fn.this_ptr);
	if (sprite == NULL)
	{
	    sprite = dynamic_cast<sprite_instance*>(fn.env->get_target());
	}
	assert(sprite);

	// @@ horrible uh ?
	fn.result->set_int(sprite->get_bytes_total());
}

static void sprite_load_movie(const fn_call& /* fn */)
{
	log_error("FIXME: %s not implemented yet", __PRETTY_FUNCTION__);
	//moviecliploader_loadclip(fn);
}

static void sprite_hit_test(const fn_call& fn)
{
	assert(dynamic_cast<sprite_instance*>(fn.this_ptr));
	//sprite_instance* sprite = static_cast<sprite_instance*>(fn.this_ptr);

	static bool warned_1_arg = false;
	static bool warned_2_arg = false;
	static bool warned_3_arg = false;

	switch (fn.nargs)
	{
		case 1: // target
		{
			as_value& tgt_val = fn.arg(0);
			character* target = fn.env->find_target(tgt_val);
			if ( ! target )
			{
				IF_VERBOSE_ASCODING_ERRORS(
				log_warning("Can't find hitTest target %s",
					tgt_val.to_string());
				);
				fn.result->set_undefined();
				return;
			}
			if ( ! warned_1_arg ) {
				log_warning("hitTest(target) unimplemented");
				warned_1_arg=true;
			}
			fn.result->set_undefined();
			break;
		}

		case 2: // x, y
		{
			double x = fn.arg(0).to_number();
			double y = fn.arg(1).to_number();
			if ( ! warned_2_arg ) {
				log_error("hitTest(%g,%g) unimplemented",
				x,y);
				warned_2_arg=true;
			}
			fn.result->set_undefined();
			break;
		}

		case 3: // x, y, shapeFlag
		{
			double x = fn.arg(0).to_number();
			double y = fn.arg(1).to_number();
			bool shapeFlag = fn.arg(2).to_bool();
			if ( ! warned_3_arg ) {
				log_error("hitTest(%g,%g,%d) unimplemented",
					x,y,shapeFlag);
				warned_3_arg=true;
			}
			fn.result->set_undefined();
			break;
		}

		default:
		{
			IF_VERBOSE_ASCODING_ERRORS(
				log_warning("hitTest() called with %u args."
				fn.nargs);
			);
			fn.result->set_undefined();
			break;
		}
	}

	return;

}

static void sprite_create_text_field(const fn_call& fn)
{
	as_object *target=fn.this_ptr;
	if ( target == NULL )
	{
		target=fn.env->get_target();
	}
	else
	{
		// I'm curious about when does fn.this_ptr
		// actually have a value!
		log_msg("-- %s: this_ptr(%p)!=target(%p) --\n",
			__PRETTY_FUNCTION__,
			(void*)fn.this_ptr, (void*)fn.env->get_target());
	}

	sprite_instance* sprite = dynamic_cast<sprite_instance*>(target);
	assert(sprite);

	if (fn.nargs != 6) // name, depth, x, y, width, height
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_msg("createTextField called with %d args, "
			"expected 6 - returning undefined", fn.nargs);
		);
		fn.result->set_undefined();
		return;
	}

	if ( fn.arg(0).get_type() != as_value::STRING )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_msg("First argument of createTextField is not a string"
			" - returning undefined");
		);
		fn.result->set_undefined();
		return;
	}
	//std::string txt_name = fn.arg(0).to_string();

	if ( fn.arg(1).get_type() != as_value::NUMBER)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_msg("Second argument of createTextField is not a number"
			" - returning undefined");
		);
		fn.result->set_undefined();
		return;
	}

	//double txt_depth = fn.arg(1).to_number();

	if ( fn.arg(2).get_type() != as_value::NUMBER)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_msg("Third argument of createTextField is not a number"
			" - returning undefined");
		);
		fn.result->set_undefined();
		return;
	}

	//double txt_x = fn.arg(2).to_number();

	if ( fn.arg(3).get_type() != as_value::NUMBER)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_msg("Fourth argument of createTextField is not a number"
			" - returning undefined");
		);
		fn.result->set_undefined();
		return;
	}

	//double txt_y = fn.arg(3).to_number();

	if ( fn.arg(4).get_type() != as_value::NUMBER )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_msg("Fifth argument of createTextField is not a number"
			" - returning undefined");
		);
		fn.result->set_undefined();
		return;
	}
	//double txt_width = fn.arg(4).to_number();

	if (fn.arg(5).get_type() != as_value::NUMBER)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_msg("Fifth argument of createTextField is not a number"
			" - returning undefined");
		);
		fn.result->set_undefined();
		return;
	}
	//double txt_height = fn.arg(5).to_number();


	// sprite is ourself, we should add a new
	// member, named txt_name and being a
	// TextField (text_character_def?)
	//

	// Get target's movie definition
	movie_definition *mds = sprite->get_movie_definition();

	//log_msg("Target's movie definition at %p\n", (void*)mds);

	// Do I need the smart_ptr.here ?
	boost::intrusive_ptr<text_character_def> txt = new text_character_def(mds);

	// Now add a the new TextField to the display list.

	// display_list::add wants a character, so we
	// must construct one.
	//
	// Unfortunately our text_character_def does
	// not inherit from character, but from character_def
	// which seems to be in another inheritance branch...
	//
	// Should we make text_character_def inherit from
	// character or define a new structure for TextField ?
	//
#if 0
	movie *m = sprite->get_root_movie();
	assert(m);

	character *txt_char = new character(m);
	assert(txt_char);
#endif


	log_error("FIXME: %s unfinished\n", __PRETTY_FUNCTION__);

#if 0
	// Here we add the character to the displayList.

	cxform txt_cxform;
	matrix txt_matrix;
	bool replace_if_depth_occupied = true;
	sprite->m_display_list->add_display_object(
		txt_char, txt_depth, replace_if_depth_occupied,
		txt_cxform, txt_matrix, txt_ratio, clip_depth
		);
#endif

	


	// We should return a ref to the newly created
	// TextField here
	
	//fn.result->set_as_object(txt.get());



	//assert(0); 
}

//getNextHighestDepth() : Number
static void
sprite_getNextHighestDepth(const fn_call& fn)
{
	sprite_instance* sprite = dynamic_cast<sprite_instance*>(fn.this_ptr);
	if (sprite == NULL)
	{
		// Handle programming errors
		IF_VERBOSE_ASCODING_ERRORS (
		log_error("getNextHighestDepth called against an object"
			" which is NOT a MovieClip (%s), "
			"returning undefined", typeid(fn.this_ptr).name());
		);
		fn.result->set_undefined();
		return;
	}

	unsigned int nextdepth = sprite->getNextHighestDepth();
	fn.result->set_double(static_cast<double>(nextdepth));
}

// getURL(url:String, [window:String], [method:String]) : Void
static void
sprite_getURL(const fn_call& fn)
{
	log_error("FIXME: MovieClip.getURL() not implemented yet");
}

// startDrag([lockCenter:Boolean], [left:Number], [top:Number],
// 	[right:Number], [bottom:Number]) : Void`
static void
sprite_startDrag(const fn_call& fn)
{
	log_error("FIXME: MovieClip.startDrag() not implemented yet");
}

// stopDrag() : Void
static void
sprite_stopDrag(const fn_call& /*fn*/)
{
	log_error("FIXME: MovieClip.stopDrag() not implemented yet");
}

static void
movieclip_ctor(const fn_call& fn)
{
	log_msg("User tried to invoke new MovieClip()");
	fn.result->set_undefined();
}

static void
attachMovieClipInterface(as_object& o)
{
	int target_version = o.getVM().getSWFVersion();

	// SWF5 or higher
	o.set_member("attachMovie", &sprite_attach_movie);
	o.set_member("play", &sprite_play);
	o.set_member("stop", &sprite_stop);
	o.set_member("gotoAndStop", &sprite_goto_and_stop);
	o.set_member("gotoAndPlay", &sprite_goto_and_play);
	o.set_member("nextFrame", &sprite_next_frame);
	o.set_member("prevFrame", &sprite_prev_frame);
	o.set_member("getBytesLoaded", &sprite_get_bytes_loaded);
	o.set_member("getBytesTotal", &sprite_get_bytes_total);
	o.set_member("loadMovie", &sprite_load_movie);
	o.set_member("hitTest", &sprite_hit_test);
	o.set_member("duplicateMovieClip", &sprite_duplicate_movieclip);
	o.set_member("swapDepths", &sprite_swap_depths);
	o.set_member("removeMovieClip", &sprite_remove_movieclip);
	o.set_member("startDrag", &sprite_startDrag);
	o.set_member("stopDrag", &sprite_stopDrag);
	o.set_member("getURL", &sprite_getURL);
	if ( target_version  < 6 ) return;

	// SWF6 or higher
	o.set_member("attachAudio", &sprite_attach_audio);
	o.set_member("createTextField", &sprite_create_text_field);
	o.set_member("getDepth", &sprite_get_depth);
	o.set_member("createEmptyMovieClip", &sprite_create_empty_movieclip);
	if ( target_version  < 7 ) return;

	// SWF7 or higher
	o.set_member("getNextHighestDepth", &sprite_getNextHighestDepth);
	if ( target_version  < 8 ) return;

	// TODO: many more methods, see MovieClip class ...

}

static as_object*
getMovieClipInterface()
{
	static boost::intrusive_ptr<as_object> proto;
	if ( proto == NULL )
	{
		proto = new as_object();
		attachMovieClipInterface(*proto);
		proto->set_member("constructor", &movieclip_ctor); 
		proto->set_member_flags("constructor", 1);
	}
	return proto.get();
}

void
movieclip_class_init(as_object& global)
{
	// This is going to be the global MovieClip "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl=NULL;

	if ( cl == NULL )
	{
		cl=new builtin_function(&movieclip_ctor, getMovieClipInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachMovieClipInterface(*cl);
		     
	}

	// Register _global.MovieClip
	global.set_member("MovieClip", cl.get());
}


//------------------------------------------------
// sprite_instance helper classes
//------------------------------------------------


class HeightFinder {
public:
	float _h;
	HeightFinder(): _h(0) {}
	bool operator() (character* ch)
	{
		float ch_h = ch->get_height();
		if (ch_h > _h) {
			_h = ch_h;
		}
		return true; // keep scanning
	}
	float getHeight() {
		return _h;
	}
};

class WidthFinder {
public:
	float _w;
	WidthFinder(): _w(0) {}
	bool operator() (character* ch) 
	{
		float ch_w = ch->get_width();
		if (ch_w > _w) {
			_w = ch_w;
		}
		return true; // keep scanning
	}
	float getWidth() {
		return _w;
	}
};

//------------------------------------------------
// sprite_instance
//------------------------------------------------

sprite_instance::sprite_instance(
		movie_definition* def, movie_root* r,
		character* parent, int id)
	:
	character(parent, id),
	m_mouse_state(UP),
	m_root(r),
	m_play_state(PLAY),
	m_current_frame(0),
	m_time_remainder(0),
	m_update_frame(true),
	m_has_looped(false),
	m_accept_anim_moves(true),
	m_frame_time(0.0f),
	m_has_keypress_event(false),
	m_def(def),
	m_on_event_load_called(false)
{
	assert(m_def != NULL);
	assert(m_root != NULL);

	set_prototype(getMovieClipInterface());
			
	//m_root->add_ref();	// @@ circular!
	m_as_environment.set_target(this);

	// Initialize the flags for init action executed.
	m_init_actions_executed.assign(m_def->get_frame_count(), false);
#if 0 // replaced with above line (sounds more readable)
//	m_init_actions_executed.resize(m_def->get_frame_count());
//	for (std::vector<bool>::iterator p = m_init_actions_executed.begin(); p != m_init_actions_executed.end(); ++p)
//	    {
//		*p = false;
//	    }
#endif

	// assert(m_root); // duplicated assert ...
	m_frame_time = 1.0f / m_root->get_frame_rate();	// cache
	m_time_remainder = m_frame_time;
}

sprite_instance::~sprite_instance()
{

	if (m_has_keypress_event)
	{
		m_root->remove_keypress_listener(this);
	}

	m_display_list.clear();
	//m_root->drop_ref();
}

character* sprite_instance::get_character_at_depth(int depth)
{
	return m_display_list.get_character_at_depth(depth);
}

// Set *val to the value of the named member and
// return true, if we have the named member.
// Otherwise leave *val alone and return false.
bool sprite_instance::get_member(const tu_stringi& name, as_value* val)
{
	// FIXME: use addProperty interface for these !!
	as_standard_member std_member = get_standard_member(name);
	switch (std_member)
	{
	default:
	case M_INVALID_MEMBER:
	    break;
	case M_X:
	    //if (name == "_x")
	{
	    matrix	m = get_matrix();
	    val->set_double(TWIPS_TO_PIXELS(m.m_[0][2]));
	    return true;
	}
	case M_Y:
	    //else if (name == "_y")
	{
	    matrix	m = get_matrix();
	    val->set_double(TWIPS_TO_PIXELS(m.m_[1][2]));
	    return true;
	}
	case M_XSCALE:
	    //else if (name == "_xscale")
	{
	    matrix m = get_matrix();	// @@ or get_world_matrix()?  Test this.
	    float xscale = m.get_x_scale();
	    val->set_double(xscale * 100);		// result in percent
	    return true;
	}
	case M_YSCALE:
	    //else if (name == "_yscale")
	{
	    matrix m = get_matrix();	// @@ or get_world_matrix()?  Test this.
	    float yscale = m.get_y_scale();
	    val->set_double(yscale * 100);		// result in percent
	    return true;
	}
	case M_CURRENTFRAME:
	    //else if (name == "_currentframe")
	{
	    val->set_int(m_current_frame + 1);
	    return true;
	}
	case M_TOTALFRAMES:
	    //else if (name == "_totalframes")
	{
	    // number of frames.  Read only.
	    val->set_int(m_def->get_frame_count());
	    return true;
	}
	case M_ALPHA:
	    //else if (name == "_alpha")
	{
	    // Alpha units are in percent.
	    val->set_double(get_cxform().m_[3][0] * 100.f);
	    return true;
	}
	case M_VISIBLE:
	    //else if (name == "_visible")
	{
	    val->set_bool(get_visible());
	    return true;
	}
	case M_WIDTH: // _width
	{
	    // Verified using samples/test_rotation.swf
	    val->set_double(TWIPS_TO_PIXELS(get_width()));
	    return true;
	}
	case M_HEIGHT: // _height
	{
	    // Verified using samples/test_rotation.swf
	    val->set_double(TWIPS_TO_PIXELS(get_height()));
	    return true;
	}
	case M_ROTATION:
	    //else if (name == "_rotation")
	{
	    // Verified against Macromedia player using samples/test_rotation.swf
	    float	angle = get_matrix().get_rotation();

	    // Result is CLOCKWISE DEGREES, [-180,180]
	    angle *= 180.0f / float(M_PI);

	    val->set_double(angle);
	    return true;
	}

	/// FIXME: use a contextual 'target' member
	case M_TARGET:
	    //else if (name == "_target")
	{
	    // Full path to this object; e.g. "/_level0/sprite1/sprite2/ourSprite"
	    val->set_string(getTargetPath().c_str());
	    return true;
	}
	case M_FRAMESLOADED:
	    //else if (name == "_framesloaded")
	{
	    val->set_int(m_def->get_loading_frame());
	    return true;
	}
	case M_NAME:
	    //else if (name == "_name")
	{
	    if ( _vm.getSWFVersion() < 6 && get_name().empty() )
		val->set_undefined();
	    else
	    	val->set_string(get_name().c_str());

	    return true;
	}
	case M_DROPTARGET:
	    //else if (name == "_droptarget")
	{
	    // Absolute path in slash syntax where we were last dropped (?)
	    // @@ TODO
		static bool warned = false;
		if ( ! warned ) {
			log_warning("FIXME: MovieClip._droptarget unimplemented");
			warned=true;
		}

	    if ( _vm.getSWFVersion() > 5 )
	    	val->set_string("");
	    else
		val->set_undefined();
	    return true;
	}

	///
	/// FIXME: add a valid 'url' member. Currently 
	/// the verbatim "gnash" value is assigned to it.
	/// The 'url' member should be inherited by
	/// parent *unless* we loaded an external resource
	/// into this movieclip.
	///
	case M_URL:
	    //else if (name == "_url")
	{
		// A sprite's url is the url this
		// sprite has been "downloaded" from.
		// If this is an ActionScript-created sprite
		// we might use the actions creating
		// it as defining where was it "downloaded" from.
		//
	    val->set_string(m_def->get_url().c_str()); // "gnash"
	    return true;
	}
	case M_HIGHQUALITY:
	    //else if (name == "_highquality")
	{
	    // Whether we're in high quality mode or not.
	    val->set_bool(true);
	    return true;
	}
	case M_FOCUSRECT:
	    //else if (name == "_focusrect")
	{
	    // Is a yellow rectangle visible around a focused movie clip (?)
	    val->set_bool(false);
	    return true;
	}
	case M_SOUNDBUFTIME:
	    //else if (name == "_soundbuftime")
	{
	    // Number of seconds before sound starts to stream.
	    val->set_double(0.0);
	    return true;
	}
	case M_XMOUSE:
	    //else if (name == "_xmouse")
	{
	    // Local coord of mouse IN PIXELS.
	    int	x, y, buttons;
	    assert(m_root);
	    m_root->get_mouse_state(&x, &y, &buttons);

	    matrix	m = get_world_matrix();

	    point	a(PIXELS_TO_TWIPS(x), PIXELS_TO_TWIPS(y));
	    point	b;
			
	    m.transform_by_inverse(&b, a);

	    val->set_double(TWIPS_TO_PIXELS(b.m_x));
	    return true;
	}
	case M_YMOUSE:
	    //else if (name == "_ymouse")
	{
	    // Local coord of mouse IN PIXELS.
	    int	x, y, buttons;
	    assert(m_root);
	    m_root->get_mouse_state(&x, &y, &buttons);

	    matrix	m = get_world_matrix();

	    point	a(PIXELS_TO_TWIPS(x), PIXELS_TO_TWIPS(y));
	    point	b;
			
	    m.transform_by_inverse(&b, a);

	    val->set_double(TWIPS_TO_PIXELS(b.m_y));
	    return true;
	}
	case M_PARENT:
	{
		if (m_parent==NULL)
		// _parent is undefined for root movies
		{
			return false;
		}
		else
		{
			assert(dynamic_cast<as_object*>(get_parent()));
			val->set_as_object(static_cast<as_object*>(get_parent()));
			return true;
		}
	}
	case M_ONLOAD:
	{
	    if (m_as_environment.get_member(std::string(name.c_str()), val))
		{
		    return true;
		}
	    // Optimization: if no hit, don't bother looking in the display list, etc.
	    return false;
	}
		case M_ONROLLOVER:
		{
			return get_event_handler(event_id::ROLL_OVER, val);
		}
		case M_ONROLLOUT:
		{
			return get_event_handler(event_id::ROLL_OUT, val);
		}
	}	// end switch

	// Try variables.
	if (m_as_environment.get_member(std::string(name.c_str()), val))
	{
	    return true;
	}

	// Not a built-in property.  Check items on our
	// display list.
	character* ch = m_display_list.get_character_by_name_i(std::string(name.c_str()));
	if (ch)
	{
	    // Found object.
	    val->set_as_object(static_cast<as_object*>(ch));
	    return true;
	}

	// Try textfield variables
	edit_text_character* etc = get_textfield_variable(name.c_str());
	if ( etc )
	{
	    	val->set_string(etc->get_text_value());
	}

	// Invoke the default get_member 
	return get_member_default(name, val);

}

// Take care of this frame's actions.
void sprite_instance::do_actions()
{
	testInvariant();

	execute_actions(m_action_list);
	assert(m_action_list.empty());

	testInvariant();
}

size_t
sprite_instance::get_frame_number(const as_value& frame_spec) const
{
	size_t frame_number;

	// Figure out what frame to call.
	if (frame_spec.get_type() == as_value::STRING)
	{
		if (m_def->get_labeled_frame(frame_spec.to_string(), &frame_number) == false)
		{
			// Try converting to integer.
			frame_number = (size_t)frame_spec.to_number();
		}
	}
	else
	{
		// convert from 1-based to 0-based
		frame_number = (size_t) frame_spec.to_number() - 1;
	}

	return frame_number;
}

/// Execute the actions for the specified frame. 
//
/// The frame_spec could be an integer or a string.
///
void sprite_instance::call_frame_actions(const as_value& frame_spec)
{
	size_t	frame_number = get_frame_number(frame_spec);


	if (frame_number >= m_def->get_frame_count())
	{
		    // No dice.
		    log_error("call_frame('%s') -- unknown frame\n", frame_spec.to_string());
		    return;
	}

	// Take not of iterator to last element
	ActionList::iterator top_iterator = m_action_list.end();
	--top_iterator; // now points to last element in *current* list

#ifndef NDEBUG
	size_t original_size = m_action_list.size();
#endif

	// Set the current sound_stream_id to -1, meaning that no stream are
	// active. If there are an active stream it will be updated while
	// executing the execute_tags.
	set_sound_stream_id(-1);

	// Execute the execute_tag actions

	const PlayList& playlist = m_def->get_playlist(frame_number);
	for (size_t i=0, n=playlist.size(); i<n; ++i)
	{
		execute_tag*	e = playlist[i];
		if (e->is_action_tag())
		{
			e->execute(this);
		}
	}

	// Execute any new actions triggered by the tag,
	// leaving existing actions to be executed.

	++top_iterator; // now points to one past last of *previous* list
	ActionList::const_iterator it = top_iterator;
	while (it != m_action_list.end())
	{
		execute_action(*(*it));
		++it;
	}
	m_action_list.erase(top_iterator, m_action_list.end());

	assert(m_action_list.size() == original_size);
}

character* sprite_instance::add_empty_movieclip(const char* name, int depth)
{
	cxform color_transform;
	matrix matrix;

	// empty_sprite_def will be deleted during deliting sprite
	sprite_definition* empty_sprite_def = new sprite_definition(NULL, NULL);

	sprite_instance* sprite =	new sprite_instance(empty_sprite_def, m_root, this, 0);
	sprite->set_name(name);

	m_display_list.place_character(
		sprite,
		depth,
		color_transform,
		matrix,
		0.0f,
		0); 

	return sprite;
}

void sprite_instance::clone_display_object(const std::string& name,
	const std::string& newname, uint16_t depth)
{
//            GNASH_REPORT_FUNCTION;

    character* ch = m_display_list.get_character_by_name(name);
    if (ch)
	{
	    std::vector<swf_event*>	dummy_event_handlers;

	    add_display_object(
		ch->get_id(),
		newname.c_str(),
		dummy_event_handlers,
		depth,
		true, // replace if depth is occupied (to drop)
		ch->get_cxform(),
		ch->get_matrix(),
		ch->get_ratio(),
		ch->get_clip_depth());
	    // @@ TODO need to duplicate ch's event handlers, and presumably other members?
	    // Probably should make a character::clone() function to handle this.
	}
}

#if 1
void sprite_instance::remove_display_object(const tu_string& name_tu)
{
//	    GNASH_REPORT_FUNCTION;

	std::string name(name_tu.c_str());

	character* ch = m_display_list.get_character_by_name(name);
	if (ch)
	{
	    // @@ TODO: should only remove movies that were created via clone_display_object --
	    // apparently original movies, placed by anim events, are immune to this.
	    remove_display_object(ch->get_depth(), ch->get_id());
	}
}
#endif

bool sprite_instance::on_event(const event_id& id)
{
	testInvariant();

	bool called = false;
			
	// First, check for built-in event handler.
	{
		as_value	method;
		if (get_event_handler(id, &method))
		{
			// Dispatch.
			call_method0(method, &m_as_environment, this);

			called = true;
			// Fall through and call the function also, if it's defined!
			// (@@ Seems to be the behavior for mouse events; not tested & verified for
			// every event type.)
		}
	}

	// Check for member function.
	{
		// In ActionScript 2.0, event method names are CASE SENSITIVE.
		// In ActionScript 1.0, event method names are CASE INSENSITIVE.
		const tu_stringi&	method_name = id.get_function_name();
		if (method_name.length() > 0)
		{
			as_value	method;
			if (get_member(method_name, &method))
			{
				call_method0(method, &m_as_environment, this);
				called = true;
			}
		}
	}

	testInvariant();

	return called;
}

character*
sprite_instance::get_relative_target(const std::string& name)
{
	character* ch = get_relative_target_common(name);

	if ( ! ch )
	{
		// See if we have a match on the display list.
		return m_display_list.get_character_by_name(name);
	}

	return ch; // possibly NULL
}

void sprite_instance::set_member(const tu_stringi& name,
		const as_value& val)
{
#ifdef DEBUG_DYNTEXT_VARIABLES
log_msg("sprite[%p]::set_member(%s, %s)", (void*)this, name.c_str(), val.to_string());
#endif

	as_standard_member	std_member = get_standard_member(name);
	switch (std_member)
	{
		default:
		case M_INVALID_MEMBER:
		    break;
		case M_X:
		    //if (name == "_x")
		{
		    matrix	m = get_matrix();
		    m.m_[0][2] = infinite_to_fzero(PIXELS_TO_TWIPS(val.to_number()));
		    set_matrix(m);

		    m_accept_anim_moves = false;

		    return;
		}
		case M_Y:
		    //else if (name == "_y")
		{
		    matrix	m = get_matrix();
		    m.m_[1][2] = infinite_to_fzero(PIXELS_TO_TWIPS(val.to_number()));
		    set_matrix(m);

		    m_accept_anim_moves = false;

		    return;
		}
		case M_XSCALE:
		    //else if (name == "_xscale")
		{
		    matrix	m = get_matrix();

                    double scale_percent = val.to_number();

                    // Handle bogus values
                    if (isnan(scale_percent))
                    {
			log_warning("Attempt to set _xscale to %g, refused",
                            scale_percent);
                        return;
                    }
                    else if (scale_percent < 0 )
                    {
			log_warning("Attempt to set _xscale to %g, use 0",
                            scale_percent);
                        scale_percent = 0;
                    }
//                    else if (scale_percent > 100 )
//                    {
//			log_warning("Attempt to set _xscale to %g, use 100",
//                            scale_percent);
//                        scale_percent = 100;
//                    }

                    // input is in percent
		    float scale = (float)scale_percent/100.f;

		    // Decompose matrix and insert the desired value.
		    float	x_scale = scale;
		    float	y_scale = m.get_y_scale();
		    float	rotation = m.get_rotation();
		    m.set_scale_rotation(x_scale, y_scale, rotation);

		    set_matrix(m);
		    m_accept_anim_moves = false;
		    return;
		}
		case M_YSCALE:
		    //else if (name == "_yscale")
		{
		    matrix	m = get_matrix();

                    double scale_percent = val.to_number();

                    // Handle bogus values
                    if (isnan(scale_percent))
                    {
			log_warning("Attempt to set _yscale to %g, refused",
                            scale_percent);
                        return;
                    }
                    else if (scale_percent < 0 )
                    {
			log_warning("Attempt to set _yscale to %g, use 0",
                            scale_percent);
                        scale_percent = 0;
                    }
//                    else if (scale_percent > 100 )
//                    {
//			log_warning("Attempt to set _yscale to %g, use 100",
//                            scale_percent);
//                        scale_percent = 100;
//                    }

                    // input is in percent
		    float scale = (float)scale_percent/100.f;

		    // Decompose matrix and insert the desired value.
		    float	x_scale = m.get_x_scale();
		    float	y_scale = scale;
		    float	rotation = m.get_rotation();
		    m.set_scale_rotation(x_scale, y_scale, rotation);

		    set_matrix(m);
		    m_accept_anim_moves = false;
		    return;
		}
		case M_ALPHA:
		    //else if (name == "_alpha")
		{
		    // Set alpha modulate, in percent.
		    cxform	cx = get_cxform();
		    cx.m_[3][0] = infinite_to_fzero(val.to_number()) / 100.f;
		    set_cxform(cx);
		    m_accept_anim_moves = false;
		    return;
		}
		case M_VISIBLE:
		    //else if (name == "_visible")
		{
		    set_visible(val.to_bool());
		    m_accept_anim_moves = false;
		    return;
		}
		case M_WIDTH:
		    //else if (name == "_width")
		{
		    // @@ tulrich: is parameter in world-coords or local-coords?
		    matrix	m = get_matrix();
		    m.m_[0][0] = infinite_to_fzero(PIXELS_TO_TWIPS(val.to_number()));
		    float w = get_width();
		    if (fabsf(w) > 1e-6f)
			{
			    m.m_[0][0] /= w;
			}
		    set_matrix(m);
		    m_accept_anim_moves = false;
		    return;
		}
		case M_HEIGHT:
		    //else if (name == "_height")
		{
		    // @@ tulrich: is parameter in world-coords or local-coords?
		    matrix	m = get_matrix();
		    m.m_[1][1] = infinite_to_fzero(PIXELS_TO_TWIPS(val.to_number()));
		    float h = get_width();
		    if (fabsf(h) > 1e-6f)
			{
			    m.m_[1][1] /= h;
			}
		    set_matrix(m);
		    m_accept_anim_moves = false;
		    return;
		}
		case M_ROTATION:
		    //else if (name == "_rotation")
		{
		    matrix	m = get_matrix();

		    // Decompose matrix and insert the desired value.
		    float	x_scale = m.get_x_scale();
		    float	y_scale = m.get_y_scale();
		    float	rotation = (float) val.to_number() * float(M_PI) / 180.f;	// input is in degrees
		    m.set_scale_rotation(x_scale, y_scale, rotation);

		    set_matrix(m);
		    m_accept_anim_moves = false;
		    return;
		}
		case M_HIGHQUALITY:
		    //else if (name == "_highquality")
		{
		    // @@ global { 0, 1, 2 }
//				// Whether we're in high quality mode or not.
//				val->set(true);
		    return;
		}
		case M_FOCUSRECT:
		    //else if (name == "_focusrect")
		{
//				// Is a yellow rectangle visible around a focused movie clip (?)
//				val->set(false);
		    return;
		}
		case M_SOUNDBUFTIME:
		    //else if (name == "_soundbuftime")
		{
		    // @@ global
//				// Number of seconds before sound starts to stream.
//				val->set(0.0);
		    return;
		}
		case M_ONROLLOVER:
		{
			set_event_handler(event_id::ROLL_OVER, val);
			return;
		}
		case M_ONROLLOUT:
		{
			set_event_handler(event_id::ROLL_OUT, val);
			return;
		}
	}	// end switch

	// Not a built-in property.  See if we have a
	// matching edit_text character in our display
	// list.
	bool text_val = val.get_type() == as_value::STRING
		|| val.get_type() == as_value::NUMBER;
	if (text_val)
	{
			// CASE INSENSITIVE compare. 
			// In ActionScript 2.0, this must change
			// to CASE SENSITIVE!!!
			character* ch = m_display_list.get_character_by_name_i( name.c_str());
			if ( ch ) // item found
			{
				const char* text = val.to_string();
				ch->set_text_value(text);
				return;
			}
	}

#ifdef DEBUG_DYNTEXT_VARIABLES
log_msg(" not a standard member nor a character");
#endif

	// Try textfield variables
	//
	// FIXME: Turn textfield variables into Getter/Setters (Properties)
	//        so that set_member_default will do this automatically.
	//        The problem is that setting a TextVariable named after
	//        a builtin property will prevent *any* setting for the
	//        property (ie: have a textfield use _x as variable name and
	//        be scared)
	//
	edit_text_character* etc = get_textfield_variable(name.c_str());
	if ( etc )
	{
#ifdef DEBUG_DYNTEXT_VARIABLES
log_msg(" it's a Text Variable!");
#endif
		etc->set_text_value(val.to_string());
	}
#ifdef DEBUG_DYNTEXT_VARIABLES
	else
	{
log_msg(" it's NOT a Text Variable!");
	}
#endif

	// If that didn't work call the default set_member
	set_member_default(name, val);


	// If that didn't work, set a variable within this environment.
	// TODO: check if we broke anything with this!
	//if ( ! t ) m_as_environment.set_member(name.c_str(), val);
}

const char* sprite_instance::get_variable(const char* path_to_var) const
{
    assert(m_parent == NULL);	// should only be called on the root movie.

    std::string path(path_to_var);

    // NOTE: this is static so that the string
    // value won't go away after we return!!!
    // It'll go away during the next call to this
    // function though!!!  NOT THREAD SAFE!
    static as_value	val;

    val = m_as_environment.get_variable(path);

    return val.to_string();	// ack!
}

void sprite_instance::set_variable(const char* path_to_var,
		const wchar_t* new_value)
{
	if (path_to_var == NULL)
	{
		log_error("NULL path_to_var passed to set_variable()\n");
		return;
	}
	if (new_value == NULL)
	{
		log_error("NULL passed to set_variable('%s',"
			" NULL)\n", path_to_var);
		return;
	}

	// should only be called on the root movie.
	assert(m_parent == NULL);

	std::string path(path_to_var);
	as_value val(new_value);

	m_as_environment.set_variable(path, val);
}

void sprite_instance::set_variable(const char* path_to_var,
		const char* new_value)
{
	    assert(m_parent == NULL);	// should only be called on the root movie.

	    if (path_to_var == NULL)
		{
		    log_error("NULL path_to_var passed to set_variable()\n");
		    return;
		}
	    if (new_value == NULL)
		{
		    log_error("NULL passed to set_variable('%s', NULL)\n", path_to_var);
		    return;
		}

	    std::string path(path_to_var);
	    as_value val(new_value);

	    m_as_environment.set_variable(path, val);
}

void sprite_instance::has_keypress_event()
{
	m_has_keypress_event = true;
}

void sprite_instance::advance_sprite(float delta_time)
{
	//GNASH_REPORT_FUNCTION;

	// mouse drag.
	character::do_mouse_drag();

	if (m_on_event_load_called)
	{
		on_event(event_id::ENTER_FRAME);
	}

	size_t frame_count = m_def->get_frame_count();

	// Update current and next frames.
	if (m_play_state == PLAY)
	{
		int prev_frame = m_current_frame;
		if (m_on_event_load_called)
		{
			increment_frame_and_check_for_loop();
		}

		// Execute the current frame's tags.
		// First time execute_frame_tags(0) executed in dlist.cpp(child) or movie_def_impl(root)
	if (m_current_frame != (size_t)prev_frame)
		{
			//Vitaly:
			// Macromedia Flash does not call remove display object tag
			// for 1-st frame therefore we should do it for it :-)
			if (m_current_frame == 0 && frame_count > 1)
			{
			 	set_invalidated();

				// affected depths
				const PlayList& playlist = m_def->get_playlist(0);
				std::vector<uint16> affected_depths;
				for (unsigned int i = 0; i < playlist.size(); i++)
				{
					uint16 depth = (playlist[i]->get_depth_id_of_replace_or_add_tag()) >> 16;
					if (depth != static_cast<uint16>(-1))
					{
						affected_depths.push_back(depth);
					}
				}

				if (affected_depths.size() > 0)
				{
					m_display_list.clear_unaffected(affected_depths);					
				}
				else
				{
					m_display_list.clear();
			 	}
			 	
			}
			execute_frame_tags(m_current_frame);
		}
	}

	do_actions();

	// Advance everything in the display list.
	m_display_list.advance(delta_time);

	execute_actions(m_goto_frame_action_list);
	assert(m_goto_frame_action_list.empty());
}

// child movieclip advance
void sprite_instance::advance(float delta_time)
{
//	GNASH_REPORT_FUNCTION;

	// Vitaly:
	// child movieclip frame rate is the same the root movieclip frame rate
	// that's why it is not needed to analyze 'm_time_remainder' 
	if (m_on_event_load_called == false)
	{
		on_event(event_id::LOAD);	// clip onload

		//
		if (m_has_keypress_event)
		{
			m_root->add_keypress_listener(this);
		}
	}
	
	advance_sprite(delta_time);

	m_on_event_load_called = true;
}

void
sprite_instance::execute_action(action_buffer& ab)
{
	as_environment& env = m_as_environment; // just type less

	int local_stack_top = env.get_local_frame_top();

	env.add_frame_barrier();

	ActionExec exec(ab, env);
	exec();

	env.set_local_frame_top(local_stack_top);
}

void
sprite_instance::execute_frame_tags(size_t frame, bool state_only)
{
	testInvariant();

	assert(frame < m_def->get_frame_count());

	// Execute this frame's init actions, if necessary.
	if (m_init_actions_executed[frame] == false)
	{

		const PlayList* init_actions = m_def->get_init_actions(frame);

		if ( init_actions && ! init_actions->empty() )
		{

			IF_VERBOSE_ACTION(
				log_action("Executing " SIZET_FMT 
					" *init* actions in frame " SIZET_FMT
					" of sprite %s", init_actions->size(),
					frame, getTargetPath().c_str());
			);


			// Need to execute these actions.
			std::for_each(init_actions->begin(), init_actions->end(),
				std::bind2nd(std::mem_fun(&execute_tag::execute), this));

			// Mark this frame done, so we never execute these
			// init actions again.
			m_init_actions_executed[frame] = true;
		}
	}

	const PlayList& playlist = m_def->get_playlist(frame);

	IF_VERBOSE_ACTION(
		log_action("Executing " SIZET_FMT " actions in frame "
			SIZET_FMT " of sprite %s %s",
			playlist.size(), frame, getTargetPath().c_str(),
			state_only ? "(state only)" : "" );
	);

	if (state_only)
	{
		std::for_each(playlist.begin(), playlist.end(),
			std::bind2nd(std::mem_fun(&execute_tag::execute_state), this));
	}
	else
	{
		std::for_each(playlist.begin(), playlist.end(),
			std::bind2nd(std::mem_fun(&execute_tag::execute), this));
	}

	testInvariant();
}

void sprite_instance::execute_frame_tags_reverse(size_t frame)
{
	testInvariant();

	assert(frame < m_def->get_frame_count());

	const PlayList& playlist = m_def->get_playlist(frame);

	for (unsigned int i=0, n=playlist.size(); i<n; ++i)
	{
	    execute_tag*	e = playlist[i];
	    e->execute_state_reverse(this, frame);
	}

	testInvariant();
}

void sprite_instance::execute_remove_tags(int frame)
{
	    assert(frame >= 0);
	    assert((size_t)frame < m_def->get_frame_count());

	    const PlayList& playlist = m_def->get_playlist(frame);
	    for (unsigned int i = 0; i < playlist.size(); i++)
		{
		    execute_tag*	e = playlist[i];
		    if (e->is_remove_tag())
			{
			    e->execute_state(this);
			}
		}
}

execute_tag*
sprite_instance::find_previous_replace_or_add_tag(int frame,
		int depth, int id)
{
	uint32 depth_id = ((depth & 0x0FFFF) << 16) | (id & 0x0FFFF);

	for (int f = frame - 1; f >= 0; f--)
	{
	    const PlayList& playlist = m_def->get_playlist(f);
	    for (int i = playlist.size() - 1; i >= 0; i--)
		{
		    execute_tag*	e = playlist[i];
		    if (e->get_depth_id_of_replace_or_add_tag() == depth_id)
			{
			    return e;
			}
		}
	}

    return NULL;
}

void
sprite_instance::goto_frame(size_t target_frame_number)
{
//	IF_VERBOSE_DEBUG(log_msg("sprite::goto_frame(%d)\n", target_frame_number));//xxxxx

	//	target_frame_number = iclamp(target_frame_number, 0, m_def->get_frame_count() - 1);
	// Macromedia Flash ignores goto_frame(bad_frame)
	if (target_frame_number > m_def->get_frame_count() - 1 ||
			target_frame_number == m_current_frame)	// to prevent infinitive recursion
	{
		set_play_state(STOP);
		return;
	}

	// Unless the target frame is the next one, stop playback of soundstream
	int stream_id = get_sound_stream_id();
	if (target_frame_number != m_current_frame+1 && stream_id != -1) {
		sound_handler* sh = get_sound_handler();
		if (sh != NULL) sh->stop_sound(stream_id);
		set_sound_stream_id(-1);
	}

	size_t loaded_frames = get_loaded_frames();
	if ( target_frame_number > loaded_frames )
	{
#if 0 // debugging
		log_msg("loaded frames: %u, target frame number: %u",
			loaded_frames, target_frame_number);
#endif
		// we might be asking for an already loaded frame
		// (consider a backward goto and then a forward goto)
		m_def->ensure_frame_loaded(target_frame_number);
	}


	if (target_frame_number < m_current_frame)
	{
		for (size_t f = m_current_frame; f>target_frame_number; --f)
		{
			execute_frame_tags_reverse(f);
		}
		m_action_list.clear();
		execute_frame_tags(target_frame_number, false);
		//we don't have the concept of a DisplayList update anymore
		//m_display_list.update();

	}
	else if (target_frame_number > m_current_frame)
	{
		for (size_t f = m_current_frame+1; f<target_frame_number; ++f)
		{
			execute_frame_tags(f, true);
		}

		m_action_list.clear();
		execute_frame_tags(target_frame_number, false);
		//we don't have the concept of a DisplayList update anymore
		//m_display_list.update();

	}

	m_current_frame = target_frame_number;      

	// goto_frame stops by default.
	set_play_state(STOP);

	// After entering to advance_sprite() m_current_frame points to frame
	// that already is executed. 
	// Macromedia Flash do goto_frame then run actions from this frame.
	// We do too.

	m_goto_frame_action_list = m_action_list; 
	m_action_list.clear();

}

bool sprite_instance::goto_labeled_frame(const char* label)
{
	size_t target_frame;
	if (m_def->get_labeled_frame(label, &target_frame))
	{
		goto_frame(target_frame);
		return true;
	}
	else
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_warning("sprite_instance::goto_labeled_frame('%s') "
			"unknown label\n", label);
		);
		return false;
	}
}

void sprite_instance::display()
{
//	GNASH_REPORT_FUNCTION;

	if (get_visible() == false)	{
		// We're invisible, so don't display!
		
		// Note: dlist.cpp will avoid to even call display() so this will probably
		// never happen.
		return;
	}
	
	// check if the sprite (and it's childs) needs to be drawn 
	rect bounds;
	m_display_list.get_invalidated_bounds(&bounds, true);
	
	if (gnash::render::bounds_in_clipping_area(bounds)) {
	  m_display_list.display();
	  clear_invalidated();
	}
	  
	do_display_callback();
}

void sprite_instance::swap_characters(character* ch1, character* ch2)
{
	ch1->set_invalidated();
	ch2->set_invalidated();

	m_display_list.swap_characters(ch1, ch2);	
}

character*
sprite_instance::add_display_object(
		uint16_t character_id,
		const char* name,
		const std::vector<swf_event*>& event_handlers,
		uint16_t depth, 
		bool /* replace_if_depth_is_occupied */,
		const cxform& color_transform, const matrix& matrix,
		float ratio, uint16_t clip_depth)
{
//	    GNASH_REPORT_FUNCTION;
	    assert(m_def != NULL);

	    character_def*	cdef = m_def->get_character_def(character_id);
	    if (cdef == NULL)
		{
		    log_error("sprite::add_display_object(): unknown cid = %d\n", character_id);
		    return NULL;
		}

	    // If we already have this object on this
	    // plane, then move it instead of replacing
	    // it.
	    character*	existing_char = m_display_list.get_character_at_depth(depth);
	    if (existing_char
		&& existing_char->get_id() == character_id
		&& ((name == NULL && existing_char->get_name().length() == 0)
		    || (name && existing_char->get_name() == name)))
		{
//			IF_VERBOSE_DEBUG(log_msg("add changed to move on depth %d\n", depth));//xxxxxx
			// compare events 
			const Events& existing_events = existing_char->get_event_handlers();
size_t n = event_handlers.size();
if (existing_events.size() == n)
{
	bool same_events = true;
	for (size_t i = 0; i < n; i++)
	{
		Events::const_iterator it = existing_events.find(event_handlers[i]->m_event);
		if ( it != existing_events.end() )
		{
			as_value result = it->second;
			// compare actionscipt in event
			if (event_handlers[i]->m_method == result)
			{
				continue;
			}
		}
		same_events = false;
		break;
	}
	
	if (same_events)
	{
		move_display_object(depth, true, color_transform, true, matrix, ratio, clip_depth);
		return NULL;
	}
}
		}
	    //printf("%s: character %s, id is %d, count is %d\n", __FUNCTION__, existing_char->get_name(), character_id,m_display_list.get_character_count()); // FIXME:

	    assert(cdef);
	    boost::intrusive_ptr<character> ch = cdef->create_character_instance(this,
			character_id);
	    assert(ch.get() != NULL);
	    if (name != NULL && name[0] != 0)
		{
		    ch->set_name(name);
		}

	    // Attach event handlers (if any).
	    {for (int i = 0, n = event_handlers.size(); i < n; i++)
		{
		    event_handlers[i]->attach_to(ch.get());
		}}

	    m_display_list.place_character(
		ch.get(),
		depth,
		color_transform,
		matrix,
		ratio,
		clip_depth);

	    assert(ch == NULL || ch->get_ref_count() > 1);
	    return ch.get();
}

void
sprite_instance::replace_display_object(
		uint16_t character_id,
		const char* name,
		uint16_t depth,
		bool use_cxform,
		const cxform& color_transform,
		bool use_matrix,
		const matrix& mat,
		float ratio,
		uint16_t clip_depth)
{
	assert(m_def != NULL);
	// printf("%s: character %s, id is %d\n", __FUNCTION__, name, character_id); // FIXME: debugging crap

	character_def*	cdef = m_def->get_character_def(character_id);
	if (cdef == NULL)
	{
		log_error("sprite::replace_display_object(): "
			"unknown cid = %d\n", character_id);
		return;
	}
	assert(cdef);

	boost::intrusive_ptr<character> ch = cdef->create_character_instance(this,
			character_id);

	replace_display_object(
		ch.get(), name, depth,
		use_cxform, color_transform,
		use_matrix, mat,
		ratio, clip_depth);
}

void sprite_instance::replace_display_object(
		character* ch,
		const char* name,
		uint16_t depth,
		bool use_cxform,
		const cxform& color_transform,
		bool use_matrix,
		const matrix& mat,
		float ratio,
		uint16_t clip_depth)
{
    //printf("%s: character %s, id is %d\n", __FUNCTION__, name, ch->get_id()); // FIXME:

    assert(ch != NULL);

    if (name != NULL && name[0] != 0)
	{
	    ch->set_name(name);
	}

	 set_invalidated();

    m_display_list.replace_character(
	ch,
	depth,
	use_cxform,
	color_transform,
	use_matrix,
	mat,
	ratio,
	clip_depth);
	
}

int sprite_instance::get_id_at_depth(int depth)
{
    character*	ch = m_display_list.get_character_at_depth(depth);
    if ( ! ch ) return -1;
    else return ch->get_id();
}

void sprite_instance::increment_frame_and_check_for_loop()
{
	//GNASH_REPORT_FUNCTION;

	size_t frame_count = m_def->get_frame_count();
	if ( ++m_current_frame >= frame_count )
	{
		// Loop.
		m_current_frame = 0;
		m_has_looped = true;
		if (frame_count > 1)
		{
			//m_display_list.reset();
		}
	}

#if 0 // debugging
	log_msg("Frame %u/%u, bytes %u/%u",
		m_current_frame, frame_count,
		get_bytes_loaded(), get_bytes_total());
#endif
}

/// Find a character hit by the given coordinates.
class MouseEntityFinder {

	character* _m;

	float _x;

	float _y;

public:

	MouseEntityFinder(float x, float y)
		:
		_m(NULL),
		_x(x),
		_y(y)
	{}

	bool operator() (character* ch)
	{
		if ( ! ch->get_visible() ) return true;

		character* te = ch->get_topmost_mouse_entity(_x, _y);
		if ( te )
		{
			_m = te;
			return false; // done
		}

		return true; // haven't found it yet
	}

	character* getEntity() { return _m; }
		
};

character*
sprite_instance::get_topmost_mouse_entity(float x, float y)
{
	//GNASH_REPORT_FUNCTION;

	if (get_visible() == false)
	{
		return NULL;
	}

	matrix	m = get_matrix();
	point	p;
	m.transform_by_inverse(&p, point(x, y));

	MouseEntityFinder finder(p.m_x, p.m_y);
	m_display_list.visitBackward(finder);
	character* ch = finder.getEntity();

	if ( ch && can_handle_mouse_event() )
	{
		return this;
	}
	else
	{
		return ch; // might be NULL
	}

}

bool
sprite_instance::can_handle_mouse_event()
{
    // We should cache this!
    as_value dummy;

    // Functions that qualify as mouse event handlers.
    const char* FN_NAMES[] = {
	"onKeyPress",
	"onRelease",
	"onDragOver",
	"onDragOut",
	"onPress",
	"onReleaseOutside",
	"onRollout",
	"onRollover",
    };
    for (unsigned int i = 0; i < ARRAYSIZE(FN_NAMES); i++) {
	if (get_member(FN_NAMES[i], &dummy)) {
	    return true;
	}
    }

    // Event handlers that qualify as mouse event handlers.
    const event_id::id_code EH_IDS[] = {
	event_id::PRESS,
	event_id::RELEASE,
	event_id::RELEASE_OUTSIDE,
	event_id::ROLL_OVER,
	event_id::ROLL_OUT,
	event_id::DRAG_OVER,
	event_id::DRAG_OUT,
    };
    {for (unsigned int i = 0; i < ARRAYSIZE(EH_IDS); i++) {
	if (get_event_handler(EH_IDS[i], &dummy)) {
	    return true;
	}
    }}

    return false;
}
		
void sprite_instance::restart()
{
    m_current_frame = 0;
    m_time_remainder = 0;
    m_update_frame = true;
    m_has_looped = false;
    m_play_state = PLAY;

    execute_frame_tags(m_current_frame);
    //we don't have the concept of a DisplayList update anymore
    //m_display_list.update();
}

float sprite_instance::get_height() const
{
	HeightFinder f;
	// the const_cast is just to avoid defining a const version
	// of DisplayList::visitForward, HeightFinder will NOT
	// modify the DisplayList elements in any way
	const_cast<DisplayList&>(m_display_list).visitForward(f);
	return f.getHeight(); 
}

float sprite_instance::get_width() const
{
	WidthFinder f;
	// the const_cast is just to avoid defining a const version
	// of DisplayList::visitForward, WidthFinder will NOT
	// modify the DisplayList elements in any way
	const_cast<DisplayList&>(m_display_list).visitForward(f);
	return f.getWidth(); 
}

void sprite_instance::do_something(void *timer)
{
    as_value	val;
    as_object      *obj, *this_ptr;
    as_environment *as_env;

    //printf("FIXME: %s:\n", __FUNCTION__);
    Timer *ptr = (Timer *)timer;
    //log_msg("INTERVAL ID is %d\n", ptr->getIntervalID());

    const as_value&	timer_method = ptr->getASFunction();
    as_env = ptr->getASEnvironment();
    this_ptr = ptr->getASObject();
    obj = ptr->getObject();
    //m_as_environment.push(obj);
		
    as_c_function_ptr	cfunc = timer_method.to_c_function();
    if (cfunc) {
	// It's a C function. Call it.
	//log_msg("Calling C function for interval timer\n");
	//(*cfunc)(&val, obj, as_env, 0, 0);
	(*cfunc)(fn_call(&val, obj, &m_as_environment, 0, 0));
			
    } else if (as_function* as_func = timer_method.to_as_function()) {
	// It's an ActionScript function. Call it.
	as_value method;
	//log_msg("Calling ActionScript function for interval timer\n");
	(*as_func)(fn_call(&val, (as_object *)this_ptr, as_env, 0, 0));
	//(*as_func)(&val, (as_object *)this_ptr, &m_as_environment, 1, 1);
    } else {
	log_error("error in call_method(): method is not a function\n");
    }    
}	

character*
sprite_instance::get_character(int /* character_id */)
{
	//return m_def->get_character_def(character_id);
	// @@ TODO -- look through our dlist for a match
	log_msg("FIXME: %s doesn't even check for a char",
		__PRETTY_FUNCTION__);
	return NULL;
}

float
sprite_instance::get_timer() const
{
	return m_root->get_timer();
}

void
sprite_instance::clear_interval_timer(int x)
{
	m_root->clear_interval_timer(x);
}

int
sprite_instance::add_interval_timer(void *timer)
{
	return m_root->add_interval_timer(timer);
}

sprite_instance*
sprite_instance::get_root_movie()
{
	assert(m_root);
	return m_root->get_root_movie();
}

float
sprite_instance::get_pixel_scale() const
{
	return m_root->get_pixel_scale();
}

void
sprite_instance::get_mouse_state(int* x, int* y, int* buttons)
{
	m_root->get_mouse_state(x, y, buttons);
}

void
sprite_instance::get_drag_state(drag_state* st)
{
    *st = m_root->m_drag_state;
}

void
sprite_instance::stop_drag()
{
	assert(m_parent == NULL);	// we must be the root movie!!!
	m_root->stop_drag();
}

void
sprite_instance::set_drag_state(const drag_state& st)
{
	m_root->m_drag_state = st;
}

float
sprite_instance::get_background_alpha() const
{
    // @@ this doesn't seem right...
    return m_root->get_background_alpha();
}

void
sprite_instance::set_background_color(const rgba& color)
{
	m_root->set_background_color(color);
}


/* public */
void
sprite_instance::set_textfield_variable(const std::string& name,
		edit_text_character* ch)
{
	assert(ch);

	// lazy allocation
	if ( ! _text_variables.get() )
	{
		_text_variables.reset(new TextfieldMap);
	}

	// TODO: should variable name be considered case-insensitive ?
	_text_variables->operator[] (name) = ch;
}

/* private */
edit_text_character*
sprite_instance::get_textfield_variable(const std::string& name)
{
	// nothing allocated yet...
	if ( ! _text_variables.get() ) return NULL;

	// TODO: should variable name be considered case-insensitive ?
	TextfieldMap::iterator it = _text_variables->find(name);
	if ( it == _text_variables->end() )
	{
		return NULL;
	}
	else
	{
		return it->second.get();
	}
} 

void 
sprite_instance::get_invalidated_bounds(rect* bounds, bool force) {
  
  bounds->expand_to_rect(m_old_invalidated_bounds);
  
  if (!m_visible) return;
  // TODO: check if alpha=0 (return if so)
  
  m_display_list.get_invalidated_bounds(bounds, force||m_invalidated);
}

const char*
sprite_instance::call_method_args(const char* method_name,
		const char* method_arg_fmt, va_list args)
{
    // Keep m_as_environment alive during any method calls!
    boost::intrusive_ptr<as_object>	this_ptr(this);

    return call_method_parsed(&m_as_environment, this,
		method_name, method_arg_fmt, args);
}

const std::string&
sprite_instance::getTargetPath() const
{
	if ( _target.empty() ) _target = computeTargetPath();
	else
	{
		// What if set_name() is called *after*
		// we computed the _target string ?
		// let's check this... the design doesn't
		// take the problem into account..
		assert (_target == computeTargetPath() );
	}

	return _target;
}

/*private*/
std::string
sprite_instance::computeTargetPath() const
{

	// TODO: check what happens when this character
	//       is a movie_instance loaded into another
	//       running movie.

	typedef std::vector<std::string> Path;
	Path path;

	// Build parents stack
	const character* ch = this;
	for (;;)
	{
		const character* parent = ch->get_parent();

		// Don't push the _root name on the stack
		if ( ! parent )
		{
			assert(ch->get_name().empty());
			break;
		}

		path.push_back(ch->get_name());
		ch = parent;
	} 

	if ( path.empty() ) return "/";

	// Build the target string from the parents stack
	std::string target;
	for ( Path::reverse_iterator
			it=path.rbegin(), itEnd=path.rend();
			it != itEnd;
			++it )
	{
		target += "/" + *it; 
	}

	return target;
}

} // namespace gnash
