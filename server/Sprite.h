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

// Implementation for ActionScript MovieClip object.

// A sprite, or MovieClip, is a mini movie-within-a-movie. 
// It doesn't define its own characters;
// it uses the characters from the parent
// movie, but it has its own frame counter, display list, etc.
//
// @@ are we sure it doesn't define its own chars ?
//
// The sprite implementation is divided into a
// sprite_definition and a sprite_instance.  The _definition
// holds the immutable data for a sprite, while the _instance
// contains the state for a specific instance being updated
// and displayed in the parent movie's display list.

#ifndef GNASH_SPRITE_H
#define GNASH_SPRITE_H

#include "Movie.h"
#include "dlist.h" // display_list 
#include "stream.h"

namespace gnash
{
	// Forward declarations
	struct sprite_instance;
	struct sprite_definition;


	//
	// sprite_definition
	//
	struct sprite_definition : public movie_definition_sub
	{
		movie_definition_sub* m_movie_def;	// parent movie.
		array<array<execute_tag*> > m_playlist;	// movie control events for each frame.
		stringi_hash<int> m_named_frames;	// stores 0-based frame #'s
		int m_frame_count;
		int m_loading_frame;

		sprite_definition(movie_definition_sub* m) :
			m_movie_def(m),
			m_frame_count(0),
			m_loading_frame(0)
		{
			assert(m_movie_def);
		}

		~sprite_definition()
		{
		    // Release our playlist data.
		    {for (int i = 0, n = m_playlist.size(); i < n; i++)
			{
			    for (int j = 0, m = m_playlist[i].size(); j < m; j++)
				{
				    delete m_playlist[i][j];
				}
			}}
		}

	    // overloads from movie_definition
	    virtual float	get_width_pixels() const { return 1; }
	    virtual float	get_height_pixels() const { return 1; }
	    virtual int	get_frame_count() const { return m_frame_count; }
	    virtual float	get_frame_rate() const { return m_movie_def->get_frame_rate(); }
	    virtual int	get_loading_frame() const { return m_loading_frame; }
	    virtual int	get_version() const { return m_movie_def->get_version(); }
	    virtual void	add_character(int id, character_def* ch) { log_error("add_character tag appears in sprite tags!\n"); }
	    virtual void	add_font(int id, font* ch) { log_error("add_font tag appears in sprite tags!\n"); }
	    virtual font*	get_font(int id) { return m_movie_def->get_font(id); }
	    virtual void	set_jpeg_loader(jpeg::input* j_in) { assert(0); }
	    virtual jpeg::input*	get_jpeg_loader() { return NULL; }
	    virtual bitmap_character_def*	get_bitmap_character(int id) { return m_movie_def->get_bitmap_character(id); }
	    virtual void	add_bitmap_character(int id, bitmap_character_def* ch) { log_error("add_bc appears in sprite tags!\n"); }
	    virtual sound_sample*	get_sound_sample(int id) { return m_movie_def->get_sound_sample(id); }
	    virtual void	add_sound_sample(int id, sound_sample* sam) { log_error("add sam appears in sprite tags!\n"); }

	    // @@ would be nicer to not inherit these...
	    virtual create_bitmaps_flag	get_create_bitmaps() const { assert(0); return DO_LOAD_BITMAPS; }
	    virtual create_font_shapes_flag	get_create_font_shapes() const { assert(0); return DO_LOAD_FONT_SHAPES; }
	    virtual int	get_bitmap_info_count() const { assert(0); return 0; }
	    virtual bitmap_info*	get_bitmap_info(int i) const { assert(0); return NULL; }
	    virtual void	add_bitmap_info(bitmap_info* bi) { assert(0); }

	    virtual void	export_resource(const tu_string& symbol, resource* res) { log_error("can't export from sprite\n"); }
	    virtual smart_ptr<resource>	get_exported_resource(const tu_string& sym) { return m_movie_def->get_exported_resource(sym); }
	    virtual void	add_import(const char* source_url, int id, const char* symbol) { assert(0); }
	    virtual void	visit_imported_movies(import_visitor* v) { assert(0); }
	    virtual void	resolve_import(const char* source_url, movie_definition* d) { assert(0); }
	    virtual character_def*	get_character_def(int id)
		{
		    return m_movie_def->get_character_def(id);
		}

		virtual void	generate_font_bitmaps() { assert(0); }

		virtual void output_cached_data(tu_file* out,
			const cache_options& options)
		{
		    // Nothing to do.
		    return;
		}

		virtual void	input_cached_data(tu_file* in)
		{
		    // Nothing to do.
		    return;
		}

	    	virtual movie_interface* create_instance()
		{
		    return NULL;
		}

		// Create a (mutable) instance of our definition.  The
		// instance is created to live (temporarily) on some level on
		// the parent movie's display list.
		//
		// overloads from character_def
		inline virtual character* create_character_instance(
			movie* parent, int id);


		/* sprite_definition */
		virtual void	add_execute_tag(execute_tag* c)
		{
		    m_playlist[m_loading_frame].push_back(c);
		}

	    /* sprite_definition */
	    virtual void	add_init_action(int sprite_id, execute_tag* c)
		{
		    // Sprite def's should not have do_init_action tags in them!  (@@ correct?)
		    log_error("sprite_definition::add_init_action called!  Ignored.\n");
		}

	    /* sprite_definition */
	    virtual void	add_frame_name(const char* name)
		// Labels the frame currently being loaded with the
		// given name.  A copy of the name string is made and
		// kept in this object.
		{
		    assert(m_loading_frame >= 0 && m_loading_frame < m_frame_count);

		    tu_string	n = name;
		    int	currently_assigned = 0;
		    if (m_named_frames.get(n, &currently_assigned) == true)
			{
			    log_error("add_frame_name(%d, '%s') -- frame name already assigned to frame %d; overriding\n",
				      m_loading_frame,
				      name,
				      currently_assigned);
			}
		    m_named_frames.set(n, m_loading_frame);	// stores 0-based frame #
		}

	    /* sprite_definition */
	    bool	get_labeled_frame(const char* label, int* frame_number)
		// Returns 0-based frame #
		{
		    return m_named_frames.get(label, frame_number);
		}

	    const array<execute_tag*>&	get_playlist(int frame_number)
		// frame_number is 0-based
		{
		    return m_playlist[frame_number];
		}

		// Sprites do not have init actions in their
		// playlists!  Only the root movie
		// (movie_def_impl) does (@@ correct?)
		virtual const array<execute_tag*>*get_init_actions(int frame_number)
		{
		    return NULL;
		}


		// Read the sprite info from input stream.
		// Consists of a series of tags.
		//
		// @@ Could be another constructor
		void read(stream* in)
		{
		    int	tag_end = in->get_tag_end_position();

		    m_frame_count = in->read_u16();
		    // ALEX: some SWF files have been seen that have 0-frame sprites.
		    // The Macromedia player behaves as if they have 1 frame.
		    if (m_frame_count < 1)
			{
			    m_frame_count = 1;
			}
		    m_playlist.resize(m_frame_count);	// need a playlist for each frame

		    IF_VERBOSE_PARSE(log_msg("  frames = %d\n", m_frame_count));

		    m_loading_frame = 0;

		    while ((Uint32) in->get_position() < (Uint32) tag_end)
			{
			    int	tag_type = in->open_tag();
			    loader_function lf = NULL;
			    if (tag_type == 1)
				{
				    // show frame tag -- advance to the next frame.
				    IF_VERBOSE_PARSE(log_msg("  show_frame (sprite)\n"));
				    m_loading_frame++;
				}
			    else if (s_tag_loaders.get(tag_type, &lf))
				{
				    // call the tag loader.  The tag loader should add
				    // characters or tags to the movie data structure.
				    (*lf)(in, tag_type, this);
				}
			    else
				{
				    // no tag loader for this tag type.
				    IF_VERBOSE_PARSE(log_msg("*** no tag loader for type %d\n", tag_type));
				}

			    in->close_tag();
			}

		    IF_VERBOSE_PARSE(log_msg("  -- sprite END --\n"));
		}
	};


	//
	// sprite_instance
	//


	struct sprite_instance : public character
	{
		smart_ptr<movie_definition_sub>	m_def;
		movie_root*	m_root;

		display_list	m_display_list;
		array<action_buffer*>	m_action_list;

		play_state	m_play_state;
		int		m_current_frame;
		float		m_time_remainder;
		bool		m_update_frame;
		bool		m_has_looped;

		// once we've been moved by ActionScript,
		// don't accept moves from anim tags.
		bool	        m_accept_anim_moves;

		// a bit-array class would be ideal for this
		array<bool>	m_init_actions_executed;

		as_environment	m_as_environment;

		// For built-in sprite ActionScript methods.
		static as_object as_builtins;
		static void init_builtins();


		enum mouse_state
		{
			UP = 0,
			DOWN,
			OVER
		};
		mouse_state m_mouse_state;

		sprite_instance(movie_definition_sub* def,
			movie_root* r, movie* parent, int id)
		:
		character(parent, id),
		m_def(def),
		m_root(r),
		m_play_state(PLAY),
		m_current_frame(0),
		m_time_remainder(0),
		m_update_frame(true),
		m_has_looped(false),
		m_accept_anim_moves(true),
		m_mouse_state(UP)
		{
			assert(m_def != NULL);
			assert(m_root != NULL);
					
			//m_root->add_ref();	// @@ circular!
			m_as_environment.set_target(this);

			init_builtins();

			// Initialize the flags for init action executed.
			m_init_actions_executed.resize(m_def->get_frame_count());
			memset(&m_init_actions_executed[0], 0,
			   sizeof(m_init_actions_executed[0]) * m_init_actions_executed.size());
		}

		// sprite instance of add_interval_handler()
		virtual int    add_interval_timer(void *timer)
		{
		    // log_msg("FIXME: %s:\n", __FUNCTION__);
		    return m_root->add_interval_timer(timer);
		}

	    virtual void    clear_interval_timer(int x)
		{
		    // log_msg("FIXME: %s:\n", __FUNCTION__);
		    m_root->clear_interval_timer(x);
		}
		

	    /* sprite_instance */
	    virtual void    do_something(void *timer)
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
					
		    } else if (as_as_function* as_func = timer_method.to_as_function()) {
			// It's an ActionScript function. Call it.
			as_value method;
			//log_msg("Calling ActionScript function for interval timer\n");
			(*as_func)(fn_call(&val, (as_object_interface *)this_ptr, as_env, 0, 0));
			//(*as_func)(&val, (as_object_interface *)this_ptr, &m_as_environment, 1, 1);
		    } else {
			log_error("error in call_method(): method is not a function\n");
		    }    
		}	

	    virtual ~sprite_instance()
		{
		    m_display_list.clear();
		    //m_root->drop_ref();
		}

	    movie_interface*	get_root_interface() { return m_root; }
	    movie_root*	get_root() { return m_root; }
	    movie*	get_root_movie() { return m_root->get_root_movie(); }

	    movie_definition*	get_movie_definition() { return m_def.get_ptr(); }

	    float	get_width()
		{
		    float	w = 0;
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
		}


	    float	get_height()
		{
		    float	h = 0; 
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
		}

	    int	get_current_frame() const { return m_current_frame; }
	    int	get_frame_count() const { return m_def->get_frame_count(); }

	    void	set_play_state(play_state s)
		// Stop or play the sprite.
		{
		    if (m_play_state != s)
			{
			    m_time_remainder = 0;
			}

		    m_play_state = s;
		}
	    play_state	get_play_state() const { return m_play_state; }


	    character*	get_character(int character_id)
		{
	//			return m_def->get_character_def(character_id);
		    // @@ TODO -- look through our dlist for a match
		    return NULL;
		}

	    float	get_background_alpha() const
		{
		    // @@ this doesn't seem right...
		    return m_root->get_background_alpha();
		}

	    float	get_pixel_scale() const { return m_root->get_pixel_scale(); }

	    virtual void	get_mouse_state(int* x, int* y, int* buttons)
		{
		    m_root->get_mouse_state(x, y, buttons);
		}

	    void	set_background_color(const rgba& color)
		{
		    m_root->set_background_color(color);
		}

	    float	get_timer() const { return m_root->get_timer(); }

	    void	restart()
		{
		    m_current_frame = 0;
		    m_time_remainder = 0;
		    m_update_frame = true;
		    m_has_looped = false;
		    m_play_state = PLAY;

		    execute_frame_tags(m_current_frame);
		    m_display_list.update();
		}


	    virtual bool	has_looped() const { return m_has_looped; }

	    virtual bool	get_accept_anim_moves() const { return m_accept_anim_moves; }

	    inline int	transition(int a, int b) const
		// Combine the flags to avoid a conditional. It would be faster with a macro.
		{
		    return (a << 2) | b;
		}


	    bool can_handle_mouse_event()
		// Return true if we have any mouse event handlers.
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
			

	    /* sprite_instance */
	    virtual movie*	get_topmost_mouse_entity(float x, float y)
		// Return the topmost entity that the given point
		// covers that can receive mouse events.  NULL if
		// none.  Coords are in parent's frame.
		{
		    if (get_visible() == false) {
			return NULL;
		    }

		    matrix	m = get_matrix();
		    point	p;
		    m.transform_by_inverse(&p, point(x, y));

		    int i, n = m_display_list.get_character_count();
		    // Go backwards, to check higher objects first.
		    for (i = n - 1; i >= 0; i--)
			{
			    character* ch = m_display_list.get_character(i);
					
			    if (ch != NULL && ch->get_visible())
				{
				    movie*	te = ch->get_topmost_mouse_entity(p.m_x, p.m_y);
				    if (te)
					{
					    // The containing entity that 1) is closest to root and 2) can
					    // handle mouse events takes precedence.
					    if (can_handle_mouse_event()) {
						return this;
					    } else {
						return te;
					    }
					}
				}
			}

		    return NULL;
		}


	    /* sprite_instance */
	    void	increment_frame_and_check_for_loop()
		// Increment m_current_frame, and take care of looping.
		{
		    m_current_frame++;

		    int	frame_count = m_def->get_frame_count();
		    if (m_current_frame >= frame_count)
			{
			    // Loop.
			    m_current_frame = 0;
			    m_has_looped = true;
			    if (frame_count > 1)
				{
				    m_display_list.reset();
				}
			}
		}

	    /* sprite_instance */
	    virtual void	advance(float delta_time)
		{
	//	    printf("%s:\n", __PRETTY_FUNCTION__); // FIXME:

		    // Keep this (particularly m_as_environment) alive during execution!
		    smart_ptr<as_object_interface>	this_ptr(this);

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
				    int	current_frame0 = m_current_frame;
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

			    // Clean up display list (remove dead objects).
			    m_display_list.update();
			}

		    // Skip excess time.  TODO root caller should
		    // loop to prevent this happening; probably
		    // only root should keep m_time_remainder, and
		    // advance(dt) should be a discrete tick()
		    // with no dt.
		    m_time_remainder = fmod(m_time_remainder, frame_time);
		}

	    /*sprite_instance*/
	    void	execute_frame_tags(int frame, bool state_only = false)
		// Execute the tags associated with the specified frame.
		// frame is 0-based
		{
		    // Keep this (particularly m_as_environment) alive during execution!
		    smart_ptr<as_object_interface>	this_ptr(this);

		    assert(frame >= 0);
		    assert(frame < m_def->get_frame_count());

		    // Execute this frame's init actions, if necessary.
		    if (m_init_actions_executed[frame] == false)
			{
			    const array<execute_tag*>*	init_actions = m_def->get_init_actions(frame);
			    if (init_actions && init_actions->size() > 0)
				{
				    // Need to execute these actions.
				    for (int i= 0; i < init_actions->size(); i++)
					{
					    execute_tag*	e = (*init_actions)[i];
					    e->execute(this);
					}

				    // Mark this frame done, so we never execute these init actions
				    // again.
				    m_init_actions_executed[frame] = true;
				}
			}

		    const array<execute_tag*>&	playlist = m_def->get_playlist(frame);
		    for (int i = 0; i < playlist.size(); i++)
			{
			    execute_tag*	e = playlist[i];
			    if (state_only)
				{
				    e->execute_state(this);
				}
			    else
				{
				    e->execute(this);
				}
			}
		}


	    /*sprite_instance*/
	    void	execute_frame_tags_reverse(int frame)
		// Execute the tags associated with the specified frame, IN REVERSE.
		// I.e. if it's an "add" tag, then we do a "remove" instead.
		// Only relevant to the display-list manipulation tags: add, move, remove, replace.
		//
		// frame is 0-based
		{
		    // Keep this (particularly m_as_environment) alive during execution!
		    smart_ptr<as_object_interface>	this_ptr(this);

		    assert(frame >= 0);
		    assert(frame < m_def->get_frame_count());

		    const array<execute_tag*>&	playlist = m_def->get_playlist(frame);
		    for (int i = 0; i < playlist.size(); i++)
			{
			    execute_tag*	e = playlist[i];
			    e->execute_state_reverse(this, frame);
			}
		}

			
	    /*sprite_instance*/
	    execute_tag*	find_previous_replace_or_add_tag(int frame, int depth, int id)
		{
		    uint32	depth_id = ((depth & 0x0FFFF) << 16) | (id & 0x0FFFF);

		    for (int f = frame - 1; f >= 0; f--)
			{
			    const array<execute_tag*>&	playlist = m_def->get_playlist(f);
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


	    /*sprite_instance*/
	    void	execute_remove_tags(int frame)
		// Execute any remove-object tags associated with the specified frame.
		// frame is 0-based
		{
		    assert(frame >= 0);
		    assert(frame < m_def->get_frame_count());

		    const array<execute_tag*>&	playlist = m_def->get_playlist(frame);
		    for (int i = 0; i < playlist.size(); i++)
			{
			    execute_tag*	e = playlist[i];
			    if (e->is_remove_tag())
				{
				    e->execute_state(this);
				}
			}
		}


		// Take care of this frame's actions.
		void do_actions();


	    /*sprite_instance*/
	    void	goto_frame(int target_frame_number)
		// Set the sprite state at the specified frame number.
		// 0-based frame numbers!!  (in contrast to ActionScript and Flash MX)
		{
	//			IF_VERBOSE_DEBUG(log_msg("sprite::goto_frame(%d)\n", target_frame_number));//xxxxx

		    target_frame_number = iclamp(target_frame_number, 0, m_def->get_frame_count() - 1);

		    if (target_frame_number < m_current_frame)
			{
			    for (int f = m_current_frame; f > target_frame_number; f--)
				{
				    execute_frame_tags_reverse(f);
				}

			    execute_frame_tags(target_frame_number, false);
			    m_display_list.update();
			}
		    else if (target_frame_number > m_current_frame)
			{
			    for (int f = m_current_frame + 1; f < target_frame_number; f++)
				{
				    execute_frame_tags(f, true);
				}

			    execute_frame_tags(target_frame_number, false);
			    m_display_list.update();
			}

		    m_current_frame = target_frame_number;      

		    // goto_frame stops by default.
		    m_play_state = STOP;
		}


	    bool	goto_labeled_frame(const char* label)
		// Look up the labeled frame, and jump to it.
		{
		    int	target_frame = -1;
		    if (m_def->get_labeled_frame(label, &target_frame))
			{
			    goto_frame(target_frame);
			    return true;
			}
		    else
			{
			    IF_VERBOSE_ACTION(
				log_error("error: movie_impl::goto_labeled_frame('%s') unknown label\n", label));
			    return false;
			}
		}

			
	    /*sprite_instance*/
	    void	display()
		{
		    if (get_visible() == false)
			{
			    // We're invisible, so don't display!
			    return;
			}

		    m_display_list.display();

		    do_display_callback();
		}

	    /*sprite_instance*/
	    character*	add_display_object(
		Uint16 character_id,
		const char* name,
		const array<swf_event*>& event_handlers,
		Uint16 depth,
		bool replace_if_depth_is_occupied,
		const cxform& color_transform,
		const matrix& matrix,
		float ratio,
		Uint16 clip_depth)
		// Add an object to the display list.
		{
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
			    move_display_object(depth, true, color_transform, true, matrix, ratio, clip_depth);
			    return NULL;
			}
		    //printf("%s: character %s, id is %d, count is %d\n", __FUNCTION__, existing_char->get_name(), character_id,m_display_list.get_character_count()); // FIXME:

		    assert(cdef);
		    smart_ptr<character>	ch = cdef->create_character_instance(this, character_id);
		    assert(ch != NULL);
		    if (name != NULL && name[0] != 0)
			{
			    ch->set_name(name);
			}

		    // Attach event handlers (if any).
		    {for (int i = 0, n = event_handlers.size(); i < n; i++)
			{
			    event_handlers[i]->attach_to(ch.get_ptr());
			}}

		    m_display_list.add_display_object(
			ch.get_ptr(),
			depth,
			replace_if_depth_is_occupied,
			color_transform,
			matrix,
			ratio,
			clip_depth);

		    assert(ch == NULL || ch->get_ref_count() > 1);
		    return ch.get_ptr();
		}


	    /*sprite_instance*/
	    void	move_display_object(
		Uint16 depth,
		bool use_cxform,
		const cxform& color_xform,
		bool use_matrix,
		const matrix& mat,
		float ratio,
		Uint16 clip_depth)
		// Updates the transform properties of the object at
		// the specified depth.
		{
		    m_display_list.move_display_object(depth, use_cxform, color_xform, use_matrix, mat, ratio, clip_depth);
		}


	    /*sprite_instance*/
	    void	replace_display_object(
		Uint16 character_id,
		const char* name,
		Uint16 depth,
		bool use_cxform,
		const cxform& color_transform,
		bool use_matrix,
		const matrix& mat,
		float ratio,
		Uint16 clip_depth)
		{
		    assert(m_def != NULL);
		    // printf("%s: character %s, id is %d\n", __FUNCTION__, name, character_id); // FIXME: debugging crap

		    character_def*	cdef = m_def->get_character_def(character_id);
		    if (cdef == NULL)
			{
			    log_error("sprite::replace_display_object(): unknown cid = %d\n", character_id);
			    return;
			}
		    assert(cdef);

		    smart_ptr<character>	ch = cdef->create_character_instance(this, character_id);
		    assert(ch != NULL);

		    if (name != NULL && name[0] != 0)
			{
			    ch->set_name(name);
			}

		    m_display_list.replace_display_object(
			ch.get_ptr(),
			depth,
			use_cxform,
			color_transform,
			use_matrix,
			mat,
			ratio,
			clip_depth);
		}


	    /*sprite_instance*/
	    void	replace_display_object(
		character* ch,
		const char* name,
		Uint16 depth,
		bool use_cxform,
		const cxform& color_transform,
		bool use_matrix,
		const matrix& mat,
		float ratio,
		Uint16 clip_depth)
		{
		    printf("%s: character %s, id is %d\n", __FUNCTION__, name, ch->get_id()); // FIXME:

		    assert(ch != NULL);

		    if (name != NULL && name[0] != 0)
			{
			    ch->set_name(name);
			}

		    m_display_list.replace_display_object(
			ch,
			depth,
			use_cxform,
			color_transform,
			use_matrix,
			mat,
			ratio,
			clip_depth);
		}


	    /*sprite_instance*/
	    void	remove_display_object(Uint16 depth, int id)
		// Remove the object at the specified depth.
		// If id != -1, then only remove the object at depth with matching id.
		{
		    m_display_list.remove_display_object(depth, id);
		}


	    /*sprite_instance*/
	    void	add_action_buffer(action_buffer* a)
		// Add the given action buffer to the list of action
		// buffers to be processed at the end of the next
		// frame advance.
		{
		    m_action_list.push_back(a);
		}


	    /*sprite_instance*/
	    int	get_id_at_depth(int depth)
		// For debugging -- return the id of the character at the specified depth.
		// Return -1 if nobody's home.
		{
		    int	index = m_display_list.get_display_index(depth);
		    if (index == -1)
			{
			    return -1;
			}

		    character*	ch = m_display_list.get_display_object(index).m_character.get_ptr();

		    return ch->get_id();
		}


	    //
	    // ActionScript support
	    //


	    /* sprite_instance */
	    virtual void	set_variable(const char* path_to_var, const char* new_value)
		{
		    assert(m_parent == NULL);	// should only be called on the root movie.

		    if (path_to_var == NULL)
			{
			    log_error("error: NULL path_to_var passed to set_variable()\n");
			    return;
			}
		    if (new_value == NULL)
			{
			    log_error("error: NULL passed to set_variable('%s', NULL)\n", path_to_var);
			    return;
			}

		    array<with_stack_entry>	empty_with_stack;
		    tu_string	path(path_to_var);
		    as_value	val(new_value);

		    m_as_environment.set_variable(path, val, empty_with_stack);
		}

	    /* sprite_instance */
	    virtual void	set_variable(const char* path_to_var, const wchar_t* new_value)
		{
		    if (path_to_var == NULL)
			{
			    log_error("error: NULL path_to_var passed to set_variable()\n");
			    return;
			}
		    if (new_value == NULL)
			{
			    log_error("error: NULL passed to set_variable('%s', NULL)\n", path_to_var);
			    return;
			}

		    assert(m_parent == NULL);	// should only be called on the root movie.

		    array<with_stack_entry>	empty_with_stack;
		    tu_string	path(path_to_var);
		    as_value	val(new_value);

		    m_as_environment.set_variable(path, val, empty_with_stack);
		}

	    /* sprite_instance */
	    virtual const char*	get_variable(const char* path_to_var) const
		{
		    assert(m_parent == NULL);	// should only be called on the root movie.

		    array<with_stack_entry>	empty_with_stack;
		    tu_string	path(path_to_var);

		    // NOTE: this is static so that the string
		    // value won't go away after we return!!!
		    // It'll go away during the next call to this
		    // function though!!!  NOT THREAD SAFE!
		    static as_value	val;

		    val = m_as_environment.get_variable(path, empty_with_stack);

		    return val.to_string();	// ack!
		}

		// Set *val to the value of the named member and
		// return true, if we have the named member.
		// Otherwise leave *val alone and return false.
		bool get_member(const tu_stringi& name, as_value* val);

			
	    /* sprite_instance */
	    virtual void	set_member(const tu_stringi& name, const as_value& val)
		// Set the named member to the value.  Return true if we have
		// that member; false otherwise.
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
			    m.m_[0][2] = (float) PIXELS_TO_TWIPS(val.to_number());
			    set_matrix(m);

			    m_accept_anim_moves = false;

			    return;
			}
			case M_Y:
			    //else if (name == "_y")
			{
			    matrix	m = get_matrix();
			    m.m_[1][2] = (float) PIXELS_TO_TWIPS(val.to_number());
			    set_matrix(m);

			    m_accept_anim_moves = false;

			    return;
			}
			case M_XSCALE:
			    //else if (name == "_xscale")
			{
			    matrix	m = get_matrix();

			    // Decompose matrix and insert the desired value.
			    float	x_scale = (float) val.to_number() / 100.f;	// input is in percent
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

			    // Decompose matrix and insert the desired value.
			    float	x_scale = m.get_x_scale();
			    float	y_scale = (float) val.to_number() / 100.f;	// input is in percent
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
			    cx.m_[3][0] = float(val.to_number()) / 100.f;
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
			    m.m_[0][0] = float(PIXELS_TO_TWIPS(val.to_number()));
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
			    m.m_[1][1] = float(PIXELS_TO_TWIPS(val.to_number()));
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
		    bool	text_val = val.get_type() == as_value::STRING
			|| val.get_type() == as_value::NUMBER;
		    if (text_val)
			{
			    bool	success = false;
			    for (int i = 0, n = m_display_list.get_character_count(); i < n; i++)
				{
				    character*	ch = m_display_list.get_character(i);
				    // CASE INSENSITIVE compare.  In ActionScript 2.0, this
				    // changes to CASE SENSITIVE!!!
				    if (name == ch->get_text_name())
					{
					    const char* text = val.to_string();
					    ch->set_text_value(text);
					    success = true;
					}
				}
			    if (success) return;
			}

		    // If that didn't work, set a variable within this environment.
		    m_as_environment.set_member(name, val);
		}


	    /* sprite_instance */
	    virtual movie*	get_relative_target(const tu_string& name)
		// Find the movie which is one degree removed from us,
		// given the relative pathname.
		//
		// If the pathname is "..", then return our parent.
		// If the pathname is ".", then return ourself.  If
		// the pathname is "_level0" or "_root", then return
		// the root movie.
		//
		// Otherwise, the name should refer to one our our
		// named characters, so we return it.
		//
		// NOTE: In ActionScript 2.0, top level names (like
		// "_root" and "_level0") are CASE SENSITIVE.
		// Character names in a display list are CASE
		// SENSITIVE. Member names are CASE INSENSITIVE.  Gah.
		//
		// In ActionScript 1.0, everything seems to be CASE
		// INSENSITIVE.
		{
		    if (name == "." || name == "this")
			{
			    return this;
			}
		    else if (name == "..")
			{
			    return get_parent();
			}
		    else if (name == "_level0"
			     || name == "_root")
			{
			    return m_root->m_movie.get_ptr();
			}

		    // See if we have a match on the display list.
		    return m_display_list.get_character_by_name(name);
		}


	    /* sprite_instance */
	    virtual void	call_frame_actions(const as_value& frame_spec)
		// Execute the actions for the specified frame.  The
		// frame_spec could be an integer or a string.
		{
		    int	frame_number = -1;

		    // Figure out what frame to call.
		    if (frame_spec.get_type() == as_value::STRING)
			{
			    if (m_def->get_labeled_frame(frame_spec.to_string(), &frame_number) == false)
				{
				    // Try converting to integer.
				    frame_number = (int) frame_spec.to_number();
				}
			}
		    else
			{
			    // convert from 1-based to 0-based
			    frame_number = (int) frame_spec.to_number() - 1;
			}

		    if (frame_number < 0 || frame_number >= m_def->get_frame_count())
			{
			    // No dice.
			    log_error("error: call_frame('%s') -- unknown frame\n", frame_spec.to_string());
			    return;
			}

		    int	top_action = m_action_list.size();

		    // Execute the actions.
		    const array<execute_tag*>&	playlist = m_def->get_playlist(frame_number);
		    for (int i = 0; i < playlist.size(); i++)
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
			    m_action_list.remove(top_action);
			}
		    assert(m_action_list.size() == top_action);
		}


	    /* sprite_instance */
	    virtual void	set_drag_state(const drag_state& st)
		{
		    m_root->m_drag_state = st;
		}

	    /* sprite_instance */
	    virtual void	stop_drag()
		{
		    assert(m_parent == NULL);	// we must be the root movie!!!
				
		    m_root->stop_drag();
		}


	    /* sprite_instance */
	    virtual void	get_drag_state(drag_state* st)
		{
		    *st = m_root->m_drag_state;
		}


	    void	clone_display_object(const tu_string& name, const tu_string& newname, Uint16 depth)
		// Duplicate the object with the specified name and add it with a new name 
		// at a new depth.
		{
		    character* ch = m_display_list.get_character_by_name(name);
		    if (ch)
			{
			    array<swf_event*>	dummy_event_handlers;

			    add_display_object(
				ch->get_id(),
				newname.c_str(),
				dummy_event_handlers,
				depth,
				true,	// replace if depth is occupied
				ch->get_cxform(),
				ch->get_matrix(),
				ch->get_ratio(),
				ch->get_clip_depth());
			    // @@ TODO need to duplicate ch's event handlers, and presumably other members?
			    // Probably should make a character::clone() function to handle this.
			}
		}


	    void	remove_display_object(const tu_string& name)
		// Remove the object with the specified name.
		{
		    character* ch = m_display_list.get_character_by_name(name);
		    if (ch)
			{
			    // @@ TODO: should only remove movies that were created via clone_display_object --
			    // apparently original movies, placed by anim events, are immune to this.
			    remove_display_object(ch->get_depth(), ch->get_id());
			}
		}

			
	    /* sprite_instance */
	    virtual bool	on_event(event_id id)
		// Dispatch event handler(s), if any.
		{
		    // Keep m_as_environment alive during any method calls!
		    smart_ptr<as_object_interface>	this_ptr(this);

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
			const tu_stringi&	method_name = id.get_function_name().to_tu_stringi();
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


	    /*sprite_instance*/
	    virtual void	on_event_load()
		// Do the events that (appear to) happen as the movie
		// loads.  frame1 tags and actions are executed (even
		// before advance() is called).  Then the onLoad event
		// is triggered.
		{
		    execute_frame_tags(0);
		    do_actions();
		    on_event(event_id::LOAD);
		}

	    // Do the events that happen when there is XML data waiting
	    // on the XML socket connection.
	    virtual void	on_event_xmlsocket_onxml()
		{
		    log_msg("FIXME: %s: unimplemented\n", __FUNCTION__);
		    on_event(event_id::SOCK_XML);
		}
			
	    // Do the events that (appear to) happen on a specified interval.
	    virtual void	on_event_interval_timer()
		{
		    log_msg("FIXME: %s: unimplemented\n", __FUNCTION__);
		    on_event(event_id::TIMER);
		}

	    // Do the events that happen as a MovieClip (swf 7 only) loads.
	    virtual void	on_event_load_progress()
		{
		    log_msg("FIXME: %s: unimplemented\n", __FUNCTION__);
		    on_event(event_id::LOAD_PROGRESS);
		}

	    /*sprite_instance*/
	    virtual const char*	call_method_args(const char* method_name, const char* method_arg_fmt, va_list args)
		{
		    // Keep m_as_environment alive during any method calls!
		    smart_ptr<as_object_interface>	this_ptr(this);

		    return call_method_parsed(&m_as_environment, this, method_name, method_arg_fmt, args);
		}

	    /* sprite_instance */
	    virtual void	attach_display_callback(const char* path_to_object, void (*callback)(void*), void* user_ptr)
		{
		    assert(m_parent == NULL);	// should only be called on the root movie.

		    array<with_stack_entry>	dummy;
		    as_value	obj = m_as_environment.get_variable(tu_string(path_to_object), dummy);
		    as_object_interface*	as_obj = obj.to_object();
		    if (as_obj)
			{
			    movie*	m = as_obj->to_movie();
			    if (m)
				{
				    m->set_display_callback(callback, user_ptr);
				}
			}
		}
	};


	// INLINES
	character* sprite_definition::create_character_instance(movie* parent,
		int id)
	{
	    sprite_instance* si = new sprite_instance(this,
		parent->get_root(), parent, id);
	    return si;
	}


} // end of namespace gnash

#endif // GNASH_SPRITE_H
