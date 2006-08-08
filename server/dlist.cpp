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

#include "dlist.h"
#include "log.h"
#include "render.h"
#include "gnash.h"

#include <typeinfo>
#include <iostream>
#include <algorithm>

namespace gnash {

struct DepthEquals {

	int _depth;

	DepthEquals(int depth)
		:
		_depth(depth)
	{}

	bool operator() (const DisplayItem& item) {
		if ( ! item.get_ptr() ) return false;
		return item->get_depth() == _depth;
	}
};

struct DepthGreaterOrEqual {

	int _depth;

	DepthGreaterOrEqual(int depth)
		:
		_depth(depth)
	{}

	bool operator() (const DisplayItem& item) {
		if ( ! item.get_ptr() ) return false;
		return item->get_depth() >= _depth;
	}
};

struct NameEquals {

	const tu_string& _name;

	NameEquals(const tu_string& name)
		:
		_name(name)
	{}

	bool operator() (const DisplayItem& item) {
		if ( ! item.get_ptr() ) return false;
		return item->get_name() == _name;
	}
};

struct NameEqualsNoCase {

	const tu_stringi& _name;

	NameEqualsNoCase(const tu_stringi& name)
		:
		_name(name)
	{}

	bool operator() (const DisplayItem& item) {
		if ( ! item.get_ptr() ) return false;
		return item->get_name() == _name;
	}
};

character*
DisplayList::get_character_at_depth(int depth)
{
	//GNASH_REPORT_FUNCTION;
	//dump(std::cout);

	for (iterator it = _characters.begin(),
			itEnd = _characters.end();
		it != itEnd; ++it)
	{
		character* ch = it->get_ptr();
		assert(ch); // is this really needed ?

		// found
		if ( ch->get_depth() == depth ) return ch;

		// non-existent (chars are ordered by depth)
		if ( ch->get_depth() > depth ) return NULL;
	}

	return NULL;

}


character*
DisplayList::get_character_by_name(const tu_string& name)
{
	container_type::iterator it = find_if(
			_characters.begin(),
			_characters.end(),
			NameEquals(name));

	if ( it == _characters.end() ) return NULL;
	else return it->get_ptr();

}

character*
DisplayList::get_character_by_name_i(const tu_stringi& name)
{
	container_type::iterator it = find_if(
			_characters.begin(),
			_characters.end(),
			NameEqualsNoCase(name));

	if ( it == _characters.end() ) return NULL;
	else return it->get_ptr();
}


void
DisplayList::place_character(
	character* ch, 
	uint16_t depth,
	const cxform& color_xform, 
	const matrix& mat, 
	float ratio,
	uint16_t clip_depth)
{
//	GNASH_REPORT_FUNCTION;
	//IF_VERBOSE_DEBUG(log_msg("dl::add(%d, '%s')\n", depth, ch->get_name()));//xxxxx

	//log_msg("Before adding, list is:");
	//dump(std::cout);

	assert(ch);
	
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
		//IF_VERBOSE_DEBUG(dbglogfile << "This is a new character" << std::endl );
		// add the new char
		_characters.insert(it, DisplayItem(ch));
	}
	else
	{
		//IF_VERBOSE_DEBUG(dbglogfile << "Replacing existing character" << std::endl );
		// replace existing char
		*it = DisplayItem(ch);
	}

	ch->on_event(event_id::CONSTRUCT);
	ch->execute_frame_tags(0, true);	// create dlist
						// only. FIXME: this
						// doesn't appear to
						// actually do anything.

	//log_msg("After adding, list is:");
	//dump(std::cout);
}

void
DisplayList::replace_character(
	character* ch,
	uint16_t depth,
	bool use_cxform,
	const cxform& color_xform,
	bool use_matrix,
	const matrix& mat,
	float ratio,
	uint16_t clip_depth)
{
	//GNASH_REPORT_FUNCTION;

	ch->set_depth(depth);
	ch->set_cxform(color_xform);
	ch->set_matrix(mat);
	ch->set_ratio(ratio);
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
//			log_msg("dl::replace_display_object()"
//				" no obj at depth %d\n", depth)
//		);

		// add the new char
		_characters.insert(it, di);

		ch->on_event(event_id::CONSTRUCT);
		ch->execute_frame_tags(0, true);	// create dlist
							// only. FIXME: this
		                                        // doesn't appear to
						        // actually do anything.
	}
	else
	{
		if (!use_cxform)
		{
			// Use the cxform from the old character.
			ch->set_cxform((*it)->get_cxform());
		}

		if (!use_matrix)
		{
			// Use the matrix from the old character.
			ch->set_matrix((*it)->get_matrix());
		}

		// replace existing char
		*it = di;
	}

}
	
	
// Updates the transform properties of the object at
// the specified depth.
void
DisplayList::move_display_object(
	uint16_t depth,
	bool use_cxform,
	const cxform& color_xform,
	bool use_matrix,
	const matrix& mat,
	float ratio,
	uint16_t /* clip_depth */)
{
	//GNASH_REPORT_FUNCTION;
	//IF_VERBOSE_DEBUG(log_msg("dl::move(%d)\n", depth));//xxxxx

	character* ch = get_character_at_depth(depth);
	if ( ! ch )
	{
		log_error("move_display_object() -- "
			"can't find object at depth %d\n",
			depth);
		return;
	}

	if (ch->get_accept_anim_moves() == false)
	{
		// This character is rejecting anim moves.  This happens after it
		// has been manipulated by ActionScript.
		return;
	}

	if (use_cxform)
	{
		ch->set_cxform(color_xform);
	}
	if (use_matrix)
	{
		ch->set_matrix(mat);
	}
	ch->set_ratio(ratio);
	// move_display_object apparently does not change clip depth!  Thanks to Alexeev Vitaly.
	// ch->set_clip_depth(clip_depth);
}
	
	
// Removes the object at the specified depth.
void
DisplayList::remove_display_object(uint16_t depth)
{
	//GNASH_REPORT_FUNCTION;

	//log_msg("Before removing, list is:");
	//dump(std::cout);

#ifndef NDEBUG
	container_type::size_type size = _characters.size();
#endif

	// TODO: optimize to take by-depth order into account
	container_type::iterator new_end = remove_if(
			_characters.begin(),
			_characters.end(),
			DepthEquals(depth));

	if ( new_end != _characters.end() )
	{
		//Vitaly: UNLOAD event in DisplayList::clear() it is not caused,
		// since character is removed already
		DisplayItem& di = *new_end;
		if (new_end->get_ptr())
		{
			di->on_event(event_id::UNLOAD);
		}

		_characters.erase(new_end, _characters.end());

	}

#ifndef NDEBUG
	assert(size >= _characters.size());
#endif

	//log_msg("Done removing, list is:");
	//dump(std::cout);
}
	
	
// clear the display list.
void
DisplayList::clear()
{
	//GNASH_REPORT_FUNCTION;

	for (iterator it = _characters.begin(),
			itEnd = _characters.end();
		it != itEnd; ++it)
	{
		DisplayItem& di = *it;
		if ( ! it->get_ptr() ) continue;
		di->on_event(event_id::UNLOAD);
	}
		
	_characters.clear();
}
	
	
// reset the references to the display list.
void
DisplayList::reset()
{
	// GNASH_REPORT_FUNCTION;

	// We just clear the container, but
	// might eventually keep it allocated
	// to reduce allocation costs.
	_characters.clear();
}
	
void
DisplayList::advance(float delta_time)
// advance referenced characters.
{
	//GNASH_REPORT_FUNCTION;

	container_type::size_type size = _characters.size();

	for (iterator it = _characters.begin(),
			itEnd = _characters.end();
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

		if (_characters.size() != size)
		{
			log_error("gnash bug: dlist size changed due to character actions, bailing on update!\n");
			break;
		}

		// keep the character alive in case actions in it
		// will remove it from displaylist.
		smart_ptr<character> ch = *it;
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
	
//		printf(".");

	bool masked = false;
	int highest_masked_layer = 0;
	
	//log_msg("number of objects to be drawn %i\n", m_display_object_array.size());
	
	for( iterator it = _characters.begin(),
			endIt = _characters.end();
		it != endIt; ++it)
	{
		DisplayItem& dobj = *it;

		//character*	ch = dobj.m_character.get_ptr();
		character*	ch = dobj.get_ptr();

		assert(ch);

		if (ch->get_visible() == false)
		{
			// Don't display.
			continue;
		}

		if (ch->get_clip_depth() > 0)
		{
//				log_msg("depth %i, clip_depth %i\n", dobj.m_depth, dobj.m_clip_depth);
		}

		// check whether a previous mask should be disabled
		if (masked)
		{
			if (ch->get_depth() > highest_masked_layer)
			{
//					log_msg("disabled mask before drawing depth %i\n", ch->get_depth());
				masked = false;
				// turn off mask
				render::disable_mask();
			}
		}

		// check whether this object should become mask
		if (ch->get_clip_depth() > 0)
		{
			//log_msg("begin submit mask\n");
			render::begin_submit_mask();
		}
		
		ch->display();

		if (ch->get_clip_depth() > 0)
		{
//				log_msg("object drawn\n");
		}
		
		// if this object should have become a mask,
		// inform the renderer that it now has all
		// information about it
		if (ch->get_clip_depth() > 0)
		{
			//log_msg("end submit mask\n");
			render::end_submit_mask();
			highest_masked_layer = ch->get_clip_depth();
			masked = true;
		}
	}
	
	if (masked)
	{
		// If a mask masks the scene all the way up to the highest
		// layer, it will not be disabled at the end of drawing
		// the display list, so disable it manually.
		render::disable_mask();
	}
}

/*private*/
void
DisplayList::dump(std::ostream& os)
{
	int num=0;
	for( iterator it = _characters.begin(),
			endIt = _characters.end();
		it != endIt; ++it)
	{
		DisplayItem& dobj = *it;
		os << "Item " << num << " at depth " << dobj->get_depth()
			<< " (char id " << dobj->get_id() << ")" << std::endl;
		num++;
	}
}

} // namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
