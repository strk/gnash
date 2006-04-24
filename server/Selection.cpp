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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "Selection.h"

namespace gnash {

Selection::Selection() {
}

Selection::~Selection() {
}


void
Selection::addListener()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Selection::getBeginIndex()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Selection::getCaretIndex()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Selection::getEndIndex()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Selection::getFocus()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Selection::removeListener()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Selection::setFocus()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
selection_new(const fn_call& fn)
{
    selection_as_object *selection_obj = new selection_as_object;

    selection_obj->set_member("addlistener", &selection_addlistener);
    selection_obj->set_member("getbeginindex", &selection_getbeginindex);
    selection_obj->set_member("getcaretindex", &selection_getcaretindex);
    selection_obj->set_member("getendindex", &selection_getendindex);
    selection_obj->set_member("getfocus", &selection_getfocus);
    selection_obj->set_member("removelistener", &selection_removelistener);
    selection_obj->set_member("setfocus", &selection_setfocus);

    fn.result->set_as_object(selection_obj);
}
void selection_addlistener(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void selection_getbeginindex(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void selection_getcaretindex(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void selection_getendindex(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void selection_getfocus(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void selection_removelistener(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void selection_setfocus(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

} // end of gnaash namespace

