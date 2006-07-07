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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <character.h>
#include <sprite_instance.h> 

namespace gnash
{

void
character::do_mouse_drag()
{
    drag_state	st;
    get_drag_state(&st);
    if (this == st.m_character)
	{
	    // We're being dragged!
	    int	x, y, buttons;
	    get_root_movie()->get_mouse_state(&x, &y, &buttons);

	    point	world_mouse(PIXELS_TO_TWIPS(x), PIXELS_TO_TWIPS(y));
	    if (st.m_bound)
		{
		    // Clamp mouse coords within a defined rect.
		    world_mouse.m_x =
			fclamp(world_mouse.m_x, st.m_bound_x0, st.m_bound_x1);
		    world_mouse.m_y =
			fclamp(world_mouse.m_y, st.m_bound_y0, st.m_bound_y1);
		}

	    if (st.m_lock_center)
		{
		    matrix	world_mat = get_world_matrix();
		    point	local_mouse;
		    world_mat.transform_by_inverse(&local_mouse, world_mouse);

		    matrix	parent_world_mat;
		    if (m_parent)
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
		}
	}
}

matrix
character::get_world_matrix() const
{
	matrix m;
	if (m_parent)
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
	if (m_parent)
	{
	    m = m_parent->get_world_cxform();
	}
	m.concatenate(get_cxform());

	return m;
}

void
character::get_drag_state(drag_state* st)
{
	assert(m_parent);
	m_parent->get_drag_state(st);
}

sprite_instance*
character::get_root_movie()
{
	return m_parent->get_root_movie();
}

void
character::get_mouse_state(int* x, int* y, int* buttons)
{
	get_parent()->get_mouse_state(x, y, buttons);
}

character*
character::get_relative_target_common(const tu_string& name)
{
	if (name == "." || name == "this")
	{
	    return this;
	}
	else if (name == "..")
	{
		// this is possibly NULL, it seems
		return get_parent();
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
