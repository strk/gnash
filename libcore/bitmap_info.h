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


#ifndef GNASH_BITMAP_INFO_H
#define GNASH_BITMAP_INFO_H

#include "ref_counted.h" // for inheritance
#include "dsodefs.h"

// Forward declarations
namespace image {
	class ImageBase;
}


namespace gnash {

/// Your render_handler creates bitmap_info's for gnash.  You
/// need to subclass bitmap_info in order to add the
/// information and functionality your app needs to render
/// using textures.
class DSOEXPORT bitmap_info : public ref_counted
{
public:
//	virtual void layout_image(image::ImageBase* /*im*/) { };
//	image::ImageBase*  m_suspended_image;

	unsigned int	m_texture_id;		// nuke?
	int		m_original_width;	// nuke?
	int		m_original_height;	// nuke?
		
	bitmap_info()
		:
//		m_suspended_image(NULL),
		m_texture_id(0),
		m_original_width(0),
		m_original_height(0)
		{
		}
};
	

}	// namespace gnash

#endif // GNASH_BITMAP_INFO_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
