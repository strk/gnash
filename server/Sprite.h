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
// 
// Linking Gnash statically or dynamically with other modules is making
// a combined work based on Gnash. Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
// 
// In addition, as a special exception, the copyright holders of Gnash give
// you permission to combine Gnash with free software programs or
// libraries that are released under the GNU LGPL and/or with Mozilla, 
// so long as the linking with Mozilla, or any variant of Mozilla, is
// through its standard plug-in interface. You may copy and distribute
// such a system following the terms of the GNU GPL for Gnash and the
// licenses of the other code concerned, provided that you include the
// source code of that other code when and as the GNU GPL requires
// distribution of source code. 
// 
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is
// their choice whether to do so.  The GNU General Public License gives
// permission to release a modified version without this exception; this
// exception also makes it possible to release a modified version which
// carries forward this exception.
//

// Implementation for ActionScript MovieClip object.

/// \page Sprite Sprites and MovieClips
///
/// A Sprite, or MovieClip, is a mini movie-within-a-movie. 
///
/// It doesn't define its own characters;
/// it uses the characters from the parent
/// movie, but it has its own frame counter, display list, etc.
///
/// @@ are we sure it doesn't define its own chars ?
///
/// The sprite implementation is divided into 
/// gnash::sprite_definition and gnash::sprite_instance.
///
/// The _definition holds the immutable data for a sprite (as read
/// from an SWF stream), while the _instance contains the state for
/// a specific run of if (frame being played, mouse state, timers,
/// display list as updated by actions, ...)
///

#ifndef GNASH_SPRITE_H
#define GNASH_SPRITE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vector>

#include "Movie.h"
#include "dlist.h" // display_list 
#include "stream.h"
#include "log.h"

namespace gnash
{
	// Forward declarations
	struct sprite_instance;
	struct sprite_definition;


	//
	// sprite_definition
	//
	struct sprite_definition : public movie_definition
	{
		movie_definition* m_movie_def;	// parent movie.
		std::vector<std::vector<execute_tag*> > m_playlist;	// movie control events for each frame.
		stringi_hash<int> m_named_frames;	// stores 0-based frame #'s
		int m_frame_count;
		int m_loading_frame;

		sprite_definition(movie_definition* m) :
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
		    m_named_frames[n] = m_loading_frame;	// stores 0-based frame #
		}

	    /* sprite_definition */
	    bool	get_labeled_frame(const char* label, int* frame_number)
		// Returns 0-based frame #
		{
		    return m_named_frames.get(label, frame_number);
		}

	    const std::vector<execute_tag*>&	get_playlist(int frame_number)
		// frame_number is 0-based
		{
		    return m_playlist[frame_number];
		}

		// Sprites do not have init actions in their
		// playlists!  Only the root movie
		// (movie_def_impl) does (@@ correct?)
		virtual const std::vector<execute_tag*>*get_init_actions(int frame_number)
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


	/// Stateful Sprite object. Also known as a MovieClip.
	struct sprite_instance : public character
	{
		smart_ptr<movie_definition>	m_def;
		movie_root*	m_root;

		display_list	m_display_list;

		//std::vector<action_buffer*>	m_action_list;
		std::vector<action_buffer*>	m_action_list;

		play_state	m_play_state;
		int		m_current_frame;
		float		m_time_remainder;
		bool		m_update_frame;
		bool		m_has_looped;

		// once we've been moved by ActionScript,
		// don't accept moves from anim tags.
		bool	        m_accept_anim_moves;

		// a bit-array class would be ideal for this
		std::vector<bool>	m_init_actions_executed;

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

		sprite_instance(movie_definition* def,
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
			for (std::vector<bool>::iterator p = m_init_actions_executed.begin(); p != m_init_actions_executed.end(); ++p)
			    {
			        *p = false;
			    }
		}

                // sprite instance of add_interval_handler()
		virtual int    add_interval_timer(void *timer)
		{
		    return m_root->add_interval_timer(timer);
		}

		virtual void  clear_interval_timer(int x)
		{
		    m_root->clear_interval_timer(x);
		}
		

		/// Interval timer timeout executor
		virtual void    do_something(void *timer);

		virtual ~sprite_instance()
		{
		    m_display_list.clear();
		    //m_root->drop_ref();
		}

		movie_interface* get_root_interface() {
			return m_root;
		}

		movie_root* get_root() {
			return m_root;
		}

		movie*	get_root_movie() {
			return m_root->get_root_movie();
		}

		movie_definition* get_movie_definition() {
			return m_def.get_ptr();
		}

	    	float get_width();

		float	get_height();

		int get_current_frame() const
		{
			return m_current_frame;
		}

		int get_frame_count() const
		{
			return m_def->get_frame_count();
		}

		/// Stop or play the sprite.
		void set_play_state(play_state s)
		{
		    if (m_play_state != s) m_time_remainder = 0;
		    m_play_state = s;
		}

		play_state get_play_state() const { return m_play_state; }

		character* get_character(int character_id)
		{
			//return m_def->get_character_def(character_id);
			// @@ TODO -- look through our dlist for a match
			log_msg("FIXME: %s doesn't even check for a char",
				__PRETTY_FUNCTION__);
			return NULL;
		}

		float	get_background_alpha() const
		{
		    // @@ this doesn't seem right...
		    return m_root->get_background_alpha();
		}

		float	get_pixel_scale() const
		{
			return m_root->get_pixel_scale();
		}

		virtual void	get_mouse_state(int* x, int* y, int* buttons)
		{
		    m_root->get_mouse_state(x, y, buttons);
		}

		void	set_background_color(const rgba& color)
		{
		    m_root->set_background_color(color);
		}

		float	get_timer() const { return m_root->get_timer(); }

		void	restart();


		virtual bool has_looped() const
		{
			return m_has_looped;
		}

		virtual bool get_accept_anim_moves() const
		{
			return m_accept_anim_moves;
		}

		/// Combine the flags to avoid a conditional.
		/// It would be faster with a macro.
		inline int transition(int a, int b) const
		{
		    return (a << 2) | b;
		}


		/// Return true if we have any mouse event handlers.
		bool can_handle_mouse_event();

		/// Return the topmost entity that the given point
		/// covers that can receive mouse events.  NULL if
		/// none.  Coords are in parent's frame.
	    	virtual movie*	get_topmost_mouse_entity(float x, float y);

		/// Increment m_current_frame, and take care of looping.
		void	increment_frame_and_check_for_loop();

		virtual void	advance(float delta_time);

		/// Execute the tags associated with the specified frame.
		/// frame is 0-based
	    	void execute_frame_tags(int frame, bool state_only = false);


		/// Execute the tags associated with the specified frame,
		/// IN REVERSE.
		/// I.e. if it's an "add" tag, then we do a "remove" instead.
		/// Only relevant to the display-list manipulation tags:
		/// add, move, remove, replace.
		///
		/// frame is 0-based
	    	void execute_frame_tags_reverse(int frame);

			
	    	execute_tag* find_previous_replace_or_add_tag(int frame,
			int depth, int id);


		/// Execute any remove-object tags associated with
		/// the specified frame.
		/// frame is 0-based
	    	void	execute_remove_tags(int frame);


		/// Take care of this frame's actions.
		void do_actions();


		/// Set the sprite state at the specified frame number.
		//
		/// 0-based frame numbers!! 
		///(in contrast to ActionScript and Flash MX)
		///
		void	goto_frame(int target_frame_number);


		/// Look up the labeled frame, and jump to it.
		bool goto_labeled_frame(const char* label);

			
		/// Display (render?) this Sprite/MovieClip, unless invisible
		void	display();

		/// Add an object to the DisplayList.
		character*	add_display_object(
			Uint16 character_id,
			const char* name,
			const std::vector<swf_event*>& event_handlers,
			Uint16 depth,
			bool replace_if_depth_is_occupied,
			const cxform& color_transform,
			const matrix& matrix,
			float ratio,
			Uint16 clip_depth);


		/// Updates the transform properties of the object at
		/// the specified depth.
		void	move_display_object(
				Uint16 depth,
				bool use_cxform,
				const cxform& color_xform,
				bool use_matrix,
				const matrix& mat,
				float ratio,
				Uint16 clip_depth)
		{
		    m_display_list.move_display_object(depth, use_cxform, color_xform, use_matrix, mat, ratio, clip_depth);
		}


		void	replace_display_object(
				Uint16 character_id,
				const char* name,
				Uint16 depth,
				bool use_cxform,
				const cxform& color_transform,
				bool use_matrix,
				const matrix& mat,
				float ratio,
				Uint16 clip_depth);


		void	replace_display_object(
				character* ch,
				const char* name,
				Uint16 depth,
				bool use_cxform,
				const cxform& color_transform,
				bool use_matrix,
				const matrix& mat,
				float ratio,
				Uint16 clip_depth);


		/// Remove the object at the specified depth.
		/// If id != -1, then only remove the object
		/// at depth with matching id.
		void	remove_display_object(Uint16 depth, int id)
		{
		    m_display_list.remove_display_object(depth, id);
		}


		/// Add the given action buffer to the list of action
		/// buffers to be processed at the end of the next
		/// frame advance.
		void	add_action_buffer(action_buffer* a)
		{
		    m_action_list.push_back(a);
		}


		/// For debugging -- return the id of the character
		/// at the specified depth.
		/// Return -1 if nobody's home.
		int	get_id_at_depth(int depth);


		//
		// ActionScript support
		//


		/// Set the named variable to the value
	    	virtual void set_variable(const char* path_to_var,
			const char* new_value);

		/// Set the named variable to the wide value
	    	virtual void set_variable(const char* path_to_var,
			const wchar_t* new_value);

		/// Returns address to static buffer. NOT THREAD SAFE!
		virtual const char* get_variable(const char* path_to_var) const;

		// Set *val to the value of the named member and
		// return true, if we have the named member.
		// Otherwise leave *val alone and return false.
		bool get_member(const tu_stringi& name, as_value* val);

			
		/// Set the named member to the value. 
		//
		/// Return true if we have
		/// that member; false otherwise.
	    	virtual void set_member(const tu_stringi& name,
			const as_value& val);


		/// Find the movie which is one degree removed from us,
		/// given the relative pathname.
		///
		/// If the pathname is "..", then return our parent.
		/// If the pathname is ".", then return ourself.  If
		/// the pathname is "_level0" or "_root", then return
		/// the root movie.
		///
		/// Otherwise, the name should refer to one our our
		/// named characters, so we return it.
		///
		/// NOTE: In ActionScript 2.0, top level names (like
		/// "_root" and "_level0") are CASE SENSITIVE.
		/// Character names in a display list are CASE
		/// SENSITIVE. Member names are CASE INSENSITIVE.  Gah.
		///
		/// In ActionScript 1.0, everything seems to be CASE
		/// INSENSITIVE.
		virtual movie*	get_relative_target(const tu_string& name);


		/// Execute the actions for the specified frame. 
		//
		/// The frame_spec could be an integer or a string.
		///
		virtual void call_frame_actions(const as_value& frame_spec);


		virtual void set_drag_state(const drag_state& st) {
		    m_root->m_drag_state = st;
		}

		virtual void stop_drag() {
		    assert(m_parent == NULL);	// we must be the root movie!!!
				
		    m_root->stop_drag();
		}

		/* sprite_instance */
		virtual void	get_drag_state(drag_state* st)
		{
		    *st = m_root->m_drag_state;
		}


		/// Duplicate the object with the specified name
		/// and add it with a new name  at a new depth.
		void clone_display_object(const tu_string& name,
			const tu_string& newname, Uint16 depth);

		/// Remove the object with the specified name.
		void remove_display_object(const tu_string& name);

		/// Dispatch event handler(s), if any.
		virtual bool	on_event(event_id id);


		/// Do the events that (appear to) happen as the movie
		/// loads.  frame1 tags and actions are executed (even
		/// before advance() is called).  Then the onLoad event
		/// is triggered.
		virtual void	on_event_load()
		{
		    execute_frame_tags(0);
		    do_actions();
		    on_event(event_id::LOAD);
		}

		/// Do the events that happen when there is XML data waiting
		/// on the XML socket connection.
		/// FIXME: unimplemented
		virtual void	on_event_xmlsocket_onxml()
		{
		    log_msg("FIXME: %s: unimplemented\n", __FUNCTION__);
		    on_event(event_id::SOCK_XML);
		}
			
		/// Do the events that (appear to) happen on a
		/// specified interval.
		virtual void	on_event_interval_timer()
		{
		    log_msg("FIXME: %s: unimplemented\n", __FUNCTION__);
		    on_event(event_id::TIMER);
		}

		/// Do the events that happen as a MovieClip (swf 7 only) loads.
		virtual void	on_event_load_progress()
		{
		    log_msg("FIXME: %s: unimplemented\n", __FUNCTION__);
		    on_event(event_id::LOAD_PROGRESS);
		}

		/// Call a method with a list of arguments
		virtual const char* call_method_args(const char* method_name,
			const char* method_arg_fmt, va_list args)
		{
		    // Keep m_as_environment alive during any method calls!
		    smart_ptr<as_object>	this_ptr(this);

		    return call_method_parsed(&m_as_environment, this,
				method_name, method_arg_fmt, args);
		}

		virtual void	attach_display_callback(
			const char* path_to_object,
			void (*callback)(void*), void* user_ptr)
		{
//                  GNASH_REPORT_FUNCTION;
                  
			// should only be called on the root movie.
			assert(m_parent == NULL);

			std::vector<with_stack_entry>	dummy;
			as_value obj = m_as_environment.get_variable(tu_string(path_to_object), dummy);
			as_object*	as_obj = obj.to_object();
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
