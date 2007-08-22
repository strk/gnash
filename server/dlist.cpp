// dlist.cpp:  Display lists, for Gnash.
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

#include "dlist.h"
#include "log.h"
#include "render.h"
#include "gnash.h" 
#include "StringPredicates.h"
#include "sprite_instance.h"

#include <typeinfo>
#include <iostream>
#include <algorithm>
#include <stack>

namespace gnash {

class DepthEquals {
public:
	int _depth;

	DepthEquals(int depth)
		:
		_depth(depth)
	{}

	bool operator() (const DisplayItem& item) {
		if ( ! item.get() ) return false;
		return item->get_depth() == _depth;
	}
};

class DepthGreaterOrEqual {
public:
	int _depth;

	DepthGreaterOrEqual(int depth)
		:
		_depth(depth)
	{}

	bool operator() (const DisplayItem& item) {
		if ( ! item.get() ) return false;
		return item->get_depth() >= _depth;
	}
};

class NameEquals {
public:
	const std::string& _name;

	NameEquals(const std::string& name)
		:
		_name(name)
	{}

	bool operator() (const DisplayItem& item) {
		if ( ! item.get() ) return false;
		return item->get_name() == _name;
	}
};

class NameEqualsNoCase {

	StringNoCaseEqual noCaseEquals;

public:
	const std::string& _name;

	NameEqualsNoCase(const std::string& name)
		:
		_name(name)
	{}

	bool operator() (const DisplayItem& item)
	{
		if ( ! item.get() ) return false;
		return noCaseEquals(item->get_name(), _name);
	}
};

int
DisplayList::getNextHighestDepth() const
{
	int nexthighestdepth=0;
	for (const_iterator it = _characters.begin(),
			itEnd = _characters.end();
		it != itEnd; ++it)
	{
		character* ch = it->get();
		assert(ch); // is this really needed ?

		int chdepth = ch->get_depth();
		if ( chdepth >= nexthighestdepth )
		{
			nexthighestdepth = chdepth+1;
		}
	}
	return nexthighestdepth;
}

character*
DisplayList::get_character_at_depth(int depth)
{
	//GNASH_REPORT_FUNCTION;
	//dump();

	for (iterator it = _characters.begin(),
			itEnd = _characters.end();
		it != itEnd; ++it)
	{
		character* ch = it->get();
		assert(ch); // is this really needed ?

		// found
		if ( ch->get_depth() == depth ) return ch;

		// non-existent (chars are ordered by depth)
		if ( ch->get_depth() > depth ) return NULL;
	}

	return NULL;

}


character*
DisplayList::get_character_by_name(const std::string& name)
{
	container_type::iterator it = find_if(
			_characters.begin(),
			_characters.end(),
			NameEquals(name));

	if ( it == _characters.end() ) return NULL;
	else return it->get();

}

character*
DisplayList::get_character_by_name_i(const std::string& name)
{
	container_type::iterator it = find_if(
			_characters.begin(),
			_characters.end(),
			NameEqualsNoCase(name));

	if ( it == _characters.end() ) return NULL;
	else return it->get();
}


void
DisplayList::place_character(
	character* ch, 
	int depth,
	const cxform& color_xform, 
	const matrix& mat, 
	int ratio,
	int clip_depth)
{
//	GNASH_REPORT_FUNCTION;
	//log_msg(_("dl::add(%d, '%s')"), depth, ch->get_name());

	//log_msg(_("Before adding, list is:"));
	//dump();

	assert(ch);
	ch->set_invalidated();
	ch->set_depth(depth);
	ch->set_cxform(color_xform);
	ch->set_matrix(mat);
	ch->set_ratio(ratio);
	ch->set_clip_depth(clip_depth);

	container_type::iterator it = find_if(
			_characters.begin(), _characters.end(),
			DepthGreaterOrEqual(depth));

	if ( it == _characters.end() || (*it)->get_depth() != depth )
	{
		//log_msg(_("place_character: new character at depth %d"), depth);
		
		// add the new char
		_characters.insert(it, DisplayItem(ch));
	}
	else
	{
		//log_msg(_("place_character: replacing existing character at depth %d"), depth);
		
		// remember bounds of old char
		InvalidatedRanges old_ranges;	
		(*it)->add_invalidated_bounds(old_ranges, true);	
	
		(*it)->unload();
		// replace existing char
		*it = DisplayItem(ch);
		
		// extend invalidated bounds
		ch->extend_invalidated_bounds(old_ranges); 				
	}

	// Give life to this instance
	ch->construct();
}

void
DisplayList::add(character* ch, bool replace)
{
	int depth = ch->get_depth();

	container_type::iterator it = find_if(
			_characters.begin(), _characters.end(),
			DepthGreaterOrEqual(depth));
	if ( it == _characters.end() || (*it)->get_depth() != depth )
	{
		_characters.insert(it, DisplayItem(ch));
	}
	else if ( replace )
	{
		*it = DisplayItem(ch);
	}
}

void
DisplayList::addAll(std::vector<character*>& chars, bool replace)
{
	for (std::vector<character*>::iterator it=chars.begin(),
			itEnd=chars.end();
			it != itEnd; ++it)
	{
		add(*it, replace);
	}
}

void
DisplayList::replace_character(
	character* ch,
	int depth,
	const cxform* color_xform,
	const matrix* mat,
	int ratio,
	int clip_depth)
{
	//GNASH_REPORT_FUNCTION;

	ch->set_invalidated();
	ch->set_depth(depth);
	if ( color_xform ) ch->set_cxform(*color_xform);
	if ( mat ) ch->set_matrix(*mat);
	if(ratio != character::noRatioValue)
	{
		ch->set_ratio(ratio);
	}
	ch->set_clip_depth(clip_depth);
	ch->restart();

	container_type::iterator it = find_if(
			_characters.begin(), _characters.end(),
			DepthGreaterOrEqual(depth));

	DisplayItem di(ch);

	if ( it == _characters.end() || (*it)->get_depth() != depth )
	{

		// Error, no existing object found at depth.
//		IF_VERBOSE_DEBUG(
//			log_msg(_("dl::replace_display_object()"
//				" no obj at depth %d"), depth)
//		);

		// add the new char
		_characters.insert(it, di);

	}
	else
	{
		character* oldch = it->get();

		InvalidatedRanges old_ranges;
	
		if (!color_xform)
		{
			// Use the cxform from the old character.
			ch->set_cxform(oldch->get_cxform());
		}

		if (!mat)
		{
			// Use the matrix from the old character.
			ch->set_matrix(oldch->get_matrix());
		}
		
		// remember bounds of old char
		oldch->add_invalidated_bounds(old_ranges, true);		

		// replace existing char		
		*it = di;
		
		// extend invalidated bounds
		// WARNING: when a new Button character is added,
		//          the invalidated bounds computation will likely
		//          be bogus, as the actual character shown is not instantiated
		//          until ::construct for buttons (I'd say this is a bug in button_character_instance)
		//          UdoG, following ? 
		//
		ch->extend_invalidated_bounds(old_ranges);				

	}

	// Give life to this instance
	ch->construct();
}
	
	
// Updates the transform properties of the object at
// the specified depth.
void
DisplayList::move_display_object(
	int depth,
	const cxform* color_xform,
	const matrix* mat,
	int ratio,
	int /* clip_depth */)
{
	//GNASH_REPORT_FUNCTION;
	//IF_VERBOSE_DEBUG(log_msg(_("dl::move(%d)"), depth));

	character* ch = get_character_at_depth(depth);
	if ( ! ch )
	{
		// FIXME, should this be log_aserror?
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("move_display_object() -- "
			"can't find object at depth %d"),
			depth);
		);
		return;
	}

	// TODO: is sign of depth related to accepting anim moves ?
	if (ch->get_accept_anim_moves() == false)
	{
		// This character is rejecting anim moves.  This happens after it
		// has been manipulated by ActionScript.
		return;
	}

	if (color_xform)
	{
		ch->set_cxform(*color_xform);
	}
	if (mat)
	{
		ch->set_matrix(*mat);
	}
	if(ratio != character::noRatioValue)
	{
		ch->set_ratio(ratio);
	}
}
	
	
// Removes the object at the specified depth.
void
DisplayList::remove_display_object(int depth)
{
	//GNASH_REPORT_FUNCTION;

	//log_msg(_("Before removing, list is:"));
	//dump();

#ifndef NDEBUG
	container_type::size_type size = _characters.size();
#endif

	// TODO: optimize to take by-depth order into account
	container_type::iterator it = find_if(
			_characters.begin(),
			_characters.end(),
			DepthEquals(depth));

	if ( it != _characters.end() )
	{
		(*it)->unload();
		_characters.erase(it);

	}

#ifndef NDEBUG
	assert(size >= _characters.size());
#endif

	//log_msg(_("Done removing, list is:"));
	//dump();
}
	
	
// clear the display list.
void
DisplayList::clear(bool call_unload)
{
	//GNASH_REPORT_FUNCTION;

	// This might eventually become obsoleted
	if ( call_unload )
	{
		for (iterator it = _characters.begin(),
				itEnd = _characters.end();
			it != itEnd; ++it)
		{
			DisplayItem& di = *it;
			if ( ! it->get() ) continue;
			di->unload(); // if call_unload
		}
	}
		
	_characters.clear();
}

void
DisplayList::swapDepths(character* ch1, int newdepth)
{

	assert(ch1->get_depth() != newdepth);

	container_type::iterator it1 = find(_characters.begin(), _characters.end(), ch1);

	// upper bound ...
	container_type::iterator it2 = find_if(_characters.begin(), _characters.end(),
			DepthGreaterOrEqual(newdepth));

	if ( it1 == _characters.end() )
	{
		log_error("First argument to DisplayList::swapDepth() is NOT a character in the list. Call ignored.");
		return;
	}

	// Found another character at the given depth
	if ( it2 != _characters.end() && (*it2)->get_depth() == newdepth )
	{
		DisplayItem ch2 = *it2;

		int srcdepth = ch1->get_depth();

		ch2->set_depth(srcdepth);

		// TODO: we're not actually invalidated ourselves, rather our parent is...
		//       UdoG ? Want to verify this ?
		ch2->set_invalidated();

		// We won't accept static transforms after a depth swap.
		// See displaylist_depths_test6.swf for more info.
		ch2->transformedByScript();

		iter_swap(it1, it2);
	}

	// No character found at the given depth
	else
	{
		// Move the character to the new position
		// NOTE: insert *before* erasing, in case the list is
		//       the only referer of the ref-counted character
		_characters.insert(it2, ch1);
		_characters.erase(it1);
	}

	// don't change depth before the iter_swap case above, as
	// we'll need it to assign to the new character
	ch1->set_depth(newdepth);

	// TODO: we're not actually invalidated ourselves, rather our parent is...
	//       UdoG ? Want to verify this ?
	ch1->set_invalidated();

	// We won't accept static transforms after a depth swap.
	// See displaylist_depths_test6.swf for more info.
	ch1->transformedByScript();

#ifndef NDEBUG
	// TODO: make this a testInvariant() method for DisplayList
	DisplayList sorted = *this;
	sorted.sort();
	assert(*this == sorted); // check we didn't screw up ordering
#endif

}
	
void DisplayList::reset(movie_definition& movieDef, size_t tgtFrame, bool call_unload)
{
	//GNASH_REPORT_FUNCTION;

	// 1. Find all "timeline depth" for the target frame, querying the
	//    Timeline object in the sprite/movie definition (see implementation details)
	std::vector<int>  save;
	movieDef.getTimelineDepths(tgtFrame, save);

//#define GNASH_DEBUG_TIMELINE 1
#undef GNASH_DEBUG_TIMELINE
#ifdef GNASH_DEBUG_TIMELINE
	cout << "Depths found to save: " << endl;
	std::ostream_iterator<int> ostrIter(cout, "," ) ;
	std::copy(save.begin(), save.end(), ostrIter);
	cout << endl;
	cout << "Current DisplayList: " << *this << endl;
#endif


	typedef std::vector<int>::iterator SeekIter;

	SeekIter startSeek = save.begin();
    SeekIter endSeek = save.end();

	for (iterator it = _characters.begin(),	itEnd = _characters.end(); it != itEnd; )
	{
		DisplayItem& di = *it;

		int di_depth = di->get_depth();

		/// We won't scan chars in the dynamic depth zone
		if ( di_depth >= 0 ) return;

		/// Always remove non-timeline instances ?
		/// Seems so, at least for duplicateMovieClip
		TimelineInfo* info = di->getTimelineInfo();
		//if ( di->getTimelineInfo() == NULL )
		//if ( di->isDynamic() )
		if ( ! info )
		{
			// Not to be saved, killing
			if ( call_unload ) di->unload();
			it = _characters.erase(it);
			continue;
		}
		
		// remove all shapes and morphs, it should be safe and correct.
		// but suboptimal when considering the invalidated bound.
		// we need to do this in some corner cases. 
		if(!di->isActionScriptReferenceable())
		{
			it = _characters.erase(it);
			continue;
		}

		/// Only remove if not in the save vector, 
		SeekIter match = std::find(startSeek, endSeek, di_depth);
		if( match == save.end())
		{
			// Not to be saved, killing
			if ( call_unload ) di->unload();
			it = _characters.erase(it);
			continue;
		}

		// To be saved, don't kill

		// IFF the 'save' vector is known to be sorted
		// we can let next seek start to next seek item
		// We can't assume this here, unless we request
		// by caller, in the dox.
		//startSeek = ++match;

		++it;
	}
}

void
DisplayList::clear_except(std::vector<character*>& exclude, bool call_unload)
{
	//GNASH_REPORT_FUNCTION;

	for (iterator it = _characters.begin(),	itEnd = _characters.end(); it != itEnd; )
	{
		DisplayItem& di = *it;

		bool is_affected = false;
		for (size_t i=0, n=exclude.size(); i<n; ++i)
		{
			if (exclude[i] == di.get())
			{
				is_affected = true;
				break;
			}
		}

		if (is_affected == false)
		{
			if ( call_unload ) di->unload();
			it = _characters.erase(it);
			continue;
		}
		it++;
	}
}

void
DisplayList::clear_except(const DisplayList& exclude, bool call_unload)
{
	//GNASH_REPORT_FUNCTION;

	assert(&exclude != this);
	const container_type& keepchars = exclude._characters;

	for (iterator it = _characters.begin(),	itEnd = _characters.end(); it != itEnd; )
	{
		DisplayItem& di = *it;

		bool is_affected = false;
		for (const_iterator kit = keepchars.begin(), kitEnd = keepchars.end(); kit != kitEnd; ++kit)
		{
			if ( *kit == di )
			{
				is_affected = true;
				break;
			}
		}

		if (is_affected == false)
		{
			if ( call_unload ) di->unload();
			it = _characters.erase(it);
			continue;
		}
		it++;
	}
}

void
DisplayList::clear(std::vector<character*>& which, bool call_unload)
{
	//GNASH_REPORT_FUNCTION;

	for (iterator it = _characters.begin(),	itEnd = _characters.end(); it != itEnd; )
	{
		DisplayItem& di = *it;

		bool is_affected = false;
		for (size_t i=0, n=which.size(); i<n; ++i)
		{
			if (which[i] == di.get())
			{
				is_affected = true;
				break;
			}
		}

		if (is_affected)
		{
			if ( call_unload ) di->unload();
			it = _characters.erase(it);
			continue;
		}
		it++;
	}
}

void
DisplayList::clear(const DisplayList& from, bool call_unload)
{
	//GNASH_REPORT_FUNCTION;

	const container_type dropchars = from._characters;

	for (iterator it = _characters.begin(),	itEnd = _characters.end(); it != itEnd; )
	{
		DisplayItem& di = *it;

		bool is_affected = false;
		for (const_iterator kit = dropchars.begin(), kitEnd = dropchars.end(); kit != kitEnd; ++kit)
		{
			if ( *kit == di )
			{
				is_affected = true;
				break;
			}
		}

		if (is_affected)
		{
			if ( call_unload ) di->unload();
			it = _characters.erase(it);
			continue;
		}
		it++;
	}
}
	
void
DisplayList::advance(float delta_time)
// advance referenced characters.
{
	//GNASH_REPORT_FUNCTION;

//	container_type::size_type size = _characters.size();

	// That there was no crash gnash we iterate through the copy
	std::list<DisplayItem> tmp_list = _characters;

	for (iterator it = tmp_list.begin(), itEnd = tmp_list.end();
		it != itEnd; ++it)
	{
		// @@@@ TODO FIX: If array changes size due to
		// character actions, the iteration may not be
		// correct!
		//
		// What's the correct thing to do here?  Options:
		//
		// * copy the display list at the beginning,
		// iterate through the copy
		//
		// * use (or emulate) a linked list instead of
		// an array (still has problems; e.g. what
		// happens if the next or current object gets
		// removed from the dlist?)
		//
		// * iterate through current array in depth
		// order.  Always find the next object using a
		// search of current array (but optimize the
		// common case where nothing has changed).
		//
		// * ???
		//
		// Need to test to see what Flash does.

//		if (_characters.size() != size)
//		{
//			log_error(_("gnash bug: dlist size changed due to character actions, bailing on update"));
//			break;
//		}

		// keep the character alive in case actions in it
		// will remove it from displaylist.
		boost::intrusive_ptr<character> ch = *it;
		assert(ch!=NULL);

		ch->advance(delta_time);
	}

}
	
	
// Display the referenced characters. Lower depths
// are obscured by higher depths.
void
DisplayList::display()
{
    //GNASH_REPORT_FUNCTION;
    std::stack<int> clipDepthStack;
    
    for( iterator it = _characters.begin(), endIt = _characters.end();
                  it != endIt; ++it)
    {
        character* ch = it->get();
        assert(ch);

        // Check if this charater or any of its parents is a mask.
        // Characters act as masks should always be rendered to the
        // mask buffer despite their visibility.
        character * parent = ch->get_parent();
        bool renderAsMask = ch->isMask();
        while(!renderAsMask && parent)
        {
            renderAsMask = parent->isMask();
            parent = parent->get_parent();
        }
        
        // check for non-mask hiden characters
        if( !renderAsMask && (ch->get_visible() == false))
        {
            // Avoid stale old_invalidated_rect
            ch->clear_invalidated(); 
            // Don't display non-mask hiden characters
            continue;
        }
    
        int depth = ch->get_depth();
        // Discard useless masks
        while(!clipDepthStack.empty() && (depth > clipDepthStack.top()))
        {
            clipDepthStack.pop();
            render::disable_mask();
        }

        int clipDepth = ch->get_clip_depth();
        // Push a new mask to the masks stack
        if(clipDepth != character::noClipDepthValue)
        {
            clipDepthStack.push(clipDepth);
            render::begin_submit_mask();
        }
        
        ch->display();
        
        // Notify the renderer that mask drawing has finished.
        if (ch->isMask())
        {
            render::end_submit_mask();
        }
    } //end of for

    // Discard any remaining masks
    while(!clipDepthStack.empty())
    {
        clipDepthStack.pop();
        render::disable_mask();
    }
}

/*public*/
void
DisplayList::dump() const
{
	int num=0;
	for( const_iterator it = _characters.begin(),
			endIt = _characters.end();
		it != endIt; ++it)
	{
		const DisplayItem& dobj = *it;
		log_msg(_("Item %d at depth %d (char id %d, name %s, type %s"),
			num, dobj->get_depth(), dobj->get_id(),
			dobj->get_name().c_str(), typeid(*dobj).name());
		num++;
	}
}


void 
DisplayList::add_invalidated_bounds(InvalidatedRanges& ranges, bool force) {
    
	for( iterator it = _characters.begin(),
			endIt = _characters.end();
		it != endIt; ++it)
	{
    DisplayItem& dobj = *it;
#ifndef GNASH_USE_GC
    assert(dobj->get_ref_count() > 0);
#endif // ndef GNASH_USE_GC
    dobj->add_invalidated_bounds(ranges, force);
	}
	
}


/// This method is not in the header in the hope DisplayItemDepthLess
/// will be inlined by compiler

struct DisplayItemDepthLess {
	bool operator() (const DisplayItem& d1, const DisplayItem& d2)
	{
		return d1->get_depth() < d2->get_depth();
	}
};

void
DisplayList::sort()
{
	_characters.sort(DisplayItemDepthLess());
}

std::ostream& operator<< (std::ostream& os, const DisplayList& dl)
{
	for (DisplayList::const_iterator it = dl._characters.begin(),
			itEnd = dl._characters.end();
			it != itEnd;
			++it)
	{
		const DisplayItem& item = *it; 
		if ( it != dl._characters.begin() ) os << " | ";
		os << "Character id:" << item->get_id()
			<< " name:" << item->get_name()
			<< " depth:" << item->get_depth();
	}

	return os;
}

} // namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
