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
//

#ifndef __MOUSE_H__
#define __MOUSE_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "impl.h"

namespace gnash {
  
class Mouse {
public:
    Mouse();
    ~Mouse();
   void addListener();
   void hide();
   void removeListener();
   void show();
private:
    bool _onown;
    bool _onove;
    bool _onp;
    bool _onheel;
};

class mouse_as_object : public as_object
{
public:
    Mouse obj;
};

void mouse_new(const fn_call& fn);
void mouse_addlistener(const fn_call& fn);
void mouse_hide(const fn_call& fn);
void mouse_removelistener(const fn_call& fn);
void mouse_show(const fn_call& fn);

} // end of gnash namespace

// __MOUSE_H__
#endif

