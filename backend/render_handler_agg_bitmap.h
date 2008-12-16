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

#ifndef BACKEND_RENDER_HANDLER_AGG_BITMAP_H
#define BACKEND_RENDER_HANDLER_AGG_BITMAP_H

// This include file used only to make render_handler_agg more readable.


namespace gnash {

/// Bitmap class used internally by the AGG renderer. There's no reason to
/// use it outside. It does not much except providing all necessary information. 
class agg_bitmap_info_base : public BitmapInfo
{
public:

  int get_width() const { return m_width; }  
  int get_height() const { return m_height;  }  
  int get_bpp() const { return m_bpp; }  
  int get_rowlen() const { return m_rowlen; }  
  boost::uint8_t* get_data() const { return m_data; }

protected:
  boost::uint8_t* m_data;
  int m_width;
  int m_height;
  int m_bpp;
  int m_rowlen;  
};


/// The class itself uses a template. Currently this is unnecessary and it may
/// be removed but an older implementation required this method and it may be
/// necessary again when the last missing parts of the renderer will be
/// implemented. 
template <class PixelFormat>
class agg_bitmap_info : public agg_bitmap_info_base 
{
public:

  agg_bitmap_info(int width, int height, int rowlen, boost::uint8_t* data, int bpp) 
  {
    //printf("creating bitmap %dx%d pixels with %d bytes rowsize\n", width,height,rowlen);

    m_bpp = bpp;    
    m_width = width;
    m_height = height;
    m_rowlen = rowlen;

    m_data = new boost::uint8_t[height*rowlen];
    memcpy(m_data, data, height*rowlen);   
  }
  
  ~agg_bitmap_info() {
    delete [] m_data;
  }
    
};


} // namespace gnash

#endif // BACKEND_RENDER_HANDLER_AGG_BITMAP_H 
