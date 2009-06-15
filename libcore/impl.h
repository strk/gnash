// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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


#ifndef GNASH_IMPL_H
#define GNASH_IMPL_H

#include "dsodefs.h"

#include "smart_ptr.h"
#include "TagLoadersTable.h"

namespace gnash {

//
// Loader callbacks.
//
	
// Register a loader function for a certain tag type.  Most
// standard tags are handled within gnash.  Host apps might want
// to call this in order to handle special tag types.

/// Register a tag loader for the given tag
void register_tag_loader(SWF::TagType t,
        SWF::TagLoadersTable::TagLoader lf);
	
}	// end namespace gnash


#endif // GNASH_IMPL_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
