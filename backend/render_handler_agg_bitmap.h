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

#ifndef BACKEND_RENDER_HANDLER_AGG_BITMAP_H
#define BACKEND_RENDER_HANDLER_AGG_BITMAP_H

// This include file used only to make render_handler_agg more readable.

namespace gnash {

/// The class itself uses a template. Currently this is unnecessary and it may
/// be removed but an older implementation required this method and it may be
/// necessary again when the last missing parts of the renderer will be
/// implemented. And when might that be? I don't think I'll wait.
class agg_bitmap_info : public BitmapInfo
{
public:

  agg_bitmap_info(std::auto_ptr<GnashImage> im)
      :
      _image(im),
      _bpp(_image->type() == GNASH_IMAGE_RGB ? 24 : 32)
  {
  }
 
  int get_width() const { return _image->width(); }  
  int get_height() const { return _image->height();  }  
  int get_bpp() const { return _bpp; }  
  int get_rowlen() const { return _image->pitch(); }  
  boost::uint8_t* get_data() const { return _image->data(); }
  
private:

  std::auto_ptr<GnashImage> _image;

  int _bpp;
    
};


} // namespace gnash

#endif // BACKEND_RENDER_HANDLER_AGG_BITMAP_H 
