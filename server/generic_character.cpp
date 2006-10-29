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

/* $Id: generic_character.cpp,v 1.2 2006/10/29 18:34:11 rsavoye Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "generic_character.h"

namespace gnash
{

void
generic_character::get_invalidated_bounds(rect* bounds, bool force)
{
	bounds->expand_to_rect(m_old_invalidated_bounds);
	if (m_visible && (m_invalidated||force))
	{
		bounds->expand_to_transformed_rect(get_world_matrix(), 
			m_def->get_bound());            
	}    
}

void
generic_character::enclose_own_bounds(rect *) const
{
	log_error("generic_character::enclose_own_bounds unimplemented");
	assert(0); // TO BE IMPLEMENTED!!!!!
}

movie*
generic_character::get_topmost_mouse_entity(float x, float y)
{
	assert(get_visible());	// caller should check this.

	// @@ is there any generic_character derivate that
	//    can actually handle mouse events ?
	if ( ! can_handle_mouse_event() ) return NULL;

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

} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
