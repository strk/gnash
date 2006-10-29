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

#ifndef __TEXTSNAPSHOT_H__
#define __TEXTSNAPSHOT_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "impl.h"

namespace gnash {
  
class TextSnapshot {
public:
    TextSnapshot();
    ~TextSnapshot();
   void findText();
   void getCount();
   void getSelected();
   void getSelectedText();
   void getText();
   void hitTestTextNearPos();
   void setSelectColor();
   void setSelected();
private:
};

class textsnapshot_as_object : public as_object
{
public:
    TextSnapshot obj;
};

void textsnapshot_new(const fn_call& fn);
void textsnapshot_findtext(const fn_call& fn);
void textsnapshot_getcount(const fn_call& fn);
void textsnapshot_getselected(const fn_call& fn);
void textsnapshot_getselectedtext(const fn_call& fn);
void textsnapshot_gettext(const fn_call& fn);
void textsnapshot_hittesttextnearpos(const fn_call& fn);
void textsnapshot_setselectcolor(const fn_call& fn);
void textsnapshot_setselected(const fn_call& fn);

} // end of gnash namespace

// __TEXTSNAPSHOT_H__
#endif

