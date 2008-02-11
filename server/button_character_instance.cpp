// button_character_instance.cpp:  Mouse-sensitive buttons, for Gnash.
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


#include "button_character_instance.h"
#include "button_character_def.h"
#include "action.h" // for as_standard_member enum
#include "as_value.h"

#include "ActionExec.h"
#include "sprite_instance.h"
#include "movie_root.h"
#include "VM.h"
#include "builtin_function.h"
#include "fn_call.h" // for shared ActionScript getter-setters
#include "GnashException.h" // for shared ActionScript getter-setters
#include "ExecutableCode.h"
#include "namedStrings.h"

/** \page buttons Buttons and mouse behaviour

Observations about button & mouse behavior

Entities that receive mouse events: only buttons and sprites, AFAIK

When the mouse button goes down, it becomes "captured" by whatever
element is topmost, directly below the mouse at that moment.  While
the mouse is captured, no other entity receives mouse events,
regardless of how the mouse or other elements move.

The mouse remains captured until the mouse button goes up.  The mouse
remains captured even if the element that captured it is removed from
the display list.

If the mouse isn't above a button or sprite when the mouse button goes
down, then the mouse is captured by the background (i.e. mouse events
just don't get sent, until the mouse button goes up again).

Mouse events:

+------------------+---------------+-------------------------------------+
| Event            | Mouse Button  | description                         |
=========================================================================
| onRollOver       |     up        | sent to topmost entity when mouse   |
|                  |               | cursor initially goes over it       |
+------------------+---------------+-------------------------------------+
| onRollOut        |     up        | when mouse leaves entity, after     |
|                  |               | onRollOver                          |
+------------------+---------------+-------------------------------------+
| onPress          |  up -> down   | sent to topmost entity when mouse   |
|                  |               | button goes down.  onRollOver       |
|                  |               | always precedes onPress.  Initiates |
|                  |               | mouse capture.                      |
+------------------+---------------+-------------------------------------+
| onRelease        |  down -> up   | sent to active entity if mouse goes |
|                  |               | up while over the element           |
+------------------+---------------+-------------------------------------+
| onDragOut        |     down      | sent to active entity if mouse      |
|                  |               | is no longer over the entity        |
+------------------+---------------+-------------------------------------+
| onReleaseOutside |  down -> up   | sent to active entity if mouse goes |
|                  |               | up while not over the entity.       |
|                  |               | onDragOut always precedes           |
|                  |               | onReleaseOutside                    |
+------------------+---------------+-------------------------------------+
| onDragOver       |     down      | sent to active entity if mouse is   |
|                  |               | dragged back over it after          |
|                  |               | onDragOut                           |
+------------------+---------------+-------------------------------------+

There is always one active entity at any given time (considering NULL to
be an active entity, representing the background, and other objects that
don't receive mouse events).

When the mouse button is up, the active entity is the topmost element
directly under the mouse pointer.

When the mouse button is down, the active entity remains whatever it
was when the button last went down.

The active entity is the only object that receives mouse events.

!!! The "trackAsMenu" property alters this behavior!  If trackAsMenu
is set on the active entity, then onReleaseOutside is filtered out,
and onDragOver from another entity is allowed (from the background, or
another trackAsMenu entity). !!!


Pseudocode:

active_entity = NULL
mouse_button_state = UP
mouse_inside_entity_state = false
frame loop:
  if mouse_button_state == DOWN

    // Handle trackAsMenu
    if (active_entity->trackAsMenu)
      possible_entity = topmost entity below mouse
      if (possible_entity != active_entity && possible_entity->trackAsMenu)
        // Transfer to possible entity
	active_entity = possible_entity
	active_entity->onDragOver()
	mouse_inside_entity_state = true;

    // Handle onDragOut, onDragOver
    if (mouse_inside_entity_state == false)
      if (mouse is actually inside the active_entity)
        // onDragOver
	active_entity->onDragOver()
        mouse_inside_entity_state = true;

    else // mouse_inside_entity_state == true
      if (mouse is actually outside the active_entity)
        // onDragOut
	active_entity->onDragOut()
	mouse_inside_entity_state = false;

    // Handle onRelease, onReleaseOutside
    if (mouse button is up)
      if (mouse_inside_entity_state)
        // onRelease
        active_entity->onRelease()
      else
        // onReleaseOutside
	if (active_entity->trackAsMenu == false)
          active_entity->onReleaseOutside()
      mouse_button_state = UP
    
  if mouse_button_state == UP
    new_active_entity = topmost entity below the mouse
    if (new_active_entity != active_entity)
      // onRollOut, onRollOver
      active_entity->onRollOut()
      active_entity = new_active_entity
      active_entity->onRollOver()
    
    // Handle press
    if (mouse button is down)
      // onPress
      active_entity->onPress()
      mouse_inside_entity_state = true
      mouse_button_state = DOWN

*/


namespace gnash {

/// A couple of typedefs to make code neater
typedef button_character_instance Button;
typedef boost::intrusive_ptr<Button> ButtonPtr;

static bool charDepthLessThen(const character* ch1, const character* ch2) 
{
	return ch1->get_depth() < ch2->get_depth();
}

static void
attachButtonInterface(as_object& o)
{
	//int target_version = o.getVM().getSWFVersion();

	boost::intrusive_ptr<builtin_function> gettersetter;

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
	
	gettersetter = new builtin_function(&character::target_getset, NULL);
	o.init_property("_target", *gettersetter, *gettersetter);
	

#if 0
	gettersetter = new builtin_function(&character::onrollover_getset, NULL);
	o.init_property("onRollOver", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::onrollout_getset, NULL);
	o.init_property("onRollOut", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::onpress_getset, NULL);
	o.init_property("onPress", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::onrelease_getset, NULL);
	o.init_property("onRelease", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::onreleaseoutside_getset, NULL);
	o.init_property("onReleaseOutside", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::onload_getset, NULL);
	o.init_property("onLoad", *gettersetter, *gettersetter);
#endif

	//--

	gettersetter = new builtin_function(&button_character_instance::enabled_getset, NULL);
	o.init_property("enabled", *gettersetter, *gettersetter);

}

button_character_instance::button_character_instance(
		button_character_definition* def,
		character* parent, int id)
	:
	character(parent, id),
	m_def(def),
	m_last_mouse_flags(IDLE),
	m_mouse_flags(IDLE),
	m_mouse_state(UP),
	m_enabled(true)
{
	assert(m_def);

	attachButtonInterface(*this);

	// check up presence Key events
	// TODO: use a service of button_character_def, not this hard-coded thing here
	for (size_t i = 0, e = m_def->m_button_actions.size(); i < e; ++i)
	{
		// TODO: use labels, not magic numbers here !!
		if (m_def->m_button_actions[i]->m_conditions & 0xFE00)	// check up on CondKeyPress: UB[7]
		{
			_vm.getRoot().add_key_listener(this);
			break;
		}
	}

}

button_character_instance::~button_character_instance()
{
	_vm.getRoot().remove_key_listener(this);
}


bool 
button_character_instance::get_enabled()
{
	return m_enabled;
}

void 
button_character_instance::set_enabled(bool value)
{
	if (value == m_enabled) return;
	m_enabled = value; 
	
	// NOTE: no visual change
}


as_value
button_character_instance::enabled_getset(const fn_call& fn)
{
	ButtonPtr ptr = ensureType<Button>(fn.this_ptr);

	as_value rv;

	if ( fn.nargs == 0 ) // getter
	{
		rv = as_value(ptr->get_enabled());
	}
	else // setter
	{
		ptr->set_enabled(fn.arg(0).to_bool());
	}
	return rv;
}



// called from Key listener only
// (the above line is wrong, it's also called with onConstruct, for instance)
bool
button_character_instance::on_event(const event_id& id)
{

	if( (id.m_id==event_id::KEY_PRESS) && (id.keyCode == key::INVALID) )
	{
		// onKeypress only responds to valid key code
		return false;
	}

	bool called = false;

	// Add appropriate actions to the global action list ...
	// TODO: should we execute immediately instead ?
	for (size_t i = 0, ie=m_def->m_button_actions.size(); i<ie; ++i)
	{
		button_action& ba = *(m_def->m_button_actions[i]);

		int keycode = (ba.m_conditions & 0xFE00) >> 9;
		
		// Test match between button action conditions and the SWF code
		// that maps to id.keyCode (the gnash unique key code). 
		if (id.m_id == event_id::KEY_PRESS && gnash::key::codeMap[id.keyCode][key::SWF] == keycode)
		{
			// Matching action.
			VM::get().getRoot().pushAction(ba.m_actions, boost::intrusive_ptr<character>(this));
			called = true;
		}
	}

	return called;
}

void
button_character_instance::restart()
{
  set_invalidated();
	m_last_mouse_flags = IDLE;
	m_mouse_flags = IDLE;
	m_mouse_state = UP;
	size_t r, r_num =  m_record_character.size();
	for (r = 0; r < r_num; r++)
	{
		m_record_character[r]->restart();
	}
}

void
button_character_instance::display()
{
//	GNASH_REPORT_FUNCTION;

	std::vector<character*> actChars;
	get_active_characters(actChars);
	std::sort(actChars.begin(), actChars.end(), charDepthLessThen);

	std::for_each(actChars.begin(), actChars.end(), std::mem_fun(&character::display)); 

	clear_invalidated();
}


character*
button_character_instance::get_topmost_mouse_entity(float x, float y)
// Return the topmost entity that the given point covers.  NULL if none.
// I.e. check against ourself.
{
	if ( (!get_visible()) || (!get_enabled()))
	{
		return 0;
	}

	//-------------------------------------------------
	// Check our active and visible childrens first
	//-------------------------------------------------

	typedef std::vector<character*> Chars;
	Chars actChars;
	get_active_characters(actChars);

	if ( ! actChars.empty() )
	{
		std::sort(actChars.begin(), actChars.end(), charDepthLessThen);

		matrix  m = get_matrix();
		point p;
		m.transform_by_inverse(&p, point(x, y));

		for (Chars::reverse_iterator it=actChars.rbegin(), itE=actChars.rend(); it!=itE; ++it)
		{
			character* ch = *it;
			if ( ! ch->get_visible() ) continue;
			character *hit = ch->get_topmost_mouse_entity(p.x, p.y);
			if ( hit ) return hit;
		}
	}

	//-------------------------------------------------
	// If that failed, check our hit area
	//-------------------------------------------------

	// Find hit characters
	Chars hitChars;
	get_active_characters(hitChars, HIT);
	if ( hitChars.empty() ) return 0;

	// point is in parent's space,
	// we need to convert it in world space
	point wp(x,y);
	character* parent = get_parent();
	if ( parent )
	{
		parent->get_world_matrix().transform(wp);
	}

	for (size_t i=0, e=hitChars.size(); i<e; ++i)
	{
		character* ch = hitChars[i];

		if ( ch->pointInVisibleShape(wp.x, wp.y) )
		{
			// The mouse is inside the shape.
			return this;
		}
	}

	return NULL;
}


void
button_character_instance::on_button_event(const event_id& event)
{
  e_mouse_state new_state = m_mouse_state;
  
	// Set our mouse state (so we know how to render).
	switch (event.m_id)
	{
	case event_id::ROLL_OUT:
	case event_id::RELEASE_OUTSIDE:
		new_state = UP;
		break;

	case event_id::RELEASE:
	case event_id::ROLL_OVER:
	case event_id::DRAG_OUT:
	case event_id::MOUSE_UP:
		new_state = OVER;
		break;

	case event_id::PRESS:
	case event_id::DRAG_OVER:
	case event_id::MOUSE_DOWN:
		new_state = DOWN;
		break;

	default:
		//abort();	// missed a case?
		log_error(_("Unhandled button event %s"), event.get_function_name().c_str());
		break;
	};
	
	
	set_current_state(new_state);
    
	// Button transition sounds.
	if (m_def->m_sound != NULL)
	{
		int bi; // button sound array index [0..3]
		media::sound_handler* s = get_sound_handler();

		// Check if there is a sound handler
		if (s != NULL) {
			switch (event.m_id)
			{
			case event_id::ROLL_OUT:
				bi = 0;
				break;
			case event_id::ROLL_OVER:
				bi = 1;
				break;
			case event_id::PRESS:
				bi = 2;
				break;
			case event_id::RELEASE:
				bi = 3;
				break;
			default:
				bi = -1;
				break;
			}
			if (bi >= 0)
			{
				button_character_definition::button_sound_info& bs = m_def->m_sound->m_button_sounds[bi];
				// character zero is considered as null character
				if (bs.m_sound_id > 0)
				{
					if (m_def->m_sound->m_button_sounds[bi].m_sam != NULL)
					{
						if (bs.m_sound_style.m_stop_playback)
						{
							s->stop_sound(bs.m_sam->m_sound_handler_id);
						}
						else
						{
							s->play_sound(bs.m_sam->m_sound_handler_id, bs.m_sound_style.m_loop_count, 0, 0, (bs.m_sound_style.m_envelopes.size() == 0 ? NULL : &bs.m_sound_style.m_envelopes));
						}
					}
				}
			}
		}
	}


	// @@ eh, should just be a lookup table.
	int	c = 0;
	if (event.m_id == event_id::ROLL_OVER) c |= (button_action::IDLE_TO_OVER_UP);
	else if (event.m_id == event_id::ROLL_OUT) c |= (button_action::OVER_UP_TO_IDLE);
	else if (event.m_id == event_id::PRESS) c |= (button_action::OVER_UP_TO_OVER_DOWN);
	else if (event.m_id == event_id::RELEASE) c |= (button_action::OVER_DOWN_TO_OVER_UP);
	else if (event.m_id == event_id::DRAG_OUT) c |= (button_action::OVER_DOWN_TO_OUT_DOWN);
	else if (event.m_id == event_id::DRAG_OVER) c |= (button_action::OUT_DOWN_TO_OVER_DOWN);
	else if (event.m_id == event_id::RELEASE_OUTSIDE) c |= (button_action::OUT_DOWN_TO_IDLE);
	//IDLE_TO_OVER_DOWN = 1 << 7,
	//OVER_DOWN_TO_IDLE = 1 << 8,

	// From: "ActionScript - The Definiteve Guide" by Colin Moock
	// (chapter 10: Events and Event Handlers)

	// "Event-based code [..] is said to be executed asynchronously
	//  because the triggering of events can occur at arbitrary times."

	// Immediately execute all events actions (don't append to
	// parent's action buffer for later execution!)

	for (size_t i = 0; i < m_def->m_button_actions.size(); i++)
	{
		button_action& ba = *(m_def->m_button_actions[i]);

		if (ba.m_conditions & c)
		{
			// Matching action.
			action_buffer& ab = ba.m_actions;
			IF_VERBOSE_ACTION(
				log_action(_("Executing actions for "
					"button condition %d"), c);
			);
			ActionExec exec(ab, get_environment());
			exec();
		}
	}

	// check for built-in event handler.
	std::auto_ptr<ExecutableCode> code ( get_event_handler(event) );
	if ( code.get() )
	{
		code->execute();
	}
	else
	{
		//log_debug(_("No handler for event: %s"), event.get_function_name().c_str());
	}


	// Call conventional attached method.
	boost::intrusive_ptr<as_function> method = getUserDefinedEventHandler(event.get_function_key());
	if ( method )
	{
		call_method0(as_value(method.get()), &(get_environment()), this);
	}
}

void 
button_character_instance::get_active_characters(std::vector<character*>& list)
{
	get_active_characters(list, m_mouse_state);
}

void 
button_character_instance::get_active_characters(std::vector<character*>& list,
  e_mouse_state state)
{
	list.clear();
	
	for (size_t i = 0; i < m_def->m_button_records.size(); i++)
	{
		button_record&	rec = m_def->m_button_records[i];
		assert(m_record_character.size() > i);
		if (m_record_character[i] == NULL)
		{
			continue;
		}
		if ((state == UP && rec.m_up)
		    || (state == DOWN && rec.m_down)
		    || (state == OVER && rec.m_over)
		    || (state == HIT && rec.m_hit_test))
		{
			list.push_back(m_record_character[i].get());
		}
	} // for button record	
}

void
button_character_instance::set_current_state(e_mouse_state new_state)
{
	if (new_state == m_mouse_state)
		return;
		
	// save current "display list"
	std::vector<character*> old_list;
	get_active_characters(old_list, m_mouse_state);
	
	// load new "display list" 
	// NOTE: We don't change state yet, so that set_invalidated() can 
	// load the current bounds first.
	std::vector<character*> new_list;
	get_active_characters(new_list, new_state);
		
	// see if the two lists differ and restart characters if needed
	if (new_list.size() != old_list.size())
		set_invalidated();		// something changed 
  
  size_t old_count = old_list.size();
  size_t new_count = new_list.size();
  for (size_t i=0; i<new_count; i++) {

  	bool found=false;
  	for (size_t j=0; j<old_count; j++) { 
	  	if (new_list[i] == old_list[j]) {
				found=true;
				break; 
			}
		}
		if (!found) {
			// character (re-)appeared on stage -> restart!
			new_list[i]->restart();
			set_invalidated();
		} 
	}

	// effectively change state
	m_mouse_state=new_state;
	 
}

//
// ActionScript overrides
//



void 
button_character_instance::add_invalidated_bounds(InvalidatedRanges& ranges, 
	bool force)
{
  if (!m_visible) return; // not visible anyway

	ranges.add(m_old_invalidated_ranges);  

  // TODO: Instead of using these for loops again and again, wouldn't it be a
  // good idea to have a generic "get_record_character()" method?
	for (size_t i = 0; i < m_def->m_button_records.size(); i++)
	{
		button_record&	rec = m_def->m_button_records[i];
		assert(m_record_character.size() > i);
		if (m_record_character[i] == NULL)
		{
			continue;
		}
		if ((m_mouse_state == UP && rec.m_up)
		    || (m_mouse_state == DOWN && rec.m_down)
		    || (m_mouse_state == OVER && rec.m_over))
		{
				/*bounds->expand_to_transformed_rect(get_world_matrix(), 
          m_record_character[i]->get_bound());*/
        m_record_character[i]->add_invalidated_bounds(ranges, 
          force||m_invalidated);        
		}
	}

}

geometry::Range2d<float>
button_character_instance::getBounds() const
{
	for (size_t i = 0; i < m_def->m_button_records.size(); i++)
	{
		button_record&	rec = m_def->m_button_records[i];
		assert(m_record_character.size() > i);
		if (m_record_character[i] == NULL)
		{
			continue;
		}
		if ((m_mouse_state == UP && rec.m_up)
		    || (m_mouse_state == DOWN && rec.m_down)
		    || (m_mouse_state == OVER && rec.m_over))
		{
			// TODO: should we consider having multiple characters
			//       for a single state ?
			return m_record_character[i]->getBounds();
		}
	}
	return geometry::Range2d<float>(geometry::nullRange);
}

bool
button_character_instance::pointInShape(float x, float y) const
{
	for (size_t i = 0; i < m_def->m_button_records.size(); i++)
	{
		button_record&	rec = m_def->m_button_records[i];
		assert(m_record_character.size() > i);
		if (m_record_character[i] == NULL)
		{
			continue;
		}
		if ((m_mouse_state == UP && rec.m_up)
		    || (m_mouse_state == DOWN && rec.m_down)
		    || (m_mouse_state == OVER && rec.m_over))
		{
			// TODO: should we consider having multiple characters
			//       for a single state ?
			return m_record_character[i]->pointInShape(x, y);
		}
	}
	return false; // no shape
}

as_object*
button_character_instance::get_path_element(string_table::key key)
{
	as_object* ch = get_path_element_character(key);
	if ( ch ) return ch;

	std::string name = _vm.getStringTable().value(key);
	return getChildByName(name); // possibly NULL
}

character *
button_character_instance::getChildByName(const std::string& name) const
{
	// See if we have a match on the button records list
	for (size_t i=0, n=m_record_character.size(); i<n; ++i)
	{
		character* child = m_record_character[i].get();
		const char* pat_c = child->get_name().c_str();
		const char* nam_c = name.c_str();

  		if ( _vm.getSWFVersion() >= 7 )
		{
			if (! strcmp(pat_c, nam_c) ) return child;
		}
		else
		{
			if ( ! strcasecmp(pat_c, nam_c) ) return child;
		}
	}

	return NULL;
}

void
button_character_instance::stagePlacementCallback()
{
	saveOriginalTarget(); // for soft refs

	// Register this button instance as a live character
	// do we need this???
	//_vm.getRoot().addLiveChar(this);

	size_t r, r_num =  m_def->m_button_records.size();
	m_record_character.resize(r_num);

	for (r = 0; r < r_num; r++)
	{
		button_record& bdef = m_def->m_button_records[r];

		const matrix&	mat = bdef.m_button_matrix;
		const cxform&	cx = bdef.m_button_cxform;
		int ch_depth = bdef.m_button_layer;
		int ch_id = bdef.m_character_id;

		boost::intrusive_ptr<character> ch = bdef.m_character_def->create_character_instance(this, ch_id);
		ch->set_matrix(mat);
		ch->set_cxform(cx);
		ch->set_depth(ch_depth);
		assert(ch->get_parent() == this);

		if (ch->get_name().empty() && ch->wantsInstanceName()) 
		{
			std::string instance_name = getNextUnnamedInstanceName();
			ch->set_name(instance_name.c_str());
		}

		m_record_character[r] = ch;

		ch->stagePlacementCallback(); // give this character life (TODO: they aren't on stage, are them ?)
	}

	// there's no INITIALIZE/CONSTRUCT/LOAD/ENTERFRAME/UNLOAD events for buttons
}

#ifdef GNASH_USE_GC
void
button_character_instance::markReachableResources() const
{
	assert(isReachable());

	m_def->setReachable();

	// Markstate characters as reachable
	for (CharsVect::const_iterator i=m_record_character.begin(), e=m_record_character.end();
			i!=e; ++i)
	{
		(*i)->setReachable();
	}

	// character class members
	markCharacterReachable();
}
#endif // GNASH_USE_GC

bool
button_character_instance::unload()
{
	bool childsHaveUnload = false;

	// We need to unload all childs, or the global instance list will keep growing forever !
	//std::for_each(m_record_character.begin(), m_record_character.end(), boost::bind(&character::unload, _1));
	for (CharsVect::iterator i=m_record_character.begin(), e=m_record_character.end(); i!=e; ++i)
	{
		boost::intrusive_ptr<character> ch = *i;
		if ( ch->unload() ) childsHaveUnload = true;
		//log_debug("Button child %s (%s) unloaded", ch->getTarget().c_str(), typeName(*ch).c_str());
	}

	bool hasUnloadEvent = character::unload();

	return hasUnloadEvent || childsHaveUnload;
}

bool
button_character_instance::get_member(string_table::key name_key, as_value* val,
  string_table::key nsname)
{
  // FIXME: use addProperty interface for these !!
  // TODO: or at least have a character:: protected method take
  //       care of these ?
  //       Duplicates code in character::get_path_element_character too..
  //
  if (name_key == NSV::PROP_uROOT)
  {

    // Let ::get_root() take care of _lockroot
    movie_instance* relRoot = get_root();
    val->set_as_object( relRoot );
    return true;
  }

  // NOTE: availability of _global doesn't depend on VM version
  //       but on actual movie version. Example: if an SWF4 loads
  //       an SWF6 (to, say, _level2), _global will be unavailable
  //       to the SWF4 code but available to the SWF6 one.
  //
  if ( getSWFVersion() > 5 && name_key == NSV::PROP_uGLOBAL ) // see MovieClip.as
  {
    // The "_global" ref was added in SWF6
    val->set_as_object( _vm.getGlobal() );
    return true;
  }

  const std::string& name = _vm.getStringTable().value(name_key);

  movie_root& mr = _vm.getRoot();
  unsigned int levelno;
  if ( mr.isLevelTarget(name, levelno) )
  {
    movie_instance* mo = mr.getLevel(levelno).get();
    if ( mo )
    {
      val->set_as_object(mo);
      return true;
    }
    else
    {
      return false;
    }
	}

  // TOCHECK : Try object members, BEFORE display list items
  //
  if (get_member_default(name_key, val, nsname))
  {

// ... trying to be useful to Flash coders ...
// The check should actually be performed before any return
// prior to the one due to a match in the DisplayList.
// It's off by default anyway, so not a big deal.
// See bug #18457
#define CHECK_FOR_NAME_CLASHES 1
#ifdef CHECK_FOR_NAME_CLASHES
    IF_VERBOSE_ASCODING_ERRORS(
    if ( getChildByName(name) )
    {
      log_aserror(_("A button member (%s) clashes with "
          "the name of an existing character "
          "in its display list.  "
          "The member will hide the "
          "character"), name.c_str());
    }
    );
#endif

    return true;
  }


  // Try items on our display list.
  character* ch = getChildByName(name);

  if (ch)
  {
      // Found object.

      // If the object is an ActionScript referenciable one we
      // return it, otherwise we return ourselves
      if ( ch->isActionScriptReferenceable() )
      {
        val->set_as_object(ch);
      }
      else
      {
        val->set_as_object(this);
      }

      return true;
  }

  return false;

}

int
button_character_instance::getSWFVersion() const
{
	return m_def->getSWFVersion();
}

} // end of namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
