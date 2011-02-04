// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#ifndef __LAUNCHER_PLUGIN_H__
#define __LAUNCHER_PLUGIN_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <memory> // for auto_ptr
#include "as_object.h"

namespace gnash
{

class Launcher {
public:
    Launcher();
    ~Launcher();
    void launch(const char *filespec);
    const char *create_md5sum(const char *filespec);
    bool verify_md5sum(const char *filespec, const char *md5);
private:
    const char *_name;
};

extern "C" {
    void launcher_class_init(as_object &obj);  
    /// Return an  instance
}

std::auto_ptr<as_object> init_launcher_instance();

} // end of gnash namespace

// __LAUNCHER_PLUGIN_H__
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
