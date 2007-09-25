// sprite_instance.h:  Stateful live Sprite instance, for Gnash.
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
//


// Stateful live Sprite instance

#ifndef GNASH_SPRITE_INSTANCE_H
#define GNASH_SPRITE_INSTANCE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "edit_text_character.h" // temp hack
#include "movie_definition.h" // for inlines
#include "dlist.h" // DisplayList 
#include "log.h"
#include "as_environment.h" // for composition
#include "DynamicShape.h" // for composition
//#include "LoadVariablesThread.h" // for composition
#include "Range2d.h"

#include <vector>
#include <list>
#include <map>
#include <string>

// Forward declarations
namespace gnash {
	class movie_instance;
	class swf_event;
	class drag_state;
	class LoadVariablesThread;
}

namespace gnash
{

/// Stateful Sprite object. Also known as a MovieClip.
//
/// Instance of this class are also known as "timelines".
/// This means that they define a variable scope (see
/// the as_environment member) and are divided into "frames"
///
class sprite_instance : public character
{

public:

	typedef std::list<const action_buffer*> ActionList;

	// definition must match movie_definition::PlayList
	typedef std::vector<execute_tag*> PlayList;

	typedef std::vector<swf_event*> SWFEventsVector;


	/// @param root
	///	The "relative" _root of this sprite, which is the 
	///	instance of top-level sprite defined by the same
	///	SWF that also contained *this* sprite definition.
	///	Note that this can be *different* from the top-level
	///	movie accessible trought the VM, in case this sprite
	///	was defined in an externally loaded movie.
	///
	///
	sprite_instance(movie_definition* def,
		movie_instance* root, character* parent, int id);

	virtual ~sprite_instance();

	enum play_state
	{
		PLAY,
		STOP
	};

	/// Type of execute tags
	//
	/// TODO: move to execute_tag.h ?
	///
	enum control_tag_type
	{
		/// Action tag
		TAG_ACTION = 1<<0,

		/// DisplayList tag
		TAG_DLIST  = 1<<1
	};

	virtual void has_key_event() {
		m_has_key_event = true;
	}

	virtual void has_mouse_event();

	/// \brief
	/// Return this sprite's relative root as
	/// specified at contruction time
	///
	virtual sprite_instance* get_root_movie();

	/// \brief
	/// Return the sprite_definition (or movie_definition)
	/// from which this sprite_instance has been created
        movie_definition* get_movie_definition() {
                return m_def.get();
        }

	/// \brief
	/// Return version of the SWF definition of this instance
	/// as been parsed from.
	//
        int getSWFVersion()
	{
                return m_def->get_version();
        }

	/// Get the composite bounds of all component drawing elements
	geometry::Range2d<float> getBounds() const;

	// See dox in character.h
	bool pointInShape(float x, float y) const;

	// See dox in character.h
	bool pointInVisibleShape(float x, float y) const;

	/// Return 0-based index to current frame
	size_t get_current_frame() const
	{
		return m_current_frame;
	}

	size_t get_frame_count() const
	{
		return m_def->get_frame_count();
	}

	/// Return number of completely loaded frames of this sprite/movie
	//
	/// Note: the number is also the last frame accessible (frames
	/// numberes are 1-based)
	///
	size_t get_loaded_frames() const
	{
		return m_def->get_loading_frame();
	}

	/// Return total number of bytes in the movie
	/// (not sprite!)
	size_t get_bytes_total() const
	{
		return isDynamic() ? 0 : m_def->get_bytes_total();
	}

	/// Return number of loaded bytes in the movie
	/// (not sprite!)
	size_t get_bytes_loaded() const
	{
		return isDynamic() ? 0 : m_def->get_bytes_loaded();
	}

	const rect& get_frame_size() const
	{
		return m_def->get_frame_size();
	}

	/// Stop or play the sprite.
	void set_play_state(play_state s)
	{
	    //if (m_play_state != s) m_time_remainder = 0;
	    m_play_state = s;
	}

	play_state get_play_state() const { return m_play_state; }

	character* get_character(int character_id);

	// delegates to movie_root (possibly wrong)
	virtual float get_background_alpha() const;

	// delegates to movie_root 
	virtual float	get_pixel_scale() const;

	// delegates to movie_root 
	virtual void get_mouse_state(int& x, int& y, int& buttons);

	// delegates to movie_root (possibly wrong)
	void	set_background_color(const rgba& color);

	//float	get_timer() const;

	void	restart();


	bool has_looped() const
	{
		return m_has_looped;
	}

	/// Combine the flags to avoid a conditional.
	/// It would be faster with a macro.
	inline int transition(int a, int b) const
	{
	    return (a << 2) | b;
	}


	/// Return true if we have any mouse event handlers.
	//
	/// NOTE: this function currently does not consider
	///       general mouse event handlers MOUSE_MOVE, MOUSE
	virtual bool can_handle_mouse_event() const;

	/// \brief
	/// Return the topmost entity that the given point
	/// covers that can receive mouse events.  NULL if
	/// none.  Coords are in parent's frame.
	virtual character* get_topmost_mouse_entity(float x, float y);
	
	virtual bool wantsInstanceName()
	{
		return true; // sprites can be referenced 
	}

	virtual void	advance(float delta_time);
	//virtual void	advance_root(float delta_time);
	virtual void	advance_sprite(float delta_time);

	/// Set the sprite state at the specified frame number.
	//
	/// 0-based frame numbers!! 
	///(in contrast to ActionScript and Flash MX)
	///
	void	goto_frame(size_t target_frame_number);

	/// Parse frame spec and return a 0-based frame number.
	//
	/// If frame spec cannot be converted to !NAN and !Infinity number
	/// it will be converted to a string and considered a
	/// frame label (returns false if referring to an
	/// unknwown label).
	///
	/// @param frame_spec
	///	The frame specification.
	///
	/// @param frameno
	///	The evaluated frame number (0-based)
	///
	/// @return
	///	True if the frame_spec could be resolved to a frame number.
	///	False if the frame_spec was invalid.
	///
	bool get_frame_number(const as_value& frame_spec, size_t& frameno) const;


	/// Look up the labeled frame, and jump to it.
	bool goto_labeled_frame(const std::string& label);

		
	/// Display (render?) this Sprite/MovieClip, unless invisible
	void	display();

	/// Swap depth of the given characters in the DisplayList
	//
	/// See DisplayList::swapDepths for more info
	///
	void swapDepths(character* ch1, int newdepth)
	{
		m_display_list.swapDepths(ch1, newdepth);
	}

	/// Return the character at given depth in our DisplayList.
	//
	/// @return NULL if the specified depth is available (no chars there)
	///
	character* get_character_at_depth(int depth);

	character* add_empty_movieclip(const char* name, int depth);

	boost::intrusive_ptr<character> add_textfield(const std::string& name,
			int depth, float x, float y, float width, float height);

	/// Place a character or mask to the DisplayList (make a timeline instance) 
	//
	/// This method instantiates the given character definition (character_id)
	/// and places it on the stage at the given depth using the specified
	/// parameters for name, position, color, ratio(?) and clipping depth (if any).
	///
	/// If the specified depth is already occupied by an instance of the same
	/// character (character_id), the existing instance will be transformed using
	/// move_display_object() instead.
	///
	/// If the specified depth is already occupied by an instance of a different
	/// character (including any dynamically-created instance), the behaviour is
	/// controlled by the is_jumping_back flag. If false, this
	/// call will result in a no-op. If true, the previously existing character
	/// will be replaced by the new one, with unload() method invoked on the
	/// removed character. 
	///
	/// Any successful new placement triggers invokation of the newly created
	/// instance's LOAD event.
	///
	///
	/// @param character_id
	///	The ID of the character to be added.
	///	It will be seeked in the CharacterDictionary
	///
	/// @param name
	///	The name to give to the newly created instance.
	///	If NULL, the new instance will be assigned a sequential
	///	name in the form 'instanceN', where N is incremented
	///	at each call, starting from 1.
	///
	/// @param event_handlers
	///
	/// @param depth
	///	The depth to assign to the newly created instance.
	///
	/// @param color_transform
	///	The color transform to apply to the newly created instance.
	///
	/// @param matrix
	///	The matrix transform to apply to the newly created instance.
	///
	/// @param ratio
	///
	/// @param clip_depth
	///	If != character::noClipDepthValue, mark the created instance
	///	as a clipping layer. The shape of the placed character will be
	///	used as a mask for all higher depths up to this value.
	///	TODO: assert(clip_depth > depth) ?
	///
	/// @return 
	///	A pointer to the character being added or NULL
	///	if this call results in a move of an existing character 
	///	or in a no-op due to is_jumping_back being false. 
	///       
	character* add_display_object(
		uint16_t character_id,
		const char* name,
		const SWFEventsVector& event_handlers,
		int depth,
		const cxform& color_transform,
		const matrix& matrix,
		int ratio,
		int clip_depth);

	/// Attach the given character instance to current display list
	//
	/// @param newch
	///	The character instance to attach.
	///
	/// @param depth
	///	The depth to assign to the instance.
	///
	/// @return true on success, false on failure
	///	FIXME: currently never returns false !
	///
	bool attachCharacter(character& newch, int depth);

	/// Handle placement event
	//
	/// This callback will (not known to be a problem):
	///
	/// (1) Register ourselves with the global instance list
	/// (2) Take note of our original target path
	/// (3) Register as listener of core broadcasters
	/// (4) Execute tags of frame 0
	///
	/// The callback will also (known to be bogus):
	//
	/// (1) Construct this instance as an ActionScript object.
	///     See constructAsScriptObject() method.
	///
	virtual void stagePlacementCallback();

	/// Unload all contents in the displaylist and this instance
	/// See character::unload for more info
	bool unload();

	/// See DisplayList::move_display_object, this method is just a proxy to that...
	//
	/// @param color_xform
	///	The color tranform to assign to the new character.
	///	If NULL the default color transform will be kept.
	///
	/// @param mat
	///	The matrix tranform to assign to the new character.
	///	If NULL the default matrix will be kept.
	///
	void	move_display_object(
			int depth,
			const cxform* color_xform,
			const matrix* mat,
			int ratio,
			int clip_depth)
	{
	    m_display_list.move_display_object(depth, color_xform, mat, ratio, clip_depth);
	}


	///
	/// @param color_xform
	///	The color tranform to assign to the new character.
	///	If NULL the default color transform will be kept.
	///
	/// @param mat
	///	The matrix tranform to assign to the new character.
	///	If NULL the default matrix will be kept.
	///
	void	replace_display_object(
			uint16_t character_id,
			const char* name,
			int depth,
			const cxform* color_xform,
			const matrix* mat,
			int ratio,
			int clip_depth);


	///
	/// @param color_xform
	///	The color tranform to assign to the new character.
	///	If NULL the default color transform will be kept.
	//
	/// @param mat
	///	The matrix tranform to assign to the new character.
	///	If NULL the default matrix will be kept.
	///
	void	replace_display_object(
			character* ch,
			const char* name,
			int depth,
			const cxform* color_xform,
			const matrix* mat,
			int ratio,
			int clip_depth);


	/// \brief
	/// Remove the object at the specified depth.
	//
	/// NOTE: the id parameter is unused, but currently
	/// required to avoid break of inheritance from movie.h
	///
	void	remove_display_object(int depth, int /* id */)
	{
	    set_invalidated();
	    m_display_list.remove_display_object(depth);
	}


	/// Add the given action buffer to the list of action
	/// buffers to be processed at the end of the next
	/// frame advance.
	void	add_action_buffer(const action_buffer* a)
	{
		if ( ! _callingFrameActions ) queueAction(*a);
		else execute_action(*a);
	}

	/// Execute a single action buffer (DOACTION block)
	void execute_action(const action_buffer& ab);

	/// For debugging -- return the id of the character
	/// at the specified depth.
	/// Return -1 if nobody's home.
	int	get_id_at_depth(int depth);

	sprite_instance* to_movie () { return this; }

	/// Load a movie in this sprite, replacing it
	//
	/// Return: true if it succeeded, false otherwise
	///
	bool loadMovie(const URL& url);

	/// \brief
	/// Load url-encoded variables from the given url, optionally
	/// sending variables from this timeline too.
	//
	///
	/// A LoadVariablesThread will be started to load and parse variables
	/// and added to the _loadVariableRequests. Then, at every ::advance_sprite
	/// any completed threads will be processed
	/// (see processCompletedLoadVariableRequests)
	///
	/// NOTE: the given url will be securit-checked
	///
	/// @param url
	///	The url to load variables from. It is expected that
	///	the caller already checked host security.
	///
	/// @param sendVarsMethod
	///	If 0 (the default) no variables will be sent.
	///	If 1, GET will be used.
	///	If 2, POST will be used.
	///
	void loadVariables(const URL& url, short sendVarsMethod=0);

	//
	// ActionScript support
	//

	// See dox in as_object.h
	bool get_member(string_table::key name, as_value* val);
		
	/// Set the named member to the value. 
	//
	/// Return true if we have that member; false otherwise.
	///
	virtual void set_member(string_table::key name, const as_value& val);

	/// Overridden to look in DisplayList for a match
	virtual character* get_relative_target(const std::string& name);

	/// Execute the actions for the specified frame. 
	//
	/// The frame_spec could be an integer or a string.
	///
	virtual void call_frame_actions(const as_value& frame_spec);

	// delegates to movie_root 
	virtual void stop_drag();

	/// Duplicate this sprite in its timeline
	//
	/// Add the new character at a the given depth to this sprite
	/// parent displaylist.
	///
	/// NOTE: the call will fail for the root movie (no parent).
	/// NOTE2: any character at the given target depth will be
	///        replaced by the new character
	/// NOTE3: event handlers will also be copied
	///
	/// @param init_object
	///	If not null, will be used to copy properties over.
	///
	boost::intrusive_ptr<sprite_instance> duplicateMovieClip(
		const std::string& newname,
		int newdepth, as_object* init_object=NULL);
		
	/// Dispatch event handler(s), if any.
	virtual bool	on_event(const event_id& id);


	/// Do the events that (appear to) happen as the movie
	/// loads.  frame1 tags and actions are executed (even
	/// before advance() is called).  Then the onLoad event
	/// is triggered.

//	virtual void	on_event_load()
//	{
//	    execute_frame_tags(0);
//	    do_actions();
//	    on_event(event_id::LOAD);
//	}

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
		const char* method_arg_fmt, va_list args);

	// inherited from character class, see dox in character.h
	as_environment& get_environment() {
		return m_as_environment;
	}

	/// \brief
	/// Set a TextField variable to this timeline
	//
	/// A TextField variable is a variable that acts
	/// as a setter/getter for a TextField 'text' member.
	///
	void set_textfield_variable(const std::string& name,
			edit_text_character* ch);

	void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);
			

	const DisplayList& getDisplayList() const {
		return m_display_list;
	}

	/// Return the next highest available depth
	//
	/// Placing an object at the depth returned by
	/// this function should result in a character
	/// that is displayd above all others
	///
	int getNextHighestDepth() const {
		return m_display_list.getNextHighestDepth();
	}

	void testInvariant() const {
		assert(m_play_state == PLAY || m_play_state == STOP);

		// m_current_frame may be 0, since this is our initial
		// condition. Still, frame count might be 0 as well, and
		// loaded frames too !
		//assert(m_current_frame < m_def->get_frame_count());
#ifndef GNASH_USE_GC 
		assert(get_ref_count() > 0); // or we're constructed but
		                             // not stored in a boost::intrusive_ptr
#endif
	}

	/// Set the current m_sound_stream_id
	virtual void set_sound_stream_id(int id){ m_sound_stream_id = id; }

	/// Get the current m_sound_stream_id
	virtual int get_sound_stream_id() { return m_sound_stream_id;}

	/// Override for character::set_name to proprely update
	/// _target and _target_dot.
	virtual void set_name(const char* name);

	/// Remove this sprite from the stage.
	//
	/// This function is intended to be called by 
	/// effect of a removeMovieClip() ActionScript call
	/// and implements the checks required for this specific
	/// case.
	///
	/// Callers are:
	///	- The ActionRemoveClip tag handler.
	///	- The global removeMovieClip(target) function.
	///	- The MovieClip.removeMovieClip() method.
	///
	/// The removal will not occur if the depth of this
	/// characters is not in the "dynamic" range [0..1048575]
	/// as described at the following URL:
	/// 
	///	http://www.senocular.com/flash/tutorials/depths/?page=2
	///
	/// A testcases for this behaviour can be found in 
	///
	///	testsuite/misc-ming.all/displaylist_depths_test.swf
	///
	void removeMovieClip();

	/// @{ Drawing API
	
	void lineStyle(uint16_t thickness, const rgba& color)
	{
		_drawable->lineStyle(thickness, color);
	}

	void resetLineStyle()
	{
		_drawable->resetLineStyle();
	}

	void beginFill(const rgba& color)
	{
		_drawable->beginFill(color);
	}

	void endFill()
	{
		_drawable->endFill();
	}

	void moveTo(float x, float y)
	{
		_drawable->moveTo(x, y);
	}

	void lineTo(float x, float y)
	{
		set_invalidated();
		_drawable->lineTo(x, y);
	}

	void curveTo(float cx, float cy, float ax, float ay)
	{
		set_invalidated();
		_drawable->curveTo(cx, cy, ax, ay);
	}

	void clear()
	{
		set_invalidated();
		_drawable->clear();
	}

	/// @} Drawing API
	

	typedef std::map<std::string, std::string> VariableMap;

	/// Set all variables in the given map with their corresponding values
	DSOEXPORT void setVariables(VariableMap& vars);

	/// Enumerate child characters
	//
	/// See as_object::enumerateNonProperties(as_environment&) for more info.
	///
	virtual void enumerateNonProperties(as_environment&) const;

	/// Return original target path to this object, in dot notation
	/// as of at construction time.
	//
	/// This is needed to properly dereference dangling soft-references
	/// See testcase misc-swfc.all/soft_reference_test1.sc
	///
	const std::string& getOrigTarget() const
	{
		return _origTarget;
	}

	/// Delete characters removed from the stage
	/// from the display lists
	void cleanupDisplayList();

        /// Queue the given action buffer
        //
        /// The action will be pushed on the current
        /// global list (see movie_root).
        ///
        void queueAction(const action_buffer& buf);

private:

	/// Construct this instance as an ActionScript object
	//
	/// This method invokes the constructor associated with our
	/// definition, either MovieClip or any user-speficied one
	/// (see sprite_definition::registerClass). 
	/// It will also invoke the onClipConstruct and onConstruct handlers.
	///
	void constructAsScriptObject();

	/// Register this sprite as a listener of core broadcasters
	//
	/// This is currently only used for key events broadcaster
	/// and it's supposed to be called only once (or should we
	/// also call it whenever onKey* user-defined function is defined ?
	///
	void registerAsListener();

	/// \brief
	/// Return value of the 'enabled' property, casted to a boolean value.
	/// True if not found (undefined to bool evaluates to false).
	//
	/// When a MovieClip is "disabled", its handlers of button-like events 
	/// are disabled, and automatic tab ordering won't include it.
	///
	/// See event_id::is_button_event().
	///
	bool isEnabled() const;


	/// Forbid copy
	sprite_instance(const sprite_instance&)
		:
		character(NULL, 0)
	{
		assert(0);
	}

	/// Forbid assignment
	sprite_instance& operator=(const sprite_instance&) { assert(0); return *this; }

	/// \brief
	/// Call has_key_event() or has_mouse_event()
	/// if the given string correspond to an event handler
	/// for which registering as a listener of Mouse or Key is needed
	//
	///
	/// @param name
	///	Member name. 
	///
	void checkForKeyOrMouseEvent(const std::string& name);
#if 0
	/// Duplicate the object with the specified name
	/// and add it with a new name  at a new depth.
	void clone_display_object(const std::string& name,
		const std::string& newname, int depth);
#endif

	/// Advance to a previous frame.
	//
	/// This function will basically restore the DisplayList as it supposedly
	/// was *before* executing tags in target frame and then execute target
	/// frame tags (both DLIST and ACTION ones).
	///
	/// In practice, it will:
	///
	/// - Remove from current DisplayList:
	///	- Timeline instances constructed after target frame 
	///	- Timeline instances constructed before or at the target frame but no more at the original depth
	///	- Dynamic instances found in the static depth zone
	/// - Execute all displaylist tags from first to one-before target frame,
	///   appropriately setting m_current_frame as it goes, finally execute both displaylist and action
	///   tags for target frame.
	///
	/// Callers of this methods are:
	///	- goto_frame (for jump-backs)
	///	- advance_sprite (for loop-back)
	///
	/// See: http://www.gnashdev.org/wiki/index.php/TimelineControl#Timeline_instances
	///
	/// @param targetFrame
	///	The target frame for which we're willing to restore the static DisplayList.
	///	0-based.
	//
	/// POSTCONDITIONS:
	///
	///	- m_current_frame == targetFrame
	///
	/// TODO: consider using this same function for jump-forward too,
	///       with some modifications...
	///
	void restoreDisplayList(size_t targetFrame);

	/// Queue actions in the action list
	//
	/// The list of action will be pushed on the current
	/// global list (see movie_root).
	///
	void queueActions(ActionList& action_list);

	/// Execute the actions in the action list
	//
	/// The list of action will be consumed starting from the first
	/// element. When the function returns the list should be empty.
	///
	void execute_actions(ActionList& action_list);

	// TODO: shouldn't we keep this by intrusive_ptr ?
	movie_instance*	m_root;

	/// Current Display List contents.
	DisplayList	m_display_list;

	/// The canvas for dynamic drawing
	//
	/// WARNING: since DynamicShape is a character_def, which is
	///          in turn a ref_counted, we'd better keep
	///          this by intrusive_ptr, even if we're the sole
	///          owners. The problem is in case a pointer to this
	///	     instance ever gets passed to some function wrapping
	///	     it into an intrusive_ptr, in which case the stack
	///          object will be destroyed, with horrible consequences ...
	///
	boost::intrusive_ptr<DynamicShape> _drawable;

	/// The need of an instance here is due to the renderer
	/// insising on availability a shape_character_def instance
	/// that has a parent (why?)
	///
	boost::intrusive_ptr<character> _drawable_inst;

	// this is deprecated, we'll be pushing gotoframe target
	// actions to the global action queue
	//ActionList	m_goto_frame_action_list;

	play_state	m_play_state;

	// 0-based index to current frame
	size_t		m_current_frame;

	// true if this sprite reached the last frame and restarted
	bool		m_has_looped;

	// true is the sprite is jumping back.
	bool		is_jumping_back;

	// true is we're calling frame actions
	bool _callingFrameActions;

	// a bit-array class would be ideal for this
	std::set<size_t>	m_init_actions_executed;

	/// This timeline's variable scope
	as_environment	m_as_environment;

	/// Increment m_current_frame, and take care of looping.
	void increment_frame_and_check_for_loop();

	/// key events are: KEY_DOWN, KEY_PRESS, KEY_UP
	bool m_has_key_event;

	bool m_has_mouse_event;

	typedef boost::intrusive_ptr< edit_text_character > TextFieldPtr;
	typedef std::vector< TextFieldPtr > TextFieldPtrVect;

	/// A container for textfields, indexed by their variable name
	typedef std::map< std::string, TextFieldPtrVect > TextFieldMap;

	/// We'll only allocate Textfield variables map if
	/// we need them (ie: anyone calls set_textfield_variable)
	///
	std::auto_ptr<TextFieldMap> _text_variables;

	/// \brief
	/// Returns a vector of TextField associated with the given variable name,
	/// or NULL if no such variable name is known.
	//
	/// A TextField variable is a variable that acts
	/// as a setter/getter for a TextField 'text' member.
	///
	/// Search is case-sensitive.
	///
	/// @todo find out wheter we should be case sensitive or not
	///
	/// @return a pointer inside a vector, will be invalidated by modifications
	///         of the vector (set_textfield_variable)
	///
	TextFieldPtrVect* get_textfield_variable(const std::string& name);

	/// soundid for current playing stream. If no stream set to -1
	int m_sound_stream_id;

	std::string _origTarget;

protected:

	void place_character(character* ch, int depth,
			const cxform& color_transform, const matrix& mat,
			int ratio, int clip_depth)
	{
		m_display_list.place_character(ch, depth, color_transform, mat, ratio, clip_depth);
	}

	/// Execute the tags associated with the specified frame.
	///
	/// @param frame
	///	Frame number. 0-based
	///
	/// @param typeflags
	///     Which kind of control tags we want to execute. 
	///	See control_tag_type enum.
	///
	void execute_frame_tags(size_t frame, int typeflags=TAG_DLIST|TAG_ACTION); 

	/// \brief
	/// This is either sprite_definition (for sprites defined by
	/// DefineSprite tag) or movie_def_impl (for the top-level movie).
	boost::intrusive_ptr<movie_definition>	m_def;

	/// List of loadVariables requests
	typedef std::list<LoadVariablesThread*> LoadVariablesThreads;

	/// List of active loadVariable requests 
	//
	/// At ::advance_sprite time, all completed requests will
	/// be processed (variables imported in this timeline scope)
	/// and removed from the list.
	LoadVariablesThreads _loadVariableRequests;

	/// Process any completed loadVariables request
	void processCompletedLoadVariableRequests();

	/// Process a completed loadVariables request
	void processCompletedLoadVariableRequest(LoadVariablesThread& request);

#ifdef GNASH_USE_GC
	/// Mark sprite-specific reachable resources and invoke
	/// the parent's class version (markCharacterReachable)
	//
	/// sprite-specific reachable resources are:
	/// 	- DisplayList items (current, backup and frame0 ones)
	///	- Canvas for dynamic drawing (_drawable)
	///	- Drawable instance (_drawable_inst)
	///	- sprite environment
	///	- definition the sprite has been instantiated from
	///	- Textfields having an associated variable registered in this instance.
	///	- Relative root of this instance (m_root)
	///
	virtual void markReachableResources() const;
#endif // GNASH_USE_GC
};

/// Initialize the global MovieClip class
void movieclip_class_init(as_object& global);


} // end of namespace gnash

#endif // GNASH_SPRITE_INSTANCE_H
