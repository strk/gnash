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

// Stateful live Sprite instance

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// This needs to be included first for NetBSD systems or we get a weird
// problem with pthread_t being defined too many times if we use any
// STL containers.
#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

#include "log.h" 
//#include "action.h" 
#include "gnash.h"
//#include "Sprite.h"
#include "sprite_instance.h"
#include "movie_definition.h"
#include "MovieClipLoader.h" // @@ temp hack for loading tests
#include "as_function.h"
#include "text_character_def.h" // @@ temp hack for createTextField exp.
#include "execute_tag.h"
#include "fn_call.h"
//#include "tu_random.h"
#include "Key.h"
#include "movie_root.h"
#include "swf_event.h"

#include <vector>
#include <string>
#include <cmath>

#include <functional> // for mem_fun, bind1st
#include <algorithm> // for for_each

using namespace std;

namespace gnash {

//------------------------------------------------
// Utility funx
//------------------------------------------------

// Execute the actions in the action list, in the given
// environment.
static void
execute_actions(as_environment* env,
		const std::vector<action_buffer*>& action_list)
{
	// action_list.size() may be changed due to actions
	for (unsigned int i=0; i < action_list.size(); ++i)
	{
	    action_list[i]->execute(env);
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
	sprite->set_play_state(movie_interface::PLAY);
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
	sprite->set_play_state(movie_interface::STOP);
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
	sprite->set_play_state(movie_interface::PLAY);
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
	sprite->set_play_state(movie_interface::STOP);
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
	sprite->set_play_state(movie_interface::STOP);
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
	sprite->set_play_state(movie_interface::STOP);
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

	assert(fn.nargs==6); // name, depth, x, y, width, height

	assert(fn.arg(0).get_type()==as_value::STRING);
	tu_string txt_name = fn.arg(0).to_string();

	assert(fn.arg(1).get_type()==as_value::NUMBER);
	//double txt_depth = fn.arg(1).to_number();

	assert(fn.arg(2).get_type()==as_value::NUMBER);
	//double txt_x = fn.arg(2).to_number();

	assert(fn.arg(3).get_type()==as_value::NUMBER);
	//double txt_y = fn.arg(3).to_number();

	assert(fn.arg(4).get_type()==as_value::NUMBER);
	//double txt_width = fn.arg(4).to_number();

	assert(fn.arg(5).get_type()==as_value::NUMBER);
	//double txt_height = fn.arg(5).to_number();


	// sprite is ourself, we should add a new
	// member, named txt_name and being a
	// TextField (text_character_def?)
	//

	// Get target's movie definition
	movie_definition *mds = sprite->get_movie_definition();

	log_msg("Target's movie definition at %p\n", (void*)mds);

	// Do I need the smart_ptr here ?
	smart_ptr<text_character_def> txt = new text_character_def(mds);

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
	
	//fn.result->set_as_object(txt.get_ptr());



	//assert(0); 
}


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
	m_def(def),
	m_root(r),
	m_play_state(PLAY),
	m_current_frame(0),
	m_time_remainder(0),
	m_update_frame(true),
	m_has_looped(false),
	m_accept_anim_moves(true),
	m_frame_time(0.0f),
	m_has_keypress_event(false),
	m_on_event_load_called(false)
{
	assert(m_def != NULL);
	assert(m_root != NULL);
			
	//m_root->add_ref();	// @@ circular!
	m_as_environment.set_target(this);

	init_builtins();

	// Initialize the flags for init action executed.
	m_init_actions_executed.resize(m_def->get_frame_count());
	for (std::vector<bool>::iterator p = m_init_actions_executed.begin(); p != m_init_actions_executed.end(); ++p)
	    {
		*p = false;
	    }

	assert(m_root);
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

//
// Initialize the Sprite/MovieClip builtin class 
//
as_object sprite_instance::as_builtins;
void sprite_instance::init_builtins()
{
	static bool done=false;
	if ( done ) return;

	as_builtins.set_member("play", &sprite_play);
	as_builtins.set_member("stop", &sprite_stop);
	as_builtins.set_member("gotoAndStop", &sprite_goto_and_stop);
	as_builtins.set_member("gotoAndPlay", &sprite_goto_and_play);
	as_builtins.set_member("nextFrame", &sprite_next_frame);
	as_builtins.set_member("prevFrame", &sprite_prev_frame);
	as_builtins.set_member("getBytesLoaded", &sprite_get_bytes_loaded);
	as_builtins.set_member("getBytesTotal", &sprite_get_bytes_total);
	as_builtins.set_member("loadMovie", &sprite_load_movie);
	as_builtins.set_member("createTextField", &sprite_create_text_field);

	// @TODO
	//as_builtins.set_member("startDrag", &sprite_start_drag);
	//as_builtins.set_member("stopDrag", &sprite_stop_drag);
	//as_builtins.set_member("getURL", &sprite_get_url);
	//as_builtins.set_member("swapDepths", &sprite_swap_depths);
	// ... many more, see MovieClip class ...

	done=true;
}

// Set *val to the value of the named member and
// return true, if we have the named member.
// Otherwise leave *val alone and return false.
bool sprite_instance::get_member(const tu_stringi& name, as_value* val)
{
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
	case M_WIDTH:
	    //else if (name == "_width")
	{
	    matrix	m = get_world_matrix();
	    rect	transformed_rect;

	    // @@ not sure about this...
	    rect	source_rect;
	    source_rect.m_x_min = 0;
	    source_rect.m_y_min = 0;
	    source_rect.m_x_max = (float) get_width();
	    source_rect.m_y_max = (float) get_height();

	    transformed_rect.enclose_transformed_rect(get_world_matrix(), source_rect);
	    val->set_double(TWIPS_TO_PIXELS(transformed_rect.width()));
	    return true;
	}
	case M_HEIGHT:
	    //else if (name == "_height")
	{
	    rect	transformed_rect;

	    // @@ not sure about this...
	    rect	source_rect;
	    source_rect.m_x_min = 0;
	    source_rect.m_y_min = 0;
	    source_rect.m_x_max = (float) get_width();
	    source_rect.m_y_max = (float) get_height();

	    transformed_rect.enclose_transformed_rect(get_world_matrix(), source_rect);
	    val->set_double(TWIPS_TO_PIXELS(transformed_rect.height()));
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
	    val->set_string("/_root");
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
	    val->set_tu_string(get_name());
	    return true;
	}
	case M_DROPTARGET:
	    //else if (name == "_droptarget")
	{
	    // Absolute path in slash syntax where we were last dropped (?)
	    // @@ TODO
	    val->set_string("/_root");
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
			assert(dynamic_cast<as_object*>(m_parent));
			val->set_as_object(static_cast<as_object*>(m_parent));
			return true;
		}
	}
	case M_ONLOAD:
	{
	    if (m_as_environment.get_member(name, val))
		{
		    return true;
		}
	    // Optimization: if no hit, don't bother looking in the display list, etc.
	    return false;
	}
	}	// end switch

	// Try variables.
	if (m_as_environment.get_member(name, val))
	{
	    return true;
	}

	// Not a built-in property.  Check items on our
	// display list.
	character* ch = m_display_list.get_character_by_name_i(name);
	if (ch)
	{
	    // Found object.
	    val->set_as_object(static_cast<as_object*>(ch));
	    return true;
	}

	// Try static builtin functions.
	if (as_builtins.get_member(name, val))
	{
	    return true;
	}

	return false;
}

// Take care of this frame's actions.
void sprite_instance::do_actions()
{
    // Keep m_as_environment alive during any method calls!
    smart_ptr<as_object>	this_ptr(this);

    execute_actions(&m_as_environment, m_action_list);
    m_action_list.resize(0);
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

	unsigned int top_action = m_action_list.size();

	// Execute the execute_tag actions

	const std::vector<execute_tag*>&playlist = m_def->get_playlist(frame_number);
	for (int i=0, n=playlist.size(); i<n; ++i)
	{
		execute_tag*	e = playlist[i];
		if (e->is_action_tag())
		{
			e->execute(this);
		}
	}

	// Execute any new actions triggered by the tag,
	// leaving existing actions to be executed.

	while (m_action_list.size() > top_action)
	{
		m_action_list[top_action]->execute(&m_as_environment);
		//m_action_list.remove(top_action);
		m_action_list.erase(m_action_list.begin()+top_action);
	}

	assert(m_action_list.size() == top_action);
}

void sprite_instance::clone_display_object(const tu_string& name,
	const tu_string& newname, uint16_t depth)
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
void sprite_instance::remove_display_object(const tu_string& name)
{
//	    GNASH_REPORT_FUNCTION;
	character* ch = m_display_list.get_character_by_name(name);
	if (ch)
	{
	    // @@ TODO: should only remove movies that were created via clone_display_object --
	    // apparently original movies, placed by anim events, are immune to this.
	    remove_display_object(ch->get_depth(), ch->get_id());
	}
}
#endif

bool sprite_instance::on_event(event_id id)
{
	    // Keep m_as_environment alive during any method calls!
	    smart_ptr<as_object>	this_ptr(this);

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

	    return called;
}

character*
sprite_instance::get_relative_target(const tu_string& name)
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
			character* ch = m_display_list.get_character_by_name_i( name);
			if ( ch ) // item found
			{
				const char* text = val.to_string();
				ch->set_text_value(text);
				return;
			}
	}

	// If that didn't work, set a variable within this environment.
	m_as_environment.set_member(name, val);
}

const char* sprite_instance::get_variable(const char* path_to_var) const
{
    assert(m_parent == NULL);	// should only be called on the root movie.

    tu_string	path(path_to_var);

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

	tu_string	path(path_to_var);
	as_value	val(new_value);

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

	    tu_string	path(path_to_var);
	    as_value	val(new_value);

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
				// affected depths
				const std::vector<execute_tag*>&	playlist = m_def->get_playlist(0);
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

	execute_actions(&m_as_environment, m_goto_frame_action_list);
	m_goto_frame_action_list.resize(0);

}

#if 0
// _root movieclip advance
void sprite_instance::advance_root(float delta_time)
{
	//GNASH_REPORT_FUNCTION;

	assert ( get_root()->get_root_movie() == this );

	movie_definition* md = get_movie_definition();

	// Load next frame if available (+2 as m_current_frame is 0-based)
	//
	// We do this inside advance_root to make sure
	// it's only for a root sprite (not a sprite defined
	// by DefineSprite!)
	md->ensure_frame_loaded(min(get_current_frame()+2, get_frame_count()));

	m_time_remainder += delta_time;

	// Check for the end of frame
	if (m_time_remainder >= m_frame_time)
	{

		// Vitaly: random should go continuously that:
		// 1. after restart of the player the situation has not repeated
		// 2. by different machines the random gave different numbers
		tu_random::next_random();

		m_time_remainder -= m_frame_time;
		advance_sprite(delta_time);

		if (m_on_event_load_called == false)
		{
			on_event(event_id::LOAD);	// root onload
			m_on_event_load_called = true;
		}

		m_time_remainder = fmod(m_time_remainder, m_frame_time);
	}
	else
	{
//	log_msg("no time remained");
	}
}
#endif

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

#if 0
void sprite_instance::advance(float delta_time)
{
//	GNASH_REPORT_FUNCTION;

// Keep this (particularly m_as_environment) alive during execution!
	smart_ptr<as_object>	this_ptr(this);

	assert(m_def != NULL && m_root != NULL);

	// Advance everything in the display list.
	m_display_list.advance(delta_time);

	// mouse drag.
	character::do_mouse_drag();

	m_time_remainder += delta_time;

	const float	frame_time = 1.0f / m_root->get_frame_rate();	// @@ cache this

	// Check for the end of frame
	if (m_time_remainder >= frame_time)
	{
		m_time_remainder -= frame_time;

		// Update current and next frames.
		if (m_play_state == PLAY)
		{
			int frame_count = m_def->get_frame_count();
			if ( m_current_frame == frame_count && frame_count > 1 )
			{
				m_display_list.reset();
			}

			int current_frame0 = m_current_frame;
			increment_frame_and_check_for_loop();

			// Execute the current frame's tags.
			if (m_current_frame != current_frame0)
			{
				execute_frame_tags(m_current_frame);
			}
		}

		// Dispatch onEnterFrame event.
		on_event(event_id::ENTER_FRAME);

		do_actions();

		//we don't have the concept of a DisplayList update anymore
		//m_display_list.update();
	}

	// Skip excess time.  TODO root caller should
	// loop to prevent this happening; probably
	// only root should keep m_time_remainder, and
	// advance(dt) should be a discrete tick()
	// with no dt.
	m_time_remainder = fmod(m_time_remainder, frame_time);
}
#endif

void
sprite_instance::execute_frame_tags(size_t frame, bool state_only)
{

	// Keep this (particularly m_as_environment) alive during execution!
	smart_ptr<as_object>	this_ptr(this);

	assert(frame < m_def->get_frame_count());

	// Execute this frame's init actions, if necessary.
	if (m_init_actions_executed[frame] == false)
	{
		const std::vector<execute_tag*>* init_actions = 
			m_def->get_init_actions(frame);

		if ( init_actions && ! init_actions->empty() )
		{

			// Need to execute these actions.
			for_each(init_actions->begin(), init_actions->end(),
				bind2nd(mem_fun(&execute_tag::execute), this));

			// Mark this frame done, so we never execute these
			// init actions again.
			m_init_actions_executed[frame] = true;
		}
	}

	const std::vector<execute_tag*>& playlist = m_def->get_playlist(frame);
	if (state_only)
	{
		for_each(playlist.begin(), playlist.end(),
			bind2nd(mem_fun(&execute_tag::execute_state), this));
	}
	else
	{
		for_each(playlist.begin(), playlist.end(),
			bind2nd(mem_fun(&execute_tag::execute), this));
	}
}

void sprite_instance::execute_frame_tags_reverse(size_t frame)
{

	// Keep this (particularly m_as_environment) alive during execution!
	smart_ptr<as_object>	this_ptr(this);

	assert(frame < m_def->get_frame_count());

	const std::vector<execute_tag*>& playlist = m_def->get_playlist(frame);

	for (unsigned int i=0, n=playlist.size(); i<n; ++i)
	{
	    execute_tag*	e = playlist[i];
	    e->execute_state_reverse(this, frame);
	}
}

void sprite_instance::execute_remove_tags(int frame)
{
	    assert(frame >= 0);
	    assert((size_t)frame < m_def->get_frame_count());

	    const std::vector<execute_tag*>&	playlist = m_def->get_playlist(frame);
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
	    const std::vector<execute_tag*>&	playlist = m_def->get_playlist(f);
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
    m_action_list.resize(0);
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

    m_action_list.resize(0);
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

   m_goto_frame_action_list = m_action_list; //.assign(m_action_list.begin(), m_action_list.end());
	 m_action_list.resize(0);

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
		log_error("movie_impl::goto_labeled_frame('%s') "
			"unknown label\n", label);
		return false;
	}
}

void sprite_instance::display()
{
//	GNASH_REPORT_FUNCTION;
    
	if (get_visible() == false)	{
		// We're invisible, so don't display!
		return;
	}

	m_display_list.display();

	do_display_callback();
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
//				IF_VERBOSE_DEBUG(log_msg("add changed to move on depth %d\n", depth));//xxxxxx
			// compare events 
			//hash<event_id, as_value>* existing_events = (hash<event_id, as_value>*) existing_char->get_event_handlers();
			const hash<event_id, as_value>* existing_events = existing_char->get_event_handlers();
			size_t n = event_handlers.size();
			if (existing_events->size() == n)
			{
				bool same_events = true;
				for (size_t i = 0; i < n; i++)
				{
					as_value result;
					if (existing_events->get(event_handlers[i]->m_event, &result))
					{
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
	    smart_ptr<character> ch = cdef->create_character_instance(this,
			character_id);
	    assert(ch.get_ptr() != NULL);
	    if (name != NULL && name[0] != 0)
		{
		    ch->set_name(name);
		}

	    // Attach event handlers (if any).
	    {for (int i = 0, n = event_handlers.size(); i < n; i++)
		{
		    event_handlers[i]->attach_to(ch.get_ptr());
		}}

	    m_display_list.place_character(
		ch.get_ptr(),
		depth,
		color_transform,
		matrix,
		ratio,
		clip_depth);

	    assert(ch == NULL || ch->get_ref_count() > 1);
	    return ch.get_ptr();
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

	smart_ptr<character> ch = cdef->create_character_instance(this,
			character_id);

	replace_display_object(
		ch.get_ptr(), name, depth,
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
#if 0 // don't mess with DisplayList indexes
    int	index = m_display_list.get_display_index(depth);
    if (index == -1) return -1;

    character*	ch = m_display_list.get_display_object(index).m_character.get_ptr();
#endif
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

	movie* _m;

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

		movie* te = ch->get_topmost_mouse_entity(_x, _y);
		if ( te )
		{
			_m = te;
			return false; // done
		}

		return true; // haven't found it yet
	}

	movie* getEntity() { return _m; }
		
};

movie*
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
	movie* ch = finder.getEntity();

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

float sprite_instance::get_height() 
{
	HeightFinder f;
	m_display_list.visitForward(f);
	return f.getHeight(); 

#if 0 // rewritten to use the visitor pattern

    int i, n = m_display_list.get_character_count();
    character* ch;
    for (i=0; i < n; i++)
	{
	    ch = m_display_list.get_character(i);
	    if (ch != NULL)
		{
		    float	ch_h = ch->get_height();
		    if (ch_h > h)
			{
			    h = ch_h;
			}
		}
	}
    return h;

#endif // 0
}

float sprite_instance::get_width() 
{
	WidthFinder f;
	m_display_list.visitForward(f);
	return f.getWidth(); 

#if 0 // rewritten to use visitor pattern
    int i, n = m_display_list.get_character_count();
    character* ch;
    for (i = 0; i < n; i++)
	{
	    ch = m_display_list.get_character(i);
	    if (ch != NULL)
		{
		    float ch_w = ch->get_width();
		    if (ch_w > w)
			{
			    w = ch_w;
			}
		}
	}

    return w;
#endif
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
} // namespace gnash
