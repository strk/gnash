// dlist.h:  Display list definitions, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifndef GNASH_DLIST_H
#define GNASH_DLIST_H

#include "container.h"
#include "types.h"
#include "impl.h"
#include "snappingrange.h"
#include "character.h"

#include <list>
#include <iosfwd>

namespace gnash {
	class cxform;
}

namespace gnash {

/// A DisplayItem is simply a character object 
typedef boost::intrusive_ptr<character> DisplayItem;

/// A list of on-stage characters, ordered by depth
//
/// Any sprite_instance has an associated DisplayList
/// that may change from frame to frame due to control
/// tags instructing when to add or remove characters
/// from the stage.
///
class DisplayList {

public:

	/// Output operator
	friend std::ostream& operator<< (std::ostream&, const DisplayList&);

	/// \brief
	/// Place a new character in this display list
	/// replacing any other char at the same depth.
	//
	/// If a character is replaced, it's unload() method
	/// is invoked.
	///
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
		int depth,
		const cxform& color_xform,
		const matrix& mat,
		int ratio,
		int clip_depth);

	/// \brief
	/// Puts a new character at the specified depth, replacing any
	/// existing character.
	//
	/// If use_cxform or use_matrix are false, and a character is
	/// present at the given depth, then keep those respective
	/// properties from the existing character.
	///
	/// @param color_xform
	///	The color tranform to assign to the new character.
	///	If NULL the default color transform will be kept.
	//
	/// @param mat
	///	The matrix tranform to assign to the new character.
	///	If NULL the default matrix will be kept.
	///
	void replace_character(
		character* ch,
		int depth,
		const cxform* color_xform,
		const matrix* mat,
		int ratio,
		int clip_depth);

	/// \brief
	/// Change depth of the given characters in the list,
	/// swapping with any existing character at target depth.
	//
	/// List ordering will be maintained by this function.
	///
	/// Any character affected by this operation (none on invalid call,
	/// 1 if new depth is not occupied, 2 otherwise) will be:
	///	- bounds invalidated (see character::set_invalidated)
	///	- marked as script-transformed (see character::transformedByScript)
	/// 
	/// @param ch
	///	The character to apply depth swapping to.
	///	If not found in the list, an error is raised
	///	and no other action is taken.
	///
	/// @param depth
	///	The new depth to assign to the given character.
	///	If occupied by another character, the target character
	///	will get the current depth of the first.
	///	If target depth equals the current depth of character, an
	///	assertion fails, as I think the caller should check this instead.
	///
	void swapDepths(character* ch, int depth);

	/// \brief
	/// Updates the transform properties of the object at the
	/// specified depth, unless its get_accept_anim_moves() returns false.
	//
	///  See character::get_accept_anim_moves()
	///
	/// @param color_xform
	///	The color tranform to assign to the new character.
	///	If NULL the default color transform will be kept.
	//
	/// @param mat
	///	The matrix tranform to assign to the new character.
	///	If NULL the default matrix will be kept.
	///
	void	move_display_object(
		int depth,
		const cxform* color_xform,
		const matrix* mat,
		int ratio,
		int clip_depth);

	/// Removes the object at the specified depth.
	//
	/// Calls unload on the removed character.
	///
	void	remove_display_object(int depth);

	/// Clear the display list.
	//
	/// @param call_unload
	///	If true, UNLOAD event will be invoked on the characters being
	///	removed. False by default.
	///
	void clear(bool call_unload=false);

	/// \brief
	/// Clear all characters in this DisplayList that are also found
	/// in the given DisplayList
	//
	/// @param from
	///	A DisplayList containing character instances to clear.
	///	Any instance found in in will be removed from this DisplayList.
	///
	/// @param call_unload
	///	If true, UNLOAD event will be invoked on the characters being
	///	removed. False by default.
	///
	void clear(const DisplayList& from, bool call_unload=false);

	/// \brief
	/// Clear all characters in the display list also found in the given vector.
	//
	/// @param which
	///	A vector containing character instances to remove.
	///	Any instance found in the vector will be removed
	///	from this DisplayList.
	///
	/// @param call_unload
	///	If true, UNLOAD event will be invoked on the characters being
	///	removed. False by default.
	///
	void clear(std::vector<character*>& which, bool call_unload=false);

	/// \brief
	/// Clear all characters in the display list except the ones
	/// contained in the given vector.
	//
	/// @param exclude
	///	A vector containing character instances to keep.
	///	Any instance not found in the vector will be removed
	///	from this DisplayList.
	///
	/// @param call_unload
	///	If true, UNLOAD event will be invoked on the characters being
	///	removed. False by default.
	///
	void clear_except(std::vector<character*>& exclude, bool call_unload=false);

	/// \brief
	/// Clear all characters in this DisplayList except the ones
	/// contained in the given DisplayList
	//
	/// @param exclude
	///	A DisplayList containing character instances to keep.
	///	Any instance not found in in will be removed
	///	from this DisplayList.
	///
	/// @param call_unload
	///	If true, UNLOAD event will be invoked on the characters being
	///	removed. False by default.
	///
	void clear_except(const DisplayList& exclude, bool call_unload=false);

	/// Add all characters in the list, maintaining depth-order
	//
	/// @param chars
	///	The characters to add
	///
	/// @param replace
	///	If true the given characters would replace any
	///	pre-existing character at the same depth.
	///
	void addAll(std::vector<character*>& chars, bool replace);

	/// Add a character in the list, maintaining depth-order
	//
	///
	/// @param ch
	///	The character to add
	///
	/// @param replace
	///	If true the given character would replace any
	///	pre-existing character at the same depth.
	///
	void add(character* ch, bool replace);

	/// \brief
	/// Reset the list removing any static character not supposed to be there
	/// in the given target frame.
	//
	/// Only instances in static depth are candidates for removal, and not all
	/// of them are removed. Dynamic instances in static depth zone are always
	/// removed. Timeline instances are only removed if not supposed to be
	/// there in the target frame. This information is extracted from the
	/// Timeline object associated with the given movie_definition (movieDef).
	///
	/// This method implements steps 1 and 2 of 3rd redesign attempt for
	/// display list reconstruction. 
	/// See: http://www.gnashdev.org/wiki/index.php/TimelineControl
	///
	/// @param movieDef
	///	Movie definition from which to extract Timeline information.
	///
	/// @param targetFrame
	///	0-based frame number we are jumping back to.
	///
	/// @param call_unload
	///	If true, UNLOAD event will be invoked on the characters being
	///	removed. 
	///
	void reset(movie_definition& movieDef, size_t targetFrame, bool call_unload);

	/// Just an alias for clear()
	void reset() {
		clear();
	}

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
	character* get_character_by_name(const std::string& name);

	const character* get_character_by_name(const std::string& name) const
	{
		return const_cast<DisplayList*>(this)->get_character_by_name(name);
	}

	/// \brief
	/// May return NULL.
	/// If there are multiples, returns the *first* match only!
	character* get_character_by_name_i(const std::string& name);

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

	/// \brief 
	/// Visit each and all character in the list.
	//
	/// Scan happens in arbitrary order, if order is
	/// important use visitBackward or visitForward
	///
	/// The visitor functor will receive a character pointer,
	/// it's return value is not used so can return void.
	template <class V>
	inline void visitAll(V& visitor);

	/// dump list to logfile/stderr
	void dump() const;

  /// Like character_instance::add_invalidated_bounds() this method calls the
  /// method with the same name of all childs.	
	void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);	
	

	/// Return number of elements in the list
	size_t size() const
	{ 
		return _characters.size();
	}

	/// Return true if the list contains no elements 
	bool empty() const 
	{
		return _characters.empty();
	}

	/// Return the next highest available depth
	//
	/// Placing an object at the depth returned by
	/// this function should result in a character
	/// that is displayd above all others
	///
	int getNextHighestDepth() const;

	/// Sort list by depth (lower depths first)
	//
	/// You only need calling this method if depth
	/// of characters on the list has been externally
	/// changed. Usually it is DisplayList itself
	/// assigning depths, so won't need to call it.
	///
	/// A notable use for this is backing up a specific
	/// state and restoring it later. Restore step would
	/// need reordering.
	///
	void sort ();
	
	bool operator==(const DisplayList& other) const { return _characters == other._characters; }

	bool operator!=(const DisplayList& other) const { return _characters != other._characters; }

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
		if ( ! visitor(di.get()) ) break;
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
		if ( ! visitor(di.get()) ) break;
	}
}

template <class V>
void
DisplayList::visitAll(V& visitor)
{
	for (iterator it = _characters.begin(),
			itEnd = _characters.end();
		it != itEnd; ++it)
	{
		visitor(it->get());
	}
}

std::ostream& operator<< (std::ostream&, const DisplayList&);

} // namespace gnash


#endif // GNASH_DLIST_H



// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
