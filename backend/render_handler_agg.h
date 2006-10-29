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

/* $Id: render_handler_agg.h,v 1.7 2006/10/15 10:10:23 nihilus Exp $ */

#ifndef BACKEND_RENDER_HANDLER_AGG_H
#define BACKEND_RENDER_HANDLER_AGG_H

namespace gnash {

// Base class to shield GUIs from AGG's pixelformat classes 
class render_handler_agg_base : public render_handler
{
public:
  // these methods need to be accessed from outside:
  virtual void init_buffer(unsigned char *mem, int size, int x, int y)=0;  
};


/// Create a render handler 
DSOEXPORT render_handler_agg_base*
  create_render_handler_agg(char *pixelformat);

} // namespace gnash

#endif // BACKEND_RENDER_HANDLER_CAIRO_H
