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
//

#ifndef GNASH_GENERIC_CHARACTER_H
#define GNASH_GENERIC_CHARACTER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "character.h" // for inheritance

#include "shape_character_def.h" // for get_invalidated_bounds 

#include <cassert>

namespace gnash {

// Forward declarations
class character_def;

/// For characters that don't store unusual state in their instances.
class generic_character : public character
{

protected:

    character_def*	m_def;

public:

    generic_character(character_def* def, character* parent, int id)
	:
	character(parent, id),
	m_def(def)
	{
	    assert(m_def);
	}

	virtual bool can_handle_mouse_event()	{ return false;	}

    virtual void	display()
	{
//			GNASH_REPORT_FUNCTION;
		
	    m_def->display(this);	// pass in transform info
	    clear_invalidated();
	    do_display_callback();
	}

    // @@ tulrich: these are used for finding bounds; TODO
    // need to do this using enclose_transformed_rect(),
    // not by scaling the local height/width!

    virtual float	get_height() const
	{
	    matrix	m = get_world_matrix();
	    float	h = m_def->get_height_local() * m.m_[1][1];
	    return h;
	}

    virtual float	get_width() const
	{
	    matrix	m = get_world_matrix();
	    float	w = m_def->get_width_local() * m.m_[0][0];
	    return w;
	}

    // new, from Vitaly.
    virtual movie*	get_topmost_mouse_entity(float x, float y)
	{
	    assert(get_visible());	// caller should check this.

	    matrix	m = get_matrix();
	    point	p;
	    m.transform_by_inverse(&p, point(x, y));

	    if (m_def->point_test_local(p.m_x, p.m_y))
		{
		    // The mouse is inside the shape.
		    return this;
		}
	    return NULL;
	}

	/// Return the character definition from which this
	/// instance derive. 
    character_def* get_character_def() { return m_def; }
    
  
  void enclose_own_bounds(rect *) const {
    assert(0); // TO BE IMPLEMENTED!!!!!
  }
    
	void get_invalidated_bounds(rect* bounds, bool force) {
    if (m_visible && (m_invalidated||force)) {
      bounds->expand_to_transformed_rect(get_world_matrix(), 
        m_def->get_bound());            
    }    
  }
    

};


}	// end namespace gnash


#endif // GNASH_GENERIC_CHARACTER_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
