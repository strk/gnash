// button.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// SWF buttons.  Mouse-sensitive update/display, actions, etc.


#include "button.h"

#include "action.h"
#include "render.h"
#include "sound.h"
#include "stream.h"
//#include "Movie.h"
#include "movie_definition.h"


/*

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
	void	generate_mouse_button_events(mouse_button_state* ms)
	{
		smart_ptr<movie>	active_entity = ms->m_active_entity;
		smart_ptr<movie>	topmost_entity = ms->m_topmost_entity;

		if (ms->m_mouse_button_state_last == 1)
		{
			// Mouse button was down.

			// Handle trackAsMenu dragOver
			if (active_entity == NULL
			    || active_entity->get_track_as_menu())
			{
				if (topmost_entity != NULL
				    && topmost_entity != active_entity
				    && topmost_entity->get_track_as_menu() == true)
				{
					// Transfer to topmost entity, dragOver
					active_entity = topmost_entity;
					active_entity->on_button_event(event_id::DRAG_OVER);
					ms->m_mouse_inside_entity_last = true;
				}
			}

			// Handle onDragOut, onDragOver
			if (ms->m_mouse_inside_entity_last == false)
			{
				if (topmost_entity == active_entity)
				{
					// onDragOver
					if (active_entity != NULL)
					{
						active_entity->on_button_event(event_id::DRAG_OVER);
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
						active_entity->on_button_event(event_id::DRAG_OUT);
					}
					ms->m_mouse_inside_entity_last = false;
				}
			}

			// Handle onRelease, onReleaseOutside
			if (ms->m_mouse_button_state_current == 0)
			{
				// Mouse button just went up.
				ms->m_mouse_button_state_last = 0;

				if (active_entity != NULL)
				{
					if (ms->m_mouse_inside_entity_last)
					{
						// onRelease
						active_entity->on_button_event(event_id::RELEASE);
					}
					else
					{
						// onReleaseOutside
						if (active_entity->get_track_as_menu() == false)
						{
							active_entity->on_button_event(event_id::RELEASE_OUTSIDE);
						}
					}
				}
			}
		}
    
		if (ms->m_mouse_button_state_last == 0)
		{
			// Mouse button was up.

			// New active entity is whatever is below the mouse right now.
			if (topmost_entity != active_entity)
			{
				// onRollOut
				if (active_entity != NULL)
				{
					active_entity->on_button_event(event_id::ROLL_OUT);
				}

				active_entity = topmost_entity;

				// onRollOver
				if (active_entity != NULL)
				{
					active_entity->on_button_event(event_id::ROLL_OVER);
				}

				ms->m_mouse_inside_entity_last = true;
			}
    
			// mouse button press
			if (ms->m_mouse_button_state_current == 1)
			{
				// onPress
				if (active_entity != NULL)
				{
					active_entity->on_button_event(event_id::PRESS);
				}
				ms->m_mouse_inside_entity_last = true;
				ms->m_mouse_button_state_last = 1;
			}
		}

		// Write the (possibly modified) smart_ptr copies back
		// into the state struct.
		ms->m_active_entity = active_entity;
		ms->m_topmost_entity = topmost_entity;
	}


	struct button_character_instance : public character
	{
		button_character_definition*	m_def;
		std::vector< smart_ptr<character> >	m_record_character;

		enum mouse_flags
		{
			IDLE = 0,
			FLAG_OVER = 1,
			FLAG_DOWN = 2,
			OVER_DOWN = FLAG_OVER|FLAG_DOWN,

			// aliases
			OVER_UP = FLAG_OVER,
			OUT_DOWN = FLAG_DOWN
		};
		int	m_last_mouse_flags, m_mouse_flags;
		enum e_mouse_state
		{
			UP = 0,
			DOWN,
			OVER
		};
		e_mouse_state m_mouse_state;

		button_character_instance(button_character_definition* def, movie* parent, int id)
			:
			character(parent, id),
			m_def(def),
			m_last_mouse_flags(IDLE),
			m_mouse_flags(IDLE),
			m_mouse_state(UP)
		{
			assert(m_def);

			int r, r_num =  m_def->m_button_records.size();
			m_record_character.resize(r_num);

			movie_definition* movie_def = parent->get_root_movie()->get_movie_definition();

			for (r = 0; r < r_num; r++)
			{
				button_record*	bdef = &m_def->m_button_records[r];

				if (bdef->m_character_def == NULL)
				{
					// Resolve the character id.
					bdef->m_character_def = movie_def->get_character_def(bdef->m_character_id);
				}
				assert(bdef->m_character_def != NULL);

				const matrix&	mat = m_def->m_button_records[r].m_button_matrix;
				const cxform&	cx = m_def->m_button_records[r].m_button_cxform;

				smart_ptr<character>	ch = bdef->m_character_def->create_character_instance(this, id);
				m_record_character[r] = ch;
				ch->set_matrix(mat);
				ch->set_cxform(cx);
				ch->restart();
			}
		}

		~button_character_instance()
		{
		}

		movie_root*	get_root() { return get_parent()->get_root(); }
		movie*	get_root_movie() { return get_parent()->get_root_movie(); }

		void	restart()
		{
			m_last_mouse_flags = IDLE;
			m_mouse_flags = IDLE;
			m_mouse_state = UP;
			int r, r_num =  m_record_character.size();
			for (r = 0; r < r_num; r++)
			{
				m_record_character[r]->restart();
			}
		}


		virtual void	advance(float delta_time)
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


		void	display()
		{
// 		        GNASH_REPORT_FUNCTION;
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
					m_record_character[i]->display();
				}
			}

			do_display_callback();
		}

		inline int	transition(int a, int b) const
		// Combine the flags to avoid a conditional. It would be faster with a macro.
		{
			return (a << 2) | b;
		}


		virtual movie*	get_topmost_mouse_entity(float x, float y)
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
					//return m_record_character[i].get_ptr();
				}
			}}

			return NULL;
		}


		virtual void	on_button_event(event_id event)
		{
			// Set our mouse state (so we know how to render).
			switch (event.m_id)
			{
			case event_id::ROLL_OUT:
			case event_id::RELEASE_OUTSIDE:
				m_mouse_state = UP;
				break;

			case event_id::RELEASE:
			case event_id::ROLL_OVER:
			case event_id::DRAG_OUT:
				m_mouse_state = OVER;
				break;

			case event_id::PRESS:
			case event_id::DRAG_OVER:
				m_mouse_state = DOWN;
				break;

			default:
				assert(0);	// missed a case?
				break;
			};

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
							assert(m_def->m_sound->m_button_sounds[bi].m_sam != NULL);
							if (bs.m_sound_style.m_stop_playback)
							{
								s->stop_sound(bs.m_sam->m_sound_handler_id);
							}
							else
							{
								s->play_sound(bs.m_sam->m_sound_handler_id, bs.m_sound_style.m_loop_count);
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

			// Add appropriate actions to the movie's execute list...
			{for (unsigned int i = 0; i < m_def->m_button_actions.size(); i++)
			{
				if (m_def->m_button_actions[i].m_conditions & c)
				{
					// Matching action.
					for (unsigned int j = 0; j < m_def->m_button_actions[i].m_actions.size(); j++)
					{
						get_parent()->add_action_buffer(m_def->m_button_actions[i].m_actions[j]);
					}
				}
			}}

			// Call conventional attached method.
			// @@ TODO
		}


		void restart_characters(int condition)
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


		virtual void	get_mouse_state(int* x, int* y, int* buttons)
		{
			get_parent()->get_mouse_state(x, y, buttons);
		}


		//
		// ActionScript overrides
		//

		virtual void	set_member(const tu_stringi& name, const as_value& val)
		{
			// TODO: pull these up into a base class, to
			// share as much as possible with sprite_instance.
			as_standard_member	std_member = get_standard_member(name);
			switch (std_member)
			{
			default:
			case M_INVALID_MEMBER:
				break;
			case M_VISIBLE:  // _visible
			{
				m_visible = val.to_bool();
				return;
			}
			case M_ALPHA:  // _alpha
			{
				// Set alpha modulate, in percent.
				cxform	cx = get_cxform();
				cx.m_[3][0] = float(val.to_number()) / 100.f;
				set_cxform(cx);
				//m_accept_anim_moves = false;
				return;
			}
			case M_X:  // _x
			{
				matrix	m = get_matrix();	// @@ get_world_matrix()???
				m.m_[0][2] = float(PIXELS_TO_TWIPS(val.to_number()));
				this->set_matrix(m);
				return;
			}
			case M_Y:  // _y
			{
				matrix	m = get_matrix();	// @@ get_world_matrix()???
				m.m_[1][2] = float(PIXELS_TO_TWIPS(val.to_number()));
				this->set_matrix(m);
				return;
			}
// evan : need set_width and set_height function for struct character
#if 0
			case M_WIDTH:  // _width
			{
				for (int i = 0; i < m_def->m_button_records.size(); i++)
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
						m_record_character[i]->set_width(val.to_number);
						// @@ evan: should we return here?
						return;
					}
				}

				return;
			}
			else if (name == "enabled")
			{
				m_enabled = val.to_bool();
			}
			case M_HEIGHT:  // _height
			{
				for (int i = 0; i < m_def->m_button_records.size(); i++)
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
						m_record_character[i]->set_height(val.to_number);
						// @@ evan: should we return here?
						return;
					}
				}

				return;
			}
#endif
			}

			log_error("error: button_character_instance::set_member('%s', '%s') not implemented yet\n",
					  name.c_str(),
					  val.to_string());
		}

		virtual bool	get_member(const tu_stringi& name, as_value* val)
		{
			// TODO: pull these up into a base class, to
			// share as much as possible with sprite_instance.
			as_standard_member	std_member = get_standard_member(name);
			switch (std_member)
			{
			default:
			case M_INVALID_MEMBER:
				break;
			case M_VISIBLE:  // _visible
			{
				val->set_bool(this->get_visible());
				return true;
			}
			case M_ALPHA:  // _alpha
			{
				// @@ TODO this should be generic to struct character!
				// Alpha units are in percent.
				val->set_double(get_cxform().m_[3][0] * 100.f);
				return true;
			}
			case M_X:  // _x
			{
				matrix	m = get_matrix();	// @@ get_world_matrix()???
				val->set_double(TWIPS_TO_PIXELS(m.m_[0][2]));
				return true;
			}
			case M_Y:  // _y
			{
				matrix	m = get_matrix();	// @@ get_world_matrix()???
				val->set_double(TWIPS_TO_PIXELS(m.m_[1][2]));
				return true;
			}
			case M_WIDTH:  // _width
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
						val->set_double(TWIPS_TO_PIXELS(m_record_character[i]->get_width()));
						// @@ evan: should we return here?
						return true;
					}
				}

				// from the experiments with macromedia flash player
				val->set_double(0);
				return true;
			}
			case M_HEIGHT:  // _height
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
						val->set_double(TWIPS_TO_PIXELS(m_record_character[i]->get_height()));
						// @@ evan: should we return here?
						return true;
					}
				}

				// from the experiments with macromedia flash player
				val->set_double(0);
				return true;
			}
			}

			return false;
		}

		// not sure if we need to override this one.
		//virtual const char*	get_text_value() const { return NULL; }	// edit_text_character overrides this
	};


	//
	// button_record
	//

	bool	button_record::read(stream* in, int tag_type, movie_definition* m)
	// Return true if we read a record; false if this is a null record.
	{
		int	flags = in->read_u8();
		if (flags == 0)
		{
			return false;
		}
		m_hit_test = flags & 8 ? true : false;
		m_down = flags & 4 ? true : false;
		m_over = flags & 2 ? true : false;
		m_up = flags & 1 ? true : false;

		m_character_id = in->read_u16();
		m_character_def = NULL;
		m_button_layer = in->read_u16(); 
		m_button_matrix.read(in);

		if (tag_type == 34)
		{
			m_button_cxform.read_rgba(in);
		}

		return true;
	}


	//
	// button_action
	//


	button_action::~button_action()
	{
		m_actions.clear();
	}

	void	button_action::read(stream* in, int tag_type)
	{
		// Read condition flags.
		if (tag_type == 7)
		{
			m_conditions = OVER_DOWN_TO_OVER_UP;
		}
		else
		{
			assert(tag_type == 34);
			m_conditions = in->read_u16();
		}

		// Read actions.
		IF_VERBOSE_ACTION(log_msg("-- actions in button\n")); // @@ need more info about which actions
		action_buffer*	a = new action_buffer;
		a->read(in);
		m_actions.push_back(a);
	}


	//
	// button_character_definition
	//

	button_character_definition::button_character_definition()
		:
		m_sound(NULL)
	// Constructor.
	{
	}

	button_character_definition::~button_character_definition()
	{
		delete m_sound;
	}


	void button_character_definition::sound_info::read(stream* in)
	{
		m_in_point = m_out_point = m_loop_count = 0;
		in->read_uint(2);	// skip reserved bits.
		m_stop_playback = in->read_uint(1) ? true : false;
		m_no_multiple = in->read_uint(1) ? true : false;
		m_has_envelope = in->read_uint(1) ? true : false;
		m_has_loops = in->read_uint(1) ? true : false;
		m_has_out_point = in->read_uint(1) ? true : false;
		m_has_in_point = in->read_uint(1) ? true : false;
		if (m_has_in_point) m_in_point = in->read_u32();
		if (m_has_out_point) m_out_point = in->read_u32();
		if (m_has_loops) m_loop_count = in->read_u16();
		if (m_has_envelope)
		{
			int nPoints = in->read_u8();
			m_envelopes.resize(nPoints);
			for (int i=0; i < nPoints; i++)
			{
				m_envelopes[i].m_mark44 = in->read_u32();
				m_envelopes[i].m_level0 = in->read_u16();
				m_envelopes[i].m_level1 = in->read_u16();
			}
		}
		else
		{
			m_envelopes.resize(0);
		}
		IF_VERBOSE_PARSE(
			log_msg("	has_envelope = %d\n", m_has_envelope);
			log_msg("	has_loops = %d\n", m_has_loops);
			log_msg("	has_out_point = %d\n", m_has_out_point);
			log_msg("	has_in_point = %d\n", m_has_in_point);
			log_msg("	in_point = %d\n", m_in_point);
			log_msg("	out_point = %d\n", m_out_point);

			log_msg("	loop_count = %d\n", m_loop_count);
			log_msg("	envelope size = %zd\n", m_envelopes.size());
		);
	}



	void	button_character_definition::read(stream* in, int tag_type, movie_definition* m)
	// Initialize from the given stream.
	{
		assert(tag_type == 7 || tag_type == 17 || tag_type == 34);

		if (tag_type == 7)
		{
			// Old button tag.
				
			// Read button character records.
			for (;;)
			{
				button_record	r;
				if (r.read(in, tag_type, m) == false)
				{
					// Null record; marks the end of button records.
					break;
				}
				m_button_records.push_back(r);
			}

			// Read actions.
			m_button_actions.resize(m_button_actions.size() + 1);
			m_button_actions.back().read(in, tag_type);
		}
		else if (tag_type == 17)
		{
			assert(m_sound == NULL);	// redefinition button sound is error
			m_sound = new button_sound_def();
			IF_VERBOSE_PARSE(log_msg("button sound options:\n"));
			for (int i = 0; i < 4; i++)
			{
				button_sound_info& bs = m_sound->m_button_sounds[i];
				bs.m_sound_id = in->read_u16();
				if (bs.m_sound_id > 0)
				{
					bs.m_sam = (sound_sample_impl*) m->get_sound_sample(bs.m_sound_id);
					if (bs.m_sam == NULL)
					{
//						printf("sound tag not found, sound_id=%d, button state #=%i", bs.sound_id, i);
					}
					IF_VERBOSE_PARSE(log_msg("\n	sound_id = %d\n", bs.m_sound_id));
					bs.m_sound_style.read(in);
				}
			}
		}
		else if (tag_type == 34)
		{
			// Read the menu flag.
			m_menu = in->read_u8() != 0;

			int	button_2_action_offset = in->read_u16();
			int	next_action_pos = in->get_position() + button_2_action_offset - 2;

			// Read button records.
			for (;;)
			{
				button_record	r;
				if (r.read(in, tag_type, m) == false)
				{
					// Null record; marks the end of button records.
					break;
				}
				m_button_records.push_back(r);
			}

			if (button_2_action_offset > 0)
			{
				in->set_position(next_action_pos);

				// Read Button2ActionConditions
				for (;;)
				{
					int	next_action_offset = in->read_u16();
					next_action_pos = in->get_position() + next_action_offset - 2;

					m_button_actions.resize(m_button_actions.size() + 1);
					m_button_actions.back().read(in, tag_type);

					if (next_action_offset == 0
					    || in->get_position() >= in->get_tag_end_position())
					{
						// done.
						break;
					}

					// seek to next action.
					in->set_position(next_action_pos);
				}
			}
		}
	}


	character*	button_character_definition::create_character_instance(movie* parent, int id)
	// Create a mutable instance of our definition.
	{
		character*	ch = new button_character_instance(this, parent, id);
		return ch;
	}
};


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
