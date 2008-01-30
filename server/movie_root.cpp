// movie_root.cpp:  The root movie, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include "movie_root.h"
#include "log.h"
#include "sprite_instance.h"
#include "movie_instance.h" // for implicit upcast to sprite_instance
#include "render.h"
#include "VM.h"
#include "ExecutableCode.h"
#include "Stage.h"
#include "utility.h"
#include "URL.h"
#include "namedStrings.h"
#include "GnashException.h"
#include "sound_handler.h"
#include "timers.h" // for Timer use

#include <iostream>
#include <string>
#include <map>
#include <typeinfo>
#include <cassert>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/bind.hpp>

//#define GNASH_DEBUG 1
//#define GNASH_DEBUG_LOADMOVIE_REQUESTS_PROCESSING 1
//#define GNASH_DEBUG_TIMERS_EXPIRATION 1

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
	_movies(),
	_rootMovie(),
	_allowRescale(true),
	_invalidated(true),
	_disableScripts(false),
	_processingActionLevel(movie_root::apSIZE),
	_hostfd(-1)
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

void
movie_root::clearIntervalTimers()
{
	for (TimerMap::iterator it=_intervalTimers.begin(),
			itE=_intervalTimers.end();
			it != itE; ++it)
	{
		delete it->second;
	}
	_intervalTimers.clear();
}

movie_root::~movie_root()
{
	clearActionQueue();
	clearIntervalTimers();

	assert(testInvariant());
}

void
movie_root::setRootMovie(movie_instance* movie)
{
	_rootMovie = movie;

	m_viewport_x0 = 0;
	m_viewport_y0 = 0;
	m_viewport_width = (int)movie->get_movie_definition()->get_width_pixels();
	m_viewport_height = (int)movie->get_movie_definition()->get_height_pixels();
	m_pixel_scale = 1;

	// assert(movie->get_depth() == 0); ?
	movie->set_depth(character::staticDepthOffset);

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
	assert(static_cast<unsigned int>(movie->get_depth()) == num+character::staticDepthOffset);


	Levels::iterator it = _movies.find(movie->get_depth());
	if ( it == _movies.end() )
	{
		_movies[movie->get_depth()] = movie; 
	}
	else
    	{
		// don't leak overloaded levels

		LevelMovie lm = it->second;
		if ( lm.get() == _rootMovie.get() )
		{
			// NOTE: this is not enough to trigger
			//       an application reset. Was tested
			//       but not automated. If curious
			//       use swapDepths against _level0
			//       and load into the new target while
			//       a timeout/interval is active.
			log_debug("Replacing starting movie");
		}

		if ( num == 0 )
		{
			log_debug("Loading into _level0");

			// NOTE: this was tested but not automated, the
			//       test sets an interval and then loads something
			//       in _level0. The result is the interval is disabled.
			clearIntervalTimers();


			// TODO: check what else we should do in these cases 
			//       (like, unregistering all childs etc...)
			//       Tested, but not automated, is that other
			//       levels should be maintained alive.
		}

		it->second->destroy();
		it->second = movie;
	}

	movie->set_invalidated();
	
	/// Notify placement 
	movie->stagePlacementCallback();

	assert(testInvariant());
}

void
movie_root::swapLevels(boost::intrusive_ptr<sprite_instance> movie, int depth)
{
	assert(movie);

//#define GNASH_DEBUG_LEVELS_SWAPPING 1

	int oldDepth = movie->get_depth();

#ifdef GNASH_DEBUG_LEVELS_SWAPPING
	log_debug("Before swapLevels (source depth %d, target depth %d) levels are: ", oldDepth, depth);
	for (Levels::iterator i=_movies.begin(), e=_movies.end(); i!=e; ++i)
	{
		log_debug(" %d: %p (%s @ depth %d)", i->first, (void*)(i->second.get()), i->second->getTarget().c_str(), i->second->get_depth());
	}
#endif

	if ( oldDepth < character::staticDepthOffset ) // should include _level0 !
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%s.swapDepth(%d): movie has a depth (%d) below static depth zone (%d), won't swap it's depth"),
			movie->getTarget().c_str(), depth, oldDepth, character::staticDepthOffset);
		);
		return;
	}

	if ( oldDepth >= 0 ) 
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%s.swapDepth(%d): movie has a depth (%d) below static depth zone (%d), won't swap it's depth"),
			movie->getTarget().c_str(), depth, oldDepth, character::staticDepthOffset);
		);
		return;
	}

	int oldNum = oldDepth; // -character::staticDepthOffset;
	Levels::iterator oldIt = _movies.find(oldNum);
	if ( oldIt == _movies.end() )
	{
		log_debug("%s.swapDepth(%d): target depth (%d) contains no movie",
			movie->getTarget().c_str(), depth, oldNum);
		return;
	}

	int newNum = depth; // -character::staticDepthOffset;
	movie->set_depth(depth);
	Levels::iterator targetIt = _movies.find(newNum);
	if ( targetIt == _movies.end() )
	{
		_movies.erase(oldIt);
		_movies[newNum] = movie;
	}
	else
	{
		boost::intrusive_ptr<sprite_instance> otherMovie = targetIt->second;
		otherMovie->set_depth(oldDepth);
		oldIt->second = otherMovie;
		targetIt->second = movie;
	}
	
#ifdef GNASH_DEBUG_LEVELS_SWAPPING
	log_debug("After swapLevels levels are: ");
	for (Levels::iterator i=_movies.begin(), e=_movies.end(); i!=e; ++i)
	{
		log_debug(" %d: %p (%s @ depth %d)", i->first, (void*)(i->second.get()), i->second->getTarget().c_str(), i->second->get_depth());
	}
#endif
	
	// TODO: invalidate self, not the movie
	movie->set_invalidated();
	
	assert(testInvariant());
}

void
movie_root::dropLevel(int depth)
{
	// should be checked by caller
	assert ( depth >= 0 && depth <= 1048575 );

	Levels::iterator it = _movies.find(depth);
	if ( it == _movies.end() )
	{
		log_error("movie_root::dropLevel called against a movie not found in the levels container");
		return;
	}

	sprite_instance* mo = it->second.get();
	if ( mo == getRootMovie() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Original root movie can't be removed"));
		);
		return;
	}

	// TOCHECK: safe to erase here ?
	mo->unload();
	mo->destroy();
	_movies.erase(it);

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
	ch->set_depth(num+character::staticDepthOffset);

	save_extern_movie(extern_movie.get());

	setLevel(num, extern_movie);

	return true;
}

boost::intrusive_ptr<movie_instance>
movie_root::getLevel(unsigned int num) const
{
	Levels::const_iterator i = _movies.find(num+character::staticDepthOffset);
	if ( i == _movies.end() ) return 0;

	assert(boost::dynamic_pointer_cast<movie_instance>(i->second));
	return boost::static_pointer_cast<movie_instance>(i->second);
}

void
movie_root::reset()
{
	media::sound_handler* sh = get_sound_handler();
	if ( sh ) sh->reset();
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

	// remove all intervals
	clearIntervalTimers();

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
		// WARNING: don't allow swapping depth of the root movie !!
		const rect& frame_size = _rootMovie->get_frame_size();

		float	scale_x = m_viewport_width / TWIPS_TO_PIXELS(frame_size.width());
		float	scale_y = m_viewport_height / TWIPS_TO_PIXELS(frame_size.height());
		m_pixel_scale = fmax(scale_x, scale_y);

	}
	else // rescale not allowed, notify Stage (if any)
	{
		//log_msg("Rescaling disabled");
		boost::intrusive_ptr<Stage> stage = getStageObject();
		if ( stage ) stage->onResize();
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
		// TODO: use a named string...

		as_value kval;
		as_object* global = VM::get().getGlobal();

		std::string objName = PROPNAME("Key");
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

boost::intrusive_ptr<as_object>
movie_root::getMouseObject()
{
	VM& vm = VM::get();

	// TODO: test what happens with the global "Mouse" object
	//       is removed or overridden by the user
	if ( ! _mouseobject )
	{
		as_value val;
		as_object* global = VM::get().getGlobal();

		std::string objName = PROPNAME("Mouse");
		if (global->get_member(vm.getStringTable().find(objName), &val) )
		{
			//log_debug("Found member 'Mouse' in _global: %s", val.to_debug_string().c_str());
			_mouseobject = val.to_object();
		}
	}

	return _mouseobject;
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
	//
	// First of all, notify the _global.Key object about key event
	//
	key_as_object * global_key = notify_global_key(k, down);

	// Notify character key listeners for clip key events
	notify_key_listeners(k, down);

	// Notify both character and non-character Key listeners
	//	for user defined handerlers.
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

#if 0
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
#endif

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

    float x = PIXELS_TO_TWIPS(m_mouse_x);
    float y = PIXELS_TO_TWIPS(m_mouse_y);

    // Generate a mouse event
    m_mouse_button_state.m_topmost_entity = getTopmostMouseEntity(x, y);
    m_mouse_button_state.m_mouse_button_state_current = (m_mouse_buttons & 1);

    // Set _droptarget if dragging a sprite
    sprite_instance* dragging = 0;
    character* draggingChar = getDraggingCharacter();
    if ( draggingChar ) dragging = draggingChar->to_movie();
    if ( dragging )
    {
	// TODO: optimize making findDropTarget and getTopmostMouseEntity
	//       use a single scan.
        const character* dropChar = findDropTarget(x, y, dragging);
        if ( dropChar )
        {
            // Use target of closest script character containing this
	    dropChar = dropChar->getClosestASReferenceableAncestor();
            dragging->setDropTarget(dropChar->getTargetPath());
        }
	else dragging->setDropTarget("");

    }

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
		float xoffset = int(world_mouse.x - world_origin.x);
		float yoffset = int(world_mouse.y - world_origin.y);

		m_drag_state.setOffset(xoffset, yoffset);
	}
	assert(testInvariant());
}

void
movie_root::doMouseDrag()
{
	character* dragChar = getDraggingCharacter(); 
	if ( ! dragChar ) return; // nothing to do

	if ( dragChar->isUnloaded() )
	{
		// Reset drag state if dragging char was unloaded
		m_drag_state.reset();
		return; 
	}

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
movie_root::advance()
{
	// GNASH_REPORT_FUNCTION;

	// Do mouse drag, if needed
	doMouseDrag();

	try
	{

	// Advance all non-unloaded characters in the LiveChars list
	// in reverse order (last added, first advanced)
	// NOTE: can throw ActionLimitException
	advanceLiveChars(); 

	// Process loadMovie requests
	// 
	// NOTE: should be done before executing timers,
	// 	 see swfdec's test/trace/loadmovie-case-{5,6}.swf 
	// NOTE: processing loadMovie requests after advanceLiveChars
	//       is known to fix more tests in misc-mtasc.all/levels.swf
	//       to be checked if it keeps the swfdec testsuite safe
	processLoadMovieRequests();

	// Execute expired timers
	// NOTE: can throw ActionLimitException
	executeTimers();


	cleanupUnloadedListeners();

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
	const rect& frame_size = getRootMovie()->get_frame_size();

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
	const char* result = getRootMovie()->call_method_args(method_name,
		method_arg_fmt, args);
	va_end(args);

	return result;
}

const
char* movie_root::call_method_args(const char* method_name,
		const char* method_arg_fmt, va_list args)
{
	assert(testInvariant());
	return getRootMovie()->call_method_args(method_name, method_arg_fmt, args);
}

void movie_root::cleanupUnloadedListeners(CharacterList& ll)
{
    // remove unloaded character listeners from movie_root
    for (CharacterList::iterator iter = ll.begin(); iter != ll.end(); )
    {
        character* ch = iter->get();
        if ( ch->isUnloaded() ) iter = ll.erase(iter);
        else ++iter;
    }
    
}

void movie_root::notify_key_listeners(key::code k, bool down)
{
	// log_msg("Notifying " SIZET_FMT " character Key listeners", 
	//  m_key_listeners.size());

	KeyListeners copy = m_key_listeners;
	for (CharacterList::iterator iter = copy.begin(), itEnd=copy.end();
			iter != itEnd; ++iter)
	{
		// sprite, button & input_edit_text characters
		character* ch = iter->get();
		if ( ! ch->isUnloaded() )
		{
			if(down)
			{
				// KEY_UP and KEY_DOWN events are unrelated to any key!
				ch->on_event(event_id(event_id::KEY_DOWN, key::INVALID)); 
				ch->on_event(event_id(event_id::KEY_PRESS, key::codeMap[k][0]));
			}
			else
			{
				ch->on_event(event_id(event_id::KEY_UP, key::INVALID));   
			}
		}
	}

    assert(testInvariant());

    if( ! copy.empty() )
	{
		// process actions queued in the above step
		processActionQueue();
	}
}

/* static private */
void movie_root::add_listener(CharacterList& ll, character* listener)
{
	assert(listener);
	for(CharacterList::iterator i = ll.begin(), e = ll.end(); i != e; ++i)
	{
		// Conceptually, we don't need to add the same character twice.
		// but see edit_text_character::setFocus()...
		if(*i == listener)  return;
	}

	ll.push_front(listener);
}

/* static private */
void movie_root::remove_listener(CharacterList& ll, character* listener)
{
	assert(listener);
	for(CharacterList::iterator iter = ll.begin(); iter != ll.end(); )
	{
		if(*iter == listener) iter = ll.erase(iter);
		else ++iter;
	}
}

void
movie_root::notify_mouse_listeners(const event_id& event)
{
	//log_msg("Notifying " SIZET_FMT " listeners about %s",
	//		m_mouse_listeners.size(), event.get_function_name().c_str());

	CharacterList copy = m_mouse_listeners;
	for (CharacterList::iterator iter = copy.begin(), itEnd=copy.end();
			iter != itEnd; ++iter)
	{
		character* ch = iter->get();
		if ( ! ch->isUnloaded() )
		{
			ch->on_event(event);
		}
	}

	// Now broadcast message for Mouse listeners
	typedef boost::intrusive_ptr<as_object> ObjPtr;
	ObjPtr mouseObj = getMouseObject();
	if ( mouseObj )
	{
		mouseObj->callMethod(NSV::PROP_BROADCAST_MESSAGE, as_value(PROPNAME(event.get_function_name())));
	}

	assert(testInvariant());

    if( ! copy.empty() )
	{
		// process actions queued in the above step
		processActionQueue();
	}
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

character*
movie_root::getDraggingCharacter() const
{
	return m_drag_state.getCharacter();
}

const character*
movie_root::getEntityUnderPointer() const
{
	float x = PIXELS_TO_TWIPS(m_mouse_x);
	float y = PIXELS_TO_TWIPS(m_mouse_y);
        const character* dropChar = findDropTarget(x, y, getDraggingCharacter()); 
	return dropChar;
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
		q.pop_front(); 
		code->execute();
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
movie_root::flushHigherPriorityActionQueues()
{
    if( ! processingActions() )
	{
		// only flush the actions queue when we are processing the queue.
		// ie. we don't want to flush the queue during executing user event handlers,
		// which are not pushed at the moment.
		return;
	}

	if ( _disableScripts )
	{
		//log_debug(_("Scripts are disabled, global instance list has %d elements"), _liveChars.size());
		/// cleanup anything pushed later..
		clearActionQueue();
		return;
	}

	int lvl=minPopulatedPriorityQueue();
	while ( lvl<_processingActionLevel )
	{
		lvl = processActionQueue(lvl);
	}

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

#if 0
	// Immediately execute code targetted at a lower level while processing
	// an higher level.
	if ( processingActions() && lvl < _processingActionLevel )
	{
		log_debug("Action pushed in level %d executed immediately (as we are currently executing level %d)", lvl, _processingActionLevel);
		code->execute();
		return;
	}
#endif

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

#if 0
	// Immediately execute code targetted at a lower level while processing
	// an higher level.
	if ( processingActions() && lvl < _processingActionLevel )
	{
		log_debug("Action pushed in level %d executed immediately (as we are currently executing level %d)", lvl, _processingActionLevel);
		code->execute();
		return;
	}
#endif

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

#if 0
	// Immediately execute code targetted at a lower level while processing
	// an higher level.
	if ( processingActions() && lvl < _processingActionLevel )
	{
		log_debug("Action pushed in level %d executed immediately (as we are currently executing level %d)", lvl, _processingActionLevel);
		code->execute();
		return;
	}
#endif

	_actionQueue[lvl].push_back(code.release());
}

/* private */
void
movie_root::executeTimers()
{
#ifdef GNASH_DEBUG_TIMERS_EXPIRATION
        log_debug("Checking %d timers for expiration", _intervalTimers.size());
#endif

	unsigned long now = VM::get().getTime();

	typedef std::multimap<unsigned int, Timer*> ExpiredTimers;
	ExpiredTimers expiredTimers;

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
			unsigned long elapsed;
			if ( timer->expired(now, elapsed) )
			{
				expiredTimers.insert( make_pair(elapsed, timer) );
			}
		}

		it = nextIterator;
	}

	for (ExpiredTimers::iterator it=expiredTimers.begin(),
			itEnd=expiredTimers.end();
		it != itEnd; ++it)
	{
		it->second->executeAndReset();
	}

	if ( ! expiredTimers.empty() )
	{
		// process actions queued when executing interval callbacks
		processActionQueue();
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

    // Mark original top-level movie
    // This should always be in _movies, but better make sure
    if ( _rootMovie ) _rootMovie->setReachable();

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

    // Mark character key listeners
    std::for_each(m_key_listeners.begin(), m_key_listeners.end(), boost::bind(&character::setReachable, _1));

    // Mark character mouse listeners
    std::for_each(m_mouse_listeners.begin(), m_mouse_listeners.end(), boost::bind(&character::setReachable, _1));

    // Mark global Key object
    if ( _keyobject ) _keyobject->setReachable();

    // Mark global Mouse object
    if ( _mouseobject ) _mouseobject->setReachable();

    // Mark character being dragged, if any
    m_drag_state.markReachableResources();

    // TODO: we should theoretically NOT need to mark _liveChars here
    //   as any element in this list should be NOT unloaded and
    //   thus marked as reachable by it's parent or properly unloaded
    //   and thus removed from this list by cleanupDisplayList.
    //   Due to some bug I'm researching on, we'll mark them for now...
    //   See http://savannah.gnu.org/bugs/index.php?21070
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

const character *
movie_root::findDropTarget(float x, float y, character* dragging) const
{
	for (Levels::const_reverse_iterator i=_movies.rbegin(), e=_movies.rend(); i!=e; ++i)
	{
		const character* ret = i->second->findDropTarget(x, y, dragging);
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
movie_root::advanceLiveChar(boost::intrusive_ptr<character> ch)
{

	if ( ! ch->isUnloaded() )
	{
#ifdef GNASH_DEBUG
		log_debug("    advancing character %s", ch->getTarget().c_str());
#endif
		ch->advance();
	}
#ifdef GNASH_DEBUG
	else {
		log_debug("    character %s is unloaded, not advancing it", ch->getTarget().c_str());
	}
#endif
}

void
movie_root::advanceLiveChars()
{

#ifdef GNASH_DEBUG
	log_debug("---- movie_root::advance: %d live characters in the global list", _liveChars.size());
#endif

	std::for_each(_liveChars.begin(), _liveChars.end(), boost::bind(advanceLiveChar, _1));
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

	boost::uint8_t newAlpha = iclamp(frnd(alpha * 255.0f), 0, 255);

        if ( m_background_color.m_a != newAlpha )
	{
		setInvalidated();
        	m_background_color.m_a = newAlpha;
	}
}

character*
movie_root::findCharacterByTarget(const std::string& tgtstr_orig) const
{
	if ( tgtstr_orig.empty() ) return NULL;

	std::string tgtstr = PROPNAME(tgtstr_orig);

	VM& vm = VM::get();
	string_table& st = vm.getStringTable();

	// NOTE: getRootMovie() would be problematic in case the original
	//       root movie is replaced by a load to _level0... 
	//       (but I guess we'd also drop loadMovie requests in that
	//       case... just not tested)
	as_object* o = _movies.begin()->second.get();

	string::size_type from = 0;
	while ( string::size_type to=tgtstr.find_first_of('.', from) )
	{
		string part(tgtstr, from, to-from);
		o = o->get_path_element(st.find(part));
		if ( ! o ) {
#ifdef GNASH_DEBUG_TARGET_RESOLUTION
			log_debug("Evaluating character target path: element '%s' of path '%s' not found",
				part.c_str(), tgtstr.c_str());
#endif
			return NULL;
		}
		if ( to == string::npos ) break;
		from = to+1;
	}
	return o->to_character();
}

void
movie_root::loadMovie(const URL& url, const std::string& target, movie_root::LoadMethod method)
{
    log_debug("movie_root::loadMovie(%s, %s)", url.str().c_str(), target.c_str());
    _loadMovieRequests.push_front(LoadMovieRequest(url, target, method));
}

void
movie_root::processLoadMovieRequest(const LoadMovieRequest& r)
{
    const std::string& target = r.getTarget();
    const URL& url = r.getURL();
    LoadMethod method = r.getMethod();

    if ( target.compare(0, 6, "_level") == 0 && target.find_first_not_of("0123456789", 7) == string::npos )
    {
        unsigned int levelno = atoi(target.c_str()+6);
        log_debug(_("processLoadMovieRequest: Testing _level loading (level %u)"), levelno);
        loadLevel(levelno, url);
        return;
    }

    character* ch = findCharacterByTarget(target);
    if ( ! ch )
    {
        log_debug("Target %s of a loadMovie request doesn't exist at processing time", target.c_str());
        return;
    }

    sprite_instance* sp = ch->to_movie();
    if ( ! sp )
    {
        log_unimpl("loadMovie against a %s character", typeName(*ch).c_str());
        return;
    }

    if ( method )
    {
        log_unimpl("loadMovie with method %s", method == 1 ? "GET" : method == 2 ? "POST" : "UNKWNOWN");
    }

    sp->loadMovie(url);
}

void
movie_root::processLoadMovieRequests()
{
#ifdef GNASH_DEBUG_LOADMOVIE_REQUESTS_PROCESSING
    log_debug("Processing %d loadMovie requests", _loadMovieRequests.size());
#endif
    for (LoadMovieRequests::iterator it=_loadMovieRequests.begin();
            it != _loadMovieRequests.end(); )
    {
        const LoadMovieRequest& lr=*it;
        processLoadMovieRequest(lr);
        it = _loadMovieRequests.erase(it);
    }
}

bool
movie_root::isLevelTarget(const std::string& name, unsigned int& levelno)
{
  if ( VM::get().getSWFVersion() > 6 )
  {
    if ( name.compare(0, 6, "_level") ) return false;
  }
  else
  {
    if ( strncasecmp(name.c_str(), "_level", 6) ) return false;
  }

  if ( name.find_first_not_of("0123456789", 7) != std::string::npos ) return false;
  levelno = atoi(name.c_str()+6); // getting 0 here for "_level" is intentional
  return true;

}

} // namespace gnash

