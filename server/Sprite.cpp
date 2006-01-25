//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

// Implementation for MovieClip object.

#include "action.h" 
#include "gnash.h"
#include "Sprite.h"
#include "MovieClipLoader.h" // @@ temp hack for loading tests

namespace gnash {

	// Execute the actions in the action list, in the given
	// environment.
	static void
	execute_actions(as_environment* env, const array<action_buffer*>&
		action_list)
	{
	    for (int i = 0; i < action_list.size(); i++)
		{
		    action_list[i]->execute(env);
		}
	}

	//
	// sprite built-in ActionScript methods
	//

	static void sprite_play(const fn_call& fn)
	{
	    sprite_instance* sprite = (sprite_instance*) fn.this_ptr;
	    if (sprite == NULL)
		{
		    sprite = (sprite_instance*) fn.env->get_target();
		}
	    assert(sprite);
	    sprite->set_play_state(movie_interface::PLAY);
	}

	static void sprite_stop(const fn_call& fn)
	{
	    sprite_instance* sprite = (sprite_instance*) fn.this_ptr;
	    if (sprite == NULL)
		{
		    sprite = (sprite_instance*) fn.env->get_target();
		}
	    assert(sprite);
	    sprite->set_play_state(movie_interface::STOP);
	}

	static void sprite_goto_and_play(const fn_call& fn)
	{
	    sprite_instance* sprite = (sprite_instance*) fn.this_ptr;
	    if (sprite == NULL)
		{
		    sprite = (sprite_instance*) fn.env->get_target();
		}
	    assert(sprite);

	    if (fn.nargs < 1)
		{
		    log_error("error: sprite_goto_and_play needs one arg\n");
		    return;
		}

	    int	target_frame = int(fn.arg(0).to_number() - 1);	// Convert to 0-based

	    sprite->goto_frame(target_frame);
	    sprite->set_play_state(movie_interface::PLAY);
	}

	static void sprite_goto_and_stop(const fn_call& fn)
	{
	    sprite_instance* sprite = (sprite_instance*) fn.this_ptr;
	    if (sprite == NULL)
		{
		    sprite = (sprite_instance*) fn.env->get_target();
		}
	    assert(sprite);

	    if (fn.nargs < 1)
		{
		    log_error("error: sprite_goto_and_stop needs one arg\n");
		    return;
		}

	    int	target_frame = int(fn.arg(0).to_number() - 1);	// Convert to 0-based

	    sprite->goto_frame(target_frame);
	    sprite->set_play_state(movie_interface::STOP);
	}

	static void sprite_next_frame(const fn_call& fn)
	{
	    sprite_instance* sprite = (sprite_instance*) fn.this_ptr;
	    if (sprite == NULL)
		{
		    sprite = (sprite_instance*) fn.env->get_target();
		}
	    assert(sprite);

	    int frame_count = sprite->get_frame_count();
	    int current_frame = sprite->get_current_frame();
	    if (current_frame < frame_count)
		{
		    sprite->goto_frame(current_frame + 1);
		}
	    sprite->set_play_state(movie_interface::STOP);
	}

	static void sprite_prev_frame(const fn_call& fn)
	{
	    sprite_instance* sprite = (sprite_instance*) fn.this_ptr;
	    if (sprite == NULL)
		{
		    sprite = (sprite_instance*) fn.env->get_target();
		}
	    assert(sprite);

	    int current_frame = sprite->get_current_frame();
	    if (current_frame > 0)
		{
		    sprite->goto_frame(current_frame - 1);
		}
	    sprite->set_play_state(movie_interface::STOP);
	}

	static void sprite_get_bytes_loaded(const fn_call& fn)
	{
	    sprite_instance* sprite = (sprite_instance*) fn.this_ptr;
	    if (sprite == NULL)
		{
		    sprite = (sprite_instance*) fn.env->get_target();
		}
	    assert(sprite);

	    fn.result->set_int(sprite->get_root()->get_file_bytes());
	}

	static void sprite_get_bytes_total(const fn_call& fn)
	{
	    sprite_instance* sprite = (sprite_instance*) fn.this_ptr;
	    if (sprite == NULL)
		{
		    sprite = (sprite_instance*) fn.env->get_target();
		}
	    assert(sprite);

	    fn.result->set_int(sprite->get_root()->get_file_bytes());
	}

	static void sprite_load_movie(const fn_call& fn)
	{
		log_error("Not implemented yet");
		//moviecliploader_loadclip(fn);
	}

	//
	// Initialize the Sprite/MovieClip builtin class 
	//
	as_object sprite_instance::as_builtins;
	void sprite_instance::init_builtins()
	{
		static int done=0;
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

		// @TODO
		//as_builtins.set_member("startDrag", &sprite_start_drag);
		//as_builtins.set_member("stopDrag", &sprite_stop_drag);
		//as_builtins.set_member("getURL", &sprite_get_url);
		//as_builtins.set_member("swapDepths", &sprite_swap_depths);
		// ... many more, see MovieClip class ...
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
		    val->set_int(m_def->get_frame_count());
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
		case M_URL:
		    //else if (name == "_url")
		{
		    // our URL.
		    val->set_string("gnash");
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
		    val->set_as_object_interface(static_cast<as_object_interface*>(m_parent));
		    return true;
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
		    val->set_as_object_interface(static_cast<as_object_interface*>(ch));
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
	    smart_ptr<as_object_interface>	this_ptr(this);

	    execute_actions(&m_as_environment, m_action_list);
	    m_action_list.resize(0);
	}

} // namespace gnash
