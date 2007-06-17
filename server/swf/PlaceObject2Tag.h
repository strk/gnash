// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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
//
//

/* $Id: PlaceObject2Tag.h,v 1.11 2007/06/17 09:07:27 zoulunkai Exp $ */

#ifndef GNASH_SWF_PLACEOBJECT2TAG_H
#define GNASH_SWF_PLACEOBJECT2TAG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "DisplayListTag.h" // for inheritance
#include "swf.h" // for tag_type definition
#include "matrix.h" // for composition
#include "cxform.h" // for composition 
#include "tu_types.h" // for uint16_t, uint32_t etc. definition 

//#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>

// Forward declarations
namespace gnash {
	class stream;
	class sprite_instance;
	class swf_event;
	class movie_definition;
}

namespace gnash {
namespace SWF {

/// SWF Tag PlaceObject (4) or PlaceObject2 (9) 
//
/// The PlaceObject tags can be used to:
///	- Place a character to a depth. See isPlace().
///	- Transform the character placed at a depth. See isMove().
///	- Replace a character at a depth. See isReplace().
///
/// In any case a single Timeline depth is affected.
/// Postcondition of this tag execution is presence of an instance
/// at the affected depth. See getDepth().
///
class PlaceObject2Tag : public DisplayListTag
{
public:

	typedef std::vector<swf_event*> EventHandlers;

	PlaceObject2Tag(const movie_definition& def)
		:
		DisplayListTag(0), // why is it 0 here and -1 for RemoveObjectTag ??
		m_tag_type(0),
		m_name(NULL),
		m_ratio(0),
		m_has_matrix(false),
		m_has_cxform(false),
		m_character_id(0),
		m_clip_depth(0),
		m_place_type(PLACE),
		_movie_def(def)
	{
	}

	~PlaceObject2Tag();

	/// Read SWF::PLACEOBJECT or SWF::PLACEOBJECT2
	void read(stream* in, tag_type tag, int movie_version);

	/// Place/move/whatever our object in the given movie.
	void execute(sprite_instance* m) const;

	/// Return true if this tag places a character
	bool isPlace() const { return m_place_type == PLACE; }

	/// Return true if this tag replaces a character
	bool isReplace() const { return m_place_type == REPLACE; }

	/// Return true if this tag transforms a character
	bool isMove() const { return m_place_type == MOVE; }

	static void loader(stream* in, tag_type tag, movie_definition* m);

    int getRatio() const { return m_ratio;}

private:

	int	m_tag_type;
	char*	m_name;
	int 	m_ratio;
	cxform	m_color_transform;
	matrix	m_matrix;
	bool	m_has_matrix;
	bool	m_has_cxform;
	uint16_t	m_character_id;
	int 	m_clip_depth;
	uint32_t all_event_flags; 

	enum place_type
	{
		PLACE,
		MOVE,
		REPLACE
	} m_place_type;

	const movie_definition& _movie_def;

	EventHandlers m_event_handlers;

	// read SWF::PLACEOBJECT 
	void readPlaceObject(stream* in);

	// read placeObject2 actions
	void readPlaceActions(stream* in, int movie_version);

	// read SWF::PLACEOBJECT2
	void readPlaceObject2(stream* in, int movie_version);

};

} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_PLACEOBJECT2TAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
