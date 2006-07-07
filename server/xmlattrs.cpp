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

#include <sys/types.h>
#include <sys/stat.h>
#include <vector>

#include "log.h"
#include "action.h"
#include "impl.h"
#include "smart_ptr.h"
#include "tu_config.h"
#include "as_function.h"

#ifdef HAVE_LIBXML

#include "xmlattrs.h"

#include <unistd.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>

using namespace std;

namespace gnash {
  
//#define DEBUG_MEMORY_ALLOCATION 1
  
//std::vector<as_object *> _xmlobjs;    // FIXME: hack alert

XMLAttr::XMLAttr()
{
#ifdef DEBUG_MEMORY_ALLOCATION
    log_msg("\t\tCreating XMLAttr data at %p \n", this);
#endif
    _name = 0;
    _value = 0;
}

XMLAttr::~XMLAttr()
{
#ifdef DEBUG_MEMORY_ALLOCATION
    log_msg("\t\tDeleting XMLAttr data %s at %p \n", this->_name, this);
#endif
    //log_msg("%s: %p \n", __FUNCTION__, this);
    if (_name) {
        delete _name;
    }
    if (_value) {
        delete _value;
    }  
}  

} // end of gnash namespace

// HAVE_LIBXML
#endif


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
