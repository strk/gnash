// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

namespace gnash {

/// Your render_handler creates BitmapInfos for gnash.  You
/// need to subclass BitmapInfo in order to add the
/// information and functionality your app needs to render
/// using textures.
class DSOEXPORT BitmapInfo : public ref_counted
{
public:

	BitmapInfo() {}

    virtual ~BitmapInfo() {}
};
	

}	// namespace gnash

#endif // GNASH_BITMAP_INFO_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
