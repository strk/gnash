// generic_character.cpp:  Mouse/Character handling, for Gnash.
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

#include "generic_character.h"

namespace gnash
{

void
generic_character::add_invalidated_bounds(InvalidatedRanges& ranges, 
  bool force)
{
  ranges.add(m_old_invalidated_ranges);
  if (m_visible && (m_invalidated||force))
  {
    rect bounds;    
    bounds.expand_to_transformed_rect(getWorldMatrix(), 
      m_def->get_bound());
    ranges.add(bounds.getRange());            
  }    
}

void
generic_character::enclose_own_bounds(rect *) const
{
  log_unimpl("generic_character::enclose_own_bounds");
  abort(); // TO BE IMPLEMENTED!!!!!
}

bool
generic_character::pointInShape(boost::int32_t  x, boost::int32_t  y) const
{
  SWFMatrix wm = getWorldMatrix();
  SWFMatrix wm_inverse = wm.invert();
  point  lp(x, y);
  wm_inverse.transform(lp);
  return m_def->point_test_local(lp.x, lp.y, wm);
}


void  
generic_character::display()
{
  m_def->display(this); // pass in transform info
  
  clear_invalidated();
}

} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
