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
#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

#include <iostream>
#include <string>

#include "MovieClip.h"
#include "tu_file.h"
#include "zlib_adapter.h"
//#include "stream.h"
//#include "jpeg.h"
//#include "fontlib.h"
//#include "font.h"
#include "log.h"
//#include "Sprite.h"
//#include "sprite_instance.h"
#include "render.h"

using namespace std;

namespace gnash
{

// Forward declarations

// @@ should be found somewhere else I guess..
//movie_interface* create_instance();

void
movieclip_init(as_object* /* global */)
{
#if 0
    // This is going to be the global MovieClip "class"/"function"
    static as_function *func=new function_as_object();

    // We make the 'prototype' element be a reference to
    // the __proto__ element
    as_object* proto = func->m_prototype;
    proto->add_ref();

    proto->set_member("constructor", func); //as_value(func));
    proto->set_member_flags("constructor", 1);

    func->set_member("prototype", as_value(proto));

    // Register _global.Function
    global->set_member("Function", func);
#endif
}

} // namespace gnash

