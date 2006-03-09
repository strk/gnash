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
// Linking Gnash statically or dynamically with other modules is making
// a combined work based on Gnash. Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
// 
// In addition, as a special exception, the copyright holders of Gnash give
// you permission to combine Gnash with free software programs or
// libraries that are released under the GNU LGPL and/or with Mozilla, 
// so long as the linking with Mozilla, or any variant of Mozilla, is
// through its standard plug-in interface. You may copy and distribute
// such a system following the terms of the GNU GPL for Gnash and the
// licenses of the other code concerned, provided that you include the
// source code of that other code when and as the GNU GPL requires
// distribution of source code. 
// 
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is
// their choice whether to do so.  The GNU General Public License gives
// permission to release a modified version without this exception; this
// exception also makes it possible to release a modified version which
// carries forward this exception.
//
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "TextSnapshot.h"

namespace gnash {

TextSnapshot::TextSnapshot() {
}

TextSnapshot::~TextSnapshot() {
}


void
TextSnapshot::findText()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
TextSnapshot::getCount()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
TextSnapshot::getSelected()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
TextSnapshot::getSelectedText()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
TextSnapshot::getText()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
TextSnapshot::hitTestTextNearPos()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
TextSnapshot::setSelectColor()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
TextSnapshot::setSelected()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void
textsnapshot_new(const fn_call& fn)
{
    textsnapshot_as_object *textsnapshot_obj = new textsnapshot_as_object;

    textsnapshot_obj->set_member("findtext", &textsnapshot_findtext);
    textsnapshot_obj->set_member("getcount", &textsnapshot_getcount);
    textsnapshot_obj->set_member("getselected", &textsnapshot_getselected);
    textsnapshot_obj->set_member("getselectedtext", &textsnapshot_getselectedtext);
    textsnapshot_obj->set_member("gettext", &textsnapshot_gettext);
    textsnapshot_obj->set_member("hittesttextnearpos", &textsnapshot_hittesttextnearpos);
    textsnapshot_obj->set_member("setselectcolor", &textsnapshot_setselectcolor);
    textsnapshot_obj->set_member("setselected", &textsnapshot_setselected);

    fn.result->set_as_object(textsnapshot_obj);
}
void textsnapshot_findtext(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void textsnapshot_getcount(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void textsnapshot_getselected(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void textsnapshot_getselectedtext(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void textsnapshot_gettext(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void textsnapshot_hittesttextnearpos(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void textsnapshot_setselectcolor(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void textsnapshot_setselected(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

} // end of gnaash namespace

