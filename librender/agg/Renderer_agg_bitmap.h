// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#ifndef BACKEND_RENDER_HANDLER_AGG_BITMAP_H
#define BACKEND_RENDER_HANDLER_AGG_BITMAP_H

#include <memory>
#include <memory>
#include <boost/cstdint.hpp>

#include "GnashImage.h"
#include "CachedBitmap.h"

namespace gnash {

class agg_bitmap_info : public CachedBitmap
{
public:
  
    agg_bitmap_info(std::unique_ptr<image::GnashImage> im)
        :
        _image(im.release()),
        _bpp(_image->type() == image::TYPE_RGB ? 24 : 32)
    {
    }
  
    image::GnashImage& image() {
        assert(!disposed());
        return *_image;
    }
  
    void dispose() {
        _image.reset();
    }

    bool disposed() const {
        return !_image.get();
    }
   
    int get_width() const { return _image->width(); }  
    int get_height() const { return _image->height();  }  
    int get_bpp() const { return _bpp; }  
    int get_rowlen() const { return _image->stride(); }  
    boost::uint8_t* get_data() const { return _image->begin(); }
    
private:
  
    std::unique_ptr<image::GnashImage> _image;
  
    int _bpp;
      
};


} // namespace gnash

#endif // BACKEND_RENDER_HANDLER_AGG_BITMAP_H 
