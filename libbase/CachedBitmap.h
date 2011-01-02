// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#include "ref_counted.h"
#include "dsodefs.h"

namespace gnash {

namespace image {
    class GnashImage;
}

/// A CachedBitmap is created by the renderer in a format of its choosing.
//
/// CachedBitmaps are generally left alone by libcore, but the BitmapData
/// API provides a way of manipulating bitmaps. For this reason an image()
/// function is required, which must return a GnashImage for manipulation.
class DSOEXPORT CachedBitmap : public ref_counted
{
public:

    CachedBitmap() {}

    virtual ~CachedBitmap() {}

    /// Return a GnashImage for manipulation.
    //
    /// The changes to the data must be cached before the next rendering.
    virtual image::GnashImage& image() = 0;

    /// Free the memory associated with this CachedBitmap.
    //
    /// This allows ActionScript a little bit of control over memory.
    virtual void dispose() = 0;

    /// Whether the CachedBitmap has been disposed.
    //
    /// A disposed CachedBitmap has no data and should not be rendered.
    virtual bool disposed() const = 0;

};
	
} // namespace gnash

#endif


// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
