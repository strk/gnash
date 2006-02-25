// dlist.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A list of active characters.


#ifndef GNASH_DLIST_H
#define GNASH_DLIST_H


#include "container.h"
#include "types.h"
#include "impl.h"

namespace gnash {
	
	/// A struct to serve as an entry in the display list.
	struct display_object_info {
		bool	m_ref;

		/// state is held in here
		smart_ptr<character>	m_character;
		
		display_object_info()
				:
				m_ref(false)
		{
		}
		
		display_object_info(const display_object_info& di)
				:
				m_ref(false)
		{
			*this = di;
		}
		
		~display_object_info()
		{
		}
		
		void	operator=(const display_object_info& di)
		{
			m_ref = di.m_ref;
			m_character = di.m_character;
		}
		
		void	set_character(character* ch)
		{
			m_character = ch;
		}
		
		/// For qsort().
		static int compare(const void* _a, const void* _b);
	};


	/// A list of active characters.
	struct display_list {
		// TODO use better names!
		int	find_display_index(int depth);
		int	get_display_index(int depth);
		
		/// Add a new character in this display list.
		//
		/// If applicable, the event_id::LOAD event
		/// associated with the given character
		/// is called as last step of addition. 
		///
		/// @param ch 
		///	the character to be added into the list
		///
		/// @param depth 
		///	depth to be assign to the character
		///	using character::set_depth
		///
		/// @param replace_if_depth_is_occupied 
		///	If this is false, caller wants to allow multiple
		///	objects	with the same depth.
		///	find_display_index() returns the first matching
		///	depth, if there	are any, so the new character
		///	will get inserted before all the others with the
		///	same depth.  This matches the semantics
		///	described by Alexi's SWF ref.  (This is all
		///	for legacy SWF compatibility anyway.)
		///
		/// @param color_xform
		///	Color transform to be applied to the character
		///	using character::set_cxform
		///
		/// @param mat
		///	matrix to be assigned to the character
		///	using character::set_matrix
		///
		/// @param ratio
		///	ratio (scale?) to be assigned to the character
		///	using character::set_ratio
		///
		/// @param clip_depth
		///	clip_depth (?) to be assigned to the character
		///	using character::set_clip_depth
		///
		void	add_display_object(
			character* ch,
			Uint16 depth,
			bool replace_if_depth_is_occupied,
			const cxform& color_xform,
			const matrix& mat,
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
			Uint16 clip_depth);

		/// Puts a new character at the specified depth, replacing any
		/// existing character.  If use_cxform or use_matrix are false,
		/// then keep those respective properties from the existing
		/// character.
		void	replace_display_object(
			character* ch,
			Uint16 depth,
			bool use_cxform,
			const cxform& color_xform,
			bool use_matrix,
			const matrix& mat,
			float ratio,
			Uint16 clip_depth);

		/// Removes the object at the specified depth.
		void	remove_display_object(Uint16 depth, int id);

		/// clear the display list.
		void	clear();

		/// reset the references to the display list.
		void	reset();

		/// remove unreferenced objects.
		void	update();

		/// advance referenced characters.
		void	advance(float delta_time);

		/// Display the referenced characters.
		/// Lower depths are obscured by higher depths.
		void	display();

		// unused
		//void	display(const display_info& di);
		
		int	get_character_count() { return m_display_object_array.size(); }
		character*	get_character(int index) { return m_display_object_array[index].m_character.get_ptr(); }

		// May return NULL.
		character*	get_character_at_depth(int depth);

		// May return NULL.
		// If there are multiples, returns the *first* match only!
		character*	get_character_by_name(const tu_string& name);

		// May return NULL.
		// If there are multiples, returns the *first* match only!
		character*	get_character_by_name_i(const tu_stringi& name);

		inline const display_object_info&	get_display_object(int idx) const
		// get the display object at the given position.
		{
			return m_display_object_array[idx];
		}


//		void	set_character_position(character* ch, float x, float y);

	private:
		std::vector<display_object_info> m_display_object_array;
	};


}


#endif // GNASH_DLIST_H



// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
