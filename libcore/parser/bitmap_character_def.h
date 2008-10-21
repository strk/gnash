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

#ifndef GNASH_BITMAP_CHARACTER_DEF_H
#define GNASH_BITMAP_CHARACTER_DEF_H

#include "smart_ptr.h" // GNASH_USE_GC
#include "gnash.h" // for bitmap_info definition
#include "ref_counted.h" // for character_def inheritance
#include "smart_ptr.h"
#include "bitmap_info.h" // for dtor visibility by intrusive_ptr
#include "GC.h" // for GcResource (markReachableResources)

#include <cassert>
#include <memory> // for auto_ptr


namespace gnash {
namespace image {
	class ImageBase;
}
}

namespace gnash {

/// Definition of a bitmap character
//
/// This includes:
///
///	- SWF::DEFINEBITS
///	- SWF::DEFINEBITSJPEG2
///	- SWF::DEFINEBITSJPEG3
///	- SWF::DEFINELOSSLESS
///	- SWF::DEFINELOSSLESS2
///
/// The definition currently only takes an image::ImageRGB 
/// or image::ImageRGBA pointer. We should probably move
/// the methods for actually reading such tags instead.
///
/// One problem with this class is that it relies on the
/// availability of a render_handler in order to transform
/// image::ImageRGB or image::ImageRGBA to a bitmap_info.
///
class bitmap_character_def : public ref_counted // @@ why not character_def ?
{

public:

	/// Construct a bitmap_character_def from an image::ImageRGB
	//
	/// NOTE: uses currently registered render_handler to
	///       create a bitmap_info, don't call before a renderer
	///	  has been registered
	///
 	bitmap_character_def(std::auto_ptr<image::ImageBase> image);

	bitmap_info* get_bitmap_info() {
		return _bitmap_info.get();
	}

protected:

#ifdef GNASH_USE_GC
	/// Mark reachable resources (for GC)
	//
	/// Reachable resources are:
	///	- bitmap info (_bitmap_info)
	///
	void markReachableResources() const
	{
		if ( _bitmap_info ) _bitmap_info->setReachable();
	}
#endif // GNASH_USE_GC

private:

	boost::intrusive_ptr<bitmap_info> _bitmap_info;
};



}	// end namespace gnash


#endif // GNASH_BITMAP_CHARACTER_DEF_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
