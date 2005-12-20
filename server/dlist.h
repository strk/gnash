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
	
	// A struct to serve as an entry in the display list.
	struct display_object_info {
		bool	m_ref;
		smart_ptr<character>	m_character;	// state is held in here
		
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
		
		static int compare(const void* _a, const void* _b); // For qsort().
	};


	// A list of active characters.
	struct display_list {
		// TODO use better names!
		int	find_display_index(int depth);
		int	get_display_index(int depth);
		
		void	add_display_object(
			character* ch,
			Uint16 depth,
			bool replace_if_depth_is_occupied,
			const cxform& color_xform,
			const matrix& mat,
			float ratio,
			Uint16 clip_depth);
		void	move_display_object(
			Uint16 depth,
			bool use_cxform,
			const cxform& color_xform,
			bool use_matrix,
			const matrix& mat,
			float ratio,
			Uint16 clip_depth);
		void	replace_display_object(
			character* ch,
			Uint16 depth,
			bool use_cxform,
			const cxform& color_xform,
			bool use_matrix,
			const matrix& mat,
			float ratio,
			Uint16 clip_depth);
		void	remove_display_object(Uint16 depth, int id);

		// clear the display list.
		void	clear();

		// reset the references to the display list.
		void	reset();

		// remove unreferenced objects.
		void	update();

		// advance referenced characters.
		void	advance(float delta_time);

		// display the referenced characters.
		void	display();
		void	display(const display_info& di);
		
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
		array<display_object_info> m_display_object_array;
	};


}


#endif // GNASH_DLIST_H



// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
