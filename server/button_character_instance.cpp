// button.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// SWF buttons.  Mouse-sensitive update/display, actions, etc.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "button_character_instance.h"
#include "button_character_def.h"
#include "action.h" // for as_standard_member enum

#include "ActionExec.h"
#include "sprite_instance.h"
#include "movie_root.h"
#include "VM.h"
#include "builtin_function.h"

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

	gettersetter = new builtin_function(&character::xmouse_getset, NULL);
	o.init_property("_xmouse", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::ymouse_getset, NULL);
	o.init_property("_ymouse", *gettersetter, *gettersetter);

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

}

button_character_instance::button_character_instance(
		button_character_definition* def,
		character* parent, int id)
	:
	character(parent, id),
	m_def(def),
	m_last_mouse_flags(IDLE),
	m_mouse_flags(IDLE),
	m_mouse_state(UP)
{
	assert(m_def);

	attachButtonInterface(*this);

	int r, r_num =  m_def->m_button_records.size();
	m_record_character.resize(r_num);

	for (r = 0; r < r_num; r++)
	{
		button_record& bdef = m_def->m_button_records[r];

		const matrix&	mat = m_def->m_button_records[r].m_button_matrix;
		const cxform&	cx = m_def->m_button_records[r].m_button_cxform;

		boost::intrusive_ptr<character> ch = bdef.m_character_def->create_character_instance(this, id);
		m_record_character[r] = ch;
		ch->set_matrix(mat);
		ch->set_cxform(cx);
		ch->restart();
	}

	// check up presence KeyPress events
	for (unsigned int i = 0; i < m_def->m_button_actions.size(); i++)
	{
		if (m_def->m_button_actions[i].m_conditions & 0xFE00)	// check up on CondKeyPress: UB[7]
		{
			_vm.getRoot().add_keypress_listener(this);
			break;
		}
	}

}

button_character_instance::~button_character_instance()
{
	_vm.getRoot().remove_keypress_listener(this);
}

// called from keypress listener only
// (the above line is wrong, it's also called with onConstruct, for instance)
bool
button_character_instance::on_event(const event_id& id)
{

#if 0
	if (id.m_id != event_id::KEY_PRESS)
	{
		log_warning("Unsupported event for button instance: %s", id.get_function_name().c_str());
		return false;
	}
#endif

	static const event_id s_key[32] =
	{
		event_id(),
		event_id(event_id::KEY_PRESS, key::LEFT),
		event_id(event_id::KEY_PRESS, key::RIGHT),
		event_id(event_id::KEY_PRESS, key::HOME),
		event_id(event_id::KEY_PRESS, key::END),
		event_id(event_id::KEY_PRESS, key::INSERT),
		event_id(event_id::KEY_PRESS, key::DELETEKEY),
		event_id(),
		event_id(event_id::KEY_PRESS, key::BACKSPACE),	//8
		event_id(),
		event_id(),
		event_id(),
		event_id(),
		event_id(event_id::KEY_PRESS, key::ENTER),	//13
		event_id(event_id::KEY_PRESS, key::UP),
		event_id(event_id::KEY_PRESS, key::DOWN),
		event_id(event_id::KEY_PRESS, key::PGUP),
		event_id(event_id::KEY_PRESS, key::PGDN),
		event_id(event_id::KEY_PRESS, key::TAB),
		// 32-126 folows ASCII
	};

	bool called = false;

	sprite_instance* parent = get_parent()->to_movie();
	assert ( parent );

	// Add appropriate actions to the movie's execute list...
	// TODO: should we execute immediately instead ?
	for (size_t i = 0, ie=m_def->m_button_actions.size(); i<ie; ++i)
	{
		button_action& ba = m_def->m_button_actions[i];

		int keycode = (ba.m_conditions & 0xFE00) >> 9;
		event_id key_event = keycode < 32 ? s_key[keycode] : event_id(event_id::KEY_PRESS, (key::code) keycode);
		if (key_event == id)
		{
			// Matching action.
			for (size_t j=0, je=ba.m_actions.size(); j<je; ++j)
			{
				parent->add_action_buffer(ba.m_actions[j]);
			}
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
	int r, r_num =  m_record_character.size();
	for (r = 0; r < r_num; r++)
	{
		m_record_character[r]->restart();
	}
}

void
button_character_instance::advance(float delta_time)
{
//			printf("%s:\n", __PRETTY_FUNCTION__); // FIXME:
	// Implement mouse-drag.
	character::do_mouse_drag();

	matrix	mat = get_world_matrix();

	// Advance our relevant characters.
	{for (unsigned int i = 0; i < m_def->m_button_records.size(); i++)
	{
		button_record&	rec = m_def->m_button_records[i];
		if (m_record_character[i] == NULL)
		{
			continue;
		}

		// Matrix
		matrix sub_matrix = mat;
		sub_matrix.concatenate(rec.m_button_matrix);

		// Advance characters that are activated by the new mouse state
		if (((m_mouse_state == UP) && (rec.m_up)) ||
		    ((m_mouse_state == DOWN) && (rec.m_down)) ||
		    ((m_mouse_state == OVER) && (rec.m_over)))
		{
			m_record_character[i]->advance(delta_time);
		}
	}}
}


void
button_character_instance::display()
{
// 		        GNASH_REPORT_FUNCTION;

  // repeat for each layer to ensure correct depths
  for (int layer=m_def->m_min_layer; layer<=m_def->m_max_layer; layer++) 
  {   

  	for (unsigned int i = 0; i < m_def->m_button_records.size(); i++)
  	{
  		button_record&	rec = m_def->m_button_records[i];
  		if (m_record_character[i] == NULL)
  		{
  			continue;
  		}
  		if (m_def->m_button_records[i].m_button_layer != layer)
  		{
  			continue;
  		}
  		if ((m_mouse_state == UP && rec.m_up)
  		    || (m_mouse_state == DOWN && rec.m_down)
  		    || (m_mouse_state == OVER && rec.m_over))
  		{
				matrix	mat = get_world_matrix();
				m_record_character[i]->display();
  		}
  	} // for button record
  } // for layer

	clear_invalidated();
	do_display_callback();
}


character*
button_character_instance::get_topmost_mouse_entity(float x, float y)
// Return the topmost entity that the given point covers.  NULL if none.
// I.e. check against ourself.
{
	if (get_visible() == false) {
		return false;
	}

	matrix	m = get_matrix();
	point	p;
	m.transform_by_inverse(&p, point(x, y));

	{for (unsigned int i = 0; i < m_def->m_button_records.size(); i++)
	{
		button_record&	rec = m_def->m_button_records[i];
		if (rec.m_character_id < 0 || rec.m_hit_test == false)
		{
			continue;
		}

		// Find the mouse position in button-record space.
		point	sub_p;
		rec.m_button_matrix.transform_by_inverse(&sub_p, p);

		if (rec.m_character_def->point_test_local(sub_p.m_x, sub_p.m_y))
		{
			// The mouse is inside the shape.
			return this;
			// @@ Are there any circumstances where this is correct:
			//return m_record_character[i].get();
		}
	}}

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
	case event_id::MOUSE_UP:
	case event_id::ROLL_OVER:
	case event_id::DRAG_OUT:
		new_state = OVER;
		break;

	case event_id::PRESS:
	case event_id::DRAG_OVER:
	case event_id::MOUSE_DOWN:
		new_state = DOWN;
		break;

	default:
		assert(0);	// missed a case?
		break;
	};
	
	if (new_state!=m_mouse_state) {
    set_invalidated();
    m_mouse_state=new_state; 
  }
    

	// Button transition sounds.
	if (m_def->m_sound != NULL)
	{
		int bi; // button sound array index [0..3]
		sound_handler* s = get_sound_handler();

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

	// restart the characters of the new state.
	restart_characters(c);

	// From: "ActionScript - The Definiteve Guide" by Colin Moock
	// (chapter 10: Events and Event Handlers)

	// "Event-based code [..] is said to be executed asynchronously
	//  because the triggering of events can occur at arbitrary times."

	// Immediately execute all events actions (don't append to
	// parent's action buffer for later execution!)

	{for (unsigned int i = 0; i < m_def->m_button_actions.size(); i++)
	{
		if (m_def->m_button_actions[i].m_conditions & c)
		{
			// Matching action.
			for (unsigned int j = 0; j < m_def->m_button_actions[i].m_actions.size(); j++)
			{
				action_buffer* ab = m_def->m_button_actions[i].m_actions[j];
				assert(ab);
				IF_VERBOSE_ACTION(
					log_action("Executing actions for "
						"button condition %d\n", c);
				);
				ActionExec exec(*ab, get_environment());
				exec();
				
			}
		}
	}}

	// check for built-in event handler.
	as_value method;
	if ( get_event_handler(event, &method) && ! method.is_undefined() )
	{
		call_method0(method, &(get_environment()), this);
	}
	else
	{
		//log_warning("No handler for event: %s", event.get_function_name().c_str());
	}


	// Call conventional attached method.
	// @@ TODO
}


void
button_character_instance::restart_characters(int condition)
{
	// Restart our relevant characters
	for (unsigned int i = 0; i < m_def->m_button_records.size(); i++)
	{
		bool	restart = false;
		button_record* rec = &m_def->m_button_records[i];

		switch (m_mouse_state)
		{
		case OVER:
		{
			if ((rec->m_over) && (condition & button_action::IDLE_TO_OVER_UP))
			{
				restart = true;
			}
			break;
		}
		// @@ Hm, are there other cases where we restart stuff?
		default:
		{
			break;
		}
		}

		if (restart == true)
		{
			m_record_character[i]->restart();
		}
	}
}

//
// ActionScript overrides
//


void 
button_character_instance::get_invalidated_bounds(rect* bounds, bool force) 
{
  if (!m_visible) return; // not visible anyway
  
  bounds->expand_to_rect(m_old_invalidated_bounds);

  // TODO: Instead of using these for loops again and again, wouldn't it be a
  // good idea to have a generic "get_record_character()" method?
	for (unsigned int i = 0; i < m_def->m_button_records.size(); i++)
	{
		button_record&	rec = m_def->m_button_records[i];
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
        m_record_character[i]->get_invalidated_bounds(bounds, 
          force||m_invalidated);        
		}
	}
}

float
button_character_instance::get_width() const
{
	for (unsigned int i = 0; i < m_def->m_button_records.size(); i++)
	{
		button_record&	rec = m_def->m_button_records[i];
		if (m_record_character[i] == NULL)
		{
			continue;
		}
		if ((m_mouse_state == UP && rec.m_up)
		    || (m_mouse_state == DOWN && rec.m_down)
		    || (m_mouse_state == OVER && rec.m_over))
		{
			return m_record_character[i]->get_width();
		}
	}
	return 0.0;
}

float
button_character_instance::get_height() const
{
	for (unsigned int i = 0; i < m_def->m_button_records.size(); i++)
	{
		button_record&	rec = m_def->m_button_records[i];
		if (m_record_character[i] == NULL)
		{
			continue;
		}
		if ((m_mouse_state == UP && rec.m_up)
		    || (m_mouse_state == DOWN && rec.m_down)
		    || (m_mouse_state == OVER && rec.m_over))
		{
			return m_record_character[i]->get_height();
		}
	}
	return 0.0;
}

} // end of namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
