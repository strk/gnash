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
#include "render.h"  // for bounds_in_clipping_area()
#include "sprite_instance.h"
#include "movie_definition.h"
#include "MovieClipLoader.h" // @@ temp hack for loading tests
#include "as_value.h"
#include "as_function.h"
#include "edit_text_character_def.h" // @@ temp hack for createTextField exp.
#include "execute_tag.h"
#include "fn_call.h"
#include "Key.h"
#include "movie_root.h"
#include "movie_instance.h"
#include "swf_event.h"
#include "sprite_definition.h"
#include "ActionExec.h"
#include "builtin_function.h"
#include "smart_ptr.h"
#include "VM.h"
#include "Range2d.h" // for getBounds
#include "GnashException.h"
#include "URL.h"
#include "sound_handler.h"
#include "StreamProvider.h"
#include "URLAccessManager.h" // for loadVariables
#include "LoadVariablesThread.h" 
#include "ExecutableCode.h"

#include <vector>
#include <string>
#include <cmath>

#include <functional> // for mem_fun, bind1st
#include <algorithm> // for for_each
#include <boost/algorithm/string/case_conv.hpp>

// This needs to be included first for NetBSD systems or we get a weird
// problem with pthread_t being defined too many times if we use any
// STL containers.
#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

namespace gnash {

//#define GNASH_DEBUG 1

// Forward declarations
static as_object* getMovieClipInterface();
static void attachMovieClipInterface(as_object& o);
static void attachMovieClipProperties(as_object& o);

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

static as_value sprite_play(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);

	sprite->set_play_state(sprite_instance::PLAY);
	return as_value();
}

static as_value sprite_stop(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);

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
	return as_value();
}

//removeMovieClip() : Void
static as_value sprite_remove_movieclip(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);
	sprite->removeMovieClip();
	return as_value();
}

// attachMovie(idName:String, newName:String,
//             depth:Number [, initObject:Object]) : MovieClip
static as_value sprite_attach_movie(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);
	as_value rv;

	if (fn.nargs < 3 || fn.nargs > 4)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("attachMovie called with wrong number of arguments"
			" expected 3 to 4, got (%d) - returning undefined",
			fn.nargs);
		);
		return rv;
	}

	// Get exported resource 
	std::string id_name = fn.arg(0).to_std_string();

	boost::intrusive_ptr<resource> exported = sprite->get_movie_definition()->get_exported_resource(id_name.c_str());
	if ( exported == NULL )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("attachMovie: '%s': no such exported resource - "
			"returning undefined",
			id_name.c_str());
		);
		return rv; 
	}
	movie_definition* exported_movie = dynamic_cast<movie_definition*>(exported.get());
	if ( ! exported_movie )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("attachMovie: exported resource '%s' "
			"is not a movie definition (%s) -- "
			"returning undefined",
			id_name.c_str(),
			typeid(*(exported.get())).name());
		);
		return rv;
	}

	std::string newname = fn.arg(1).to_std_string();

	// should we support negative depths ? YES !
	int depth_val = uint16_t(fn.arg(2).to_number());

	boost::intrusive_ptr<character> newch = exported_movie->create_character_instance(sprite.get(), depth_val);
	assert( dynamic_cast<sprite_instance*>(newch.get()) );
	assert( newch.get() > (void*)0xFFFF );
	assert(newch->get_ref_count() > 0);

	newch->set_name(newname.c_str());

	// place_character() will set depth on newch
	if ( ! sprite->attachCharacter(*newch, depth_val) )
	{
		log_error("Could not attach character at depth %d", depth_val);
		return rv;
	}

	newch->setDynamic();

	/// Properties must be copied *after* the call to attachCharacter
	/// because attachCharacter() will reset matrix !!
	if (fn.nargs > 3 ) {
		boost::intrusive_ptr<as_object> initObject = fn.arg(3).to_object();
		if ( initObject ) {
			log_msg("Initializing properties from object");
			newch->copyProperties(*initObject);
		} else {
			// This is actually a valid thing to do,
			// the documented behaviour is to just NOT
			// initializing the properties in this
			// case.
			IF_VERBOSE_MALFORMED_SWF(
			log_aserror("Fourth argument of attachMovie "
				"doesn't cast to an object (%s)",
				fn.arg(3).to_string());
			);
		}
	}
	rv = as_value(newch.get());
	return rv;
}

// attachAudio(id:Object) : Void
static as_value sprite_attach_audio(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);
	UNUSED(sprite);

	static bool warned = false;
	if ( ! warned )
	{
		log_error("FIXME: MovieClip.attachAudio() unimplemented -- "
			"returning undefined");
		warned=true;
	}
	return as_value();
}

//createEmptyMovieClip(name:String, depth:Number) : MovieClip
static as_value sprite_create_empty_movieclip(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);

	if (fn.nargs != 2)
	{
		if (fn.nargs < 2)
		{
			IF_VERBOSE_ASCODING_ERRORS(
				log_aserror("createEmptyMovieClip needs "
					"2 args, but %d given,"
					" returning undefined.",
					fn.nargs);
			);
			return as_value();
		}
		else
		{
			IF_VERBOSE_ASCODING_ERRORS(
				log_aserror("createEmptyMovieClip takes "
					"2 args, but %d given, discarding"
					" the one in excess.",
					fn.nargs);
			)
		}
	}

	character* ch = sprite->add_empty_movieclip(fn.arg(0).to_string(), int(fn.arg(1).to_number()));
	return as_value(ch);
}

static as_value sprite_get_depth(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);

	int n = sprite->get_depth();

	return as_value(n);
}

//swapDepths(target:Object) : Void
static as_value sprite_swap_depths(const fn_call& fn)
{
	typedef boost::intrusive_ptr<sprite_instance> SpritePtr;
	typedef boost::intrusive_ptr<character> CharPtr;

	SpritePtr sprite = ensureType<sprite_instance>(fn.this_ptr);
	int this_depth = sprite->get_depth();

	// Lower bound of source depth below which swapDepth has no effect
	static const int lowerDepthBound = -16384;

	as_value rv;

	if (fn.nargs < 1)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("%s.swapDepths() needs one arg.", sprite->getTarget().c_str());
		);
		return rv;
	}

	if ( this_depth < lowerDepthBound )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		stringstream ss; fn.dump_args(ss);
		log_aserror("%s.swapDepths(%s) : won't swap a clip below depth %d (%d).",
			sprite->getTarget().c_str(), ss.str().c_str(), lowerDepthBound, this_depth);
		);
		return rv;
	}


	SpritePtr this_parent = dynamic_cast<sprite_instance*>(sprite->get_parent());
	if ( ! this_parent )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		stringstream ss; fn.dump_args(ss);
		log_aserror("%s.swapDepths(%s): this sprite has no parent, "
			"swapping depth of root ?",
			sprite->getTarget().c_str(),
			ss.str().c_str());
		);
		return rv;
	}
	

	CharPtr target = NULL;
	int target_depth = 0;

	// sprite.swapDepth(sprite)
	if ( SpritePtr target_sprite = fn.arg(0).to_sprite() )
	{
		if ( sprite == target_sprite )
		{
			IF_VERBOSE_ASCODING_ERRORS(
			log_aserror("%s.swapDepths(%s): invalid call, swapping to self?",
				sprite->getTarget().c_str(), target_sprite->getTarget().c_str());
			);
			return rv;
		}

		SpritePtr target_parent = dynamic_cast<sprite_instance*>(sprite->get_parent());
		if ( this_parent != target_parent )
		{
			IF_VERBOSE_ASCODING_ERRORS(
			log_aserror("%s.swapDepths(%s): invalid call, the two characters don't have the same parent",
				sprite->getTarget().c_str(), target_sprite->getTarget().c_str());
			);
			return rv;
		}

		target_depth = target_sprite->get_depth();
		target = boost::dynamic_pointer_cast<character>(target_sprite);
	}
	else
	{
		// sprite.swapDepth(depth)
		double td = fn.arg(0).to_number(&(fn.env()));
		if ( isnan(td) )
		{
			IF_VERBOSE_ASCODING_ERRORS(
			stringstream ss; fn.dump_args(ss);
			log_aserror("%s.swapDepths(%s): first argument invalid "
				"(neither a sprite nor a number).",
				sprite->getTarget().c_str(),
				ss.str().c_str());
			);
			return rv;
		}

		// TODO : check other kind of validities ?

		target_depth = int(td);
		target = this_parent->get_character_at_depth(target_depth);
	}

	sprite->set_depth(target_depth);
	if ( target )
	{
		target->set_depth(this_depth);
		this_parent->swap_characters(sprite.get(), target.get());
	}
	return rv;

}

// TODO: wrap the functionality in a sprite_instance method
//       and invoke it from here, this should only be a wrapper
//
//duplicateMovieClip(name:String, depth:Number, [initObject:Object]) : MovieClip
static as_value sprite_duplicate_movieclip(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);
	
	if (fn.nargs < 2)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("MovieClip.duplicateMovieClip() needs 2 or 3 args");
	    	);
		return as_value();
	}

	std::string newname = fn.arg(0).to_std_string(&(fn.env()));
	int depth = int(fn.arg(1).to_number());

	boost::intrusive_ptr<sprite_instance> ch;

	// Copy members from initObject
	if (fn.nargs == 3)
	{
		boost::intrusive_ptr<as_object> initObject = fn.arg(2).to_object();
		ch = sprite->duplicateMovieClip(newname, depth, initObject.get());
	}
	else
	{
		ch = sprite->duplicateMovieClip(newname, depth);
	}

	return as_value(ch.get());
}

static as_value sprite_goto_and_play(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);

	if (fn.nargs < 1)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("sprite_goto_and_play needs one arg");
		);
		return as_value();
	}

	// Convert to 0-based
	size_t target_frame = size_t(fn.arg(0).to_number() - 1);
	sprite->goto_frame(target_frame);
	sprite->set_play_state(sprite_instance::PLAY);
	return as_value();
}

static as_value sprite_goto_and_stop(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);

	if (fn.nargs < 1)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("sprite_goto_and_stop needs one arg");
		);
		return as_value();
	}

	// Convert to 0-based
	size_t target_frame = size_t(fn.arg(0).to_number() - 1);

	sprite->goto_frame(target_frame);
	sprite->set_play_state(sprite_instance::STOP);
	return as_value();
}

static as_value sprite_next_frame(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);

	size_t frame_count = sprite->get_frame_count();
	size_t current_frame = sprite->get_current_frame();
	if (current_frame < frame_count)
	{
	    sprite->goto_frame(current_frame + 1);
	}
	sprite->set_play_state(sprite_instance::STOP);
	return as_value();
}

static as_value sprite_prev_frame(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);

	size_t current_frame = sprite->get_current_frame();
	if (current_frame > 0)
	{
	    sprite->goto_frame(current_frame - 1);
	}
	sprite->set_play_state(sprite_instance::STOP);
	return as_value();
}

static as_value sprite_get_bytes_loaded(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);

	return as_value(sprite->get_bytes_loaded());
}

static as_value sprite_get_bytes_total(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);

	// @@ horrible uh ?
	return as_value(sprite->get_bytes_total());
}

// my_mc.loadMovie(url:String [,variables:String]) : Void
static as_value sprite_load_movie(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);
	UNUSED(sprite);

	if (fn.nargs < 1) // url
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_msg("Invalid call to MovieClip.loadMove(), "
			"expected 1 or 2 args, got %d - returning undefined",
			fn.nargs);
		);
		return as_value();
	}

	std::string urlstr = fn.arg(0).to_std_string();
	if (urlstr.empty())
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_msg("First argument passed to MovieClip.loadMove(%s) "
			"evaluates to an empty string - "
			"returning undefined",
			ss.str().c_str());
		);
		return as_value();
	}
	const URL& baseurl = get_base_url();
	URL url(urlstr, baseurl);

	if (fn.nargs > 1)
	{
		// TODO: implement support for second argument
		log_error("FIXME: second argument of MovieClip.loadMovie(%s, <variables>) "
			"will be discarded (unsupported)", urlstr.c_str());
		//return;
	}

	sprite->loadMovie(url);
	//log_warning("MovieClip.loadMovie(%s) - TESTING ", url.str().c_str());


	//log_error("FIXME: %s not implemented yet", __PRETTY_FUNCTION__);
	//moviecliploader_loadclip(fn);
	return as_value();
}

// my_mc.loadVariables(url:String [, variables:String]) : Void
static as_value sprite_load_variables(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);
	UNUSED(sprite);

	if (fn.nargs < 1) // url
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_msg("Invalid call to MovieClip.loadVariables(), "
			"expected 1 or 2 args, got %d - returning undefined",
			fn.nargs);
		);
		return as_value();
	}

	std::string urlstr = fn.arg(0).to_std_string();
	if (urlstr.empty())
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss; fn.dump_args(ss);
		log_msg("First argument passed to MovieClip.loadVariables(%s) "
			"evaluates to an empty string - "
			"returning undefined",
			ss.str().c_str());
		);
		return as_value();
	}
	const URL& baseurl = get_base_url();
	URL url(urlstr, baseurl);

	short method = 0;

	if (fn.nargs > 1)
	{
		std::string methodstring = fn.arg(1).to_std_string();
		// Should we be case-insensitive in comparing these ?
		if ( methodstring == "GET" ) method = 1;
		else if ( methodstring == "POST" ) method = 2;
	}

	sprite->loadVariables(url, method);
	log_warning("MovieClip.loadVariables(%s) - TESTING ", url.str().c_str());


	//log_error("FIXME: %s not implemented yet", __PRETTY_FUNCTION__);
	//moviecliploader_loadclip(fn);
	return as_value();
}

// my_mc.unloadMovie() : Void
static as_value sprite_unload_movie(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);
	UNUSED(sprite);

	// See http://sephiroth.it/reference.php?id=429

	static bool warned = false;
	if ( ! warned )
	{
		log_error("FIXME: MovieClip.unloadMovie() not implemented yet");
		warned=true;
	}
	return as_value();
}

static as_value sprite_hit_test(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);
	UNUSED(sprite);

	static bool warned_1_arg = false;
	static bool warned_2_arg = false;
	static bool warned_3_arg = false;

	switch (fn.nargs)
	{
		case 1: // target
		{
			as_value& tgt_val = fn.arg(0);
			character* target = fn.env().find_target(tgt_val);
			if ( ! target )
			{
				IF_VERBOSE_ASCODING_ERRORS(
				log_aserror("Can't find hitTest target %s",
					tgt_val.to_string());
				);
				return as_value();
			}
			if ( ! warned_1_arg ) {
				log_error("FIXME: hitTest(target) unimplemented");
				warned_1_arg=true;
			}
			break;
		}

		case 2: // x, y
		{
			double x = fn.arg(0).to_number();
			double y = fn.arg(1).to_number();
			if ( ! warned_2_arg ) {
				log_error("FIXME: hitTest(%g,%g) unimplemented",
				x,y);
				warned_2_arg=true;
			}
			break;
		}

		case 3: // x, y, shapeFlag
		{
			double x = fn.arg(0).to_number();
			double y = fn.arg(1).to_number();
			bool shapeFlag = fn.arg(2).to_bool();
			if ( ! warned_3_arg ) {
				log_error("FIXME: hitTest(%g,%g,%d) unimplemented",
					x,y,shapeFlag);
				warned_3_arg=true;
			}
			break;
		}

		default:
		{
			IF_VERBOSE_ASCODING_ERRORS(
				log_aserror("hitTest() called with %u args.",
					fn.nargs);
			);
			break;
		}
	}

	return as_value();

}

static as_value
sprite_create_text_field(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);

	if (fn.nargs != 6) // name, depth, x, y, width, height
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_msg("createTextField called with %d args, "
			"expected 6 - returning undefined", fn.nargs);
		);
		return as_value();
	}

	if ( ! fn.arg(0).is_string() ) 
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_msg("First argument of createTextField is not a string"
			" - returning undefined");
		);
		return as_value();
	}
	std::string txt_name = fn.arg(0).to_string();

	if ( ! fn.arg(1).is_number() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_msg("Second argument of createTextField is not a number"
			" - returning undefined");
		);
		return as_value();
	}
	int txt_depth = int(fn.arg(1).to_number());

	if ( ! fn.arg(2).is_number() ) 
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_msg("Third argument of createTextField is not a number"
			" - returning undefined");
		);
		return as_value();
	}
	float txt_x = fn.arg(2).to_number();

	if ( ! fn.arg(3).is_number() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_msg("Fourth argument of createTextField is not a number"
			" - returning undefined");
		);
		return as_value();
	}
	float txt_y = fn.arg(3).to_number();

	if ( ! fn.arg(4).is_number() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_msg("Fifth argument of createTextField is not a number"
			" - returning undefined");
		);
		return as_value();
	}
	float txt_width = fn.arg(4).to_number();

	if ( ! fn.arg(5).is_number() ) 
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_msg("Fifth argument of createTextField is not a number"
			" - returning undefined");
		);
		return as_value();
	}
	float txt_height = fn.arg(5).to_number();

	boost::intrusive_ptr<character> txt = sprite->add_textfield(txt_name,
			txt_depth, txt_x, txt_y, txt_width, txt_height);

	// createTextField returns void, it seems
	return as_value(); 
}

//getNextHighestDepth() : Number
static as_value
sprite_getNextHighestDepth(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);

	int nextdepth = sprite->getNextHighestDepth();
	return as_value(static_cast<double>(nextdepth));
}

// getURL(url:String, [window:String], [method:String]) : Void
static as_value
sprite_getURL(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);
	UNUSED(sprite);

	static bool warned = false;
	if ( ! warned )
	{
		log_error("FIXME: MovieClip.getURL() not implemented yet");
		warned=true;
	}
	return as_value();
}

// getBounds(targetCoordinateSpace:Object) : Object
static as_value
sprite_getBounds(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);


	geometry::Range2d<float> bounds  = sprite->getBounds();

	if ( fn.nargs > 0 )
	{
		boost::intrusive_ptr<sprite_instance> target = fn.arg(0).to_sprite();
		if ( ! target )
		{
			IF_VERBOSE_ASCODING_ERRORS(
			log_aserror("MovieClip.getBounds(%s) : invalid call, first arg must be a sprite",
				fn.arg(0).to_debug_string().c_str());
			);
			return as_value();
		}

		matrix tgtwmat = target->get_world_matrix();
		matrix srcwmat = sprite->get_world_matrix();
		// TODO: fixme, we should likely use the world matrixes for a straight and inverse transform
		matrix invtgtwmat; invtgtwmat.set_inverse(tgtwmat);
		matrix m = srcwmat;
		m.concatenate(invtgtwmat);

		//m.transform(bounds);
		//tgtwmat.transform_by_inverse(bounds);
		std::stringstream ss;

		ss << "Local bounds: " << bounds << endl;
		srcwmat.transform(bounds);
		ss << "src-w-transformed bounds: " << bounds << "(srcwmat is " << srcwmat << ")" << endl;
		tgtwmat.transform_by_inverse(bounds);
		ss << "tgt-w-invtransfor bounds: " << bounds << "(tgtwmat is " << tgtwmat << ")" << endl;
		log_msg("%s", ss.str().c_str());
		log_error("FIXME: MovieClip.getBounds(%s) broken", fn.arg(0).to_debug_string().c_str());
	}

	// Magic numbers here... dunno why
	double xMin = 6710886.35;
	double yMin = 6710886.35;
	double xMax = 6710886.35;
	double yMax = 6710886.35;

	if ( bounds.isFinite() )
	{
		xMin = bounds.getMinX()/20;
		yMin = bounds.getMinY()/20;
		xMax = bounds.getMaxX()/20;
		yMax = bounds.getMaxY()/20;
	}

	boost::intrusive_ptr<as_object> bounds_obj(new as_object());
	bounds_obj->init_member("xMin", as_value(xMin));
	bounds_obj->init_member("yMin", as_value(yMin));
	bounds_obj->init_member("xMax", as_value(xMax));
	bounds_obj->init_member("yMax", as_value(yMax));

	// xMin, xMax, yMin, and yMax
	static bool warned = false;
	if ( ! warned )
	{
		log_error("FIXME: MovieClip.getBounds() TESTING");
		warned=true;
	}

	return as_value(bounds_obj.get());
}

static as_value
sprite_globalToLocal(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);
	UNUSED(sprite);

	static bool warned = false;
	if ( ! warned )
	{
		log_error("FIXME: MovieClip.globalToLocal() not implemented yet");
		warned=true;
	}
	return as_value();
}

static as_value
sprite_endFill(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);
	sprite->endFill();
	return as_value();
}

static as_value
sprite_lineTo(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);

	if ( fn.nargs < 2 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
			log_aserror("MovieClip.lineTo() takes two args");
		);
		return as_value();
	}

	float x = PIXELS_TO_TWIPS(fn.arg(0).to_number());
	float y = PIXELS_TO_TWIPS(fn.arg(1).to_number());

	sprite->lineTo(x, y);

	return as_value();
}

static as_value
sprite_moveTo(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);

	if ( fn.nargs < 2 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
			log_aserror("MovieClip.moveTo() takes two args");
		);
		return as_value();
	}

	float x = PIXELS_TO_TWIPS(fn.arg(0).to_number());
	float y = PIXELS_TO_TWIPS(fn.arg(1).to_number());

	sprite->moveTo(x, y);

	return as_value();
}

static as_value
sprite_lineStyle(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);

	uint16_t thickness = 0;
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
	uint8_t a = 255;


	if ( fn.nargs > 0 )
	{
		thickness = uint16_t(PIXELS_TO_TWIPS(uint16_t(fclamp(fn.arg(0).to_number(), 0, 255))));

		if ( fn.nargs > 1 )
		{
			// 2^24 is the max here
			uint32_t rgbval = uint32_t(fclamp(fn.arg(1).to_number(), 0, 16777216));
			r = uint8_t( (rgbval&0xFF0000) >> 16);
			g = uint8_t( (rgbval&0x00FF00) >> 8);
			b = uint8_t( (rgbval&0x0000FF) );

			if ( fn.nargs > 2 )
			{
				float alphaval = fclamp(fn.arg(2).to_number(), 0, 255);
				a = uint8_t(alphaval);
			}
		}

	}


	rgba color(r, g, b, a);
	//log_msg("Color: %s", color.toString().c_str());

	sprite->lineStyle(thickness, color);

	return as_value();
}

static as_value
sprite_curveTo(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);

	if ( fn.nargs < 4 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
			log_aserror("MovieClip.curveTo() takes four args");
		);
		return as_value();
	}

	float cx = PIXELS_TO_TWIPS(fn.arg(0).to_number());
	float cy = PIXELS_TO_TWIPS(fn.arg(1).to_number());
	float ax = PIXELS_TO_TWIPS(fn.arg(2).to_number());
	float ay = PIXELS_TO_TWIPS(fn.arg(3).to_number());

	sprite->curveTo(cx, cy, ax, ay);

	return as_value();
}

static as_value
sprite_clear(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);

	sprite->clear();

	return as_value();
}

static as_value
sprite_beginFill(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);

	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
	uint8_t a = 255;

	if ( fn.nargs > 0 )
	{
		// 2^24 is the max here
		uint32_t rgbval = uint32_t(fclamp(fn.arg(0).to_number(), 0, 16777216));
		r = uint8_t( (rgbval&0xFF0000) >> 16);
		g = uint8_t( (rgbval&0x00FF00) >> 8);
		b = uint8_t( (rgbval&0x0000FF) );

	}

	rgba color(r, g, b, a);

	sprite->beginFill(color);

	return as_value();
}

static as_value
sprite_beginGradientFill(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);
	UNUSED(sprite);

	static bool warned = false;
	if ( ! warned )
	{
		log_error("FIXME: MovieClip.beginGradientFill() not implemented yet");
		warned=true;
	}
	return as_value();
}

// startDrag([lockCenter:Boolean], [left:Number], [top:Number],
// 	[right:Number], [bottom:Number]) : Void`
static as_value
sprite_startDrag(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);
	UNUSED(sprite);

	static bool warned = false;
	if ( ! warned )
	{
		log_error("FIXME: MovieClip.startDrag() not implemented yet");
		warned=true;
	}
	return as_value();
}

// stopDrag() : Void
static as_value
sprite_stopDrag(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> sprite = ensureType<sprite_instance>(fn.this_ptr);
	UNUSED(sprite);

	static bool warned = false;
	if ( ! warned )
	{
		log_error("FIXME: MovieClip.stopDrag() not implemented yet");
		warned=true;
	}
	return as_value();
}

static as_value
movieclip_ctor(const fn_call& /* fn */)
{
	boost::intrusive_ptr<as_object> clip = new as_object(getMovieClipInterface());
	//attachMovieClipProperties(*clip);
	return as_value(clip.get());
}


static as_value
sprite_currentframe_get(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> ptr = ensureType<sprite_instance>(fn.this_ptr);

	return as_value(ptr->get_current_frame() + 1);
}

static as_value
sprite_totalframes_get(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> ptr = ensureType<sprite_instance>(fn.this_ptr);

	return as_value(ptr->get_frame_count());
}

static as_value
sprite_framesloaded_get(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> ptr = ensureType<sprite_instance>(fn.this_ptr);

	return as_value(ptr->get_loaded_frames());
}

static as_value
sprite_target_get(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> ptr = ensureType<sprite_instance>(fn.this_ptr);

	return as_value(ptr->getTargetPath().c_str());
}

static as_value
sprite_name_getset(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> ptr = ensureType<sprite_instance>(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
		VM& vm = VM::get();
		const std::string& name = ptr->get_name();
		if ( vm.getSWFVersion() < 6 && name.empty() )
		{
			return as_value();
		} 
		else
		{
			return as_value(name.c_str());
		}
	}
	else // setter
	{
		ptr->set_name(fn.arg(0).to_string(&fn.env()));
		//IF_VERBOSE_ASCODING_ERRORS(
		//log_aserror("Attempt to set read-only property '_name'");
		//);
	}

	return as_value();
}

static as_value
sprite_droptarget_getset(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> ptr = ensureType<sprite_instance>(fn.this_ptr);
	UNUSED(ptr);

	static bool warned = false;
	if ( ! warned )
	{
		log_error("FIXME: MovieClip._droptarget unimplemented");
		warned=true;
	}

	VM& vm = VM::get();
	if ( vm.getSWFVersion() > 5 )
	{
		return as_value("");
	} 
	else
	{
		return as_value();
	}
}

static as_value
sprite_url_getset(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> ptr = ensureType<sprite_instance>(fn.this_ptr);

	return as_value(ptr->get_movie_definition()->get_url().c_str());
}

static as_value
sprite_highquality_getset(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> ptr = ensureType<sprite_instance>(fn.this_ptr);
	UNUSED(ptr);

	if ( fn.nargs == 0 ) // getter
	{
		// We don't support quality settings
		return as_value(true);
	}
	else // setter
	{
		static bool warned=false;
		if ( ! warned ) {
			log_warning("MovieClip._highquality setting is unsupported");
			warned = true;
		}
	}
	return as_value();
}

// TODO: move this to character class, _focusrect seems a generic property
static as_value
sprite_focusrect_getset(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> ptr = ensureType<sprite_instance>(fn.this_ptr);
	UNUSED(ptr);

	if ( fn.nargs == 0 ) // getter
	{
		// Is a yellow rectangle visible around a focused movie clip (?)
		// We don't support focuserct settings
		return as_value(false);
	}
	else // setter
	{
		static bool warned=false;
		if ( ! warned ) {
			log_warning("MovieClip._focusrect setting is unsupported");
			warned = true;
		}
	}
	return as_value();
}

static as_value
sprite_soundbuftime_getset(const fn_call& fn)
{
	boost::intrusive_ptr<sprite_instance> ptr = ensureType<sprite_instance>(fn.this_ptr);
	UNUSED(ptr);

	if ( fn.nargs == 0 ) // getter
	{
		// Number of seconds before sound starts to stream.
		return as_value(0.0);
	}
	else // setter
	{
		static bool warned=false;
		if ( ! warned ) {
			log_warning("MovieClip._soundbuftime setting is unsupported");
			warned = true;
		}
	}
	return as_value();
}

/// Properties (and/or methods) *inherited* by MovieClip instances
static void
attachMovieClipInterface(as_object& o)
{
	int target_version = o.getVM().getSWFVersion();

	boost::intrusive_ptr<builtin_function> gettersetter;

	// SWF5 or higher
	o.init_member("attachMovie", new builtin_function(sprite_attach_movie));
	o.init_member("play", new builtin_function(sprite_play));
	o.init_member("stop", new builtin_function(sprite_stop));
	o.init_member("gotoAndStop", new builtin_function(sprite_goto_and_stop));
	o.init_member("gotoAndPlay", new builtin_function(sprite_goto_and_play));
	o.init_member("nextFrame", new builtin_function(sprite_next_frame));
	o.init_member("prevFrame", new builtin_function(sprite_prev_frame));
	o.init_member("getBytesLoaded", new builtin_function(sprite_get_bytes_loaded));
	o.init_member("getBytesTotal", new builtin_function(sprite_get_bytes_total));
	o.init_member("loadMovie", new builtin_function(sprite_load_movie));
	o.init_member("loadVariables", new builtin_function(sprite_load_variables));
	o.init_member("unloadMovie", new builtin_function(sprite_unload_movie));
	o.init_member("hitTest", new builtin_function(sprite_hit_test));
	o.init_member("duplicateMovieClip", new builtin_function(sprite_duplicate_movieclip));
	o.init_member("swapDepths", new builtin_function(sprite_swap_depths));
	o.init_member("removeMovieClip", new builtin_function(sprite_remove_movieclip));
	o.init_member("startDrag", new builtin_function(sprite_startDrag));
	o.init_member("stopDrag", new builtin_function(sprite_stopDrag));
	o.init_member("getURL", new builtin_function(sprite_getURL));
	o.init_member("getBounds", new builtin_function(sprite_getBounds));
	o.init_member("globalToLocal", new builtin_function(sprite_globalToLocal));
	if ( target_version  < 6 ) return;

	// SWF6 or higher
	o.init_member("beginFill", new builtin_function(sprite_beginFill));
	o.init_member("beginGradientFill", new builtin_function(sprite_beginGradientFill));
	o.init_member("clear", new builtin_function(sprite_clear));
	o.init_member("curveTo", new builtin_function(sprite_curveTo));
	o.init_member("lineStyle", new builtin_function(sprite_lineStyle));
	o.init_member("lineTo", new builtin_function(sprite_lineTo));
	o.init_member("moveTo", new builtin_function(sprite_moveTo));
	o.init_member("endFill", new builtin_function(sprite_endFill));
	o.init_member("attachAudio", new builtin_function(sprite_attach_audio));
	o.init_member("createTextField", new builtin_function(sprite_create_text_field));
	o.init_member("getDepth", new builtin_function(sprite_get_depth));
	o.init_member("createEmptyMovieClip", new builtin_function(sprite_create_empty_movieclip));
	if ( target_version  < 7 ) return;

	// SWF7 or higher
	o.init_member("getNextHighestDepth", new builtin_function(sprite_getNextHighestDepth));
	if ( target_version  < 8 ) return;

	// TODO: many more methods, see MovieClip class ...

}

/// Properties (and/or methods) attached to every *instance* of a MovieClip 
static void
attachMovieClipProperties(as_object& o)
{
	//int target_version = o.getVM().getSWFVersion();

	boost::intrusive_ptr<builtin_function> gettersetter;

	// This is a normal property, can be overridden, deleted and enumerated
	o.init_member( "$version", VM::get().getPlayerVersion(), 0); 

	//
	// Properties (TODO: move to appropriate SWF version section)
	//

	gettersetter = new builtin_function(&character::x_getset, NULL);
	o.init_property("_x", *gettersetter, *gettersetter);


	gettersetter = new builtin_function(&character::y_getset, NULL);
	o.init_property("_y", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::xscale_getset, NULL);
	o.init_property("_xscale", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::yscale_getset, NULL);
	o.init_property("_yscale", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::xmouse_get, NULL);
	o.init_readonly_property("_xmouse", *gettersetter);

	gettersetter = new builtin_function(&character::ymouse_get, NULL);
	o.init_readonly_property("_ymouse", *gettersetter);

	gettersetter = new builtin_function(&character::alpha_getset, NULL);
	o.init_property("_alpha", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::visible_getset, NULL);
	o.init_property("_visible", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::width_getset, NULL);
	o.init_property("_width", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::height_getset, NULL);
	o.init_property("_height", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::rotation_getset, NULL);
	o.init_property("_rotation", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::parent_getset, NULL);
	o.init_property("_parent", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&sprite_currentframe_get, NULL);
	o.init_property("_currentframe", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&sprite_totalframes_get, NULL);
	o.init_property("_totalframes", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&sprite_framesloaded_get, NULL);
	o.init_property("_framesloaded", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&sprite_target_get, NULL);
	o.init_property("_target", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&sprite_name_getset, NULL);
	o.init_property("_name", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&sprite_droptarget_getset, NULL);
	o.init_property("_droptarget", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&sprite_url_getset, NULL);
	o.init_property("_url", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&sprite_highquality_getset, NULL);
	o.init_property("_highquality", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&sprite_focusrect_getset, NULL);
	o.init_property("_focusrect", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&sprite_soundbuftime_getset, NULL);
	o.init_property("_soundbuftime", *gettersetter, *gettersetter);

#if 0
	gettersetter = new builtin_function(&character::onrollover_getset, NULL);
	o.init_property("onRollOver", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::onrollout_getset, NULL);
	o.init_property("onRollOut", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::onload_getset, NULL);
	o.init_property("onLoad", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::onmouseup_getset, NULL);
	o.init_property("onMouseUp", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::onmousedown_getset, NULL);
	o.init_property("onMouseDown", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::onmousemove_getset, NULL);
	o.init_property("onMouseMove", *gettersetter, *gettersetter);
#endif

}

static as_object*
getMovieClipInterface()
{
	static boost::intrusive_ptr<as_object> proto;
	if ( proto == NULL )
	{
		proto = new as_object();
		attachMovieClipInterface(*proto);
		proto->init_member("constructor", new builtin_function(movieclip_ctor));
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
	global.init_member("MovieClip", cl.get());
}


//------------------------------------------------
// sprite_instance helper classes
//------------------------------------------------

/// A DisplayList visitor used to compute its overall bounds.
//
class BoundsFinder {
public:
	geometry::Range2d<float>& _bounds;
	BoundsFinder(geometry::Range2d<float>& b)
		:
		_bounds(b)
	{}
	void operator() (character* ch)
	{
		geometry::Range2d<float> chb = ch->getBounds();
		matrix m = ch->get_matrix();
		m.transform(chb);
		_bounds.expandTo(chb);
	}
};

/// A DisplayList visitor used to extract script characters
//
/// Script characters are characters created or transformed
/// by ActionScript. 
///
class ScriptObjectsFinder {
	std::vector<character*>& _dynamicChars;
	std::vector<character*>& _staticChars;
public:
	ScriptObjectsFinder(std::vector<character*>& dynamicChars,
			std::vector<character*>& staticChars)
		:
		_dynamicChars(dynamicChars),
		_staticChars(staticChars)
	{}

	void operator() (character* ch) 
	{
		// TODO: Are script-transformed object to be kept ?
		//       Need a testcase for this
		//if ( ! ch->get_accept_anim_moves() )
		//if ( ch->isDynamic() )
		int depth = ch->get_depth();
		if ( depth < -16384 || depth >= 0 )
		{
			_dynamicChars.push_back(ch);
		}
		else
		{
			_staticChars.push_back(ch);
		}
	}
};

/// A DisplayList visitor used to extract all characters
//
/// Script characters are characters created or transformed
/// by ActionScript
///
class CharactersExtractor {
	std::vector<character*>& _chars;
public:
	CharactersExtractor(std::vector<character*>& chars)
		:
		_chars(chars)
	{}

	void operator() (character* ch) 
	{
		_chars.push_back(ch);
	}
};

/// A DisplayList visitor used to unload all characters
struct UnloaderVisitor {
	bool operator() (character* ch)
	{
		ch->unload();
		return true;
	}
};


//------------------------------------------------
// sprite_instance
//------------------------------------------------

sprite_instance::sprite_instance(
		movie_definition* def, movie_instance* r,
		character* parent, int id)
	:
	character(parent, id),
	m_mouse_state(UP),
	m_root(r),
	m_display_list(),
	oldDisplayList(),
	_drawable(new DynamicShape),
	_drawable_inst(_drawable->create_character_instance(this, 0)),
	m_action_list(),
	//m_goto_frame_action_list(),
	m_play_state(PLAY),
	m_current_frame(0),
	m_has_looped(false),
	m_init_actions_executed(),
	m_as_environment(),
	m_has_keypress_event(false),
	m_has_mouse_event(false),
	_text_variables(),
	m_sound_stream_id(-1),
	_target(), // don't initialize now, as parent can be assigned later
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

	// TODO: have the 'MovieClip' constructor take care of this !
	attachMovieClipProperties(*this);

}

sprite_instance::~sprite_instance()
{

	if (m_has_keypress_event)
	{
		_vm.getRoot().remove_keypress_listener(this);
	}

	if (m_has_mouse_event)
	{
		_vm.getRoot().remove_mouse_listener(this);
	}

	m_display_list.clear();
}

character* sprite_instance::get_character_at_depth(int depth)
{
	return m_display_list.get_character_at_depth(depth);
}

// Set *val to the value of the named member and
// return true, if we have the named member.
// Otherwise leave *val alone and return false.
bool sprite_instance::get_member(const std::string& name, as_value* val)
{
	// FIXME: use addProperty interface for these !!
	if ( name == "_root" )
	{
		// TODO: handle lockroot
		val->set_as_object( VM::get().getRoot().get_root_movie() );
		return true;
	}
	if ( name == "_level0" )
	{
		// TODO: handle _level# (any level)
		val->set_as_object( VM::get().getRoot().get_root_movie() );
		return true;
	}
	if ( name == "this" )
	{
		val->set_as_object( this );
		return true;
	}

	// Try variables.
	if ( m_as_environment.get_member(name, val) )
	{
	    return true;
	}

	// Try object members, BEFORE display list items!
	// (see testcase VarAndCharClash.swf in testsuite/misc-ming.all)
	//
	// TODO: simplify the next line when get_member_default takes
	//       a std::string
	if ( get_member_default(name.c_str(), val) )
	{

// ... trying to be useful to Flash coders ...
// The check should actually be performed before any return
// prior to the one due to a match in the DisplayList.
// It's off by default anyway, so not a big deal.
// See bug #18457
//#define CHECK_FOR_NAME_CLASHES 1
#ifdef CHECK_FOR_NAME_CLASHES
		IF_VERBOSE_ASCODING_ERRORS(
		if (  m_display_list.get_character_by_name_i(name) )
		{
			log_aserror("A sprite member (%s) clashes with "
					"the name of an existing character "
					"in its display list! "
					"The member will hide the "
					"character.", name.c_str());
		}
		);
#endif

		return true;
	}


	// Try items on our display list.
	character* ch = m_display_list.get_character_by_name_i(name);
	if (ch)
	{
	    // Found object.
	    val->set_as_object(static_cast<as_object*>(ch));
	    return true;
	}

	// Try textfield variables
	edit_text_character* etc = get_textfield_variable(name);
	if ( etc )
	{
	    	val->set_string(etc->get_text_value());
		return true;
	}

	return false;

}

// Take care of this frame's actions.
void sprite_instance::do_actions()
{
	testInvariant();

	IF_VERBOSE_ACTION(
		log_action("Executing " SIZET_FMT " actions in frame " SIZET_FMT
			"/" SIZET_FMT " of sprite %s",
			m_action_list.size(),
			m_current_frame+1,
			m_def->get_frame_count(), getTargetPath().c_str());
	);

	execute_actions(m_action_list);
	assert(m_action_list.empty());

	testInvariant();
}

bool
sprite_instance::get_frame_number(const as_value& frame_spec, size_t& frameno) const
{
	//GNASH_REPORT_FUNCTION;

	as_environment* env = const_cast<as_environment*>(&m_as_environment);

	as_value str(frame_spec.to_std_string(env));

	double num =  str.to_number(env);

	if ( isnan(num) || isinf(num))
	{
		return m_def->get_labeled_frame(frame_spec.to_std_string(env), frameno);
	}

	// TODO: are we sure we shouldn't check for frames labeled with negative numbers ?
	if ( num < 1 ) return false;

	// all frame numbers >= 0 are valid, but a valid frame number may still
	// reference a non-exist frame(eg. frameno > total_frames).
	frameno = size_t(num) - 1;

	return true;
}

/// Execute the actions for the specified frame. 
//
/// The frame_spec could be an integer or a string.
///
void sprite_instance::call_frame_actions(const as_value& frame_spec)
{
	size_t frame_number;
	if ( ! get_frame_number(frame_spec, frame_number) )
	{
		// No dice.
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("call_frame('%s') -- invalid frame", frame_spec.to_debug_string().c_str());
		);
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
	sprite_definition* empty_sprite_def = new sprite_definition(get_movie_definition(), NULL);

	sprite_instance* sprite = new sprite_instance(empty_sprite_def, m_root, this, 0);
	sprite->set_name(name);
	sprite->setDynamic();

	m_display_list.place_character(
		sprite,
		depth,
		color_transform,
		matrix,
		0.0f,
		0); 

	return sprite;
}

boost::intrusive_ptr<character>
sprite_instance::add_textfield(const std::string& name, int depth, float x, float y, float width, float height)
{
	matrix txt_matrix;

	// Do I need the smart_ptr.here ?
	boost::intrusive_ptr<edit_text_character_def> txt = new edit_text_character_def(get_movie_definition());
	boost::intrusive_ptr<character> txt_char = txt->create_character_instance(this, 0);

	txt_char->set_name(name.c_str());
	txt_char->setDynamic();

	// Here we should set the proper matrix using x,y width and height

	// set _x
	txt_matrix.m_[0][2] = infinite_to_fzero(PIXELS_TO_TWIPS(x));
	// set _y
	txt_matrix.m_[1][2] = infinite_to_fzero(PIXELS_TO_TWIPS(y));
	// set _width
	txt_matrix.m_[0][0] = infinite_to_fzero(PIXELS_TO_TWIPS(width));
	// set _height
	txt_matrix.m_[1][1] = infinite_to_fzero(PIXELS_TO_TWIPS(height));

	// Here we add the character to the displayList.

	m_display_list.place_character(
		txt_char.get(),
		depth,
		cxform(),
		txt_matrix,
		0.0f,
		0);

	static bool warned = false;
	if ( ! warned )
	{
		log_error("FIXME: %s unfinished", __PRETTY_FUNCTION__);
		warned = true;
	}

	return txt_char;
}

boost::intrusive_ptr<sprite_instance> 
sprite_instance::duplicateMovieClip(const std::string& newname, int depth,
		as_object* initObject)
{
	character* parent_ch = get_parent();
	if ( ! parent_ch )
	{
		log_error("Can't clone root the movie");
		return NULL;
	}
	sprite_instance* parent = parent_ch->to_movie();
	if ( ! parent )
	{
		log_error("%s parent is not a sprite, can't clone", getTarget().c_str());
		return NULL;
	}

	boost::intrusive_ptr<sprite_instance> newsprite = new sprite_instance(m_def.get(),
			m_root, parent, get_id());
	newsprite->set_name(newname.c_str());

	newsprite->setDynamic();

	if ( initObject ) newsprite->copyProperties(*initObject);
	//else newsprite->copyProperties(*this);

	// Copy event handlers from sprite
	// We should not copy 'm_action_buffer' since the 'm_method' already contains it
	newsprite->set_event_handlers(get_event_handlers());

	// Copy drawable
	newsprite->_drawable = new DynamicShape(*_drawable);

	parent->m_display_list.place_character(
		newsprite.get(),
		depth,
		get_cxform(),
		get_matrix(),
		get_ratio(),
		get_clip_depth());
	
	return newsprite;
}

void sprite_instance::clone_display_object(const std::string& name,
	const std::string& newname, int depth)
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
    else
    {
	    log_error("clone_display_object(%s, %s, %d): could not find a character named %s to clone",
			    name.c_str(), newname.c_str(), depth, name.c_str());
    }
}

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

/* private */
void
sprite_instance::queueActions(ActionList& actions)
{
	movie_root& root = VM::get().getRoot();
	for(ActionList::iterator it=actions.begin(), itEnd=actions.end();
		       it != itEnd; ++it)
	{
		action_buffer* buf = *it;
		root.pushAction(*buf, boost::intrusive_ptr<sprite_instance>(this));
	}
}

bool
sprite_instance::on_event(const event_id& id)
{
	testInvariant();

	bool called = false;
			
	// First, check for built-in event handler.
	{
		std::auto_ptr<ExecutableCode> code ( get_event_handler(id) );
		if ( code.get() )
		{
			// Dispatch.
			code->execute();

			called = true;
			// Fall through and call the function also, if it's defined!
			// (@@ Seems to be the behavior for mouse events; not tested & verified for
			// every event type.)
		}
	}

	// If we called a built-in event handler
	// we stop here. If we don't do this,
	// MOUSE_UP, MOUSE_DOWN and MOUSE_MOVE will
	// be called twice !
	// Don't know if this is generally valid for every
	// kind of event, but we do have testcases for this one...
	if ( called ) return true;

	// Check for member function.
	{
		boost::intrusive_ptr<as_function> method = 
			getUserDefinedEventHandler(id.get_function_name());

		if ( method )
		{
			call_method0(as_value(method.get()), &m_as_environment, this);
			called = true;
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

void sprite_instance::set_member(const std::string& name,
		const as_value& val)
{
#ifdef DEBUG_DYNTEXT_VARIABLES
	log_msg("sprite[%p]::set_member(%s, %s)", (void*)this, name.c_str(), val.to_string());
#endif

	if ( val.is_function() )
	{
		checkForKeyPressOrMouseEvent(name);
	}

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
		log_error("NULL path_to_var passed to set_variable()");
		return;
	}
	if (new_value == NULL)
	{
		log_error("NULL passed to set_variable('%s',"
			" NULL)", path_to_var);
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

void sprite_instance::advance_sprite(float delta_time)
{
	//GNASH_REPORT_FUNCTION;

	// Process any pending loadVariables request
	processCompletedLoadVariableRequests();

	// mouse drag.
	character::do_mouse_drag();

	if (m_on_event_load_called)
	{
		on_event(event_id::ENTER_FRAME);
	}

#ifdef GNASH_DEBUG
	size_t frame_count = m_def->get_frame_count();

	log_msg("sprite '%s' ::advance_sprite is at frame %u/%u "
		"- onload called: %d - oldDIsplayList has %d elements",
		getTargetPath().c_str(), m_current_frame,
		frame_count, m_on_event_load_called, oldDisplayList.size());
#endif

	// Update current and next frames.
	if (m_play_state == PLAY)
	{
#ifdef GNASH_DEBUG
		log_msg("sprite_instance::advance_sprite we're in PLAY mode");
#endif

		int prev_frame = m_current_frame;

		if (m_on_event_load_called)
		{
#ifdef GNASH_DEBUG
			log_msg(" on_event_load called, incrementing");
#endif
			increment_frame_and_check_for_loop();
#ifdef GNASH_DEBUG
			log_msg(" after increment we are at frame %u/%u", m_current_frame, frame_count);
#endif
		}

		// Execute the current frame's tags.
		// First time execute_frame_tags(0) executed in dlist.cpp(child) or movie_def_impl(root)
		if (m_current_frame != (size_t)prev_frame)
		{
			execute_frame_tags(m_current_frame, TAG_DLIST|TAG_ACTION);
		}
	}
#ifdef GNASH_DEBUG
	else
	{
		log_msg("sprite_instance::advance_sprite we're in STOP mode");
		// shouldn't we execute frame tags anyway when in STOP mode ?
		//execute_frame_tags(m_current_frame);
	}
#endif

	// Advance DisplayList elements which has not been just-added.
	// 
	// These are elements in oldDisplayList cleared of all but elements
	// still in current DisplayList (the other must have been removed
	// by RemoveObject tags). I'm not sure we should do this actually,
	// as maybe we need to execute actions in removed objects *before*
	// we drop them...
	//
	// We do *not* dispatch UNLOAD event on the removed objects here
	// as we assume the event was dispatched by execution of the
	// RemoveObject tag itself. I might be wrong though, will come
	// back to this issue later.
	//
	// Note that we work on a *copy* of oldDisplayList as we're going
	// to need oldDisplayList again later, to extract the list of
	// newly added characters
	//
	DisplayList stillAlive = oldDisplayList;
	stillAlive.clear_except(m_display_list, false);
	//log_msg("Advancing %d pre-existing childs of %s", stillAlive.size(), getTargetPath().c_str());
	stillAlive.advance(delta_time);
	
	// Now execute actions on this timeline, after actions
	// in old childs timelines have been executed.
	//log_msg("Executing actions in %s timeline", getTargetPath().c_str());
	do_actions();

	// Call UNLOAD event of just removed chars !
	DisplayList justRemoved = oldDisplayList;
	justRemoved.clear_except(m_display_list, false); // true;
	// No, dont' call UNLOAD event, as it should be called by remove_display_object!

	// Finally, execute actions in newly added childs
	//
	// These are elements in the current DisplayList, cleared
	// by all elements in oldDisplayList.
	//
	// Of course we do NOT call UNLOAD events here, as
	// the chars we're clearing have *not* been removed:
	// we're simply doing internal work here...
	//
	DisplayList newlyAdded = m_display_list;
	//log_msg("%s has %d current childs and %d old childs", getTargetPath().c_str(), m_display_list.size(), oldDisplayList.size());
	newlyAdded.clear(oldDisplayList, false);
	//log_msg("Advancing %d newly-added (after clearing) childs of %s", newlyAdded.size(), getTargetPath().c_str());
	newlyAdded.advance(delta_time);

	// Remember current state of the DisplayList for next iteration
	oldDisplayList = m_display_list;
}

// child movieclip advance
void sprite_instance::advance(float delta_time)
{
//	GNASH_REPORT_FUNCTION;

	// child movieclip frame rate is the same the root movieclip frame rate
	// that's why it is not needed to analyze 'm_time_remainder' 
	if (m_on_event_load_called == false)
	{
#ifdef GNASH_DEBUG
		log_msg("Calling ONLOAD event");
#endif
		on_event(event_id::LOAD);	// clip onload

		if (m_has_keypress_event)
		{
			_vm.getRoot().add_keypress_listener(this);
		}
		// Mouse events listening is done in has_mouse_event directly.
		// This shows to work better for attachMovieTest.swf,
		// but might actually be a completely unrelated issue.
		// In particular, copying event handlers in attachMovie should
		// be more closely inspected (and need more ad-hoc testcases)
		//if (m_has_mouse_event)
		//{
			//_vm.getRoot().add_mouse_listener(this);
		//}
	}
	
	advance_sprite(delta_time);

	m_on_event_load_called = true;
}

void
sprite_instance::execute_action(action_buffer& ab)
{
	as_environment& env = m_as_environment; // just type less

	// Do not cleanup locals here, as there's nothing like
	// a movie-frame local scope...
	
	//int local_stack_top = env.get_local_frame_top();
	//env.add_frame_barrier();

	ActionExec exec(ab, env);
	exec();

	//env.set_local_frame_top(local_stack_top);
}

void
sprite_instance::resetDisplayList()
{
	// Add script objects in current DisplayList
	std::vector<character*> charsToAdd; 
	std::vector<character*> charsToKeep; 
	ScriptObjectsFinder scriptObjFinder(charsToAdd, charsToKeep);
	m_display_list.visitAll(scriptObjFinder);

	// Resort frame0 DisplayList as depth of
	// characters in it might have been
	// externally changed.
	_frame0_chars.sort();

	// Remove characters which have been removed
	_frame0_chars.clear_except(charsToKeep);

	// NOTE: script objects are *not* allowed to replace depths
	//       of static objects (change second argument to switch)
	_frame0_chars.addAll(charsToAdd, false);
	
	if ( ! (m_display_list == _frame0_chars) )
	{

		// Set this character as invalidated *before*
		// actually updating the displaylist !
		set_invalidated();

		m_display_list = _frame0_chars;			
	};
}

// 0-based frame number !
void
sprite_instance::execute_frame_tags(size_t frame, int typeflags)
{
	testInvariant();

	assert(frame < m_def->get_frame_count());

	m_is_reverse_execution = false;

	if ( frame == 0 && has_looped() )
	{
		resetDisplayList();

	}

	// Execute this frame's init actions, if necessary.
	if (m_init_actions_executed[frame] == false)
	{

		const PlayList* init_actions = m_def->get_init_actions(frame);

		if ( init_actions && ! init_actions->empty() )
		{

			IF_VERBOSE_ACTION(
				// Use 1-based frame numbers
				log_action("Executing " SIZET_FMT 
					" *init* tags in frame " SIZET_FMT
					"/" SIZET_FMT
				        " of sprite %s", init_actions->size(),
					frame+1, get_frame_count(),
					getTargetPath().c_str());
			);


			// Need to execute these actions.
			std::for_each(init_actions->begin(), init_actions->end(),
				std::bind2nd(std::mem_fun(&execute_tag::execute), this));

			// Mark this frame done, so we never execute these
			// init actions again.
			m_init_actions_executed[frame] = true;

			do_actions();
		}
	}

	const PlayList& playlist = m_def->get_playlist(frame);

	IF_VERBOSE_ACTION(
		// Use 1-based frame numbers
		log_action("Executing " SIZET_FMT " tags in frame "
			SIZET_FMT "/" SIZET_FMT " of sprite %s",
			playlist.size(), frame+1, get_frame_count(),
			getTargetPath().c_str());
	);

	for (PlayList::const_iterator it=playlist.begin(), itEnd=playlist.end();
			it != itEnd; ++it)
	{
		execute_tag* tag = *it;
		if ( typeflags & TAG_DLIST ) tag->execute_state(this);
		if ( typeflags & TAG_ACTION ) tag->execute_action(this);
	}

	if ( frame == 0 && ! has_looped() )
	{
		// Save DisplayList state
		_frame0_chars = m_display_list;
	}

	testInvariant();
}

void sprite_instance::execute_frame_tags_reverse(size_t frame)
{
	testInvariant();

	assert(frame < m_def->get_frame_count());

	m_is_reverse_execution = true;

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
	uint32_t depth_id = ((depth & 0x0FFFF) << 16) | (id & 0x0FFFF);

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
#ifdef DEBUG_GOTOFRAME
	log_msg("sprite %s ::goto_frame(" SIZET_FMT ") - current frame is "
		SIZET_FMT,
		getTargetPath().c_str(), target_frame_number, m_current_frame);
#endif

	assert(! isUnloaded() );

	// goto_frame stops by default.
	// ActionGotoFrame tells the movieClip to go to the target frame 
	// and stop at that frame. 
	set_play_state(STOP);

	if(target_frame_number == m_current_frame)
	{
		// don't push actions
		return;
	}
	if(target_frame_number > m_def->get_frame_count() - 1)
	{
		m_current_frame = m_def->get_frame_count() - 1;
		// don't push actions
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
	// target_frame_number is 0-based, get_loaded_frames() is 1-based
	// so in order to goto_frame(3) loaded_frames must be at least 4
	// if goto_frame(4) is called, and loaded_frames is 4 we're jumping
	// forward
	if ( target_frame_number >= loaded_frames )
	{
		IF_VERBOSE_ASCODING_ERRORS(
			log_aserror("GotoFrame(" SIZET_FMT ") targets a yet "
				"to be loaded frame (" SIZET_FMT ") loaded). "
				"We'll wait for it but a more correct form "
				"is explicitly using WaitForFrame instead.",
				target_frame_number+1,
				loaded_frames);

		);
		m_def->ensure_frame_loaded(target_frame_number+1);
	}


	//
	// Construct the DisplayList of the target frame
	//
	
	set_invalidated();

	if (target_frame_number < m_current_frame)
	// Go backward to a previous frame
	{
		resetDisplayList();
		for (size_t f = 0; f<target_frame_number; f++)
		{
			execute_frame_tags(f, TAG_DLIST);
		}
	}
	else
	// Go forward to a later frame
	{
		// We'd immediately return if target_frame_number == m_current_frame
		assert(target_frame_number > m_current_frame);

		// Construct the DisplayList of the target frame
		for (size_t f = m_current_frame+1; f<target_frame_number; ++f)
		{
			// Second argument requests that only "DisplayList" tags
			// are executed. This means NO actions will be
			// pushed on m_action_list.
			execute_frame_tags(f, TAG_DLIST);
		}

	}

	/// Backup current action list, as we're going to use it
	/// to fetch actions in the target frame
	ActionList actionListBackup = m_action_list;

	// m_action_list contains actions from frame 'target_frame_number'
	// to frame 'm_current_frame', too much than needed, clear it first.
	m_action_list.clear();

	// Get the actions of target frame.(We don't have a direct way to
	// do this, so use execute_frame_tags instead).
	execute_frame_tags(target_frame_number, TAG_ACTION|TAG_DLIST);

	//FIXME: set m_current_frame to the target frame;
	//  I think it's too early to do it here! Later actions in the 
	//  current frame should also be executed(Zou)
	m_current_frame = target_frame_number;      


	// After entering to advance_sprite() m_current_frame points to frame
	// that already is executed. 
	// Macromedia Flash do goto_frame then run actions from this frame.
	// We do too.

	// Stores the target frame actions to m_goto_frame_action_list, which
	// will be executed in after advancing child characters.
	// When actions execution order will be fixed all of this might
	// become obsoleted and the target frame actions could be pushed
	// directly on the preexisting m_action_list (and no need to back it up).
	//
	//m_goto_frame_action_list = m_action_list; 
	queueActions(m_action_list);

	// Restore ActionList  from backup
	m_action_list = actionListBackup;

}

bool sprite_instance::goto_labeled_frame(const std::string& label)
{
	size_t target_frame;
	if (m_def->get_labeled_frame(label, target_frame))
	{
		goto_frame(target_frame);
		return true;
	}

    IF_VERBOSE_MALFORMED_SWF(
    log_swferror("sprite_instance::goto_labeled_frame('%s') "
			"unknown label", label.c_str());
    );
    return false;
}

void sprite_instance::display()
{
	//GNASH_REPORT_FUNCTION;

	if (get_visible() == false)
	{
		// We're invisible, so don't display!
		
		// Note: dlist.cpp will avoid to even call display() so this will probably
		// never happen.
		return;
	}
	
	// check if the sprite (and it's childs) needs to be drawn
	InvalidatedRanges ranges;
	m_display_list.add_invalidated_bounds(ranges, true);
	
	// expand to bounds of _drawable
	rect drawable_bounds; 
	drawable_bounds.expand_to_transformed_rect(get_world_matrix(), _drawable->get_bound());
	ranges.add(drawable_bounds.getRange());
	
	if (gnash::render::bounds_in_clipping_area(ranges))
	{
		_drawable->finalize();
		// TODO: I'd like to draw the definition directly..
		//       but it seems that the backend insists in
		//       accessing the *parent* of the character
		//       passed as "instance" for the drawing.
		//       When displaying top-level movie this will
		//       be NULL and gnash will segfault
		//       Thus, this drawable_instance is basically just
		//       a container for a parent :(
		_drawable_inst->display();

		m_display_list.display();		
	}
	clear_invalidated();
	  
	do_display_callback();
}

void sprite_instance::swap_characters(character* ch1, character* ch2)
{
	ch1->set_invalidated();
	ch2->set_invalidated();

	m_display_list.swap_characters(ch1, ch2);	
}

bool
sprite_instance::attachCharacter(character& newch, int depth)
{

	// place_character() will set depth on newch
	m_display_list.place_character(
		&newch,
		depth,
		cxform(),
		matrix(),
		1.0,
		0);

	return true; // FIXME: check return from place_character above ?
}

character*
sprite_instance::add_display_object(
		uint16_t character_id,
		const char* name,
		const std::vector<swf_event*>& event_handlers,
		int depth, 
		bool replace_if_depth_is_occupied,
		const cxform& color_transform, const matrix& matrix,
		float ratio, int clip_depth)
{
//	    GNASH_REPORT_FUNCTION;
	assert(m_def != NULL);

	character_def*	cdef = m_def->get_character_def(character_id);
	if (cdef == NULL)
	{
		IF_VERBOSE_MALFORMED_SWF(
			log_swferror("sprite_instance::add_display_object(): "
				"unknown cid = %d", character_id);
		);
		return NULL;
	}

	character* existing_char = m_display_list.get_character_at_depth(depth);

	if (existing_char)
	{

		// If we already have this object on this
		// plane, then move it instead of replacing it.
		if ( existing_char->get_id() == character_id )
		{
			// TODO: update name ?
			move_display_object(depth, true, color_transform,
				true, matrix, ratio, clip_depth);
			return NULL;
		}

		// If we've been asked NOT to replace existing chars just
		// return NULL now
		if ( ! replace_if_depth_is_occupied )
		{
			return NULL;
		}
	}
	//printf("%s: character %s, id is %d, count is %d\n", __FUNCTION__, existing_char->get_name(), character_id,m_display_list.get_character_count()); 

	assert(cdef);
	boost::intrusive_ptr<character> ch = cdef->create_character_instance(this, character_id);
	assert(ch.get() != NULL);

	if ( name )
	{
		ch->set_name(name);
	}
	else if ( ch->wantsInstanceName() )
	{
		// MovieClip instances *need* a name, to properly setup
		// an as_value for them (values are kept by "target path"
		// reference.  We syntetize an instance name in this case.
		// TODO: check if we need to do this for other character types
		std::string instance_name = getNextUnnamedInstanceName();
		ch->set_name(instance_name.c_str());
	}

	// Attach event handlers (if any).
	for (size_t i = 0, n = event_handlers.size(); i < n; i++)
	{
		swf_event* ev = event_handlers[i];
		ch->add_event_handler(ev->event(), ev->action());
		//event_handlers[i]->attach_to(*ch);
	}

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
		int depth,
		bool use_cxform,
		const cxform& color_transform,
		bool use_matrix,
		const matrix& mat,
		float ratio,
		int clip_depth)
{
	assert(m_def != NULL);
	//log_msg("%s: character %s, id is %d, depth is %d", __FUNCTION__, name, character_id, depth); // FIXME: debugging crap

	character_def*	cdef = m_def->get_character_def(character_id);
	if (cdef == NULL)
	{
		log_error("sprite::replace_display_object(): "
			"unknown cid = %d", character_id);
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
		int depth,
		bool use_cxform,
		const cxform& color_transform,
		bool use_matrix,
		const matrix& mat,
		float ratio,
		int clip_depth)
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
	if ( ! ch ) 
	{
		ch = _drawable_inst->get_topmost_mouse_entity(p.m_x, p.m_y);
	}

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
sprite_instance::can_handle_mouse_event() const
{
	// Event handlers that qualify as mouse event handlers.
	static const event_id EH[] =
	{
		event_id(event_id::PRESS),
		event_id(event_id::RELEASE),
		event_id(event_id::RELEASE_OUTSIDE),
		event_id(event_id::ROLL_OVER),
		event_id(event_id::ROLL_OUT),
		event_id(event_id::DRAG_OVER),
		event_id(event_id::DRAG_OUT),
	};

	for (unsigned int i = 0; i < ARRAYSIZE(EH); i++)
	{
		const event_id &event = EH[i];

		// Check event handlers
		if ( get_event_handler(event.id()).get() )
		{
			return true;
		}

		// Check user-defined event handlers
		if ( getUserDefinedEventHandler(event.get_function_name()) )
		{
			return true;
		}
	}

	return false;
}
		
void sprite_instance::restart()
{
	//GNASH_REPORT_FUNCTION;

    m_current_frame = 0;

    m_has_looped = false;
    m_play_state = PLAY;

    // We're about to reset the displayList,
    // so take note of the current bounds
    // for the renderer to know what to 
    // redraw.
    set_invalidated();

    // Clear current display list and 
    // its backup
    m_display_list.clear();
    oldDisplayList.clear();

    // TODO: wipe out all members !!
    clearProperties();

    // Construct the sprite again
    construct();

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

//float
//sprite_instance::get_timer() const
//{
//	return m_root->get_timer();
//}


sprite_instance*
sprite_instance::get_root_movie()
{
	assert(m_root);
	return m_root; // could as well be myself !
}

float
sprite_instance::get_pixel_scale() const
{
	return _vm.getRoot().get_pixel_scale();
}

void
sprite_instance::get_mouse_state(int& x, int& y, int& buttons)
{
	_vm.getRoot().get_mouse_state(x, y, buttons);
}

void
sprite_instance::stop_drag()
{
	assert(m_parent == NULL);	// we must be the root movie!!!
	_vm.getRoot().stop_drag();
}

float
sprite_instance::get_background_alpha() const
{
    // @@ this doesn't seem right...
    return _vm.getRoot().get_background_alpha();
}

void
sprite_instance::set_background_color(const rgba& color)
{
	_vm.getRoot().set_background_color(color);
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
sprite_instance::add_invalidated_bounds(InvalidatedRanges& ranges, 
	bool force)
{

	// nothing to do if this sprite is not visible
	if (!m_visible || get_cxform().is_invisible() )
	{
    ranges.add(m_old_invalidated_ranges); // (in case we just hided)
		return;
	}

	if ( ! m_invalidated && ! m_child_invalidated && ! force )
	{
		return;
	}
  
 
  // m_child_invalidated does not require our own bounds
  if ( m_invalidated || force )      
  {
  	// Add old invalidated bounds
		ranges.add(m_old_invalidated_ranges); 
  }
  
  
	m_display_list.add_invalidated_bounds(ranges, force||m_invalidated);

	_drawable_inst->add_invalidated_bounds(ranges, force||m_invalidated);

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

const std::string&
sprite_instance::getTarget() const
{
	if ( ! _target_dot.empty() ) return _target_dot;

	std::string levelString = "_level0"; // TODO: support real levels!

	const std::string& targetPath = getTargetPath();
	if ( targetPath == "/" ) _target_dot = levelString;
	else
	{
		_target_dot = levelString + targetPath;
		for (std::string::size_type i=0; i<_target_dot.length(); ++i)
		{
			if ( _target_dot[i] == '/' ) _target_dot[i] = '.';
		}
	}

	return _target_dot;
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
			// it is completely legal to set root's _name
			//assert(ch->get_name().empty());
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

const char*
sprite_instance::get_text_value() const
{
	return getTarget().c_str();
}

// WARNING: THIS SNIPPET NEEDS THE CHARACTER TO BE "INSTANTIATED", which is
//          it's target path needs to exist, or any as_value for it will be
//          a dangling reference to an unexistent sprite !
void
sprite_instance::construct()
{
#ifdef GNASH_DEBUG
	log_msg("Constructing sprite '%s'", getTargetPath().c_str());
#endif

	// We *might* avoid this, but better safe then sorry
	m_def->ensure_frame_loaded(0);

	// Backup the DisplayList *before* manipulating it !
	assert( oldDisplayList.empty() );

	on_event(event_id::CONSTRUCT);
	execute_frame_tags(0, TAG_DLIST|TAG_ACTION);	

	if ( _name.empty() )
	{
		// instance name will be needed for properly setting up
		// a reference to 'this' object for ActionScript actions.
		// If the instance doesn't have a name, it will NOT be
		// an ActionScript referenciable object so we don't have
		// anything more to do.
		return;
	}

	sprite_definition* def = dynamic_cast<sprite_definition*>(m_def.get());

	// We won't "construct" top-level movies
	if ( ! def ) return;

	as_function* ctor = def->getRegisteredClass();
	//log_msg("Attached sprite's registered class is %p", (void*)ctor); 

	// TODO: builtin constructors are different from user-defined ones
	// we should likely change that. See also vm/ASHandlers.cpp (construct_object)
	if ( ctor && ! ctor->isBuiltin() )
	{
		// Set the new prototype *after* the constructor was called
		as_object* proto = ctor->getPrototype();
		set_prototype(proto);

		//log_msg("Calling the user-defined constructor against this sprite_instance");
		fn_call call(this, &(get_environment()), 0, 0);

		// we don't use the constructor return (should we?)
		(*ctor)(call);
	}
}

void
sprite_instance::unload()
{
#ifdef GNASH_DEBUG
	log_msg("Unloading sprite '%s'", getTargetPath().c_str());
#endif

	UnloaderVisitor visitor;
	m_display_list.visitForward(visitor);
	character::unload();

}

void
sprite_instance::set_name(const char* name)
{
	_name = name;

	// Reset these so they get computed next
	// time someone request them.
	_target.clear();
	_target_dot.clear();
}

bool
sprite_instance::loadMovie(const URL& url)
{
	boost::intrusive_ptr<movie_definition> md ( create_library_movie(url) );
	if (md == NULL)
	{
		log_error("can't create movie_definition for %s",
			url.str().c_str());
		return false;
	}

	boost::intrusive_ptr<movie_instance> extern_movie;
	extern_movie = md->create_movie_instance();
	if (extern_movie == NULL)
	{
		log_error("can't create extern movie_instance "
			"for %s", url.str().c_str());
		return false;
	}

	// Parse query string
	VariableMap vars;
	url.parse_querystring(url.querystring(), vars);
	extern_movie->setVariables(vars);

	save_extern_movie(extern_movie.get());

	const char* name = get_name().c_str();
	int depth = get_depth();
	bool use_cxform = false;
	cxform color_transform = get_cxform();
	bool use_matrix = false;
	matrix mat = get_matrix();
	float ratio = get_ratio();
	int clip_depth = get_clip_depth();
	//character* new_movie = extern_movie->get_root_movie();

	// Get a pointer to our own parent 
	character* parent = get_parent();
	if ( parent )
	{
		extern_movie->set_parent(parent);

		sprite_instance* parent_sp = parent->to_movie();
		assert(parent_sp);
		parent_sp->replace_display_object(
				   extern_movie.get(),
				   name,
				   depth,
				   use_cxform,
				   color_transform,
				   use_matrix,
				   mat,
				   ratio,
				   clip_depth);
	}
	else
	{
		movie_root& root = _vm.getRoot();
		// Make sure we won't kill ourself !
		assert(get_ref_count() > 1);
		root.setRootMovie(extern_movie.get());
	}

	return true;
}

void
sprite_instance::has_mouse_event()
{
	m_has_mouse_event = true;
	_vm.getRoot().add_mouse_listener(this);
}

void 
sprite_instance::loadVariables(const URL& url, short sendVarsMethod)
{
	// Check host security
	if ( ! URLAccessManager::allow(url) )
	{
		return;
	}

	if ( sendVarsMethod )
	{
		log_error("FIXME: MovieClip.loadVariables() with GET/POST unimplemented yet - won't append vars for now");
	}

	_loadVariableRequests.push_back(new LoadVariablesThread(url));
	_loadVariableRequests.back().process();
	//log_msg(SIZET_FMT " loadVariables requests pending", _loadVariableRequests.size());

}

/*private*/
void
sprite_instance::processCompletedLoadVariableRequest(LoadVariablesThread& request)
{
	assert(request.completed());

	// TODO: consider adding a setVariables(std::map) for use by this
	//       and by Player class when dealing with -P command-line switch

	LoadVariablesThread::ValuesMap& vals = request.getValues();
	for (LoadVariablesThread::ValuesMap::const_iterator it=vals.begin(),
			itEnd=vals.end();
		it != itEnd; ++it)
	{
		const string& name = it->first;
		const string& val = it->second;
		log_msg("Setting variable '%s' to value '%s'", name.c_str(), val.c_str());
		set_variable(name.c_str(), val.c_str()); // should it be set_member ?
	}
}

/*private*/
void
sprite_instance::processCompletedLoadVariableRequests()
{
	// Nothing to do (just for clarity)
	if ( _loadVariableRequests.empty() ) return;

	for (LoadVariablesThreads::iterator it=_loadVariableRequests.begin();
			it != _loadVariableRequests.end(); ++it)
	{
		LoadVariablesThread& request = *it;
		if ( request.completed() )
		{
			processCompletedLoadVariableRequest(request);
			it = _loadVariableRequests.erase(it);
		}
	}
}

void
sprite_instance::setVariables(VariableMap& vars)
{
	for (VariableMap::const_iterator it=vars.begin(), itEnd=vars.end();
		it != itEnd; ++it)
	{
		const string& name = it->first;
		const string& val = it->second;
		set_variable(name.c_str(), val.c_str());
	}
}

void
sprite_instance::removeMovieClip()
{
	int depth = get_depth();
	if ( depth < 0 || depth > 1048575 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("removeMovieClip(%s): sprite depth (%d) out of the "
			"'dynamic' zone [0..1048575], won't remove",
			getTarget().c_str(), depth);
		);
		return;
	}

	sprite_instance* parent = dynamic_cast<sprite_instance*>(get_parent());
	if (parent)
	{
		// second argument is arbitrary, see comments above
		// the function declaration in sprite_instance.h
		parent->remove_display_object(depth, 0);
	}
	else
	{
		// I guess this can only happen if someone uses _root.swapDepth([0..1048575])
		log_error("Can't remove sprite %s as it has no parent!", getTarget().c_str());
	}

}

void
sprite_instance::checkForKeyPressOrMouseEvent(const std::string& name)
{
	// short-cut
	if ( name.size() < 9 ) return;

	// TODO: don't use strcmp/strcasecmp, we're a C++ application after all !

	typedef int (*cmp_t) (const char*, const char*);
	cmp_t cmp = strcmp;
	if ( _vm.getSWFVersion() < 7 ) cmp = strcasecmp;

	const char* ptr = name.c_str();

	if ( ! cmp(ptr, "onkeypress") )
	{
		has_keypress_event();
	}
	else if ( ! cmp(ptr, "onMouseDown")
		|| ! cmp(ptr, "onMouseUp") 
		|| ! cmp(ptr, "onMouseMove") )
	{
		has_mouse_event();
	}

}

geometry::Range2d<float>
sprite_instance::getBounds() const
{
	typedef geometry::Range2d<float> Range;

	Range bounds;
	BoundsFinder f(bounds);
	const_cast<DisplayList&>(m_display_list).visitAll(f);
	Range drawableBounds = _drawable->get_bound().getRange();
	bounds.expandTo(drawableBounds);
	return bounds;
}

} // namespace gnash
