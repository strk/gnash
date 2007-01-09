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

/* $Id: xmlattrs.cpp,v 1.2 2007/01/09 15:14:20 rsavoye Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <vector>

#include "action.h"
#include "impl.h"
#include "tu_config.h"
#include "as_function.h"

#include "xmlattrs.h"

#ifdef DEBUG_MEMORY_ALLOCATION
	#include "log.h"
#endif

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


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
