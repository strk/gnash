// dlist.h:  Display list definitions, for Gnash.
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

#ifndef GNASH_DLIST_H
#define GNASH_DLIST_H

#include "impl.h"
#include "snappingrange.h"
#include "character.h"

#include <list>
#include <iosfwd>
#ifndef NDEBUG
#include "log.h"
#include <set>  // for testInvariant
#endif

// GNASH_PARANOIA_LEVEL:
// 0 : (not unimplemented)
// 1 : quick assertions
// 2 : add testInvariant
//
#ifndef GNASH_PARANOIA_LEVEL
# define GNASH_PARANOIA_LEVEL 1
#endif

namespace gnash {
	class cxform;
}

namespace gnash {

/// A DisplayItem is simply a character object 
typedef boost::intrusive_ptr<character> DisplayItem;

/// A list of on-stage characters, ordered by depth
//
/// Any MovieClip has an associated DisplayList
/// that may change from frame to frame due to control
/// tags instructing when to add or remove characters
/// from the stage.
///
class DisplayList {

public:

	void testInvariant() const
	{
#if GNASH_PARANOIA_LEVEL > 1
#ifndef NDEBUG
		DisplayList sorted = *this;
		// check no duplicated depths above non-removed zone.
		std::set<int> depths;
		for (const_iterator it=beginNonRemoved(_charsByDepth), itEnd=_charsByDepth.end(); it!=itEnd; ++it)
		{
			boost::intrusive_ptr<character> ch = *it;
			int depth = ch->get_depth();
			if ( ! depths.insert(depth).second )
			{
				log_debug("Depth %d is duplicated in DisplayList %p", depth, (const void*)this);
				abort();
			}
		}
		assert(isSorted()); // check we didn't screw up ordering
#endif
#endif // GNASH_PARANOIA_LEVEL > 1
	}

	/// Output operator
	friend std::ostream& operator<< (std::ostream&, const DisplayList&);

	/// \brief
	/// Place a new character at the specified depth,
	/// replacing any existing character at the same depth.
	//
	/// If a character is replaced, it's unload() method
	/// is invoked.
	///
	/// If applicable, the event_id::LOAD event
	/// associated with the given character
	/// is called as last step of addition. 
	///
	/// @param ch 
	///	the new character to be added into the list.
	///
	/// @param depth 
	///	depth at which the new character is placed.
	///
    /// @param initObj
    /// an object to initialize the new character's properties with.
	void place_character(character* ch, int depth, as_object* initObj = 0);

	/// \brief
	/// Replace the old character at the specified depth with
	/// the given new character.
	//
	/// Calls unload on the removed character.
	///
	/// @param ch 
	///	the new character to be put
	///
	/// @param depth 
	///	depth to be replaced
	///
	/// @param use_old_cxform
	/// true:  set the new character's cxform to the old one.
	/// false: keep the new character's cxform.
	///
	/// @param use_old_matrix
	/// true:  set the new character's transformation SWFMatrix to the old one.
	/// false: keep the new character's transformation SWFMatrix.
	///
	void replace_character(character* ch, int depth, 
		bool use_old_cxform,
		bool use_old_matrix);

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
	///	The color tranform to assign to the character at the given depth.
	///	If NULL the orignial color transform will be kept.
	//
	/// @param mat
	///	The SWFMatrix tranform to assign to the character at the given depth.
	///	If NULL the orignial SWFMatrix will be kept.
	///
	/// @param ratio
	/// The new ratio value to assign to the character at the given depth.
	/// If NULL the original ratio will be kept.
	///
	/// @clip_depth
	/// Not used at the moment.
	/// 
	void	move_character(
		int depth,
		const cxform* color_xform,
		const SWFMatrix* mat,
		int* ratio,
		int* clip_depth);

	/// Removes the object at the specified depth.
	//
	/// Calls unload on the removed character.
	///
	void	remove_character(int depth);

	/// Remove all unloaded character from the list
	//
	/// Removed characters still in the list are those
	/// on which onUnload event handlers were defined..
	///
	/// NOTE: we don't call the function recursively in the 
	///       contained elements, as that should not be needed
	///	  (ie: any inned thing will not be accessible anyway)
	///
	void removeUnloaded();

	/// Unload the characters in this DisplayList removing
	/// all but the ones with on onUnload event defined
	/// (checked by calling ::unload on them) and keeping
	/// the others, w/out depth-shifting them.
	///
	/// Return true if any child was kept (as they had onUnload defined)
	///
	bool unload();

	/// destroy all characters in this DisplayList
	void destroy();

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
	/// Display the referenced characters.
	/// Lower depths are obscured by higher depths.
	void display();
	
	void omit_display();

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
	///
	/// NOTE: all elements in the list are visited, even
	///       the removed ones (unloaded)
	/// TODO: inspect if worth providing an arg to skip removed
	///
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
	///
	/// NOTE: all elements in the list are visited, even
	///       the removed ones (unloaded)
	/// TODO: inspect if worth providing an arg to skip removed
	///
	template <class V>
	inline void visitBackward(V& visitor);
	template <class V>
	inline void visitBackward(V& visitor) const;

	/// \brief 
	/// Visit each and all character in the list.
	//
	/// Scan happens in arbitrary order, if order is
	/// important use visitBackward or visitForward
	///
	/// The visitor functor will receive a character pointer,
	/// it's return value is not used so can return void.
	///
	/// NOTE: all elements in the list are visited, even
	///       the removed ones (unloaded)
	/// TODO: inspect if worth providing an arg to skip removed
	///
	template <class V>
	inline void visitAll(V& visitor);

	template <class V>
	inline void visitAll(V& visitor) const;

	/// dump list to logfile/stderr
	void dump() const;

  /// Like character_instance::add_invalidated_bounds() this method calls the
  /// method with the same name of all childs.	
	void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);	
	
	void dump_character_tree(const std::string prefix) const;
	

	/// Return number of elements in the list
	size_t size() const
	{ 
		return _charsByDepth.size();
	}

	/// Return true if the list contains no elements 
	bool empty() const 
	{
		return _charsByDepth.empty();
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
	
	/// \brief
	/// merge the given display list
	void mergeDisplayList(DisplayList& newList);

	bool operator==(const DisplayList& other) const { return _charsByDepth == other._charsByDepth; }

	bool operator!=(const DisplayList& other) const { return _charsByDepth != other._charsByDepth; }

private:

	typedef std::list<DisplayItem> container_type;
	typedef container_type::iterator iterator;
	typedef container_type::const_iterator const_iterator;
	typedef container_type::reverse_iterator reverse_iterator;
	typedef container_type::const_reverse_iterator const_reverse_iterator;

	/// Return an iterator to the first element of the container NOT in the "removed" depth zone
	static iterator beginNonRemoved(container_type& c);

	/// Return an constant iterator to the first element of the container NOT in the "removed" depth zone
	static const_iterator beginNonRemoved(const container_type& c);

	/// Return an iterator succeeding the last element in zone (-16384, 0xffff-16384)
	static iterator dlistTagsEffectivZoneEnd(container_type& c);
	
	/// Return an constant iterator succeeding the last element in (-16384, 0xffff-16384)
	static const_iterator dlistTagsEffectivZoneEnd(const container_type& c);


	/// Re-insert a removed-from-stage character after appropriately
	/// shifting its depth based on the character::removedDepthOffset
	/// value.
	//
	/// PRE-CONDITIONS 
	///	- ch::isUnloaded() returns true (assertion fails otherwise)
	///	- ch is not already in the list (assertion fails otherwise)
	///
	/// TODO: inspect what should happen if the target depth is already occupied
	///
	void reinsertRemovedCharacter(boost::intrusive_ptr<character> ch);

	container_type _charsByDepth;

	/// Check that the list is sorted by depth
	bool isSorted() const;
};

template <class V>
void
DisplayList::visitForward(V& visitor)
{
	for (iterator it = _charsByDepth.begin(),
			itEnd = _charsByDepth.end();
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
	for (reverse_iterator it = _charsByDepth.rbegin(),
			itEnd = _charsByDepth.rend();
		it != itEnd; ++it)
	{
		DisplayItem& di = *it;
		if ( ! visitor(di.get()) ) break;
	}
}

template <class V>
void
DisplayList::visitBackward(V& visitor) const
{
	for (const_reverse_iterator it = _charsByDepth.rbegin(),
			itEnd = _charsByDepth.rend();
		it != itEnd; ++it)
	{
		const DisplayItem& di = *it;
		if ( ! visitor(di.get()) ) break;
	}
}

template <class V>
void
DisplayList::visitAll(V& visitor)
{
	for (iterator it = _charsByDepth.begin(),
			itEnd = _charsByDepth.end();
		it != itEnd; ++it)
	{
		visitor(it->get());
	}
}

template <class V>
void
DisplayList::visitAll(V& visitor) const
{
	for (const_iterator it = _charsByDepth.begin(),
			itEnd = _charsByDepth.end();
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
