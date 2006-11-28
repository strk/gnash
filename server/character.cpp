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
//
//

/* $Id: character.cpp,v 1.12 2006/11/28 16:20:27 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "character.h"
#include "sprite_instance.h"

namespace gnash
{

void
character::do_mouse_drag()
{
	drag_state st;
	get_drag_state(st);
	if ( this == st.getCharacter() )
	{
		// We're being dragged!
		int	x, y, buttons;
		get_root_movie()->get_mouse_state(x, y, buttons);

		point world_mouse(PIXELS_TO_TWIPS(x), PIXELS_TO_TWIPS(y));
		if ( st.hasBounds() )
		{
			// Clamp mouse coords within a defined rect.
			// (it is assumed that drag_state keeps
			st.getBounds().clamp(world_mouse);
		}

		if (st.isLockCentered())
		{
		    matrix	world_mat = get_world_matrix();
		    point	local_mouse;
		    world_mat.transform_by_inverse(&local_mouse, world_mouse);

		    matrix	parent_world_mat;
		    if (m_parent != NULL)
			{
			    parent_world_mat = m_parent->get_world_matrix();
			}

		    point	parent_mouse;
		    parent_world_mat.transform_by_inverse(&parent_mouse, world_mouse);
					
		    // Place our origin so that it coincides with the mouse coords
		    // in our parent frame.
		    matrix	local = get_matrix();
		    local.m_[0][2] = parent_mouse.m_x;
		    local.m_[1][2] = parent_mouse.m_y;
		    set_matrix(local);
		}
		else
		{
			// Implement relative drag...
			static bool warned_relative_drag = false;
			if ( ! warned_relative_drag )
			{
				log_warning("FIXME: Relative drag unsupported");
		    		warned_relative_drag = true;
		    	}
		}
	}
}

matrix
character::get_world_matrix() const
{
	matrix m;
	if (m_parent != NULL)
	{
	    m = m_parent->get_world_matrix();
	}
	m.concatenate(get_matrix());

	return m;
}

cxform
character::get_world_cxform() const
{
	cxform	m;
	if (m_parent != NULL)
	{
	    m = m_parent->get_world_cxform();
	}
	m.concatenate(get_cxform());

	return m;
}

void
character::get_drag_state(drag_state& st)
{
	assert(m_parent != NULL);
	assert(m_parent->get_ref_count() > 0);
	m_parent->get_drag_state(st);
}

sprite_instance*
character::get_root_movie()
{
	assert(m_parent != NULL);
	assert(m_parent->get_ref_count() > 0);
	return m_parent->get_root_movie();
}

void
character::get_mouse_state(int& x, int& y, int& buttons)
{
	assert(m_parent != NULL);
	assert(m_parent->get_ref_count() > 0);
	get_parent()->get_mouse_state(x, y, buttons);
}

character*
character::get_relative_target_common(const std::string& name)
{
	if (name == "." || name == "this")
	{
	    return this;
	}
	else if (name == "..")
	{
		// Never NULL
		character* parent = get_parent();
		if ( ! parent )
		{
			// AS code trying to access something before the root
			log_warning("ActionScript code trying to refrence"
				" before the root MovieClip");
			parent = this;
		}
		return parent;
	}
	else if (name == "_level0"
	     || name == "_root")
	{
		return get_root_movie();
	}

	return NULL;
}


} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
