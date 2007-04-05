// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

/* $Id: TextFormat.h,v 1.1 2007/04/05 01:28:40 nihilus Exp $ */

#ifndef __GNASH_ASOBJ_TEXTFORMAT_H__
#define __GNASH_ASOBJ_TEXTFORMAT_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <memory> // for auto_ptr

namespace gnash {

class as_object;

/// Initialize the global Boolean class
void textformat_class_init(as_object& global);

/// Return a TextFormat instance (in case the core lib needs it)
//std::auto_ptr<as_object> init_textformat_instance();
  
} // end of gnash namespace

// __GNASH_ASOBJ_BOOLEAN_H__
#endif

