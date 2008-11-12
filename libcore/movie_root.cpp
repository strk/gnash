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

#include "smart_ptr.h" // GNASH_USE_GC
#include "movie_root.h"
#include "log.h"
#include "MovieClip.h"
#include "movie_instance.h" // for implicit upcast to MovieClip
#include "render.h"
#include "VM.h"
#include "ExecutableCode.h"
#include "Stage_as.h"
#include "utility.h"
#include "URL.h"
#include "namedStrings.h"
#include "GnashException.h"
#include "sound_handler.h"
#include "timers.h" // for Timer use
#include "GnashKey.h" // key::code

#include <boost/algorithm/string/replace.hpp>
#include <utility>
#include <iostream>
#include <string>
#include <map>
#include <typeinfo>
#include <cassert>
#include <functional> // std::bind2nd, std::equal_to
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/bind.hpp>
#include <unistd.h>

#ifdef USE_SWFTREE
# include "tree.hh"
#endif

//#define GNASH_DEBUG 1
//#define GNASH_DEBUG_LOADMOVIE_REQUESTS_PROCESSING 1
//#define GNASH_DEBUG_TIMERS_EXPIRATION 1

// Defining the macro below prints info about
// cleanup of live chars (advanceable + key/mouse listeners)
// Is useful in particular to check for cost of multiple scans
// when a movie destruction destrois more elements.
//
// NOTE: I think the whole confusion here was introduced
//       by zou making it "optional" to ::unload() childs
//       when being unloaded. Zou was trying to avoid
//       queuing an onUnload event, which I suggested we'd
//       do by having unload() take an additional argument
//       or similar. Failing to tag childs as unloaded
//       will result in tagging them later (in ::destroy)
//       which will require scanning the lists again
//       (key/mouse + advanceable).
//       See https://savannah.gnu.org/bugs/index.php?21804
//
//#define GNASH_DEBUG_DLIST_CLEANUP 1

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


movie_root::movie_root(const movie_definition& def,
        VirtualClock& clock, const RunInfo& runInfo)
	:
    _runInfo(runInfo),
    _originalURL(def.get_url()),
    _vm(VM::init(def.get_version(), *this, clock)),
	_interfaceHandler(0),
	_fsCommandHandler(0),
	m_viewport_x0(0),
	m_viewport_y0(0),
	m_viewport_width(1),
	m_viewport_height(1),
	m_background_color(255, 255, 255, 255),
    m_background_color_set(false),
	m_timer(0.0f),
	m_mouse_x(0),
	m_mouse_y(0),
	m_mouse_buttons(0),
	_lastTimerId(0),
	m_active_input_text(NULL),
	m_time_remainder(0.0f),
	m_drag_state(),
	_movies(),
	_childs(),
	_rootMovie(),
	_invalidated(true),
	_disableScripts(false),
	_processingActionLevel(movie_root::apSIZE),
	_hostfd(-1),
	_alignMode(0),
	_scaleMode(showAll),
	_displayState(normal),
	_recursionLimit(256),
	_timeoutLimit(15),
	_movieAdvancementDelay(83), // ~12 fps by default
	_lastMovieAdvancement(0)
{
}

void
movie_root::disableScripts()
{
	_disableScripts = true;

	// NOTE: we won't clear the action queue now
	// to avoid invalidating iterators as we've
	// been probably called during processing
	// of the queue.
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
	movie_definition* md = movie->get_movie_definition();
	float fps = md->get_frame_rate();
	_movieAdvancementDelay = static_cast<int>(1000/fps);

	_lastMovieAdvancement = _vm.getTime();

	m_viewport_width = static_cast<int>(md->get_width_pixels());
	m_viewport_height = static_cast<int>(md->get_height_pixels());

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
		boost::format fmt = boost::format(_("ActionLimits hit during setRootMovie: %s. Disable scripts?")) % al.what();
		handleActionLimitHit(fmt.str());
	}
    catch (ActionParserException& e)
    {
        log_error("ActionParserException thrown during setRootMovie: %s", e.what());
    }

	cleanupAndCollect();
}

/*private*/
void
movie_root::handleActionLimitHit(const std::string& msg)
{
	bool disable = true;
	if ( _interfaceHandler ) disable = _interfaceHandler->yesNo(msg);
	else log_error("No user interface registered, assuming 'Yes' answer to "
            "question: %s", msg);
	if ( disable )
	{
		disableScripts();
		clearActionQueue();
	}
}

void
movie_root::cleanupAndCollect()
{
	// Cleanup the stack.
	_vm.getStack().clear();

	cleanupUnloadedListeners();
	cleanupDisplayList();
	GC::get().collect();
}

/* private */
void
movie_root::setLevel(unsigned int num, boost::intrusive_ptr<movie_instance> movie)
{
	assert(movie != NULL);
	assert(static_cast<unsigned int>(movie->get_depth()) ==
	                        num + character::staticDepthOffset);


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
movie_root::swapLevels(boost::intrusive_ptr<MovieClip> movie, int depth)
{
	assert(movie);

//#define GNASH_DEBUG_LEVELS_SWAPPING 1

	int oldDepth = movie->get_depth();

#ifdef GNASH_DEBUG_LEVELS_SWAPPING
	log_debug("Before swapLevels (source depth %d, target depth %d) levels are: ", oldDepth, depth);
	for (Levels::const_iterator i=_movies.begin(), e=_movies.end(); i!=e; ++i)
	{
		log_debug(" %d: %p (%s @ depth %d)", i->first, (void*)(i->second.get()), i->second->getTarget(), i->second->get_depth());
	}
#endif

	if ( oldDepth < character::staticDepthOffset ) // should include _level0 !
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%s.swapDepth(%d): movie has a depth (%d) below static depth zone (%d), won't swap its depth"),
			movie->getTarget(), depth, oldDepth, character::staticDepthOffset);
		);
		return;
	}

	if ( oldDepth >= 0 ) 
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("%s.swapDepth(%d): movie has a depth (%d) below static depth zone (%d), won't swap its depth"),
			movie->getTarget(), depth, oldDepth, character::staticDepthOffset);
		);
		return;
	}

	int oldNum = oldDepth; // -character::staticDepthOffset;
	Levels::iterator oldIt = _movies.find(oldNum);
	if ( oldIt == _movies.end() )
	{
		log_debug("%s.swapDepth(%d): target depth (%d) contains no movie",
			movie->getTarget(), depth, oldNum);
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
		boost::intrusive_ptr<MovieClip> otherMovie = targetIt->second;
		otherMovie->set_depth(oldDepth);
		oldIt->second = otherMovie;
		targetIt->second = movie;
	}
	
#ifdef GNASH_DEBUG_LEVELS_SWAPPING
	log_debug("After swapLevels levels are: ");
	for (Levels::const_iterator i=_movies.begin(), e=_movies.end(); i!=e; ++i)
	{
		log_debug(" %d: %p (%s @ depth %d)", i->first, (void*)(i->second.get()), i->second->getTarget(), i->second->get_depth());
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

	MovieClip* mo = it->second.get();
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
	boost::intrusive_ptr<movie_definition> md (
            create_library_movie(url, _runInfo));
	if (!md)
	{
		log_error(_("can't create movie_definition for %s"),
			url.str());
		return false;
	}

	boost::intrusive_ptr<movie_instance> extern_movie;
	extern_movie = md->create_movie_instance();
	if (extern_movie == NULL)
	{
		log_error(_("can't create extern movie_instance "
			"for %s"), url.str());
		return false;
	}

	// Parse query string
	MovieClip::VariableMap vars;
	url.parse_querystring(url.querystring(), vars);
	extern_movie->setVariables(vars);

	character* ch = extern_movie.get();
	ch->set_depth(num+character::staticDepthOffset);

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
	sound::sound_handler* sh = _runInfo.soundHandler();
	if ( sh ) sh->reset();
	clear();
	_disableScripts = false;
}

void
movie_root::clear()
{
	// reset background color, to allow 
	// next load to set it again.
	m_background_color.set(255,255,255,255);
	m_background_color_set = false;

	// wipe out live chars
	_liveChars.clear();

	// wipe out queued actions
	clearActionQueue();

	// wipe out all levels
	_movies.clear();

	// wipe out all childs
	_childs.clear();

	// remove all intervals
	clearIntervalTimers();

	// remove key/mouse listeners
	m_key_listeners.clear();
	m_mouse_listeners.clear();

	// Cleanup the stack.
	_vm.getStack().clear();

#ifdef GNASH_USE_GC
	// Run the garbage collector again
	GC::get().collect();
#endif

	setInvalidated();
}

boost::intrusive_ptr<Stage_as>
movie_root::getStageObject()
{
	as_value v;
	assert ( VM::isInitialized() ); // return NULL;
	as_object* global = _vm.getGlobal();
	if ( ! global ) return NULL;
	if (!global->get_member(NSV::PROP_iSTAGE, &v) ) return NULL;
	return boost::dynamic_pointer_cast<Stage_as>(v.to_object());
}
		
void
movie_root::set_display_viewport(int x0, int y0, int w, int h)
{
	assert(testInvariant());

	m_viewport_x0 = x0;
	m_viewport_y0 = y0;
	m_viewport_width = w;
	m_viewport_height = h;

	if ( _scaleMode == noScale ) // rescale not allowed, notify Stage (if any)
	{
		//log_debug("Rescaling disabled");
		boost::intrusive_ptr<Stage_as> stage = getStageObject();
		if ( stage ) stage->notifyResize();
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

boost::intrusive_ptr<Key_as>
movie_root::getKeyObject()
{
	// TODO: test what happens with the global "Key" object
	//       is removed or overridden by the user

	if (!_keyobject)
	{
		// This isn't very performant... 
		// it will keep trying to find it even if impossible
		// to find.
		// TODO: use a named string...

		as_value kval;
		as_object* global = _vm.getGlobal();

		if (global->get_member(NSV::CLASS_KEY, &kval) )
		{
			//log_debug("Found member 'Key' in _global: %s", kval.to_string());
			boost::intrusive_ptr<as_object> obj = kval.to_object();
			//log_debug("_global.Key to_object() : %s @ %p", typeid(*obj).name(), obj);
			_keyobject = boost::dynamic_pointer_cast<Key_as>( obj );
		}
	}

	return _keyobject;
}

boost::intrusive_ptr<as_object>
movie_root::getMouseObject()
{
	// TODO: test what happens with the global "Mouse" object
	//       is removed or overridden by the user
	if ( ! _mouseobject )
	{
		as_value val;
		as_object* global = _vm.getGlobal();

		if (global->get_member(NSV::CLASS_MOUSE, &val) )
		{
			//log_debug("Found member 'Mouse' in _global: %s", val);
			_mouseobject = val.to_object();
		}
	}

	return _mouseobject;
}


Key_as *
movie_root::notify_global_key(key::code k, bool down)
{
	if ( _vm.getSWFVersion() < 5 )
	{
		// Key was added in SWF5
		return NULL; 
	}

	boost::intrusive_ptr<Key_as> keyobject = getKeyObject();
	if ( keyobject )
	{
		if (down) _keyobject->set_key_down(k);
		else _keyobject->set_key_up(k);
	}
	else
	{
		log_error("gnash::notify_key_event(): _global.Key doesn't "
				"exist, or isn't the expected built-in");
	}

	return _keyobject.get();
}

bool
movie_root::notify_key_event(key::code k, bool down)
{
	//
	// First of all, notify the _global.Key object about key event
	//
	Key_as * global_key = notify_global_key(k, down);

	// Notify character key listeners for clip key events
	notify_key_listeners(k, down);

	// Notify both character and non-character Key listeners
	//	for user defined handerlers.
	if (global_key)
	{
	    try
	    {
	        // Can throw an action limit exception if the stack limit is 0 or 1,
	        // i.e. if the stack is at the limit before it contains anything.
            // A stack limit like that is hardly of any use, but could be used
            // maliciously to crash Gnash.
		    if(down)
		    {
			    global_key->notify_listeners(event_id::KEY_DOWN);
			    global_key->notify_listeners(event_id::KEY_PRESS);
		    }
		    else
		    {
			    global_key->notify_listeners(event_id::KEY_UP);
	        }
	    }
	    catch (ActionLimitException &e)
	    {
            log_error(_("ActionLimits hit notifying key listeners: %s."), e.what());
            clearActionQueue();
	    }
	}

	processActionQueue();

	return false; // should return true if needs update ...
}


bool
movie_root::notify_mouse_clicked(bool mouse_pressed, int button_mask)
{
	assert(testInvariant());

	//log_debug("Mouse click notification");
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

// Return whether any action triggered by this event requires display redraw.
// See page about events_handling (in movie_interface.h)
//
/// TODO: make this code more readable !
bool
movie_root::generate_mouse_button_events()
{

    MouseButtonState& ms = m_mouse_button_state;

	// Did this event trigger any action that needs redisplay ?
	bool need_redisplay = false;

    // TODO: have on_button_event return
    // whether the action must trigger
    // a redraw.

    switch (ms.previousButtonState)
    {
        case MouseButtonState::DOWN:
	    {
		    // TODO: Handle trackAsMenu dragOver
		    // Handle onDragOut, onDragOver
		    if (!ms.wasInsideActiveEntity)
		    {
			    if (ms.topmostEntity == ms.activeEntity)
			    {
				    // onDragOver
				    if (ms.activeEntity)
				    {
					    ms.activeEntity->on_button_event(event_id::DRAG_OVER);
					    need_redisplay=true;
				    }
				    ms.wasInsideActiveEntity = true;
			    }
		    }
		    else if (ms.topmostEntity != ms.activeEntity)
		    {
			    // onDragOut
			    if (ms.activeEntity)
			    {
				    ms.activeEntity->on_button_event(event_id::DRAG_OUT);
				    need_redisplay=true;
			    }
			    ms.wasInsideActiveEntity = false;
		    }

		    // Handle onRelease, onReleaseOutside
		    if (ms.currentButtonState == MouseButtonState::UP)
		    {
			    // Mouse button just went up.
			    ms.previousButtonState = MouseButtonState::UP;

			    if (ms.activeEntity)
			    {
				    if (ms.wasInsideActiveEntity)
				    {
					    // onRelease
					    ms.activeEntity->on_button_event(event_id::RELEASE);
					    need_redisplay = true;
				    }
				    else
				    {
					    // TODO: Handle trackAsMenu 
					    // onReleaseOutside
					    ms.activeEntity->on_button_event(event_id::RELEASE_OUTSIDE);
					    // We got out of active entity
					    ms.activeEntity = NULL; // so we don't get RollOut next...
					    need_redisplay = true;
				    }
			    }
		    }
	        return need_redisplay;
	    }


	    case MouseButtonState::UP:
        {
	        // New active entity is whatever is below the mouse right now.
	        if (ms.topmostEntity != ms.activeEntity)
	        {
		        // onRollOut
		        if (ms.activeEntity != NULL)
		        {
			        ms.activeEntity->on_button_event(event_id::ROLL_OUT);
			        need_redisplay=true;
		        }

		        ms.activeEntity = ms.topmostEntity;

		        // onRollOver
		        if (ms.activeEntity != NULL)
		        {
			        ms.activeEntity->on_button_event(event_id::ROLL_OVER);
			        need_redisplay=true;
		        }

		        ms.wasInsideActiveEntity = true;
	        }

	        // mouse button press
	        if (ms.currentButtonState == MouseButtonState::DOWN )
	        {
		        // onPress

		        // set/kill focus for current root
		        character* current_active_entity = getFocus();

		        // It's another entity ?
		        if (current_active_entity != ms.activeEntity.get())
		        {
			        // First to clean focus
			        if (current_active_entity != NULL)
			        {
				        current_active_entity->on_event(event_id::KILLFOCUS);
				        need_redisplay=true;
				        setFocus(NULL);
			        }

			        // Then to set focus
			        if (ms.activeEntity)
			        {
				        if (ms.activeEntity->on_event(event_id::SETFOCUS))
				        {
					        setFocus(ms.activeEntity.get());
				        }
			        }
		        }

		        if (ms.activeEntity)
		        {
			        ms.activeEntity->on_button_event(event_id::PRESS);
			        need_redisplay=true;
		        }
		        ms.wasInsideActiveEntity = true;
		        ms.previousButtonState = MouseButtonState::DOWN;
	        }
        }
        default:
      	    return need_redisplay;
    }

}


bool
movie_root::fire_mouse_event()
{
//	GNASH_REPORT_FUNCTION;

	assert(testInvariant());

    boost::int32_t x = PIXELS_TO_TWIPS(m_mouse_x);
    boost::int32_t y = PIXELS_TO_TWIPS(m_mouse_y);

    // Generate a mouse event
    m_mouse_button_state.topmostEntity = getTopmostMouseEntity(x, y);
    m_mouse_button_state.currentButtonState = (m_mouse_buttons & 1);

    // Set _droptarget if dragging a sprite
    MovieClip* dragging = 0;
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

    bool need_redraw = false;

    // FIXME: need_redraw might also depend on actual
    //        actions execution (consider updateAfterEvent).

    try
    {
        need_redraw = generate_mouse_button_events();
        processActionQueue();
    }
    catch (ActionLimitException& al)
    {
        boost::format fmt = boost::format(_("ActionLimits hit during mouse event processing: %s. Disable scripts ?")) % al.what();
        handleActionLimitHit(fmt.str());
    }

    return need_redraw;

}

void
movie_root::get_mouse_state(int& x, int& y, int& buttons)
{
//	    GNASH_REPORT_FUNCTION;

//             log_debug ("X is %d, Y is %d, Button is %d", m_mouse_x,
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
		SWFMatrix chmat = ch->getWorldMatrix();
		point world_origin;
		chmat.transform(&world_origin, origin);

		// Get current mouse coordinates
		boost::int32_t x, y, buttons;
		get_mouse_state(x, y, buttons);
		point world_mouse(PIXELS_TO_TWIPS(x), PIXELS_TO_TWIPS(y));

		boost::int32_t xoffset = world_mouse.x - world_origin.x;
		boost::int32_t yoffset = world_mouse.y - world_origin.y;

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

	SWFMatrix	parent_world_mat;
	character* parent = dragChar->get_parent();
	if (parent != NULL)
	{
	    parent_world_mat = parent->getWorldMatrix();
	}

	if (! m_drag_state.isLockCentered())
	{
		world_mouse.x -= m_drag_state.xOffset();
		world_mouse.y -= m_drag_state.yOffset();
	}

	if ( m_drag_state.hasBounds() )
	{
		rect bounds;
		// bounds are in local coordinate space
		bounds.enclose_transformed_rect(parent_world_mat, m_drag_state.getBounds());
		// Clamp mouse coords within a defined rect.
		bounds.clamp(world_mouse);
	}

    parent_world_mat.invert().transform(world_mouse);			
	// Place our origin so that it coincides with the mouse coords
	// in our parent frame.
	// TODO: add a character::set_translation ?
	SWFMatrix	local = dragChar->getMatrix();
	local.set_translation(world_mouse.x, world_mouse.y);
	dragChar->setMatrix(local); //no need to update caches when only changing translation
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
		log_error("FIXME: %d timers currently active, won't add another one", _intervalTimers.size());
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
	unsigned int now = _vm.getTime();

    try {

	    if ( (now - _lastMovieAdvancement) >= _movieAdvancementDelay )
	    {
		    advanceMovie();
		    // setting to 'now' discards time spent on actual rendering and
		    // action processing.
		    // if rendering and action processing takes too much time
		    // we'll always be late here, so FPS will effectively be
		    // slower. Might add a check here allowing a tolerance
		    // and printing a warnign when we're later then tolerated...
		    //
		    _lastMovieAdvancement = now; // or _vm.getTime(); ?
	    }

	    // TODO: execute timers ?
	    executeTimers();
	}
	catch (ActionLimitException& al)
    {
	    //log_error(_("ActionLimits hit during advance: %s. Disabling scripts"), al.what());
	    //disableScripts();

        // The PP does not disable scripts when the stack limit is reached,
        // but rather struggles on. 
	    log_error(_("Action limit hit during advance: %s"), al.what());
	    clearActionQueue();
    }
    catch (ActionParserException& e)
    {
        log_error(_("Buffer overread during advance: %s"), e.what());
        clearActionQueue();
    }
}
	
void
movie_root::advanceMovie()
{
	// GNASH_REPORT_FUNCTION;

	// Do mouse drag, if needed
	doMouseDrag();

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

	// Process queued actions
	// NOTE: can throw ActionLimitException
	processActionQueue();

	cleanupAndCollect();

	assert(testInvariant());
}


void
movie_root::display()
{
//	GNASH_REPORT_FUNCTION;

	assert(testInvariant());

	clearInvalidated();

	// TODO: should we consider the union of all levels bounds ?
	const rect& frame_size = getRootMovie()->get_frame_size();
	if ( frame_size.is_null() )
	{
		// TODO: check what we should do if other levels
		//       have valid bounds
		log_debug("original root movie had null bounds, not displaying");
		return;
	}

	render::begin_display(
		m_background_color,
		m_viewport_x0, m_viewport_y0,
		m_viewport_width, m_viewport_height,
		frame_size.get_x_min(), frame_size.get_x_max(),
		frame_size.get_y_min(), frame_size.get_y_max());


	for (Levels::iterator i=_movies.begin(), e=_movies.end(); i!=e; ++i)
	{
		boost::intrusive_ptr<MovieClip> movie = i->second;

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

	for (Childs::iterator i=_childs.begin(), e=_childs.end(); i!=e; ++i)
	{
		character* ch = i->second;

		ch->clear_invalidated();

		if (ch->get_visible() == false) continue;

		ch->display();

	}

	render::end_display();
}



void movie_root::cleanupUnloadedListeners(CharacterList& ll)
{
    bool needScan;

#ifdef GNASH_DEBUG_DLIST_CLEANUP
    int scansCount = 0;
#endif

    do
    {

#ifdef GNASH_DEBUG_DLIST_CLEANUP
      scansCount++;
      int cleaned =0;
#endif

      needScan=false;

      // remove unloaded character listeners from movie_root
      for (CharacterList::iterator iter = ll.begin(); iter != ll.end(); )
      {
          character* ch = iter->get();
          if ( ch->isUnloaded() )
          {
            if ( ! ch->isDestroyed() )
            {
              ch->destroy();
              needScan=true; // ->destroy() might mark already-scanned chars as unloaded
            }
            iter = ll.erase(iter);

#ifdef GNASH_DEBUG_DLIST_CLEANUP
            cleaned++;
#endif

          }

          else ++iter;
      }

#ifdef GNASH_DEBUG_DLIST_CLEANUP
      cout << " Scan " << scansCount << " cleaned " << cleaned << " instances" << endl;
#endif

    } while (needScan);
    
}

void movie_root::notify_key_listeners(key::code k, bool down)
{
	// log_debug("Notifying %d character Key listeners", 
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
				// Pass the unique Gnash key code!
				ch->on_event(event_id(event_id::KEY_PRESS, k));
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
	for(CharacterList::const_iterator i = ll.begin(), e = ll.end(); i != e; ++i)
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

#if 0
	for(CharacterList::iterator iter = ll.begin(); iter != ll.end(); )
	{
		if(*iter == listener) iter = ll.erase(iter);
		else ++iter;
	}
#else
    // This should be faster.	
	ll.remove_if(std::bind2nd(std::equal_to<boost::intrusive_ptr<character> >(), listener));
#endif	
}

void
movie_root::notify_mouse_listeners(const event_id& event)
{
	//log_debug("Notifying %d listeners about %s",
	//		m_mouse_listeners.size(), event);

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

        try
        {
            // Can throw an action limit exception if the stack limit is 0 or 1.
            // A stack limit like that is hardly of any use, but could be used
            // maliciously to crash Gnash.
		    mouseObj->callMethod(NSV::PROP_BROADCAST_MESSAGE, as_value(PROPNAME(event.get_function_name())));
		}
	    catch (ActionLimitException &e)
	    {
            log_error(_("ActionLimits hit notifying mouse events: %s."), e.what());
            clearActionQueue();
	    }
	    
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
	return m_mouse_button_state.activeEntity.get();
}

character*
movie_root::getDraggingCharacter() const
{
	return m_drag_state.getCharacter();
}

const character*
movie_root::getEntityUnderPointer() const
{
	boost::int32_t x = PIXELS_TO_TWIPS(m_mouse_x);
	boost::int32_t y = PIXELS_TO_TWIPS(m_mouse_y);
    const character* dropChar = findDropTarget(x, y, getDraggingCharacter()); 
	return dropChar;
}


bool
movie_root::isMouseOverActiveEntity() const
{
	assert(testInvariant());

	boost::intrusive_ptr<character> entity ( m_mouse_button_state.activeEntity );
	if ( ! entity.get() ) {
        return false;
    }
    else {
        return true;
    }
}

/// Get actionscript width of stage, in pixels. The width
/// returned depends on the scale mode.
unsigned int
movie_root::getStageWidth() const
{
    if (_scaleMode == noScale)
    {
        return m_viewport_width;    
    }

    // If scaling is allowed, always return the original movie size.
    return static_cast<unsigned int>(get_movie_definition()->get_width_pixels());
}

/// Get actionscript height of stage, in pixels. The height
/// returned depends on the scale mode.
unsigned int
movie_root::getStageHeight() const
{
    if (_scaleMode == noScale)
    {
        return m_viewport_height;    
    }

    // If scaling is allowed, always return the original movie size.
    return static_cast<unsigned int>(get_movie_definition()->get_height_pixels());
}

/// Takes a short int bitfield: the four bits correspond
/// to the AlignMode enum 
void
movie_root::setStageAlignment(short s)
{
    _alignMode = s;
    callInterface("Stage.align", "");
}

/// Returns a pair of enum values giving the actual alignment
/// of the stage after align mode flags are evaluated.
movie_root::StageAlign
movie_root::getStageAlignment() const
{

    /// L takes precedence over R. Default is centred.
    StageHorizontalAlign ha = STAGE_H_ALIGN_C;
    if (_alignMode.test(STAGE_ALIGN_L)) ha = STAGE_H_ALIGN_L;
    else if (_alignMode.test(STAGE_ALIGN_R)) ha = STAGE_H_ALIGN_R;

    /// T takes precedence over B. Default is centred.
    StageVerticalAlign va = STAGE_V_ALIGN_C;
    if (_alignMode.test(STAGE_ALIGN_T)) va = STAGE_V_ALIGN_T;
    else if (_alignMode.test(STAGE_ALIGN_B)) va = STAGE_V_ALIGN_B;

    return std::make_pair(ha, va);
}

/// Returns the string representation of the current align mode,
/// which must always be in the order: LTRB
std::string
movie_root::getStageAlignMode() const
{
    std::string align;
    if (_alignMode.test(STAGE_ALIGN_L)) align.push_back('L');
    if (_alignMode.test(STAGE_ALIGN_T)) align.push_back('T');
    if (_alignMode.test(STAGE_ALIGN_R)) align.push_back('R');
    if (_alignMode.test(STAGE_ALIGN_B)) align.push_back('B');
    
    return align;
}

void
movie_root::setStageScaleMode(ScaleMode sm)
{
    if ( _scaleMode == sm ) return; // nothing to do

    bool notifyResize = false;
    if ( sm == noScale || _scaleMode == noScale )
    {
        // If we go from or to noScale, we notify a resize
        // if and only if display viewport is != then actual
        // movie size
        movie_definition* md = _rootMovie->get_movie_definition();

        log_debug("Going to or from scaleMode=noScale. Viewport:%dx%d Def:%dx%d",
                    m_viewport_width, m_viewport_height,
                    md->get_width_pixels(), md->get_height_pixels());

        if ( m_viewport_width != md->get_width_pixels()
             || m_viewport_height != md->get_height_pixels() )
        {
            notifyResize = true;
        }
    }

    _scaleMode = sm;
    callInterface("Stage.align", "");    

    if ( notifyResize )
    {
        boost::intrusive_ptr<Stage_as> stage = getStageObject();
        if ( stage ) stage->notifyResize();
    }
}

void
movie_root::setStageDisplayState(const DisplayState ds)
{
    _displayState = ds;

    boost::intrusive_ptr<Stage_as> stage = getStageObject();
    if ( stage ) stage->notifyFullScreen( (_displayState == fullScreen) );

	if (!_interfaceHandler) return; // No registered callback
	
	if (_displayState == fullScreen)
	{
	    callInterface("Stage.displayState", "fullScreen");
	}
	else if (_displayState == normal)
	{
	    callInterface("Stage.displayState", "normal");
	}   
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

	for (Childs::reverse_iterator i=_childs.rbegin(), e=_childs.rend(); i!=e; ++i)
	{
		i->second->add_invalidated_bounds(ranges, force);
	}
}

void 
movie_root::dump_character_tree() const 
{
    // @@ deprecated
  for (Levels::const_iterator i=_movies.begin(), e=_movies.end(); i!=e; ++i)
	{
	  log_debug("--- movie at depth %d:", i->second->get_depth());
		i->second->dump_character_tree("CTREE: ");
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
		log_debug(" Processing %d actions in priority queue %d (call %u)",
		            q.size(), lvl, calls);
	}
#endif

	// _actionQueue may be changed due to actions (appended-to)
	// this loop might be optimized by using an iterator
	// and a final call to .clear() 
	while ( ! q.empty() )
	{
		std::auto_ptr<ExecutableCode> code(q.front());
		q.pop_front(); 
		code->execute();

		int minLevel = minPopulatedPriorityQueue();
		if ( minLevel < lvl )
		{
#ifdef GNASH_DEBUG
			log_debug(" Actions pushed in priority %d (< "
					"%d), restarting the scan (call"
					" %u)", minLevel, lvl, calls);
#endif
			return minLevel;
		}
	}

	assert(q.empty());

#ifdef GNASH_DEBUG
	if ( actionsToProcess )
	{
		log_debug(" Done processing actions in priority queue "
				"%d (call %u)", lvl, calls);
	}
#endif

	return minPopulatedPriorityQueue();
}

void
movie_root::flushHigherPriorityActionQueues()
{
    if( ! processingActions() )
	{
		// only flush the actions queue when we are 
		// processing the queue.
		// ie. we don't want to flush the queue 
		// during executing user event handlers,
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

	// Cleanup the stack.
	_vm.getStack().clear();

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
	log_debug("Pushed action buffer for target %s", 
			target->getTargetPath());
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
	log_debug("Pushed function (event hanlder?) with target %s", target->getTargetPath());
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

	unsigned long now = _vm.getTime();

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
				expiredTimers.insert( std::make_pair(elapsed, timer) );
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

    // Mark childs as reachable
    for (Childs::const_reverse_iterator i=_childs.rbegin(), e=_childs.rend(); i!=e; ++i)
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

    // Mark global Key object
    if ( _keyobject ) _keyobject->setReachable();

    // Mark global Mouse object
    if ( _mouseobject ) _mouseobject->setReachable();

    // Mark character being dragged, if any
    m_drag_state.markReachableResources();

    // NOTE: we don't need to mark _liveChars as any elements in that list
    //       should be NOT unloaded and thus marked as reachable by their
    //       parent.
    //std::for_each(_liveChars.begin(), _liveChars.end(), boost::bind(&character::setReachable, _1));
#if GNASH_PARANOIA_LEVEL > 1
    for (LiveChars::const_iterator i=_liveChars.begin(), e=_liveChars.end(); i!=e; ++i)
    {
        assert((*i)->isReachable());
    }
#endif
    
    // NOTE: cleanupUnloadedListeners should have cleaned up all unloaded key listeners 
    //       the remaining ones should be marked by their parents
    //std::for_each(m_key_listeners.begin(), m_key_listeners.end(), boost::bind(&character::setReachable, _1));
#if GNASH_PARANOIA_LEVEL > 1
    for (LiveChars::const_iterator i=m_key_listeners.begin(), e=m_key_listeners.end(); i!=e; ++i)
    {
        assert((*i)->isReachable());
    }
#endif

    // NOTE: cleanupUnloadedListeners should have cleaned up all unloaded mouse listeners 
    //       the remaining ones should be marked by their parents
    //std::for_each(m_mouse_listeners.begin(), m_mouse_listeners.end(), boost::bind(&character::setReachable, _1));
#if GNASH_PARANOIA_LEVEL > 1
    for (LiveChars::const_iterator i=m_mouse_listeners.begin(), e=m_mouse_listeners.end(); i!=e; ++i)
    {
        assert((*i)->isReachable());
    }
#endif

}
#endif // GNASH_USE_GC

character *
movie_root::getTopmostMouseEntity(boost::int32_t x, boost::int32_t y)
{

	for (Childs::reverse_iterator i=_childs.rbegin(), e=_childs.rend(); i!=e; ++i)
	{
		character* ret = i->second->get_topmost_mouse_entity(x, y);
		if ( ret ) return ret;
	}

	for (Levels::reverse_iterator i=_movies.rbegin(), e=_movies.rend(); i!=e; ++i)
	{
		character* ret = i->second->get_topmost_mouse_entity(x, y);
		if ( ret ) return ret;
	}

	return NULL;
}

const character *
movie_root::findDropTarget(boost::int32_t x, boost::int32_t y, character* dragging) const
{
	for (Childs::const_reverse_iterator i=_childs.rbegin(), e=_childs.rend(); i!=e; ++i)
	{
		const character* ret = i->second->findDropTarget(x, y, dragging);
		if ( ret ) return ret;
	}
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

	// Let every sprite cleanup the local DisplayList
        //
        // TODO: we might skip this additinal scan by delegating
        //       cleanup of the local DisplayLists in the ::display
        //       method of each sprite, but that will introduce 
        //       problems when we'll implement skipping ::display()
        //       when late on FPS. Alternatively we may have the
        //       MovieClip::markReachableResources take care
        //       of cleaning up unloaded... but that will likely
        //       introduce problems when allowing the GC to run
        //       at arbitrary times.
        //       The invariant to keep is that cleanup of unloaded characters
        //       in local display lists must happen at the *end* of global action
        //       queue processing.
        //
        for (Childs::reverse_iterator i=_childs.rbegin(), e=_childs.rend(); i!=e; ++i)
        {
                MovieClip* mc = dynamic_cast<MovieClip*>(i->second);
                if ( mc ) mc->cleanupDisplayList();
        }
        for (Levels::reverse_iterator i=_movies.rbegin(), e=_movies.rend(); i!=e; ++i)
        {
                i->second->cleanupDisplayList();
        }

	// Now remove from the instance list any unloaded character
	// Note that some characters may be unloaded but not yet destroyed,
	// in this case we'll also destroy them, which in turn might unload
	// further characters, maybe already scanned, so we keep scanning
	// the list until no more unloaded-but-non-destroyed characters
	// are found.
	// Keeping unloaded-but-non-destroyed characters wouldn't really hurt
	// in that ::advanceLiveChars would skip any unloaded characters.
	// Still, the more we remove the less work GC has to do...
	//

	bool needScan;
#ifdef GNASH_DEBUG_DLIST_CLEANUP
	int scansCount = 0;
#endif
	do {
#ifdef GNASH_DEBUG_DLIST_CLEANUP
		scansCount++;
		int cleaned =0;
#endif
		needScan=false;

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
				if ( ! ch->isDestroyed() )
				{
#ifdef GNASH_DEBUG_DLIST_CLEANUP
					cout << ch->getTarget() << "(" << typeName(*ch) << ") was unloaded but not destroyed, destroying now" << endl;
#endif
					ch->destroy();
					needScan=true; // ->destroy() might mark already-scanned chars as unloaded
				}
#ifdef GNASH_DEBUG_DLIST_CLEANUP
				else
				{
					cout << ch->getTarget() << "(" << typeName(*ch) << ") was unloaded and destroyed" << endl;
				}
#endif

				i = _liveChars.erase(i);

#ifdef GNASH_DEBUG_DLIST_CLEANUP
				cleaned++;
#endif
			}
			else
			{
				++i;
			}
		}

#ifdef GNASH_DEBUG_DLIST_CLEANUP
		cout << " Scan " << scansCount << " cleaned " << cleaned << " instances" << endl;
#endif
	} while (needScan);

#ifdef GNASH_DEBUG_INSTANCE_LIST
	if ( _liveChars.size() > maxLiveChars )
	{
		maxLiveChars = _liveChars.size();
		log_debug("Global instance list grew to %d entries", maxLiveChars);
	}
#endif

}

/*static private*/
void
movie_root::advanceLiveChar(boost::intrusive_ptr<character> ch)
{

	if ( ! ch->isUnloaded() )
	{
#ifdef GNASH_DEBUG
		log_debug("    advancing character %s", ch->getTarget());
#endif
		ch->advance();
	}
#ifdef GNASH_DEBUG
	else {
		log_debug("    character %s is unloaded, not advancing it", ch->getTarget());
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

	if ( m_background_color_set ) return;
	m_background_color_set = true;

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

	boost::uint8_t newAlpha = utility::clamp<int>(utility::frnd(alpha * 255.0f), 0, 255);

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

	string_table& st = _vm.getStringTable();

	// NOTE: getRootMovie() would be problematic in case the original
	//       root movie is replaced by a load to _level0... 
	//       (but I guess we'd also drop loadMovie requests in that
	//       case... just not tested)
	as_object* o = _movies.begin()->second.get();

	std::string::size_type from = 0;
	while ( std::string::size_type to=tgtstr.find_first_of('.', from) )
	{
		std::string part(tgtstr, from, to-from);
		o = o->get_path_element(st.find(part));
		if ( ! o ) {
#ifdef GNASH_DEBUG_TARGET_RESOLUTION
			log_debug("Evaluating character target path: element '%s' of path '%s' not found",
				part, tgtstr);
#endif
			return NULL;
		}
		if ( to == std::string::npos ) break;
		from = to+1;
	}
	return o->to_character();
}

void
movie_root::getURL(const std::string& urlstr, const std::string& target,
        const std::string& data, MovieClip::VariablesMethod method)
{

    if (_hostfd == -1)
    {
        /// If there is no hosting application, call the URL launcher. For
        /// safety, we resolve the URL against the base URL for this run.
        /// The data is not sent at all.
        URL url(urlstr, _runInfo.baseURL());

        gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
        std::string command = rcfile.getURLOpenerFormat();

        /// Try to avoid letting flash movies execute
        /// arbitrary commands (sic)
        ///
        /// Maybe we should exec here, but if we do we might have problems
        /// with complex urlOpenerFormats like:
        ///    firefox -remote 'openurl(%u)'
        ///
        ///
        /// NOTE: this escaping implementation is far from optimal, but
        ///       I felt pretty in rush to fix the arbitrary command
        ///      execution... we'll optimize if needed
        ///
        std::string safeurl = url.str(); 
        boost::replace_all(safeurl, "\\", "\\\\");    // escape backslashes first
        boost::replace_all(safeurl, "'", "\\'");    // then single quotes
        boost::replace_all(safeurl, "\"", "\\\"");    // double quotes
        boost::replace_all(safeurl, ";", "\\;");    // colons
        boost::replace_all(safeurl, " ", "\\ ");    // spaces
        boost::replace_all(safeurl, ">", "\\>");    // output redirection
        boost::replace_all(safeurl, "<", "\\<");    // input redirection
        boost::replace_all(safeurl, "&", "\\&");    // background (sic)
        boost::replace_all(safeurl, "\n", "\\n");    // newline
        boost::replace_all(safeurl, "\r", "\\r");    // return
        boost::replace_all(safeurl, "\t", "\\t");    // tab
        boost::replace_all(safeurl, "|", "\\|");    // pipe
        boost::replace_all(safeurl, "`", "\\`");    // backtick

        boost::replace_all(safeurl, "(", "\\(");    // subshell :'(
        boost::replace_all(safeurl, ")", "\\)");    // 
        boost::replace_all(safeurl, "}", "\\}");    // 
        boost::replace_all(safeurl, "{", "\\{");    // 

        boost::replace_all(safeurl, "$", "\\$");    // variable expansions

        boost::replace_all(command, "%u", safeurl);

        log_debug (_("Launching URL: %s"), command);
        std::system(command.c_str());
        return;
    }

    /// This is when there is a hosting application.
    std::ostringstream request;
    std::string querystring;
    switch (method)
    {
        case MovieClip::METHOD_POST:
             request << "POST " << target << ":" << 
                data << "$" << urlstr << std::endl;
             break;

        // METHOD_GET and METHOD_NONE are the same, except that
        // for METHOD_GET we append the variables to the query
        // string.
        case MovieClip::METHOD_GET:
            // Append vars to URL query string
            if (urlstr.find("?") == std::string::npos) {
                querystring = "?";
            }
            else querystring = "&";
            querystring.append(data);

        case MovieClip::METHOD_NONE:
            // use the original url, non parsed (the browser will know
            // better how to resolve relative urls and handle
            // javascript)
            request << "GET " << target << ":" << urlstr << std::endl;
            break;
    }

    std::string requestString = request.str();
    size_t len = requestString.length();
    // TODO: should mutex-protect this ?
    // NOTE: we are assuming the hostfd is set in blocking mode here..
    log_debug(_("Attempt to write geturl requests fd %d"), _hostfd);

    int ret = write(_hostfd, requestString.c_str(), len);
    if (ret == -1)
    {
        log_error(_("Could not write to user-provided host requests "
                    "fd %d: %s"), _hostfd, std::strerror(errno));
    }
    if (static_cast<size_t>(ret) < len)
    {
        log_error(_("Could only write %d bytes over %d required to "
                    "user-provided host requests fd %d"),
                    ret, len, _hostfd);
    }

    // The request string ends with newline, and we don't want to log that
    requestString.resize(requestString.size() - 1);
    log_debug(_("Sent request '%s' to host fd %d"), requestString, _hostfd);

}

void
movie_root::loadMovie(const std::string& urlstr, const std::string& target,
        const std::string& data, MovieClip::VariablesMethod method)
{

    /// URL security is checked in StreamProvider::getStream() down the
    /// chain.
    URL url(urlstr, _runInfo.baseURL());

    /// If the method is MovieClip::METHOD_NONE, we send no data.
    if (method == MovieClip::METHOD_GET)
    {
        std::string varsToSend(urlstr);
        /// GET: append data to query string.
        std::string qs = url.querystring();
        if ( qs.empty() ) varsToSend.insert(0, 1, '?');
        else varsToSend.insert(0, 1, '&');
        url.set_querystring(qs + varsToSend);
    }

    log_debug("movie_root::loadMovie(%s, %s)", url.str(), target);

    const std::string* postdata = NULL;

    /// POST: send variables using the POST method.
    if (method == MovieClip::METHOD_POST) postdata = &data;
    _loadMovieRequests.push_front(LoadMovieRequest(url, target, postdata));
}

void
movie_root::processLoadMovieRequest(const LoadMovieRequest& r)
{
    const std::string& target = r.getTarget();
    const URL& url = r.getURL();
    bool usePost = r.usePost();
    const std::string& postData = r.getPostData();

    if (target.compare(0, 6, "_level") == 0 &&
            target.find_first_not_of("0123456789", 7) == std::string::npos)
    {
        unsigned int levelno = std::strtoul(target.c_str() + 6, NULL, 0);
        log_debug(_("processLoadMovieRequest: Testing _level loading "
                    "(level %u)"), levelno);
        loadLevel(levelno, url);
        return;
    }

    character* ch = findCharacterByTarget(target);
    if (!ch)
    {
        log_debug("Target %s of a loadMovie request doesn't exist at "
                "processing time", target);
        return;
    }

    MovieClip* sp = ch->to_movie();
    if (!sp)
    {
        log_unimpl("loadMovie against a %s character", typeName(*ch));
        return;
    }

    if ( usePost )
    {
    	sp->loadMovie(url, &postData);
    }
    else
    {
        sp->loadMovie(url);
    }
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
  if ( _vm.getSWFVersion() > 6 )
  {
    if ( name.compare(0, 6, "_level") ) return false;
  }
  else
  {
    StringNoCaseEqual noCaseCmp;
    if (!noCaseCmp(name.substr(0, 6), "_level")) return false;
  }

  if ( name.find_first_not_of("0123456789", 7) != std::string::npos ) return false;
  levelno = strtoul(name.c_str()+6, NULL, 0); // getting 0 here for "_level" is intentional
  return true;

}

void
movie_root::setScriptLimits(boost::uint16_t recursion, boost::uint16_t timeout)
{

    // This tag reported in some sources to be ignored for movies
    // below SWF7. However, on Linux with PP version 9, the tag
    // takes effect on SWFs of any version.
    log_debug(_("Setting script limits: max recursion %d, "
            "timeout %d seconds"), recursion, timeout);

    _recursionLimit = recursion;
    _timeoutLimit = timeout;
    
}


#ifdef USE_SWFTREE
void
movie_root::getMovieInfo(tree<StringPair>& tr, tree<StringPair>::iterator it)
{

    const std::string yes = _("yes");
    const std::string no = _("no");

    tree<StringPair>::iterator localIter;

    //
    /// Stage
    //
    movie_definition* def = get_movie_definition();
    assert(def);

    it = tr.insert(it, StringPair("Stage Properties", ""));

    std::ostringstream os;
    os << "SWF " << def->get_version();
    localIter = tr.append_child(it, StringPair("SWF version", os.str()));
    localIter = tr.append_child(it, StringPair("URL", def->get_url()));

    // TODO: format this better?
    localIter = tr.append_child(it, StringPair("Descriptive metadata",
                                        def->getDescriptiveMetadata()));
 
    /// Stage: real dimensions.
    os.str("");
    os << def->get_width_pixels() <<
        "x" << def->get_height_pixels();
    localIter = tr.append_child(it, StringPair("Real dimensions", os.str()));

    /// Stage: rendered dimensions.
    os.str("");
    os << m_viewport_width << "x" << m_viewport_height;
    localIter = tr.append_child(it, StringPair("Rendered dimensions", os.str()));

#if 0
    /// Stage: pixel scale
    os.str("");
    os << m_pixel_scale;
    localIter = tr.append_child(it, StringPair("Pixel scale", os.str()));

    /// Stage: scaling allowed.
    localIter = tr.append_child(it, StringPair("Scaling allowed",
                _allowRescale ? yes : no));

    //  TODO: add _scaleMode, _valign and _haling info
#endif

    // Stage: scripts state (enabled/disabled)
    localIter = tr.append_child(it, StringPair("Scripts",
                _disableScripts ? " disabled" : "enabled"));
                
    /// Stage: number of live characters
    os.str("");
    os << _liveChars.size();
    localIter = tr.append_child(it, StringPair(_("Live characters"), os.str()));

	/// Live characters tree
	for (LiveChars::const_iterator i=_liveChars.begin(), e=_liveChars.end();
	                                                           i != e; ++i)
	{
	    (*i)->getMovieInfo(tr, localIter);
	}

}
#endif

void
movie_root::handleFsCommand(const std::string& cmd, const std::string& arg) const
{
	if ( _fsCommandHandler ) _fsCommandHandler->notify(cmd, arg);
}

std::string
movie_root::callInterface(const std::string& cmd, const std::string& arg) const
{
	if ( _interfaceHandler ) return _interfaceHandler->call(cmd, arg);

	log_error("Hosting application registered no callback for events/queries");

	return "<no iface to hosting app>";
}

void
movie_root::addChild(character* ch)
{
    int newDepth = _childs.empty() ? 0 : _childs.rbegin()->second->get_depth()+1;
    ch->set_depth(newDepth);

    assert(!_childs[newDepth]);

    _childs[newDepth] = ch;

    ch->set_invalidated();

	/// Notify placement 
	ch->stagePlacementCallback();
}

void
movie_root::addChildAt(character* ch, int depth)
{
    ch->set_depth(depth);
	Childs::iterator it = _childs.find(depth);
	if ( it == _childs.end() )
    {
        _childs[depth] = ch;
    }
    else
    {
        // don't leak overloaded childs
        it->second->destroy();
        it->second = ch;
    }

    ch->set_invalidated();

	/// Notify placement 
	ch->stagePlacementCallback();

	assert(testInvariant());
}

} // namespace gnash

