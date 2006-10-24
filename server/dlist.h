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
// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//



#ifndef GNASH_DLIST_H
#define GNASH_DLIST_H


#include "container.h"
#include "types.h"
#include "impl.h"

#include <list>
#include <iosfwd>

namespace gnash {

/// A DisplayItem is simply a character object 
typedef smart_ptr<character> DisplayItem;

/// A list of on-stage characters, ordered by depth
//
/// Any sprite_instance has an associated DisplayList
/// that may change from frame to frame due to control
/// tags instructing when to add or remove characthers
/// from the stage.
///
class DisplayList {

public:

	/// \brief
	/// Place a new character in this display list
	/// replacing any other char at the same depth.
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
	void	place_character(
		character* ch,
		uint16_t depth,
		const cxform& color_xform,
		const matrix& mat,
		float ratio,
		uint16_t clip_depth);

	/// \brief
	/// Puts a new character at the specified depth, replacing any
	/// existing character.
	//
	/// If use_cxform or use_matrix are false, and a character is
	/// present at the given depth, then keep those respective
	/// properties from the existing character.
	///
	/// TODO: use pointers for matrix and cxform, and use NULL
	///       instead of the two bool arguments
	///
	void replace_character(
		character* ch,
		uint16_t depth,
		bool use_cxform,
		const cxform& color_xform,
		bool use_matrix,
		const matrix& mat,
		float ratio,
		uint16_t clip_depth);

	void swap_characters(character* ch, character* ch2);

	/// Updates the transform properties of the object at
	/// the specified depth.
	void	move_display_object(
		uint16_t depth,
		bool use_cxform,
		const cxform& color_xform,
		bool use_matrix,
		const matrix& mat,
		float ratio,
		uint16_t clip_depth);

	/// Removes the object at the specified depth.
	void	remove_display_object(uint16_t depth);

	/// \brief
	/// Clear the display list, calling the UNLOAD event
	/// on each item still present
	void clear();

	//Vitaly:
	// It is executed only before the second and the subsequent
	// execution of execute_frame_tags(0) for sprite_instance
	// with frame count > 1.
	// Deletes the display objects created during execution 
	// of frames 2,... and not displayed in the 1-st frame.
	// Macromedia Flash does not call remove display object tag
	// for 1-st frame
	void clear_unaffected(std::vector<uint16>& affected_depths);

	/// \brief
	/// Clear the display list, w/out calling the UNLOAD event
	/// on the items.
	void reset();

	/// advance referenced characters.
	void advance(float delta_time);

	/// \brief
	/// Display the referenced characters.
	/// Lower depths are obscured by higher depths.
	void display();

	/// May return NULL.
	character* get_character_at_depth(int depth);

	const character* get_character_at_depth(int depth) const {
		return const_cast<DisplayList*>(this)->get_character_at_depth(depth);
	}

	/// \brief
	/// May return NULL.
	/// If there are multiples, returns the *first* match only!
	character* get_character_by_name(const tu_string& name);

	const character* get_character_by_name(const tu_string& name) const {
		return const_cast<DisplayList*>(this)->get_character_by_name(name);
	}

	/// \brief
	/// May return NULL.
	/// If there are multiples, returns the *first* match only!
	character* get_character_by_name_i(const tu_stringi& name);

	/// \brief 
	/// Visit each character in the list in depth order
	/// (lower depth first).
	//
	/// The visitor functor will 
	/// receive a character pointer; must return true if
	/// it wants next item or false to exit the loop.
	template <class V>
	inline void visitForward(V& visitor);

	/// \brief 
	/// Visit each character in the list in reverse depth
	/// order (higher depth first).
	//
	/// The visitor functor
	/// will receive a character pointer; must return true if
	/// it wants next item or false
	/// to exit the loop.
	template <class V>
	inline void visitBackward(V& visitor);

	/// dump list to given output stream (debugging)
	void dump(std::ostream& os) const;

  /// Like character_instance::get_invalidated_bounds() this method calls the
  /// method with the same name of all childs.	
	void get_invalidated_bounds(rect* bounds, bool force);
	

	/// Return number of elements in the list
	size_t size() const { 
		return _characters.size();
	}

private:

	typedef std::list<DisplayItem> container_type;
	typedef container_type::iterator iterator;
	typedef container_type::const_iterator const_iterator;
	typedef container_type::reverse_iterator reverse_iterator;
	typedef container_type::const_reverse_iterator const_reverse_iterator;

	container_type _characters;


};

template <class V>
void
DisplayList::visitForward(V& visitor)
{
	for (iterator it = _characters.begin(),
			itEnd = _characters.end();
		it != itEnd; ++it)
	{
		DisplayItem& di = *it;
		if ( ! visitor(di.get_ptr()) )
		{
			break;
		}
	}
}

template <class V>
void
DisplayList::visitBackward(V& visitor)
{
	for (reverse_iterator it = _characters.rbegin(),
			itEnd = _characters.rend();
		it != itEnd; ++it)
	{
		DisplayItem& di = *it;

		//if ( ! di.get_ptr() ) continue;

		if ( ! visitor(di.get_ptr()) )
		{
			// Can so happens that the uppermost depth contains shape
			// and under it the button lays
			// therefore we skip empty(no events) depth
			if (di.get_ptr()->can_handle_mouse_event())
			{
				break;
			}
		}
	}
}

}


#endif // GNASH_DLIST_H



// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
