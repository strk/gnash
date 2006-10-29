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

#ifndef __CUSTOMACTIONS_H__
#define __CUSTOMACTIONS_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "impl.h"

namespace gnash {
  
class CustomActions {
public:
    CustomActions();
    ~CustomActions();
   void get();
   void install();
   void list();
   void uninstall();
private:
};

class customactions_as_object : public as_object
{
public:
    CustomActions obj;
};

void customactions_new(const fn_call& fn);
void customactions_get(const fn_call& fn);
void customactions_install(const fn_call& fn);
void customactions_list(const fn_call& fn);
void customactions_uninstall(const fn_call& fn);

} // end of gnash namespace

// __CUSTOMACTIONS_H__
#endif

