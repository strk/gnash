// movie_root.cpp:  The root movie, for Gnash.
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

#include "movie_root.h"
#include "log.h"
#include "sprite_instance.h"
#include "movie_instance.h" // for implicit upcast to sprite_instance
#include "render.h"
#include "VM.h"
#include "tu_random.h"
#include "ExecutableCode.h"
#include "Stage.h"
#include "utility.h"
#include "URL.h"
#include "namedStrings.h"
#include "GnashException.h"
#ifdef NEW_KEY_LISTENER_LIST_DESIGN
  #include "action.h"
#endif

#include <iostream>
#include <string>
#include <typeinfo>
#include <cassert>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/bind.hpp>

//#define GNASH_DEBUG 1

using namespace std;

namespace gnash
{

inline bool
movie_root::testInvariant() const
{
	// TODO: fill this function !
	// The _movies map can not invariantably
	// be non-empty as the stage is autonomous
	// itself
	//assert( ! _movies.empty() );

	return true;
}


movie_root::movie_root()
	:
	m_viewport_x0(0),
	m_viewport_y0(0),
	m_viewport_width(1),
	m_viewport_height(1),
	m_pixel_scale(1.0f),
	m_background_color(255, 255, 255, 255),
	m_timer(0.0f),
	m_mouse_x(0),
	m_mouse_y(0),
	m_mouse_buttons(0),
	m_userdata(NULL),
	m_on_event_xmlsocket_ondata_called(false),
	m_on_event_xmlsocket_onxml_called(false),
	m_on_event_load_progress_called(false),
	_lastTimerId(0),
	m_active_input_text(NULL),
	m_time_remainder(0.0f),
	m_drag_state(),
	_allowRescale(true),
	_invalidated(true),
	_disableScripts(false),
	_processingActionLevel(movie_root::apSIZE)
{
}

void
movie_root::disableScripts()
{
	_disableScripts=true;

	// NOTE: we won't clear the action queue now
	//       to avoid invalidating iterators as we've
	//       been probably called during processing
	//       of the queue.
	//
	//clearActionQueue();
}

void
movie_root::clearActionQueue()
{
    for (int lvl=0; lvl<apSIZE; ++lvl)
    {
        ActionQueue& q = _actionQueue[lvl];
	    for (ActionQueue::iterator it=q.begin(),
	    		itE=q.end();
	    		it != itE; ++it)
	    {
	    	delete *it;
	    }
	    q.clear();
    }
}

movie_root::~movie_root()
{
	clearActionQueue();

	for (TimerMap::iterator it=_intervalTimers.begin(),
			itE=_intervalTimers.end();
			it != itE; ++it)
	{
		delete it->second;
	}
	assert(testInvariant());
}

void
movie_root::setRootMovie(movie_instance* movie)
{
	m_viewport_x0 = 0;
	m_viewport_y0 = 0;
	m_viewport_width = (int)movie->get_movie_definition()->get_width_pixels();
	m_viewport_height = (int)movie->get_movie_definition()->get_height_pixels();
	m_pixel_scale = 1;

	try
	{
		setLevel(0, movie);

		// actions in first frame of _level0 must execute now, before next advance,
		// or they'll be executed with _currentframe being set to 2
		processActionQueue();
	}
	catch (ActionLimitException& al)
	{
		log_error(_("ActionLimits hit during setRootMovie: %s. Disabling scripts"), al.what());
		disableScripts();
		clearActionQueue();
	}
}

/* private */
void
movie_root::setLevel(unsigned int num, boost::intrusive_ptr<movie_instance> movie)
{
	assert(movie != NULL);
	assert(static_cast<unsigned int>(movie->get_depth()) == num);

	//movie->setLevel(num)
	//movie->set_depth(num);
	//movie->set_name(ss.str().c_str());

	//if ( _movies.size() < num+1 ) _movies.resize(num+1);
	_movies[num] = movie; // [num] = movie;

	movie->set_invalidated();
	
	/// Notify placement 
	movie->stagePlacementCallback();

	assert(testInvariant());
}

bool
movie_root::loadLevel(unsigned int num, const URL& url)
{
	boost::intrusive_ptr<movie_definition> md ( create_library_movie(url) );
	if (md == NULL)
	{
		log_error(_("can't create movie_definition for %s"),
			url.str().c_str());
		return false;
	}

	boost::intrusive_ptr<movie_instance> extern_movie;
	extern_movie = md->create_movie_instance();
	if (extern_movie == NULL)
	{
		log_error(_("can't create extern movie_instance "
			"for %s"), url.str().c_str());
		return false;
	}

	// Parse query string
	sprite_instance::VariableMap vars;
	url.parse_querystring(url.querystring(), vars);
	extern_movie->setVariables(vars);

	character* ch = extern_movie.get();
	ch->set_depth(num);

	save_extern_movie(extern_movie.get());

	setLevel(num, extern_movie);

	return true;
}

boost::intrusive_ptr<movie_instance>
movie_root::getLevel(unsigned int num) const
{
	Levels::const_iterator i = _movies.find(num);
	if ( i == _movies.end() ) return 0;

	assert(boost::dynamic_pointer_cast<movie_instance>(i->second));
	return boost::static_pointer_cast<movie_instance>(i->second);
}

void
movie_root::reset()
{
	clear();
	_disableScripts = false;
}

void
movie_root::clear()
{
	// wipe out live chars
	_liveChars.clear();

	// wipe out queued actions
	clearActionQueue();

	// wipe out all levels
	_movies.clear();

#ifdef GNASH_USE_GC
	// Run the garbage collector again
	GC::get().collect();
#endif

	setInvalidated();
}

boost::intrusive_ptr<Stage>
movie_root::getStageObject()
{
	as_value v;
	if ( ! VM::isInitialized() ) return NULL;
	as_object* global = VM::get().getGlobal();
	if ( ! global ) return NULL;
	if (!global->get_member(NSV::PROP_iSTAGE, &v) ) return NULL;
	return boost::dynamic_pointer_cast<Stage>(v.to_object());
}
		
void
movie_root::set_display_viewport(int x0, int y0, int w, int h)
{
	assert(testInvariant());

    m_viewport_x0 = x0;
    m_viewport_y0 = y0;
    m_viewport_width = w;
    m_viewport_height = h;

    	if ( _allowRescale ) // Recompute pixel scale.
	{
		//log_msg("Rescaling allowed");

		// should we cache this ? it's immutable after all !
		const rect& frame_size = _movies[0]->get_frame_size();

		float	scale_x = m_viewport_width / TWIPS_TO_PIXELS(frame_size.width());
		float	scale_y = m_viewport_height / TWIPS_TO_PIXELS(frame_size.height());
		m_pixel_scale = fmax(scale_x, scale_y);

	}
	else // rescale not allowed, notify Stage (if any)
	{
		//log_msg("Rescaling disabled");
		boost::intrusive_ptr<Stage> stage = getStageObject();
		// how do I get the environment from ??
		if ( stage ) stage->onResize(NULL);
	}

	assert(testInvariant());
}

bool
movie_root::notify_mouse_moved(int x, int y)
{
	assert(testInvariant());

    m_mouse_x = x;
    m_mouse_y = y;
    notify_mouse_listeners(event_id(event_id::MOUSE_MOVE));
    return fire_mouse_event();

}

boost::intrusive_ptr<key_as_object>
movie_root::getKeyObject()
{
	VM& vm = VM::get();

	// TODO: test what happens with the global "Key" object
	//       is removed or overridden by the user

	if (!_keyobject)
	{
		// This isn't very performant... 
		// it will keep trying to find it even if impossible
		// to find.

		as_value kval;
		as_object* global = VM::get().getGlobal();

		std::string objName = "Key";
		if ( vm.getSWFVersion() < 7 )
		{
			boost::to_lower(objName, vm.getLocale());
		}
		if (global->get_member(vm.getStringTable().find(objName), &kval) )
		{
			//log_msg("Found member 'Key' in _global: %s", kval.to_string());
			boost::intrusive_ptr<as_object> obj = kval.to_object();
			//log_msg("_global.Key to_object() : %s @ %p", typeid(*obj).name(), obj);
			_keyobject = boost::dynamic_pointer_cast<key_as_object>( obj );
		}
	}

	return _keyobject;
}


key_as_object *
movie_root::notify_global_key(key::code k, bool down)
{
	VM& vm = VM::get();
	if ( vm.getSWFVersion() < 5 )
	{
		// Key was added in SWF5
		return NULL; 
	}

	boost::intrusive_ptr<key_as_object> keyobject = getKeyObject();
	if ( keyobject )
	{
		if (down) _keyobject->set_key_down(k);
		else _keyobject->set_key_up(k);
	}
	else
	{
		log_error("gnash::notify_key_event(): _global.Key doesn't exist, or isn't the expected built-in\n");
	}

	return _keyobject.get();
}

bool
movie_root::notify_key_event(key::code k, bool down)
{
//GNASH_REPORT_FUNCTION;

	//
	// First of all, notify the _global.Key object about key event
	//
	key_as_object * global_key = notify_global_key(k, down);

	// Notify character key listeners.
	notify_key_listeners(k, down);
#ifndef NEW_KEY_LISTENER_LIST_DESIGN
	// Notify both character and non-character Key listeners
	//	for user defined handerlers.
	// FIXME: this may violates the event order
	if(global_key)
	{
		if(down)
		{
			global_key->notify_listeners(event_id::KEY_DOWN);
			global_key->notify_listeners(event_id::KEY_PRESS);
		}
		else
			global_key->notify_listeners(event_id::KEY_UP);
	}
#endif	
	processActionQueue();

	return false; // should return true if needs update ...
}


bool
movie_root::notify_mouse_clicked(bool mouse_pressed, int button_mask)
{
	assert(testInvariant());

	//log_msg("Mouse click notification");
	if (mouse_pressed)
	{
		m_mouse_buttons |= button_mask;
		notify_mouse_listeners(event_id(event_id::MOUSE_DOWN));
	}
	else
	{
		m_mouse_buttons &= ~button_mask;
		notify_mouse_listeners(event_id(event_id::MOUSE_UP));
	}

	return fire_mouse_event();
}

void
movie_root::notify_mouse_state(int x, int y, int buttons)
{
	assert(testInvariant());

    m_mouse_x = x;
    m_mouse_y = y;
    m_mouse_buttons = buttons;
    fire_mouse_event();

	assert(testInvariant());
}

// Return wheter any action triggered by this event requires display redraw.
// See page about events_handling (in movie_interface.h)
//
/// TODO: make this code more readable !
bool
generate_mouse_button_events(mouse_button_state* ms)
{
	boost::intrusive_ptr<character> active_entity = ms->m_active_entity;
	boost::intrusive_ptr<character> topmost_entity = ms->m_topmost_entity;

	// Did this event trigger any action that needs redisplay ?
	bool need_redisplay = false;

	if (ms->m_mouse_button_state_last == mouse_button_state::DOWN)
	{
		// Mouse button was down.

		// TODO: Handle trackAsMenu dragOver

		// Handle onDragOut, onDragOver
		if (ms->m_mouse_inside_entity_last == false)
		{
			if (topmost_entity == active_entity)
			{
				// onDragOver
				if (active_entity != NULL)
				{
					active_entity->on_button_event(event_id::DRAG_OVER);
					// TODO: have on_button_event return
					//       wheter the action must trigger
					//       a redraw.
					need_redisplay=true;
				}
				ms->m_mouse_inside_entity_last = true;
			}
		}
		else
		{
			// mouse_inside_entity_last == true
			if (topmost_entity != active_entity)
			{
				// onDragOut
				if (active_entity != NULL)
				{
#ifndef GNASH_USE_GC
					assert(active_entity->get_ref_count() > 1); // we are NOT the only object holder !
#endif // GNASH_USE_GC
					active_entity->on_button_event(event_id::DRAG_OUT);
					// TODO: have on_button_event return
					//       wheter the action must trigger
					//       a redraw.
					need_redisplay=true;
				}
				ms->m_mouse_inside_entity_last = false;
			}
		}

		// Handle onRelease, onReleaseOutside
		if (ms->m_mouse_button_state_current == mouse_button_state::UP)
		{
			// Mouse button just went up.
			ms->m_mouse_button_state_last = mouse_button_state::UP;

			if (active_entity != NULL)
			{
				if (ms->m_mouse_inside_entity_last)
				{
					// onRelease
					active_entity->on_button_event(event_id::RELEASE);
					// TODO: have on_button_event return
					//       wheter the action must trigger
					//       a redraw.
					need_redisplay=true;
				}
				else
				{
					// TODO: Handle trackAsMenu 

					// onReleaseOutside
					active_entity->on_button_event(event_id::RELEASE_OUTSIDE);
					// TODO: have on_button_event return
					//       wheter the action must trigger
					//       a redraw.
					need_redisplay=true;
				}
			}
		}
	}

	if ( ms->m_mouse_button_state_last == mouse_button_state::UP )
	{
		// Mouse button was up.

		// New active entity is whatever is below the mouse right now.
		if (topmost_entity != active_entity)
		{
			// onRollOut
			if (active_entity != NULL)
			{
				active_entity->on_button_event(event_id::ROLL_OUT);
				// TODO: have on_button_event return
				//       wheter the action must trigger
				//       a redraw.
				need_redisplay=true;
			}

			active_entity = topmost_entity;

			// onRollOver
			if (active_entity != NULL)
			{
				active_entity->on_button_event(event_id::ROLL_OVER);
				// TODO: have on_button_event return
				//       wheter the action must trigger
				//       a redraw.
				need_redisplay=true;
			}

			ms->m_mouse_inside_entity_last = true;
		}

		// mouse button press
		if (ms->m_mouse_button_state_current == mouse_button_state::DOWN )
		{
			// onPress

			// set/kill focus for current root
			movie_root& mroot = VM::get().getRoot();
			character* current_active_entity = mroot.getFocus();

			// It's another entity ?
			if (current_active_entity != active_entity.get())
			{
				// First to clean focus
				if (current_active_entity != NULL)
				{
					current_active_entity->on_event(event_id::KILLFOCUS);
					// TODO: have on_button_event return
					//       wheter the action must trigger
					//       a redraw.
					need_redisplay=true;
					mroot.setFocus(NULL);
				}

				// Then to set focus
				if (active_entity != NULL)
				{
					if (active_entity->on_event(event_id::SETFOCUS))
					{
						mroot.setFocus(active_entity.get());
					}
				}
			}

			if (active_entity != NULL)
			{
				active_entity->on_button_event(event_id::PRESS);
				// TODO: have on_button_event return
				//       wheter the action must trigger
				//       a redraw.
				need_redisplay=true;
			}
			ms->m_mouse_inside_entity_last = true;
			ms->m_mouse_button_state_last = mouse_button_state::DOWN;
		}
	}

	// Write the (possibly modified) boost::intrusive_ptr copies back
	// into the state struct.
	ms->m_active_entity = active_entity;
	ms->m_topmost_entity = topmost_entity;

	//if ( ! need_redisplay ) log_msg("Hurray, an event didnt' trigger redisplay!");
	return need_redisplay;
}


bool
movie_root::fire_mouse_event()
{
//	GNASH_REPORT_FUNCTION;

	assert(testInvariant());

    // Generate a mouse event
    m_mouse_button_state.m_topmost_entity = getTopmostMouseEntity(PIXELS_TO_TWIPS(m_mouse_x), PIXELS_TO_TWIPS(m_mouse_y));
    m_mouse_button_state.m_mouse_button_state_current = (m_mouse_buttons & 1);

    bool need_redraw = generate_mouse_button_events(&m_mouse_button_state);

    // FIXME: need_redraw might also depend on actual
    //        actions execution (consider updateAfterEvent).
    processActionQueue();

    return need_redraw;

}

void
movie_root::get_mouse_state(int& x, int& y, int& buttons)
{
//	    GNASH_REPORT_FUNCTION;

//             log_msg ("X is %d, Y is %d, Button is %d", m_mouse_x,
//			 m_mouse_y, m_mouse_buttons);

	assert(testInvariant());

	x = m_mouse_x;
	y = m_mouse_y;
	buttons = m_mouse_buttons;

	assert(testInvariant());
}

void
movie_root::get_drag_state(drag_state& st)
{
	assert(testInvariant());

	st = m_drag_state;

	assert(testInvariant());
}

void
movie_root::set_drag_state(const drag_state& st)
{
	m_drag_state = st;
	character* ch = st.getCharacter();
	if ( ch && ! st.isLockCentered() )
	{
		// Get coordinates of the character's origin
		point origin(0, 0);
		matrix chmat = ch->get_world_matrix();
		point world_origin;
		chmat.transform(&world_origin, origin);

		// Get current mouse coordinates
		int x, y, buttons;
		get_mouse_state(x, y, buttons);
		point world_mouse(PIXELS_TO_TWIPS(x), PIXELS_TO_TWIPS(y));

		// Compute offset
		int xoffset = world_mouse.x - world_origin.x;
		int yoffset = world_mouse.y - world_origin.y;

		m_drag_state.setOffset(xoffset, yoffset);
	}
	assert(testInvariant());
}

void
movie_root::doMouseDrag()
{
	character* dragChar = m_drag_state.getCharacter();
	if ( ! dragChar || dragChar->isUnloaded() ) return; // nothing to do

	int	x, y, buttons;
	get_mouse_state(x, y, buttons);

	point world_mouse(PIXELS_TO_TWIPS(x), PIXELS_TO_TWIPS(y));
	if ( m_drag_state.hasBounds() )
	{
		// Clamp mouse coords within a defined rect.
		m_drag_state.getBounds().clamp(world_mouse);
	}

	if (! m_drag_state.isLockCentered())
	{
		world_mouse.x -= m_drag_state.xOffset();
		world_mouse.y -= m_drag_state.yOffset();
	}

	matrix	parent_world_mat;
	character* parent = dragChar->get_parent();
	if (parent != NULL)
	{
	    parent_world_mat = parent->get_world_matrix();
	}

	point	parent_mouse;
	parent_world_mat.transform_by_inverse(&parent_mouse, world_mouse);
			
	// Place our origin so that it coincides with the mouse coords
	// in our parent frame.
	matrix	local = dragChar->get_matrix();
	local.set_translation( parent_mouse.x, parent_mouse.y );
	dragChar->set_matrix(local);
}

unsigned int
movie_root::add_interval_timer(std::auto_ptr<Timer> timer, bool internal)
{
	assert(timer.get());
	assert(testInvariant());
			
	int id = ++_lastTimerId;
	if ( internal ) id = -id;

	if ( _intervalTimers.size() >= 255 )
	{
		// TODO: Why this limitation ? 
		log_error("FIXME: " SIZET_FMT " timers currently active, won't add another one", _intervalTimers.size());
	}

	assert(_intervalTimers.find(id) == _intervalTimers.end());
	_intervalTimers[id] = timer.release(); 
	return id;
}
	
bool
movie_root::clear_interval_timer(unsigned int x)
{
	TimerMap::iterator it = _intervalTimers.find(x);
	if ( it == _intervalTimers.end() ) return false;

	// We do not remove the element here because
	// we might have been called during execution
	// of another timer, thus during a scan of the _intervalTimers
	// container. If we use erase() here, the iterators in executeTimers
	// would be invalidated. Rather, executeTimers() would check container
	// elements for being still active and remove the cleared one in a safe way
	// at each iteration.
	it->second->clearInterval();

	return true;

}
	
void
movie_root::advance(float delta_time)
{
	// GNASH_REPORT_FUNCTION;

	// Do mouse drag, if needed
	doMouseDrag();

	try
	{

	// Execute expired timers
	// NOTE: can throw ActionLimitException
	executeTimers();

#ifndef NEW_KEY_LISTENER_LIST_DESIGN
	// Cleanup key listeners (remove unloaded characters)
	// FIXME: not all key listeners could be cleaned here!
	// (eg. characters unloaded by loop-back won't be cleared until next advancement)
	cleanup_key_listeners();
#endif
	// random should go continuously that:
	// 1. after restart of the player the situation has not repeated
	// 2. by different machines the random gave different numbers
	tu_random::next_random();
			
	// Advance all non-unloaded characters in the LiveChars list
	// in reverse order (last added, first advanced)
	// NOTE: can throw ActionLimitException
	advanceLiveChars(delta_time); 

#ifdef NEW_KEY_LISTENER_LIST_DESIGN
	cleanup_key_listeners();
#endif

	// Process queued actions
	// NOTE: can throw ActionLimitException
	processActionQueue();

	}
	catch (ActionLimitException& al)
	{
		log_error(_("ActionLimits hit during advance: %s. Disabling scripts"), al.what());
		disableScripts();
		clearActionQueue();
	}

	// Delete characters removed from the stage
	// from the display lists
	cleanupDisplayList();

#ifdef GNASH_USE_GC
	// Run the garbage collector (step back !!)
	GC::get().collect();
#endif

	assert(testInvariant());
}


void
movie_root::display()
{
//	GNASH_REPORT_FUNCTION;

	assert(testInvariant());

	// should we cache this ? it's immutable after all !
	const rect& frame_size = getLevel(0)->get_frame_size();

	clearInvalidated();

	render::begin_display(
		m_background_color,
		m_viewport_x0, m_viewport_y0,
		m_viewport_width, m_viewport_height,
		frame_size.get_x_min(), frame_size.get_x_max(),
		frame_size.get_y_min(), frame_size.get_y_max());


	for (Levels::iterator i=_movies.begin(), e=_movies.end(); i!=e; ++i)
	{
		boost::intrusive_ptr<sprite_instance> movie = i->second;

		movie->clear_invalidated();

		if (movie->get_visible() == false) continue;

		// null frame size ? don't display !
		const rect& sub_frame_size = movie->get_frame_size();

		if ( sub_frame_size.is_null() )
		{
			log_debug("_level%u has null frame size, skipping", i->first);
			continue;
		}

		movie->display();

	}

	render::end_display();
}


const char*
movie_root::call_method(const char* method_name,
		const char* method_arg_fmt, ...)
{
	assert(testInvariant());

	va_list	args;
	va_start(args, method_arg_fmt);
	const char* result = getLevel(0)->call_method_args(method_name,
		method_arg_fmt, args);
	va_end(args);

	return result;
}

const
char* movie_root::call_method_args(const char* method_name,
		const char* method_arg_fmt, va_list args)
{
	assert(testInvariant());
	return getLevel(0)->call_method_args(method_name, method_arg_fmt, args);
}

#ifdef NEW_KEY_LISTENER_LIST_DESIGN

void movie_root::cleanup_key_listeners()
{
#ifdef KEY_LISTENERS_DEBUG
    size_t prevsize = _keyListeners.size();
    log_msg("Cleaning up %u key listeners", _keyListeners.size());
#endif

    for (KeyListeners::iterator iter = _keyListeners.begin();
        iter != _keyListeners.end(); )
    {
        // The listener object has no registered key event handlers, remove it.
        if( !iter->hasUserRegistered() && !iter->hasOnClipRegistered() )
        {
            _keyListeners.erase(iter++);
        }
        else 
        {
            boost::intrusive_ptr<as_object> obj = iter->get();
			character* ch = dynamic_cast<character*>(obj.get());
            // The listener object is unloaded, remove it.
            // TODO: Don't do this again in the character destructors. should we?
            if ( ch && ch->isUnloaded() ) 
            {
                _keyListeners.erase(iter++);
            }
            else
                ++iter;
        }
    }

#ifdef KEY_LISTENERS_DEBUG
    size_t currsize = _keyListeners.size();
    log_msg("Cleaned up %u listeners (from %u to %u)", prevsize-currsize, prevsize, currsize);
#endif
}

void movie_root::notify_key_listeners(key::code k, bool down)
{
    //log_msg("Notifying " SIZET_FMT " keypress listeners", _keyListeners.size());

    for (KeyListeners::iterator iter = _keyListeners.begin();
        iter != _keyListeners.end(); ++iter)
    {
		boost::intrusive_ptr<as_object> obj = iter->get();
        character* ch = dynamic_cast<character*>(obj.get());
        // notify character listeners
        if ( ch && ! ch->isUnloaded() ) 
        {
            if(down)
            {
                // invoke onClipKeyDown handler
                ch->on_event(event_id(event_id::KEY_DOWN, key::INVALID));
                
                if(iter->hasUserRegistered())
                // invoke onKeyDown handler
                {
			VM& vm = VM::get();
			string_table& st =vm.getStringTable();
			ch->callMethod(st.find(PROPNAME("onKeyDown")), ch->get_environment());
                }
                // invoke onClipKeyPress handler
                ch->on_event(event_id(event_id::KEY_PRESS, key::codeMap[k][0]));
            }
            else
            {
                //invoke onClipKeyUp handler
                ch->on_event(event_id(event_id::KEY_UP, key::INVALID));   

                if(iter->hasUserRegistered())
                // invoke onKeyUp handler
                {
			VM& vm = VM::get();
			string_table& st =vm.getStringTable();
			ch->callMethod(st.find(PROPNAME("onKeyUp")), ch->get_environment());
                }
            }
        }
        // notify non-character listeners
        else 
        {
            if(down) 
            {
                iter->get()->on_event(event_id(event_id::KEY_DOWN, key::INVALID));
            }
            else 
            {
                iter->get()->on_event(event_id(event_id::KEY_UP, key::INVALID));
            }
        }
    }
    assert(testInvariant());
}

void movie_root::add_key_listener(const KeyListener & listener)
{
    KeyListeners::iterator target = _keyListeners.find(listener);
    if(target == _keyListeners.end())
    // The key listener is not in the container, then add it.
    {
        _keyListeners.insert(listener);
    }
    else
    // The key listener is already in the container, then register it(again).
    {
        if(listener.hasUserRegistered())
        {
            target->registerUserHandler();
        }
        if(listener.hasOnClipRegistered())
        {
            target->registerOnClipHandler();
        }
    }

    assert(testInvariant());
}

void movie_root::remove_key_listener(as_object* listener)
{
    _keyListeners.erase(KeyListener(listener));

    assert(testInvariant());
}

#else // ndef NEW_KEY_LISTENER_LIST_DESIGN

void movie_root::cleanup_key_listeners()
{
#ifdef KEY_LISTENERS_DEBUG
	size_t prevsize = _keyListeners.size();
	log_msg("Cleaning up %u key listeners", _keyListeners.size());
#endif

	for (ListenerSet::iterator iter = m_key_listeners.begin(); iter != m_key_listeners.end(); )
	{
		// TODO: handle non-character objects too !
		character* ch = dynamic_cast<character*>(iter->get());
		if ( ch && ch->isUnloaded() )
		{
			ListenerSet::iterator toremove = iter;
			++iter;
			//log_msg("cleanup_key_listeners: Removing unloaded key listener %p", iter->get());
			m_key_listeners.erase(toremove);
		}
		else
		{
			++iter;
		}
	}

#ifdef KEY_LISTENERS_DEBUG
	size_t currsize = _keyListeners.size();
	log_msg("Cleaned up %u listeners (from %u to %u)", prevsize-currsize, prevsize, currsize);
#endif
}

void movie_root::notify_key_listeners(key::code k, bool down)
{
	log_msg("Notifying " SIZET_FMT " keypress listeners", 
		m_key_listeners.size());

	for (ListenerSet::iterator iter = m_key_listeners.begin();
			 iter != m_key_listeners.end(); ++iter)
	{
		// sprite, button & input_edit_text characters
		// TODO: invoke functions on non-characters !
		character* ch = dynamic_cast<character*>(iter->get());
		if ( ch && ! ch->isUnloaded() )
		{
			if(down)
			{
				// KEY_UP and KEY_DOWN events are unrelated to any key!
				ch->on_event(event_id(event_id::KEY_DOWN, key::INVALID)); 
        ch->on_event(event_id(event_id::KEY_PRESS, key::codeMap[k][0]));
			}
			else
				ch->on_event(event_id(event_id::KEY_UP, key::INVALID));   
		}
	}

	assert(testInvariant());
}

void movie_root::add_key_listener(as_object* listener)
{
	if ( m_key_listeners.insert(listener).second )
	{
		//log_msg("Added key listener %p", (void*)listener);
	}
	else
	{
		//log_msg("key listener %p was already in the known set", (void*)listener);
	}
	assert(testInvariant());
}

void movie_root::remove_key_listener(as_object* listener)
{
	//log_msg("Removing key listener %p - %u listeners currently ", (void*)listener, m_key_listeners.size());
	m_key_listeners.erase(listener);
	//log_msg("After removing key listener %p, %u listeners are left", (void*)listener, m_key_listeners.size());
	assert(testInvariant());
}
#endif // ndef NEW_KEY_LISTENER_LIST_DESIGN

void movie_root::add_mouse_listener(as_object* listener)
{
	m_mouse_listeners.insert(listener);
	assert(testInvariant());
}

void movie_root::remove_mouse_listener(as_object* listener)
{
	m_mouse_listeners.erase(listener);
	assert(testInvariant());
}

void movie_root::notify_mouse_listeners(const event_id& event)
{
	//log_msg("Notifying " SIZET_FMT " listeners about %s",
	//		m_mouse_listeners.size(), event.get_function_name().c_str());

	for (ListenerSet::iterator iter = m_mouse_listeners.begin();
			iter != m_mouse_listeners.end(); ++iter)
	{
		// sprite, button & input_edit_text characters
		// TODO: invoke functions on non-characters !
		character* ch = dynamic_cast<character*>(iter->get()); 
		if ( ch )
		{
			ch->on_event(event);
		}
	}

	assert(testInvariant());
}

character*
movie_root::getFocus()
{
	assert(testInvariant());
	return m_active_input_text;
}

void
movie_root::setFocus(character* ch)
{
	m_active_input_text = ch;
	assert(testInvariant());
}

character*
movie_root::getActiveEntityUnderPointer() const
{
	return m_mouse_button_state.m_active_entity.get();
}

bool
movie_root::isMouseOverActiveEntity() const
{
	assert(testInvariant());

	boost::intrusive_ptr<character> entity ( m_mouse_button_state.m_active_entity );
	if ( ! entity.get() ) return false;

#if 0 // debugging...
	log_msg("The active entity under the pointer is a %s",
		typeid(*entity).name());
#endif

	return true;
}

void
movie_root::add_invalidated_bounds(InvalidatedRanges& ranges, bool force)
{
	if ( isInvalidated() )
	{
		ranges.setWorld();
		return;
	}

	for (Levels::reverse_iterator i=_movies.rbegin(), e=_movies.rend(); i!=e; ++i)
	{
		i->second->add_invalidated_bounds(ranges, force);
	}
}

int
movie_root::minPopulatedPriorityQueue() const
{
	for (int l=0; l<apSIZE; ++l)
	{
		if ( ! _actionQueue[l].empty() ) return l;
	}
	return apSIZE;
}

int
movie_root::processActionQueue(int lvl)
{
	ActionQueue& q = _actionQueue[lvl];

	assert( minPopulatedPriorityQueue() == lvl );

#ifdef GNASH_DEBUG
	static unsigned calls=0;
	++calls;
	bool actionsToProcess = !q.empty();
	if ( actionsToProcess )
	{
		log_debug(" Processing %d actions in priority queue %d (call %u)", q.size(), lvl, calls);
	}
#endif

	// _actionQueue may be changed due to actions (appended-to)
	// this loop might be optimized by using an iterator
	// and a final call to .clear() 
	while ( ! q.empty() )
	{
		ExecutableCode* code = q.front();
		code->execute();
		q.pop_front(); 
		delete code;

		int minLevel = minPopulatedPriorityQueue();
		if ( minLevel < lvl )
		{
#ifdef GNASH_DEBUG
			log_debug(" Actions pushed in priority %d (< %d), restarting the scan (call %u)", minLevel, lvl, calls);
#endif
			return minLevel;
		}
	}

	assert(q.empty());

#ifdef GNASH_DEBUG
	if ( actionsToProcess )
	{
		log_debug(" Done processing actions in priority queue %d (call %u)", lvl, calls);
	}
#endif

	return minPopulatedPriorityQueue();

}

void
movie_root::processActionQueue()
{
	if ( _disableScripts )
	{
		//log_debug(_("Scripts are disabled, global instance list has %d elements"), _liveChars.size());
		/// cleanup anything pushed later..
		clearActionQueue();
		return;
	}

	_processingActionLevel=minPopulatedPriorityQueue();
	while ( _processingActionLevel<apSIZE )
	{
		_processingActionLevel = processActionQueue(_processingActionLevel);
	}

}

void
movie_root::pushAction(std::auto_ptr<ExecutableCode> code, int lvl)
{
	assert(lvl >= 0 && lvl < apSIZE);

	// Immediately execute code targetted at a lower level while processing
	// an higher level.
	if ( processingActions() && lvl < _processingActionLevel )
	{
		log_debug("Action pushed in level %d executed immediately (as we are currently executing level %d)", lvl, _processingActionLevel);
		code->execute();
		return;
	}

	_actionQueue[lvl].push_back(code.release());
}

void
movie_root::pushAction(const action_buffer& buf, boost::intrusive_ptr<character> target, int lvl)
{
	assert(lvl >= 0 && lvl < apSIZE);
#ifdef GNASH_DEBUG
	log_msg("Pushed action buffer for target %s", target->getTargetPath().c_str());
#endif

	std::auto_ptr<ExecutableCode> code ( new GlobalCode(buf, target) );

	// Immediately execute code targetted at a lower level while processing
	// an higher level.
	if ( processingActions() && lvl < _processingActionLevel )
	{
		log_debug("Action pushed in level %d executed immediately (as we are currently executing level %d)", lvl, _processingActionLevel);
		code->execute();
		return;
	}

	_actionQueue[lvl].push_back(code.release());
}

void
movie_root::pushAction(boost::intrusive_ptr<as_function> func, boost::intrusive_ptr<character> target, int lvl)
{
	assert(lvl >= 0 && lvl < apSIZE);
#ifdef GNASH_DEBUG
	log_msg("Pushed function (event hanlder?) with target %s", target->getTargetPath().c_str());
#endif

	std::auto_ptr<ExecutableCode> code ( new FunctionCode(func, target) );

	// Immediately execute code targetted at a lower level while processing
	// an higher level.
	if ( processingActions() && lvl < _processingActionLevel )
	{
		log_debug("Action pushed in level %d executed immediately (as we are currently executing level %d)", lvl, _processingActionLevel);
		code->execute();
		return;
	}
	_actionQueue[lvl].push_back(code.release());
}

/* private */
void
movie_root::executeTimers()
{
	for (TimerMap::iterator it=_intervalTimers.begin(), itEnd=_intervalTimers.end();
			it != itEnd; )
	{
		// Get an iterator to next element, as we'll use
		// erase to drop cleared timers, and that would
		// invalidate the current iterator.
		//
		// FYI: it's been reported on ##iso-c++ that next
		//      C++ version will fix std::map<>::erase(iterator)
		//      to return the next valid iterator,
		//      like std::list<>::erase(iterator) does.
		//      For now, we'll have to handle this manually)
		//
		TimerMap::iterator nextIterator = it;
		++nextIterator;

		Timer* timer = it->second;

		if ( timer->cleared() )
		{
			// this timer was cleared, erase it
			delete timer;
			_intervalTimers.erase(it);
		}
		else
		{
			if ( timer->expired() )
			{
				//cout << " EXPIRED, start time is now " << timer.getStart() << endl;
				(*timer)();
			}
		}

		it = nextIterator;
	}

}

#ifdef GNASH_USE_GC
void
movie_root::markReachableResources() const
{
	// Mark movie levels as reachable
	for (Levels::const_reverse_iterator i=_movies.rbegin(), e=_movies.rend(); i!=e; ++i)
	{
		i->second->setReachable();
	}

	// Mark mouse entities 
	m_mouse_button_state.markReachableResources();
	
	// Mark timer targets
	for (TimerMap::const_iterator i=_intervalTimers.begin(), e=_intervalTimers.end();
			i != e; ++i)
	{
		i->second->markReachableResources();
	}

	// Mark resources reachable by queued action code
    for (int lvl=0; lvl<apSIZE; ++lvl)
    {
        const ActionQueue& q = _actionQueue[lvl];
    	for (ActionQueue::const_iterator i=q.begin(), e=q.end();
    			i != e; ++i)
    	{
    		(*i)->markReachableResources();
    	}
    }

#ifdef NEW_KEY_LISTENER_LIST_DESIGN
	// Mark key listeners
	for (KeyListeners::const_iterator i=_keyListeners.begin(), e=_keyListeners.end();
			i != e; ++i)
	{
		i->setReachable();
	}
#else
	// Mark key listeners
	for (ListenerSet::const_iterator i=m_key_listeners.begin(), e=m_key_listeners.end();
			i != e; ++i)
	{
		(*i)->setReachable();
	}
#endif

	// Mark global key object
	if ( _keyobject ) _keyobject->setReachable();

	// TODO: we should theoretically NOT need to mark _liveChars here
	// 	 as any element in this list should be NOT unloaded and
	// 	 thus marked as reachable by it's parent or properly unloaded
	// 	 and thus removed from this list by cleanupDisplayList.
	// 	 Due to some bug I'm researching on, we'll mark them for now...
	// 	 See http://savannah.gnu.org/bugs/index.php?21070
	//
	//log_debug("Marking %d live chars", _liveChars.size());
	for (LiveChars::const_iterator i=_liveChars.begin(), e=_liveChars.end();
			i != e; ++i)
	{
		(*i)->setReachable();
	}
	
	
}
#endif // GNASH_USE_GC

character *
movie_root::getTopmostMouseEntity(float x, float y)
{
	for (Levels::reverse_iterator i=_movies.rbegin(), e=_movies.rend(); i!=e; ++i)
	{
		character* ret = i->second->get_topmost_mouse_entity(x, y);
		if ( ret ) return ret;
	}
	return NULL;
}

void
movie_root::cleanupDisplayList()
{

#define GNASH_DEBUG_INSTANCE_LIST 1

#ifdef GNASH_DEBUG_INSTANCE_LIST
	// This 
	static size_t maxLiveChars = 0;
#endif

	// Remove unloaded characters from the _liveChars list
	for (LiveChars::iterator i=_liveChars.begin(), e=_liveChars.end(); i!=e;)
	{
		AdvanceableCharacter ch = *i;
		if ( ch->isUnloaded() )
		{
			// the sprite might have been destroyed already
			// by effect of an unload() call with no onUnload
			// handlers available either in self or child
			// characters
			if ( ! ch->isDestroyed() ) ch->destroy();
			i = _liveChars.erase(i);
		}
		else
		{
			++i;
		}
	}

#ifdef GNASH_DEBUG_INSTANCE_LIST
	if ( _liveChars.size() > maxLiveChars )
	{
		maxLiveChars = _liveChars.size();
		log_debug("Global instance list grew to " SIZET_FMT " entries", maxLiveChars);
	}
#endif

	// Let every sprite cleanup the local DisplayList
        //
        // TODO: we might skip this additinal scan by delegating
        //       cleanup of the local DisplayLists in the ::display
        //       method of each sprite, but that will introduce 
        //       problems when we'll implement skipping ::display()
        //       when late on FPS. Alternatively we may have the
        //       sprite_instance::markReachableResources take care
        //       of cleaning up unloaded... but that will likely
        //       introduce problems when allowing the GC to run
        //       at arbitrary times.
        //       The invariant to keep is that cleanup of unloaded characters
        //       in local display lists must happen at the *end* of global action
        //       queue processing.
        //
        for (Levels::reverse_iterator i=_movies.rbegin(), e=_movies.rend(); i!=e; ++i)
        {
                i->second->cleanupDisplayList();
        }
}

/*static private*/
void
movie_root::advanceLiveChar(boost::intrusive_ptr<character> ch, float delta_time)
{

	if ( ! ch->isUnloaded() )
	{
#ifdef GNASH_DEBUG
		log_debug("    advancing character %s", ch->getTarget().c_str());
#endif
		ch->advance(delta_time);
	}
#ifdef GNASH_DEBUG
	else {
		log_debug("    character %s is unloaded, not advancing it", ch->getTarget().c_str());
	}
#endif
}

void
movie_root::advanceLiveChars(float delta_time)
{

#ifdef GNASH_DEBUG
	log_debug("---- movie_root::advance: %d live characters in the global list", _liveChars.size());
#endif

	std::for_each(_liveChars.begin(), _liveChars.end(), boost::bind(advanceLiveChar, _1, delta_time));
}

void
movie_root::set_background_color(const rgba& color)
{
	//GNASH_REPORT_FUNCTION;

        if ( m_background_color != color )
	{
		setInvalidated();
        	m_background_color = color;
	}
}

void
movie_root::set_background_alpha(float alpha)
{
	//GNASH_REPORT_FUNCTION;

	uint8_t newAlpha = iclamp(frnd(alpha * 255.0f), 0, 255);

        if ( m_background_color.m_a != newAlpha )
	{
		setInvalidated();
        	m_background_color.m_a = newAlpha;
	}
}

} // namespace gnash

